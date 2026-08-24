import { useCallback, useEffect, useState } from "react";
import { Link, useParams } from "react-router-dom";
import {
  Alert,
  Breadcrumb,
  BreadcrumbItem,
  Button,
  Card,
  CardBody,
  DescriptionList,
  DescriptionListDescription,
  DescriptionListGroup,
  DescriptionListTerm,
  Label,
  PageSection,
  Tab,
  TabTitleText,
  Tabs,
  Title,
} from "@patternfly/react-core";
import { api } from "../api/client";
import type { Instance } from "../api/types";
import Terminal from "../components/Terminal";
import { formatBytes, formatDuration } from "../util/format";

export default function InstanceDetail() {
  const { name = "" } = useParams();
  const [instance, setInstance] = useState<Instance | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [activeTab, setActiveTab] = useState<string | number>("overview");
  const [shell, setShell] = useState("bash");

  const refresh = useCallback(() => {
    api
      .getInstance(name)
      .then(setInstance)
      .catch((e) => setError(e.message));
  }, [name]);

  useEffect(refresh, [refresh]);

  if (error) {
    return (
      <PageSection hasBodyWrapper={false}>
        <Alert variant="danger" title={`Could not load ${name}`}>
          {error}
        </Alert>
      </PageSection>
    );
  }

  return (
    <>
      <PageSection hasBodyWrapper={false}>
        <Breadcrumb>
          <BreadcrumbItem>
            <Link to="/instances">Instances</Link>
          </BreadcrumbItem>
          <BreadcrumbItem isActive>{name}</BreadcrumbItem>
        </Breadcrumb>
        <div className="pf-v6-l-flex pf-m-space-items-md pf-v6-u-mt-sm">
          <Title headingLevel="h1" size="2xl">
            {name}
          </Title>
          {instance && (
            <Label color={instance.status === "Running" ? "green" : "grey"}>
              {instance.status}
            </Label>
          )}
          {instance?.status === "Stopped" && (
            <Button
              variant="primary"
              onClick={async () => {
                await api.instanceAction(name, "start");
                refresh();
              }}
            >
              Start
            </Button>
          )}
          {instance?.status === "Running" && (
            <Button
              variant="secondary"
              onClick={async () => {
                await api.instanceAction(name, "stop");
                refresh();
              }}
            >
              Stop
            </Button>
          )}
        </div>
        <Tabs
          activeKey={activeTab}
          onSelect={(_e, key) => setActiveTab(key)}
          className="pf-v6-u-mt-lg"
        >
          <Tab eventKey="overview" title={<TabTitleText>Overview</TabTitleText>}>
            {instance && (
              <Card className="pf-v6-u-mt-lg">
                <CardBody>
                  <DescriptionList isHorizontal columnModifier={{ default: "1Col" }}>
                    <DescriptionListGroup>
                      <DescriptionListTerm>Type</DescriptionListTerm>
                      <DescriptionListDescription>
                        {instance.type}
                      </DescriptionListDescription>
                    </DescriptionListGroup>
                    <DescriptionListGroup>
                      <DescriptionListTerm>Profiles</DescriptionListTerm>
                      <DescriptionListDescription>
                        {(instance.profiles ?? []).join(", ") || "—"}
                      </DescriptionListDescription>
                    </DescriptionListGroup>
                    <DescriptionListGroup>
                      <DescriptionListTerm>PID</DescriptionListTerm>
                      <DescriptionListDescription>
                        {instance.state?.pid ?? "—"}
                      </DescriptionListDescription>
                    </DescriptionListGroup>
                    <DescriptionListGroup>
                      <DescriptionListTerm>Processes</DescriptionListTerm>
                      <DescriptionListDescription>
                        {instance.state?.processes ?? "—"}
                      </DescriptionListDescription>
                    </DescriptionListGroup>
                    <DescriptionListGroup>
                      <DescriptionListTerm>Memory used</DescriptionListTerm>
                      <DescriptionListDescription>
                        {formatBytes(instance.state?.memoryUsed)}{" "}
                        (peak {formatBytes(instance.state?.memoryUsagePeak)})
                      </DescriptionListDescription>
                    </DescriptionListGroup>
                    <DescriptionListGroup>
                      <DescriptionListTerm>CPU time</DescriptionListTerm>
                      <DescriptionListDescription>
                        {formatDuration(instance.state?.cpuSeconds)}
                      </DescriptionListDescription>
                    </DescriptionListGroup>
                    <DescriptionListGroup>
                      <DescriptionListTerm>Disk</DescriptionListTerm>
                      <DescriptionListDescription>
                        {formatBytes(instance.state?.diskUsed)}
                        {instance.state?.diskTotal
                          ? ` / ${formatBytes(instance.state.diskTotal)}`
                          : ""}
                      </DescriptionListDescription>
                    </DescriptionListGroup>
                    <DescriptionListGroup>
                      <DescriptionListTerm>Created</DescriptionListTerm>
                      <DescriptionListDescription>
                        {instance.createdAt || "—"}
                      </DescriptionListDescription>
                    </DescriptionListGroup>
                  </DescriptionList>
                </CardBody>
              </Card>
            )}
          </Tab>
          <Tab eventKey="console" title={<TabTitleText>Console</TabTitleText>}>
            {instance?.status === "Running" ? (
              <>
                <div className="pf-v6-u-mt-lg">
                  <Button
                    variant="control"
                    onClick={() => setShell(shell === "bash" ? "sh" : "bash")}
                  >
                    Shell: {shell} (click to switch)
                  </Button>
                </div>
                <Terminal instanceName={name} shell={shell} />
              </>
            ) : (
              <Alert
                variant="info"
                isInline
                title="Console requires a running instance"
                className="pf-v6-u-mt-lg"
              />
            )}
          </Tab>
        </Tabs>
      </PageSection>
    </>
  );
}
