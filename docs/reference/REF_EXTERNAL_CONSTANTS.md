# REF · External Comparison Constants — Canonical Standard

**Tag:** [REFERENCE]
**Date:** 2026-05-19
**Status:** Canonical single source for the *edition* and *sourcing discipline* of every externally-measured constant FTD compares against.
**Companion docs:** `REF_EXPERIMENTAL_STATUS.md` (experiment-tracking timeline — distinct purpose).

---

## §0 · Purpose

FTD's derived and conjectured values are compared against experiment. Those experimental values must be (a) drawn from one stated edition and (b) sourced from one canonical place, so that no document silently cites a superseded measurement. This document **is** that canonical place — for the *edition standard and sourcing discipline*. It does not replace the machine-readable constant modules (§4); it governs which edition they track.

A 2026-05-19 project-wide sweep found ~34 sites carrying the superseded **CODATA 2018** inverse fine-structure constant, ~9 of them *mislabelled* "CODATA 2022". This document is the standard that prevents recurrence.

## §1 · The standard

> **All current FTD references to externally-measured constants use CODATA 2022 (fundamental constants) and PDG 2024 (particle properties).**

- **CODATA 2022** is the current internationally-recommended set of fundamental physical constants (NIST). It supersedes CODATA 2018 and 2014.
- **PDG 2024** (Particle Data Group) is the current review for particle masses, widths, and mixing parameters.
- When a future edition supersedes these: update this document first, then the mirrors (§4), then run a consistency sweep.

## §2 · Discipline — comparison targets, not inputs

These values are used **only** for numerical audit and dimensional reconstruction. **They are not assumed in the pure mathematical spine.** The algebraic spine (G\*, the master quadratic, the FQCR structure) is independent of every measured value; experimental constants enter only where an FTD output is *checked against* or *calibrated to* experiment. A reference to a constant in this table is never a derivation input.

## §3 · Canonical values

The dimensional-reconstruction and α-bridge set. The full particle-mass catalogue lives in the machine-readable mirror (§4); this table is the authoritative *edition record* for the constants the framework most depends on.

| Constant | Symbol | Value | Std. uncertainty | Edition |
|---|---|---|---|---|
| Inverse fine-structure constant | α⁻¹ | 137.035999177 | ±0.000000021 (rel. ~1.5×10⁻¹⁰) | CODATA 2022 |
| Newton gravitational constant | G | 6.67430×10⁻¹¹ m³ kg⁻¹ s⁻² | ±0.00015×10⁻¹¹ | CODATA 2022 (unchanged from 2018) |
| Planck length | ℓ_P | 1.616255×10⁻³⁵ m | ±0.000018×10⁻³⁵ | CODATA 2022 (unchanged from 2018) |
| Planck time | t_P | 5.391247×10⁻⁴⁴ s | ±0.000060×10⁻⁴⁴ | CODATA 2022 |
| Electron mass | m_e | 0.51099895069 MeV/c² | ±0.00000000016 | CODATA 2022 |
| W boson mass | m_W | 80.3692 GeV | ±0.0133 | PDG 2024 |
| Z boson mass | m_Z | 91.1876 GeV | ±0.0021 | PDG 2024 |

**Edition-change notes.** α⁻¹ changed CODATA 2018 → 2022: `137.035999084` → `137.035999177` (Δ ≈ 9.3×10⁻¹⁰ relative — below every FTD precision claim, so no physics result changes; this is a citation-correctness standard, not a numerical revision). G and ℓ_P did **not** change between 2018 and 2022. m_W moved PDG 2022 → 2024 (`80.377` → `80.3692` GeV).

## §4 · Where the values live

| Layer | File | Role |
|---|---|---|
| Edition authority + discipline | **this document** | which edition; sourcing rule; provenance rule |
| Python (machine-readable) | `scripts/constants.py` → `class Experimental` | canonical Python mirror; Python comparison code imports from here |
| Self-contained verifier mirror | `scripts/proofs/common.py` | proof scripts are intentionally import-free for independent verifiability; carries an edition-matched copy |
| Web | `engine/web/js/constants.js` | canonical web-dashboard mirror |
| C++ tests | `engine/tests/*` (literals) | the C++ engine keeps no external-constants header; test files carry edition-matched literals |

**Rule:** a mirror's value must match this document's edition. If a mirror and this document disagree, **this document's edition is authoritative** and the mirror is corrected. To change an edition, edit this document first.

## §5 · Citation rule for future references

Any file comparing an FTD value to experiment must either:

1. **Import** the value — `from constants import Experimental` (Python), the `*_PHYS` exports (web); or
2. If the file is **self-contained by design** (proof scripts, scan harnesses), copy the value **with an inline edition tag that matches the value** — e.g. `# CODATA 2022` / `# PDG 2024`.

