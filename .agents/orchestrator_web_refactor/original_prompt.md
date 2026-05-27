# Original User Request

## 2026-05-26T23:04:16-05:00

You are the Project Orchestrator for the FTD Web Dashboard Refactoring mission.

Your working directory is: c:\Users\cpaci\Desktop\ftd\.agents\orchestrator_web_refactor
You must follow all FTD project instructions in CLAUDE.md, AGENTS.md, and docs/SPEC_FTD.md.

### Mission
Refactor the FTD web dashboard codebase to ensure exceptional modularity, DRY compliance, clear lifecycle management, and zero memory/computation leaks (both in JS heap and WebGL context).

### Guidelines & Working Style
1. Maintain three core tracking files in your folder:
   - `plan.md` (milestones, decomposition, and roadmap)
   - `progress.md` (active work, task log, completion status)
   - `context.md` (notes, technical references, state)
2. Use the subagent catalog (e.g. `teamwork_preview_explorer` or `worker` or `reviewer` / `self`) to delegate tasks. Ensure each subagent gets its own isolated directory under `.agents/` (e.g., `.agents/explorer_web_refactor/`, `.agents/worker_web_refactor/`, etc.). Do NOT write code directly in this orchestrator turn, but orchestrate, decompose, and verify.
3. Coordinate the specialists to:
   - Perform an exploratory sweep of the JS files in `engine/web/js/` to find modular duplication, components without proper lifecycle methods (`mount`, `update`, `destroy`), and WebGL resources without explicit `.dispose()` calls.
   - Design a clean, unified lifecycle interface.
   - Refactor UI modules, view controllers, and renderers (field-renderer, flux-renderer, topology-sheet-renderer) to adhere to the component lifecycle and dispose of all Three.js geometries, materials, textures, render targets, and global event listeners.
   - Run the Playwright test suite (`npx playwright test` inside `engine/web/tests/` or via appropriate scripts) to ensure no regressions and 146/146 test completion.
4. When all milestones are 100% complete and verified, write a final handoff report (handoff.md) in your directory and send a completion message to the Sentinel.

Please start by initializing your tracking files, analyzing the codebase, and preparing your master plan.
