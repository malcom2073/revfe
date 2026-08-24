import { useEffect, useState } from "react";
import {
  Alert,
  Label,
  PageSection,
  Title,
} from "@patternfly/react-core";
import { Table, Tbody, Td, Th, Thead, Tr } from "@patternfly/react-table";
import { api } from "../api/client";
import type { NetworkInfo, NetworkUsedBy } from "../api/types";

function usedByLabels(net: NetworkInfo) {
  return (net.usedBy ?? []).map((u: NetworkUsedBy, idx) => (
    <Label
      key={`${u.kind}:${u.name}:${idx}`}
      color={u.kind === "instance" ? "teal" : u.kind === "profile" ? "orange" : "grey"}
      isCompact
      className="pf-v6-u-mr-xs"
    >
      {u.kind === "instance" || u.kind === "profile" ? u.name : `${u.kind}: ${u.name}`}
    </Label>
  ));
}

export default function Networks() {
  const [networks, setNetworks] = useState<NetworkInfo[]>([]);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    api
      .listNetworks()
      .then(setNetworks)
      .catch((e) => setError(e.message));
  }, []);

  return (
    <PageSection hasBodyWrapper={false}>
      <Title headingLevel="h1" size="2xl">
        Networks
      </Title>
      {error && (
        <Alert variant="danger" title="Could not load networks" className="pf-v6-u-mt-md">
          {error}
        </Alert>
      )}
      <Table aria-label="Networks table" variant="compact" className="pf-v6-u-mt-md">
        <Thead>
          <Tr>
            <Th>Name</Th>
            <Th>Type</Th>
            <Th>Status</Th>
            <Th>IPv4</Th>
            <Th>IPv6</Th>
            <Th>Used by</Th>
          </Tr>
        </Thead>
        <Tbody>
          {networks.map((net) => (
            <Tr key={net.name}>
              <Td dataLabel="Name">
                {net.name}
                {net.managed && (
                  <>
                    {" "}
                    <Label color="green" isCompact>
                      managed
                    </Label>
                  </>
                )}
              </Td>
              <Td dataLabel="Type">{net.type}</Td>
              <Td dataLabel="Status">{net.status || "—"}</Td>
              <Td dataLabel="IPv4">
                {net.ipv4 ? (
                  <>
                    {net.ipv4}
                    {net.ipv4Nat && (
                      <>
                        {" "}
                        <Label color="blue" isCompact>
                          NAT
                        </Label>
                      </>
                    )}
                  </>
                ) : (
                  "—"
                )}
              </Td>
              <Td dataLabel="IPv6">
                {net.ipv6 ? (
                  <>
                    {net.ipv6}
                    {net.ipv6Nat && (
                      <>
                        {" "}
                        <Label color="blue" isCompact>
                          NAT
                        </Label>
                      </>
                    )}
                  </>
                ) : (
                  "—"
                )}
              </Td>
              <Td dataLabel="Used by">{usedByLabels(net)}</Td>
            </Tr>
          ))}
          {networks.length === 0 && !error && (
            <Tr>
              <Td colSpan={6}>No networks found.</Td>
            </Tr>
          )}
        </Tbody>
      </Table>
    </PageSection>
  );
}
