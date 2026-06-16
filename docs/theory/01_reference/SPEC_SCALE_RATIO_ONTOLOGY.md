# SPEC_SCALE_RATIO_ONTOLOGY.md

**Title:** Scale-Ratio Ontology — Framework Commitment **FC-3**
**Status:** `[AXIOM]`-class Framework Commitment (a declaration, **not** a derivation) — **FTD-0304** (registered 2026-06-15)
**Depends on:** `SPEC_FTD_FRAMEWORK_V1.md` (the constitution, FTD-0254), `SPEC_SCALE_CONTEXT_READOUT.md`
**Precedence:** `LEDGER > constitution > this SPEC` (per the 5.46 conflict-precedence rule)

---

## 0. Executive statement

FTD's multi-scale ontology — *"not many worlds, many scales"* (`scale.h`) — is sharpened
into a declared commitment: **every phenomenon operates at its own discrete scale, and only
dimensionless ratios are physical.** Absolute lengths — the lattice spacing `a` (UV cutoff) and
the simulation box `L` (IR cutoff) — are **observation scales of the apparatus**, not properties
of the thing observed.

The load-bearing consequence is a split that earlier work conflated:

- **Identity** (*is this a phenomenon at all?*) is fixed by a phenomenon's **internal** scale
  hierarchy and is **box-independent**.
- **Observability** (*can I measure it cleanly in this apparatus?*) is apparatus-relative
  (`R/a`, `R/L`).

`SPEC_SCALE_CONTEXT_READOUT.md`'s admissibility gate measured an *absolute* extent `R_eff` and
compared it to the box (`ζ = R_eff/L`). The canonical A=14 Koopman cloud then came out
`R_eff ∝ L`, `ζ ≈ 0.50` at every `L` (that doc, §5.4) — read as "percolating." Under FC-3 the
correct reading is sharper and box-independent: a uniform thermal field has **no internal scale
hierarchy** — it is scale-free vacuum, so it is **not a phenomenon**, independent of how it sits
in any box. This SPEC states the ontology; the gate becomes one *application* of it.

---

## 1. The commitment (FC-3)

> **FC-3 [AXIOM-class commitment].** Physical content in FTD is **scale-ratio-covariant**:
> every phenomenon carries an intrinsic scale, and an FTD observable's physical content depends
> only on **dimensionless ratios** of scales — never on an absolute lattice or box length. The
> lattice spacing `a` and box size `L` are properties of the *observation*, not of the *thing*.

FC-3 is a **declaration, not a theorem** — it occupies the constitution's **Framework
Commitments** register (three-register structure: frozen Postulates P1–P5 / Framework
Commitments / Calibrations, per `SPEC_FTD_FRAMEWORK_V1.md`). It stands alongside:

| Commitment | Picks | Source |
| :-- | :-- | :-- |
| FC-0 | the ℤ[i] reading | FTD-0249 |
| FC-1 | the commutative observable algebra is complete (declines the measurement map M) | FTD-0255 |
| FC-2 | the arrow is native; the Lorentzian metric is emergent-IR; space ⊥ time | FTD-0256 |
| **FC-3** | **scale-ratio-covariance: only internal ratios are physical; `a`, `L` are observation scales** | **FTD-0304** |

The theorem proves the *fork*; the commitment *picks the branch*. Motivation (the
consistency case, not a proof): dimensionless ratios are the renormalization-group / effective-
field-theory invariants, and FTD's discrete substrate has no a-priori absolute length once
`a ≡ ℓ_P` is itself a calibration (`SPEC_DIMENSIONAL_MAP.md`). FC-3 makes that stance explicit
and **killable** (§7).

---

## 2. Identity vs Observability

| | **Identity** (admissibility) | **Observability** (measurability) |
| :-- | :-- | :-- |
| Question | Is this a phenomenon with its own scale? | Can I read it cleanly in *this* apparatus? |
| Depends on | internal ratios only | `a`, `L` (apparatus) |
| Box-independent? | **yes** | no |
| Failure means | not a phenomenon (scale-free vacuum) | a real phenomenon, poorly resolved here |

A thing does not stop being a phenomenon because the box is too small to see it cleanly; that is
an *observability* limit, recorded separately and never used to deny identity.

---

## 3. The `ScaleContext` object

A phenomenon instantiates a scale context from **three internal features**, all intrinsic to the
thing (no `a`, no `L`):

