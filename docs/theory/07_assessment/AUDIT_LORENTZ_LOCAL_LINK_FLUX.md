# FTD-0417 — Local link-flux photon candidate

**Date:** 2026-07-22  
**Status:** `[SELECTED ONTOLOGY EXTENSION]` + `[THEOREM — exact gauge invariance, locality, free pole, and full-band stability]` + `[DERIVED — tree-level cone defect]` + `[FTD-0419 SUCCESSOR — step-scheme automatic cancellation closed negative]` + `[OPEN — ternary current map and on-shell threshold]`  
**Verdict:** `LOCAL-PHOTON-SECTOR-EXISTS; IMPROVED-CONE-SACRIFICED; INTERACTING-RECOVERY-OPEN`  
**Verifier:** `scripts/proofs/proof_lorentz_local_link_flux.py`

---

## 0. Outcome

The nonlocal connection prescription `A=P_T J` is not needed if the flux
sector is enlarged by one explicit local type: a real `U(1)` connection
`A_mu(n)` on every oriented spacetime link. Electric flux is then the
time-space plaquette field strength, magnetic flux is the spatial plaquette
field strength, and a photon is a transverse normal mode of those local link
variables.

The smallest frozen action is noncompact anisotropic lattice `U(1)`:

$$
S_A=\frac12\sum_n\left[
\sum_i F_{0i}(n)^2+c_A^2\sum_{i<j}F_{ij}(n)^2
\right],
\qquad c_A^2=\frac17.
$$

Every term occupies one unit plaquette. The action is exactly gauge invariant,
and its temporal-gauge leapfrog realization reads only adjacent plaquettes.
For a transverse plane wave its exact pole is

$$
4\sin^2\frac{\theta}{2}
=\frac17\,4\sum_i\sin^2\frac{q_i}{2}.
$$

The complete Brillouin band is stable because the right-hand side after
division by four is at most `3/7<1`.

This is a local photon-sector candidate, not a derivation from P1--P5. It adds
a conjugate link connection that is absent from the present site-centered
`(s,J)` engine state. It also does **not** retain the FTD-0411/0413 quartic-
improved `T_B/M18` pole: agreement with the selected matter cone now holds
only at leading order. The full interacting threshold remains uncalculated.

---

## 1. Frozen field content

Work on an arbitrarily large finite diagnostic lattice. Periodic lattices may
be used for Fourier verification; no completed-infinity limit is assumed.

The selected fields are:

1. `A_mu(n) in R` on each oriented spacetime link;
2. `U_mu(n)=exp(i g A_mu(n))` as the transporter seen by charged matter;
3. the existing ternary site state `s(n) in {-1,0,+1}`;
4. a local conserved link current `j_mu(n)` when sources are present.

For the forward difference

$$
\Delta_\mu f(n)=f(n+\hat\mu)-f(n),
$$

define the unit-plaquette curvature

$$
F_{\mu\nu}(n)=\Delta_\mu A_\nu(n)-\Delta_\nu A_\mu(n).
$$

The gauge transformation is

$$
A_\mu(n)\mapsto A_\mu(n)+\chi(n)-\chi(n+\hat\mu),
$$

so that

$$
U_\mu(n)\mapsto
e^{ig\chi(n)}U_\mu(n)e^{-ig\chi(n+\hat\mu)}.
$$

Because forward differences commute, `F_mu_nu` is unchanged exactly. No
inverse Laplacian or transverse projection occurs in the microscopic action.

The physical local flux variables are

$$
E_i=F_{0i},\qquad
B_i=\frac12\epsilon_{ijk}F_{jk}.
$$

Thus `A` is not itself the flux. It is the local connection whose curvature
and canonical momentum carry magnetic and electric flux. A photon is an
oscillation of `(E,B)`, not a constituent of the lattice.

### 1.1 Adoption bill

This construction makes three selections explicit:

| Input | Status | Price |
|---|---|---|
| independent real link connection `A_mu` | `[SELECTED ONTOLOGY EXTENSION]` | one new continuous link type and its gauge redundancy |
| noncompact unit-plaquette Maxwell action | `[SELECTED ACTION]` | imported lattice-gauge mechanism |
| `c_A^2=1/7` | inherited `[SELECTION]` | leading cone matched to the FTD-0411 two-domain branch |

The connection-carrier choice is booked under existing import-ledger line
`IMP-S4`. FTD-0417 broadens that line from the projected branch alone to the
mutually exclusive choice between global projection and an independent local
link carrier. No `IMP-S5` is added because the two branches are alternatives,
not two simultaneously consumed connection types.

