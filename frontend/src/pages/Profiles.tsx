import { useCallback, useEffect, useState } from "react";
import {
  Alert,
  Button,
  Card,
  CardBody,
  CardTitle,
  Gallery,
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
} from "@patternfly/react-core";
import { PencilAltIcon, PlusCircleIcon, TimesIcon } from "@patternfly/react-icons";
import { Table, Tbody, Td, Th, Thead, Tr } from "@patternfly/react-table";
import { api } from "../api/client";
import type { ProfileDevice, ProfileInfo } from "../api/types";
import ProfileEditModal from "./ProfileEditModal";

const DEVICE_TYPE_COLORS: Record<string, "blue" | "green" | "orange" | "purple" | "grey"> = {
  disk: "orange",
  nic: "blue",
  gpu: "purple",
  usb: "green",
  unix: "grey",
};

function deviceTypeBadge(device: ProfileDevice) {
  const type = String(device.type ?? "unknown");
  return (
    <Label
      color={DEVICE_TYPE_COLORS[type.replace(/-.*/, "")] ?? "grey"}
      isCompact
    >
      {type}
    </Label>
  );
}

export default function Profiles() {
  const [profiles, setProfiles] = useState<ProfileInfo[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [notice, setNotice] = useState<string | null>(null);
  const [editing, setEditing] = useState<ProfileInfo | null>(null);
  const [creating, setCreating] = useState(false);
  const [deleting, setDeleting] = useState<ProfileInfo | null>(null);

  const load = useCallback(() => {
    api
      .listProfiles()
      .then(setProfiles)
      .catch((e) => setError(e.message));
  }, []);

  useEffect(load, [load]);

  const removeProfile = async () => {
    if (!deleting) return;
    setError(null);
    try {
      await api.deleteProfile(deleting.name);
      setNotice(`Profile "${deleting.name}" deleted.`);
      setDeleting(null);
      load();
    } catch (e) {
      setError((e as Error).message);
      setDeleting(null);
    }
  };

  return (
    <PageSection hasBodyWrapper={false}>
      <Title headingLevel="h1" size="2xl">
        Profiles
      </Title>
      <Title headingLevel="h2" size="md" className="pf-v6-u-text-color-subtle">
        Reusable configuration templates applied to instances at creation time
      </Title>
      {error && (
        <Alert variant="danger" title={error} isInline className="pf-v6-u-mt-md" />
      )}
      {notice && (
        <Alert variant="success" title={notice} isInline className="pf-v6-u-mt-md" />
      )}
      <Toolbar className="pf-v6-u-mt-md">
        <ToolbarContent>
          <ToolbarItem>
            <Button
              variant="primary"
              icon={<PlusCircleIcon />}
              onClick={() => setCreating(true)}
            >
              Create profile
            </Button>
          </ToolbarItem>
        </ToolbarContent>
      </Toolbar>
      <Gallery hasGutter className="pf-v6-u-mt-lg">
        {profiles.map((profile) => (
          <Card key={profile.name} isFullHeight>
            <CardTitle>
              {profile.name}
              {profile.name === "default" && (
                <>
                  {" "}
                  <Label color="green" isCompact>
                    default
                  </Label>
                </>
              )}
              {profile.description && (
                <span className="pf-v6-u-text-color-subtle pf-v6-u-ml-sm">
                  {profile.description}
                </span>
              )}
            </CardTitle>
            <CardBody>
              <p
                className="pf-v6-u-text-color-subtle"
                style={{ display: "flex", alignItems: "center" }}
              >
                <span style={{ marginRight: "auto" }}>
                  Used by{" "}
                  {(profile.usedBy ?? []).map((u) => u.name).join(", ") ||
                    "nothing"}
                </span>
                <Button
                  variant="plain"
                  aria-label={`Edit ${profile.name}`}
                  onClick={() => setEditing(profile)}
                >
                  <PencilAltIcon />
                </Button>
                <Button
                  variant="plain"
                  aria-label={`Delete ${profile.name}`}
                  onClick={() => setDeleting(profile)}
                >
                  <TimesIcon />
                </Button>
              </p>

              <Title headingLevel="h4" className="pf-v6-u-mt-md">
                Configuration
              </Title>
              {Object.keys(profile.config).length > 0 ? (
                <Table aria-label={`Config of ${profile.name}`} variant="compact">
                  <Thead>
                    <Tr>
                      <Th>Key</Th>
                      <Th>Value</Th>
                    </Tr>
                  </Thead>
                  <Tbody>
                    {Object.entries(profile.config).map(([key, value]) => (
                      <Tr key={key}>
                        <Td dataLabel="Key">{key}</Td>
                        <Td dataLabel="Value">{String(value)}</Td>
                      </Tr>
                    ))}
                  </Tbody>
                </Table>
              ) : (
                <p className="pf-v6-u-text-color-subtle">
                  No config keys — all limits unset (host defaults).
                </p>
              )}

              <Title headingLevel="h4" className="pf-v6-u-mt-lg">
                Devices
              </Title>
              <Table aria-label={`Devices of ${profile.name}`} variant="compact">
                <Thead>
                  <Tr>
                    <Th>Name</Th>
                    <Th>Type</Th>
                    <Th>Details</Th>
                  </Tr>
                </Thead>
                <Tbody>
                  {Object.entries(profile.devices).map(([devName, device]) => (
                    <Tr key={devName}>
                      <Td dataLabel="Name">{devName}</Td>
                      <Td dataLabel="Type">{deviceTypeBadge(device)}</Td>
                      <Td dataLabel="Details">
                        {Object.entries(device)
                          .filter(([k]) => k !== "type")
                          .map(([k, v]) => `${k}=${v}`)
                          .join(" · ") || "—"}
                      </Td>
                    </Tr>
                  ))}
                  {Object.keys(profile.devices).length === 0 && (
                    <Tr>
                      <Td colSpan={3}>No devices.</Td>
                    </Tr>
                  )}
                </Tbody>
              </Table>
            </CardBody>
          </Card>
        ))}
        {profiles.length === 0 && !error && (
          <Card>
            <CardBody>No profiles found.</CardBody>
          </Card>
        )}
      </Gallery>

      {(creating || editing) && (
        <ProfileEditModal
          existing={editing}
          onClose={() => {
            setCreating(false);
            setEditing(null);
          }}
          onSaved={(savedName) => {
            setNotice(`Profile "${savedName}" saved.`);
            setCreating(false);
            setEditing(null);
            load();
          }}
        />
      )}

      <Modal isOpen={deleting !== null} onClose={() => setDeleting(null)}>
        <ModalHeader title={`Delete profile "${deleting?.name}"?`} />
        <ModalBody>
          This profile will be permanently removed. Incus will refuse if
          instances or other profiles still use it.
        </ModalBody>
        <ModalFooter>
          <Button variant="danger" onClick={removeProfile}>
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
