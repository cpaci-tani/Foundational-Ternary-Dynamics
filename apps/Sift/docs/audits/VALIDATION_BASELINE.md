# Sift historical validation baseline

**Status:** Historical evidence captured before Script Studio and Hardware Monitor were added. This file preserves the observed 2026-07-14 run and must not be used as evidence for the current application. Current validation is produced by `scripts/validate.ps1` and `scripts/validate-feature-audit.ps1` under `artifacts/`.

## Run metadata

- Start UTC: `2026-07-14T20:13:47.8094411Z`
- Revision: `0317e10e70c203b1db81a88af990c0dfe57a4b16`
- Working directory: `C:\Users\cpaci\Desktop\ftd`
- Command: `powershell.exe -NoProfile -ExecutionPolicy Bypass -File "apps\Sift\scripts\validate.ps1"`
- OS: `Microsoft Windows 10.0.26200.8737`
- .NET SDK: `10.0.301`
- Exit code: `0`
- Elapsed time: `109.335` seconds
- Outcome: `PASS`
- First failure: none observed

## Observed validation evidence

A failed or not-reached gate is not a pass.

- Unit tests: Passed `68`, Failed `0`, Skipped `0`.
- Integration validation reported: `Sift validation passed: 35 tweaks, 7 preflight-reviewed, 218 processes sampled, WinUI-only source verified.`
- Release build: succeeded with `0` warnings and `0` errors.
- Native UI validation passed: Home, Optimize, Task Manager, Performance, Startup, Maintenance, Health, Recovery, Storage, Installed apps, System information, and Settings.
- Native UI final result: `Sift native UI validation passed`.
- `git diff --check`: returned success. It emitted line-ending warnings for unrelated files.
- Final validation line: `Sift validation completed successfully.`

## Raw evidence

The raw console evidence is the invoking Cursor terminal for this run. No
`apps/Sift/artifacts/validation-baseline.log` file was written, so this
baseline does not claim that an artifact log exists.
