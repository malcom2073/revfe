# RevFe TODO — gaps vs. the Incus API

Compiled against the Incus REST API reference (`doc/rest-api.yaml`, main branch).
Priorities: **P1** = core usability, **P2** = common admin tasks, **P3** = advanced/nice-to-have.

---

## P1 — Core instance & resource management

### 1. Edit an instance (config / devices / profiles)
- API: `PUT`/`PATCH /1.0/instances/{name}` (PUT replaces, PATCH merges — prefer PATCH for edits)
- UI: an "Edit" action on the instance detail page; reuse the creation wizard's structure but prepopulate
  current config, devices, and selected profiles. Apply changes live or via a "restart required" notice.
- Backend: `update_instance(name, patch, full_replace=False)` provider method + `PATCH /instances/<name>` route
- Frontend: instance edit modal, wire into instance detail; new `ClientInstanceEdit` spec type; refresh after save

### 2. Rename / move an instance
- API: `POST /1.0/instances/{name}` (body: `name` for rename; `pool`/`instance_only` for cross-pool move)
- UI: "Rename" action (Rename modal) on instance detail; move only makes sense if multiple storage pools exist
- Backend: `rename_instance(name, new_name, pool=None)` + `POST /instances/<name>/rename`
- Frontend: rename modal + `renameInstance(name, newName)`; update URL/breadcrumb after rename

### 3. Custom storage volume CRUD
- API: `GET/POST /1.0/storage-pools/{pool}/volumes` and
  `GET/PUT/PATCH/DELETE /1.0/storage-pools/{pool}/volumes/{type}/{name}`
  (type `custom` is the user-managed one; containers/VMs/images are read-only here)
- UI: on the Storage page, add a "Create volume" button (name, pool, size, content type filesystem/block),
  plus delete; later attach/detach from instances
- Backend: `create_volume(pool, name, size, content_type)`, `delete_volume(pool, name)`,
  `update_volume(...)` + routes; only `custom` volumes exposed as writable
- Frontend: volume create/delete modals on Storage page

### 4. Instance snapshots — edit, rename, expire
- API: `PUT`/`RENAME`/`DELETE /1.0/instances/{name}/snapshots/{snapshot}` (we already do delete/restore)
- UI: "Edit snapshot" (stateful toggle, expiry date) and "Rename" on the Snapshots tab
- Backend: `update_snapshot(name, snap, body)`, `rename_snapshot(name, snap, new)` + routes
- Frontend: extend SnapshotsTab row actions

### 5. Instance rebuild
- API: `POST /1.0/instances/{name}/rebuild` (re-apply image, keep name — useful for golden images in prod)
- UI: "Rebuild" button on instance detail with image picker (reuse General tab image selection) + confirm
- Backend: `rebuild_instance(name, image_ref)` + `POST /instances/<name>/rebuild`

### 6. Instance backups
- API: `GET/POST /1.0/instances/{name}/backups`, `GET/DELETE .../backups/{backup}`,
  `POST .../backups/{backup}/export` (stream via operation websocket)
- UI: "Backups" section on instance detail: list, create (optimized/compressed), download (export), delete, restore
- Backend: `create_backup`, `list_backups`, `delete_backup`, `export_backup` (stream file) + routes
- Frontend: backups tab/modal, download handling via blob

### 7. Instance file browser (push/pull)
- API: `GET /1.0/instances/{name}/files?path=...` (download file), `POST .../files` (upload),
  `DELETE .../files?path=...`, plus a `GET .../files?path=/&recursive=1` list via MD5/HMAC transport
- UI: "Files" tab on instance detail: browse directory tree, upload local file, edit/download/delete
- Backend: `get_file`, `put_file`, `delete_file`, `list_files` provider methods + routes
- Frontend: file browser component (needs a directory listing → recurse/parse headers approach)

### 8. Instance logs
- API: `GET /1.0/instances/{name}/logs` (list), `GET /1.0/instances/{name}/logs/{filename}`
  (includes `exec-output/` subdir; query `+&type=...` for plain/errored)
