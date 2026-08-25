export interface ServerInfo {
  name: string;
  version: string;
  os: string;
  kernel?: string;
  architectures: string[];
  cpu?: number;
  memoryTotal?: number;
  project: string;
  provider: string;
  auth?: "trusted" | "untrusted";
  message?: string;
}

export interface InterfaceAddress {
  address: string;
  family: "inet" | "inet6" | string;
  netmask?: string;
  scope?: string;
}

export interface NetworkInterface {
  name: string;
  addresses: InterfaceAddress[];
}

export interface DiskUsage {
  name: string;
  usage?: number | null;
  total?: number | null;
}

export interface InstanceState {
  pid?: number;
  processes?: number;
  memoryUsed?: number;
  memoryUsagePeak?: number;
  cpuSeconds?: number;
  interfaces?: NetworkInterface[];
  disks?: DiskUsage[];
}

export interface Instance {
  name: string;
  status: string;
  statusCode: number;
  type: "container" | "virtual-machine";
  profiles: string[];
  createdAt: string;
  state?: InstanceState | null;
  config?: Record<string, unknown>;
  devices?: Record<string, unknown>;
}

export interface ImageInfo {
  fingerprint: string;
  description: string;
  os: string;
  release: string;
  sizeBytes: number;
  aliases: string[];
  uploadedAt: string;
  instanceType?: "container" | "virtual-machine" | null;
}

export interface StoragePool {
  name: string;
  driver: string;
  description: string;
  status: string;
}

export interface ProfileDevice {
  type: string;
  [key: string]: unknown;
}

export interface ProfileInfo {
  name: string;
  description: string;
  config: Record<string, string>;
  devices: Record<string, ProfileDevice>;
  usedBy?: { kind: string; name: string }[];
}

export interface NetworkUsedBy {
  kind: "instance" | "profile" | "other";
  name: string;
}

export interface NetworkInfo {
  name: string;
  type: string;
  description: string;
  managed: boolean;
  status?: string;
  ipv4?: string | null;
  ipv6?: string | null;
  ipv4Nat?: boolean;
  ipv6Nat?: boolean;
  usedBy?: NetworkUsedBy[];
}

export interface RemoteImage {
  ref: string;
  os: string;
  release: string;
  title: string;
  version: string;
  arch: string;
  variant: string;
  aliases: string[];
  sizes: Partial<Record<"container" | "virtual-machine", number>>;
  published: string;
}

export interface RemoteCatalog {
  fetchedAt: string | null;
  images: RemoteImage[];
}

export interface StorageVolume {
  name: string;
  type: "container" | "virtual-machine" | "image" | "custom" | string;
  contentType: string;
  imageDescription?: string;
}

export interface StoragePool {
  name: string;
  driver: string;
  description: string;
  status: string;
  usedByCount: number;
  usage?: { used: number; total: number } | null;
  volumes: StorageVolume[];
}

export interface MetricsPoint {
  timestamp: string;
  instances: Record<
    string,
    {
      cpuSeconds?: number;
      memoryUsed?: number;
      memTotal?: number;
      netRx?: number;
      netTx?: number;
      diskRead?: number;
      diskWrite?: number;
      processes?: number;
    }
  >;
}

export interface MetricsHistory {
  interval: number;
  instances: string[];
  points: MetricsPoint[];
}

export interface Snapshot {
  name: string;
  createdAt: string;
  stateful: boolean;
  expiresAt: string | null;
}

export interface RunningOperation {
  id: string;
  description: string;
  status: string;
  statusCode: number;
  metadata: Record<string, unknown>;
}

export interface CreateInstanceSpec {
  name: string;
  image: string;
  type: "container" | "virtual-machine";
  profiles: string[];
  limits: { cpu?: number; memory?: string; disk_gb?: number };
  disk_pool?: string;
  network?: string | null;
  config?: Record<string, string>;
}

export interface IncusEvent {
  type: string;
  timestamp: string;
  metadata: unknown;
}
