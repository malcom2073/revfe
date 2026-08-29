import { test, expect } from "@playwright/test";
import {
  installApiMocks,
  installExecWsMock,
  mockInstances,
  mockInstanceDetail,
  mockMetricsHistory,
  mockRemoteCatalog,
  sseScript,
  FULL_FP,
} from "./mocks";

test.beforeEach(async ({ page }) => {
  await installApiMocks(page);
});

test("shell shows server info and nav", async ({ page }) => {
  await page.goto("/");
  const masthead = page.getByRole("banner");
  await expect(masthead.getByText("RevFe")).toBeVisible();
  await expect(masthead.getByText("test-host")).toBeVisible();
  await expect(masthead.getByText("Incus 6.0.0")).toBeVisible();

  const nav = page.getByRole("navigation");
  await expect(nav.getByRole("link", { name: /Dashboard/ })).toBeVisible();
  await expect(nav.getByRole("link", { name: /Instances/ })).toBeVisible();
});

test("dashboard shows live metrics charts and stat cards", async ({ page }) => {
  await installApiMocks(page);
  await page.goto("/");

  const main = page.getByRole("main");
  await expect(
    page.getByRole("heading", { name: "Dashboard" })
  ).toBeVisible();

  // Stat cards derived from the latest sample
  await expect(main.getByText("Instances").first()).toBeVisible();
  await expect(main.getByText("12", { exact: true })).toBeVisible(); // total processes: 4 + 8
  await expect(main.getByText(/354.8 MiB/)).toBeVisible(); // memory used

  // Charts render with legends per instance
  await expect(main.getByText("CPU usage by instance (%)")).toBeVisible();
  await expect(main.getByText("Memory usage by instance (MiB)")).toBeVisible();
  await expect(main.getByText("Network throughput (KiB/s)")).toBeVisible();
  await expect(main.getByText("web-01").first()).toBeVisible();

  // Regression: line/area paths must have valid coordinates (a double-"Z"
  // timestamp bug once rendered every x as NaN with axes still visible).
  await page.waitForTimeout(500);
  const nanPaths = await main.locator("path[d*='NaN']").count();
  expect(nanPaths).toBe(0);
  const areaPaths = await main.locator("path[d^='M']").count();
  expect(areaPaths).toBeGreaterThan(0);
});

test("instances table lists both instances with live state", async ({
  page,
}) => {
  await page.goto("/instances");
  await expect(
    page.getByRole("heading", { name: "Instances" })
  ).toBeVisible();
  const table = page.getByRole("grid");
  await expect(table).toBeVisible();
  for (const inst of mockInstances) {
    await expect(table.getByRole("link", { name: inst.name })).toBeVisible();
  }
  await expect(table.getByText("Running").first()).toBeVisible();
  await expect(table.getByText("Stopped").first()).toBeVisible();
  await expect(table.getByText("VM").first()).toBeVisible();
  await expect(table.getByText("CT").first()).toBeVisible();
  // Real bug regression: memory must be human-formatted, not raw bytes
  await expect(table.getByText("256 MiB")).toHaveCount(1);
  await expect(table.getByText("268435456")).toHaveCount(0);
});

test("instance actions call the API and refresh", async ({ page }) => {
  const api = await installApiMocks(page);
  await page.route("**/api/v1/instances/web-01/stop", (route) => {
    api.counts["web-01:stop"] = (api.counts["web-01:stop"] ?? 0) + 1;
    return route.fulfill({ json: { ok: true } });
  });
  await page.route("**/api/v1/instances/debian-vm/start", (route) => {
    api.counts["debian-vm:start"] = (api.counts["debian-vm:start"] ?? 0) + 1;
    return route.fulfill({ json: { ok: true } });
  });

  await page.goto("/instances");
  const runningRow = page.getByRole("row", { name: /web-01/ });
  await runningRow
    .getByRole("button", { name: "Stop web-01", exact: true })
    .click();
  await expect
    .poll(() => api.counts["web-01:stop"] ?? 0, { timeout: 5_000 })
    .toBe(1);

  const stoppedRow = page.getByRole("row", { name: /debian-vm/ });
  await stoppedRow
    .getByRole("button", { name: "Start debian-vm", exact: true })
    .click();
  await expect
    .poll(() => api.counts["debian-vm:start"] ?? 0, { timeout: 5_000 })
    .toBe(1);
});

