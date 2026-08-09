# DERIV — The Two Owed Proofs: Operational Hiding and Lorentz Recovery

**Status:** `[DERIVED — PROOF 1 COMPLETE AS A REDUCTION]` +
`[DERIVED — PROOF 2 FREE SECTOR, EXACT]` +
`[DERIVED — RADIATIVE dim-4 CHANNEL CLOSED BY SYMMETRY + SINGLE-SECTOR REDUCTION]` +
`[OPEN — PROOF 2 COMMON CONE / INTERACTING VERTICES]` +
`[BOOKED — FTD-0815]`
**Date:** 2026-08-07; §2.5 added 2026-08-08
**Artifacts:** `scripts/experiments/temporal_interior/derive_lorentz_free_sector.py` (exact
symbolic derivation), `scripts/experiments/temporal_interior/verify_hiding_reduction.py`
(numerical verification of both claims),
`scripts/experiments/temporal_interior/derive_radiative_reduction.py` (stencil uniqueness,
cubic averaging, single-sector absorption)
**Parents:** `FOUND_SPA_CHAIN_RELATIVITY_EXTENSION_v1.md` §§4, 8, 15
(where the two proofs are declared owed), FTD-0796 (the Fine/CHSH trap),
`AUDIT_LORENTZ_RECOVERY_HARD.md` (the LR-0..LR-6 contract).
**Production impact:** none. No engine state, tag, or golden changes.

---

## 0. What was owed, and what is delivered

A preferred-foliation ontology owes two independent proofs before its
hidden order can be called scientifically harmless:

> **(R4b)** the selector law is *operationally hidden* — the pushforward
> of its history measure onto recorded data matches the covariant target
> for every experiment; and
>
> **(LR)** every observable sector passes ordinary Lorentz-recovery tests.

This document delivers:

- **Proof 1 (R4b): complete, as a reduction.** R4b is *not* an
  independent obligation. It follows from the Born pushforward together
  with spacelike order-independence. The corollary matters more than the
  theorem: the real barrier is not relativistic at all.
- **Proof 2 (LR): the free sector, exactly.** The production stencil is
  isotropic through fourth order — not by tuning but by construction —
  and the leading anisotropy is computed in closed form,
  $|\Delta v/v| = (ka)^4/3240$, which is $10^{-56}$ at the highest
  observed photon energies. §2.5 then reduces the radiative-stability
  problem: its dimension-4 channel is closed three independent ways, and
  what remains is the already-registered common-cone question.

---

## 1. Proof 1 — operational hiding reduces to the Born pushforward

### 1.1 Statement

Let a selector law on a physically fixed preferred foliation $f_0$
generate a measure $\mu_{f_0}$ over complete realized histories. For an
operational experiment $\mathcal{E}$, let $\pi_{\mathcal{E}}$ forget
unobservable history detail and retain recorded data, and let
$P^{\rm cov}_{\mathcal{E}}$ be the covariant target distribution.

> **Theorem 1.** Suppose
> **(H1)** *Born pushforward.* For every experiment, the marginal of
> $\mu_{f_0}$ on recorded data equals the quantum composed-history weight
> $P(h) = \Tr[\mathcal{I}^{M_N}_{o_N}\circ\cdots\circ
> \mathcal{I}^{M_1}_{o_1}(\rho_0)]$; and
> **(H2)** *spacelike order-independence* (R3) holds for those weights.
> Then $(\pi_{\mathcal{E}})_*\mu_{f_0} = P^{\rm cov}_{\mathcal{E}}$ for
> every operational experiment $\mathcal{E}$ — that is, **(R4b) holds**.

### 1.2 Proof

Let $\mathcal{E}$ be any operational experiment producing record $r$. By
(H1) the distribution of $r$ under $\mu_{f_0}$ is the quantum weight
$P(h)$. By (H2) $P(h)$ is invariant under exchange of the composition
order of spacelike-separated instruments; since two admissible foliations
of the same experiment differ precisely by such exchanges, $P(h)$ takes
the same value for every admissible foliation and therefore equals
$P^{\rm cov}_{\mathcal{E}}(r)$. Hence
$(\pi_{\mathcal{E}})_*\mu_{f_0} = P^{\rm cov}_{\mathcal{E}}$. $\square$

The proof is nearly trivial once the hypotheses are stated, and that is
the point: **its value is relocation, not resolution.** It removes an
item from the debt column by showing it was never a separate item.

### 1.3 What the theorem does *not* say

- It is **one-way**. R4b does not imply the Born pushforward: a selector
  could produce non-quantum yet foliation-independent statistics.
