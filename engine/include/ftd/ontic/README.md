# engine/include/ftd/ontic — Canonical 9-layer derivation chain

**Purpose.** The "everything from nothing" C++ derivation chain. Every
physics constant in the FTD framework derives from D=3 (spatial dimensions)
and varpi (lemniscate constant) through 9 layers. This directory holds the
canonical `constexpr` definitions; `engine/include/ftd/constants.h`
re-exports them into the `ftd::` namespace with `static_assert` guards.

## Public API

Headers in this directory expose `constexpr` constants in `namespace ftd::ontic`.
Consumers that want a flat `ftd::ALPHA` (etc.) include
`engine/include/ftd/constants.h` instead, which re-exports via `using` directives.

## Internal structure (layer order)

| Layer | Header | What it derives |
|---|---|---|
| -1 | `ontic.h` (umbrella) | EULER_E (self-referential seed) |
| 0 | `ontic.h` | EULER_GAMMA, GAMMA_QUARTER (transcendentals) |
| 0b | `ontic.h` | NOME_LEMNISCATIC, THETA_LEMNISCATIC (modular selection) |
| 1 | `lemniscate.h` | VARPI, GAUSS_CONSTANT_M, G_STAR, PI_FTD, packing fraction PF |
| 2b | `ontic.h` | K_CRIT (where i emerges), X_BORN (Born rule degenerate root) |
| 3 | `master_quadratic.h` | x_+, x_-, COEFFICIENT 16, alpha-derivation chain |
| 4 | `ontic.h` | Framework integers: D=3, N_c=3, N_gen=3, N_f=6, N_base=4, b_3=7, N_eff=13 |
| 5 | `gauge_couplings.h` | G_C (state-flux coupling), ALPHA, ALPHA_S_MZ, SIN2_WEINBERG, ALPHA_WEAK |
| 5b | `gauge_couplings.h` | QCD running, B0_NF5, B0_NF6, LAMBDA_QCD, M_Z |
| 6 | `particle_masses.h` | K_B (electron mass anchor 0.511 MeV), K_GENESIS, mass ratios, R_BOHR |
| 6b | `particle_masses.h` | LAMBDA_HIGGS, M_HIGGS, V_HIGGS, M_W |
| 7 | `neutrino.h` | Neutrino mass formula chain |
| 8 | `consciousness.h` | Scale 11 / observer formalism constants |

Plus: the umbrella `engine/include/ftd/ontic.h` (above this directory) imports
all sub-headers and provides the namespace.

## Dependencies

- **Imports from**: standard library only (`<cmath>`, `<cstdint>`)
- **Imported by**: `engine/include/ftd/constants.h` (re-export with guards), `ontic_audit.cpp` (self-audit)
- **No external dependencies** — pure compile-time constants

## Static_assert chain

Header guards in `engine/include/ftd/constants.h` (lines 128–132) ensure:
- `G_C * G_C ≈ ALPHA` (1e-8 tolerance)
- Master quadratic identities (`N_BASE = N_c² - N_c - 2`, etc.)

Extending these when adding new derived constants is required (see
ADR-0009 / CONTRACTS.md §7).

## How to extend

- **Add a new derived constant** → place at the right layer; update
  `constants.h` re-export; mirror to `engine/web/js/constants.js`
  (canonical JS) and `engine/include/ftd/constants_gpu.cuh` (device);
  optionally `scripts/constants.py` (canonical Python).
- **Change a layer's derivation** → write a derivation note in
  `docs/theory/03_derivations/DERIV_*.md`; cite from the new constant's
  comment block (`// Implements LEDGER#C-NNN [TAG]`).

## Invariants

- The 9 layers are strictly hierarchical — Layer 5 cannot reference
  Layer 6 (no upward references)
- Every numeric literal MUST be derived (not hardcoded) from earlier
  layers, OR explicitly tagged `[IMPOSED]` with rationale
- All constants are `constexpr` (compile-time)
- Consumers call by full namespace (`ftd::ontic::ALPHA`) or via the flat
  re-export in `ftd::` from `constants.h`

## Related docs

- [CONTRACTS.md §7](../../../../CONTRACTS.md#7--constants-chain-contract) (constants chain)
- [docs/adr/0009-epistemic-tag-system.md](../../../../docs/adr/0009-epistemic-tag-system.md)
- [docs/SPEC_FTD.md](../../../../docs/SPEC_FTD.md)
- [docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md](../../../../docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md) (canonical theorems-only reference)
- Mirrors: `scripts/constants.py`, `engine/web/js/constants.js`, `engine/include/ftd/constants_gpu.cuh`