test("create wizard walks through all tabs and submits", async ({ page }) => {
  const api = await installApiMocks(page);
  await page.goto("/instances");
  await page.getByRole("button", { name: "Create instance" }).click();

  const dialog = page.getByRole("dialog");

  // General tab: name + type + image
  await dialog.getByLabel("Name", { exact: false }).fill("e2e-box");
  await dialog
    .getByLabel("Instance type")
    .selectOption({ label: "Container (system)" });
  const imageInput = dialog.getByLabel("Image reference");
  await expect(imageInput).toBeVisible();
  await imageInput.fill("images:debian/13");

  // Resources tab
  await dialog.getByRole("tab", { name: "Resources" }).click();
  await dialog
    .getByRole("spinbutton", { name: "CPU limit" })
    .fill("2");
  await dialog
    .getByRole("spinbutton", { name: "Memory limit" })
    .fill("512");
  await dialog
    .getByRole("spinbutton", { name: /Root disk size/ })
    .fill("10");
  await expect(
    dialog.getByRole("combobox", { name: "Storage pool" })
  ).toContainText("default");

  // Configuration tab: network + config key
  await dialog.getByRole("tab", { name: "Configuration" }).click();
  await dialog.getByRole("combobox", { name: "NIC network" }).selectOption("incusbr0");
  await dialog.getByRole("button", { name: "Add config key" }).click();
  await dialog.getByPlaceholder("e.g. boot.autostart").fill("boot.autostart");
  await dialog.getByPlaceholder("true").fill("true");

  // Review tab: verify summary + create
  await dialog.getByRole("tab", { name: "Review" }).click();
  await expect(dialog.getByText("e2e-box")).toBeVisible();
  await expect(dialog.getByText("images:debian/13")).toBeVisible();
  await dialog.getByRole("button", { name: "Create instance" }).click();

  await expect
    .poll(() => api.counts.create ?? 0, { timeout: 5_000 })
    .toBe(1);
  const payload = api.lastCreatePayload() as Record<string, unknown>;
  expect(payload.image).toBe("images:debian/13");
  expect(payload.network).toBe("incusbr0");
  expect(payload.config).toMatchObject({ "boot.autostart": "true" });
  await expect(dialog).not.toBeVisible();
});

test("create wizard filters local images by instance type", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  await page.goto("/instances");
  await page.getByRole("button", { name: "Create instance" }).click();

  const dialog = page.getByRole("dialog");
  await dialog.getByLabel("Name", { exact: false }).fill("fp-box");

  // Container is the default type: the VM-only image must be hidden.
  await expect(dialog.getByRole("radio", { name: /Debian 13 amd64/ })).toBeVisible();
  await expect(
    dialog.getByRole("radio", { name: new RegExp(FULL_FP.slice(0, 12)) })
  ).toHaveCount(0);

  // Switch to VM: container image hides, VM image appears.
  await dialog
    .getByLabel("Instance type")
    .selectOption({ label: "Virtual machine" });
  await expect(
    dialog.getByRole("radio", { name: new RegExp(FULL_FP.slice(0, 12)) })
  ).toBeVisible();
  await expect(
    dialog.getByRole("radio", { name: /Debian 13 amd64/ })
  ).toHaveCount(0);
});

test("create wizard sends full fingerprint for alias-less local images", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  await page.goto("/instances");
  await page.getByRole("button", { name: "Create instance" }).click();

  const dialog = page.getByRole("dialog");
  await dialog.getByLabel("Name", { exact: false }).fill("fp-box");

  // The alias-less image in mocks is VM-only; switch type so it shows.
  await dialog
    .getByLabel("Instance type")
    .selectOption({ label: "Virtual machine" });

  // Regression: alias-less cached images must send the FULL fingerprint,
  // not a truncated stub that Incus cannot resolve.
  await dialog
    .getByRole("radio", { name: new RegExp(FULL_FP.slice(0, 12)) })
    .check();
  await expect(dialog.getByLabel("Image reference")).toHaveValue(FULL_FP);

  // Click through to Review and create
  await dialog.getByRole("tab", { name: "Review" }).click();
  await expect(dialog.getByText(FULL_FP)).toBeVisible();
  await dialog.getByRole("button", { name: "Create instance" }).click();

  await expect
    .poll(() => api.counts.create ?? 0, { timeout: 5_000 })
    .toBe(1);
  const payload = api.lastCreatePayload() as Record<string, unknown>;
  expect(payload.image).toBe(FULL_FP);
  expect(payload.type).toBe("virtual-machine");
});

test("instance detail overview renders stats", async ({ page }) => {
  await page.goto("/instances/web-01");
  await expect(page.getByRole("heading", { name: "web-01" })).toBeVisible();
  await expect(page.getByText("container")).toBeVisible();
  await expect(page.getByText("256 MiB")).toHaveCount(1); // memory used
  await expect(page.getByText("2m 0s")).toBeVisible(); // cpu time formatted
});

test("instance detail network tab shows addresses with copy buttons", async ({
  page,
}) => {
  await page.goto("/instances/web-01");
  const main = page.getByRole("main");
  await main.getByRole("tab", { name: "Network" }).click();

  await expect(main.getByText("eth0")).toBeVisible();
  await expect(main.getByText("10.227.129.137").first()).toBeVisible();
  await expect(main.getByText("inet").first()).toBeVisible();
  await expect(main.getByText("global").first()).toBeVisible();
  await expect(
    main.getByRole("button", { name: "Copy 10.227.129.137" })
  ).toBeVisible();
});

test("instance detail disks tab shows usage bars", async ({ page }) => {
  await page.goto("/instances/web-01");
  const main = page.getByRole("main");
  await main.getByRole("tab", { name: "Disks" }).click();

  await expect(main.getByText("Disk root")).toBeVisible();
  await expect(main.getByText(/1 GiB of 10 GiB/)).toBeVisible();
});

