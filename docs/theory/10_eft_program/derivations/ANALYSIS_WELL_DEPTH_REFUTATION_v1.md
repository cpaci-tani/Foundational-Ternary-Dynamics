# FTD-0790 — FTD-0788 Refuted: the Lattice Quantum is a Coordinate Coincidence v1

**Status:** `[REFUTATION — FTD-0788 WITHDRAWN]` +
`[EXACT — TRIVIAL: z*G(1) IS IDENTICALLY z*G(0) - 1]` +
`[COORDINATE COINCIDENCE — INADMISSIBLE UNDER THE FTD-0388 PRECEDENT]` +
`[CORRECTION — MY MISLABELING CLAIM AGAINST THE ENGINE HEADER WAS WRONG]`
**Verdict:** `EPS_ORIGIN_UNRESOLVED_IDENTIFICATION_IS_A_COORDINATE_COINCIDENCE`
**Parents:** `FTD-0388`, `FTD-0788` (refuted here), `FTD-0789`
**Production impact:** none

## 1. The refutation, in one line

`z*G(1) = z*G(0) - 1` **identically, by the definition of the discrete
Laplacian's source term** — so FTD-0788's entire apparatus (Watson integral,
Glasser–Zucker Γ-form, nearest-neighbour Green's function) reduces to the map
`K -> K - 1` applied to `K_GENESIS`, and carries **no lattice information
whatsoever**. The identity holds for any lattice, any coordination number, any
`G(0)`.

## 2. The chain is selection on imposition on convention

FTD-0788 claimed `K_GENESIS = z*G(0)` as registered exact structure. It is not:

- **`K_MANIFEST := W_SC` is `[SELECTION — ADOPTED, owner ruling 2026-07-17]`**
  (LEDGER FTD-0388). Only the *value given the identification* is theorem-grade.
- **The `N_c` factor is `[IMPOSED]`** (`SPEC_ENERGY_SCALES_AND_DETECTABILITY.md`:
  "fill all `N_c` colour channels `[IMPOSED]`"). No derivation exists in `docs/`.
- **The remaining factor 2 is a declared convention.** `PREREG_SELFENERGY_
  PINNING_v1.md` states three live conventions give three different constants
  and the result is meaningful only with one declared; under its own
  `E_term6` row (`(4/3)*W_SC`), `K_GENESIS` would be `2.0219` and the match
  evaporates.

So the "coordination number `z = 6`" is really `N_c (=3, IMPOSED) x 2
(convention)`. Re-reading that product as the simple-cubic coordination number
is numerically identical and structurally a different object. Worse, `N_c`
enters `K_GENESIS` through a *genesis-specific* argument — filling colour
channels to **create a particle** — with **no analogue for a pair bond**.

**And the factor does all the work.** Since `Omega(A, eps) = sqrt(eps) *
Omega(A, 1)` exactly, C2 is a pure one-sided threshold `eps > 0.0944` (field
band). The bare nearest-neighbour Green's function **fails**:

| candidate | value | field band | wave band |
|---|---|---|---|
| `G(1)` | 0.086064 | **FAIL** | **FAIL** |
| `2*G(1)` | 0.172129 | pass | **FAIL** |
| `3*G(1)` | 0.258193 | pass | pass (marginal) |
| `6*G(1) = W-1` | 0.516386 | pass | pass |

Everything FTD-0788 claimed rests on the multiplicative 6, which is not
derived.

## 3. Dimensional category error, twice

1. **The subtracted `1` is a source charge, not an energy.** `z[G(0)-G(1)] =
   delta_0 = 1` is a unit *charge*; `eps` is a potential depth. FTD-0788
   subtracts a charge from a threshold with no stated conversion.
2. **`K_GENESIS` is not an energy.** Confirmed in source
   (`transmutation_phases.cpp`): `if (jmag <= K_GENESIS) continue;` — it gates
   a **flux amplitude**. The corpus itself uses the square as the energy scale
   (`K_MANIFEST^2 = 0.255492`; "local field-energy density at activation is
   `>= K_GENESIS^2`"). The dimensionally eligible comparison object is
   `K_GENESIS^2 = 2.2994`, not `1.5164` — and `K_MANIFEST`, the only
   dimensionally eligible quantity, is a **field** self-energy, which §3 of
   FTD-0788 had just excluded.

## 4. The internal contradiction

FTD-0788 §3 argued `eps` must be a *matter-sector* scale, "not `alpha`", to
exclude the field-mediated candidate. §4 then took `eps` from `G`, **the
Gauss/Poisson propagator** — FTD-0388's registered meaning for the adopted
constant is verbatim "the substrate's unit-charge **Gauss self-energy**." The
document excludes the field sector and then derives its answer from that
sector's propagator at coincident points.

Claim (b) fails on both legs independently:

- **Compact support does not imply "not field-mediated."** Integrating out a
  field with an ultralocal quadratic term coupled to finite-range source form
  factors yields an induced matter–matter potential equal to the convolution
  of the form factors — **strictly compactly supported**. The premise holds
  only for a *massless, unscreened* mediator, which begs the question since
  the framework's own candidate mediator is `[OPEN]`. Self-undermining, too:
  FTD-0788 §6 names the decisive problem as "derive the compact law **from
  the native force**" — if that succeeds, the law *is* force-mediated.
- **Separate bookkeeping is accounting, not ontology.** And it is circular
  here: the pair term reads `-0.00956` *because `eps = 0.01` is hardcoded in
  `V`*. Using it as evidence about `eps`'s origin begs the question.

## 5. Look-elsewhere: the target is a half-line

Because `Omega ~ sqrt(eps)` exactly, "satisfies C2" carries **exactly one
bit**: `eps > 0.0944`, with no upper bound. A restricted grammar over
`{W, G(0), G(1), N_c, z, C_WAVE^2, 1, 2}` puts **34 values in [0.22, 1.6] and
~26 above the C2 threshold**. A match against a half-line with ~26 competitors
is not evidence. FTD-0788's 5-row contamination table does not discriminate:
those entries differ by 2% *because the `eps` values differ by 2%*.

Symmetrically, §2's "`eps = 0.01` is an orphan" is an artifact of a four-item
candidate list — the same grammar yields ~38 expressions within a factor of 2
of `0.01`, several within 6%. **Orphanhood is not evidence for any adoptive
parent.**

Note this is *worsened*, not helped, by FTD-0789's finding that C5 does not
tighten with `eps`: C5 was the only candidate for an upper bound that could
have made the target a finite interval.

## 6. FTD-0388's own guard forbids exactly this move

LEDGER FTD-0388, verbatim: *"the `W_SC ≈ 0.511` value-match numerology remains
**inadmissible** (this adopts an identification, not a coincidence); **an
adoption is never a derivation**."* The prereg records the value-match form as
ruled inadmissible under the `[COORDINATE COINCIDENCE]` class
(`AUDIT_MASS_CHAIN_REDTEAM` Axis B precedent). FTD-0788 took that adopted
identification, treated it as derived exact structure, and value-matched a
second quantity to it — **precisely the pattern the parent pre-emptively ruled
out** — without citing the guard.

## 7. CORRECTION: my mislabeling claim against the engine header was wrong

I stated that `particle_masses.h:60,64` mislabels its constants (that
`K_MANIFEST := W_SC` is wrong because `W_SC = 1.5164`). **That was my error.**
The repo's registered convention is `W_SC = 0.5054620197`, stated identically
in LEDGER FTD-0388, `PREREG_SELFENERGY_PINNING_v1.md`, `SPEC_FTD.md`, and
`tooltips/definitions.js`. The header is consistent with four independent
registered sources. **FTD-0788 silently switched to the literature convention
(`W_Watson = 1.516386`) and then reported the repo's correct usage as a
defect.** No engine comment needs fixing; this document does.

Unambiguous relations, to avoid repeating the collision:

```text
W_Watson (literature) = 1.51638605915197801816 = 6*G(0)
repo W_SC             = W_Watson/3 = 2*G(0)   = 0.50546201971732600000
G(0) = 0.25273100985866300271     G(1) = 0.08606434319199633604
K_MANIFEST = repo W_SC = 2*G(0)   K_GENESIS = 3*K_MANIFEST = 6*G(0) = W_Watson
```

The "residual 1.8e-17" FTD-0788 quoted as an independent match is the
double-precision round-off of a by-construction equality — circular.

## 8. What survives

**The three lattice numbers, and only those.** `G(0)`, `G(1)`, and the
identity `G(0) - G(1) = 1/6` were verified independently by Bessel-integral
representations (agreeing with the Γ-form to `1.9e-18`) and by direct
Brillouin-zone lattice sums converging as `1/L^2`. That arithmetic is correct.
Everything built on top of it is withdrawn.

**Correct tags for the withdrawn claim:** `[EXACT — TRIVIAL: z*G(1) is
identically z*G(0) - 1]` + `[COORDINATE COINCIDENCE — inadmissible under the
FTD-0388 precedent]`. FTD-0788's `[SELECTION — MOTIVATED]` was too strong and
its verdict line (`EPS_IS_A_LATTICE_QUANTUM_NOT_A_FREE_SELECTION`) asserted as
settled exactly what its own §7 disclaimed.

## 9. What would have to be true

Four things, all currently false: a derivation (not an analogy) outputting a
nearest-neighbour energy proportional to `G(1)` with a **derived** prefactor;
a justification for `N_c` in a two-body bond; dimensional closure between a
unit lattice charge, a `|J|` amplitude gate, and an energy; and a **two-sided**
C2 test, since a one-sided threshold cannot discriminate among ~26 candidates.

**Honest residue:** `eps = 0.01` has no derivation, `W_SC - 1` has no
derivation, and the question "what sets the well depth" is exactly where it
was before FTD-0788 — except that the look-elsewhere arithmetic in §5 now
makes clear how weak *any* value-match evidence in this range would be.
