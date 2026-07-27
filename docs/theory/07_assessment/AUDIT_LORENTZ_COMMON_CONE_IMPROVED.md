# FTD-0413 — Moore-Local Improved Fermion Common-Cone Gate

**Date:** 2026-07-22  
**Status:** `[DERIVED — normalized face-diagonal ansatz]` + `[SELECTED IMPLEMENTATION PROTOTYPE]` + `[FREE COMMON-CONE-THROUGH-q4]` + `[OPEN — q6, interacting, gauge/gravity, and radiative recovery]`  
**Verdict:** `COMMON-CONE-THROUGH-q4; LIVE-MULTI-SECTOR-CONE-OPEN`  
**Predecessors:** FTD-0407–0412  
**Verification:** `scripts/proofs/proof_lorentz_common_cone_improved.py`; native test `lorentz_common_cone_improved`

---

## 0. Result

FTD-0412 proved that the axial Wilson Hamiltonian cannot share the complete
quartic pole of the selected BCC-time/SC+FCC-space flux prototype. That no-go
fixed the kinetic symbol to `sin(q_i)` and varied only the scalar Wilson
parameter `r`.

The smallest enlargement inside the same one-shell Moore neighbourhood is a
transverse face-diagonal average:

$$
K_i(q)=\sin q_i\,[a+b(\cos q_j+\cos q_k)],\qquad \{i,j,k\}=\{x,y,z\}.
$$

Unit infrared normalization requires `a+2b=1`. Requiring the massless
Hermitian Wilson pole to have no quartic tensor then uniquely fixes, within
this ansatz,

$$
\boxed{a=b=\frac13},\qquad
\boxed{r^2=\frac43}.
$$

Selecting the FTD-0411 cone `c_s²=1/7` gives a free matter pole that agrees
with the selected q4-free flux pole through quartic order. All seven Wilson
doublers remain gapped. The implementation uses only axial SC and
face-diagonal FCC neighbours and is gauge covariant under the existing U(1)
link convention.

This is a real escape from the FTD-0412 scalar-`r` no-go, but it is not Lorentz
recovery. The construction is reverse-solved from the q4 cancellation target,
is default off, and the matter and flux poles disagree at q6. Manifested
matter, dynamical gauge propagation, native gravity propagation, interactions,
and radiative stability are not closed.

---

## 1. Exact quartic derivation

Write

$$
S_2=q_x^2+q_y^2+q_z^2,\quad
Q_4=q_x^4+q_y^4+q_z^4,\quad
P_{22}=q_x^2q_y^2+q_x^2q_z^2+q_y^2q_z^2.
$$

After imposing `a=1-2b`, the kinetic norm has expansion

$$
\sum_i K_i^2
=S_2-\frac13Q_4-2bP_{22}+O(q^6).
$$

The unchanged Wilson mass symbol is

$$
W(q)=\sum_i(1-\cos q_i)=\frac12S_2-\frac1{24}Q_4+O(q^6),
$$

so for `m=0`,

$$
\frac{E^2}{c_s^2}
=S_2+
\left(-\frac13+\frac{r^2}{4}\right)Q_4
+\left(-2b+\frac{r^2}{2}\right)P_{22}
+O(q^6).
$$

The two tensor coefficients vanish iff

$$
-\frac13+\frac{r^2}{4}=0,
\qquad
-2b+\frac{r^2}{2}=0.
$$

Therefore `r²=4/3`, `b=1/3`, and normalization gives `a=1/3`. This solution
is unique in the declared normalized one-parameter kinetic ansatz. The positive
root `r=2/sqrt(3)` is selected to preserve the conventional positive Wilson
mass orientation.

The escape does not contradict FTD-0412: that result proved incompatibility
for `b=0`. FTD-0413 adds a new face-diagonal kinetic coefficient and therefore
leaves the earlier theorem's hypothesis class.

---

## 2. Local gauge-covariant realization

For a face-diagonal displacement `s_i e_i+s_j e_j`, two shortest link paths
exist. The implemented transporter is

$$
U_{ij}^{s_i s_j}(n)=\frac12\left[
U_i^{s_i}(n)U_j^{s_j}(n+s_i e_i)
+U_j^{s_j}(n)U_i^{s_i}(n+s_j e_j)
\right].
$$

Both products transform with the same start and endpoint phases, hence their
average is gauge covariant. Pairing each forward corner with the adjoint
backward corner makes the kinetic difference anti-Hermitian; multiplication
by `-i alpha_i` makes the Hamiltonian contribution Hermitian.

