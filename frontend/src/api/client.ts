async function request<T>(path: string, init?: RequestInit): Promise<T> {
  const resp = await fetch(`/api/v1${path}`, {
    headers: init?.body ? { "Content-Type": "application/json" } : undefined,
    ...init,
  });
  if (!resp.ok) {
    let message = `Request failed (${resp.status})`;
    try {
      const body = await resp.json();
      if (body.error) message = body.error;
    } catch {
      // keep default
    }
    throw new Error(message);
  }
  return resp.json() as Promise<T>;
}

export const api = {
  serverInfo: () => request<import("./types").ServerInfo>("/server"),
  health: () => request<{ ok: boolean; error?: string }>("/health"),
  listInstances: () => request<import("./types").Instance[]>("/instances"),
  getInstance: (name: string) =>
    request<import("./types").Instance>(`/instances/${encodeURIComponent(name)}`),
  instanceAction: (name: string, action: string) =>
    request<{ ok: boolean }>(`/instances/${encodeURIComponent(name)}/${action}`, {
      method: "POST",
      body: "{}",
    }),
  deleteInstance: (name: string) =>
    request<{ ok: boolean }>(`/instances/${encodeURIComponent(name)}/delete`, {
      method: "POST",
      body: "{}",
    }),
  createInstance: (spec: import("./types").CreateInstanceSpec) =>
    request<{ ok: boolean }>("/instances", {
      method: "POST",
      body: JSON.stringify(spec),
    }),
  listImages: () => request<import("./types").ImageInfo[]>("/images"),
  pullImage: (ref: string) =>
    request<{ ok: boolean; operation: string }>("/images/pull", {
      method: "POST",
      body: JSON.stringify({ image: ref }),
    }),
  runningOperations: () =>
    request<import("./types").RunningOperation[]>("/operations"),
  remoteImages: () => request<import("./types").RemoteCatalog>("/remote-images"),
  storage: () => request<import("./types").StoragePool[]>("/storage"),
  metricsHistory: () =>
    request<import("./types").MetricsHistory>("/metrics/history"),
  listSnapshots: (name: string) =>
    request<import("./types").Snapshot[]>(
      `/instances/${encodeURIComponent(name)}/snapshots`
    ),
  createSnapshot: (name: string, snapName?: string, stateful = false) =>
    request<{ ok: boolean; name: string }>(
      `/instances/${encodeURIComponent(name)}/snapshots`,
      {
        method: "POST",
        body: JSON.stringify({ name: snapName, stateful }),
      }
    ),
  restoreSnapshot: (name: string, snapshot: string) =>
    request<{ ok: boolean }>(
      `/instances/${encodeURIComponent(
        name
      )}/snapshots/${encodeURIComponent(snapshot)}/restore`,
      { method: "POST", body: "{}" }
    ),
  deleteSnapshot: (name: string, snapshot: string) =>
    request<{ ok: boolean }>(
      `/instances/${encodeURIComponent(
        name
      )}/snapshots/${encodeURIComponent(snapshot)}/delete`,
      { method: "POST", body: "{}" }
    ),
  refreshRemoteImages: () =>
    request<import("./types").RemoteCatalog>("/remote-images/refresh", {
      method: "POST",
      body: "{}",
    }),
  deleteImage: (fingerprint: string) =>
    request<{ ok: boolean }>(
      `/images/${encodeURIComponent(fingerprint)}`,
      { method: "DELETE" }
    ),
  listProfiles: () => request<string[]>("/profiles"),
  listStoragePools: () =>
    request<import("./types").StoragePool[]>("/storage-pools"),
  listNetworks: () => request<import("./types").NetworkInfo[]>("/networks"),
};

export function execWsUrl(name: string, shell: string): string {
  const proto = location.protocol === "https:" ? "wss" : "ws";
  return `${proto}://${location.host}/api/v1/instances/${encodeURIComponent(
    name
  )}/exec?shell=${encodeURIComponent(shell)}`;
}

export function eventsUrl(): string {
  return "/api/v1/events";
}
