import { useEffect, useMemo, useState } from "react";
import {
  Alert,
  Button,
  Checkbox,
  Form,
  FormGroup,
  Modal,
  ModalBody,
  ModalFooter,
  ModalHeader,
  Tab,
  Tabs,
  TabTitleText,
  TextInput,
  HelperText,
  HelperTextItem,
  Title,
} from "@patternfly/react-core";
import { MinusCircleIcon, PlusCircleIcon } from "@patternfly/react-icons";
import { api } from "../api/client";
import type { Instance, ProfileInfo } from "../api/types";
import DeviceEditor, {
  devicesToRows,
  type DeviceEditorValue,
} from "../components/DeviceEditor";

interface KV {
  key: string;
  value: string;
}

const NAME_RE = /^[a-zA-Z0-9_-]{1,63}$/;

function toRows(obj: Record<string, unknown>): KV[] {
  return Object.entries(obj).map(([key, value]) => ({
    key,
    value: String(value),
  }));
}

/** Only the user-editable config keys belong in a PATCH payload; volatile.*
 * and image.* keys are managed or read-only on the Incus side. */
function editableRows(cfg: Record<string, unknown>): KV[] {
  return toRows(cfg).filter(
    (r) => !r.key.startsWith("volatile.") && !r.key.startsWith("image.")
  );
}