At identity links the position-space operator has exact symbol

$$
K_i(q)=\frac{\sin q_i}{3}
\left(1+\cos q_j+\cos q_k\right).
$$

Its support is the central site, six axial neighbours, and twelve face
diagonals. It reads no body diagonal and no radius-two site, so the one-step
dependency remains inside the SC+FCC Moore shell.

The new coefficient is `WilsonDiracParams::kinetic_transverse_weight`, whose
default is zero. It changes only the real-time Hermitian Hamiltonian. The
retained spatial `D_W` diagnostic and its CUDA mirror remain on their historical
axial stencil.

---

## 3. Full-band and doubler checks

The exact free spectrum is

$$
E^2(q)=c_s^2\sum_iK_i(q)^2+
\left[m+c_s r\sum_i(1-\cos q_i)\right]^2.
$$

At every Brillouin corner `K_i=0`. If `n_pi` coordinates equal `pi`, then

$$
W=2n_{\pi},\qquad
\frac{E^2}{c_s^2}=\frac{16}{3}n_{\pi}^2
\quad(m=0).
$$

Only the origin has `n_pi=0`; all seven other corners remain positively
gapped. The native gate evaluates the implemented Hamiltonian over every mode
of the complete `L=8` Brillouin zone and compares it with the exact formula.
It also applies the operator to a delta source and verifies that all nonzero
support remains in the SC+FCC shell.

The existing random-link test now evaluates the `b=1/3` Hamiltonian as well as
the default `b=0` Hamiltonian and checks both gauge covariance and Hermiticity.

---

## 4. First surviving mismatch

For the selected coefficients, the semidiscrete matter Hamiltonian gives

$$
\frac{E^2}{c_s^2}
=S_2+
\frac1{36}S_2^3+
\frac1{36}S_2Q_4-
\frac1{15}Q_6+O(q^8).
$$

The literal BCC-time flux branch gives

$$
\frac{\theta_{\rm flux}^2}{c_s^2}
=S_2-\frac{61}{17640}S_2^3
+\frac1{72}S_2Q_4-\frac1{90}Q_6+O(q^8).
$$

Here `Q4=sum(q_i^4)` and `Q6=sum(q_i^6)`. The unequal `S2 Q4` and `Q6`
coefficients prove a q6 mismatch. The implemented RK4
matter clock cannot remove it: its phase map starts
`theta_RK4=x-x^5/120+O(x^7)`, which adds only an isotropic
`-c_s^4 S2^3/60` term after factoring out `c_s²`. Thus exact time-step
accounting changes the isotropic q6 coefficient but leaves the mixed mismatch.

Consequently FTD-0413 closes the free flux/Wilson-matter comparison only
through q4. It does not establish a common dimension-eight pole.

---

## 5. Status and assumption bill

| Item | Status |
|---|---|
| Quartic solution inside normalized face-diagonal ansatz | `[DERIVED]` |
| `a=b=1/3`, `r=2/sqrt(3)` as physical stencil | `[SELECTED]` — reverse-solved from q4 cancellation |
| `c_s²=1/7` | inherited `[SELECTED]` two-domain cone from FTD-0411 |
| Symmetric two-path diagonal transporter | `[SELECTED]`, gauge-covariant implementation |
| Exact free full-band Hamiltonian spectrum | `[THEOREM — implemented operator]` + native verification |
| Seven Wilson doublers gapped | `[THEOREM — implemented free operator]` |
| Free selected flux/matter common cone through q4 | `[DERIVED given selections]` |
| Common q6 pole | `[CLOSED NEGATIVE — this stencil versus literal/surrogate FTD-0411 clocks]` |
| Production/live common cone | `[OPEN]` |
| Interacting/radiatively stable common cone | `[OPEN]` |

---

## 6. Remaining hard gates

1. Derive or independently motivate the transverse average and Wilson `r`
   without using the cancellation target.
2. Put flux and matter on one exact discrete temporal transfer rather than a
   flux Floquet clock plus RK4 matter clock.
3. Match or eliminate the complete q6 tensor without leaving P4 or creating
   ghosts/additional light modes.
4. Produce dynamical gauge and gravity poles and include them in the cone
   matrix.
5. Compute the exact-symmetry-allowed dimension-3/4 operator-mixing matrix.
6. Pass interacting Ward/unitarity, SME, and operational composite-clock gates.

Until those close, the correct statement is:

> A selected SC+FCC-local Wilson matter stencil shares the selected BCC-time
> flux cone through q4 in the free theory. Lorentz covariance and a live
> multi-sector common cone remain open.
