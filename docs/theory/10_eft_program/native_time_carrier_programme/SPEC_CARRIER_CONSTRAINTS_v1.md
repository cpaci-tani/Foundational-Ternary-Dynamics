# SPEC — Minimum Constraints on a Native G* Carrier v1

> **Scope clarification (2026-08-10):** “minimum” here means the full
> acceptance floor for a **native `G*` carrier campaign**, not the logical
> definition of any clock. A harmonic oscillator can carry phase and elapsed
> time even though `dOmega/dI=0`; it fails this document's quartic `G*` gate,
> not clockhood by definition. See
> [`SPEC_SUBSTRATE_NATIVE_CLOCK_MINIMUM_v1.md`](SPEC_SUBSTRATE_NATIVE_CLOCK_MINIMUM_v1.md)
> for the strict clock → local physical clock → `G*` clock hierarchy.

**Status:** `[SYNTHESIS — CONSOLIDATED CONSTRAINT SPEC]`; every constraint
cites a registered result, no new claims
**Purpose:** the checklist a campaign designer inherits instead of
re-deriving; any proposed carrier candidate must be scored against all twelve
before preregistration
**Parents:** `FTD-0772`, `FTD-0776`, `FTD-0778/0779/0780`, `FTD-0781`,
`FTD-0782`, `FTD-0783`, `FTD-0784`, `FTD-0800`, `FTD-0801`, sidebranch
§§29–32
**Registered:** 2026-08-03. Companion narrative: sidebranch
`RELATIVITY_CLOSURE_DERIVATION.md` §32.8.

A *carrier* is a native, persistent, recurrent dynamical object whose
occupancy statistics could exhibit `G*` as a measured output (Gate C). The
constraints divide into three tiers: what must exist, what must be read, and
what makes a finding count.

## Tier 1 — The dynamics (what must exist)

**C1. A nonlinear conservative sector** — the affine sector has zero
conservative anharmonicity (FTD-0781, source). Currently this means the
movement-enabled coupled matter–field sector, or an unregistered new native
mechanism.

**C2. Spectrum avoidance** — the fundamental and all harmonics must avoid the
acoustic band: `n*Omega` outside `[0, omega_B]` for all `n` (MacKay–Aubry
non-resonance, sidebranch §32.2). For the band topology `[0, omega_B]` this
forces `Omega > omega_B`, which for a quartic law means the amplitude floor
`A > A_c = G* omega_B / (2 sqrt(pi))`. Violation = radiation = envelope decay
= FTD-0772's measured failure mode.

**C3. An intermediate-exponent confining potential** — **still the binding
constraint, and still with no native realization.** (FTD-0787 briefly claimed
to realize it; **refuted by FTD-0789** — the claimed quartic was a rectilinear
chord across an exactly flat bend direction.) FTD-0789 supplies a decidable
criterion: for central-force networks at zero tension, `n = 2` iff rigid,
`n = infinity` iff the flex extends to a finite mechanism, and **`n = 4`
requires first-order flexibility with second-order rigidity** — `null(H)`
nonempty with the quartic form positive definite on it. Both registered
extremes fail it oppositely (the connected block is rigid, FTD-0637/0638; the
isolated trimer is a free mechanism). FTD-0800 then found no `n = 4` mechanism
in its actual screened set (38 sampled `N = 3..6` equilibria and SC blocks
through `L = 4`); the apparent SC quartic is affine/clamped and vanishes under
free relaxation. Its post-hoc `N = 6` graph-class follow-up is only an
`[EXPLORATORY NUMERICAL SCREEN]`: 51 accepted sampled embeddings contain no
stress-plus-flex hit, while 11 graph classes, other realization components and
rank strata, and the chosen separation floor remain unresolved. **There is no
`N <= 6` no-go.** FTD-0801 returns `N4_CLAMPED_ONLY` on the tested periodic
triangulated sheets: the decisive `cos(2q.x)` witness reaches exact zero after
cell release, while the `6x6` random residual remains unresolved. This supports
only that the resolved quartic witnesses found so far under the scoped zero-tension,
central-force, single-scale law are clamped; it is not a universal quartic
no-go. Statement of the constraint: null-flat bottom (`V''(0) = 0`) with `~q^4` growth
sustained over at least a decade of amplitude (`a >~ 8`). Finite-depth wells
fail twice — `a_max ~ O(1)` and separatrix softening — pinning at `n ~ 2`.
Hard walls harden but land at `n = infinity`, `G*`-free. This constraint
*implies* hardening `dOmega/dA > 0`, so hardening need not be listed
separately.

