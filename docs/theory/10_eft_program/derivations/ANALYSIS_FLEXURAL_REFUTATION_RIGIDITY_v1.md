# FTD-0789 — FTD-0787 Refuted; C3 Restated as Second-Order Rigidity v1

**Status:** `[REFUTATION — FTD-0787's C3_REALIZED VERDICT WITHDRAWN]` +
`[THEOREM — MAXWELL TRICHOTOMY FOR CENTRAL-FORCE NETWORKS AT ZERO TENSION]` +
`[RESTORATION — FTD-0783's BRACKET COROLLARY STANDS]` +
`[OPEN — C3 UNREALIZED, NOW WITH A DECIDABLE CRITERION]`
**Verdict:** `FLEXURAL_QUARTIC_IS_A_CONSTRAINT_ARTIFACT_C3_REQUIRES_SECOND_ORDER_RIGIDITY`
**Parents:** `FTD-0783`, `FTD-0787` (refuted here), `FTD-0788`, `FTD-0637/0638`
**Production impact:** none

## 1. The refutation

FTD-0787 claimed the transverse displacement of a collinear trimer gives an
exactly null-flat quartic potential, hence C3 realized. **The algebra was
right and the physics was wrong.** Independently verified here.

The registered law is `U_N = sum_{a<b} A_ab V(q_ab)` with the polarity mask
`A_ab = (1 - sigma_a sigma_b)/2`. For a bonded trimer `A—B—C`, both bonds are
opposite-polarity, so `sigma_A = -sigma_B` and `sigma_C = -sigma_B`, hence
`sigma_A = sigma_C` and

```text
A_AC = (1 - sigma_A sigma_C)/2 = 0    identically, at ANY separation.
```

(FTD-0787 argued `A` and `C` do not interact because `q = 4` lies outside the
compact support — true, but weaker and beside the point.) Therefore

```text
U = V(|AB|^2) + V(|BC|^2)
```

**depends only on the two bond lengths.** The bend angle is an exact symmetry
direction. Verified to machine zero at every angle from 180° to 5°: the
energy never leaves `-2 eps` while the bond lengths stay at 1.

FTD-0787 displaced `B` transversely while holding `A` and `C` fixed in `x`,
which stretches both bonds to `sqrt(1 + d^2)`. **The system is free not to
make that stretch** — it reaches the same transverse offset by bending at
constant bond length, for zero energy. The claimed `24 eps d^4` is the cost of
a rectilinear chord drawn across an exactly flat valley, reported as the
valley's curvature.

Consequences: there is no quartic, no barrier, no separatrix, and no
frequency. `Omega_max`, `dOmega/dA`, the "first native hardening mode," and
the 8/12 scorecard all rest on the artifact. **C3 is not realized.**

## 2. Why, structurally: Maxwell counting

For a central-force network at zero tension the Hessian is a sum of `B`
rank-one longitudinal terms, so `rank(H) <= B` and

```text
#zero modes  >=  3N - B .
```

The trimer: `3N - B = 9 - 2 = 7`. Computed spectrum (9x9, numerical):

```text
eigenvalues: [0, 0, 0, 0, 0, 0, 0, 0.96, 2.88]
             \______ 7 zeros ______/   96 eps   3*96 eps
```

Five zeros are trivial (3 translations + 2 rotations of a linear molecule);
the remaining **two are the degenerate bends**. The trimer is *hypostatic*,
and its bend is a **finite mechanism** — not merely an infinitesimal flex — so
it is flat to all orders, not quartic.

## 3. The correct trichotomy (this is the salvage)

The failure sharpens rather than destroys the constraint. For central-force
networks at zero tension:

| network | soft directions | exponent |
|---|---|---|
| **rigid** (`B >= 3N-6`, positive-definite Hessian) | none | `n = 2` — harmonic |
| **first-order flexible, mechanism extends** (hypostatic, finite mechanism) | flat to all orders | `n = infinity` — free |
| **first-order flexible, SECOND-ORDER RIGID** | infinitesimal flex that does *not* extend to a finite motion | **`n = 4`** |

```text
C3 requires a native configuration that is FIRST-ORDER FLEXIBLE but
SECOND-ORDER RIGID: null(H) nonempty, yet the quartic form positive
definite on null(H).
```

