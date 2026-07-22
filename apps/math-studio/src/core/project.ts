import type { MathProject } from "./types";

export const PROJECT_SCHEMA_VERSION = 1 as const;

export function createId(prefix = "item"): string {
  return `${prefix}-${Date.now().toString(36)}-${Math.random().toString(36).slice(2, 8)}`;
}

export function cloneProject(project: MathProject): MathProject {
  return structuredClone(project);
}

export function serializeProject(project: MathProject): string {
  return JSON.stringify({ ...project, updatedAt: new Date().toISOString() }, null, 2);
}

export function parseProject(input: string): MathProject {
  const raw = JSON.parse(input) as Partial<MathProject>;
  if (raw.schemaVersion !== PROJECT_SCHEMA_VERSION) throw new Error("Unsupported project schema");
  if (!raw.formulas || !Array.isArray(raw.parameters) || !raw.timeline) throw new Error("Incomplete project document");
  return raw as MathProject;
}
