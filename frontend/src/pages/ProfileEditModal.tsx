import { useMemo, useState } from "react";
import {
  Alert,
  Button,
  Form,
  FormGroup,
  Modal,
  ModalBody,
  ModalFooter,
  ModalHeader,
  TextInput,
} from "@patternfly/react-core";
import { MinusCircleIcon, PlusCircleIcon } from "@patternfly/react-icons";
import { api } from "../api/client";
import type { ProfileInfo } from "../api/types";
import DeviceEditor, {
  devicesToRows,
  type DeviceEditorValue,
} from "../components/DeviceEditor";

interface KV {
  key: string;
  value: string;
}

function toRows(obj: Record<string, unknown>): KV[] {
  return Object.entries(obj).map(([key, value]) => ({
    key,
    value: String(value),
  }));
}

export default function ProfileEditModal({
  existing,
  onClose,
  onSaved,
}: {
  /** Null when creating a new profile. */
  existing: ProfileInfo | null;
  onClose: () => void;
  onSaved: (name: string) => void;
}) {
  const isEdit = existing !== null;
  const [name, setName] = useState(existing?.name ?? "");
  const [description, setDescription] = useState(existing?.description ?? "");
  const [configRows, setConfigRows] = useState<KV[]>(
    toRows(existing?.config ?? {})
  );
  const [devices, setDevices] = useState<DeviceEditorValue[]>(() => {
    if (!existing) {
      return [{ name: "", type: "disk", props: [{ key: "path", value: "/" }, { key: "pool", value: "default" }] }];
    }
    return devicesToRows(existing.devices);
  });
  const [error, setError] = useState<string | null>(null);
  const [saving, setSaving] = useState(false);

  const nameValid = /^[a-zA-Z0-9_-]{1,63}$/.test(name.trim());
  const devicesValid = devices.every(
    (d) => /^[a-zA-Z0-9_-]{1,63}$/.test(d.name.trim()) && d.type
  );

  const payload = useMemo(() => {
    return {
      name: name.trim(),
      description: description.trim(),
      config: Object.fromEntries(
        configRows
          .filter((r) => r.key.trim())
          .map((r) => [r.key.trim(), r.value])
      ),
      devices: devices.map((d) => ({
        name: d.name.trim(),
        type: d.type,
        ...Object.fromEntries(
          d.props.filter((p) => p.key.trim()).map((p) => [p.key.trim(), p.value])
        ),
      })),
    };
  }, [name, description, configRows, devices]);

  const save = async () => {
    setError(null);
    setSaving(true);
    try {
      if (isEdit && existing) {
        await api.updateProfile(existing.name, payload);
        onSaved(existing.name);
      } else {
        await api.createProfile(payload);
        onSaved(payload.name);
      }
    } catch (e) {
      setError((e as Error).message);
      setSaving(false);
    }
  };

  return (
    <Modal
      isOpen
      variant="large"
      onClose={onClose}
      aria-label={isEdit ? `Edit profile ${name}` : "Create profile"}
    >
      <ModalHeader
        title={isEdit ? `Edit profile "${name}"` : "Create profile"}
      />
      <ModalBody>
        <Form isHorizontal>
          {error && (
            <Alert variant="danger" title={error} isInline />
          )}
          <FormGroup label="Name" isRequired fieldId="prof-name">
            <TextInput
              id="prof-name"
              value={name}
              onChange={(_e, v) => setName(v)}
              isDisabled={isEdit}
              validated={name && !nameValid ? "error" : "default"}
            />
          </FormGroup>
          <FormGroup label="Description" fieldId="prof-desc">
            <TextInput
              id="prof-desc"
              value={description}
              onChange={(_e, v) => setDescription(v)}
              placeholder="What is this profile for?"
            />
          </FormGroup>

          {/* Config */}
          <FormGroup label="Config keys" fieldId="prof-config">
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
                      rows.map((r, i) =>
                        i === idx ? { ...r, key: v } : r
                      )
                    )
                  }
                />
                <TextInput
                  aria-label={`Config value ${idx + 1}`}
                  placeholder="2"
                  value={row.value}
                  onChange={(_e, v) =>
                    setConfigRows((rows) =>
                      rows.map((r, i) =>
                        i === idx ? { ...r, value: v } : r
                      )
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
              onClick={() =>
                setConfigRows((rows) => [...rows, { key: "", value: "" }])
              }
            >
              Add config key
            </Button>
          </FormGroup>

          {/* Devices */}
          <FormGroup label="Devices" fieldId="prof-devices">
            <DeviceEditor value={devices} onChange={setDevices} />
          </FormGroup>
        </Form>
      </ModalBody>
      <ModalFooter>
        <Button
          variant="primary"
          onClick={save}
          isLoading={saving}
          isDisabled={!nameValid || !devicesValid || saving}
        >
          {isEdit ? "Save changes" : "Create profile"}
        </Button>{" "}
        <Button variant="link" onClick={onClose}>
          Cancel
        </Button>
      </ModalFooter>
    </Modal>
  );
}
