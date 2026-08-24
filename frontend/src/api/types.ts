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

export interface InstanceState {
  pid?: number;
  processes?: number;
  memoryUsed?: number;
  memoryUsagePeak?: number;
  cpuSeconds?: number;
  diskUsed?: number;
  diskTotal?: number;
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