| Feature | Meaning |
| :-- | :-- |
| `R` | intrinsic extent (the phenomenon's characteristic size) |
| `ξ` | coherence length (the scale over which the field is internally correlated) |
| `δ` | shell thickness (active-boundary / falloff width) |

It reports the **two identity ratios**:

- **χ = ξ / R** — *coherence*. A phenomenon is internally **correlated across its extent** —
  `ξ` not much smaller than `R`, i.e. `χ ≥ χ_min` (a floor); thermal vacuum is incoherent
  (`ξ ~ a ≪ R ⇒ χ → 0`).
- **β = δ / R** — *concentration*. A phenomenon has a distinct **core→edge hierarchy**
  (`β` below a ceiling); a uniform fill has no such structure.

**Identity verdict:** a thing *has its own scale* (is a phenomenon) **iff** it is coherent
**and** concentrated:

```
is_phenomenon  ⟺  (χ ≥ χ_min)  ∧  (β ≤ β_max)        [IMPOSED bands; §6, §7]
```

Both axes are required: a coherent-but-spread blob (`χ` high, `β` high) and a peaked-but-noisy
speckle (`β` low, `χ` low) are each correctly excluded.

---

## 4. The three orthogonal readout axes

Identity is **necessary but not sufficient** for a public physical readout (e.g. α). Readout
requires three independent gates:

1. **Identity** — is a phenomenon (§3: coherence + concentration). *Box-independent.*
2. **Observability** — resolved above the grid and fitting the box: `κ = R/a` not too small,
   `ζ = R/L` not too large. *Apparatus-relative; a caveat on the measurement.*
3. **Stability** — dynamically self-bound: the self-confinement flux-balance fixed point
   `Φ_out(R*) = Φ_ret(R*)`, `dΦ/dR < 0` (`SPEC_SCALE_CONTEXT_READOUT §3`).

The self-confinement and box machinery already built are **not discarded** — they are demoted
from "what makes it a phenomenon" to axes 2 and 3. This is also the honest, layered reason the
A=14 cloud fails: it is not even **admissible** (incoherent vacuum, axis 1), before observability
or stability are even reached.

---

## 5. Subsumption map (deferred retrofit — shown, not built here)

FC-3 unifies three existing notions of scale as **instances** of one `ScaleContext`:

| Instance | Source of `R` (intrinsic) | `ξ`, `δ` | Status |
| :-- | :-- | :-- | :-- |
| Atom (`AtomicClosureContext`) | declared: `R_BOHR·n²/Z_eff` | `δ_valence`, `ξ_orbital` | exists; wrap to emit `ScaleContext` |
| Flux cloud (`scale_context.cpp` gate) | measured: energy-weighted `R_eff` | flux autocorr `ξ`, radial `δ` | exists; reshape to internal-ratio identity |
| Any ontic entity (`scale.h` `OnticEntity.boundary`) | the entity's `boundary` field | per-level | exists; expose as `ScaleContext` |

The retrofits (rewiring these three to the shared object, and replacing the gate's box-relative
classifier with the identity criterion) are a **later implementation arc**, not this pass.

---

## 6. Minimal reference implementation (the only code built now)

A pure, header-only, dependency-free value object — `engine/include/ftd/scale_ratio.h`:

```cpp
namespace ftd {
struct ScaleRatio {                 // a phenomenon's internal scale features
    double R     = 0.0;             // intrinsic extent
    double xi    = 0.0;             // coherence length
    double delta = 0.0;             // shell thickness
    double chi()  const { return R > 0.0 ? xi    / R : 0.0; }  // coherence ratio
    double beta() const { return R > 0.0 ? delta / R : 0.0; }  // concentration ratio
};
struct ScaleRatioBands {            // [IMPOSED] identity bands (calibration deferred)
    double chi_min  = 0.5;          // coherence floor
    double beta_max = 0.6;          // concentration ceiling
};
inline bool is_phenomenon(const ScaleRatio& s, const ScaleRatioBands& b) {
    return s.chi() >= b.chi_min && s.beta() <= b.beta_max;
}
struct Observability { double kappa = 0.0; double zeta = 0.0; };  // R/a, R/L
inline Observability observe(const ScaleRatio& s, double a, double L) {
    return { a > 0.0 ? s.R / a : 0.0, L > 0.0 ? s.R / L : 0.0 };
}
}  // namespace ftd
```

Unit test (`engine/tests/test_scale_ratio.cpp`): a coherent concentrated blob → `is_phenomenon`
true; incoherent thermal speckle (`χ` low) and uniform fill (`β` high) → false; an
`AtomicClosureContext`-style instance (R,ξ,δ from the atom formulae) → true; `observe()` returns
the expected `κ,ζ` and **does not** affect `is_phenomenon`. No engine wiring, no gate rewrite.

The `chi_min = 0.5`, `beta_max = 0.6` values are `[IMPOSED]` placeholders; empirical calibration
against a known-good instance is a deferred step (and must not be tuned to admit a specific
trajectory — same discipline as `SPEC_SCALE_CONTEXT_READOUT §5.4`).

---

## 7. Epistemic accounting & falsification

- The ratio definitions `χ = ξ/R`, `β = δ/R` are `[DEFINITION]`.
- The identity bands `χ_min`, `β_max` are `[IMPOSED engineering defaults]`, calibration deferred.
- FC-3 itself is an `[AXIOM]`-class Framework Commitment (a declared choice), **FTD-0304**.
- **No tag moves.** `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays
  `[FOUNDATIONAL OBSTRUCTION]`. FC-3 **reframes** the readout obstruction (an *identity* failure,
  not merely a box artifact) and unblocks **nothing by fiat** — no α is derived here.

**Framework-level falsification criteria (constitution §6.2 style).** FC-3 is killed if any of:

1. A derived FTD observable is shown to depend **irreducibly on an absolute** `a` or `L` — a
   physical prediction that changes with the lattice/box in a way no dimensionless ratio captures.
2. The identity criterion **misclassifies an uncontroversially physical phenomenon** as vacuum
   (coherence + concentration both fail for a thing that is, by independent grounds, a real
   bounded object) — i.e., no box-independent internal criterion can separate phenomena from
   vacuum.
3. A **stable, observable readout** is demonstrated from a state that is *not* scale-separated
   (a genuinely box-filling state yields a reproducible, box-invariant physical constant),
   contradicting the claim that only internal ratios carry physical content.

---

## 8. Owner actions (status)

- ~~Register FC-3 in `SPEC_FTD_FRAMEWORK_V1.md` (the constitution) and add the **FTD-0304** LEDGER row.~~ **DONE — 2026-06-15.**
- ~~Minimal reference implementation (`engine/include/ftd/scale_ratio.h` + `engine/tests/test_scale_ratio.cpp`, 23 assertions, NO_CORE CMake target).~~ **DONE — commit `29d234e0`, 2026-06-15.**
- Retrofit the three instances of §5 to the shared `ScaleContext` (separate implementation arc).
- Calibrate the `[IMPOSED]` identity bands against a known-good instance.