- It does **not** give (R4a), strong selector-history equivariance
  $\mu_g = (\tau_{f\to g})_*\mu_f$. That is a statement about
  *unobservable* history detail, and for a genuinely preferred-foliation
  ontology it is presumably false — the microscopic birth order really is
  foliation-dependent. R4a is not needed for empirical adequacy, and the
  extension document is right to keep it separate.
- Its scope is experiments in which **settings are interventions**. Under
  the boundary-relative enlargement in which the setting-selection
  mechanism is itself modelled physically, measurement independence
  becomes an extra hypothesis, and §1.4 is exactly what bites.

### 1.4 Corollary: the barrier is Bell, not covariance

> **Corollary 1.** Let a substrate's four CHSH observables be functions on
> a single sample space under one setting-independent measure. Their
> pushforward is then a joint distribution over $\{\pm1\}^4$, so by Fine's
> theorem $|S|\le2$. Since the quantum prediction is $2\sqrt2$, hypothesis
> **(H1) fails** for such a substrate on Bell experiments — and therefore
> so does R4b.

Verified by exhaustive enumeration (`verify_hiding_reduction.py`): the
maximum of $|S|$ over all $2^4$ deterministic joint assignments is exactly
$2$, against the attained quantum $2.828427$, a gap of $0.828427$. The
same script confirms the (H2) half at machine precision: over 400 random
states and settings, $|p(A\!\prec\!B) - p(B\!\prec\!A)| \le 2.2\times
10^{-16}$ for spacelike local instruments.

**Reading.** *If a substrate reproduced quantum statistics, its preferred
foliation would automatically be hidden.* The preferred foliation is not
the problem. Reproducing the statistics is the problem, and the obstacle
is the Bell/Fine trap already registered at FTD-0796 — a constraint that
has nothing to do with relativity. The two live postures (measurement
independence, contradicted by laboratory correlations; or measurement
dependence with $S \le \min(2+3M,4)$ and $M$ unpinned) are unchanged by
anything proved here.

**Consequence for the debt column.** The relativity extension's §8 listed
two independent proofs owed by Route III. There is only one, and it is
upstream of relativity entirely.

---

## 2. Proof 2 — Lorentz recovery, free sector, exactly

### 2.1 The stencil and its symbol

The production Laplacian is the 18-point operator
$$\mathrm{Lap}(f) = \tfrac13\!\!\sum_{\rm face}\!f
+ \tfrac16\!\!\sum_{\rm edge}\!f - 4f ,$$
whose plane-wave symbol is exactly
$$L(\mathbf{k}) = \tfrac23(c_1+c_2+c_3)
+ \tfrac23(c_1c_2+c_2c_3+c_3c_1) - 4 ,
\qquad c_i = \cos k_i .$$

### 2.2 Result: isotropy through fourth order, by construction

Exact expansion (`derive_lorentz_free_sector.py`, rational arithmetic):

| order | coefficient | isotropic? |
|---|---|---|
| $k^2$ | $-(k_1^2+k_2^2+k_3^2)$ | yes: $-k^2$ |
| $k^4$ | $\tfrac1{12}(k_1^2+k_2^2+k_3^2)^2$ | **yes: $\tfrac1{12}(k^2)^2$** |
| $k^6$ | $-\tfrac1{360}\big(\sum k_i^6 + 5\sum_{i\neq j}k_i^4k_j^2\big)$ | **no — leading anisotropy** |

The fourth-order term is a perfect function of $k^2$. The control makes
the mechanism explicit: the naive 7-point stencil has $k^4$ coefficient
$\tfrac1{12}\sum_i k_i^4$, which is *not* a function of $k^2$ and is
therefore anisotropic at fourth order. **The twelve edge terms are
exactly what cancels it.** This is a structural property of the operator,
not a tuned parameter.

### 2.3 The residual anisotropy, in closed form

Evaluating the sixth-order term along the cubic extremal directions:
$$L^{(6)}\big|_{\langle100\rangle} = -\frac{k^6}{360}, \qquad
L^{(6)}\big|_{\langle110\rangle} = -\frac{k^6}{240}, \qquad
L^{(6)}\big|_{\langle111\rangle} = -\frac{11k^6}{3240},$$
so with $\omega^2 = C^2(-L)$ the fractional direction-dependent phase
velocity is
$$\boxed{\ \left|\frac{\Delta v}{v}\right|_{\langle100\rangle-\langle111\rangle}
= \frac{(ka)^4}{3240} + O\big((ka)^6\big)\ }$$
with $a$ the lattice spacing. Note the *fourth*-power suppression: the
sixth-order symbol term becomes a fourth-order velocity effect after
dividing by $k^2$.

