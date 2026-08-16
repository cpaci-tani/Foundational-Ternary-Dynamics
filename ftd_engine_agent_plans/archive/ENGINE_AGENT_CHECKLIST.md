# Engine Agent Checklist

Use this before opening a PR.

## Pre-work

- [ ] Read `META_PROJECT_ATLAS.md`.
- [ ] Read `CLAUDE.md`.
- [ ] Read `docs/WHERE_WE_LEFT_OFF.md`.
- [ ] Confirm branch name.
- [ ] Confirm whether CUDA is enabled or disabled for the current build.
- [ ] Run baseline targeted tests if the build exists.

## Implementation hygiene

- [ ] Keep theorem modules header-only if possible.
- [ ] Add one test per module.
- [ ] Prefer `ftd_add_test`.
- [ ] Use `NO_CORE` if the test does not need `ftd_core`.
- [ ] Do not alter `RenderBridge` for Plans 01–04.
- [ ] Do not alter CUDA kernels for Plans 01–04.
- [ ] Do not alter WASM bindings for Plans 01–04.
- [ ] Avoid new dependencies.

## Epistemic hygiene

- [ ] Every new doc states status.
- [ ] Every new doc states non-claims.
- [ ] Flavor graph remains CANDIDATE RECONSTRUCTION.
- [ ] Z3 center closure does not claim full confinement.
- [ ] Branch holonomy theorem does not claim particle mass.

## Test gates

- [ ] `ctest -R branch_holonomy_gap`
- [ ] `ctest -R z3_color_center`
- [ ] `ctest -R generation_graph`
- [ ] `ctest -R render_bridge_golden`
- [ ] Full CTest if local runtime permits.
- [ ] Python tests if docs/scripts were changed.

## PR description template

```md
## Summary

Adds finite graph overlay modules for branch holonomy, Z3 color-center closure, and/or generation graph diagnostics.

## Status labels

- Branch holonomy gap: THEOREM
- Z3 center closure: THEOREM / CONDITIONAL THEOREM
- Generation graph: CANDIDATE RECONSTRUCTION

## Tests

- [ ] branch_holonomy_gap
- [ ] z3_color_center
- [ ] generation_graph
- [ ] render_bridge_golden

## Non-claims

- Does not modify production tick physics.
- Does not claim full QCD confinement.
- Does not claim theorem-forced CKM.
```
