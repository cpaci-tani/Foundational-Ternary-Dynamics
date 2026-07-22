export function keyboardStep(
  current: number,
  step: number,
  direction: 1 | -1,
  modifiers: { shiftKey: boolean; altKey: boolean },
  bounds: { min?: number; max?: number } = {}
): number {
  const multiplier = modifiers.shiftKey && !modifiers.altKey ? 10 : modifiers.altKey && !modifiers.shiftKey ? 0.1 : 1;
  const next = current + direction * step * multiplier;
  return Number(Math.max(bounds.min ?? -Infinity, Math.min(bounds.max ?? Infinity, next)).toPrecision(12));
}
