import { useMemo, useState } from "react";
import {
  Alert,
  Button,
  Form,
  FormGroup,
  FormSelect,
  Modal,
  ModalBody,
  ModalFooter,
  ModalHeader,
  TextInput,
} from "@patternfly/react-core";
import { MinusCircleIcon, PlusCircleIcon } from "@patternfly/react-icons";
import { api } from "../api/client";
import type { ProfileInfo } from "../api/types";

const DEVICE_TYPES = [
  "disk",
  "nic",
  "gpu",
  "usb",
  "unix-block",
  "unix-char",
  "infiniband",
  "pci",
  "tpm",
  "proxy",
];

/** Common fields surfaced per device type; anything else is editable as raw key/value. */
const DEVICE_FIELDS: Record<string, string[]> = {
  disk: ["path", "pool", "size", "readonly"],
  nic: ["network", "nictype", "parent", "vlan", "name"],
  gpu: ["pci", "id", "gid"],
  usb: ["vendorid", "productid"],
  proxy: ["listen", "connect"],
};

interface KV {
  key: string;
  value: string;
}

interface DeviceRow {
  name: string;
  type: string;
  props: KV[];
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
  const fieldInputsInit: Record<number, Record<string, string>> = {};
  const [devices, setDevices] = useState<DeviceRow[]>(() => {
    // New profiles start with one empty root-disk row to fill in.
    if (!existing) {
      fieldInputsInit[0] = { path: "/", pool: "default", size: "" };
      return [{ name: "", type: "disk", props: [] }];
    }
    let idx = 0;
    return Object.entries(existing.devices).map(([devName, dev]) => {
      const type = String(dev.type ?? "disk");
      const known = DEVICE_FIELDS[type] ?? [];
      const props = toRows(dev).filter(
        (row) => row.key !== "type" && !known.includes(row.key)
      );
      const vals: Record<string, string> = {};
      for (const [k, v] of Object.entries(dev)) {
        if (k !== "type" && known.includes(k)) vals[k] = String(v);
      }
      fieldInputsInit[idx] = vals;
      idx++;
      return { name: devName, type, props };
    });
  });
  const [fieldInputs, setFieldInputs] = useState<
    Record<number, Record<string, string>>
  >(fieldInputsInit);
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
        ...Object.fromEntries(
          Object.entries(fieldInputs[devices.indexOf(d)] ?? {}).filter(
            ([, v]) => v !== ""
          )
        ),
      })),
    };
  }, [name, description, configRows, devices, fieldInputs]);

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

  const updateDevice = (
    index: number,
    mutate: (d: DeviceRow) => DeviceRow
  ) => {
    setDevices((rows) => rows.map((d, i) => (i === index ? mutate(d) : d)));
  };

  const setKnownValue = (index: number, field: string, value: string) => {
    setFieldInputs((cur) => ({
      ...cur,
      [index]: { ...(cur[index] ?? {}), [field]: value },
    }));
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
            {devices.map((device, idx) => {
              const fields = DEVICE_FIELDS[device.type] ?? [];
              const extras = device.props;
              return (
                <div
                  key={idx}
                  style={{
                    border: "1px solid rgba(255,255,255,0.12)",
                    borderRadius: 6,
                    padding: 12,
                    marginBottom: 12,
                  }}
                >
                  <div style={{ display: "flex", gap: 8 }}>
                    <TextInput
                      aria-label={`Device name ${idx + 1}`}
                      placeholder="name (e.g. root)"
                      value={device.name}
                      onChange={(_e, v) =>
                        updateDevice(idx, (d) => ({ ...d, name: v }))
                      }
                      style={{ maxWidth: 160 }}
                    />
                    <FormSelect
                      aria-label={`Device type ${idx + 1}`}
                      value={device.type}
                      onChange={(_e, v) =>
                        updateDevice(idx, (d) => ({
                          name: d.name,
                          type: v,
                          props: [],
                        }))
                      }
                    >
                      {DEVICE_TYPES.map((t) => (
                        <option key={t} value={t}>
                          {t}
                        </option>
                      ))}
                    </FormSelect>
                    <Button
                      variant="plain"
                      aria-label={`Remove device ${device.name || idx + 1}`}
                      onClick={() =>
                        setDevices((rows) => rows.filter((_, i) => i !== idx))
                      }
                    >
                      <MinusCircleIcon />
                    </Button>
                  </div>

                  {fields.map((field) => (
                    <div
                      key={field}
                      style={{ display: "flex", gap: 8, marginTop: 8 }}
                    >
                      <TextInput
                        readOnlyVariant="default"
                        aria-label={`Field ${field}`}
                        value={field}
                        style={{ maxWidth: 160 }}
                      />
                      <TextInput
                        aria-label={`${field} value`}
                        placeholder={field}
                        value={fieldInputs[idx]?.[field] ?? ""}
                        onChange={(_e, v) => setKnownValue(idx, field, v)}
                      />
                    </div>
                  ))}

                  {extras.map((prop, pIdx) => (
                    <div
                      key={pIdx}
                      style={{ display: "flex", gap: 8, marginTop: 8 }}
                    >
                      <TextInput
                        aria-label={`Device ${device.name || idx + 1} extra key ${pIdx + 1}`}
                        placeholder="key"
                        value={prop.key}
                        onChange={(_e, v) =>
                          updateDevice(idx, (d) => ({
                            ...d,
                            props: d.props.map((pr, i) =>
                              i === pIdx ? { ...pr, key: v } : pr
                            ),
                          }))
                        }
                        style={{ maxWidth: 160 }}
                      />
                      <TextInput
                        aria-label={`Device ${device.name || idx + 1} extra value ${pIdx + 1}`}
                        placeholder="value"
                        value={prop.value}
                        onChange={(_e, v) =>
                          updateDevice(idx, (d) => ({
                            ...d,
                            props: d.props.map((pr, i) =>
                              i === pIdx ? { ...pr, value: v } : pr
                            ),
                          }))
                        }
                      />
                      <Button
                        variant="plain"
                        aria-label={`Remove property ${prop.key || pIdx + 1}`}
                        onClick={() =>
                          updateDevice(idx, (d) => ({
                            ...d,
                            props: d.props.filter((_, i) => i !== pIdx),
                          }))
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
                      updateDevice(idx, (d) => ({
                        ...d,
                        props: [...d.props, { key: "", value: "" }],
                      }))
                    }
                  >
                    Add custom field
                  </Button>
                </div>
              );
            })}
            <Button
              variant="link"
              icon={<PlusCircleIcon />}
              onClick={() => {
                setFieldInputs((cur) => ({ ...cur, [devices.length]: {} }));
                setDevices((rows) => [
                  ...rows,
                  { name: "", type: "disk", props: [] },
                ]);
              }}
            >
              Add device
            </Button>
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