test("images page lists pre-downloaded images with delete action", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  let deleteCalls = 0;
  await page.route("**/api/v1/images/*", (route) => {
    if (route.request().method() === "DELETE") {
      deleteCalls++;
      return route.fulfill({ json: { ok: true } });
    }
    return route.fallback();
  });

  // Nav link is enabled and routes to the Images page
  await page.goto("/");
  await page.getByRole("navigation").getByRole("link", { name: /Images/ }).click();
  await expect(page.getByRole("heading", { name: "Images", exact: true })).toBeVisible();

  const table = page.getByRole("grid");
  await expect(table.getByText("Debian 13 amd64")).toBeVisible();
  await expect(
    table.getByText(FULL_FP.slice(0, 12)).first()
  ).toBeVisible();
  // Type badges
  await expect(table.getByText("CT", { exact: true })).toBeVisible();
  await expect(table.getByText("VM", { exact: true })).toBeVisible();
  // Alias label shown
  await expect(table.getByText("images:debian/13")).toBeVisible();

  // Delete hits DELETE /api/v1/images/<full fingerprint>
  await table
    .getByRole("button", { name: `Delete image ${FULL_FP.slice(0, 12)}` })
    .click();
  await expect.poll(() => deleteCalls, { timeout: 5_000 }).toBe(1);
});

test("images page tracks concurrent downloads with progress", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  api.useSseScript(sseScript);
  api.setSseDelayMs(4_000);

  await page.goto("/images");
  const main = page.getByRole("main");

  // In-flight download resumed from /operations
  await expect(main.getByText("Image download (running)")).toBeVisible();

  // Start a pull from the form
  await page.getByRole("button", { name: "Pull image" }).click();
  await page.getByLabel("Remote reference").fill("images:alpine/3.22");
  await page.getByRole("button", { name: "Start download" }).click();
  await expect
    .poll(() => api.counts.pull ?? 0, { timeout: 5_000 })
    .toBe(1);
  expect(api.lastPullPayload()).toMatchObject({ image: "images:alpine/3.22" });

  // Our new download shows up as its own progress bar
  await expect(main.getByText("Downloading images:alpine/3.22")).toBeVisible();

  // op-pull-a succeeds via SSE -> its bar disappears and images refresh;
  // the unrelated seeded download bar stays up
  await expect(
    main.getByText("Downloading images:alpine/3.22")
  ).toHaveCount(0, { timeout: 10_000 });
  await expect(main.getByText("Image download (running)")).toBeVisible();

  // op-pull-b fails -> danger alert surfaces the error after cleanup delay
  await expect(
    main.getByText(/Image download failed: no space left on device/)
  ).toBeVisible({ timeout: 15_000 });
});

test("images page catalog is cached, searchable, and downloads on demand", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  let refreshCalls = 0;
  await page.route("**/api/v1/remote-images", (route) => {
    if (route.request().method() === "GET") {
      return route.fulfill({
        json: refreshCalls > 0 ? mockRemoteCatalog : { fetchedAt: null, images: [] },
      });
    }
    return route.fallback();
  });
  await page.route("**/api/v1/remote-images/refresh", (route) => {
    refreshCalls++;
    return route.fulfill({ json: mockRemoteCatalog });
  });

  await page.goto("/images");
  const main = page.getByRole("main");

  // No cache yet -> hint shown, search disabled
  await expect(
    main.getByText(/No catalog cached yet/)
  ).toBeVisible();
  await expect(
    main.getByRole("textbox", { name: "Search remote images" })
  ).toBeDisabled();

  // Explicit refresh fetches the catalog
  await main.getByRole("button", { name: "Refresh image list" }).click();
  await expect.poll(() => refreshCalls).toBe(1);
  const table = main.getByRole("grid", { name: "Remote images table" });
  await expect(table.getByText("Alpine 3.22")).toBeVisible();
  await expect(table.getByText("Ubuntu 24.04 LTS")).toBeVisible();
  await expect(table.getByText("Debian 13")).toBeVisible();
  await expect(
    main.getByText(/Catalog fetched .* · 3 images/)
  ).toBeVisible();

  // Search narrows the list
  await main
    .getByRole("textbox", { name: "Search remote images" })
    .fill("ubuntu");
  await expect(table.getByText("Ubuntu 24.04 LTS")).toBeVisible();
  await expect(table.getByText("Alpine 3.22")).toHaveCount(0);

  // Download button kicks off a pull with the row's ref, targeting the
  // images: remote (bare refs would be treated as local aliases)
  await table
    .getByRole("button", { name: "Download ubuntu/24.04" })
    .click();
  await expect
    .poll(() => api.counts.pull ?? 0, { timeout: 5_000 })
    .toBe(1);
  expect(api.lastPullPayload()).toMatchObject({ image: "images:ubuntu/24.04" });
  await expect(main.getByText("Downloading images:ubuntu/24.04")).toBeVisible();
});