### 2.4 Numerical size

With $a = \ell_P$ (so $ka = E/E_P$, $E_P = 1.22\times10^{28}$ eV):

| probe | $E/E_P$ | $\|\Delta v/v\|$ |
|---|---|---|
| optical photon, 2 eV | $1.6\times10^{-28}$ | $2.2\times10^{-115}$ |
| Fermi-LAT GeV photon | $8.2\times10^{-20}$ | $1.4\times10^{-80}$ |
| highest observed $\gamma$, $\sim$PeV | $8.2\times10^{-14}$ | $1.4\times10^{-56}$ |
| UHECR primary, $10^{20}$ eV | $8.2\times10^{-9}$ | $1.4\times10^{-36}$ |

Modern optical-cavity isotropy tests reach $|\Delta c/c| \lesssim
10^{-18}$; astrophysical dispersion and birefringence bounds on the
dimension-six photon sector are far tighter still. **The free-sector
anisotropy is many tens of orders of magnitude below every existing
bound, at every observed energy.** LR-0 (free-sector dispersion) is
therefore satisfied with enormous margin, and the margin is structural
rather than fitted.

### 2.5 The radiative-stability problem, reduced

*(Added 2026-08-08; artifact `scripts/experiments/temporal_interior/derive_radiative_reduction.py`.
This section does **not** solve the naturalness problem for multi-species
effective field theories. It locates which part of that problem a
single-substrate theory actually inherits.)*

The Collins–Perez–Sudarsky–Urrutia–Vucetich argument is that a
Planck-suppressed dimension-6 Lorentz-violating operator, inserted in a
loop whose integral is dominated by the cutoff, generates a
**dimension-4** Lorentz-violating operator with coefficient of order
$\alpha/\pi$ rather than $(E/\Lambda)^2$. Since dimension-4 bounds sit at
$10^{-17}$–$10^{-23}$, the required tuning is $\sim10^{-20}$ or worse.
Three exact facts change what this means here.

**(A) The stencil's isotropy is forced, not tuned.** For a general
$O_h$-symmetric 18-point operator with face weight $w_f$ and edge weight
$w_e$, imposing (i) the correct continuum limit, $w_f + 4w_e = 1$, and
(ii) an isotropic $k^4$ term, gives a *unique* solution:
$$w_f = \tfrac13, \qquad w_e = \tfrac16, \qquad
\text{$k^4$ coefficient} = \tfrac1{12}(k^2)^2 .$$
These are the production weights. The isotropy of §2.2 is therefore a
structural consequence of symmetry plus normalisation, with no free
parameter to tune away.

**(B) The dimension-4 anisotropic coefficient is exactly zero, by
symmetry.** Averaging a general symmetric quadratic form $c_{ij}k_ik_j$
over all 48 elements of $O_h$ leaves $c_{ij} \propto \delta_{ij}$
(verified explicitly). A cubic-invariant quadratic form *is* isotropic.
The dangerous anisotropic dimension-4 operator is not small — it is
forbidden.

**(C) The observable content of dimension-4 violation is relational.**
For a single propagating sector with $\omega^2 = c_0^2(1+\epsilon)k^2$,
the rescaling $t \to t/\sqrt{1+\epsilon}$ removes $\epsilon$ entirely:
that sector's speed *is* the definition of the speed, and there is
nothing to compare it against. With two sectors the same rescaling leaves
$(1+\epsilon_2)/(1+\epsilon_1)$, so only the **difference** survives.
This is why every experimental bound on dimension-4 Lorentz violation is
a bound on a relative quantity — photon against matter, or one species
against another.

> **Reduction.** The radiative-stability problem is a statement about
> *differential* renormalisation: loops in one sector shift its cone
> relative to another's. Its observable content therefore requires at
> least two independently renormalised propagating sectors. A theory
> whose excitations are all configurations of **one** substrate field
> does not automatically inherit it; what it inherits instead is the
> question of whether its emergent sectors share a cone — which is
> exactly the already-registered common-cone item LR-2.

Two further points keep the reduction honest.

*The dimension-6 term is not enhanced.* Collins-type enhancement comes
from converting a $1/\Lambda^2$ coefficient into a dimensionless one by
gaining $\Lambda^2$ from the loop. A dimension-6 operator renormalising
*itself* gains nothing: corrections are $\sim(\alpha/\pi)a^2$, the same
order as the tree value, because there is no larger scale to be lifted
to. The computed dispersion $\omega^2 = C^2k^2(1 - (ka)^2/12 + \cdots)$
and anisotropy $(ka)^4/3240$ are therefore stable against this mechanism
in a way the dimension-4 coefficient would not have been.