The single most common defect found in the 2026-05-19 sweep was the **compound mislabel**: a CODATA 2018 value tagged "CODATA 2022". An edition tag is only useful if checked against the digits — CODATA 2018 α⁻¹ ends `…084`, CODATA 2022 ends `…177`.

## §6 · Provenance exemption — do not retrofit registration-time values

**Pre-registered and hash-locked artifacts legitimately retain whatever value was current when they were registered.** That is correct provenance, not drift, and such files must **not** be retrofitted to a newer edition — doing so would invalidate the pre-registration.

Provenance-protected artifacts known to carry the CODATA 2018 α⁻¹ by design:

- `tools/scan_look_elsewhere.py` — FTD-0097 look-elsewhere scan (hash-locked; the one pre-registered scan among these files per `../theory/10_eft_program/REF_PREREGISTER_MANIFEST.md`).
- `docs/theory/07_assessment/PROTOCOL_LOOK_ELSEWHERE_SCAN.md` — the locked scan specification.
- `docs/theory/10_eft_program/archive/closed_negative/PREREG_PHASE_I_NATIVE_COUPLING.md` and any other `PREREG_*` document.
- `scripts/proofs/proof_polynomial_look_elsewhere*.py` — pre-registered polynomial-scan executors.
- `scripts/proofs/proof_phase_i_native_coupling.py` — executor of the PREREG above (its α⁻¹ literal is explicitly "committed BEFORE measurement per pre-reg §2.3").
- Dated review snapshots and generated scan artifacts (`engine/results/.../scan_result.json`).

For these the only admissible hygiene fix is the *comment label* (so a "2018" value is not mislabelled "2022"), and even that should be confirmed against the hash scope before being applied. Membership of this list is verified against `REF_PREREGISTER_MANIFEST.md` — a "scan" in a filename is **not** sufficient: `tools/scan_tower_level.py` and the tower cluster have no manifest row and were retrofitted.

## §7 · Retrofit status (2026-05-19 / 2026-05-20 sweep)

The α⁻¹ `137.035999084` → `137.035999177` retrofit. The numerical delta is below every FTD precision claim; this is citation hygiene, not a numerical revision.

**Complete — 23 files / 28 α⁻¹ occurrences retrofitted to CODATA 2022:**
- 3 canonical docs — `SPEC_DOCTRINE_LEDGER.md` §5, `SPEC_PHYSICS_BRIDGE.md` §2.1, `TRACKER_ONTIC_TRUTH.md` OT-5.1.
- 11 plain derivation / exploration / benchmark files (first pass, 2026-05-19).
- 9 files verified against `REF_PREREGISTER_MANIFEST.md` as **not** pre-registered (second pass, 2026-05-20): `PAPER_A_PI_FREE_GENERATOR.tex` (its prose wrongly stated "the CODATA 2022 value is 137.035999084"), the tower cluster (`THEOREM_HARMONIC_INVARIANT_TOWER.md`, `PROTOCOL_TOWER_LEVEL_FALSIFIER.md`, `explore_tower_level_scan.py`, `proof_harmonic_invariant_tower.py`, `proof_tower_multiplier_uniqueness.py`, `tools/scan_tower_level.py`), `proof_volumetric_master_quadratic.py`, `proof_chowla_selberg_higher_h_scan.py`.

Also fixed: the m_W stale value (`scripts/constants.py`, `engine/web/js/constants.js`: PDG 2022 → PDG 2024); a 1000× error in the `scripts/constants.py` α⁻¹ uncertainty comment ("153 ppb" was an absolute-vs-relative confusion — the relative uncertainty is ~0.15 ppb); the `engine/include/ftd/wilson_dirac.h` electron-mass placeholder digit; the `dimensional_map.json` electron-mass 9th-digit `ftd_value` (`SPEC_DIMENSIONAL_MAP.md` regenerated by `build_dimensional_map.py`). `scripts/constants.py` and `engine/web/js/constants.js` already carried the correct CODATA 2022 α⁻¹.

**Correct as-is — 13 files retain `137.035999084`; not drift, do not change.**
- Pre-registered / hash-locked (§6): the FTD-0097 scan, `PROTOCOL_LOOK_ELSEWHERE_SCAN.md`, the `PREREG_PHASE_I` doc + its executor, `proof_polynomial_look_elsewhere.py` — the value was committed before measurement; retrofitting would break the pre-registration.
- Explicitly "CODATA 2018"-labelled historical references (`FOUND_BLIND_DERIVATION_CHAIN.md`, `DERIV_MASTER_QUADRATIC_CM_LVALUES.md`, `EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md`) — value matches label; `EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md` describes a 2018-value scan and stays in sync with it.
- Docs that quote the old value to document the change: this document.

---

*Created 2026-05-19 as the canonical edition standard, closing a recurring CODATA-edition drift first enumerated 2026-04-19.*
