import { useCallback, useEffect, useState } from "react";
import {
  Alert,
  Button,
  Label,
  Modal,
  ModalBody,
  ModalFooter,
  ModalHeader,
  TextInput,
  Toolbar,
} from "@patternfly/react-core";
import {
  Table,
  Tbody,
  Td,
  Th,
  Thead,
  Tr,
} from "@patternfly/react-table";
import { CameraIcon, HistoryIcon as RestoreIcon, TimesIcon } from "@patternfly/react-icons";
import { api } from "../api/client";
import type { Snapshot } from "../api/types";

export default function SnapshotsTab({
  instanceName,
}: {
  instanceName: string;
}) {
  const [snapshots, setSnapshots] = useState<Snapshot[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [notice, setNotice] = useState<string | null>(null);
  const [busy, setBusy] = useState<string | null>(null);
  const [creating, setCreating] = useState(false);
  const [snapName, setSnapName] = useState("");
  // Snapshot pending destructive confirmation: restore or delete
  const [pending, setPending] = useState<{
    action: "restore" | "delete";
    snapshot: Snapshot;
  } | null>(null);

  const refresh = useCallback(() => {
    api
      .listSnapshots(instanceName)
      .then(setSnapshots)
      .catch((e) => setError(e.message));
  }, [instanceName]);

  useEffect(refresh, [refresh]);

  const take = async () => {
    setError(null);
    setBusy("create");
    try {
      const res = await api.createSnapshot(
        instanceName,
        snapName.trim() || undefined
      );
      setNotice(`Snapshot "${res.name}" created.`);
      setSnapName("");
      setCreating(false);
      refresh();
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(null);
    }
  };

  const runPending = async () => {
    if (!pending) return;
    const { action, snapshot } = pending;
    setPending(null);
    setError(null);
    setBusy(`${action}:${snapshot.name}`);
    try {
      if (action === "restore") {
        await api.restoreSnapshot(instanceName, snapshot.name);
        setNotice(
          `Restored "${snapshot.name}". The instance was reverted to that state.`
        );
      } else {
        await api.deleteSnapshot(instanceName, snapshot.name);
        setNotice(`Snapshot "${snapshot.name}" deleted.`);
      }
      refresh();
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setBusy(null);
    }
  };

  return (
    <div>
      <Toolbar className="pf-v6-u-mt-lg">
        <Button variant="primary" icon={<CameraIcon />} onClick={() => setCreating(true)}>
          Take snapshot
        </Button>
        <Button variant="secondary" onClick={refresh}>
          Refresh
        </Button>
      </Toolbar>

      {error && (
        <Alert variant="danger" title="Error" isInline className="pf-v6-u-mt-md">
          {error}
        </Alert>
      )}
      {notice && (
        <Alert
          variant="success"
          title={notice}
          isInline
          className="pf-v6-u-mt-md"
        >
          <Button variant="link" onClick={() => setNotice(null)}>
            Dismiss
          </Button>
        </Alert>
      )}

      <Table aria-label="Snapshots table" variant="compact" className="pf-v6-u-mt-md">
        <Thead>
          <Tr>
            <Th>Name</Th>
            <Th>Created</Th>
            <Th>Type</Th>
            <Th>Actions</Th>
          </Tr>
        </Thead>
        <Tbody>
          {snapshots.map((snap) => (
            <Tr key={snap.name}>
              <Td dataLabel="Name">{snap.name}</Td>
              <Td dataLabel="Created">
                {snap.createdAt
                  ? new Date(snap.createdAt).toLocaleString()
                  : "—"}
              </Td>
              <Td dataLabel="Type">
                {snap.stateful ? (
                  <Label color="purple" isCompact>
                    stateful
                  </Label>
                ) : (
                  <Label color="grey" isCompact>
                    stateless
                  </Label>
                )}
              </Td>
              <Td dataLabel="Actions">
                <Button
                  variant="plain"
                  aria-label={`Restore ${snap.name}`}
                  isDisabled={busy !== null}
                  onClick={() => setPending({ action: "restore", snapshot: snap })}
                >
                  <RestoreIcon />
                </Button>
                <Button
                  variant="plain"
                  aria-label={`Delete ${snap.name}`}
                  isDisabled={busy !== null}
                  onClick={() => setPending({ action: "delete", snapshot: snap })}
                >
                  <TimesIcon />
                </Button>
              </Td>
            </Tr>
          ))}
          {snapshots.length === 0 && !error && (
            <Tr>
              <Td colSpan={4}>No snapshots yet.</Td>
            </Tr>
          )}
        </Tbody>
      </Table>

      <Modal isOpen={creating} onClose={() => setCreating(false)}>
        <ModalHeader title={`Take a snapshot of ${instanceName}`} />
        <ModalBody>
        <TextInput
          aria-label="Snapshot name"
          placeholder="Leave empty to auto-name (snap<timestamp>)"
          value={snapName}
          onChange={(_e, v) => setSnapName(v)}
          onKeyDown={(e) => e.key === "Enter" && take()}
        />
        <div className="pf-v6-u-mt-lg">
          <Button
            variant="primary"
            onClick={take}
            isLoading={busy === "create"}
            isDisabled={busy === "create"}
          >
            Create snapshot
          </Button>{" "}
          <Button variant="link" onClick={() => setCreating(false)}>
            Cancel
          </Button>
        </div>
        </ModalBody>
      </Modal>

      <Modal isOpen={pending !== null} onClose={() => setPending(null)}>
        <ModalHeader
          title={
            pending?.action === "restore"
              ? `Restore "${pending.snapshot.name}"?`
              : `Delete "${pending?.snapshot.name}"?`
          }
        />
        <ModalBody>
        {pending?.action === "restore" ? (
          <>
            The instance's disk state will be reverted to this snapshot. Any
            changes made since — including files created after it — will be
            lost.
          </>
        ) : (
          <>This snapshot will be permanently removed.</>
        )}
        </ModalBody>
        <ModalFooter>
          <Button
            variant={pending?.action === "restore" ? "warning" : "danger"}
            onClick={runPending}
          >
            {pending?.action === "restore" ? "Restore" : "Delete"}
          </Button>{" "}
          <Button variant="link" onClick={() => setPending(null)}>
            Cancel
          </Button>
        </ModalFooter>
      </Modal>
    </div>
  );
}
