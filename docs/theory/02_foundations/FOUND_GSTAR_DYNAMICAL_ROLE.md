# FOUND — The Dynamical Role of G*

**Tag:** [SYNTHESIS]. A cross-document integration of G*'s role in the substrate's *dynamics* (rate of time, arrow of time, coupling self-consistency, propagation closure). Introduces no theorem and promotes no tag; every claim points at its canonical source at that source's tag.
**LEDGER:** maintenance-log line; content rides on FTD-0323 (arrow/half-derivative), FTD-0001/0002 (master quadratic / G*), FTD-0050 (Link-8 closed-negative), and the FOUND_SPACETIME_EMERGENCE dynamical sections.
**Audience:** anyone asking "what does G* *do* dynamically — is it a rate, a coupling, an operator, or a fixed point?" Answer: all four, as one self-consistency constant.

---

## §0 — Thesis

G* is not a static parameter. It is the **eigen-constant of the substrate's self-referential dynamics** — the single number the lattice's own dynamics must satisfy to be consistent with themselves. It appears as four faces of one loop:

```
Z^3  --(W_3 = G*^2/2pi)-->  G*  --(x^2 = 16 G*^2 (x - G*))-->  alpha  --(governs)-->  Z^3
```

G* is the pivot. The four faces: **(1)** the *rate* of time, **(2)** the *direction* of time, **(3)** the *self-consistency* of the coupling, **(4)** the near-*closure* of propagation.

---

## §1 — Face 1: the rate of time  (G*^2 = energy per tick)  [SELECTION]

`FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md` §13 assigns a dynamical meaning to each power of G* (the "G*-ladder", derived from the master-quadratic Vieta relations):

| power | value | dynamical meaning |
|---|---|---|
| `G*^1` | 2.959 | **flux** — spatial amplitude per DoF ("what is") |
| `G*^2` | 8.754 | **energy processed per tick per DoF** — the *rate of time itself* ("what happens") |
| `G*^3` | 25.90 | **action** — spacetime history recorded per DoF ("what is kept") |

"Each multiplication by G* adds one layer of temporal structure to spatial amplitude." Time literally runs at `G*^2` energy-per-tick. Gravity and motion are *modulations of that budget*: near a mass a tick processes `G*^2 · f(r)` instead of `G*^2`, with `f(r) = 1 - r_s/r` the fraction of budget left after stored information; since energy scales as rate², proper time slows by `sqrt(f(r))`. SR and GR dilation are one mechanism:

- SR:  `dtau/dT = sqrt(1 - v^2)`   (motion drains the budget into spatial traversal)
- GR:  `dtau/dT = sqrt(f(r))`      (mass saturates the budget with stored information)

Space (Z^3) stays flat; only the observer's tick -> proper-time conversion changes (§14.8, [THEOREM] for the dilation forms). Tag: `c = 1/sqrt(3)` and the dilation formulas are [THEOREM]; "`G*^2` = energy/tick" is [SELECTION] for the identification.

---

## §2 — Face 2: the direction of time  (G* = eigenvalue of D^{-1/2})  [THEOREM]

The load-bearing theorem-grade dynamical result (`DERIV_HEAT_EQUATION_FROM_RATIO.md`, [THEOREM]):

$$D^{-1/2}\big[x^{-3/4}\big] = G^*\, x^{-1/4}$$

G* is the **exact eigenvalue of the half-integral operator `D^{-1/2}`** on the `x^{-3/4}` scaling distribution. This follows from the Riemann-Liouville formula `D^a x^b = Gamma(b+1)/Gamma(b-a+1) x^{b-a}` applied at the quarter-point `z = 1/4` (order `a = 2z-1 = -1/2`), which reproduces the Euler-reflection **ratio** `Gamma(z)/Gamma(1-z)` exactly.

The half-derivative `d_t^{1/2}` (inverse of `D^{-1/2}`) is a **Volterra integral over the entire past** — non-local in time, strictly irreversible, entropy-generating. So the arrow of time *is* the square root of the time-derivative, and G* is its eigenvalue; it "cannot be un-taken" because it integrates all history (`FOUND_ARROW_AS_SQUARE_ROOT.md`, FTD-0323). The reflection split is the even/odd decomposition under `z <-> 1-z`:

