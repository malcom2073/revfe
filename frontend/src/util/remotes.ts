export const REMOTE_SUGGESTIONS = [
  "ubuntu:24.04",
  "ubuntu:22.04",
  "images:debian/13",
  "images:debian/12",
  "images:alpine/3.22",
  "images:fedora/42",
  "images:rockylinux/9",
];

/** Extract a percentage from an Incus progress string like
 * "Downloading image: 45% (10MB/s)" or "Unpack: 100% (5GB/s)". */
export function parseProgressPct(text?: string | null): number | null {
  if (!text) return null;
  const m = text.match(/(\d{1,3})%/);
  return m ? Number(m[1]) : null;
}
