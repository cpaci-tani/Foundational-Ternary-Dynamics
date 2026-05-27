## 2026-05-27T05:32:11Z

You are a versatile worker tasked with fixing a critical entrypoint integration omission.

### Objective
Update the script entrypoint reference in `engine/web/index.html` to point to the renamed central application module `js/app.js`, resolving a 404 Not Found error and allowing Playwright tests to execute successfully.

### Detailed Steps & Specifications
1. **Locate the Omission**:
   - Open `engine/web/index.html` and find line 216 (or search for `<script type="module" src="js/app_dag.js?v=19"></script>`).
2. **Apply the Fix**:
   - Change the module source from `js/app_dag.js?v=19` to `js/app.js?v=19`.
3. **Verify via Tests**:
   - Navigate to `engine/web/tests/` and run the scenario invariants test suite using Playwright:
     ```powershell
     npx playwright test audit-regression.spec.js
     ```
   - Verify that the web server resolves the request cleanly (no 404), the dashboard loads, and all scenario invariants pass.

### Output Requirements
- Write a clear handoff report (`handoff.md`) inside your working directory `c:\Users\cpaci\Desktop\ftd\.agents\worker_entrypoint_fix\`.
- Your report must include:
  - The diff showing the modification made to `index.html`.
  - The exact command and output of the successful Playwright test execution.
- Use `send_message` to report back to your parent orchestrator (conversation ID: f229133c-6e4a-4636-b17f-0746768f4ab4) when you are done, with the path to your handoff.md.

### MANDATORY INTEGRITY WARNING
DO NOT CHEAT. All implementations must be genuine. DO NOT hardcode test results, create dummy/facade implementations, or circumvent the intended task. A Forensic Auditor will independently verify your work. Integrity violations WILL be detected and your work WILL be rejected.

Your identity is 'worker_entrypoint_fix'.
Your working directory is 'c:\Users\cpaci\Desktop\ftd\.agents\worker_entrypoint_fix'.
You are authorized to edit index.html and run Playwright test commands.