This is a decidable criterion — compute the Hessian null space, then evaluate
the fourth-order term restricted to it — and it is exactly the classical
notion of second-order (prestress-free) rigidity.

**FTD-0783's bracket corollary is therefore RESTORED, not falsified.** Its
claim that no identified native mechanism produces an `n = 4` potential
stands; FTD-0787's "falsification by exhibition" is withdrawn. The trichotomy
above is the corollary's proper generalisation: rigid pins at `n = 2`,
mechanisms escape to `n = infinity`, and `n = 4` lives on the knife-edge
between them.

## 4. Where the mechanism does survive, and why it does not help

The FTD-0787 *mechanism* — transverse displacement against an untensioned
bond — is sound whenever the bond's far ends are **pinned**, because then the
bend is unavailable and the stretch is forced. A constituent `B` collinear
between two *fixed* neighbours has 3 DOF and 2 constraints, one doubly
degenerate transverse flex, and that flex is genuinely second-order rigid:
`V ~ d^4` exactly. So the `n = 4` class is **nonempty**.

But pinning changes the kinematics: `d = y_B` and `m_eff = m = 1` instead of
`2m/3`, giving

```text
Omega_max = 0.327207   vs   field band 1.230959      3.76x short
```

**worse** than FTD-0787's already-failing 3.07x, and FTD-0788's `eps`
threshold rises correspondingly. And the pinning must itself be native: the
neighbours must be held by their own bonds, which returns the question to
whether a *native* configuration is second-order rigid.

Note the two registered endpoints of the trichotomy are both already
occupied: the connected 16-constituent block has a **positive-definite**
48-coordinate Hessian (FTD-0637/0638), so it is rigid — `n = 2`, no quartic
direction anywhere in it — while the isolated trimer is a free mechanism,
`n = infinity`. Nothing registered sits between them.

## 5. Corrections to the record

1. **FTD-0787's verdict `C3_REALIZED_NATIVELY` is WITHDRAWN.** Its §3 algebra
   (`2V(1+d^2) = -2eps + 24 eps d^4 - 32 eps d^6`), its generic expansion in
   §2, its `m_eff = 2m/3` reduction *for the constrained path*, and all its
   numerics are correct and reproduce exactly; what fails is that the path is
   not dynamically realisable. Revised score: roughly 4–5/12, with C3, C8 and
   C9 all failing.
2. **FTD-0783's structural corollary is restored**; its §6 scope amendment is
   itself withdrawn.
3. **FTD-0787 §9 swapped the stretch-mode labels.** The eigenvector with `B`
   stationary (`-1, 0, +1`) at `Omega = 0.979796` is the **symmetric** stretch;
   the one with `B` opposed (`-1/2, +1, -1/2`) at `Omega = 1.697056` is the
   **antisymmetric**. Values correct, labels reversed.
4. **The causality objection raised against FTD-0787 does not land** at the
   registered `eps = 0.01`: peak `v_B = 0.13298` at the claimed `Omega_max`,
   i.e. `0.23 C_SPEED`. It *would* bind in FTD-0788's large-`eps` regime, where
   the above-band window past 56.6% of the separatrix is superluminal — that
   correction stands independently of this refutation.
5. **C5 (genesis drain) is comfortable and does not tighten with `eps`.**
   `|J|_matter <~ G_C * C_SPEED = 0.0493` against `K_GENESIS = 1.5164`, a 31x
   margin, because `|J|` is capped by the coupling and by causality, not by
   the well depth. FTD-0788's note to the contrary is withdrawn. `[ESTIMATE]`
   — engine confirmation not run.

## 6. What this leaves

C3 is unrealized, as it was before FTD-0787 — but the search is now a
**decidable question about a specific matrix condition** rather than an
open-ended hunt: *does any native FTD configuration have a nonempty Hessian
null space on which the quartic form is positive definite?* Both registered
extremes fail it in opposite directions. That is a sharper statement of the
wall than the programme had, and it is the only thing FTD-0787 legitimately
contributes.

**Standing lesson, sixth confirmation this session:** the arithmetic survives
adversarial audit; the physics attached to it does not. FTD-0787 was written,
registered, and propagated into four documents and the handoff package within
one session, on the strength of an exact symbolic identity that was never
checked against a relaxed configuration.
