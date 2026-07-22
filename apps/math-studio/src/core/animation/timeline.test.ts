import { describe, expect, it } from "vitest";
import { evaluateCameraTimeline, evaluateTimeline } from "./timeline";
import type { Keyframe, TimelineState } from "../types";

const camera2d = { center: [0, 0] as [number, number], zoom: 1 };
const camera3d = { position: [0, -5, 2] as [number, number, number], target: [0, 0, 0] as [number, number, number], fov: 45 };
const frame = (id: string, time: number, value: number): Keyframe => ({
  id,
  time,
  interpolation: "linear",
  parameters: { a: value },
  camera2d: { center: [value, 0], zoom: 1 + value },
  camera3d: { position: [value, -5, 2], target: [0, 0, 0], fov: 45 }
});

describe("timeline evaluation", () => {
  const timeline: TimelineState = { duration: 10, fps: 60, loop: false, keyframes: [frame("a", 0, 0), frame("b", 10, 10)] };

  it("interpolates parameter keyframes", () => {
    expect(evaluateTimeline(timeline, 5, { a: -1 }).a).toBeCloseTo(5);
  });

  it("interpolates 2D and 3D camera framing", () => {
    const cameras = evaluateCameraTimeline(timeline, 5, camera2d, camera3d);
    expect(cameras.camera2d.center[0]).toBeCloseTo(5);
    expect(cameras.camera3d.position[0]).toBeCloseTo(5);
  });
});
