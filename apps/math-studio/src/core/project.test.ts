import { describe, expect, it } from "vitest";
import { parseProject, serializeProject } from "./project";
import { createExperimentProject } from "../experiments/registry";

describe("project document", () => {
  it("round-trips schema version 1", () => {
    const project = createExperimentProject("parametric-workbench");
    expect(parseProject(serializeProject(project)).experimentId).toBe("parametric-workbench");
  });

  it("rejects unknown schemas", () => {
    expect(() => parseProject('{"schemaVersion":99}')).toThrow("Unsupported project schema");
  });
});