test("storage page shows pools and maps image volumes", async ({ page }) => {
  await installApiMocks(page);

  // Storage is reachable from the nav
  await page.goto("/");
  await page
    .getByRole("navigation")
    .getByRole("link", { name: /Storage/ })
    .click();
  await expect(page.getByRole("heading", { name: "Storage" })).toBeVisible();

  const main = page.getByRole("main");
  await expect(main.getByText("btrfs")).toBeVisible(); // driver badge
  await expect(main.getByText(/5 volumes/)).toBeVisible();
  await expect(main.getByText(/used by 7 resources/)).toBeVisible();
  await expect(main.getByText("Space")).toBeVisible();
  await expect(main.getByText(/4.4 GiB of 850.8 GiB/)).toBeVisible();

  const table = main.getByRole("grid");
  // Image volumes resolve to human descriptions when known…
  await expect(table.getByText("Ubuntu 24.04 LTS")).toBeVisible();
  // …and fall back to short fingerprints for unknown images
  await expect(table.getByText("abc123def456 (image)")).toBeVisible();
  // Instance + custom volumes render with type badges
  await expect(table.getByText("web-01")).toBeVisible();
  await expect(table.getByText("backup-vol")).toBeVisible();
  await expect(table.getByText("CT", { exact: true })).toBeVisible();
  await expect(table.getByText("VM", { exact: true })).toBeVisible();
});

test("networks page lists networks with subnets and consumers", async ({
  page,
}) => {
  await installApiMocks(page);

  await page.goto("/");
  await page
    .getByRole("navigation")
    .getByRole("link", { name: /Networks/ })
    .click();
  await expect(page.getByRole("heading", { name: "Networks" })).toBeVisible();

  const table = page.getByRole("grid");
  const bridgeRow = table.getByRole("row", { name: /incusbr0/ });
  await expect(bridgeRow.getByText("managed")).toBeVisible();
  await expect(bridgeRow.getByText("Created")).toBeVisible();
  await expect(bridgeRow.getByText("10.227.129.1/24")).toBeVisible();
  await expect(bridgeRow.getByText(/NAT/).first()).toBeVisible();
  // used-by labels: profile + instances
  await expect(bridgeRow.getByText("default")).toBeVisible();
  await expect(bridgeRow.getByText("web-01")).toBeVisible();
  await expect(bridgeRow.getByText("debian-vm")).toBeVisible();

  // Physical NIC: unmanaged, no subnets
  const nicRow = table.getByRole("row", { name: /enp0s31f6/ });
  await expect(nicRow.getByText("physical")).toBeVisible();
});

test("snapshots tab lists, creates, restores, deletes", async ({ page }) => {
  const api = await installApiMocks(page);
  await page.goto("/instances/web-01");
  const main = page.getByRole("main");

  await main.getByRole("tab", { name: "Snapshots" }).click();
  const table = main.getByRole("grid");
  await expect(table.getByText("snap-pre-upgrade")).toBeVisible();
  await expect(table.getByText("clean-install")).toBeVisible();

  // Create with an explicit name
  await main.getByRole("button", { name: "Take snapshot" }).click();
  const dlg = page.getByRole("dialog");
  await expect(dlg.getByText(/Take a snapshot of web-01/)).toBeVisible();
  await dlg.getByLabel("Snapshot name").fill("e2e-snap");
  await dlg.getByRole("button", { name: "Create snapshot" }).click();
  await expect
    .poll(() => api.counts.snapshotCreate ?? 0, { timeout: 5_000 })
    .toBe(1);
  expect(api.lastSnapshotPayload()).toMatchObject({ name: "e2e-snap" });
  await expect(main.getByText(/Snapshot ".+" created./)).toBeVisible();

  // Restore requires confirmation
  await table
    .getByRole("button", { name: "Restore snap-pre-upgrade" })
    .click();
  await dlg
    .getByRole("button", { name: "Restore", exact: true })
    .click();
  await expect
    .poll(() => api.counts.snapshotRestore ?? 0, { timeout: 5_000 })
    .toBe(1);

  // Delete requires confirmation
  await table.getByRole("button", { name: "Delete clean-install" }).click();
  await dlg
    .getByRole("button", { name: "Delete", exact: true })
    .click();
  await expect
    .poll(() => api.counts.snapshotDelete ?? 0, { timeout: 5_000 })
    .toBe(1);
});

test("profiles page shows config, devices, and consumers", async ({ page }) => {
  await installApiMocks(page);

  await page.goto("/");
  await page
    .getByRole("navigation")
    .getByRole("link", { name: /Profiles/ })
    .click();
  await expect(page.getByRole("heading", { name: "Profiles" })).toBeVisible();

  const main = page.getByRole("main");
  await expect(main.getByText("Default Incus profile")).toBeVisible();
  // Consumers
  await expect(main.getByText(/Used by web-01, debian-vm/)).toBeVisible();
  // Devices table with typed badges + details
  const devicesTable = main.getByRole("grid", { name: "Devices of default" });
  await expect(devicesTable.getByText("eth0").first()).toBeVisible();
  await expect(devicesTable.getByText("network=incusbr0")).toBeVisible();
  // Empty config messaging
  await expect(
    main.getByText(/No config keys — all limits unset/)
  ).toBeVisible();
});