*The deferral is real and dated.* The moment the framework derives
charged matter with its own radiative structure — independently
renormalised sectors with independent couplings — the Collins problem
returns in full, and no argument here will avert it. The correct ledger
entry is **deferred, and it binds at the emergence of a second
independently renormalised sector**, not *solved*.

### 2.6 What does *not* close, and why

Four items of the LR-0..LR-6 contract remain open, and one of them is a
genuine external obstruction rather than unfinished work.

1. **Radiative stability — reduced in §2.5, not eliminated.** The
   dimension-4 channel is closed three ways: forbidden by cubic symmetry,
   unobservable for a single sector, and not enhanced in the dimension-6
   self-renormalisation. What remains is the common-cone question (item 2
   below), and the standing deferral: the problem returns in full at the
   emergence of a second independently renormalised sector.
2. **A common limiting cone across sectors.** The derivation above is for
   the flux sector alone. Matter, flux, and gravitational sectors must
   share one limiting speed; the corpus records no established live
   interacting common cone, and $C_{\rm SPEED} = 1/\sqrt3$ is a declared
   selection rather than a forced saturation.
3. **Interacting matching.** Free-sector isotropy says nothing about
   vertices; interaction terms carry their own anisotropies.
4. **Operational clocks and rods.** Composite boosts must be checked at
   the level of the objects that actually measure — which is where the
   clock and register programmes meet the Lorentz question.

**Verdict on Proof 2:** LR-0 is delivered exactly and with a wide margin;
the contract as a whole is **open**, and its hardest item is a known
external problem.

---


### 2.7 The one-body / two-body split (owner's observation, formalized)

