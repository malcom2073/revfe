import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import {
  Alert,
  Card,
  CardBody,
  CardTitle,
  DescriptionList,
  DescriptionListDescription,
  DescriptionListGroup,
  DescriptionListTerm,
  Gallery,
  PageSection,
  Title,
} from "@patternfly/react-core";
import {
  Area,
  AreaChart,
  CartesianGrid,
  Legend,
  ResponsiveContainer,
  Tooltip,
  XAxis,
  YAxis,
} from "recharts";
import { api } from "../api/client";
import type { MetricsHistory, MetricsPoint } from "../api/types";
import { formatBytes } from "../util/format";

const POLL_MS = 5_000;
const PALETTE = [
  "#0066cc", "#8f4700", "#3e8635", "#a30000",
  "#675347", "#40199a", "#4b5565", "#e0004d",
];

interface SeriesPoint {
  t: number;
  label: string;
  values: Record<string, number>;
}

function buildSeries(history: MetricsHistory): {
  cpuTotals: SeriesPoint[];
  mem: SeriesPoint[];
  netRates: SeriesPoint[];
} {
  const names = history.instances;
  const cpuTotals: SeriesPoint[] = [];
  const mem: SeriesPoint[] = [];
  const netRaw: { t: number; label: string; rx: number; tx: number }[] = [];

  for (const p of history.points) {
    // Backend timestamps are UTC ISO strings; they may or may not carry the
    // trailing "Z" depending on the endpoint — normalize before parsing.
    const iso = p.timestamp.endsWith("Z") ? p.timestamp : `${p.timestamp}Z`;
    const ts = new Date(iso).getTime();
    const label = new Date(ts).toLocaleTimeString([], {
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
    });
    const cpuVals: Record<string, number> = {};
    const memVals: Record<string, number> = {};
    let rx = 0;
    let tx = 0;
    for (const name of names) {
      const s: MetricsPoint["instances"][string] = p.instances[name] ?? {};
      cpuVals[name] = s.cpuSeconds ?? 0;
      memVals[name] = Math.round(((s.memoryUsed ?? 0) / 1048576) * 10) / 10;
      rx += s.netRx ?? 0;
      tx += s.netTx ?? 0;
    }
    cpuTotals.push({ t: ts, label, values: cpuVals });
    mem.push({ t: ts, label, values: memVals });
    netRaw.push({ t: ts, label, rx, tx });
  }

  const netRates: SeriesPoint[] = netRaw.map((cur, idx): SeriesPoint => {
    if (idx === 0) return { t: cur.t, label: cur.label, values: {} };
    const prev = netRaw[idx - 1];
    const dt = Math.max(1, (cur.t - prev.t) / 1000);
    return {
      t: cur.t,
      label: cur.label,
      values: {
        received: Math.max(0, cur.rx - prev.rx) / dt / 1024,
        transmitted: Math.max(0, cur.tx - prev.tx) / dt / 1024,
      },
    };
  });

  return { cpuTotals, mem, netRates };
}

/** Convert cumulative cpu-seconds into per-sample utilization (%). */
function cpuUtilization(totals: SeriesPoint[]): SeriesPoint[] {
  return totals.map((point, idx) => {
    if (idx === 0)
      return {
        ...point,
        values: Object.fromEntries(
          Object.keys(point.values).map((n) => [n, 0])
        ),
      };
    const prev = totals[idx - 1];
    const dt = Math.max(1, (point.t - prev.t) / 1000);
    return {
      ...point,
      values: Object.fromEntries(
        Object.entries(point.values).map(([name, total]) => [
          name,
          Math.max(
            0,
            Math.min(100, ((total - (prev.values[name] ?? total)) / dt) * 100)
          ),
        ])
      ),
    };
  });
}

interface TimeChartProps {
  series: SeriesPoint[];
  unit: string;
}

const AXIS_TICK = { fill: "#949494", fontSize: 11 };

