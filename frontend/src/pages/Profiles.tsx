import { useCallback, useEffect, useState } from "react";
import {
  Alert,
  Card,
  CardBody,
  CardTitle,
  Gallery,
  Label,
  PageSection,
  Title,
} from "@patternfly/react-core";
import { Table, Tbody, Td, Th, Thead, Tr } from "@patternfly/react-table";
import { api } from "../api/client";
import type { ProfileDevice, ProfileInfo } from "../api/types";

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

  const load = useCallback(() => {
    api
      .listProfiles()
      .then(setProfiles)
      .catch((e) => setError(e.message));
  }, []);

  useEffect(load, [load]);

  return (
    <PageSection hasBodyWrapper={false}>
      <Title headingLevel="h1" size="2xl">
        Profiles
      </Title>
      <Title headingLevel="h2" size="md" className="pf-v6-u-text-color-subtle">
        Reusable configuration templates applied to instances at creation time
      </Title>
      {error && (
        <Alert variant="danger" title="Could not load profiles" isInline className="pf-v6-u-mt-md">
          {error}
        </Alert>
      )}
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
              <p className="pf-v6-u-text-color-subtle">
                Used by{" "}
                {(profile.usedBy ?? []).map((u) => u.name).join(", ") || "nothing"}
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
    </PageSection>
  );
}