- **product** `Gamma(z)Gamma(1-z) = pi/sin(pi z)` ( = pi·sqrt2 at z=1/4) — even, commutative -> **reversible, integer-order** (wave / Lagrangian / unitary), currency **pi**;
- **ratio** `Gamma(z)/Gamma(1-z) = G*` — odd, order-sensitive -> **irreversible, half-order** (diffusion / arrow), currency **G***.

The reversible sector is the *square* of the irreversible one: `d_t = (d_t^{1/2})^2`; the branch/sign ambiguity of the root is confined to the half-order (arrow) layer and cancels under squaring. Engine instantiation: alpha (from G*) is the Rayleigh dissipation coefficient in the irreversible update `J(t+1) = J(t) + dJ - alpha·J(t)`.

Tags: the operator identity `G* <-> D^{-1/2}` is [THEOREM]; the *identification of physical time* with this operator is FC-2, [AXIOM]-class (constitution §3.4; FTD-0323 §4 explicitly declines to promote arrow-existence to [DERIVED]). Arrow *direction* is forced-given-FC-2 (FTD-0324), not a free branch — distinct from the two genuine unforced roots `i = sqrt(-1)` and `delta = sqrt(G*(4G*-1))`.

---

## §3 — Face 3: self-consistency feedback  (gap / budget equation)  [THEOREM] algebra + 0.2% lattice

