import { describe, expect, it } from "vitest";
import { keyboardStep } from "./keyboard-step";

describe("keyboardStep", () => {
  it("uses the declared increment without modifiers", () => {
    expect(keyboardStep(2, 0.01, 1, { shiftKey: false, altKey: false })).toBe(2.01);
  });

  it("uses Shift for coarse and Alt for fine changes", () => {
    expect(keyboardStep(2, 0.01, 1, { shiftKey: true, altKey: false })).toBe(2.1);
    expect(keyboardStep(2, 0.01, -1, { shiftKey: false, altKey: true })).toBe(1.999);
  });

  it("honors hard input bounds", () => {
    expect(keyboardStep(1, 1, 1, { shiftKey: true, altKey: false }, { max: 4 })).toBe(4);
  });
});
