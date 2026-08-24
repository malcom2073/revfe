import { useEffect, useRef } from "react";
import { Terminal as XTerm } from "@xterm/xterm";
import { FitAddon } from "@xterm/addon-fit";
import { execWsUrl } from "../api/client";

export default function Terminal({
  instanceName,
  shell,
}: {
  instanceName: string;
  shell: string;
}) {
  const containerRef = useRef<HTMLDivElement | null>(null);
  const termRef = useRef<XTerm | null>(null);

  useEffect(() => {
    const term = new XTerm({
      cursorBlink: true,
      fontFamily: "'Red Hat Mono', monospace",
      fontSize: 14,
      theme: {
        background: "#151515",
        foreground: "#f2f2f2",
      },
    });
    const fit = new FitAddon();
    term.loadAddon(fit);
    term.open(containerRef.current!);
    fit.fit();
    termRef.current = term;

    term.writeln(`Connecting to ${instanceName} (${shell})…`);

    let ws: WebSocket | null = null;
    try {
      ws = new WebSocket(execWsUrl(instanceName, shell));
    } catch (e) {
      term.writeln(`Connection failed: ${e}`);
    }

    if (ws) {
      ws.binaryType = "arraybuffer";
      ws.onopen = () => {
        term.writeln("Connected.");
        resize();
        sendResize();
      };
      ws.onmessage = (event) => {
        term.write(
          typeof event.data === "string"
            ? event.data
            : new Uint8Array(event.data)
        );
      };
      ws.onclose = () => term.writeln("\r\n\x1b[90m[disconnected]\x1b[0m");
      ws.onerror = () => term.writeln("\r\n\x1b[31m[connection error]\x1b[0m");

      const dataDisposable = term.onData((data) => {
        if (ws?.readyState === WebSocket.OPEN) {
          ws.send(new TextEncoder().encode(data));
        }
      });

      const sendResize = () => {
        if (ws?.readyState === WebSocket.OPEN) {
          ws.send(
            JSON.stringify({ resize: { cols: term.cols, rows: term.rows } })
          );
        }
      };

      const resizeDisposable = term.onResize(sendResize);
      const onWinResize = () => fit.fit();
      window.addEventListener("resize", onWinResize);
      resize();

      return () => {
        dataDisposable.dispose();
        resizeDisposable.dispose();
        window.removeEventListener("resize", onWinResize);
        ws?.close();
        term.dispose();
        termRef.current = null;
      };
    }

    function resize() {
      try {
        fit.fit();
      } catch {
        // container not measurable yet
      }
    }

    return () => {
      term.dispose();
      termRef.current = null;
    };
  }, [instanceName, shell]);

  return (
    <div
      ref={containerRef}
      style={{ height: "60vh", padding: "8px", background: "#151515" }}
    />
  );
}
