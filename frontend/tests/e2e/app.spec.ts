import { test, expect } from "@playwright/test";
import {
  installApiMocks,
  installExecWsMock,
  mockInstances,
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

test("create wizard walks through all steps and submits", async ({ page }) => {
  const api = await installApiMocks(page);
  await page.goto("/instances");
  await page.getByRole("button", { name: "Create instance" }).click();

  const dialog = page.locator(".pf-v6-c-wizard");

  // Step 1: Details
  await dialog.getByLabel("Name", { exact: false }).fill("e2e-box");
  await dialog
    .getByLabel("Instance type")
    .selectOption({ label: "Container (system)" });
  await dialog.getByRole("button", { name: "Next" }).click();

  // Step 2: Source — pick remote ref suggestion via input
  const imageInput = dialog.getByLabel("Image reference");
  await expect(imageInput).toBeVisible();
  await imageInput.fill("images:debian/13");
  await dialog.getByRole("button", { name: "Next" }).click();

  // Step 3: Resources
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
  await dialog.getByRole("button", { name: "Next" }).click();

  // Step 4: Advanced — network select + a config key
  await dialog.getByRole("combobox", { name: "NIC network" }).selectOption("incusbr0");
  await dialog.getByRole("button", { name: "Add config key" }).click();
  await dialog.getByPlaceholder("e.g. boot.autostart").fill("boot.autostart");
  await dialog.getByPlaceholder("true").fill("true");
  await dialog.getByRole("button", { name: "Next" }).click();

  // Step 5: Review + create
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
  // Wizard closes after creation
  await expect(dialog).not.toBeVisible();
});

test("create wizard filters local images by instance type", async ({
  page,
}) => {
  const api = await installApiMocks(page);
  await page.goto("/instances");
  await page.getByRole("button", { name: "Create instance" }).click();

  const dialog = page.locator(".pf-v6-c-wizard");
  await dialog.getByLabel("Name", { exact: false }).fill("fp-box");

  // Container is the default type: the VM-only image must be hidden.
  await dialog.getByRole("button", { name: "Next" }).click();
  await expect(dialog.getByRole("radio", { name: /Debian 13 amd64/ })).toBeVisible();
  await expect(
    dialog.getByRole("radio", { name: new RegExp(FULL_FP.slice(0, 12)) })
  ).toHaveCount(0);

  // Go back, switch to VM: container image hides, VM image appears.
  await dialog.getByRole("button", { name: "Back" }).click();
  await dialog
    .getByLabel("Instance type")
    .selectOption({ label: "Virtual machine" });
  await dialog.getByRole("button", { name: "Next" }).click();
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

  const dialog = page.locator(".pf-v6-c-wizard");
  await dialog.getByLabel("Name", { exact: false }).fill("fp-box");

  // The alias-less image in mocks is VM-only; switch type so it shows.
  await dialog
    .getByLabel("Instance type")
    .selectOption({ label: "Virtual machine" });
  await dialog.getByRole("button", { name: "Next" }).click();

  // Regression: alias-less cached images must send the FULL fingerprint,
  // not a truncated stub that Incus cannot resolve.
  await dialog
    .getByRole("radio", { name: new RegExp(FULL_FP.slice(0, 12)) })
    .check();
  await expect(dialog.getByLabel("Image reference")).toHaveValue(FULL_FP);
  await dialog.getByRole("button", { name: "Next" }).click();
  await dialog.getByRole("button", { name: "Next" }).click();
  await dialog.getByRole("button", { name: "Next" }).click();
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