*(Added 2026-08-08; artifact
`scripts/experiments/temporal_interior/derive_onebody_twobody_split.py`. Origin: the
owner's observation that "relativity is the temporal comparison of two
separate bodies.")*

Both reductions above dissolved an apparently relativistic obligation
into something not about relativity. The common cause is structural.

**Rotations are the little group of one body's rest frame; boosts are
not.** Acting on the rest four-velocity $u=(c,\mathbf 0)$, a spatial
rotation leaves it fixed while a boost maps it to a different frame
(verified symbolically). Hence the Lorentz group splits, exhaustively and
without overlap, by *how many bodies a test requires*:

| | one body (rotations) | two bodies (boosts) |
|---|---|---|
| compared | apparatus with itself, reoriented | one frame against another |
| tests | isotropy, sidereal variation | time dilation, cone equality, tidal deviation |
| status | **closed** ($O_h$ forces isotropy; residual $(ka)^4/3240$) | **open** |

This is exactly the seam our results fell along. §2.5(B) was an argument
about $O_h$ — a *rotation* group — and §2.3's residual is a one-body
observable. Every surviving item is a boost statement and every one is
two-body: common cone, interacting vertices, composite boosts. §2.5(C)
says it from the other side: one cone can be scaled to unity, while a
*ratio* of two cones is invariant under every global rescaling
(verified) and cannot be defined away.

> **Proposition (the boost half is clock-gated).** Every boost observable
> is a functional of at least two proper-time parametrizations — time
> dilation compares $\dd	au_1$ with $\dd	au_2$; velocity composition
> relates two frames; geodesic deviation is defined by the separation of
> two worldlines, a single freely falling body detecting nothing by the
> equivalence principle. A theory that has not constructed a physical
> clock cannot pose the boost tests at all: their observables are
> undefined in it, not merely unverified.

**Consequence for the programme.** The recovery contract has been posed
as a field-theory question — dispersions, operator dimensions, radiative
corrections. Most of it is a *metrology* question. The one-body register
is closed with numbers; the two-body register is open and gated on the
clock construction of `ANALYSIS_MINIMUM_VIABLE_CLOCK_CARRIER_v1.md`,
which the native single-scale search did not find. This is a more useful
statement of where the programme is stuck than "Lorentz invariance is
open", because it names the object whose absence is the obstruction —
and it is the *same* object the Page–Wootters reading requires for the
stage index. One construction, two obligations.

The reading extends past special relativity: curvature is defined
operationally by geodesic deviation, so gravity's observable content is
two-body as well and inherits the same gate.

---

## 3. Net effect on the debt column

| Obligation | Before | After |
|---|---|---|
| (R4b) operational hiding | owed, independent | **discharged as a reduction**; implied by the Born pushforward |
| (R4a) strong equivariance | owed | not needed for empirical adequacy; presumably false for a preferred foliation, and correctly kept separate |
| Reproducing quantum statistics | listed as the Born pushforward | **the sole surviving barrier**, and it is Bell/Fine, not relativistic |
| LR-0 free-sector dispersion | owed | **delivered exactly**: isotropic to $O(k^4)$; residual $(ka)^4/3240$ |
| LR radiative stability | owed | **reduced**: dim-4 channel closed by symmetry + single-sector unobservability; remainder is the common-cone item, and the problem returns at a second renormalised sector |
| LR one-body (rotational) register | owed | **closed**: $O_h$ forces isotropy; residual $(ka)^4/3240$ |
| LR two-body (boost) register: common cone, interactions, composite boosts | owed | **split, 2026-08-08** — see the amendment below; one item was calculable and has been calculated |

### Amendment, 2026-08-08 — the two-body bundle splits

*Source: `ANALYSIS_MASSIVE_CONE_AND_DILATION_v1.md`,
`scripts/experiments/temporal_interior/derive_massive_cone_dispersion.py`.*

The row above bundled three items under one verdict, "open and
clock-gated," with the note that *the obstruction is a missing instrument,
not a missing calculation*. That note was true of the bundle only because
the bundle concealed an item that did not need an instrument. The three
do not share a blocker:

| item | status |
|---|---|
| **Common cone, free massive modes** | **Computed; not clock-gated.** The massive M18 dispersion gives an exactly isotropic but mass-dependent limiting speed $C_{\rm eff}(M) = C(1 + M^2/12 + 19M^4/1440 + \dots)$. Within a species the dilation relation $\Omega\sqrt{1-v_g^2/C_{\rm eff}^2}$ is constant to $O(k^4)$, residual $k^4/(36M^2) \simeq M^2\beta^4/4$. A **negative** result — species do not share a cone exactly — of size $1.6\times10^{-40}$ for the proton–electron pair. |
| **Common cone, across sectors** | **Open and failing at order unity** — FTD-0412 (`AUDIT_LORENTZ_COMMON_CONE_GATE.md`), $c^2 = 1/3$ vs $1/7$ vs $1$. Wholly untouched by the above, and not to be read as mitigated by it. |
| **Interacting vertices, composite boosts** | **Open and genuinely clock-gated.** Whether a bound *composite* inherits the dilation of its constituents' free dispersion is not settled by a free-mode calculation. |

Two notes carried from that analysis, because both were nearly missed.

**The MVC cannot serve as the test clock as written.** It is a mechanical
framework, $m\ddot q = -\nabla V(|q_i-q_j|)$, hence Galilean invariant:
boosting it gives $T(v) = T(0)$ exactly, so it would report zero dilation
regardless of what the substrate does.

> ⚠ **Corrected same day** — `ANALYSIS_POTENTIAL_VALIDITY_CLOCK_GATE_v1.md`.
> This note originally continued: *"the constructed carrier is the wrong
> kind of object, and a replacement must be bound by the substrate field
> rather than by a distance potential."* Both halves are wrong.
> Retardation of the binding is **not** what supplies dilation — hydrogen's
> Coulomb binding is instantaneous to $O(\alpha^2)$ and atoms dilate
> exactly. Dilation comes from the *constituents'* dispersion, exactly as
> the composite result shows ($\delta_{\rm comp}$ averages the
> constituents' $\delta_a$; the binding supplies only the weights).
> The MVC is therefore **under-modelled, not categorically unsuitable**:
> its nodes were given Newtonian $p^2/2m$ dispersion, and replacing that
> with the lattice dispersion is a substitution, not a carrier search. The
> discarded physics is $(v/C)^2 = 2.4\%$ at $A=0.30$, not $10^{-40}$.
>
> The obstruction that *does* survive is different: at every amplitude the
> MVC's internal frequency lies inside the propagating band
> ($\omega < 2\arcsin C = 1.2310$), so such a carrier radiates. That is the
> recorded C2 band-clearance requirement, and it is now the sole identified
> blocker on the composite-boost row.

**The free-sector figure is independently reproduced.** The same expansion
yields $(v_{[100]}-v_{[111]})/C = -k^4/3240$ exactly, matching the LR-0
row above by a different route.

The honest summary is that the two proofs were not symmetric. One was an
illusion — it dissolves into a problem already on the books, and the
dissolution shows that problem is not about relativity. The other is real,
half-delivered, and its remaining half is hard for reasons the whole field
shares.

**Nothing here moves a canonical tag.** In particular the Bell posture
contradiction stands exactly as registered, $C_{\rm SPEED}$ remains a
selection, and physical Lorentz invariance remains `[OPEN]`.
