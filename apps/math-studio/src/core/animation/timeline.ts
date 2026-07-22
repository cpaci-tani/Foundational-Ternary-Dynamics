import type { Camera2DState, Camera3DState, Keyframe, TimelineState } from "../types";

function smoothstep(value: number): number {
  return value * value * (3 - 2 * value);
}

export function evaluateTimeline(timeline: TimelineState, time: number, fallback: Record<string, number>): Record<string, number> {
  const frames = [...timeline.keyframes].sort((a, b) => a.time - b.time);
  if (!frames.length) return fallback;
  if (time <= frames[0].time) return { ...fallback, ...frames[0].parameters };
  if (time >= frames[frames.length - 1].time) return { ...fallback, ...frames[frames.length - 1].parameters };

  const rightIndex = frames.findIndex((frame) => frame.time >= time);
  const left = frames[rightIndex - 1];
  const right = frames[rightIndex];
  if (left.interpolation === "hold") return { ...fallback, ...left.parameters };
  let progress = (time - left.time) / Math.max(1e-9, right.time - left.time);
  if (left.interpolation === "smooth") progress = smoothstep(progress);
  const keys = new Set([...Object.keys(fallback), ...Object.keys(left.parameters), ...Object.keys(right.parameters)]);
  const values: Record<string, number> = {};
  keys.forEach((key) => {
    const a = left.parameters[key] ?? fallback[key] ?? 0;
    const b = right.parameters[key] ?? a;
    values[key] = a + (b - a) * progress;
  });
  return values;
}

export function sortKeyframes(keyframes: Keyframe[]): Keyframe[] {
  return [...keyframes].sort((a, b) => a.time - b.time);
}

export function evaluateCameraTimeline(
  timeline: TimelineState,
  time: number,
  fallback2d: Camera2DState,
  fallback3d: Camera3DState
): { camera2d: Camera2DState; camera3d: Camera3DState } {
  const frames = sortKeyframes(timeline.keyframes);
  if (!frames.length) return { camera2d: fallback2d, camera3d: fallback3d };
  if (time <= frames[0].time) return { camera2d: frames[0].camera2d, camera3d: frames[0].camera3d };
  if (time >= frames[frames.length - 1].time) {
    const last = frames[frames.length - 1];
    return { camera2d: last.camera2d, camera3d: last.camera3d };
  }
  const rightIndex = frames.findIndex((frame) => frame.time >= time);
  const left = frames[rightIndex - 1];
  const right = frames[rightIndex];
  if (left.interpolation === "hold") return { camera2d: left.camera2d, camera3d: left.camera3d };
  let progress = (time - left.time) / Math.max(1e-9, right.time - left.time);
  if (left.interpolation === "smooth") progress = smoothstep(progress);
  const mix = (a: number, b: number) => a + (b - a) * progress;
  return {
    camera2d: {
      center: [mix(left.camera2d.center[0], right.camera2d.center[0]), mix(left.camera2d.center[1], right.camera2d.center[1])],
      zoom: mix(left.camera2d.zoom, right.camera2d.zoom)
    },
    camera3d: {
      position: left.camera3d.position.map((value, index) => mix(value, right.camera3d.position[index])) as [number, number, number],
      target: left.camera3d.target.map((value, index) => mix(value, right.camera3d.target[index])) as [number, number, number],
      fov: mix(left.camera3d.fov, right.camera3d.fov)
    }
  };
}
