import { Page, Route } from "@playwright/test";

export const mockServerInfo = {
  name: "test-host",
  version: "6.0.0",
  os: "Linux",
  kernel: "6.8.0-test",
  architectures: ["x86_64"],
  cpu: 8,
  memoryTotal: 34359738368,
  project: "default",
  provider: "incus",
  auth: "trusted",
};

export const mockInstances = [
  {
    name: "web-01",
    status: "Running",
    statusCode: 103,
    type: "container",
    profiles: ["default"],
    createdAt: "2026-08-20T10:00:00Z",
    state: {
      pid: 1234,
      processes: 42,
      memoryUsed: 268435456,
      memoryUsagePeak: 536870912,
      cpuSeconds: 120.5,
      diskUsed: 1073741824,
      diskTotal: 10737418240,
    },
  },
  {
    name: "debian-vm",
    status: "Stopped",
    statusCode: 102,
    type: "virtual-machine",
    profiles: ["default"],
    createdAt: "2026-08-21T13:37:27Z",
    state: null,
  },
];

export const mockInstanceDetail = (name: string) => {
  const base = mockInstances.find((i) => i.name === name) ?? mockInstances[0];
  return {
    ...base,
    config: { "limits.cpu": "2", "limits.memory": "512MiB" },
    devices: {},
    state:
      base.status === "Running"
        ? {
            pid: 1234,
            processes: 42,
            memoryUsed: 268435456,
            memoryUsagePeak: 536870912,
            cpuSeconds: 120.5,
            diskUsed: 1073741824,
            diskTotal: 10737418240,
          }
        : {},
  };
};

export const FULL_FP = "dcbf2416b4a6001f760e9a4b199e04c12614a1b4c6b8ca5c27ca9acfa358cf47";

export const mockImages = [
  {
    fingerprint: "abc123def4567890abcdef1234567890abcdef1234567890abcdef1234567890",
    description: "Debian 13 amd64",
    os: "Debian",
    release: "trixie",
    sizeBytes: 130_000_000,
    aliases: ["images:debian/13"],
    uploadedAt: "2026-08-01T00:00:00Z",
    instanceType: "container" as const,
  },
  {
    // VM-only cached image, no alias (common after pulling by fingerprint)
    fingerprint: FULL_FP,
    description: "",
    os: "",
    release: "",
    sizeBytes: 118_000_000,
    aliases: [],
    uploadedAt: "2026-08-20T00:00:00Z",
    instanceType: "virtual-machine" as const,
  },
];

export const mockRemoteCatalog = {
  fetchedAt: "2026-08-23T10:00:00Z",
  images: [
    {
      ref: "alpine/3.22",
      os: "Alpine",
      release: "3.22",
      title: "3.22",
      version: "",
      arch: "amd64",
      variant: "default",
      aliases: ["alpine/3.22"],
      sizes: { container: 5_000_000 },
      published: "20260822_23:08",
    },
    {
      ref: "ubuntu/24.04",
      os: "Ubuntu",
      release: "24.04",
      title: "24.04 LTS",
      version: "",
      arch: "amd64",
      variant: "default",
      aliases: ["ubuntu/24.04"],
      sizes: { container: 180_000_000, "virtual-machine": 600_000_000 },
      published: "20260814_07:00",
    },
    {
      ref: "debian/13/cloud",
      os: "Debian",
      release: "13",
      title: "13",
      version: "",
      arch: "arm64",
      variant: "cloud",
      aliases: ["debian/13/cloud"],
      sizes: { container: 90_000_000, "virtual-machine": 300_000_000 },
      published: "20260821_05:24",
    },
  ],
};

export const mockRunningOps = [
  {
    id: "op-existing-1",
    description: "Downloading image",
    status: "Running",
    statusCode: 103,
    metadata: { download_progress: "Retrieving image: 42% (10MB/s)" },
  },
];

/**
 * SSE stream for /api/v1/events that reports progress for two image
 * downloads, then completes one and fails the other.
 */
export const sseScript = [
  {
    type: "operation",
    timestamp: "2026-08-22T00:00:00Z",
    metadata: {
      id: "op-pull-a",
      description: "Downloading image",
      status: "Running",
      status_code: 103,
      metadata: { download_progress: "Retrieving image: 30% (8MB/s)" },
    },
  },
  {
    type: "operation",
    timestamp: "2026-08-22T00:00:01Z",
    metadata: {
      id: "op-pull-b",
      description: "Downloading image",
      status: "Running",
      status_code: 103,
      metadata: { download_progress: "Unpack: 66% (20MB/s)", progress: "legacy: 5%" },
    },
  },
  {
    type: "operation",
    timestamp: "2026-08-22T00:00:02Z",
    metadata: {
      id: "op-pull-a",
      description: "Downloading image",
      status: "Success",
      status_code: 200,
      metadata: {},
    },
  },
  {
    type: "operation",
    timestamp: "2026-08-22T00:00:03Z",
    metadata: {
      id: "op-pull-b",
      description: "Downloading image",
      status: "Failure",
      status_code: 400,
      metadata: { err: "no space left on device" },
    },
  },
]
  .map((e) => `data: ${JSON.stringify(e)}\n\n`)
  .join("");