test("profiles can be created, edited, and deleted", async ({ page }) => {
  const api = await installApiMocks(page);
  await page.goto("/profiles");
  const main = page.getByRole("main");

  // Create
  await main.getByRole("button", { name: "Create profile" }).click();
  const modal = page.getByRole("dialog");
  await expect(modal.getByRole("heading")).toContainText("Create profile");
  await modal.locator("#prof-name").fill("gpu-box");
  await modal.getByLabel("Description").fill("ML instances with GPU");
  // device: switch type to gpu, name it and set a known field
  await modal.getByLabel("Device name 1").fill("gpu0");
  await modal.getByLabel("Device type 1").selectOption("gpu");
  await modal.getByLabel("pci value").fill("0000:01:00.0");
  await modal
    .getByRole("button", { name: "Create profile" })
    .click();
  await expect
    .poll(() => api.counts.profileCreate ?? 0, { timeout: 5_000 })
    .toBe(1);
  const created = api.lastProfilePayload() as Record<string, unknown>;
  expect(created.name).toBe("gpu-box");
  expect(created.description).toBe("ML instances with GPU");
  const createdDevices = created.devices as Array<Record<string, unknown>>;
  expect(createdDevices[0].name).toBe("gpu0");
  expect(createdDevices[0].type).toBe("gpu");
  expect(createdDevices[0].pci).toBe("0000:01:00.0");
  await expect(main.getByText('Profile "gpu-box" saved.')).toBeVisible();

  // Edit existing default profile: add a config key
  await main.getByRole("button", { name: "Edit default" }).click();
  const editModal = page.getByRole("dialog");
  await editModal.getByRole("button", { name: "Add config key" }).click();
  await editModal.getByLabel("Config key 1", { exact: true }).fill("limits.cpu");
  await editModal.getByLabel("Config value 1").fill("4");
  await editModal.getByRole("button", { name: "Save changes" }).click();
  await expect
    .poll(() => api.counts.profileUpdate ?? 0, { timeout: 5_000 })
    .toBe(1);
  const updated = api.lastProfilePayload() as Record<string, unknown>;
  expect(updated.config).toMatchObject({ "limits.cpu": "4" });
  // devices preserved through the edit round-trip
  const updatedDevices = updated.devices as Array<{ name: string }>;
  expect(updatedDevices.map((d) => d.name)).toEqual(["eth0", "root"]);
});

test("profile delete requires confirmation", async ({ page }) => {
  const api = await installApiMocks(page);
  await page.goto("/profiles");
  const main = page.getByRole("main");

  await main.getByRole("button", { name: "Delete default" }).click();
  const dlg = page.getByRole("dialog");
  await expect(dlg.getByText(/permanently removed/)).toBeVisible();
  await dlg.getByRole("button", { name: "Delete", exact: true }).click();
  await expect
    .poll(() => api.counts.profileDelete ?? 0, { timeout: 5_000 })
    .toBe(1);
  await expect(main.getByText('Profile "default" deleted.')).toBeVisible();
});

test("create wizard still lists profile checkboxes from rich API", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  await page.goto("/instances");
  await page.getByRole("button", { name: "Create instance" }).click();
  const dialog = page.getByRole("dialog");
  // Profiles are on the General tab — visible immediately
  await expect(dialog.getByRole("checkbox", { name: "default" })).toBeChecked();
});

test("stopped instance detail offers Start button and blocks console", async ({
  page,
}) => {
  await page.goto("/instances/debian-vm");
  await page.getByRole("tab", { name: "Console" }).click();
  await expect(
    page.getByText("Console requires a running instance")
  ).toBeVisible();
});

test("running instance console opens terminal over mocked websocket", async ({
  page,
}) => {
  await installExecWsMock(page, "root@web-01:~# ");
  await page.goto("/instances/web-01");
  await page.getByRole("tab", { name: "Console" }).click();

  const terminal = page.locator(".xterm-screen");
  await expect(terminal).toBeVisible();
  await expect(page.locator(".xterm-rows")).toContainText("Connected.");
  await expect(page.locator(".xterm-rows")).toContainText("root@web-01");
});

test("instances list delete requires confirmation and calls the API", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  await page.goto("/instances");

  const row = page.getByRole("row", { name: /debian-vm/ });
  await row
    .getByRole("button", { name: "Delete debian-vm", exact: true })
    .click();

  const dlg = page.getByRole("dialog");
  await expect(
    dlg.getByText(/permanently removes the instance/)
  ).toBeVisible();
  // Cancel keeps it alive
  await dlg.getByRole("button", { name: "Cancel" }).click();
  await expect(dlg).toHaveCount(0);

  await row
    .getByRole("button", { name: "Delete debian-vm", exact: true })
    .click();
  await page
    .getByRole("dialog")
    .getByRole("button", { name: "Delete", exact: true })
    .click();
  await expect
    .poll(() => api.counts.delete ?? 0, { timeout: 5_000 })
    .toBe(1);
  await expect(page.getByRole("dialog")).toHaveCount(0);
});

