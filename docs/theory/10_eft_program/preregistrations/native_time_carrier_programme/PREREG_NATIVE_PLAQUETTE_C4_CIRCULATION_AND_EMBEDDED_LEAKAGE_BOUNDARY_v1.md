# FTD-0918 — Native plaquette `C4` circulation and embedded-leakage boundary v1

**Identifier:** `FTD-0918`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact projection of the existing production flux/wave action; no
new production term and no result-corpus read

## 1. Question

Does the existing site flux `J` and conjugate wave register `W` already
supply the antisymmetric circulation/conjugate momentum missing from the
stationary production plaquettes in FTD-0915? If the charge exists as a native
observable, is it conserved on one embedded plaquette by the current free-wave
generator, and does that generator execute the exact ternary quarter-turn?

This protocol distinguishes **native representability**, **conditional
isolated conservation**, and **embedded production closure**. It is an exact
finite algebra certificate, not a numerical search or a new engine campaign.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `engine/include/ftd/lagrangian.h` | `0225C75F34D1154CDF3783E73A86F051A3868E0E9087606E117411D75429350F` |
| `THEOREM_FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_AND_PRODUCTION_BOUNDARY_v1.md` | `656F51A4E5A533C0436E932B452A33810CD851D63E571621DF81ECB0C9BED622` |
| `THEOREM_NATIVE_TERNARY_PLAQUETTE_QUARTER_TURN_RECURSION_v1.md` | `DC98BB8E8A0CF39E832F7399631F831FF71D3216ED104B6C76384EEEF9100B26` |
| `ANALYSIS_PRODUCTION_TERNARY_PLAQUETTE_RECURRENCE_CENSUS_v1.md` | `6DD72FC5666FB8AA649055B0C6F4224FBF4E50090D898641AC67865C527E20F3` |

The certificate fails closed on any source drift.

## 3. Frozen `C4` projection

For any scalar or vector-valued site field `F=(F_0,F_1,F_2,F_3)` on the
right-handed plaquette order of FTD-0915, define its first harmonic by

\[
q_F=\frac{F_0-F_2}{2},\qquad
r_F=\frac{F_1-F_3}{2}.
\]

Apply this to production flux and wave velocity:

\[
(q,r)=(q_J,r_J),\qquad
(p_q,p_r)=(q_W,r_W).
\]

The candidate circulation charge is

\[
\mathcal L_P=q\!\cdot p_r-r\!\cdot p_q.
\]

No complex field is added. Writing `z=q+i r` is notation for this native real
doublet only.

## 4. Frozen exact claims

The certificate must prove all of the following.

1. The forward site shift induces `(q,r)->(-r,q)` and squares to `-I` on the
   first-harmonic doublet; reverse shift induces its inverse.
2. `L_P` is invariant under the forward/reverse `C4` action, odd under a
   plaquette reflection, and odd under canonical time reversal `W->-W`.
3. Every real symmetric `2 x 2` operator commuting with the quarter-turn
   matrix is a scalar multiple of the identity. Thus an invariant `C4`
   doublet of a symmetric quadratic generator is isotropic.
4. For the isolated isotropic kick--drift map

   \[
   p_q^+=p_q-h\kappa q,quad
   p_r^+=p_r-h\kappa r,quad
   q^+=q+h p_q^+,quad r^+=r+h p_r^+,
   \]

   `L_P` is conserved exactly for arbitrary `h` and `kappa`.
5. With arbitrary projected impulses `(u_q,u_r)` added to the kick, the exact
   balance law is

   \[
   \mathcal L_P^+-\mathcal L_P
   =q\!\cdot u_r-r\!\cdot u_q.
   \]

6. A common post-drift momentum damping `rho` and additive impulse/noise
   `(eta_q,eta_r)` give

   \[
   \mathcal L_{P,\mathrm{end}}
   =\rho\left(\mathcal L_P+q\!\cdot u_r-r\!\cdot u_q\right)
   +q^+\!\cdot\eta_r-r^+\!\cdot\eta_q.
   \]

   Nonuniform genesis drains are outside this common-scalar conservation law.
7. The internal four-vertex block of the production 18-point Laplacian has
   self weight `-4`, adjacent plaquette weights `1/3`, and opposite weight
   `1/6`. On `(q,r,-q,-r)` it is exactly `-(25/6)` times the identity in the
   `(q,r)` doublet.
8. With the registered `C_WAVE^2=1/3` and unit production step, the isolated
   internal doublet has `kappa=25/18`. Its kick--drift matrix has determinant
   one and trace `11/18`, so `cos(theta)=11/36`.
9. The isolated internal map is not an exact one-tick quarter-turn because
   `25/18 != 2`. It has no exact finite integer return: if `theta/pi` were
   rational, rational-cosine classification would require
   `cos(theta)` in `{0, +/-1/2, +/-1}`, contrary to `11/36`.
10. The four-site doublet is not an invariant production subspace. For the
    embedded `xy` pattern `(q,0,-q,0)`, the exterior site immediately beyond
    the positive vertex receives exact Laplacian value `q/3` on the first
    free-wave kick.
11. Consequently the local projected charge has an exterior/source torque
    ledger but no current production conservation theorem. Global or isolated
    modal conservation cannot be relabeled as bounded local-clock evidence.
12. For the selected isolated Hamiltonian

    \[
    H=\frac12(\|p_q\|^2+\|p_r\|^2)
      +\frac\kappa2(\|q\|^2+\|r\|^2),
    \]

    the circular branches `p_q=-sigma sqrt(kappa) r`,
    `p_r=sigma sqrt(kappa) q` have equal kinetic/potential energy and
    `L_P=sigma sqrt(kappa)(|q|^2+|r|^2)`. Inert/standing branches have zero net
    chiral charge.
13. `i` fixes the orientation operator but not `kappa`, an initial nonzero
    charge, a defect barrier, or a coupling magnitude. No `gamma` follows from
    `i`.
14. `G*`, Born weights, Bell settings, measurement context, selector state,
    and desired outcomes are absent. No production source is changed.

## 5. Outcome map

- **Outcome A — native observable with embedded-conservation boundary:** all
  claims pass. Book `L_P` as an exact native observable and an exact conserved
  charge only for an invariant isotropic doublet. Book the elementary
  embedded plaquette as leaky/non-invariant under the current free-wave
  generator and its exact state quarter-turn as dynamically unmatched.
- **Outcome B — representation survives but balance/leakage claim fails:**
  retain only the exact `C4` projection and symmetry parities; issue no
  conservation or production boundary.
- **Outcome C — invalid:** any source lock or exact claim fails.

Outcome A does not license a new engine term. The next admissible work is to
derive a bounded invariant doublet, a source-balanced closed circulation, or
a theorem-grade no-go from the existing full action. A newly imposed isolated
ring is only a reference model and must be priced as such.

```text
NATIVE_TYPES=FLUX_AND_WAVE_VELOCITY
C4_FIRST_HARMONIC=Q_AND_R
CANDIDATE_CHARGE=L_P
ISOLATED_ISOTROPIC_CONSERVATION=CONDITIONAL
EMBEDDED_ELEMENTARY_PLAQUETTE_INVARIANT=FALSE
ONE_TICK_EXACT_QUARTER_TURN=FALSE
FINITE_INTEGER_RETURN=FALSE_FOR_BARE_INTERNAL_MAP
PRODUCTION_CHANGED=FALSE
GSTAR_READ=FALSE
GAMMA_DERIVED=FALSE
BORN_BELL_CONTEXT_READ=FALSE
STATUS=LOCKED_PRE_CERTIFICATE
```

**LOCKED CONTENT ENDS HERE.**