export const mockPools = [
  { name: "default", driver: "btrfs", description: "", status: "Created" },
];

export const mockNetworks = [
  { name: "incusbr0", type: "bridge", description: "", managed: true },
  { name: "enp0s31f6", type: "physical", description: "", managed: false },
];

export function json(route: Route, body: unknown) {
  return route.fulfill({ json: body });
}

/** Wire up all /api/v1 mocks. Returns handles for asserting on calls. */
export interface MockApi {
  counts: Record<string, number>;
  lastCreatePayload: () => unknown;
  lastPullPayload: () => unknown;
  /** Replace the default (static) SSE stream with a custom body. */
  useSseScript: (body: string) => void;
  /** Delay SSE responses so tests can act first. */
  setSseDelayMs: (ms: number) => void;
}

export async function installApiMocks(page: Page): Promise<MockApi> {
  const counts: Record<string, number> = {};
  let createPayload: unknown;
  let pullPayload: unknown;
  let sseBody = ": connected\n\n";
  let sseDelayMs = 0;

  await page.route("**/api/v1/server", (route) => json(route, mockServerInfo));
  await page.route("**/api/v1/health", (route) =>
    json(route, { ok: true })
  );
  await page.route("**/api/v1/images", async (route) => {
    if (route.request().method() === "GET") return json(route, mockImages);
    return json(route, {});
  });
  await page.route("**/api/v1/images/pull", async (route) => {
    counts.pull = (counts.pull ?? 0) + 1;
    pullPayload = route.request().postDataJSON();
    return route.fulfill({ json: { ok: true, operation: "/1.0/operations/op-pull-a" }, status: 202 });
  });
  await page.route("**/api/v1/profiles", (route) =>
    json(route, ["default"])
  );
  await page.route("**/api/v1/storage-pools", (route) =>
    json(route, mockPools)
  );
  await page.route("**/api/v1/networks", (route) => json(route, mockNetworks));
  await page.route("**/api/v1/operations", (route) =>
    json(route, mockRunningOps)
  );
  await page.route("**/api/v1/remote-images", (route) =>
    json(route, { fetchedAt: null, images: [] })
  );
  await page.route("**/api/v1/events*", async (route) => {
    if (sseDelayMs > 0) {
      await new Promise((r) => setTimeout(r, sseDelayMs));
    }
    return route.fulfill({
      status: 200,
      headers: { "Content-Type": "text/event-stream" },
      body: sseBody,
    });
  });

  await page.route("**/api/v1/instances", async (route) => {
    const req = route.request();
    if (req.method() === "GET") return json(route, mockInstances);
    if (req.method() === "POST") {
      counts.create = (counts.create ?? 0) + 1;
      createPayload = req.postDataJSON();
      return json(route, { ok: true, metadata: {} });
    }
    return json(route, {});
  });

  await page.route(/\/api\/v1\/instances\/[^/]+$/, async (route) => {
    const url = new URL(route.request().url());
    const name = decodeURIComponent(url.pathname.split("/").pop() ?? "");
    if (!name || name === "instances") return json(route, {});
    return json(route, mockInstanceDetail(name));
  });

  return {
    counts,
    lastCreatePayload: () => createPayload,
    lastPullPayload: () => pullPayload,
    useSseScript: (body: string) => {
      sseBody = body;
    },
    setSseDelayMs: (ms: number) => {
      sseDelayMs = ms;
    },
  };
}

/**
 * Mock the exec WebSocket: echoes a banner and any text sent back
 * (uppercased marker so tests can assert round-trip).
 */
export async function installExecWsMock(page: Page, banner: string) {
  await page.routeWebSocket(/\/api\/v1\/instances\/.*\/exec.*/, (ws) => {
    ws.onMessage((message) => {
      if (typeof message === "string") {
        try {
          const parsed = JSON.parse(message);
          if ("resize" in parsed) return; // swallow resize control messages
        } catch {
          // fall through: treat as terminal input
        }
      }
      ws.send(message);
    });
    setTimeout(() => ws.send(banner), 50);
  });
}