export default function InstanceEditModal({
  instance,
  onClose,
  onSaved,
}: {
  instance: Instance;
  onClose: () => void;
  onSaved: () => void;
}) {
  const [activeTab, setActiveTab] = useState<string | number>("general");
  const [profiles, setProfiles] = useState<string[]>(instance.profiles ?? []);
  const [availableProfiles, setAvailableProfiles] = useState<ProfileInfo[]>([]);
  const [configRows, setConfigRows] = useState<KV[]>(
    editableRows((instance.config ?? {}) as Record<string, unknown>)
  );
  const [deviceRows, setDeviceRows] = useState<DeviceEditorValue[]>(() =>
    devicesToRows((instance.devices ?? {}) as Record<
      string,
      Record<string, unknown>
    >)
  );
  const [error, setError] = useState<string | null>(null);
  const [saving, setSaving] = useState(false);

  useEffect(() => {
    api.listProfiles().then(setAvailableProfiles).catch(() => setAvailableProfiles([]));
  }, []);

  const devicesValid = deviceRows.every(
    (d) => !d.name.trim() || NAME_RE.test(d.name.trim())
  );
  const canSave = profiles.length > 0 && devicesValid && !saving;

  const payload = useMemo(
    () => ({
      profiles,
      config: Object.fromEntries(
        configRows
          .filter((r) => r.key.trim())
          .map((r) => [r.key.trim(), r.value])
      ),
      devices: Object.fromEntries(
        deviceRows
          .filter((d) => d.name.trim())
          .map((d) => [
            d.name.trim(),
            {
              type: d.type,
              ...Object.fromEntries(
                d.props
                  .filter((p) => p.key.trim())
                  .map((p) => [p.key.trim(), p.value])
              ),
            },
          ])
      ),
    }),
    [profiles, configRows, deviceRows]
  );

  const save = async () => {
    setError(null);
    setSaving(true);
    try {
      await api.updateInstance(instance.name, payload);
      onSaved();
    } catch (e) {
      setError((e as Error).message);
      setSaving(false);
    }
  };

  const generalTab = (
    <Form isHorizontal>
      <FormGroup label="Name" fieldId="edit-name">
        <TextInput id="edit-name" value={instance.name} isDisabled />
      </FormGroup>
      <FormGroup label="Type" fieldId="edit-type">
        <TextInput id="edit-type" value={instance.type} isDisabled />
      </FormGroup>
      <FormGroup label="Profiles" isRequired fieldId="edit-profiles">
        {availableProfiles.map((p) => (
          <Checkbox
            key={p.name}
            id={`edit-profile-${p.name}`}
            label={p.name}
            isChecked={profiles.includes(p.name)}
            onChange={(_e, checked) =>
              setProfiles((cur) =>
                checked
                  ? [...cur, p.name]
                  : cur.filter((x) => x !== p.name)
              )
            }
          />
        ))}
        {profiles.length === 0 && (
          <HelperText>
            <HelperTextItem variant="error">
              At least one profile is required.
            </HelperTextItem>
          </HelperText>
        )}
      </FormGroup>
    </Form>
  );

  const configTab = (
    <Form isHorizontal>
      <FormGroup label="Config keys" fieldId="edit-config">
        {configRows.map((row, idx) => (
          <div
            key={idx}
            style={{ display: "flex", gap: 8, marginBottom: 8 }}
          >
            <TextInput
              aria-label={`Config key ${idx + 1}`}
              placeholder="limits.cpu"
              value={row.key}
              onChange={(_e, v) =>
                setConfigRows((rows) =>
                  rows.map((r, i) => (i === idx ? { ...r, key: v } : r))
                )
              }
            />
            <TextInput
              aria-label={`Config value ${idx + 1}`}
              placeholder="2"
              value={row.value}
              onChange={(_e, v) =>
                setConfigRows((rows) =>
                  rows.map((r, i) => (i === idx ? { ...r, value: v } : r))
                )
              }
            />
            <Button
              variant="plain"
              aria-label={`Remove config key ${idx + 1}`}
              onClick={() =>
                setConfigRows((rows) => rows.filter((_, i) => i !== idx))
              }
            >
              <MinusCircleIcon />
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
      </FormGroup>
      <FormGroup label="Devices" fieldId="edit-devices">
        <DeviceEditor value={deviceRows} onChange={setDeviceRows} />
      </FormGroup>
    </Form>
  );

  const reviewTab = (
    <Form isHorizontal>
      <Title headingLevel="h3">Summary of changes</Title>
      <table className="pf-v6-c-table pf-m-compact" style={{ marginTop: "12px" }}>
        <tbody>
          {[
            ["Profiles", profiles.join(", ") || "—"],
            ...Object.entries(payload.config).map(([k, v]) => [
              `config: ${k}`,
              String(v),
            ]),
            ...Object.keys(payload.devices).map((name) => [
              `device: ${name}`,
              JSON.stringify(payload.devices[name]),
            ]),
          ].map(([k, v]) => (
            <tr key={k}>
              <td style={{ fontWeight: 600, paddingRight: "16px" }}>{k}</td>
              <td>{v}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </Form>
  );

  return (
    <Modal isOpen variant="large" onClose={onClose}>
      <ModalHeader title={`Edit instance "${instance.name}"`} />
      <ModalBody>
        {error && (
          <Alert
            variant="danger"
            title="Update failed"
            isInline
            className="pf-v6-u-mb-md"
          >
            {error}
          </Alert>
        )}
        <Tabs
          activeKey={activeTab}
          onSelect={(_e, key) => setActiveTab(key)}
          aria-label="Instance edit tabs"
          className="pf-v6-u-mt-md"
        >
          <Tab eventKey="general" title={<TabTitleText>General</TabTitleText>}>
            {generalTab}
          </Tab>
          <Tab
            eventKey="config"
            title={<TabTitleText>Configuration</TabTitleText>}
          >
            {configTab}
          </Tab>
          <Tab eventKey="review" title={<TabTitleText>Review</TabTitleText>}>
            {error && (
              <Alert variant="danger" title="Update failed" isInline className="pf-v6-u-mt-md">
                {error}
              </Alert>
            )}
            {reviewTab}
          </Tab>
        </Tabs>
      </ModalBody>
      <ModalFooter>
        <Button variant="primary" onClick={save} isDisabled={!canSave} isLoading={saving}>
          Save changes
        </Button>{" "}
        <Button variant="link" onClick={onClose}>
          Cancel
        </Button>
      </ModalFooter>
    </Modal>
  );
}