test("instance detail delete confirms and navigates back to the list", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  await page.goto("/instances/web-01");
  await page.getByRole("button", { name: "Delete" }).click();
  const dlg = page.getByRole("dialog");
  await expect(dlg.getByText(/permanently removes the instance/)).toBeVisible();
  await dlg.getByRole("button", { name: "Delete", exact: true }).click();
  await expect
    .poll(() => api.counts.delete ?? 0, { timeout: 5_000 })
    .toBe(1);
  await expect(page.getByRole("heading", { name: "Instances" })).toBeVisible();
});

test("snapshot list badges and stateful creation round-trip", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  await page.goto("/instances/web-01");
  const main = page.getByRole("main");
  await main.getByRole("tab", { name: "Snapshots" }).click();

  // Existing snapshots carry stateful/stateless badges
  const table = main.getByRole("grid");
  await expect(table.getByText("stateless", { exact: true })).toBeVisible();
  await expect(table.getByText("stateful", { exact: true })).toBeVisible();

  // Auto-named + stateful create
  await main.getByRole("button", { name: "Take snapshot" }).click();
  const dlg = page.getByRole("dialog");
  await dlg.getByLabel("Include instance state (stateful)").check();
  await dlg.getByRole("button", { name: "Create snapshot" }).click();
  await expect
    .poll(() => api.counts.snapshotCreate ?? 0, { timeout: 5_000 })
    .toBe(1);
  expect(api.lastSnapshotPayload()).toMatchObject({ stateful: true });
  expect(api.lastSnapshotPayload() as Record<string, unknown>).not.toHaveProperty(
    "name"
  );
});

test("catalog refresh failure surfaces an error alert", async ({ page }) => {
  await page.route("**/api/v1/remote-images/refresh", (route) =>
    route.fulfill({ status: 502, json: { error: "upstream timed out" } })
  );
  await page.goto("/images");
  const main = page.getByRole("main");
  await main.getByRole("button", { name: "Refresh image list" }).click();
  await expect(main.getByText("upstream timed out")).toBeVisible();
});

test("create wizard blocks invalid names and missing images", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  await page.goto("/instances");
  await page.getByRole("button", { name: "Create instance" }).click();
  const dialog = page.getByRole("dialog");
  const createBtn = dialog.getByRole("button", {
    name: "Create instance",
    exact: true,
  });

  await expect(createBtn).toBeDisabled();

  // Invalid name shows helper text and stays blocked
  await dialog.getByLabel("Name", { exact: false }).fill("bad name!!");
  await expect(dialog.getByText(/1-63 characters/)).toBeVisible();
  await expect(createBtn).toBeDisabled();

  // Valid name + image unlocks creation
  await dialog.getByLabel("Name", { exact: false }).fill("good-name");
  await expect(dialog.getByText(/1-63 characters/)).toHaveCount(0);
  await dialog.getByLabel("Image reference").fill("ubuntu:24.04");
  await expect(createBtn).toBeEnabled();

  // Removing the image blocks again
  await dialog.getByLabel("Image reference").fill("");
  await expect(createBtn).toBeDisabled();
});

test("instance action buttons respect state and busy lock", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  await page.goto("/instances");
  const table = page.getByRole("grid");
  const webRow = table.getByRole("row", { name: /web-01/ });
  const stoppedRow = table.getByRole("row", { name: /debian-vm/ });

  // Running: stop/restart enabled, start disabled
  await expect(
    webRow.getByRole("button", { name: "Stop web-01", exact: true })
  ).toBeEnabled();
  await expect(
    webRow.getByRole("button", { name: "Restart web-01", exact: true })
  ).toBeEnabled();
  await expect(
    webRow.getByRole("button", { name: "Start web-01", exact: true })
  ).toBeDisabled();
  // Stopped: start enabled, stop/restart disabled
  await expect(
    stoppedRow.getByRole("button", { name: "Start debian-vm", exact: true })
  ).toBeEnabled();
  await expect(
    stoppedRow.getByRole("button", { name: "Stop debian-vm", exact: true })
  ).toBeDisabled();
  await expect(
    stoppedRow.getByRole("button", { name: "Restart debian-vm", exact: true })
  ).toBeDisabled();

  // While an action is in flight every action button is locked
  let startCompleted = 0;
  await page.route("**/api/v1/instances/debian-vm/start", async (route) => {
    await new Promise((r) => setTimeout(r, 2500));
    startCompleted++;
    await route.fulfill({ json: { ok: true } });
  });
  await stoppedRow
    .getByRole("button", { name: "Start debian-vm", exact: true })
    .click();
  await expect(
    webRow.getByRole("button", { name: "Restart web-01", exact: true })
  ).toBeDisabled();
  await expect(
    stoppedRow.getByRole("button", { name: "Stop debian-vm", exact: true })
  ).toBeDisabled();
  // ...and unlock again after completion
  await expect
    .poll(() => startCompleted, { timeout: 5_000 })
    .toBe(1);
  await expect(
    webRow.getByRole("button", { name: "Restart web-01", exact: true })
  ).toBeEnabled();
});

