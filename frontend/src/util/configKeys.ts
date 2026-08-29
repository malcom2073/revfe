export interface ConfigKeyEntry {
  key: string;
  description: string;
}

/** Common Incus instance/profile config keys with one-line descriptions
 * (sourced from the Incus instance options reference). A trailing `.*`
 * entry describes the whole free-form namespace (e.g. `user.*`). */
export const KNOWN_CONFIG_KEYS: ConfigKeyEntry[] = [
  { key: "agent.nic_config", description: "Use the instance device names/MTUs for the VM's default NICs" },
  { key: "cluster.evacuate", description: "What to do with this instance when a cluster member evacuates" },
  { key: "environment.*", description: "Environment variables set on boot and during exec" },
  { key: "linux.kernel_modules", description: "Comma-separated kernel modules to load before start (container)" },
  { key: "linux.sysctl.*", description: "Override a sysctl setting inside the container" },
  { key: "smbios11.*", description: "Free-form SMBIOS Type 11 key/value (virtual machine)" },
  { key: "systemd.credential.*", description: "Systemd credential passed as a read-only bind mount (container)" },
  { key: "systemd.credential-binary.*", description: "Systemd credential key/value, value Base64-encoded" },
  { key: "user.*", description: "Free-form user key/value storage (searchable metadata)" },
  { key: "boot.autorestart", description: "Restart up to 10 times over a minute on unexpected exit" },
  { key: "boot.autostart", description: "Always start the instance when the daemon starts" },
  { key: "boot.autostart.delay", description: "Seconds to wait after start before starting the next instance" },
  { key: "boot.autostart.priority", description: "Start order — highest value starts first" },
  { key: "boot.host_shutdown_action", description: "Action on host shutdown: stop, force-stop or stateful-stop" },
  { key: "boot.host_shutdown_timeout", description: "Seconds to wait for shutdown before force-stopping" },
  { key: "boot.stop.priority", description: "Shutdown order — highest value stops first" },
  { key: "cloud-init.user-data", description: "Cloud-init user data (e.g. #cloud-config YAML)" },
  { key: "cloud-init.vendor-data", description: "Cloud-init vendor data, merged with user-data" },
  { key: "cloud-init.network-config", description: "Cloud-init network configuration" },
  { key: "user.user-data", description: "Legacy cloud-init user data (use cloud-init.user-data)" },
  { key: "user.vendor-data", description: "Legacy cloud-init vendor data (use cloud-init.vendor-data)" },
  { key: "user.network-config", description: "Legacy cloud-init network config (use cloud-init.network-config)" },
  { key: "limits.cpu", description: "Number of CPUs or a range/list to pin, e.g. 2, 0-3 or 1,2,3" },
  { key: "limits.cpu.allowance", description: "CPU share as a % (soft) or time like 25ms/100ms (hard; containers)" },
  { key: "limits.cpu.nodes", description: "NUMA node IDs (or 'balanced') to place the instance CPUs on" },
  { key: "limits.cpu.priority", description: "CPU scheduling priority vs other instances, 0-10" },
  { key: "limits.disk.priority", description: "Priority of the instance's I/O requests, 0-10" },
  { key: "limits.hugepages.1GB", description: "Limit for 1 GB huge pages (bytes, with suffix)" },
  { key: "limits.hugepages.1MB", description: "Limit for 1 MB huge pages (bytes, with suffix)" },
  { key: "limits.hugepages.2MB", description: "Limit for 2 MB huge pages (bytes, with suffix)" },
  { key: "limits.hugepages.64KB", description: "Limit for 64 KB huge pages (bytes, with suffix)" },
  { key: "limits.kernel.*", description: "Kernel resource limit, e.g. nofile=1000:2000" },
  { key: "limits.memory", description: "Memory limit, e.g. 512MiB, 1GiB or 50% of host memory" },
  { key: "limits.memory.enforce", description: "Whether the memory limit is hard or soft (containers)" },
  { key: "limits.memory.hotplug", description: "Disable memory hotplug or cap hotplugged memory (VMs)" },
  { key: "limits.memory.hugepages", description: "Back the VM using huge pages (true/false)" },
  { key: "limits.memory.oom_priority", description: "OOM killer adjustment, -1000..1000 (negative = less likely killed)" },
  { key: "limits.memory.swap", description: "Allow swapping of instance memory (true/false or bytes)" },
  { key: "limits.memory.swap.priority", description: "Swap priority, 0-10 (higher = less likely swapped)" },
  { key: "limits.processes", description: "Maximum number of processes (containers)" },
  { key: "migration.incremental.memory", description: "Use incremental memory transfer when migrating (containers)" },
  { key: "migration.incremental.memory.goal", description: "Target % of memory synced before stopping, default 70" },
  { key: "migration.incremental.memory.iterations", description: "Max transfer iterations before stopping, default 10" },
  { key: "migration.stateful", description: "Allow stateful stop/start and snapshots (saves runtime state)" },
  { key: "nvidia.driver.capabilities", description: "NVIDIA driver capabilities, default compute,utility" },
  { key: "nvidia.require.cuda", description: "Required CUDA version expression" },
  { key: "nvidia.require.driver", description: "Required driver version expression" },
  { key: "nvidia.runtime", description: "Pass the host NVIDIA/CUDA runtime libraries into the instance" },
  { key: "raw.apparmor", description: "Extra AppArmor profile rules for the container" },
  { key: "raw.idmap", description: "Custom uid/gid mappings: both 1000 1000" },
  { key: "raw.lxc", description: "Raw LXC configuration (container)" },
  { key: "raw.qemu", description: "Raw QEMU command-line flags (virtual machine)" },
  { key: "raw.qemu.conf", description: "Raw QEMU configuration file options (virtual machine)" },
  { key: "raw.seccomp", description: "Additional seccomp filtering rules (container)" },
  {
    key: "security.protection.delete",
    description: "Prevent the instance from being deleted",
  },
  { key: "security.protection.shift", description: "Prevent the file system from being UID/GID shifted" },
  { key: "security.protection.start", description: "Prevent the instance from being started" },
  {
    key: "security.nesting",
    description: "Allow running Incus (nested) inside a container / nested virtualization on VMs",
  },
  {
    key: "security.privileged",
    description: "Run the container in privileged mode (no user namespace)",
  },
  { key: "security.idmap.isolated", description: "Use a purely private UID/GID mapping for the instance" },
  { key: "security.idmap.size", description: "Size of the idmap when isolated or user-supplied" },
  { key: "security.guestapi", description: "Expose the guest API to the VM's incus-agent" },
  { key: "security.secureboot", description: "Enforce UEFI secure boot with the default Microsoft keys (VMs)" },
  { key: "security.selinux.domain", description: "Custom SELinux domain (container)" },
  { key: "security.selinux.type", description: "Custom SELinux type (container)" },
  { key: "security.syscalls.allow", description: "Comma-separated syscalls to allow (mutually exclusive with deny)" },
  { key: "security.syscalls.deny", description: "Comma-separated syscalls to deny" },
  { key: "security.syscalls.deny_default", description: "Whether to deny all syscalls by default (default true)" },
  { key: "snapshots.expiry", description: "When to delete scheduled snapshots, e.g. 1M 2H 3d 4w 5m 6y" },
  { key: "snapshots.expiry.manual", description: "When to delete manually created snapshots" },
  { key: "snapshots.pattern", description: "Snapshot name template, default snap%d" },
  { key: "snapshots.schedule", description: "Cron expression or alias (@hourly, @daily) for automatic snapshots" },
  { key: "snapshots.schedule.stopped", description: "Also snapshot instances that are stopped (true/false)" },
  { key: "snapshots.stateful", description: "Include runtime state in snapshots (VMs)" },
  {
    key: "volatile.*",
    description: "Internal runtime data managed by Incus — not user-editable",
  },
];

/** One-line description for a config key, matching an exact entry first and
 * then the most specific wildcard namespace (e.g. `user.*`). */
export function describeConfigKey(key: string): string | null {
  const trimmed = key.trim();
  if (!trimmed) return null;
  const exact = KNOWN_CONFIG_KEYS.find((entry) => entry.key === trimmed);
  if (exact) return exact.description;
  const wildcard = KNOWN_CONFIG_KEYS.filter((entry) => entry.key.endsWith(".*"))
    .map((entry) => ({ prefix: entry.key.slice(0, -1), description: entry.description }))
    .filter((entry) => trimmed.startsWith(entry.prefix))
    .sort((a, b) => b.prefix.length - a.prefix.length)[0];
  return wildcard ? wildcard.description : null;
}