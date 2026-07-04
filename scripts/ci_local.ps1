# =============================================================================
# ci_local.ps1 — one-command local engine merge gate (revision 1.1).
#
# Chains, failing fast on the first red step:
#   1. Engine build            (Release, 32-way)
#   2. ctest -L merge_gate     (goldens + phase-order/lifecycle/determinism)
#   3. Playwright parity lint  (scenario-parity.spec.js — JS<->C++ scenario +
#                               toggle drift guards; no WASM load needed)
#   4. [-Wasm]  optional        engine\build_wasm.bat (only when bindings or
#                               headers crossing the WASM boundary changed)
#   5. [-Full]  optional        full ctest suite instead of merge_gate only
#
# Usage (from repo root):
#   powershell -File scripts\ci_local.ps1            # fast gate (~2 min)
#   powershell -File scripts\ci_local.ps1 -Wasm      # + WASM triple build
#   powershell -File scripts\ci_local.ps1 -Full      # full 211+ suite
#
# See engine/docs/CI_GATE.md for what the gate does and does not cover
# (GPU parity runs on the WSL2 build: scripts/ci_local.sh --gpu).
# =============================================================================
param(
    [switch]$Wasm,
    [switch]$Full
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repo 'engine\build'

function Step($name, $script) {
    Write-Host "`n=== [ci_local] $name ===" -ForegroundColor Cyan
    & $script
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ci_local] FAILED at: $name (exit $LASTEXITCODE)" -ForegroundColor Red
        exit 1
    }
}

Step 'build engine (Release)' {
    cmake --build $buildDir --config Release --parallel 32
}

if ($Full) {
    Step 'full ctest suite' {
        Push-Location $buildDir
        try { ctest -j 32 -C Release } finally { Pop-Location }
    }
} else {
    Step 'merge_gate ctest bundle' {
        Push-Location $buildDir
        try { ctest -L merge_gate -j 32 -C Release --output-on-failure } finally { Pop-Location }
    }
}

Step 'Playwright scenario/toggle parity lint' {
    Push-Location (Join-Path $repo 'engine\web\tests')
    try { npx playwright test scenario-parity.spec.js --reporter=line } finally { Pop-Location }
}

if ($Wasm) {
    Step 'WASM triple build + staged deploy' {
        & (Join-Path $repo 'engine\build_wasm.bat')
    }
}

Write-Host "`n[ci_local] ALL GREEN" -ForegroundColor Green
exit 0
