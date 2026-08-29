import { useEffect, useState } from "react";
import {
  Button,
  FormSelect,
  HelperText,
  HelperTextItem,
  TextInput,
} from "@patternfly/react-core";
import { MinusCircleIcon, PlusCircleIcon } from "@patternfly/react-icons";
import { api } from "../api/client";

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

export const NIC_TYPES = [
  "bridged",
  "macvlan",
  "ipvlan",
  "physical",
  "routed",
  "ovn",
  "sriov",
];

/** nictypes that attach the instance directly to a host interface (`parent`). */
const PARENT_BASED_NICTYPES = ["macvlan", "ipvlan", "physical", "routed", "sriov"];

const MACVLAN_HINT =
  "Macvlan puts the container directly on your LAN: it borrows the parent " +
  "interface's L2 segment and typically gets its address from your network " +
  "router's DHCP (a routable/external IP). The container cannot reach the " +
  "host itself over the macvlan link.";

const NICTYPE_HINTS: Record<string, string> = {
  macvlan: MACVLAN_HINT,
  ipvlan:
    "Like macvlan but all child interfaces share the parent's MAC, so the " +
    "LAN sees extra IPs on the same MAC. The container cannot reach the host.",
  physical:
    "Passes the entire host interface through to the container; only one " +
    "instance can use the interface at a time.",
  routed:
    "A routed interface with its own subnets; traffic is routed rather than " +
    "bridged and the container can reach the host.",
  bridged:
    "Attaches to a managed Incus bridge network (e.g. incusbr0) that provides " +
    "NAT'd addresses and per-instance DHCP.",
  ovn: "Attaches to a managed OVN overlay network.",
  sriov: "Assigns a virtual function from an SR-IOV-capable physical interface.",
};

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
  const [hostInterfaces, setHostInterfaces] = useState<string[]>([]);

  useEffect(() => {
    api
      .listHostInterfaces()
      .then((ifaces) => setHostInterfaces(ifaces.map((i) => i.name)))
      .catch(() => setHostInterfaces([]));
  }, []);

  const emit = (next: Row[]) => {
    setRows(next);
    onChange(next.map(resolve));
  };

  const mutate = (index: number, fn: (row: Row) => Row) => {
    emit(rows.map((r, i) => (i === index ? fn(r) : r)));
  };

  const setField = (idx: number, field: string, v: string) =>
    mutate(idx, (d) => ({ ...d, known: { ...d.known, [field]: v } }));

  const nictypeOf = (row: Row) => (row.known["nictype"] ?? "").trim();

  const fieldNode = (row: Row, field: string) => {
    if (row.type === "nic" && field === "nictype") {
      return (
        <TextInput
          readOnlyVariant="default"
          aria-label="Field nictype"
          value="nictype"
          style={{ maxWidth: 160 }}
        />
      );
    }
    return (
      <TextInput
        readOnlyVariant="default"
        aria-label={`Field ${field}`}
        value={field}
        style={{ maxWidth: 160 }}
      />
    );
  };

  const valueNode = (row: Row, field: string) => {
    if (row.type === "nic" && field === "nictype") {
      return (
        <FormSelect
          aria-label="nictype value"
          value={row.known["nictype"] ?? ""}
          onChange={(_e, v) => setField(rows.indexOf(row), "nictype", v)}
        >
          <option value="">auto (from network)</option>
          {NIC_TYPES.map((t) => (
            <option key={t} value={t}>
              {t}
            </option>
          ))}
        </FormSelect>
      );
    }
    if (
      row.type === "nic" &&
      field === "parent" &&
      PARENT_BASED_NICTYPES.includes(nictypeOf(row))
    ) {
      return (
        <TextInput
          aria-label="parent value"
          list="incus-host-interfaces"
          placeholder="host interface (e.g. enp3s0)"
          value={row.known["parent"] ?? ""}
          onChange={(_e, v) => setField(rows.indexOf(row), "parent", v)}
        />
      );
    }
    return (
      <TextInput
        aria-label={`${field} value`}
        placeholder={field}
        value={row.known[field] ?? ""}
        onChange={(_e, v) => setField(rows.indexOf(row), field, v)}
      />
    );
  };

  return (
    <>
      <datalist id="incus-host-interfaces">
        {hostInterfaces.map((name) => (
          <option key={name} value={name} />
        ))}
      </datalist>
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
                {fieldNode(device, field)}
                {valueNode(device, field)}
              </div>
            ))}

            {device.type === "nic" && nictypeOf(device) && (
              <HelperText style={{ marginTop: 6 }}>
                <HelperTextItem variant="default">
                  {NICTYPE_HINTS[nictypeOf(device)] ??
                    "No extra guidance available for this nic type."}
                </HelperTextItem>
              </HelperText>
            )}

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