test("instances page renders error banner and empty state", async ({
  page,
}) => {
  await page.route("**/api/v1/instances", (route) =>
    route.fulfill({ status: 500, json: { error: "boom" } })
  );
  await page.goto("/instances");
  await expect(page.getByText("Could not load instances")).toBeVisible();
  await expect(page.getByText("boom")).toBeVisible();

  await page.route("**/api/v1/instances", (route) =>
    route.fulfill({ json: [] })
  );
  await page.reload();
  await expect(page.getByText("No instances found.")).toBeVisible();
});

test("storage page shows error and empty states", async ({ page }) => {
  await page.route("**/api/v1/storage", (route) =>
    route.fulfill({ status: 500, json: { error: "pool offline" } })
  );
  await page.goto("/storage");
  await expect(page.getByText("Could not load storage")).toBeVisible();

  await page.route("**/api/v1/storage", (route) => route.fulfill({ json: [] }));
  await page.reload();
  await expect(page.getByText("No storage pools found.")).toBeVisible();
});

test("dashboard stat cards refresh on the poll interval", async ({
  page,
}) => {
  let mem = 121_000_000;
  await page.route("**/api/v1/metrics/history", (route) => {
    const points = mockMetricsHistory.points.map((p, i) => ({
      ...p,
      instances: {
        "web-01": { ...p.instances["web-01"], memoryUsed: mem + i * 1000 },
        "debian-vm": p.instances["debian-vm"],
      },
    }));
    return route.fulfill({ json: { ...mockMetricsHistory, points } });
  });
  await page.goto("/");
  const main = page.getByRole("main");
  await expect(main.getByText(/353.8 MiB/)).toBeVisible();

  mem = 500_000_000;
  await expect(main.getByText(/715.3 MiB/)).toBeVisible({ timeout: 15_000 });
});

test("console shell toggle reconnects with the new shell", async ({ page }) => {
  let wsUrl = "";
  await page.routeWebSocket(/\/api\/v1\/instances\/.*\/exec.*/, (ws) => {
    wsUrl = ws.url();
    setTimeout(() => ws.send("root@web-01:~# "), 50);
  });
  await page.goto("/instances/web-01");
  const main = page.getByRole("main");
  await main.getByRole("tab", { name: "Console" }).click();

  const toggle = main.getByRole("button", { name: /Shell: bash/ });
  await expect(toggle).toBeVisible();
  await expect.poll(() => wsUrl, { timeout: 5_000 }).toContain("shell=bash");

  await toggle.click();
  await expect(main.getByRole("button", { name: /Shell: sh/ })).toBeVisible();
  await expect.poll(() => wsUrl, { timeout: 5_000 }).toContain("shell=sh");
});

test("profile editor can add then remove config keys and devices", async ({
  page,
}) => {
  await installApiMocks(page);
  await page.goto("/profiles");
  const main = page.getByRole("main");
  await main.getByRole("button", { name: "Create profile" }).click();
  const modal = page.getByRole("dialog");

  // Config key add + remove
  await modal.getByRole("button", { name: "Add config key" }).click();
  await modal
    .getByLabel("Config key 1", { exact: true })
    .fill("limits.cpu");
  await modal.getByRole("button", { name: "Remove config key 1" }).click();
  await expect(
    modal.getByLabel("Config key 1", { exact: true })
  ).toHaveCount(0);

  // Device row add + remove
  await modal.getByRole("button", { name: "Add device" }).click();
  await expect(modal.getByLabel("Device name 2")).toBeVisible();
  await modal.getByRole("button", { name: "Remove device 2" }).click();
  await expect(modal.getByLabel("Device name 2")).toHaveCount(0);
});

test("instance edit modal patches config, devices, and profiles", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  await page.goto("/instances/web-01");
  await page.getByRole("button", { name: "Edit" }).click();
  const modal = page.getByRole("dialog");
  await expect(modal.getByText('Edit instance "web-01"')).toBeVisible();

  // General: the default profile is preselected; removing the only one blocks saving
  await expect(modal.getByRole("checkbox", { name: "default" })).toBeChecked();
  await modal.getByRole("checkbox", { name: "default" }).uncheck();
  await expect(
    modal.getByText("At least one profile is required.")
  ).toBeVisible();
  await expect(
    modal.getByRole("button", { name: "Save changes" })
  ).toBeDisabled();
  await modal.getByRole("checkbox", { name: "default" }).check();

  // Configuration: tweak an existing key, add a new one; devices pre-seeded
  await modal.getByRole("tab", { name: "Configuration" }).click();
  await modal.getByLabel("Config value 1", { exact: true }).fill("4");
  await modal.getByRole("button", { name: "Add config key" }).click();
  await modal.getByLabel("Config key 3", { exact: true }).fill("boot.autostart");
  await modal.getByLabel("Config value 3", { exact: true }).fill("true");
  await expect(modal.getByLabel("Device name 1")).toHaveValue("eth0");
  await expect(modal.getByLabel("Device name 2")).toHaveValue("root");

  await modal.getByRole("button", { name: "Save changes" }).click();
  await expect
    .poll(() => api.counts.instanceUpdate ?? 0, { timeout: 5_000 })
    .toBe(1);

  const payload = api.lastInstanceUpdatePayload() as {
    profiles: string[];
    config: Record<string, string>;
    devices: Record<string, Record<string, string>>;
  };
  expect(payload.profiles).toEqual(["default"]);
  expect(payload.config).toMatchObject({
    "limits.cpu": "4",
    "boot.autostart": "true",
  });
  expect(payload.devices.eth0).toMatchObject({ type: "nic", network: "incusbr0" });
  expect(payload.devices.root).toMatchObject({
    type: "disk",
    path: "/",
    pool: "default",
  });
  await expect(page.getByText("Instance configuration saved.")).toBeVisible();
});

