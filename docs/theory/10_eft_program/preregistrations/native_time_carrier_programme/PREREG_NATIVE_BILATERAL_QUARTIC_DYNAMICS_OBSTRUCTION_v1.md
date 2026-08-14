# FTD-0837 — Native bilateral/quartic dynamics obstruction v1

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; CERTIFICATE INVALID 20/22]`  
**Date:** 2026-08-10  
**Scope:** exact source-locked audit of the frozen CPU production core, followed
by a conditional minimum-extension theorem  
**Production impact:** none

## 1. Registered question

Does the current production substrate already supply all three dynamical
ingredients left open by FTD-0836:

1. a continuous oriented exchange between the two substrate registers;
2. a smooth quartic restoring channel rather than a quadratic or threshold
   law; and
3. an energy-closed radial mechanism selecting a nonzero stable shell?

If it does not, what is the lowest-degree two-channel extension satisfying
those requirements, and which additional type is still required to identify
its self-dual energy coordinate with the physical quartic clock?

## 2. Epistemic and scope firewall

This is not a numerical search and has no adjustable tolerance. It audits the
frozen source algebra exactly. A negative result is limited to the current
production law and its explicit smooth fixed-state branches. It does not prove
that no coarse-grained, switching-induced, or future substrate dynamics could
ever generate a quartic effective law.

The protocol may establish a source-scoped obstruction and a conditional
minimum construction. It may not promote the construction to native dynamics.
In particular:

- the labels `left` and `right` denote field registers, not biological brain
  hemispheres;
- a weak-transmutation swap is tested as an exact register operation, not
  interpreted as a continuous clock;
- the signed map `u=q|q|` is tested both before and after coarse-graining;
- the radial gain and bath account, if reached, remain a `[SELECTION]` and an
  added dynamical type; and
- `sqrt(pi) G*` remains conditional on the already selected quartic lift.

## 3. Frozen source inputs

The certificate must fail closed unless these SHA-256 values match:

| Input | SHA-256 |
|---|---|
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/energy_ledger_compute.cpp` | `2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B` |
| `engine/src/transmutation_phases.cpp` | `4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043` |
| `engine/include/ftd/term_toggles.h` | `2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `docs/theory/08_structural/EXPLR_DUAL_SUBSTRATE_STAGGERED_ENCODING.md` | `30E85A9F1ACADEF6D7D8FEF02A371480531159B2D37E4E660219AF48077CAF87` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/ANALYSIS_QUARTIC_SELECTION_REFUTED_v1.md` | `F89284886047F6F4BE638BE1D03680D3378DB81FB808E610C6FD579FA65CF358` |
| `docs/theory/03_derivations/DERIV_LAGRANGIAN_FROM_TICK_RULE.md` | `FB09580E8060D1DB79D249C6422E62A7EE33EB63DAD339487DEF89FB4910B3AA` |

## 4. Frozen algebra

### 4.1 Native register map

For any fixed production branch, represent the identical affine register maps
as

\[
 L'=AL+b,\qquad R'=AR+b.                       \tag{1}
\]

With

\[
 F=L+R,\qquad D=L-R,                            \tag{2}
\]

the certificate must derive

\[
 F'=AF+2b,\qquad D'=AD.                         \tag{3}
\]

The desired oriented exchange is

\[
 J=\begin{pmatrix}0&1\\-1&0\end{pmatrix},
 \qquad J^2=-I.                                 \tag{4}
\]

The production weak swap is frozen as

\[
 S=\begin{pmatrix}0&1\\1&0\end{pmatrix}.       \tag{5}
\]

### 4.2 Fixed-state restoring law and damping

On one modal coordinate the most general conservative fixed-state branch
licensed by an affine acceleration has

\[
 V(q)=\frac{\kappa}{2}q^2-hq.                  \tag{6}
\]

Uniform production damping acts on a quadratic energy as

\[
 E'=(1-g)^2E.                                   \tag{7}
\]

### 4.3 Ternary and coarse coordinates

For a primitive state `s in {-1,0,+1}`, test the exact identity

