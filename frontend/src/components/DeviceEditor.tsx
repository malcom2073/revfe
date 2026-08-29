import { useState } from "react";
import { Button, FormSelect, TextInput } from "@patternfly/react-core";
import { MinusCircleIcon, PlusCircleIcon } from "@patternfly/react-icons";

export const DEVICE_TYPES = [
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

export const DEVICE_FIELDS: Record<string, string[]> = {
  disk: ["path", "pool", "size", "readonly"],
  nic: ["network", "nictype", "parent", "vlan", "name"],
  gpu: ["pci", "id", "gid"],
  usb: ["vendorid", "productid"],
  proxy: ["listen", "connect"],
};

export interface CharPair {
  key: string;
  value: string;
}

export interface DeviceEditorValue {
  name: string;
  type: string;
  /** Resolved properties for the payload (raw extras + filled known fields). */
  props: CharPair[];
}

function toRows(obj: Record<string, unknown>): CharPair[] {
  return Object.entries(obj).map(([key, value]) => ({
    key,
    value: String(value),
  }));
}

/** Convert an Incus devices map (name -> device object) into editor rows. */
export function devicesToRows(
  devices: Record<string, Record<string, unknown>>
): DeviceEditorValue[] {
  return Object.entries(devices).map(([devName, dev]) => ({
    name: devName,
    type: String(dev.type ?? "disk"),
    props: toRows(dev).filter((row) => row.key !== "type"),
  }));
}

interface Row {
  name: string;
  type: string;
  props: CharPair[];
  known: Record<string, string>;
}

function resolve(row: Row): DeviceEditorValue {
  const known = Object.entries(row.known)
    .filter(([, v]) => v !== "")
    .map(([key, value]) => ({ key, value }));
  return { name: row.name, type: row.type, props: [...row.props, ...known] };
}

export default function DeviceEditor({
  value,
  onChange,
}: {
  value: DeviceEditorValue[];
  onChange: (rows: DeviceEditorValue[]) => void;
}) {
  const [rows, setRows] = useState<Row[]>(() =>
    value.map((v) => {
      const knownFields = DEVICE_FIELDS[v.type] ?? [];
      const known: Record<string, string> = {};
      const props: CharPair[] = [];
      for (const p of v.props) {
        if (knownFields.includes(p.key)) {
          if (!(p.key in known)) known[p.key] = p.value;
        } else {
          props.push(p);
        }
      }
      return { name: v.name, type: v.type, props, known };
    })
  );

  const emit = (next: Row[]) => {
    setRows(next);
    onChange(next.map(resolve));
  };

  const mutate = (index: number, fn: (row: Row) => Row) => {
    emit(rows.map((r, i) => (i === index ? fn(r) : r)));
  };

  return (
    <>
      {rows.map((device, idx) => {
        const fields = DEVICE_FIELDS[device.type] ?? [];
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
                onChange={(_e, v) => mutate(idx, (d) => ({ ...d, name: v }))}
                style={{ maxWidth: 160 }}
              />
              <FormSelect
                aria-label={`Device type ${idx + 1}`}
                value={device.type}
                onChange={(_e, v) =>
                  mutate(idx, (d) => ({
                    name: d.name,
                    type: v,
                    props: [],
                    known: {},
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
                onClick={() => emit(rows.filter((_, i) => i !== idx))}
              >
                <MinusCircleIcon />
              </Button>
            </div>

            {fields.map((field) => (
              <div key={field} style={{ display: "flex", gap: 8, marginTop: 8 }}>
                <TextInput
                  readOnlyVariant="default"
                  aria-label={`Field ${field}`}
                  value={field}
                  style={{ maxWidth: 160 }}
                />
                <TextInput
                  aria-label={`${field} value`}
                  placeholder={field}
                  value={device.known[field] ?? ""}
                  onChange={(_e, v) =>
                    mutate(idx, (d) => ({
                      ...d,
                      known: { ...d.known, [field]: v },
                    }))
                  }
                />
              </div>
            ))}

            {device.props.map((prop, pIdx) => (
              <div key={pIdx} style={{ display: "flex", gap: 8, marginTop: 8 }}>
                <TextInput
                  aria-label={`Device ${device.name || idx + 1} extra key ${pIdx + 1}`}
                  placeholder="key"
                  value={prop.key}
                  onChange={(_e, v) =>
                    mutate(idx, (d) => ({
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
                    mutate(idx, (d) => ({
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
                    mutate(idx, (d) => ({
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
                mutate(idx, (d) => ({
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
        onClick={() =>
          emit([
            ...rows,
            {
              name: "",
              type: "disk",
              props: [],
              known: { path: "/", pool: "default" },
            },
          ])
        }
      >
        Add device
      </Button>
    </>
  );
}