- UI: "Logs" tab on detail: file list + view raw text in a scrollable pre
- Backend: `list_instance_logs`, `get_instance_log(name, filename)` + routes
- Frontend: logs tab, link into console for quick debugging

---

## P2 — Common admin functionality

### 9. Real Settings page + server config editing
- API: `PATCH`/`PUT /1.0` (server config), `GET /1.0/resources` (hardware profile, partially used already)
- UI: enable the disabled "Settings" nav item: server version/env, config key/value editor (PATCH),
  storage/CPU/memory resource cards
- Backend: `update_server_config(patch)` + `PATCH /server`; reuse metrics sampler data for resources
- Frontend: Settings page, config editor reuse the profile config key/value table pattern

### 10. Image management — aliases & metadata
- API: `GET/POST /1.0/images/aliases`, `PUT/PATCH/DELETE /1.0/images/aliases/{name}`;
  `PATCH /1.0/images/{fingerprint}` (description, properties, auto-update, public); `POST .../refresh`
- UI: on Images page — alias table per image (add/remove/rename alias), edit image description/auto-update, refresh
- Backend: `create_image_alias`, `delete_image_alias`, `update_image`, `refresh_image` + routes
- Frontend: extend Images page with alias CRUD and image metadata edit

### 11. Image export / download
- API: `GET /1.0/images/{fingerprint}/export` (streams tarball)
- UI: "Download" button on Images page (right click to save)
- Backend: `export_image(fingerprint)` streaming route
- Frontend: download link/button, reuse blob handling from backups

### 12. Projects
- API: `GET/POST /1.0/projects`, `PUT/PATCH/DELETE /1.0/projects/{name}`,
  `GET /1.0/projects/{name}/state`; pass `?project=` on all resource reads
- UI: project switcher in the masthead + read-only project list somewhere (feature flag)
- Backend: `list_projects`, `get_project`, `create_project`, ... + `?project=` param threading
- Frontend: project selector; **today the app is hardcoded to the `default` project**
  (`incus.py` passes `project=default` on every call) — biggest structural change of the list

### 13. Network CRUD + state (bandwidth graphs)
- API: `POST /1.0/networks`, `PUT/PATCH/DELETE /1.0/networks/{name}`,
  `GET /1.0/networks/{name}/state`, `POST /1.0/networks/{name}` (rename)
- UI: on Networks page — create network modal (bridge/OVN), edit config, delete, "show usage" (link to a
  per-network state tab with RX/TX per-second from `/state` counters); rename action
- Backend: `create_network`, `update_network`, `delete_network`, `network_state` + routes
- Frontend: network create/edit modals, network detail view with live counters

### 14. DHCP leases
- API: `GET /1.0/networks/{name}/leases` (requires a managed network, e.g. bridge/OVN)
- UI: leases table (MAC, IPv4/IPv6, hostname, type, last use) on a network detail panel
- Backend: `network_leases(name)` + route
- Frontend: lease listing in network detail

### 15. Storage pool management
- API: `POST /1.0/storage-pools`, `PUT/PATCH/DELETE /1.0/storage-pools/{name}`,
  `POST /1.0/storage-pools/{name}` (rename), `GET /1.0/storage-pools/{name}/resources` (already used)
- UI: create pool (driver + name + config), edit config, delete (guarded when volumes exist)
- Backend: `create_pool`, `update_pool`, `delete_pool` + routes
- Frontend: storage pool create/edit modal; wire capacity display already present

### 16. Certificates (trust management)
- API: `GET/POST /1.0/certificates`, `PUT/PATCH/DELETE /1.0/certificates/{fingerprint}`
- UI: certificates table (fingerprint, name, type client/server, restricted, expiry) + add/remove
- Backend: `list_certificates`, `add_certificate`, `delete_certificate` + routes
- Frontend: certificates section (maybe under Settings), replaces the manual `make setup-cert` workflow

### 17. Field: Restart required / instance state reasons
- API: `GET /1.0/instances/{name}/state` already carries `last_error`, `disk`, etc.; expose them
- UI: show `last_error` in the detail overview / status column for stopped-errored instances
- Backend: `_normalize_state` already maps most fields — add `lastError` passthrough
- Frontend: render last error next to status; tooltip on red status

