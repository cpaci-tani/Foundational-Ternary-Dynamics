# 0015 — Engine merge gate: local + CI policy

**Status:** Accepted

## Context

The engine had no CI: deploy-pages published `engine/web/**` on every main
push with zero engine gating, and the golden gate + 211 CTests ran on local
discipline only. Committed WASM binaries could drift from source unnoticed.
(Engine revision program, tickets 0.3/1.1/1.2.)

## Decision

Three gate tiers, all built on the `merge_gate` CTest label (the pinned
goldens + phase-order/lifecycle/determinism, <2 min):
1. **Local fast**: `scripts/ci_local.ps1` (Windows: build → merge_gate →
   Playwright parity lint; `-Wasm`/`-Full` switches) and `scripts/ci_local.sh`
   (WSL2: gcc golden → merge_gate serial; `--gpu` for the RTX 5090 label set).
2. **CI**: `engine` job in `ci.yml` — windows-latest MSVC (the hash-canonical
   platform), CPU-only configure, merge_gate + unit labels, helium_scale1
   excluded (FTD-0270 pre-existing). Soak mode (`continue-on-error: true`)
   until one week of green runs, then flip to required.
3. **Deploy gate**: `engine-gate` job in `deploy-pages.yml`; `deploy` has
   `needs: engine-gate`, so Pages cannot publish an engine regression.

## Consequences

- (+) Public deploys are engine-gated; per-platform hash policy enforced in CI
- (+) One command locally reproduces what CI runs
- (−) ~10 min windows runner per deploy; GPU labels remain local-only (WSL2)
- WSL2 ctest must run SERIAL (CUDA context-creation contention — measured in
  `scripts/ci_local.sh`)

## Alternatives considered

- Linux CI runner — rejected: golden hashes are pinned on MSVC `/fp:precise`;
  gcc agreement is verified locally on WSL2, not by hosted runners.
- workflow_run chaining — rejected: self-contained `engine-gate` job inside
  deploy-pages.yml keeps the path-filter trigger semantics.

## References

- Files: `.github/workflows/ci.yml`, `.github/workflows/deploy-pages.yml`,
  `scripts/ci_local.ps1`, `scripts/ci_local.sh`, `engine/docs/CI_GATE.md`
- Cross-refs: ADR-0012 (+2026-07-02 multi-profile amendment),
  `engine/docs/DESIGN_RNG_PORTABILITY.md` (per-platform hash policy)