**C4. Localization with stress closure** — bounded support, integrated
stress balance *including* field terms (sidebranch §29 Def. 1, corrected
virial form). This is what makes it a body with invariant `M = E_0/c^2` —
and what makes it the gravitational source of the triple identity.

**C5. A drain-free amplitude window** — the field dressing must stay below
the genesis threshold, `abs(J) < K_GENESIS`, or every crossing bleeds energy
(FTD-0781's drain map). Combined with C2's floor this demands a **nonempty
window**: `A_c < A < A_drain`. Whether that window exists is itself a
checkable native condition nobody has evaluated.

**C6. Persistence** — lifetime times `Omega` much greater than 1 (many
completed cycles; FTD-0772's gate wanted at least 8, occupancy statistics
want far more), with stable return amplitude. Follows largely from C2 + C5
but is independently measurable.

## Tier 2 — The observable (what must be read)

**C7. Not a linear functional of the field** — any closing linear functional
is harmonic; any non-eigenprojection fails closure (sidebranch §32.1, exact).
Matter-position coordinates and nonlinear functionals are the only admissible
classes.

**C8. A natural coordinate: closure** — `Q'' = F(Q)` single-valued, constant
effective mass (Gate A). This is *the* master condition, because Theorem B's
occupancy machinery — `G*`'s only home — is defined **only** in a fixed
natural coordinate.

**C9. Recurrence in the coordinate** — values must be revisited; single-pass
records satisfy closure vacuously and are UNINFORMATIVE by construction (the
three-regime result, FTD-0778 as amended). Folds are licensed only under
dynamical equivariance (the fold-license rule, FTD-0779 addendum).

**C10. Fixed, state-independent support** — declared in advance; no
activity-defined index sets (the `q_active` lesson: a support that grows to
the lattice measures population, not phase).

## Tier 3 — The evidence (what makes a finding count)

**C11. Native licensing** — found in a profile where imposed/selected
phenomenology (color force, toy gravity, Lorentz branch) is not doing the
work (FTD-0782 governance); otherwise the result is an `[ENGINE FACT]` about
inserted potentials. **Corrected 2026-08-03 (FTD-0786):** this
constraint formerly added "and any coupled-sector result remains
`[ENGINE FACT]`-grade until the reciprocal-transaction problem is solved."
That problem is solved — FTD-0551's discrete-gradient transaction, built out
by FTD-0600–0739. The transaction is a **selection**, so coupled-sector
results are *selection-scoped* (declare the transaction), not blocked.

**C12. Preregistration with calibrated instruments** — locked channels,
look-elsewhere control, held-out seed, metrics validated against
known-answer synthetics (the v2 lesson: the v1 metric's verdict was
predetermined by the estimator), and telemetry of at least 20 ticks per
cycle.

## Standing score

Every registered candidate to date fails at least one Tier-1 constraint:
linear functionals fail C7 (exact); the affine sector fails C1 (FTD-0781);
the compact-law pair fails C3 on three grounds (FTD-0783); generic wells and
walls fail C3 by the bracket theorem. **C3 is the wall**: no identified
native mechanism produces the intermediate-exponent potential class.

**Updated 2026-08-03 (FTD-0786) — the movement sector is no longer
unopened, and it fails at C2.** The sector was built out (FTD-0551,
FTD-0600–0739) and the carrier question was asked there. The first internal
matter doublet sits *strictly inside* the propagating field band —
`Omega = 1.09116` against the one-axis maximum
`2 arcsin(1/sqrt 3) = 1.23096`, ratio **0.8864** — so C2 fails exactly
(FTD-0663), and it decays at `Gamma_E = 0.00653712`/tick (FTD-0676): 26.57
cycles per energy e-fold, enough to clear a raw 8-cycle recurrence count but
not to hold amplitude or support occupancy statistics. C2 cannot be repaired
by driving amplitude, because the doublet is a **linear normal mode of a
positive-definite Hessian** (`n = 2`, amplitude-independent frequency) — the
bracket theorem again. **C3 is the mechanism by which C2 could be satisfied.**

**Updated again 2026-08-03 — then REVERTED the same day.** FTD-0787 claimed
the last door was open; **FTD-0789 refuted it** and C3 is unrealized. What
survives is a sharper criterion (second-order rigidity, above) and the fact
that the two registered configurations bracket it from opposite sides. The
withdrawn claim follows for the record.

**[WITHDRAWN] FTD-0787: the last door was opened, and C3 is
realized.** The flexural mode of a collinear trimer has an exactly null-flat
quartic potential from the registered compact law with no new primitive, and
is the program's first hardening mode (`dOmega/dA = +1.0166`). It scores
**8/12** — the best of any candidate — but fails **C2**: bounded hardening
gives `Omega_max = 0.400745` against a band top of `1.230959`.

**The wall has therefore moved, and changed character.** It is no longer a
question of potential shape (solved) but of **energy scale**: with
`Omega ~ sqrt(eps/m)` and the purity window independent of `eps`, band
clearance inside the clean-quartic range requires `eps > 0.2218` (field band)
or `eps > 0.5856` (wave band), against the *selected* `eps = 0.01`. Since
`eps/C_WAVE^2 = 0.03`, matter binding sits two orders below field stiffness
and C2 asks for parity. **[WITHDRAWN — FTD-0788 was refuted by FTD-0790 the same day.** `z*G(1)` is
identically `z*G(0) - 1`, carrying no lattice content; `K_GENESIS` is
selection x imposition x convention and is a flux gate, not an energy; and
C2 is a one-sided threshold ~26 expressions clear. `eps` has no derivation
and the question stands exactly where it did. The withdrawn text follows.]**

**Resolved in part, same day, by FTD-0788.** `eps = 0.01` is an orphan —
17.8x above the field-mediated scale and 51.6x below the lattice
nearest-neighbour quantum. The compact law has *compact support*, so it is a
matter-sector contact term, not a field-mediated one; and the framework's
matter thresholds are already exact lattice Green's-function quantities
(`z G(0) = W_SC = K_GENESIS`, matched to `1.8e-17`). The companion identity
`z[G(0) - G(1)] = 1` gives the nearest-neighbour quantum
`z G(1) = W_SC - 1 = 0.5163860592`, and the compact law's minimum sits at
exactly `r0 = 1`. **Under that selection C2 is satisfied** — above the field
band over 24-100% of the separatrix with 4.0% quartic contamination at entry.

**The programme therefore reduces to one question:** is the compact law
field-mediated (`eps ~ 5.6e-4` — no carrier, ever, short by 400x) or a
matter-sector lattice quantum (`eps ~ 0.52` — carrier viable)? The two differ
by ~2700x. Deriving the compact law from the native force (FTD-0575 `[OPEN]`)
is now the single decisive item. Separately, FTD-0784 bounds the prize:
even a candidate passing all twelve delivers `G*` but cannot deliver the
FC-W surd; W stays external.

**Current C3 status (2026-08-04, FTD-0800/0801):** C3 remains unrealized.
The FTD-0788 energy-scale branch above is therefore conditional historical
provenance, not the current single decisive item: a free-body `n = 4`
mechanism must first be established. The live routes named by the scoped
screens are a two-scale interaction or pre-tension, both outside the
registered zero-tension, single-scale law and both still `[OPEN]`.
