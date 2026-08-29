import type { ReactNode } from "react";
import {
  Button,
  HelperText,
  HelperTextItem,
  TextInput,
} from "@patternfly/react-core";
import { MinusCircleIcon, PlusCircleIcon } from "@patternfly/react-icons";
import {
  KNOWN_CONFIG_KEYS,
  describeConfigKey,
} from "../util/configKeys";

export interface ConfigKeyRow {
  key: string;
  value: string;
}

interface Props {
  value: ConfigKeyRow[];
  onChange: (rows: ConfigKeyRow[]) => void;
  keyPlaceholder?: string;
  valuePlaceholder?: string;
  hint?: ReactNode;
}

const DATALIST_ID = "incus-config-keys";

export default function ConfigKeyEditor({
  value,
  onChange,
  keyPlaceholder = "limits.cpu",
  valuePlaceholder = "2",
  hint,
}: Props) {
  return (
    <>
      <datalist id={DATALIST_ID}>
        {KNOWN_CONFIG_KEYS.map(({ key }) => (
          <option key={key} value={key} />
        ))}
      </datalist>
      {value.map((row, idx) => {
        const description = describeConfigKey(row.key);
        return (
          <div key={idx} style={{ marginBottom: 8 }}>
            <div style={{ display: "flex", gap: 8 }}>
              <TextInput
                aria-label={`Config key ${idx + 1}`}
                list={DATALIST_ID}
                placeholder={keyPlaceholder}
                value={row.key}
                onChange={(_e, v) =>
                  onChange(
                    value.map((r, i) => (i === idx ? { ...r, key: v } : r))
                  )
                }
              />
              <TextInput
                aria-label={`Config value ${idx + 1}`}
                placeholder={valuePlaceholder}
                value={row.value}
                onChange={(_e, v) =>
                  onChange(
                    value.map((r, i) => (i === idx ? { ...r, value: v } : r))
                  )
                }
              />
              <Button
                variant="plain"
                aria-label={`Remove config key ${idx + 1}`}
                onClick={() =>
                  onChange(value.filter((_, i) => i !== idx))
                }
              >
                <MinusCircleIcon />
              </Button>
            </div>
            {description && (
              <HelperText style={{ marginTop: 4 }}>
                <HelperTextItem>{description}</HelperTextItem>
              </HelperText>
            )}
          </div>
        );
      })}
      <Button
        variant="link"
        icon={<PlusCircleIcon />}
        onClick={() => onChange([...value, { key: "", value: "" }])}
      >
        Add config key
      </Button>
      {hint}
    </>
  );
}