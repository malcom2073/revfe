import { useCallback, useEffect, useState } from "react";
import { Link, useParams } from "react-router-dom";
import {
  Alert,
  Breadcrumb,
  BreadcrumbItem,
  Button,
  Card,
  CardBody,
  CardTitle,
  DescriptionList,
  DescriptionListDescription,
  DescriptionListGroup,
  DescriptionListTerm,
  Gallery,
  Label,
  PageSection,
  Progress,
  Tab,
  TabTitleText,
  Tabs,
  Title,
} from "@patternfly/react-core";
import { CheckIcon, CopyIcon } from "@patternfly/react-icons";
import { api } from "../api/client";
import type { Instance } from "../api/types";
import Terminal from "../components/Terminal";
import SnapshotsTab from "./SnapshotsTab";
import { formatBytes, formatDuration } from "../util/format";

function CopyButton({ text }: { text: string }) {
  const [copied, setCopied] = useState(false);
  return (
    <Button
      variant="plain"
      aria-label={`Copy ${text}`}
      onClick={async () => {
        try {
          await navigator.clipboard.writeText(text);
          setCopied(true);
          setTimeout(() => setCopied(false), 1500);
        } catch {
          // clipboard unavailable (non-secure context)
        }
      }}
    >
      {copied ? <CheckIcon color="#3e8635" /> : <CopyIcon />}
    </Button>
  );
}

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
                      <DescriptionListTerm>Created</DescriptionListTerm>
                      <DescriptionListDescription>
                        {instance.createdAt || "—"}
                      </DescriptionListDescription>
                    </DescriptionListGroup>
                    <DescriptionListGroup>
                      <DescriptionListTerm>Disks</DescriptionListTerm>
                      <DescriptionListDescription>
                        {instance.state?.disks?.length
                          ? `${instance.state.disks.length} volume(s) — see Disks tab`
                          : "—"}
                      </DescriptionListDescription>
                    </DescriptionListGroup>
                  </DescriptionList>
                </CardBody>
              </Card>
            )}
          </Tab>

          <Tab eventKey="network" title={<TabTitleText>Network</TabTitleText>}>
            {instance?.state?.interfaces?.length ? (
              <Gallery hasGutter className="pf-v6-u-mt-lg" maxWidths={{ default: "460px" }}>
                {instance.state.interfaces.map((iface) => (
                  <Card key={iface.name}>
                    <CardTitle className="pf-v6-u-font-size-md">
                      {iface.name}
                    </CardTitle>
                    <CardBody>
                      {iface.addresses.length === 0 && (
                        <span className="pf-v6-u-text-color-subtle">No addresses</span>
                      )}
                      {iface.addresses.map((addr, idx) => {
                        const cidr = addr.netmask
                          ? `${addr.address}/${addr.netmask}`
                          : addr.address;
                        return (
                          <div
                            key={cidr}
                            style={{
                              display: "flex",
                              alignItems: "center",
                              gap: 10,
                              padding: "8px 0",
                              borderTop:
                                idx > 0 ? "1px solid rgba(255,255,255,0.08)" : undefined,
                            }}
                          >
                            <Label
                              color={addr.family === "inet" ? "teal" : "purple"}
                              isCompact
                            >
                              {addr.family}
                            </Label>
                            <span
                              style={{
                                fontFamily: "'Red Hat Mono', Consolas, monospace",
                                fontSize: "0.85rem",
                                overflowWrap: "anywhere",
                              }}
                            >
                              {cidr}
                            </span>
                            {addr.scope && (
                              <span
                                style={{
                                  marginLeft: "auto",
                                  opacity: 0.6,
                                  fontSize: "0.8rem",
                                  textTransform: "capitalize",
                                }}
                              >
                                {addr.scope}
                              </span>
                            )}
                            <CopyButton text={addr.address} />
                          </div>
                        );
                      })}
                    </CardBody>
                  </Card>
                ))}
              </Gallery>
            ) : (
              <Alert
                variant="info"
                isInline
                title="No interface information (instance stopped?)"
                className="pf-v6-u-mt-lg"
              />
            )}
          </Tab>

          <Tab
            eventKey="snapshots"
            title={<TabTitleText>Snapshots</TabTitleText>}
          >
            <SnapshotsTab instanceName={name} />
          </Tab>
          <Tab eventKey="disks" title={<TabTitleText>Disks</TabTitleText>}>
            {instance?.state?.disks?.length ? (
              <Gallery hasGutter className="pf-v6-u-mt-lg" maxWidths={{ default: "420px" }}>
                {instance.state.disks.map((disk) => {
                  const pct =
                    disk.usage !== null &&
                    disk.usage !== undefined &&
                    disk.total
                      ? Math.min(100, Math.round((disk.usage / disk.total) * 100))
                      : null;
                  return (
                    <Card key={disk.name}>
                      <CardTitle className="pf-v6-u-font-size-md">
                        {disk.name}
                      </CardTitle>
                      <CardBody>
                        {pct !== null && disk.total ? (
                          <>
                            <Progress
                              title={`Disk ${disk.name}`}
                              value={pct}
                              label={`${formatBytes(disk.usage)} of ${formatBytes(
                                disk.total
                              )}`}
                              aria-label={`Usage of ${disk.name}`}
                            />
                          </>
                        ) : (
                          formatBytes(disk.usage)
                        )}
                      </CardBody>
                    </Card>
                  );
                })}
              </Gallery>
            ) : (
              <span className="pf-v6-u-text-color-subtle">No disk usage data.</span>
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