function TimeChart({ series, unit }: TimeChartProps) {
  if (series.length < 2) {
    return (
      <div className="pf-v6-u-text-color-subtle">
        Collecting samples… chart available in a few seconds.
      </div>
    );
  }
  const names = Array.from(new Set(series.flatMap((p) => Object.keys(p.values))));
  // Recharts reads dataKeys from the top level of each row
  const rows = series.map((p) => ({ ...p.values, label: p.label }));

  return (
    <ResponsiveContainer width="100%" height={230}>
      <AreaChart data={rows} margin={{ top: 8, right: 12, left: 0, bottom: 0 }}>
        <defs>
          {names.map((name, i) => {
            const color = PALETTE[i % PALETTE.length];
            return (
              <linearGradient key={name} id={`grad-${unit}-${name}`} x1="0" y1="0" x2="0" y2="1">
                <stop offset="0%" stopColor={color} stopOpacity={0.45} />
                <stop offset="100%" stopColor={color} stopOpacity={0.02} />
              </linearGradient>
            );
          })}
        </defs>
        <CartesianGrid strokeDasharray="3 6" stroke="rgba(255,255,255,0.14)" vertical={false} />
        <XAxis
          dataKey="label"
          tick={AXIS_TICK}
          tickLine={false}
          axisLine={{ stroke: "rgba(255,255,255,0.2)" }}
          minTickGap={48}
        />
        <YAxis
          tick={AXIS_TICK}
          tickLine={false}
          axisLine={false}
          width={52}
          tickFormatter={(v: number) => `${Math.round(v)}`}
        />
        <Tooltip content={<ChartTooltip unit={unit} />} />
        <Legend
          iconType="plainline"
          wrapperStyle={{ fontSize: 12, paddingTop: 4 }}
        />
        {names.map((name, i) => (
          <Area
            key={name}
            type="monotone"
            dataKey={name}
            stroke={PALETTE[i % PALETTE.length]}
            strokeWidth={2}
            fill={`url(#grad-${unit}-${name})`}
            dot={false}
            activeDot={{ r: 3.5, strokeWidth: 0 }}
            isAnimationActive={false}
          />
        ))}
      </AreaChart>
    </ResponsiveContainer>
  );
}

function ChartTooltip({
  active,
  payload,
  label,
  unit,
}: {
  active?: boolean;
  payload?: Array<{ name?: string; value?: number; color?: string }>;
  label?: string;
  unit: string;
}) {
  if (!active || !payload || payload.length === 0) return null;
  return (
    <div
      style={{
        background: "rgba(21,21,21,0.96)",
        border: "1px solid rgba(255,255,255,0.18)",
        borderRadius: 6,
        padding: "8px 10px",
        fontSize: 12,
        boxShadow: "0 4px 12px rgba(0,0,0,0.4)",
      }}
    >
      <div style={{ opacity: 0.7, marginBottom: 4 }}>{label}</div>
      {payload.map((entry) => (
        <div key={entry.name} style={{ display: "flex", alignItems: "center", gap: 6 }}>
          <span
            style={{
              width: 9,
              height: 9,
              borderRadius: 2,
              background: entry.color,
              display: "inline-block",
            }}
          />
          <span>{entry.name}</span>
          <strong style={{ marginLeft: "auto", paddingLeft: 12 }}>
            {(entry.value ?? 0).toFixed(1)} {unit}
          </strong>
        </div>
      ))}
    </div>
  );
}

