# FTD-0950 — Preregistration: causal work-booked C18 finite-radius relaxation v1

**Identifier:** `FTD-0950`  
**Date:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Scope:** exact mathematics for the selected FTD-0949 reference action; no
production integration

## 1. Question

Can the compact anti-continuum core of FTD-0949 generate finite-support,
causal approximants to its exact exponentially tailed recursive body without
reading the completed target profile, while every field residual, energy
transaction, charge transaction, and overwritten datum is explicitly booked?

This protocol does **not** ask for exact finite-time formation. FTD-0949
proves that impossible for compact data under vacuum-preserving finite-range
ticks. It asks for the strongest finite-radius replacement compatible with
that theorem.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_UNCONTAINED_C18_EXPONENTIALLY_TAILED_RECURSIVE_CHARGE_AND_FORMATION_BOUNDARY_v1.md` | `FC1F750CA5D5ABF52608F4789BE054B43919055FCB8A9EE674CD211B8E1B6356` |
| `proof_uncontained_c18_exponentially_tailed_recursive_charge.py` | `A9C72A3DB5B9E5E4F814470F5DB2DBA4CEFEB3FB125DD3B3BE9E7E26BC0D9536` |
| `THEOREM_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_SOURCE_FRAME_BOUNDARY_v1.md` | `BD5B9DB5C9543F76241E6525B0CCD44787D16FE933D24E742C3982F9E6898981` |
| `THEOREM_QUASILOCAL_COMPANION_PREPARATION_AND_REVERSIBLE_HISTORY_FORMATION_BOUNDARY_v1.md` | `4E00155889BAD84D3ED4A7B907BFBC86589DEA6873A24529519ADE310DC9CEFB` |
| `THEOREM_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md` | `A7DC30C90C491976F58CDEAF71FB5ABFCE04952ECE971CA7FF72C65A7B9B90BF` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |

Any hash drift fails closed. No source may be repaired after execution without
a separately locked repair protocol.

## 3. Frozen local controller

Retain the exact FTD-0949 regime and notation

\[
 \Lambda=\beta A_0^4\ge10^4,
 \qquad a^2={6\over5},
 \qquad \omega^2={26\Lambda\over25},
 \qquad c={2489\over9000},
 \qquad b={11\over18000}.                                  \tag{1}
\]

Let `c_x` be one compact body-core marker: `c_o=1` at the registered core and
zero elsewhere. Set

\[
 \phi^{(0)}_x=a c_x,
 \qquad
 g(z)=2\Lambda z\left(3z^4-4z^2+1-{13\over25}\right),       \tag{2}
\]

and the local diagonal response

\[
 \ell_x={24\Lambda\over25}(1+15c_x),
 \qquad (Lu)_x=\ell_xu_x.                                  \tag{3}
\]

For the exact C18 operator `K`, define the residual and controller

\[
 \mathcal F(\psi)=K\psi+g(\psi),
 \qquad
 \mathcal T(u)=u-L^{-1}\mathcal F(\phi^{(0)}+u).             \tag{4}
\]

The **[SELECTED REFERENCE CONTROLLER]** is

\[
 u_0=0,
 \qquad u_{n+1}=\mathcal T(u_n),
 \qquad \phi_n=\phi^{(0)}+u_n.                              \tag{5}
\]

Equation (5) may read only the local core marker, the current field, and the
fixed face/edge C18 neighbourhood. It may not read the exact fixed point,
future iterates, a fitted profile, a measurement context, a Born weight, or
an external target arm.

The marked core and the controller are selected reference structure. Their
mathematical consequences may be theorem-grade; the controller itself is not
claimed to emerge from production dynamics.

## 4. Frozen convergence gates

The certificate must prove exactly:

\[
 \|u_{n+1}-u_n\|_w\le b c^n,                               \tag{6}
\]

and, for the unique FTD-0949 fixed point `u_*`,

\[
 \boxed{
 \|\phi_n-\phi_*\|_w
 \le {b\over1-c}c^n
 ={11\over13022}\left({2489\over9000}\right)^n.}           \tag{7}
\]

For any declared `epsilon>0`, define `N_epsilon` as the least integer for
which the right side of (7) is at most `epsilon`. No logarithm, continuum
limit, fitted tolerance, or completed-infinity totality is needed.

Let `B_n` be the finite C18 graph ball of depth `n` around the core. The
certificate must prove

\[
 \operatorname{supp}\phi_n\subseteq B_n,                    \tag{8}
\]

and restriction consistency: two finite-region computations agree wherever
the complete depth-`n` dependency cone is shared.

## 5. Frozen mismatch export

The field-equation mismatch at layer `n` is

\[
 r_n=\mathcal F(\phi_n).                                    \tag{9}
\]

The controller must satisfy the exact local identity

\[
 \boxed{r_n=L(u_n-u_{n+1})},                                \tag{10}
\]

and the registered residual envelope

\[
 \boxed{
 \|r_n\|_w
 \le {88\Lambda\over9375}
 \left({2489\over9000}\right)^n.}                          \tag{11}
\]

The mismatch is an output record, not silently discarded error.

## 6. Frozen local work and charge ledgers

Let face bonds have weight `1/9` and edge bonds weight `1/18`. On the
phase-covariant rotating section, assign the local field energy

\[
 h_x(\phi)=A_0^2\left[
 {\omega^2\over2}\phi_x^2
 +\Lambda\phi_x^2(\phi_x^2-1)^2
 +{1\over4}\sum_{y\sim x}w_{xy}(\phi_x-\phi_y)^2
 \right].                                                   \tag{12}
\]

Define the local controller work and the named bookkeeping reservoir
`FormationWorkLedger` by

\[
 w_{n,x}=h_x(\phi_{n+1})-h_x(\phi_n),
 \qquad
 R^{E}_{n+1,x}=R^{E}_{n,x}-w_{n,x}.                          \tag{13}
\]

Equation (13) must give exact pointwise and aggregate conservation. With

\[
 \rho={1101\over1000},
 \qquad
 C_E=\rho\left({16\over9}+{16876\over625}\Lambda\right),   \tag{14}
\]

the certificate must prove

\[
 \sum_x|w_{n,x}|\le A_0^2 C_E b c^n,
 \qquad
 \sum_{n\ge0}\sum_x|w_{n,x}|
 \le A_0^2 C_E{11\over13022}.                              \tag{15}
\]

For orientation sign `sigma in {+1,-1}`, define

\[
 q_{n,x}=\sigma\omega A_0^2
 \left(\phi_{n+1,x}^2-\phi_{n,x}^2\right),
 \qquad
 R^Q_{n+1,x}=R^Q_{n,x}-q_{n,x}.                             \tag{16}
\]

The charge ledger must conserve field plus reservoir charge exactly and obey

\[
 \sum_x|q_{n,x}|
 \le2\omega A_0^2\rho b c^n.                               \tag{17}
\]

The ledgers in (13) and (16) are signed accounts. They are not a positive,
phase-complete physical reservoir and may not be described as one.

## 7. Frozen reversible mismatch-port lift

For a fresh field-shaped port `e`, define

\[
 u^+=\mathcal T(u)+L^{-1}e,
 \qquad
 m^+=L(u-u^+).                                              \tag{18}
\]

The certificate must prove the exact local inverse

\[
 u=u^++L^{-1}m^+,
 \qquad
 e=L\left[u^+-\mathcal T(u)\right].                         \tag{19}
\]

On the fresh section `e=0`, equation (18) must reduce to (5) and export
`m^+=r_n`. The energy and charge ledger coordinates must also invert using
the recovered old field. A cotangent lift of the resulting coordinate
bijection may be recorded as symplectic, but no positive invariant
Hamiltonian may be inferred.

Every repeated layer consumes a fresh zero port or requires an independently
derived local recycling mechanism. The protocol does not authorize an
infinite pre-existing reservoir or free erasure.

## 8. Frozen outcomes

| Outcome | Required result | Promotion boundary |
|---|---|---|
| A | Equations (4)--(19), finite support, restriction consistency, target blindness, exact mismatch export, finite total work/charge variation, and reversible mismatch-port lift all pass | causal finite-radius **reference relaxation and ledger closure**; positive autonomous reservoir, port recycling, native source orientation, exact finite-tick Hamiltonian dynamics, stability, and production remain open |
| B | Geometric causal convergence passes but one of mismatch, work/charge, or reversible-port closure fails | causal approximation only; failed accounting layer remains open |
| C | The controller reads the target or lacks a finite dependency cone | reject controller |
| D | Hash, algebra, bound, or scope gate fails | no theorem |

## 9. Acceptance and stop conditions

The exact certificate must check:

1. every frozen hash and scope marker;
2. the two values of `ell_x` and the equivalence of (4) to the FTD-0949
   Banach map;
3. the exact rational constants `c`, `b`, `b/(1-c)`;
4. geometric iterate-difference and fixed-point error bounds;
5. finite C18 dependency support and restriction consistency;
6. residual identity (10) and bound (11);
7. the sum of local density (12) equals the rotating-section Hamiltonian;
8. exact work conservation and the absolute-work envelope (15);
9. exact charge conservation and envelope (17);
10. exact inverse (19), fresh-port reduction, and mismatch export;
11. target/context/Born/profile-read firewalls;
12. explicit non-promotion of signed ledgers to a positive reservoir;
13. explicit fresh-port/recycling and exact finite-tick Hamiltonian debts; and
14. the frozen outcome classifier.

Failure of any exact check yields Outcome D. No numerical near-miss search,
parameter scan, floating-point tolerance, empirical fit, or formula
substitution is permitted.

Do not modify production engine sources, CMake, `Voxel`, constants, toggles,
or default tick phases under this protocol.

## 10. Promotion boundary

Even Outcome A is not spontaneous particle formation. It would establish a
local, target-blind computation of finite-radius approximants and exact
information/energy/charge accounting for that computation. It would not
establish:

- a positive phase-complete reservoir or its local microdynamics;
- autonomous port supply, recycling, stopping, or erasure;
- a native ordered two-frame/pseudoscalar source for `sigma`, `e`, and the
  transverse direction;
- exact energy-preserving finite-tick evolution of the selected Hamiltonian;
- perturbation stability, mobility, collision identity, mass, or production
  normalization;
- `gamma`, `G*` synchronization, Born/Bell recovery, Lorentz hiding, or
  framework completeness.

The next admissible branch after Outcome A is a positive local reservoir and
port-recycling construction, audited independently from the profile
computation and from the native orientation-source question.