---

## P3 — Advanced / exploratory

### 18. Network ACLs, zones, address sets, integrations
- API: `GET/POST /1.0/network-acls`, `.../network-acls/{name}`, `.../{name}/log`;
  `GET/POST /1.0/network-zones`, `.../{zone}/records`;
  `GET/POST /1.0/network-address-sets`; `GET/POST /1.0/network-integrations`; `GET /1.0/network-allocations`
- UI: is a stretch; likely read-only lists with edit modals per resource type
- Backend/Frontend: generic "network config" CRUD; low priority until networks are used heavily

### 19. Warnings center
- API: `GET /1.0/warnings`, `PUT /1.0/warnings/{uuid}`, `DELETE /1.0/warnings/{uuid}`
- UI: bell icon in masthead → dropdown list of warnings (message, type, since) + dismiss/delete
- Backend: `list_warnings`, `update_warning(uuid, action)`, `delete_warning` + routes
- Frontend: global notifications UI; easy win via the existing SSE event stream

### 20. Port forward management
- API: `GET/POST /1.0/instances/{name}/port-forward`, `PUT/PATCH/DELETE .../port-forward/{id}`
- UI: forward rules table on instance detail (listen address/port → target port), create/edit/delete
- Backend: `list_port_forwards`, `create_port_forward`, `delete_port_forward` + routes
- Frontend: port forward tab/form (supports VM and container NICs)

### 21. VM console & agent actions
- API: `POST /1.0/instances/{name}/console` (websocket, VGA/serial), `GET .../nvram*`, `GET .../nbd`
- UI: for VMs, alternative "VGA console" in the Console tab (alongside the shell)
- Backend: console websocket bridge similar to `exec_bridge`
- Frontend: canvas/video renderer for SPICE-ish stream; requires research into Incus console protocol

### 22. Cluster management
- API: `GET/PUT /1.0/cluster`, `GET/POST /1.0/cluster/groups`, `/cluster/members` (+ `/{name}/state`)
- UI: cluster members table (member, status, load, roles) + groups
- Backend/Frontend: read-only dashboard first; join/bootstrap preseed later
- Not applicable on single-node setups, gated on `GET /1.0` `cluster.enabled`

### 23. Instance metadata / cloud-init inspection
- API: `GET /1.0/instances/{name}/metadata`, `GET .../metadata/templates` (config maps)
- UI: static editor/table showing instance metadata (cloud-init keys, templates)
- Backend/Frontend: read-only table + simple editor; low usage

### 24. Config key autocomplete (config metadata)
- API: `GET /1.0/metadata/configuration` — schema of all valid config keys, types, descriptions, defaults
- UI: use in **ProfileEditModal**, instance edit, and create "Extra configuration" to suggest keys
- Backend: `config_metadata()` passthrough route (or fetch once and cache)
- Frontend: datalist suggestions (like REMOTE_SUGGESTIONS) fed from the fetched dataset

### 25. Operation cancellation
- API: `DELETE /1.0/operations/{id}` (cancel), `GET /1.0/operations/{id}/wait`
- UI: in the image pull progress bar, add a "Cancel" button that calls the cancel endpoint
- Backend: `cancel_operation(id)` + route
- Frontend: cancel button next to download progress (Images page)

---

## Cross-cutting / notes

- **Project threading** (item 12) affects every provider call — the backend hardcodes `project=default`.
  All other items should be designed with a `project` parameter from day one.
- **Provider abstraction**: `storage_overview()` and `wait_operation()` live only on `IncusProvider`
  and aren't in the `Provider` protocol — normalize that as new methods get added.
- **404 handling**: e.g. `last_error` display and "profile in use" errors already surface Incus's own
  messages; keep that pattern when adding new failure surfaces (esp. volume/network pool delete guards).
- **E2E coverage**: every item above should add mocked-route Playwright coverage following
  `tests/e2e/mocks.ts` + `app.spec.ts` patterns before being considered done.