The master quadratic in **gap-equation form** (`DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` §1'):

$$x^2 = 16 G^{*2}(x - G^*) = 32\pi W_3\,(x - G^*), \qquad W_3 = \frac{G^{*2}}{2\pi}$$

A BCS-style self-consistency loop: `W_3` (BCC Green's function at origin) plays the **density of states**, `16 G*^2` the **coupling x DOS**, `x` the **gap**, and G* sits in the displacement `(x - G*)` as the **centre the coupling is measured from** — exactly half the harmonic mean of the roots (`HM = 2 x_+ x_- / (x_+ + x_-) = 2G*`). "The lattice's self-energy equals its own master constant because the lattice is examining itself through the propagator" (`FOUND_SELF_REFERENTIAL_CLOSURE.md` §2.2).

**Budget equation** (engine, `benchmark_budget_equation.cpp`): dividing the quadratic by `Kx` (`K = 16 G*^2`) gives

$$\frac{x}{K} + \frac{G^*}{x} = 1$$

partitioning the coupling into a Coulomb (long-range) share `x/K ~ 0.978` and a **confined (short-range) share `G*/x ~ 0.022`**. This is **verified emergent on the lattice to 0.2%** (Helmholtz split of the flux J into longitudinal/transverse parts; r=6 on L=32, r=8 on L=48 — `README_SCIENTIFIC_STATUS.md:125`). It is exactly the fixed-point map `f(x) = 16 G*^2 (1 - G*/x)` of the ARC-D2 readout (`scripts/proofs/proof_alpha_readout_fixed_point.py`), whose stable fixed point is `x_+` and whose feedback coefficient *is* G* (the structural reduction `Det = Tr·G*`).

**Three-means / harmonic structure** (`FOUND_MASTER_QUADRATIC_BARE_STRUCTURE.md` §3, [THEOREM]): `AM : GM : HM = 8G*^2 : 4G*^{3/2} : 2G*`, a geometric progression with common ratio `2·sqrt(G*) ~ 3.440`. The "harmonic-mean parent" reading (`EXPLR_MASTER_QUADRATIC_STRUCTURAL_READINGS.md`, [STRUCTURAL OBSERVATION]): `1/G* = tr(M^{-1})` for the 2x2 mixing matrix `M` — G* as the Kirchhoff parallel-combination centre of the two coupling modes.

Tags: the gap/budget algebra is [THEOREM] (Vieta); the fixed-point-loop reading carries the framework's own [SELF-CONSISTENT] tag (= unique solution of output=input, neither [THEOREM]-from-external-axioms nor [SELECTION]-among-alternatives); the budget equation is a lattice-verified identity at 0.2%. The gap equation is a **reading of an algebraic identity, NOT a derived L->infinity self-consistency limit** (the doc is explicit).

---

## §4 — Face 4: the detuned dimension  (c^2 = 1/G* vs 1/3)  [SELECTION]

Two independent routes to the wave speed (`FOUND_SPACETIME_EMERGENCE §13.5`):

- **propagation / CFL:** `c^2 = 1/D = 1/3` exactly (hard D=3 lattice fact, [THEOREM]);
- **self-consistency:** the wave equation closes perfectly only if `c^2 = 1/G*`.

They coincide only in the counterfactual `G* = 3 = D`. The real `G* = 2.9587` sits ~1.4% below the integer 3 (`1/G* = 0.33799` vs `1/3 = 0.33333`), pulled off by the lemniscatic geometry — and the framework reads *that residual failure to close on the integer dimension* as the origin of the fine-structure constant:

> "at G* = 3 = D exactly, the wave equation self-consistency closes perfectly (c^2 = 1/G*). The actual G* = 2.959 != 3 (pulled away by the lemniscate geometry) generates the fine structure constant as the deviation from perfect self-consistency." (§13.5, [THEOREM] for the CFL result; the c^2=1/G* connection and the alpha-as-deviation reading are [SELECTION]-grade.)

G* is the "almost-3": the dimension of space measured from the inside, including its self-interaction; its gap from the flat integer is the coupling.

---

## §5 — Unification

The four faces are one because they are one loop. **G* is what the lattice's dynamics have to equal to be self-consistent**, and self-consistency simultaneously fixes: the *rate* time runs (`G*^2`/tick, §1), the *direction* it runs (the `D^{-1/2}` root, §2), the *value* the coupling settles to (the gap-equation fixed point, §3), and the *closeness* propagation comes to perfect closure (`1/G*` vs `1/3`, §4). The substrate examines itself through its own propagator; G* is the eigenvalue of that self-examination.

---

## §6 — Honest guards (do not overread)

- **G* is a self-consistency CENTRE, not a RUNNING coupling.** The renormalization-group reading — master quadratic as an RG-step characteristic polynomial — is **[CLOSED NEGATIVE]** (FTD-0050, `AUDIT_LINK8_CLOSURE.md`): the coefficient `16 G*^2` lives on the BCC Watson integral, orthogonal to the engine's `(SC+FCC)/2` stencil; RG eigenvalues are scaling dimensions (O(1)), not couplings (137, 3). Do not attribute a flow.
- **The theorem-grade core is small:** the `D^{-1/2}` eigenvalue (§2), the Vieta algebra and three-means (§3), the CFL speed and dilation forms (§1/§4), and the 0.2% budget measurement. The profound-sounding faces (`G*^2`=time-rate, physical-time=`D^{-1/2}`, alpha=deviation-from-3) are [SELECTION] / FC-2-[AXIOM]-class *identifications* on top of that core.
- **`x_- <-> N_c` is RETIRED.** The old "tick budget = `1/alpha + N_c`" partition leans on the retired identification; only the trace `16 G*^2` and the Coulomb/confined split stand.
- **The gap equation is a reading**, not a derived thermodynamic-limit self-consistency condition (`DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` Abstract).

## §7 — Cross-references

`DERIV_HEAT_EQUATION_FROM_RATIO.md` (D^{-1/2} eigenvalue, [THEOREM]); `FOUND_ARROW_AS_SQUARE_ROOT.md` (FTD-0323); `FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md` §13-14 (energy/tick, dilation, detuned-3); `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` + `FOUND_SELF_REFERENTIAL_CLOSURE.md` (gap/self-consistency, [SELF-CONSISTENT]); `FOUND_MASTER_QUADRATIC_BARE_STRUCTURE.md` §3 (three means); `EXPLR_MASTER_QUADRATIC_STRUCTURAL_READINGS.md` (mixing-matrix / harmonic parent); `benchmark_budget_equation.cpp` (0.2% lattice); `scripts/proofs/proof_alpha_readout_fixed_point.py` (the fixed-point readout, ARC-D2 P1); `AUDIT_LINK8_CLOSURE.md` (FTD-0050, RG closed-negative); constitution `SPEC_FTD_FRAMEWORK_V1.md` §3.4 (pi/G* split = algebra of FC-2).
