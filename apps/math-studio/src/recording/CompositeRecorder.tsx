import { CircleStop, Video } from "lucide-react";
import { useCallback, useEffect, useRef, useState } from "react";
import { useStudioStore } from "../core/store/studio-store";

interface Props {
  source2d: React.RefObject<HTMLCanvasElement>;
  source3d: React.RefObject<HTMLCanvasElement>;
  equations: string[];
}

export function CompositeRecorder({ source2d, source3d, equations }: Props) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const recorderRef = useRef<MediaRecorder | null>(null);
  const chunksRef = useRef<Blob[]>([]);
  const stopTimerRef = useRef<number | null>(null);
  const [recording, setRecording] = useState(false);
  const project = useStudioStore((state) => state.project);
  const setPlaying = useStudioStore((state) => state.setPlaying);
  const setCurrentTime = useStudioStore((state) => state.setCurrentTime);

  const composite = useCallback(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const context = canvas.getContext("2d");
    if (!context) return;
    const width = canvas.width;
    const height = canvas.height;
    context.fillStyle = "#08100e";
    context.fillRect(0, 0, width, height);
    const sources = project.layout === "2d" ? [source2d.current] : project.layout === "3d" ? [source3d.current] : [source2d.current, source3d.current];
    const visible = sources.filter((source): source is HTMLCanvasElement => Boolean(source?.width && source?.height));
    const cellWidth = width / Math.max(1, visible.length);
    visible.forEach((source, index) => {
      const scale = Math.min(cellWidth / source.width, (height - 140) / source.height);
      const drawWidth = source.width * scale;
      const drawHeight = source.height * scale;
      context.drawImage(source, index * cellWidth + (cellWidth - drawWidth) / 2, 44 + (height - 184 - drawHeight) / 2, drawWidth, drawHeight);
    });
    context.fillStyle = "rgba(5, 12, 11, 0.92)";
    context.fillRect(0, 0, width, 44);
    context.fillRect(0, height - 140, width, 140);
    context.fillStyle = "#edf5f1";
    context.font = "600 17px system-ui";
    context.fillText(project.name.toUpperCase(), 22, 28);
    context.font = '26px "STIX Two Math", "Cambria Math", "Times New Roman", serif';
    const equationStart = height - equations.length * 30 - 10;
    equations.forEach((equation, index) => context.fillText(equation, 22, equationStart + index * 30));
  }, [equations, project.layout, project.name, source2d, source3d]);

  useEffect(() => {
    let frame = 0;
    const draw = () => {
      composite();
      frame = requestAnimationFrame(draw);
    };
    draw();
    return () => cancelAnimationFrame(frame);
  }, [composite]);

  const stop = useCallback(() => {
    if (stopTimerRef.current) window.clearTimeout(stopTimerRef.current);
    if (recorderRef.current?.state !== "inactive") recorderRef.current?.stop();
    setPlaying(false);
  }, [setPlaying]);

  const record = () => {
    const canvas = canvasRef.current;
    if (!canvas?.captureStream || typeof MediaRecorder === "undefined") return;
    const stream = canvas.captureStream(project.timeline.fps);
    const mime = ["video/webm;codecs=vp9", "video/webm;codecs=vp8", "video/webm"].find(MediaRecorder.isTypeSupported) ?? "video/webm";
    const recorder = new MediaRecorder(stream, { mimeType: mime });
    recorderRef.current = recorder;
    chunksRef.current = [];
    recorder.ondataavailable = (event) => { if (event.data.size) chunksRef.current.push(event.data); };
    recorder.onstop = () => {
      stream.getTracks().forEach((track) => track.stop());
      const blob = new Blob(chunksRef.current, { type: mime });
      const url = URL.createObjectURL(blob);
      const anchor = document.createElement("a");
      anchor.href = url;
      anchor.download = `${project.name.toLowerCase().replace(/[^a-z0-9]+/g, "-") || "math-animation"}.webm`;
      anchor.click();
      URL.revokeObjectURL(url);
      setRecording(false);
    };
    setCurrentTime(0);
    setPlaying(true);
    setRecording(true);
    recorder.start(250);
    stopTimerRef.current = window.setTimeout(stop, project.timeline.duration * 1000 + 120);
  };

  return (
    <section className="recorder-section" aria-label="Recording output">
      <div className="section-heading"><span>Program output</span><span>1920 × 1080 · WebM</span></div>
      <canvas ref={canvasRef} width={1920} height={1080} aria-label="Composited animation recording preview" />
      <button className={`command-button ${recording ? "danger" : ""}`} type="button" onClick={recording ? stop : record}>
        {recording ? <CircleStop size={16} /> : <Video size={16} />}
        {recording ? "Stop recording" : "Record timeline"}
      </button>
    </section>
  );
}