test("instance edit modal surfaces the underlying Incus error", async ({
  page,
}) => {
  await installApiMocks(page);
  await page.route("**/api/v1/instances/web-01", async (route) => {
    if (route.request().method() === "PATCH") {
      return route.fulfill({
        status: 400,
        json: { error: 'DNS name conflict between "eth1" and "eth0"' },
      });
    }
    return route.fulfill({ json: mockInstanceDetail("web-01") });
  });
  await page.goto("/instances/web-01");
  await page.getByRole("button", { name: "Edit" }).click();
  const modal = page.getByRole("dialog");
  await modal.getByRole("tab", { name: "Configuration" }).click();
  await modal.getByRole("button", { name: "Save changes" }).click();
  await expect(modal).toContainText("Update failed");
  await expect(modal).toContainText('between "eth1" and "eth0"');
});

test("config editor autocompletes known keys and shows descriptions", async ({
  page,
}) => {
  await installApiMocks(page);
  await page.goto("/instances/web-01");
  await page.getByRole("button", { name: "Edit" }).click();
  const modal = page.getByRole("dialog");
  await modal.getByRole("tab", { name: "Configuration" }).click();

  await modal.getByRole("button", { name: "Add config key" }).click();

  // Known keys are wired to a datalist for autocompletion.
  const datalist = page.locator("datalist#incus-config-keys");
  await expect(datalist.locator('option[value="limits.memory"]')).toHaveCount(1);
  await expect(datalist.locator('option[value="boot.autostart"]')).toHaveCount(1);
  await expect(datalist.locator('option[value="user.*"]')).toHaveCount(1);
  await expect(modal.getByLabel("Config key 3", { exact: true })).toHaveAttribute(
    "list",
    "incus-config-keys"
  );

  // Typing an exact known key shows its description.
  await modal.getByLabel("Config key 3", { exact: true }).fill("limits.memory");
  await expect(modal).toContainText(/Memory limit, e\.g/);

  // A key inside a free-form namespace (user.*) shows the namespace description.
  await modal.getByLabel("Config key 3", { exact: true }).fill("user.meta.key");
  await expect(modal).toContainText(/Free-form user key\/value/);

  // An unknown key gets no description (row 2's own limits.memory description stays).
  await modal.getByLabel("Config key 3", { exact: true }).fill("mystery.key");
  await expect(modal.getByText(/Memory limit, e\.g/)).toHaveCount(1);
});

test("create wizard config editor shows autocomplete suggestions and descriptions", async ({
  page,
}) => {
  await installApiMocks(page);
  await page.goto("/instances");
  await page.getByRole("button", { name: "Create instance" }).click();
  const dialog = page.getByRole("dialog");
  await dialog.getByRole("tab", { name: "Configuration" }).click();
  await dialog.getByRole("button", { name: "Add config key" }).click();

  await dialog.getByPlaceholder("e.g. boot.autostart").fill("security.nesting");
  await expect(dialog).toContainText(/Allow running Incus \(nested\)/);
  await expect(
    page.locator('datalist#incus-config-keys option[value="snapshots.schedule"]')
  ).toHaveCount(1);
});

test("instance rename posts and navigates to the new name", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  await page.goto("/instances/web-01");
  await page.getByRole("button", { name: "Rename" }).click();
  const modal = page.getByRole("dialog");
  await expect(modal.getByText('Rename instance "web-01"')).toBeVisible();

  const input = modal.getByLabel("New name");
  await expect(input).toHaveValue("web-01");
  await input.fill("web-01-prod");
  await modal.getByRole("button", { name: "Rename" }).click();

  await expect
    .poll(() => api.counts.rename ?? 0, { timeout: 5_000 })
    .toBe(1);
  expect((api.lastRenamePayload() as { name: string }).name).toBe("web-01-prod");
  await expect(page.getByRole("heading", { name: "web-01-prod" })).toBeVisible();
  await expect(page).toHaveURL(/\/instances\/web-01-prod$/);
});
