@echo off
setlocal
if not defined FTD_ENGINE_AUDIT_HARNESS (
    echo Set FTD_ENGINE_AUDIT_HARNESS to engine_audit_harness.py 1>&2
    exit /b 2
)
for %%A in ("%~dp0..\..\..\..\..") do set "FTD_REPO_ROOT=%%~fA"
for %%C in (init run-cpp run-physics run-web compile) do (
    python "%FTD_ENGINE_AUDIT_HARNESS%" --repo-root "%FTD_REPO_ROOT%" %%C || exit /b 1
)