export default function Dashboard() {
  const [history, setHistory] = useState<MetricsHistory | null>(null);
  const [error, setError] = useState<string | null>(null);
  const timerRef = useRef<ReturnType<typeof setInterval> | null>(null);

  const load = useCallback(() => {
    api
      .metricsHistory()
      .then((h) => setHistory(h))
      .catch((e) => setError(e.message));
  }, []);

  useEffect(() => {
    load();
    timerRef.current = setInterval(load, POLL_MS);
    return () => {
      if (timerRef.current) clearInterval(timerRef.current);
    };
  }, [load]);

  const { cpuSeries, memSeries, netSeries, latest } = useMemo(() => {
    if (!history || history.points.length === 0)
      return { cpuSeries: [], memSeries: [], netSeries: [], latest: null };
    const s = buildSeries(history);
    return {
      cpuSeries: cpuUtilization(s.cpuTotals),
      memSeries: s.mem,
      netSeries: s.netRates.slice(1),
      latest: history.points[history.points.length - 1],
    };
  }, [history]);

  const totals = useMemo(() => {
    let mem = 0;
    let rx = 0;
    let tx = 0;
    let procs = 0;
    if (latest) {
      for (const s of Object.values(latest.instances)) {
        mem += s.memoryUsed ?? 0;
        rx += s.netRx ?? 0;
        tx += s.netTx ?? 0;
        procs += s.processes ?? 0;
      }
    }
    const lastNet = netSeries[netSeries.length - 1];
    return {
      count: latest ? Object.keys(latest.instances).length : 0,
      memoryUsed: mem,
      rxBps: lastNet?.values.received ?? 0,
      txBps: lastNet?.values.transmitted ?? 0,
      processes: Math.round(procs),
      hasData: !!latest,
    };
  }, [latest, netSeries]);

  return (
    <PageSection hasBodyWrapper={false}>
      <Title headingLevel="h1" size="2xl">
        Dashboard
      </Title>
      <Title headingLevel="h2" size="md" className="pf-v6-u-text-color-subtle">
        Live workload metrics · sampled every 5s · last 20 minutes
      </Title>

      {error && (
        <Alert variant="danger" title="Could not load metrics" isInline className="pf-v6-u-mt-md">
          {error}
        </Alert>
      )}

      <Gallery hasGutter className="pf-v6-u-mt-lg" maxWidths={{ default: "340px" }}>
        <Card>
          <CardBody>
            <DescriptionList isCompact isHorizontal>
              <DescriptionListGroup>
                <DescriptionListTerm>Instances</DescriptionListTerm>
                <DescriptionListDescription>{totals.count}</DescriptionListDescription>
                <DescriptionListTerm>Total processes</DescriptionListTerm>
                <DescriptionListDescription>
                  {totals.hasData ? totals.processes : "—"}
                </DescriptionListDescription>
              </DescriptionListGroup>
            </DescriptionList>
          </CardBody>
        </Card>
        <Card>
          <CardBody>
            <DescriptionList isCompact isHorizontal>
              <DescriptionListGroup>
                <DescriptionListTerm>Memory used</DescriptionListTerm>
                <DescriptionListDescription>
                  {totals.hasData ? formatBytes(totals.memoryUsed) : "—"}
                </DescriptionListDescription>
                <DescriptionListTerm>Network now</DescriptionListTerm>
                <DescriptionListDescription>
                  ↓ {formatBytes(totals.rxBps * 1024)}/s · ↑{" "}
                  {formatBytes(totals.txBps * 1024)}/s
                </DescriptionListDescription>
              </DescriptionListGroup>
            </DescriptionList>
          </CardBody>
        </Card>
      </Gallery>

      <Gallery hasGutter className="pf-v6-u-mt-lg">
        <Card>
          <CardTitle>CPU usage by instance (%)</CardTitle>
          <CardBody>
            <TimeChart series={cpuSeries} unit="%" />
          </CardBody>
        </Card>
        <Card>
          <CardTitle>Memory usage by instance (MiB)</CardTitle>
          <CardBody>
            <TimeChart series={memSeries} unit="MiB" />
          </CardBody>
        </Card>
        <Card>
          <CardTitle>Network throughput (KiB/s)</CardTitle>
          <CardBody>
            <TimeChart series={netSeries} unit="KiB/s" />
          </CardBody>
        </Card>
      </Gallery>
    </PageSection>
  );
}
