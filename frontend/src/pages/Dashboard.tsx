import { useEffect, useState } from "react";
import {
  Card,
  CardBody,
  CardTitle,
  DescriptionList,
  DescriptionListDescription,
  DescriptionListGroup,
  DescriptionListTerm,
  Gallery,
  PageSection,
  Title,
} from "@patternfly/react-core";
import { api } from "../api/client";
import type { ServerInfo } from "../api/types";
import { formatBytes } from "../util/format";

export default function Dashboard() {
  const [info, setInfo] = useState<ServerInfo | null>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    api
      .serverInfo()
      .then(setInfo)
      .catch((e) => setError(e.message));
  }, []);

  return (
    <PageSection hasBodyWrapper={false}>
      <Title headingLevel="h1" size="2xl">
        Dashboard
      </Title>
      <Gallery hasGutter maxWidths={{ default: "400px" }} className="pf-v6-u-mt-lg">
        <Card>
          <CardTitle>Server</CardTitle>
          <CardBody>
            {error ? (
              error
            ) : info ? (
              <DescriptionList isCompact isHorizontal>
                <DescriptionListGroup>
                  <DescriptionListTerm>Name</DescriptionListTerm>
                  <DescriptionListDescription>{info.name}</DescriptionListDescription>
                </DescriptionListGroup>
                <DescriptionListGroup>
                  <DescriptionListTerm>Version</DescriptionListTerm>
                  <DescriptionListDescription>
                    Incus {info.version}
                  </DescriptionListDescription>
                </DescriptionListGroup>
                <DescriptionListGroup>
                  <DescriptionListTerm>Kernel</DescriptionListTerm>
                  <DescriptionListDescription>
                    {info.kernel ?? "—"}
                  </DescriptionListDescription>
                </DescriptionListGroup>
              </DescriptionList>
            ) : (
              "Loading…"
            )}
          </CardBody>
        </Card>
        <Card>
          <CardTitle>Resources</CardTitle>
          <CardBody>
            {info && (
              <DescriptionList isCompact isHorizontal>
                <DescriptionListGroup>
                  <DescriptionListTerm>CPUs</DescriptionListTerm>
                  <DescriptionListDescription>
                    {info.cpu ?? "—"}
                  </DescriptionListDescription>
                </DescriptionListGroup>
                <DescriptionListGroup>
                  <DescriptionListTerm>Total memory</DescriptionListTerm>
                  <DescriptionListDescription>
                    {formatBytes(info.memoryTotal)}
                  </DescriptionListDescription>
                </DescriptionListGroup>
                <DescriptionListGroup>
                  <DescriptionListTerm>Architectures</DescriptionListTerm>
                  <DescriptionListDescription>
                    {(info.architectures ?? []).join(", ") || "—"}
                  </DescriptionListDescription>
                </DescriptionListGroup>
              </DescriptionList>
            )}
          </CardBody>
        </Card>
      </Gallery>
    </PageSection>
  );
}
