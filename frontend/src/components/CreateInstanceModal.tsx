import { useEffect, useMemo, useState } from "react";
import {
  Alert,
  Button,
  Checkbox,
  Form,
  FormFieldGroup,
  FormFieldGroupHeader,
  FormGroup,
  FormSelect,
  HelperText,
  HelperTextItem,
  Modal,
  ModalBody,
  ModalFooter,
  ModalHeader,
  Radio,
  FormSelectOption,
  Tab,
  Tabs,
  TabTitleText,
  TextInput,
  Title,
} from "@patternfly/react-core";
import { TimesIcon, PlusCircleIcon } from "@patternfly/react-icons";
import { api } from "../api/client";
import type {
  CreateInstanceSpec,
  ImageInfo,
  NetworkInfo,
  ProfileInfo,
  StoragePool,
} from "../api/types";
import { REMOTE_SUGGESTIONS } from "../util/remotes";

interface ConfigRow {
  key: string;
  value: string;
}

export default function CreateInstanceWizard({
  onCreated,
  onClose,
}: {
  onCreated: () => void;
  onClose: () => void;
}) {
  const [activeTab, setActiveTab] = useState<string | number>("general");
  const [name, setName] = useState("");
  const [type, setType] = useState<"container" | "virtual-machine">(
    "container"
  );
  const [image, setImage] = useState("ubuntu:24.04");
  const [images, setImages] = useState<ImageInfo[]>([]);
  const [cpu, setCpu] = useState("");
  const [memoryValue, setMemoryValue] = useState("");
  const [memoryUnit, setMemoryUnit] = useState("MiB");
  const [diskGb, setDiskGb] = useState("");
  const [pools, setPools] = useState<StoragePool[]>([]);
  const [diskPool, setDiskPool] = useState("");
  const [networks, setNetworks] = useState<NetworkInfo[]>([]);
  const [network, setNetwork] = useState("");
  const [profiles, setProfiles] = useState<string[]>(["default"]);
  const [availableProfiles, setAvailableProfiles] = useState<ProfileInfo[]>([]);
  const [configRows, setConfigRows] = useState<ConfigRow[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [submitting, setSubmitting] = useState(false);

  useEffect(() => {
    api.listImages().then(setImages).catch(() => setImages([]));
    api
      .listStoragePools()
      .then((p) => {
        setPools(p);
        if (p.length > 0) setDiskPool((cur) => cur || p[0].name);
      })
      .catch(() => setPools([]));
    api.listNetworks().then(setNetworks).catch(() => setNetworks([]));
    api
      .listProfiles()
      .then((p) => {
        setAvailableProfiles(p);
        setProfiles(
          p.some((prof) => prof.name === "default") ? ["default"] : []
        );
      })
      .catch(() => setAvailableProfiles([]));
  }, []);

  const toggleProfile = (profile: string, checked: boolean) => {
    setProfiles((cur) =>
      checked ? [...cur, profile] : cur.filter((p) => p !== profile)
    );
  };

  const imageRefOf = (img: ImageInfo) => img.aliases[0] ?? img.fingerprint;
  const compatibleImages = useMemo(
    () =>
      images.filter(
        (img) => !img.instanceType || img.instanceType === type
      ),
    [images, type]
  );

  useEffect(() => {
    if (images.length === 0) return;
    const selected = images.find((img) => imageRefOf(img) === image);
    if (selected && !compatibleImages.includes(selected)) {
      setImage("ubuntu:24.04");
    }
  }, [compatibleImages, images, image]);

  const nameValid = /^[a-zA-Z0-9_-]{1,63}$/.test(name.trim());
  const imageValid = image.trim().length > 0;

  const spec: CreateInstanceSpec | null = useMemo(() => {
    if (!nameValid || !imageValid) return null;
    return {
      name: name.trim(),
      image: image.trim(),
      type,
      profiles: profiles.length > 0 ? profiles : ["default"],
      limits: {
        cpu: cpu ? Number(cpu) : undefined,
        memory:
          memoryValue && memoryUnit === "MiB"
            ? memoryValue
            : memoryValue
              ? `${memoryValue}${memoryUnit}`
              : undefined,
        disk_gb: diskGb ? Number(diskGb) : undefined,
      },
      disk_pool: diskPool || undefined,
      network: network || null,
      config: Object.fromEntries(
        configRows.filter((r) => r.key.trim()).map((r) => [r.key.trim(), r.value])
      ),
    };
  }, [
    nameValid,
    imageValid,
    name,
    image,
    type,
    profiles,
    cpu,
    memoryValue,
    memoryUnit,
    diskGb,
    diskPool,
    network,
    configRows,
  ]);

  const submit = async () => {
    if (!spec) return;
    setError(null);
    setSubmitting(true);
    try {
      await api.createInstance(spec);
      onCreated();
    } catch (e) {
      setError((e as Error).message);
    } finally {
      setSubmitting(false);
    }
  };

  const generalTab = (
    <Form isHorizontal>
      <FormGroup label="Name" isRequired fieldId="wiz-name">
        <TextInput
          id="wiz-name"
          value={name}
          onChange={(_e, v) => setName(v)}
          placeholder="my-instance"
          validated={name && !nameValid ? "error" : "default"}
        />
        {name && !nameValid && (
          <HelperText>
            <HelperTextItem variant="error">
              1-63 characters; letters, digits, dashes and underscores only
            </HelperTextItem>
          </HelperText>
        )}
      </FormGroup>
      <FormGroup label="Instance type" fieldId="wiz-type">
        <FormSelect
          id="wiz-type"
          value={type}
          onChange={(_e, v) => setType(v as typeof type)}
        >
          <FormSelectOption value="container" label="Container (system)" />
          <FormSelectOption value="virtual-machine" label="Virtual machine" />
        </FormSelect>
        <HelperText>
          <HelperTextItem>
            VMs require an image with support for the Incus agent and more host
            resources.
          </HelperTextItem>
        </HelperText>
      </FormGroup>
      {images.length > 0 && (
        <FormFieldGroup header={<FormFieldGroupHeader titleText={{ text: `Local images (${type === "container" ? "container" : "VM"}-compatible)`, id: "fg-local-images" }} />} >
          {compatibleImages.map((img) => {
            const ref = imageRefOf(img);
            const label =
              img.description ||
              `${img.fingerprint.slice(0, 12)} · ${(
                img.sizeBytes /
                1024 /
                1024
              ).toFixed(0)} MB`;
            return (
              <Radio
                key={img.fingerprint}
                isChecked={image === ref}
                name="image-source-local"
                onChange={() => setImage(ref)}
                label={label}
                description={`${
                  img.instanceType ? `[${img.instanceType === "virtual-machine" ? "VM" : "CT"}] ` : ""
                }${ref.slice(0, 12)} · ${(
                  img.sizeBytes /
                  1024 /
                  1024
                ).toFixed(0)} MB`}
                id={`img-${img.fingerprint}`}
              />
            );
          })}
          {compatibleImages.length === 0 && (
            <HelperText>
              <HelperTextItem>
                No local images compatible with {type === "container" ? "containers" : "VMs"}.
                Use a remote reference below instead.
              </HelperTextItem>
            </HelperText>
          )}
        </FormFieldGroup>
      )}
      <FormFieldGroup header={<FormFieldGroupHeader titleText={{ text: "Remote or other source", id: "fg-remote-source" }} />} >
        <FormGroup label="Image reference" fieldId="wiz-image">
          <TextInput
            id="wiz-image"
            value={image}
            onChange={(_e, v) => setImage(v)}
            list="wizard-image-suggestions"
          />
          <datalist id="wizard-image-suggestions">
            {REMOTE_SUGGESTIONS.map((s) => (
              <option key={s} value={s} />
            ))}
          </datalist>
          <HelperText>
            <HelperTextItem>
              Remote refs use remote:alias syntax — e.g. ubuntu:24.04 (Ubuntu
              cloud images), images:debian/12 (linuxcontainers.org) — or a local
              alias / fingerprint.
            </HelperTextItem>
          </HelperText>
        </FormGroup>
      </FormFieldGroup>
      <FormGroup label="Profiles" fieldId="wiz-profiles">
        {availableProfiles.map((p) => (
          <Checkbox
            key={p.name}
            id={`profile-${p.name}`}
            label={p.name}
            isChecked={profiles.includes(p.name)}
            onChange={(_e, checked) => toggleProfile(p.name, checked)}
          />
        ))}
        {availableProfiles.length === 0 && <span>No profiles found.</span>}
      </FormGroup>
    </Form>
  );

  const resourcesTab = (
    <Form isHorizontal>
      <FormGroup label="CPU limit" fieldId="wiz-cpu">
        <TextInput
          id="wiz-cpu"
          type="number"
          min={1}
          placeholder="unlimited"
          value={cpu}
          onChange={(_e, v) => setCpu(v)}
        />
        <HelperText>
          <HelperTextItem>Leave empty for no limit</HelperTextItem>
        </HelperText>
      </FormGroup>
      <FormGroup label="Memory limit" fieldId="wiz-mem">
        <div style={{ display: "flex", gap: "8px" }}>
          <TextInput
            id="wiz-mem"
            type="number"
            min={16}
            placeholder="unlimited"
            value={memoryValue}
            onChange={(_e, v) => setMemoryValue(v)}
          />
          <FormSelect
            id="wiz-mem-unit"
            value={memoryUnit}
            onChange={(_e, v) => setMemoryUnit(v)}
            style={{ width: "110px" }}
          >
            <FormSelectOption value="MiB" label="MiB" />
            <FormSelectOption value="GiB" label="GiB" />
          </FormSelect>
        </div>
      </FormGroup>
      <FormGroup label="Root disk size (GiB)" fieldId="wiz-disk">
        <TextInput
          id="wiz-disk"
          type="number"
          min={5}
          placeholder="pool default"
          value={diskGb}
          onChange={(_e, v) => setDiskGb(v)}
        />
      </FormGroup>
      <FormGroup label="Storage pool" fieldId="wiz-pool">
        <FormSelect
          id="wiz-pool"
          value={diskPool}
          onChange={(_e, v) => setDiskPool(v)}
        >
          {pools.map((p) => (
            <FormSelectOption
              key={p.name}
              value={p.name}
              label={`${p.name}${p.driver ? ` (${p.driver})` : ""}`}
            />
          ))}
        </FormSelect>
      </FormGroup>
    </Form>
  );

  const configTab = (
    <Form isHorizontal>
      <FormGroup label="NIC network" fieldId="wiz-net">
        <FormSelect
          id="wiz-net"
          value={network}
          onChange={(_e, v) => setNetwork(v)}
        >
          <FormSelectOption value="" label="Profile default" />
          {networks.map((n) => (
            <FormSelectOption
              key={n.name}
              value={n.name}
              label={`${n.name} (${n.type}${n.managed ? ", managed" : ""})`}
            />
          ))}
        </FormSelect>
        <HelperText>
          <HelperTextItem>
            Adds an eth0 NIC attached to the selected network, overriding the
            profile default.
          </HelperTextItem>
        </HelperText>
      </FormGroup>
      <FormFieldGroup header={<FormFieldGroupHeader titleText={{ text: "Extra configuration keys", id: "fg-config-keys" }} />} >
        {configRows.map((row, idx) => (
          <div
            key={idx}
            style={{ display: "flex", gap: "8px", marginBottom: "8px" }}
          >
            <TextInput
              aria-label="Config key"
              placeholder="e.g. boot.autostart"
              value={row.key}
              onChange={(_e, v) =>
                setConfigRows((rows) =>
                  rows.map((r, i) => (i === idx ? { ...r, key: v } : r))
                )
              }
            />
            <TextInput
              aria-label="Config value"
              placeholder="true"
              value={row.value}
              onChange={(_e, v) =>
                setConfigRows((rows) =>
                  rows.map((r, i) => (i === idx ? { ...r, value: v } : r))
                )
              }
            />
            <Button
              variant="plain"
              aria-label={`Remove config row ${idx + 1}`}
              onClick={() =>
                setConfigRows((rows) => rows.filter((_, i) => i !== idx))
              }
            >
              <TimesIcon />
            </Button>
          </div>
        ))}
        <Button
          variant="link"
          icon={<PlusCircleIcon />}
          onClick={() => setConfigRows((rows) => [...rows, { key: "", value: "" }])}
        >
          Add config key
        </Button>
        <HelperText>
          <HelperTextItem>
            Any instance config option — e.g. security.nesting=true,
            boot.autostart=true, user.* keys for cloud-init.
          </HelperTextItem>
        </HelperText>
      </FormFieldGroup>
    </Form>
  );

  const reviewTab = spec ? (
    <Form isHorizontal>
      <Title headingLevel="h3">Summary</Title>
      <table className="pf-v6-c-table pf-m-compact" style={{ marginTop: "12px" }}>
        <tbody>
          {[
            ["Name", spec.name],
            ["Type", spec.type],
            ["Image", spec.image],
            ["Profiles", spec.profiles.join(", ")],
            ["CPU limit", spec.limits.cpu ?? "unlimited"],
            ["Memory limit", spec.limits.memory ?? "unlimited"],
            ["Root disk", spec.limits.disk_gb ? `${spec.limits.disk_gb} GiB` : "pool default"],
            ["Storage pool", diskPool],
            ["Network", spec.network ?? "profile default"],
            ...Object.entries(spec.config ?? {}).map(([k, v]) => [`config: ${k}`, v]),
          ].map(([k, v]) => (
            <tr key={k as string}>
              <td style={{ fontWeight: 600, paddingRight: "16px" }}>{k}</td>
              <td>{String(v)}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </Form>
  ) : null;

  return (
    <>
      <style>{`.pf-v6-c-modal-box--create-instance .pf-v6-c-modal-box__body { height: 460px; overflow-y: auto; }`}</style>
      <Modal
        isOpen
        onClose={onClose}
        variant="large"
        className="pf-v6-c-modal-box--create-instance"
      >
        <ModalHeader title="Create instance" />
        <ModalBody>
          <Tabs
            activeKey={activeTab}
            onSelect={(_e, key) => setActiveTab(key)}
            aria-label="Instance creation tabs"
          >
          <Tab eventKey="general" title={<TabTitleText>General</TabTitleText>}>
            {generalTab}
          </Tab>
          <Tab eventKey="resources" title={<TabTitleText>Resources</TabTitleText>}>
            {resourcesTab}
          </Tab>
          <Tab eventKey="config" title={<TabTitleText>Configuration</TabTitleText>}>
            {configTab}
          </Tab>
          <Tab eventKey="review" title={<TabTitleText>Review</TabTitleText>}>
            {error && (
              <Alert variant="danger" title="Creation failed" isInline>
                {error}
              </Alert>
            )}
            {submitting && (
              <Alert variant="info" isInline title="Creating instance…" />
            )}
            {!spec && (
              <Alert
                variant="warning"
                isInline
                title="Provide a valid name and image to continue"
              />
            )}
            {reviewTab}
          </Tab>
        </Tabs>
      </ModalBody>
      <ModalFooter>
        <Button
          variant="primary"
          onClick={submit}
          isDisabled={!spec || submitting}
          isLoading={submitting}
        >
          Create instance
        </Button>
        <Button variant="link" onClick={onClose}>
          Cancel
        </Button>
      </ModalFooter>
      </Modal>
    </>
  );
}