\[
 s|s|=s.                                        \tag{8}
\]

The certificate must use the fixed exact ensemble
`P(+1)=3/4`, `P(-1)=1/4`, `P(0)=0` to test whether averaging
commutes with `q -> q|q|`. This is an algebraic witness, not a sampled
experiment.

### 4.4 Minimum extension

If the native ingredients fail, freeze the context-blind radial form

\[
 X'=\rho(E)JX,\qquad E=X^TX,\qquad
 E'=\rho(E)^2E.                                 \tag{9}
\]

Require `rho(1)=1`. Test first a positive constant gain, then the
lowest-degree nonconstant gain

\[
 \rho(E)=1+\eta(1-E).                           \tag{10}
\]

Close the energy account with one additional scalar register

\[
 B'=B+E-E'.                                     \tag{11}
\]

## 5. Frozen exact checks

The implementation must run 22 checks:

1. all nine source hashes;
2. invertibility of the `L/R -> F/D` register change;
3. block diagonalization of the identical operator;
4. cancellation of the shared source from `D`;
5. absence of native cross-register blocks;
6. order-four and orientation properties of `J`;
7. reflection/order-two classification of the weak swap;
8. absence of a quartic Taylor coefficient on a fixed-state branch;
9. flatness, not quarticity, after force and stiffness are both nulled;
10. `s|s|=s` on every primitive ternary state;
11. noncommutation of averaging and the signed-energy warp;
12. the independent-pair closure that would supply a square;
13. exact damping fixed-point factorization;
14. absence of a distinguished positive damping shell;
15. uniqueness of the oriented orthogonal order-four exchange up to direction;
16. the general unit-shell radial multiplier `1+2 rho'(1)`;
17. neutrality of every positive constant gain fixing the unit shell;
18. the linear gain multiplier `1-2 eta` and stability margin
    `4 eta(1-eta)`;
19. exact core-energy transfer under the radial repair;
20. exact closure of the core-plus-bath energy;
21. the conditional signed-coordinate quartic lift; and
22. the conditional `Beta(1/4,1/2)=sqrt(pi) G*` traversal.

No check may be removed, tolerance-relaxed, or replaced after execution.

## 6. Locked implementation

```text
scripts/proofs/proof_native_bilateral_quartic_dynamics_obstruction.py
```

Script SHA-256:
`4EE5CA8EE94B9B99D14A267D55A431EDAE76A2CA1143C42837123CA5DBDBD768`

After this protocol hash is recorded in the preregistration manifest, run
exactly:

```text
python scripts/proofs/proof_native_bilateral_quartic_dynamics_obstruction.py
```

## 7. Outcomes

- **Outcome A — native mechanism present:** any exact source/algebra check
  demonstrates a production cross-register quarter-turn, a smooth native
  quartic term, or a positive stable damping shell. The obstruction claim is
  rejected and the discrepant source path must be documented before any
  replacement claim.
- **Outcome B — source-scoped obstruction plus minimum conditional
  extension:** all 22 checks pass. Book that the frozen production core lacks
  the three required ingredients; book (9)--(11) only as the minimum
  conditional construction within the frozen class; keep the
  coarse-graining/pair closure and physical `G*` gearbox open.
- **Outcome C — certificate invalid:** any check fails for a tooling,
  transcription, or source-hash reason without positively exhibiting Outcome
  A. Book no theorem and repair only under a new lock.

Outcome B is not a whole-substrate impossibility theorem. It is an exact
boundary statement for the source and mathematical class frozen here.

## 8. Recorded outcome

The hash-matching implementation ran once and returned `20/22`. C1--C13,
C15--C17, and C19--C22 passed. C14 and C18 failed because the script compared
algebraically identical SymPy expressions by structural equality:

```text
g**2 - 2*g + 1        versus (1-g)**2
-4*eta*(eta-1)        versus 4*eta*(1-eta)
```

Both differences simplify exactly to zero, but that diagnostic was inspected
after the locked run. Under Outcome C, FTD-0837 books no obstruction or
minimum-extension theorem. A fresh tooling-only repair is required.