The action is reversible when written in `(A,E)` Hamiltonian form. Therefore
it is not the current many-to-one FTD update map in disguise; it is a candidate
local gauge sector that must still be reconciled with that update.

“Noncompact” is also substantive: `A_mu` is real rather than angle-valued.
This minimal action contains no compact-link monopole sectors or compact-U(1)
strong-coupling confinement mechanism. Replacing it by a cosine Wilson action
would be a different regulator with additional lattice gauge vertices and
would require a new threshold calculation.

---

## 2. Locality, gauge invariance, and Gauss law

Every `F_mu_nu(n)^2` uses the four links bounding one unit plaquette. A local
variation therefore couples a link only to plaquettes incident on that link.
In temporal gauge, the source-free equation can be written

$$
A_i^{t+1}-2A_i^t+A_i^{t-1}
=-c_A^2\,(\operatorname{curl}^\dagger
\operatorname{curl}A^t)_i.
$$

Each microscopic update reads only unit-cell neighbours. The second time
level is local memory, equivalently the electric field `E_i`; it does not
create a radius-two spatial dependency.

A direct source term is gauge invariant only for an exactly conserved lattice
current:

$$
S_j=ig\sum_{n,\mu} A_\mu(n)j_\mu(n),\qquad
\Delta^-_\mu j_\mu=0.
$$

Variation with respect to `A_0` gives the lattice Gauss constraint. The
divergence of the curl vanishes identically, so a conserved current preserves
that constraint under the local update.

The existing Wilson matter module already has gauge-covariant link
transporters, including the equal average of both shortest face-diagonal
paths. It can therefore consume `U_mu` locally. What is **not** supplied here
is a conserved map from the ternary tick history to `j_mu[s]`. Assigning
`rho proportional to s` at one time is insufficient: gauge consistency also
requires the link current that accounts for transport, pair creation, and
annihilation at every tick.

FTD-0418 later freezes a separate minimal axial spacetime Wilson regulator for
this local-link branch. It deliberately does not consume the face-diagonal
FTD-0413 prototype; the latter remains a distinct q4-improved free diagnostic.

---

## 3. Exact free photon pole

For dimensionless frequency `theta` and spatial momentum `q_i`, define

$$
\widehat\omega=2\sin\frac{\theta}{2},\qquad
\widehat q_i=2\sin\frac{q_i}{2},\qquad
\widehat{\mathbf q}^{,2}=\sum_i\widehat q_i^2.
$$

In temporal gauge and on the transverse subspace
`sum_i widehat(q_i) A_i=0`, the quadratic inverse propagator is

$$
K_T=\widehat\omega^2-c_A^2\widehat{\mathbf q}^{,2}.
$$

There are two transverse polarizations for every nonzero generic momentum.
The longitudinal component is constrained by Gauss law rather than propagated
as a third photon.

The exact physical phase is

$$
\theta(\mathbf q)=2\arcsin\left[
c_A\sqrt{\sum_i\sin^2\frac{q_i}{2}}
\right].
$$

At `c_A^2=1/7`, the arcsine argument squared lies in `[0,3/7]` throughout
the complete spatial Brillouin zone. Hence every transverse free mode has a
real phase and unit-modulus transfer roots.

The value `1/7` is not re-derived by this action. For a general positive
coefficient, the leading pole is `theta^2=c_A^2 |q|^2+...`; choosing the
FTD-0411 branch simply fixes the local electric/magnetic kinetic ratio.

---

## 4. The price of minimal locality: the q4 improvement is lost

Let

$$
S_2=\sum_iq_i^2,\qquad
Q_4=\sum_iq_i^4,\qquad
P_{22}=\sum_{i<j}q_i^2q_j^2.
$$

Series reversion of the exact pole gives

$$
\theta_A^2
=c_A^2S_2-\frac{c_A^2}{12}Q_4
+\frac{c_A^4}{12}S_2^2+O(q^6).
$$

At `c_A^2=1/7`,

$$
\boxed{
\frac{\theta_A^2}{c_A^2}
=S_2-\frac1{14}Q_4+\frac1{42}P_{22}+O(q^6)}.
$$

The selected FTD-0413 semidiscrete massless matter pole is
`E_m^2/c_A^2=S_2+O(q^6)`. Therefore this local plaquette photon and that
matter prototype disagree already at quartic order. The former FTD-0414
`O((ka)^4)` velocity envelope does not apply to this new action.

For a unit direction `n` with `R_4=sum_i n_i^4` and `q=ka`, the photon group
velocity is

$$
\frac{v_{g,A}}{c_A}
=1+\frac{1-7R_4}{56}q^2+O(q^4).
$$

