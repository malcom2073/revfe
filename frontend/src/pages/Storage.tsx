import { useEffect, useState } from "react";
import {
  Alert,
  Card,
  CardBody,
  CardTitle,
  Gallery,
  Label,
  PageSection,
  Progress,
  Title,
} from "@patternfly/react-core";
import { Table, Tbody, Td, Th, Thead, Tr } from "@patternfly/react-table";
import { api } from "../api/client";
import type { StoragePool, StorageVolume } from "../api/types";
import { formatBytes } from "../util/format";

function volumeLabel(v: StorageVolume): string {
  if (v.type === "image") return v.imageDescription ?? `${v.name.slice(0, 12)} (image)`;
  return v.name;
}

function typeBadge(v: StorageVolume) {
  switch (v.type) {
    case "container":
      return <Label color="teal" isCompact>CT</Label>;
    case "virtual-machine":
      return <Label color="purple" isCompact>VM</Label>;
    case "image":
      return <Label color="blue" isCompact>Image</Label>;
    default:
      return <Label color="grey" isCompact>{v.type}</Label>;
  }
}

export default function Storage() {
  const [pools, setPools] = useState<StoragePool[]>([]);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    api
      .storage()
      .then(setPools)
      .catch((e) => setError(e.message));
  }, []);

  return (
    <PageSection hasBodyWrapper={false}>
      <Title headingLevel="h1" size="2xl">
        Storage
      </Title>
      {error && (
        <Alert variant="danger" title="Could not load storage" className="pf-v6-u-mt-md">
          {error}
        </Alert>
      )}
      <Gallery hasGutter className="pf-v6-u-mt-lg">
        {pools.map((pool) => (
          <Card key={pool.name} isFullHeight>
            <CardTitle>
              {pool.name}
              <Label color="green" isCompact className="pf-v6-u-ml-sm">
                {pool.driver}
              </Label>
              {pool.description && (
                <span className="pf-v6-u-text-color-subtle pf-v6-u-ml-sm">
                  {pool.description}
                </span>
              )}
            </CardTitle>
            <CardBody>
              <p className="pf-v6-u-text-color-subtle">
                Status: {pool.status} · {pool.volumes.length} volumes · used by{" "}
                {pool.usedByCount} resources
              </p>
              {pool.usage && (
                <div className="pf-v6-u-mt-md" style={{ maxWidth: "420px" }}>
                  <Progress
                    title="Space"
                    value={Math.min(
                      100,
                      Math.round((pool.usage.used / pool.usage.total) * 100)
                    )}
                    label={`${formatBytes(pool.usage.used)} of ${formatBytes(
                      pool.usage.total
                    )}`}
                    aria-label={`Space usage for ${pool.name}`}
                  />
                </div>
              )}
              <Table aria-label={`Volumes on ${pool.name}`} variant="compact">
                <Thead>
                  <Tr>
                    <Th>Name</Th>
                    <Th>Type</Th>
                    <Th>Content</Th>
                  </Tr>
                </Thead>
                <Tbody>
                  {pool.volumes.map((v) => (
                    <Tr key={`${v.type}:${v.name}`}>
                      <Td dataLabel="Name">{volumeLabel(v)}</Td>
                      <Td dataLabel="Type">{typeBadge(v)}</Td>
                      <Td dataLabel="Content">{v.contentType}</Td>
                    </Tr>
                  ))}
                  {pool.volumes.length === 0 && (
                    <Tr>
                      <Td colSpan={3}>No volumes.</Td>
                    </Tr>
                  )}
                </Tbody>
              </Table>
            </CardBody>
          </Card>
        ))}
        {pools.length === 0 && !error && (
          <Card>
            <CardBody>No storage pools found.</CardBody>
          </Card>
        )}
      </Gallery>
    </PageSection>
  );
}
