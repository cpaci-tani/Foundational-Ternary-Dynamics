import { useCallback, useEffect, useRef } from "react";
import type { CurveAnalysis, CurveSample, Camera2DState, CurveStyle, PedagogySettings, Vec3 } from "../core/types";

interface Props {
  sample: CurveSample;
  style: CurveStyle;
  camera: Camera2DState;
  phase: number;
  analysis: CurveAnalysis | null;
  pedagogy: PedagogySettings;
  canvasRef: React.RefObject<HTMLCanvasElement>;
  fitNonce: number;
  onCameraChange: (camera: Camera2DState) => void;
}

export function Canvas2DViewport({ sample, style, camera, phase, analysis, pedagogy, canvasRef, fitNonce, onCameraChange }: Props) {
  const hostRef = useRef<HTMLDivElement>(null);
  const dragRef = useRef<{ x: number; y: number; center: [number, number] } | null>(null);
  const appliedFitRef = useRef(0);

  const draw = useCallback(() => {
    const canvas = canvasRef.current;
    const host = hostRef.current;
    if (!canvas || !host) return;
    const dpr = Math.min(window.devicePixelRatio || 1, 2);
    const width = Math.max(1, host.clientWidth);
    const height = Math.max(1, host.clientHeight);
    if (canvas.width !== Math.floor(width * dpr) || canvas.height !== Math.floor(height * dpr)) {
      canvas.width = Math.floor(width * dpr);
      canvas.height = Math.floor(height * dpr);
    }
    const context = canvas.getContext("2d");
    if (!context) return;
    context.setTransform(dpr, 0, 0, dpr, 0, 0);
    context.clearRect(0, 0, width, height);
    context.fillStyle = "#08100e";
    context.fillRect(0, 0, width, height);

    const radius = Math.max(sample.bounds.radius, 1e-3);
    const scale = 0.42 * Math.min(width, height) / radius * camera.zoom;
    const cx = width / 2 - camera.center[0] * scale;
    const cy = height / 2 + camera.center[1] * scale;
    if (style.showGrid) {
      const unit = scale;
      const gridStep = unit < 32 ? Math.ceil(32 / unit) : 1;
      context.strokeStyle = "rgba(112, 141, 132, 0.16)";
      context.lineWidth = 1;
      context.beginPath();
      for (let x = Math.floor((-cx / unit) / gridStep) * gridStep; x * unit + cx < width; x += gridStep) {
        context.moveTo(cx + x * unit, 0);
        context.lineTo(cx + x * unit, height);
      }
      for (let y = Math.floor(((cy - height) / unit) / gridStep) * gridStep; cy - y * unit > 0; y += gridStep) {
        context.moveTo(0, cy - y * unit);
        context.lineTo(width, cy - y * unit);
      }
      context.stroke();
    }
    if (style.showAxes) {
      context.strokeStyle = "rgba(203, 223, 216, 0.42)";
      context.beginPath();
      context.moveTo(0, cy);
      context.lineTo(width, cy);
      context.moveTo(cx, 0);
      context.lineTo(cx, height);
      context.stroke();
    }
    if (!sample.count) return;
    const gradient = context.createLinearGradient(0, height, width, 0);
    gradient.addColorStop(0, style.color);
    gradient.addColorStop(1, style.secondaryColor);
    context.strokeStyle = gradient;
    context.lineWidth = style.lineWidth;
    context.lineCap = "round";
    context.lineJoin = "round";
    context.beginPath();
    const visibleCount = style.drawScene ? Math.max(1, Math.floor(phase * (sample.count - 1)) + 1) : sample.count;
    for (let index = 0; index < visibleCount; index += 1) {
      const x = cx + sample.positions[index * 3] * scale;
      const y = cy - sample.positions[index * 3 + 1] * scale;
      if (index === 0) context.moveTo(x, y);
      else context.lineTo(x, y);
    }
    context.stroke();
    const markerIndex = style.drawScene ? visibleCount - 1 : Math.min(sample.count - 1, Math.floor(phase * (sample.count - 1)));
    const mx = cx + sample.positions[markerIndex * 3] * scale;
    const my = cy - sample.positions[markerIndex * 3 + 1] * scale;
    if (pedagogy.pointMarker) {
      context.fillStyle = "#f4f8f6";
      context.beginPath();
      context.arc(mx, my, style.pointSize, 0, Math.PI * 2);
      context.fill();
    }

    if (analysis) {
      const worldLength = sample.bounds.radius * 0.3 * pedagogy.vectorScale;
      const toCanvas = (point: Vec3): [number, number] => [cx + point[0] * scale, cy - point[1] * scale];
      const drawVector = (vector: Vec3, color: string, label: string) => {
        const start = toCanvas(analysis.position);
        const end = toCanvas([
          analysis.position[0] + vector[0] * worldLength,
          analysis.position[1] + vector[1] * worldLength,
          analysis.position[2] + vector[2] * worldLength
        ]);
        const angle = Math.atan2(end[1] - start[1], end[0] - start[0]);
        context.strokeStyle = color;
        context.fillStyle = color;
        context.lineWidth = 1.5;
        context.beginPath();
        context.moveTo(...start);
        context.lineTo(...end);
        context.stroke();
        context.beginPath();
        context.moveTo(...end);
        context.lineTo(end[0] - 8 * Math.cos(angle - 0.4), end[1] - 8 * Math.sin(angle - 0.4));
        context.lineTo(end[0] - 8 * Math.cos(angle + 0.4), end[1] - 8 * Math.sin(angle + 0.4));
        context.closePath();
        context.fill();
        context.font = "12px STIX Two Math, serif";
        context.fillText(label, end[0] + 5, end[1] - 5);
      };
      const position = toCanvas(analysis.position);
      const origin = toCanvas([0, 0, 0]);
      if (pedagogy.radiusVector) {
        context.save();
        context.strokeStyle = "rgba(244, 248, 246, 0.68)";
        context.lineWidth = 1;
        context.beginPath();
        context.moveTo(...origin);
        context.lineTo(...position);
        context.stroke();
        context.fillStyle = "#f4f8f6";
        context.font = "12px STIX Two Math, serif";
        context.fillText("r", (origin[0] + position[0]) / 2 + 4, (origin[1] + position[1]) / 2 - 4);
        context.restore();
      }
      if (pedagogy.projections) {
        context.save();
        context.setLineDash([4, 4]);
        context.strokeStyle = "rgba(100, 216, 200, 0.55)";
        context.lineWidth = 1;
        context.beginPath();
        context.moveTo(position[0], position[1]);
        context.lineTo(position[0], cy);
        context.moveTo(position[0], position[1]);
        context.lineTo(cx, position[1]);
        context.stroke();
        context.restore();
      }
      if (pedagogy.parameterAngle) {
        const theta = Math.atan2(analysis.position[1], analysis.position[0]);
        const angleRadius = sample.bounds.radius * 0.18 * scale;
        context.save();
        context.strokeStyle = "#64d8c8";
        context.lineWidth = 1.25;
        context.beginPath();
        context.arc(origin[0], origin[1], angleRadius, 0, -theta, theta > 0);
        context.stroke();
        context.fillStyle = "#64d8c8";
        context.font = "12px STIX Two Math, serif";
        context.fillText("θ", origin[0] + Math.cos(theta / 2) * (angleRadius + 8), origin[1] - Math.sin(theta / 2) * (angleRadius + 8));
        context.restore();
      }
      if (pedagogy.arcLengthLabel) {
        context.fillStyle = "#e7b453";
        context.font = "12px STIX Two Math, serif";
        const labelX = position[0] > width - 100 ? position[0] - 92 : position[0] + 9;
        context.fillText(`s = ${analysis.arcLength.toFixed(4)}`, labelX, position[1] + 17);
      }
      if (pedagogy.osculatingCircle && analysis.osculatingRadius && analysis.osculatingRadius <= sample.bounds.radius * 4) {
        const radius = analysis.osculatingRadius;
        const center: Vec3 = [
          analysis.position[0] + analysis.normal[0] * radius,
          analysis.position[1] + analysis.normal[1] * radius,
          analysis.position[2] + analysis.normal[2] * radius
        ];
        context.strokeStyle = "rgba(231, 180, 83, 0.7)";
        context.lineWidth = 1;
        context.beginPath();
        for (let index = 0; index <= 96; index += 1) {
          const angle = index / 96 * Math.PI * 2;
          const point: Vec3 = [
            center[0] + radius * (-analysis.normal[0] * Math.cos(angle) + analysis.tangent[0] * Math.sin(angle)),
            center[1] + radius * (-analysis.normal[1] * Math.cos(angle) + analysis.tangent[1] * Math.sin(angle)),
            center[2] + radius * (-analysis.normal[2] * Math.cos(angle) + analysis.tangent[2] * Math.sin(angle))
          ];
          const projected = toCanvas(point);
          if (index === 0) context.moveTo(...projected); else context.lineTo(...projected);
        }
        context.stroke();
      }
      if (pedagogy.tangent) drawVector(analysis.tangent, "#e7b453", "T");
      if (pedagogy.normal) drawVector(analysis.normal, "#ef7d8d", "N");
      if (pedagogy.binormal) drawVector(analysis.binormal, "#70a8ff", "B");
    }
  }, [analysis, camera, canvasRef, pedagogy, phase, sample, style]);

  useEffect(() => {
    draw();
    const observer = new ResizeObserver(draw);
    if (hostRef.current) observer.observe(hostRef.current);
    return () => observer.disconnect();
  }, [draw]);

  useEffect(() => {
    if (!fitNonce || appliedFitRef.current === fitNonce) return;
    appliedFitRef.current = fitNonce;
    onCameraChange({ center: [sample.bounds.center[0], sample.bounds.center[1]], zoom: 1 });
  }, [fitNonce, onCameraChange, sample.bounds.center]);

  return (
    <div className="viewport-host" ref={hostRef}>
      <canvas
        ref={canvasRef}
        aria-label="Two-dimensional mathematical curve view"
        onPointerDown={(event) => {
          event.currentTarget.setPointerCapture(event.pointerId);
          dragRef.current = { x: event.clientX, y: event.clientY, center: [...camera.center] };
        }}
        onPointerMove={(event) => {
          if (!dragRef.current) return;
          const host = hostRef.current;
          if (!host) return;
          const scale = 0.42 * Math.min(host.clientWidth, host.clientHeight) / Math.max(sample.bounds.radius, 1e-3) * camera.zoom;
          onCameraChange({ ...camera, center: [
            dragRef.current.center[0] - (event.clientX - dragRef.current.x) / scale,
            dragRef.current.center[1] + (event.clientY - dragRef.current.y) / scale
          ] });
        }}
        onPointerUp={() => { dragRef.current = null; }}
        onPointerCancel={() => { dragRef.current = null; }}
        onWheel={(event) => {
          event.preventDefault();
          onCameraChange({ ...camera, zoom: Math.max(0.1, Math.min(20, camera.zoom * Math.exp(-event.deltaY * 0.001))) });
        }}
      />
      <div className="viewport-label">P<sub>xy</sub> · planar projection</div>
    </div>
  );
}
