import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import {
  Alert,
  Button,
  Form,
  FormGroup,
  Label,
  TextInput,
  PageSection,
  Progress,
  Title,
  Toolbar,
  ToolbarContent,
  ToolbarItem,
} from "@patternfly/react-core";

import { Table, Tbody, Td, Th, Thead, Tr } from "@patternfly/react-table";
import { DownloadIcon, TimesIcon } from "@patternfly/react-icons";
import { api, eventsUrl } from "../api/client";
import type { ImageInfo, RemoteImage } from "../api/types";
import { REMOTE_SUGGESTIONS, parseProgressPct } from "../util/remotes";

interface Download {
  id: string;
  label: string;
  pct: number | null;
  err?: string;
}

const IMAGE_DOWNLOAD_DESCRIPTION = "Downloading image";

/** Incus uses "download_progress" for image pulls; older/other ops may say "progress". */
function opProgressText(
  md?: Record<string, unknown>
): string | undefined {
  if (!md) return undefined;
  const val = (md.download_progress ?? md.progress) as string | undefined;
  return typeof val === "string" ? val : undefined;
}

export default function Images() {
  const [images, setImages] = useState<ImageInfo[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [busy, setBusy] = useState<string | null>(null);
  const [pullOpen, setPullOpen] = useState(false);
  const [pullRef, setPullRef] = useState("");
  const [downloads, setDownloads] = useState<Download[]>([]);
  const [catalog, setCatalog] = useState<{
    fetchedAt: string | null;
    images: RemoteImage[];
  }>({ fetchedAt: null, images: [] });
  const [catalogSearch, setCatalogSearch] = useState("");
  const [refreshing, setRefreshing] = useState(false);
  const labelsRef = useRef<Map<string, string>>(new Map());

  const refresh = useCallback(() => {
    api
      .listImages()
      .then(setImages)
      .catch((e) => setError(e.message));
  }, []);

  // Seed in-flight image downloads (also survives page reloads)
  useEffect(() => {
    refresh();
    api
      .remoteImages()
      .then((c) => {
        if (c.fetchedAt) setCatalog(c);
      })
      .catch(() => void 0);
    api
      .runningOperations()
      .then((ops) => {
        setDownloads(
          ops
            .filter((op) => op.description === IMAGE_DOWNLOAD_DESCRIPTION)
            .map((op) => ({
              id: op.id,
              label:
                labelsRef.current.get(op.id) ??
                `Image download (${op.status.toLowerCase()})`,
              pct: parseProgressPct(opProgressText(op.metadata)),
            }))
        );
      })
      .catch(() => void 0);
  }, [refresh]);

  // Live updates for running downloads via SSE
  useEffect(() => {
    const source = new EventSource(eventsUrl());
    source.onmessage = (event) => {
      let parsed: { type?: string; metadata?: Record<string, unknown> };
      try {
        parsed = JSON.parse(event.data);
      } catch {
        return;
      }
      if (parsed.type !== "operation" || !parsed.metadata) return;
      const md = parsed.metadata as {
        id?: string;
        description?: string;
        status?: string;
        status_code?: number;
        metadata?: Record<string, unknown>;
      };
      if (md.description !== IMAGE_DOWNLOAD_DESCRIPTION || !md.id) return;
      const mdId: string = md.id;

      setDownloads((current) => {
        if (md.status_code === 200) {
          setTimeout(refresh, 500);
          return current.filter((d) => d.id !== mdId);
        }
        if (md.status_code === 400 || md.status_code === 401) {
          setTimeout(() => {
            setDownloads((cur2) => cur2.filter((d) => d.id !== mdId));
            setError(`Image download failed: ${md.metadata?.err ?? md.status}`);
          }, 5_000);
          return current.map((d) =>
            d.id === mdId
              ? {
                  ...d,
                  pct: 100,
                  err:
                    (md.metadata?.err as string | undefined) ?? md.status ?? "failed",
                }
              : d
          );
        }
        const existing = current.find((d) => d.id === mdId);
        if (!existing && !labelsRef.current.has(mdId)) return current;
        const pct = parseProgressPct(opProgressText(md.metadata));
        return current.map((d) => (d.id === mdId ? { ...d, pct } : d));
      });
    };
    return () => source.close();
  }, [refresh]);

  const remove = async (img: ImageInfo) => {
    setError(null);
    setBusy(img.fingerprint);
    try {
      await api.deleteImage(img.fingerprint);
      refresh();
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(null);
    }
  };

  const startPull = async (ref?: string) => {
    const imageRef = (ref ?? pullRef).trim();
    if (!imageRef) return;
    setError(null);
    try {
      const res = await api.pullImage(imageRef);
      const opId = res.operation?.split("/").pop() ?? "";
      labelsRef.current.set(opId, imageRef);
      setDownloads((current) => [
        ...current.filter((d) => d.id !== opId),
        { id: opId, label: imageRef, pct: 0 },
      ]);
      setPullOpen(false);
      setPullRef("");
    } catch (e) {
      setError((e as Error).message);
    }
  };

  const refreshCatalog = async () => {
    setRefreshing(true);
    setError(null);
    try {
      setCatalog(await api.refreshRemoteImages());
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setRefreshing(false);
    }
  };

  const filteredCatalog = useMemo(() => {
    const q = catalogSearch.trim().toLowerCase();
    if (!q) return catalog.images;
    return catalog.images.filter((i) =>
      `${i.os} ${i.release} ${i.title} ${i.version} ${i.arch} ${i.variant} ${i.ref}`
        .toLowerCase()
        .includes(q)
    );
  }, [catalog.images, catalogSearch]);

  return (
    <PageSection hasBodyWrapper={false}>
      <Title headingLevel="h1" size="2xl">
        Images
      </Title>
      {error && (
        <Alert variant="danger" title="Error" isInline className="pf-v6-u-mt-md">
          {error}
        </Alert>
      )}
      <Toolbar className="pf-v6-u-mt-md">
        <ToolbarContent>
          <ToolbarItem>
            <Button
              variant="primary"
              icon={<DownloadIcon />}
              onClick={() => setPullOpen((o) => !o)}
            >
              Pull image
            </Button>
          </ToolbarItem>
          <ToolbarItem>
            <Button variant="secondary" onClick={refresh}>
              Refresh
            </Button>
          </ToolbarItem>
        </ToolbarContent>
      </Toolbar>

      {pullOpen && (
        <Form isHorizontal className="pf-v6-u-mt-lg pf-v6-u-w-50">
          <FormGroup label="Remote reference" fieldId="pull-ref" isRequired>
            <TextInput
              id="pull-ref"
              value={pullRef}
              onChange={(_e, v) => setPullRef(v)}
              placeholder="images:debian/13"
              list="pull-suggestions"
              onKeyDown={(e) => {
                if (e.key === "Enter" && pullRef.trim()) startPull();
              }}
            />
            <datalist id="pull-suggestions">
              {REMOTE_SUGGESTIONS.map((s) => (
                <option key={s} value={s} />
              ))}
            </datalist>
          </FormGroup>
          <Button
            variant="primary"
            onClick={() => startPull()}
            isDisabled={!pullRef.trim()}
          >
            Start download
          </Button>
        </Form>
      )}

      {downloads.map((d) => (
        <div key={d.id} className="pf-v6-u-mt-lg" style={{ maxWidth: "480px" }}>
          <Progress
            title={`Downloading ${d.label}`}
            value={d.pct ?? 0}
            label={d.err ? `Failed: ${d.err}` : `${d.pct ?? 0}%`}
            variant={d.err ? "danger" : undefined}
          />
        </div>
      ))}

      <Title headingLevel="h2" size="lg" className="pf-v6-u-mt-xl">
        Available to download
      </Title>
      <Toolbar className="pf-v6-u-mt-sm">
        <ToolbarContent>
          <ToolbarItem>
            <Button
              variant="secondary"
              onClick={refreshCatalog}
              isLoading={refreshing}
              isDisabled={refreshing}
            >
              Refresh image list
            </Button>
          </ToolbarItem>
          {catalog.fetchedAt && (
            <ToolbarItem>
              <span className="pf-v6-u-text-color-subtle">
                Catalog fetched{" "}
                {new Date(catalog.fetchedAt).toLocaleTimeString()} ·{" "}
                {catalog.images.length} images
              </span>
            </ToolbarItem>
          )}
          <ToolbarItem>
            <TextInput
              aria-label="Search remote images"
              placeholder="Search by name, version, arch…"
              value={catalogSearch}
              onChange={(_e, v) => setCatalogSearch(v)}
              isDisabled={!catalog.fetchedAt}
            />
          </ToolbarItem>
        </ToolbarContent>
      </Toolbar>

      {!catalog.fetchedAt && (
        <Alert
          variant="info"
          isInline
          title='No catalog cached yet — click "Refresh image list" to fetch all downloadable images from the remote source.'
          className="pf-v6-u-mt-md"
        />
      )}

      {catalog.fetchedAt && (
        <Table
          aria-label="Remote images table"
          variant="compact"
          className="pf-v6-u-mt-md"
        >
          <Thead>
            <Tr>
              <Th>Image</Th>
              <Th>Arch</Th>
              <Th>Type</Th>
              <Th>Size</Th>
              <Th>Published</Th>
              <Th>Actions</Th>
            </Tr>
          </Thead>
          <Tbody>
            {filteredCatalog.slice(0, 100).map((img) => (
              <Tr key={`${img.ref}:${img.arch}:${img.variant}`}>
                <Td dataLabel="Image">
                  {img.os} {img.title}
                  {img.variant && img.variant !== "default" && (
                    <>
                      {" "}
                      <Label color="grey" isCompact>
                        {img.variant}
                      </Label>
                    </>
                  )}
                  {img.aliases[0] && (
                    <>
                      {" "}
                      <Label color="blue" isCompact>
                        {img.aliases[0]}
                      </Label>
                    </>
                  )}
                </Td>
                <Td dataLabel="Arch">{img.arch}</Td>
                <Td dataLabel="Type">
                  {[
                    img.sizes.container ? "CT" : null,
                    img.sizes["virtual-machine"] ? "VM" : null,
                  ]
                    .filter(Boolean)
                    .join(" + ")}
                </Td>
                <Td dataLabel="Size">
                  {Math.round(
                    (img.sizes.container ?? img.sizes["virtual-machine"] ?? 0) /
                      1048576
                  )}{" "}
                  MB
                </Td>
                <Td dataLabel="Published">{img.published}</Td>
                <Td dataLabel="Actions">
                  <Button
                    variant="plain"
                    aria-label={`Download ${img.ref}`}
                    onClick={() => startPull(`images:${img.ref}`)}
                  >
                    <DownloadIcon />
                  </Button>
                </Td>
              </Tr>
            ))}
            {filteredCatalog.length === 0 && (
              <Tr>
                <Td colSpan={6}>No images match “{catalogSearch}”.</Td>
              </Tr>
            )}
          </Tbody>
        </Table>
      )}
      {filteredCatalog.length > 100 && (
        <p className="pf-v6-u-text-color-subtle pf-v6-u-mt-sm">
          Showing first 100 of {filteredCatalog.length} matches — refine your
          search.
        </p>
      )}

      <Title headingLevel="h2" size="lg" className="pf-v6-u-mt-xl">
        Downloaded images
      </Title>

      <Table aria-label="Images table" variant="compact" className="pf-v6-u-mt-lg">
        <Thead>
          <Tr>
            <Th>Image</Th>
            <Th>Type</Th>
            <Th>Fingerprint</Th>
            <Th>Size</Th>
            <Th>Uploaded</Th>
            <Th>Actions</Th>
          </Tr>
        </Thead>
        <Tbody>
          {images.map((img) => (
            <Tr key={img.fingerprint}>
              <Td dataLabel="Image">
                {img.description || img.fingerprint.slice(0, 12)}
                {img.aliases.length > 0 && (
                  <>
                    {" "}
                    <Label color="blue" isCompact>
                      {img.aliases[0]}
                    </Label>
                  </>
                )}
              </Td>
              <Td dataLabel="Type">{img.instanceType === "virtual-machine" ? "VM" : img.instanceType === "container" ? "CT" : "—"}</Td>
              <Td dataLabel="Fingerprint">{img.fingerprint.slice(0, 12)}</Td>
              <Td dataLabel="Size">
                {(img.sizeBytes / 1024 / 1024).toFixed(0)} MB
              </Td>
              <Td dataLabel="Uploaded">
                {img.uploadedAt
                  ? new Date(img.uploadedAt).toLocaleDateString()
                  : "—"}
              </Td>
              <Td dataLabel="Actions">
                <Button
                  variant="plain"
                  aria-label={`Delete image ${img.fingerprint.slice(0, 12)}`}
                  isLoading={busy === img.fingerprint}
                  isDisabled={busy !== null}
                  onClick={() => remove(img)}
                >
                  <TimesIcon />
                </Button>
              </Td>
            </Tr>
          ))}
          {images.length === 0 && !error && (
            <Tr>
              <Td colSpan={6}>
                No pre-downloaded images. Pull one with the button above.
              </Td>
            </Tr>
          )}
        </Tbody>
      </Table>
    </PageSection>
  );
}
