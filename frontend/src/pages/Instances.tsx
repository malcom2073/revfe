import { useCallback, useEffect, useState } from "react";
import { Link } from "react-router-dom";
import {
  Button,
  Label,
  Modal,
  ModalBody,
  ModalFooter,
  ModalHeader,
  PageSection,
  Title,
  Toolbar,
  ToolbarContent,
  ToolbarItem,
  AlertGroup,
  Alert,
} from "@patternfly/react-core";
import {
  Table,
  Tbody,
  Td,
  Th,
  Thead,
  Tr,
} from "@patternfly/react-table";
import { PlayCircleIcon, StopCircleIcon, SyncAltIcon, TrashIcon } from "@patternfly/react-icons";
import { api, eventsUrl } from "../api/client";
import type { Instance } from "../api/types";
import CreateInstanceWizard from "../components/CreateInstanceModal";
import { formatBytes } from "../util/format";

function statusColor(status: string): "green" | "red" | "orange" | "grey" {
  switch (status) {
    case "Running":
      return "green";
    case "Stopped":
      return "grey";
    case "Starting":
    case "Stopping":
    case "Pending":
      return "orange";
    default:
      return "red";
  }
}

export default function Instances() {
  const [instances, setInstances] = useState<Instance[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [actionError, setActionError] = useState<string | null>(null);
  const [createOpen, setCreateOpen] = useState(false);
  const [busy, setBusy] = useState<string | null>(null);
  const [deleting, setDeleting] = useState<Instance | null>(null);

  const refresh = useCallback(() => {
    api
      .listInstances()
      .then(setInstances)
      .catch((e) => setError(e.message));
  }, []);

  useEffect(refresh, [refresh]);

  useEffect(() => {
    const source = new EventSource(eventsUrl());
    source.onmessage = (event) => {
      try {
        const parsed = JSON.parse(event.data);
        if (String(parsed.type ?? "").startsWith("instance")) refresh();
      } catch {
        // ignore malformed events
      }
    };
    source.onerror = () => {
      source.close();
      setTimeout(() => window.dispatchEvent(new Event("reconnect-sse")), 3000);
    };
    const reconnect = () => void 0;
    window.addEventListener("reconnect-sse", reconnect);
    return () => {
      source.close();
      window.removeEventListener("reconnect-sse", reconnect);
    };
  }, []);

  const runAction = async (name: string, action: string) => {
    setActionError(null);
    setBusy(`${name}:${action}`);
    try {
      await api.instanceAction(name, action);
      refresh();
    } catch (e) {
      setActionError((e as Error).message);
    } finally {
      setBusy(null);
    }
  };

  const removeInstance = async () => {
    if (!deleting) return;
    setActionError(null);
    setBusy(`${deleting.name}:delete`);
    try {
      await api.deleteInstance(deleting.name);
      setDeleting(null);
      refresh();
    } catch (e) {
      setActionError((e as Error).message);
      setDeleting(null);
    } finally {
      setBusy(null);
    }
  };

  return (
    <PageSection hasBodyWrapper={false}>
      <Title headingLevel="h1" size="2xl">
        Instances
      </Title>
      {error && (
        <Alert variant="danger" title="Could not load instances" className="pf-v6-u-mt-md">
          {error}
        </Alert>
      )}
      <Toolbar className="pf-v6-u-mt-md">
        <ToolbarContent>
          <ToolbarItem>
            <Button variant="primary" onClick={() => setCreateOpen(true)}>
              Create instance
            </Button>
          </ToolbarItem>
          <ToolbarItem>
            <Button variant="secondary" onClick={refresh}>
              Refresh
            </Button>
          </ToolbarItem>
        </ToolbarContent>
      </Toolbar>
      <AlertGroup>
        {actionError && (
          <Alert variant="danger" title={`Action failed: ${actionError}`} isInline />
        )}
      </AlertGroup>
      <Table aria-label="Instances table" variant="compact">
        <Thead>
          <Tr>
            <Th>Name</Th>
            <Th>Status</Th>
            <Th>Type</Th>
            <Th>Memory</Th>
            <Th>Actions</Th>
          </Tr>
        </Thead>
        <Tbody>
          {instances.map((inst) => (
            <Tr key={inst.name}>
              <Td dataLabel="Name">
                <Link to={`/instances/${encodeURIComponent(inst.name)}`}>
                  {inst.name}
                </Link>{" "}
                <Label color="blue" isCompact>
                  {inst.type === "virtual-machine" ? "VM" : "CT"}
                </Label>
              </Td>
              <Td dataLabel="Status">
                <Label color={statusColor(inst.status)} isCompact>
                  {inst.status}
                </Label>
              </Td>
              <Td dataLabel="Type">{inst.type}</Td>
              <Td dataLabel="Memory">{formatBytes(inst.state?.memoryUsed)}</Td>
              <Td dataLabel="Actions">
                <Button
                  variant="plain"
                  aria-label={`Start ${inst.name}`}
                  isLoading={busy === `${inst.name}:start`}
                  isDisabled={busy !== null || inst.status !== "Stopped"}
                  onClick={() => runAction(inst.name, "start")}
                >
                  <PlayCircleIcon />
                </Button>
                <Button
                  variant="plain"
                  aria-label={`Stop ${inst.name}`}
                  isLoading={busy === `${inst.name}:stop`}
                  isDisabled={busy !== null || inst.status === "Stopped"}
                  onClick={() => runAction(inst.name, "stop")}
                >
                  <StopCircleIcon />
                </Button>
                <Button
                  variant="plain"
                  aria-label={`Restart ${inst.name}`}
                  isLoading={busy === `${inst.name}:restart`}
                  isDisabled={busy !== null || inst.status !== "Running"}
                  onClick={() => runAction(inst.name, "restart")}
                >
                  <SyncAltIcon />
                </Button>
                <Button
                  variant="plain"
                  aria-label={`Delete ${inst.name}`}
                  isLoading={busy === `${inst.name}:delete`}
                  isDisabled={busy !== null}
                  onClick={() => setDeleting(inst)}
                >
                  <TrashIcon />
                </Button>
              </Td>
            </Tr>
          ))}
          {instances.length === 0 && !error && (
            <Tr>
              <Td colSpan={5}>No instances found.</Td>
            </Tr>
          )}
        </Tbody>
      </Table>
      {createOpen && (
        <CreateInstanceWizard
          onCreated={() => {
            setCreateOpen(false);
            refresh();
          }}
          onClose={() => setCreateOpen(false)}
        />
      )}

      <Modal isOpen={deleting !== null} onClose={() => setDeleting(null)}>
        <ModalHeader title={`Delete instance "${deleting?.name}"?`} />
        <ModalBody>
          This permanently removes the instance and its data. Consider taking
          a snapshot or backup first.
        </ModalBody>
        <ModalFooter>
          <Button
            variant="danger"
            onClick={removeInstance}
            isLoading={busy === `${deleting?.name}:delete`}
          >
            Delete
          </Button>{" "}
          <Button variant="link" onClick={() => setDeleting(null)}>
            Cancel
          </Button>
        </ModalFooter>
      </Modal>
    </PageSection>
  );
}
