import { useState } from "react";
import {
  Alert,
  Button,
  Form,
  FormGroup,
  HelperText,
  HelperTextItem,
  Modal,
  ModalBody,
  ModalFooter,
  ModalHeader,
  TextInput,
} from "@patternfly/react-core";
import { api } from "../api/client";

const NAME_RE = /^[a-zA-Z0-9_-]{1,63}$/;

export default function RenameInstanceModal({
  name,
  onClose,
  onSaved,
}: {
  name: string;
  onClose: () => void;
  onSaved: (newName: string) => void;
}) {
  const [newName, setNewName] = useState(name);
  const [error, setError] = useState<string | null>(null);
  const [saving, setSaving] = useState(false);

  const nameValid = NAME_RE.test(newName.trim());
  const unchanged = newName.trim() === name;

  const rename = async () => {
    setError(null);
    setSaving(true);
    try {
      await api.renameInstance(name, newName.trim());
      onSaved(newName.trim());
    } catch (e) {
      setError((e as Error).message);
      setSaving(false);
    }
  };

  return (
    <Modal isOpen onClose={onClose}>
      <ModalHeader title={`Rename instance "${name}"`} />
      <ModalBody>
        {error && (
        <Alert variant="danger" title="Rename failed" isInline>
          {error}
        </Alert>
      )}
        <Form isHorizontal className="pf-v6-u-mt-md">
          <FormGroup label="New name" isRequired fieldId="rename-name">
            <TextInput
              id="rename-name"
              value={newName}
              onChange={(_e, v) => setNewName(v)}
              validated={newName && !nameValid ? "error" : "default"}
              onKeyDown={(e) => e.key === "Enter" && nameValid && !unchanged && rename()}
            />
            {newName && !nameValid && (
              <HelperText>
                <HelperTextItem variant="error">
                  1-63 characters; letters, digits, dashes and underscores only
                </HelperTextItem>
              </HelperText>
            )}
          </FormGroup>
        </Form>
      </ModalBody>
      <ModalFooter>
        <Button
          variant="primary"
          onClick={rename}
          isDisabled={!nameValid || unchanged || saving}
          isLoading={saving}
        >
          Rename
        </Button>{" "}
        <Button variant="link" onClick={onClose}>
          Cancel
        </Button>
      </ModalFooter>
    </Modal>
  );
}