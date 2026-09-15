import { api } from "../api/client";
import type { ConfigKeyEntry } from "../api/types";

/** One-line description for a config key in `entries`, matching an exact key
 * first and then the most specific wildcard namespace (e.g. `user.*`). */
export function lookupDescription(
  entries: ConfigKeyEntry[],
  key: string
): string | null {
  const trimmed = key.trim();
  if (!trimmed) return null;
  const exact = entries.find((entry) => entry.key === trimmed);
  if (exact) return exact.description;
  const wildcards = entries
    .filter((entry) => entry.key.endsWith(".*"))
    .map((entry) => ({
      prefix: entry.key.slice(0, -1),
      description: entry.description,
    }))
    .filter((entry) => trimmed.startsWith(entry.prefix))
    .sort((a, b) => b.prefix.length - a.prefix.length);
  return wildcards.length > 0 ? wildcards[0].description : null;
}

let configKeysCache: Promise<ConfigKeyEntry[]> | null = null;

/** Config keys + descriptions sourced from the live daemon's metadata API
 * (`/1.0/metadata/configuration`). The promise is memoised so all editors
 * share one fetch; a failed fetch is not cached so a later mount can retry. */
export function getConfigKeys(): Promise<ConfigKeyEntry[]> {
  if (!configKeysCache) {
    configKeysCache = api.configKeys().catch((err) => {
      configKeysCache = null;
      throw err;
    });
  }
  return configKeysCache;
}