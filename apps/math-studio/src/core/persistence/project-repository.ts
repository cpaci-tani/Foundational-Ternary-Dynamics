import type { MathProject } from "../types";

export interface ProjectRepository {
  load(): MathProject | null;
  save(project: MathProject): void;
  clear(): void;
}

export class BrowserProjectRepository implements ProjectRepository {
  constructor(private readonly key = "math-studio.project.v1") {}

  load(): MathProject | null {
    try {
      const value = localStorage.getItem(this.key);
      return value ? JSON.parse(value) as MathProject : null;
    } catch {
      return null;
    }
  }

  save(project: MathProject): void {
    try {
      localStorage.setItem(this.key, JSON.stringify(project));
    } catch {
      // File import/export remains available when storage is denied.
    }
  }

  clear(): void {
    try {
      localStorage.removeItem(this.key);
    } catch {
      // Clearing unavailable storage is a no-op.
    }
  }
}

export const projectRepository: ProjectRepository = new BrowserProjectRepository();