Consequently:

* the largest photon/matter group-speed gap in the selected comparison is
  `3q^2/28+O(q^4)`, attained on an axis;
* the leading photon directional spread between an axis and a body diagonal
  is `q^2/12+O(q^4)`.

These are dimension-six tree-level cutoff effects. They vanish in the
infrared but are not protected against radiative mixing into the marginal
operators catalogued by FTD-0415.

This loss is deliberate and falsifiable. The unit-plaquette action is the
smallest local regulator. A later improved loop action may be tested, but it
must pay for larger support or auxiliary fields and must re-establish
full-band stability, gauge identities, and the current map.

---

## 5. Relation to the current site flux `J`

The old engine variable `J_i(n)` and the new electric link flux `E_i(n)` are
not automatically identical. A local site diagnostic can be defined by

$$
J_i^{\rm display}(n)=\frac12\left[E_i(n)+E_i(n-\hat i)\right].
$$

This map is local but not one-to-one. It does not derive `A` from `J`, and it
does not license replacing the live engine field without a migration proof.
Its purpose is only to show that link flux can reproduce a site-centred
display without using `P_T`.

The clean ontology is therefore:

$$
\text{causal adjacency}
+\text{link connection/flux}
+\text{site manifestation}.
$$

The lattice supplies adjacency; it is not made of photons. Photons are the
two transverse collective modes of the link-flux sector.

---

## 6. Interaction contract and exact next calculation

Charged matter couples through the already used local transporter

$$
U_\mu=e^{igA_\mu}
=1+igA_\mu-\frac{g^2}{2}A_\mu^2+O(g^3).
$$

The linear and quadratic terms are respectively the one-photon vertex and
the two-photon seagull. The face-diagonal matter transporter must be expanded
as the complete average of both shortest paths; deleting its cross terms
breaks the action-level vertex contract.

Exact gauge covariance implies the lattice Ward identity in forward-
difference convention,

$$
\sum_\mu(e^{ik_\mu}-1)\Gamma_\mu(p+k,p)
=g\left[D(p+k)-D(p)\right],
$$

with the corresponding two-photon identity obtained by a second variation.
The phase placement of `Gamma_mu` depends on the chosen link midpoint
convention, but the action identity does not.

At the time of FTD-0417, the gauge regulator lacked a compatible spacetime
matter regulator. FTD-0418 closes that definition stage with a nearest-link,
one-tick Euclidean Wilson action. It proves one massless corner, 15 positive
doubler gaps, and the exact one- and two-photon Ward identities. The action is
axial rather than SC+FCC-improved, so it shares only the leading cone with the
photon.

The remaining locked calculation is now to evaluate the full-zone
coefficients and form

$$
\delta_{\rm match}
=(\delta Z_s-\delta Z_t)
-\frac12(\delta Z_B-\delta Z_E).
$$

FTD-0419 now performs that integration in one declared `xi=1` QED_L-like step
scheme and obtains `delta_match/g²=-0.32696906(5)`. With selected `g²=alpha`,
the resulting bare threshold is about `9.28e5` too large for FTD-0416's
optimistic `1e-15` tolerance translation. A counterterm is required in that
scheme. The gauge-independent on-shell match, ternary-history current, and
real-time ontology remain independent open problems.

---

## 7. Status table

| Claim | Status |
|---|---|
| independent link connection and plaquette flux ontology | `[SELECTED ONTOLOGY EXTENSION]` |
| noncompact unit-plaquette action | `[SELECTED ACTION]` |
| exact microscopic gauge invariance | `[THEOREM — selected action]` |
| finite-support locality | `[THEOREM — selected action]` |
| exact transverse photon pole | `[THEOREM — selected action]` |
| full-band stability at `c_A^2=1/7` | `[THEOREM — selected action]` |
| leading cone speed `1/sqrt(7)` | `[IMPOSED from inherited selection]` |
| quartic tree-level defect and speed envelope | `[DERIVED — selected action]` |
| conserved ternary-history current `j_mu[s]` | `[OPEN]` |
| complete discrete-time matter regulator | `[SELECTED and verified in FTD-0418]` |
| one-loop step-scheme `delta_match` | `[NUMERICAL FACT — FTD-0419; automatic cancellation CLOSED NEGATIVE]` |
| gauge-independent on-shell match | `[OPEN — HARD]` |

The result closes the locality objection to the *existence* of a photon
sector, at the explicit cost of a new link type. It does not close the bridge
from the five FTD postulates, the ternary-current problem, the common improved
cone, unitarity of the complete update, or radiative Lorentz recovery.
