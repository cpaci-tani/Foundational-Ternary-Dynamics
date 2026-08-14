# AUDIT — G* and the Clock: Exact Spine, Maintained Reference Model, and Open Physical Bridge

**Status:** `[SYNTHESIS — EXACT IDENTITIES AND CONDITIONAL CLOCK THEOREMS]` +
`[IMPOSED REFERENCE MODEL — DIMENSIONAL ENERGY AUDIT IMPLEMENTED]` +
`[CLOSED NEGATIVE — NON-RESCALABLE G* SIGNATURE IN THE REGISTERED INTERNAL LINEAR CLOCK FIELD]` +
`[EXACT CONDITIONAL RECURSION — FTD-0840]` +
`[EXACT CONDITIONAL LOCAL VECTOR LIFT — FTD-0841]` +
`[EXACT CONDITIONAL GLOBAL CLOSURE + LOCALIZATION OBSTRUCTION — FTD-0842]` +
`[EXACT SELECTED P4-LOCAL RELATIVE CARRIER — FTD-0844]` +
`[OPEN — PRODUCTION CROSS-GRADIENT, FORMATION/READOUT, TICK MATCHING, CM SYNCHRONIZATION, AND BORN PUSHFORWARD]`

**Scope:** comprehensive clock-facing audit of the existing G* corpus. This
document introduces no LEDGER row and promotes no physical claim. It
distinguishes identities that hold for every quartic oscillator from claims
that would make G* a constant of FTD time. The companion exact certificate is
`scripts/proofs/proof_gstar_clock_deep_dive.py`; the maintained reference model
is `scripts/experiments/temporal_interior/maintained_gstar_clock_v1.py`.

**External anchors:** the lemniscatic integral identities are standard
[DLMF elliptic-integral material](https://dlmf.nist.gov/19.20); purely
quartic circuit elements are physically realizable as selected engineered
systems, for example the
[quarton](https://doi.org/10.1103/PhysRevLett.127.050502); and the CM
split/inert reduction criterion is the classical Deuring theory summarized
in the project by the point-counting certificate for $E:y^2=x^3-x$.

---

## 0. Result in one paragraph

G* is exactly the dimensionless period coefficient of a symmetric pure
quartic oscillator, equivalently the archimedean real period of the
lemniscatic CM curve after a fixed normalization. It is **not** the integer
global update order, an energy per tick, or a Born probability. A local
G*-clock is a maintained degree of freedom at the codimension-one critical
surface where the quadratic restoring term vanishes. Its cycle law, waveform,
thermal moments, and semiclassical action all contain G* exactly. But internal
dimensionless comparisons of a homogeneous clock with its own coupled phase
wave cancel the period normalization. G* survives operationally only when the
clock is compared to an independently fixed second structure—an external
tick, calibrated mass/coupling/amplitude, or a genuinely substrate-derived
rate. No current FTD dynamics fixes that matching. **FTD-0827 subsequently
closes the mathematical gearbox conditional on the selected critical quartic
law:** its energy shell `y^2=1-x^4` maps by
`(u,v)=(x^-2,-yx^-3)` to `v^2=u^3-u`, with
`du/(2v)=dx/y`, so the oriented clock differential and the conductor-32
Hecke/Frobenius system are one algebraic object. What remains open is physical:
the production substrate does not yet maintain the critical clock or realize
prime-indexed channels as operational local clocks.

**FTD-0840 subsequently closes the isolated recursive dynamics conditional on
one new coupling:** retain `(q,p)`, set `u=q|q|`, and adopt
`H=p^2/(2m)+lambda q^4=lambda(u^2+y^2)`. The registered symmetric
discrete-gradient map is globally deterministic, exactly conservative,
reversible, strictly oriented, and bounded. No bath is needed for this
isolated neutral stability. The pair coupling is absent from production, the
modal carrier is not local hardware, and the map differs from exact
finite-time quartic flow. Its exact continuum `G*` factor therefore does not
close tick matching.

**FTD-0841 subsequently closes the local canonical-type question but not the
interaction:** production voxels already carry `(J,W)`, while
`U=J otimes J` obeys `||U||_F^2=|J|^4`. Conditional on adopting
`lambda||U||_F^2`, the vector symmetric discrete gradient is globally unique,
energy- and angular-momentum-conserving, reversible, oriented, and bounded.
Each linearly polarized invariant sector is exactly the FTD-0840 clock and
inherits its continuum `G*` factor. Generic angular-momentum sectors are not
pure quartic clocks, cubic symmetry alone does not force the radial coupling,
and the production spatial field energy is not closed together with this
onsite term.

**FTD-0842 closes that combined energy only at global reference scope.** The
symmetric simultaneous discrete gradient is unique, reversible, and exactly
conserves edge plus onsite energy, but its exact inverse is dense on a
connected quotient. It is therefore not a one-Moore-shell ontic update.
Moreover, positive edge energy has only the spatially constant zero mode, so
every nonzero bounded profile has a quadratic stiffness and cannot be the
exact critical-quartic clock. This removes the simple single-positive-field
route; local energy transactions and a bounded zero/soft relative mode are
now separate live requirements.

**FTD-0844 supplies both requirements as one selected two-channel witness.**
The rank-one edge metric makes only the common mode propagate and leaves the
relative mode onsite. The common production tick invariant plus relative
quartic energies close exactly, the dependency radius is one Moore shell,
and one polarized relative site stays compact. Production has `b=0` rather
than the required `b=a`; exact decoupling also prevents readout. The carrier
is therefore selection-scoped and local, while formation, common--relative
energy exchange, and finite-tick synchronization remain open.

---

## 1. Symbol and convention firewall

Several historical files use incompatible quartic normalizations. This audit
uses

\[
H(q,p;\mu)=\frac{p^2}{2m}+\frac12\mu q^2+\lambda q^4,
\qquad m>0,\quad\lambda>0.
\tag{1}
\]

The critical clock is $\mu=0$. Its turning amplitude is $A>0$, and its
energy is $E=\lambda A^4$. When another document writes
$(p^2+q^4)/2$, it has selected $m=1$ and $\lambda=1/2$. Coefficients must be
translated before period values are compared.

The time quantities are:

| symbol | meaning | status |
|---|---|---|
| $n\in\mathbb Z$ | global substrate update order | `[AXIOM]` |
| $t$ | continuous parameter of the imposed reference Hamiltonian | `[IMPOSED]` |
| $\phi$ | unwrapped local oscillator phase | `[CONSTRUCTION]` after a clock exists |
| $T$ | local cycle period in $t$ | `[THEOREM]` for the selected Hamiltonian |
| $k_x$ | count of compliant Poincare-section crossings | `[CONSTRUCTION]` |
| $\rho$ | relative Hamiltonian rate per substrate tick | `[IMPOSED/OPEN]` |

No symbol in this table may silently replace another.

---

## 2. What G* is mathematically

Define

\[
G^*:=\frac{\Gamma(1/4)}{\Gamma(3/4)}
=\frac{\Gamma(1/4)^2}{\sqrt2\,\pi}
=\frac{2\varpi}{\sqrt\pi}
=2\sqrt\pi\,G_{\rm Gauss}.
\tag{2}
\]

Here $\varpi$ is the Bernoulli/Gauss lemniscate constant and
$G_{\rm Gauss}=1/\operatorname{AGM}(1,\sqrt2)$. The load-bearing period
identity is

\[
\int_0^1\frac{du}{\sqrt{1-u^4}}
=\frac14B\!\left(\frac14,\frac12\right)
=\frac{\sqrt\pi G^*}{4}
=\frac{\varpi}{2}.
\tag{3}
\]

Related exact faces include

\[
L(E_i,1)=\frac{\sqrt\pi G^*}{8}=\frac{\varpi}{4},
\qquad
W_{\rm BCC}=\frac{G^{*2}}{2\pi},
\tag{4}
\]

for the lemniscatic curve $E_i:y^2=x^3-x$ and the Watson BCC integral under
the project's conventions.

Equation $\pi=4\varpi^2/G^{*2}$ is an algebraic rearrangement of (2). It is
not a non-circular derivation of $\pi$ from two independently π-free inputs:
both $\varpi$ and $G^*$ are conventionally defined through period/gamma data
whose equivalence uses π.

### 2.1 Finite approximation is a limit representation, not a finite clock

The FQCR sequence

\[
G_N^*=(N+1)^{-1/2}
\prod_{j=0}^{N}\frac{j+3/4}{j+1/4}
\tag{5}
\]

converges to $G^*$ with its registered $O(N^{-2})$ acceleration. Each term is
a finite algebraic number, not generally a rational number because of the
square root. Equation (5) supplies a finite approximation scheme; it does
not make $N$ a physical clock time or show that a finite FTD region computes
that product.

---

## 3. The critical quartic clock

At $\mu=0$, energy conservation gives

\[
T=4\int_0^A\frac{dq}{\sqrt{2(E-\lambda q^4)/m}}.
\tag{6}
\]

With $q=Au$ and $E=\lambda A^4$, (3) yields

\[
\boxed{
T(A,m,\lambda)A
=\sqrt\pi G^*\sqrt{\frac{m}{2\lambda}}.}
\tag{7}
\]

Therefore

\[
\omega(A)=\frac{2\pi}{T}
=\frac{2\sqrt\pi}{G^*}A\sqrt{\frac{2\lambda}{m}}.
\tag{8}
\]

The clock is maximally nonisochronous: frequency is proportional to its own
amplitude. Amplitude noise becomes phase noise. That is poor horology but
good threshold sensing.

### 3.1 Action and the clock/ensemble bridge

The enclosed phase-space area is

\[
J(E)=\oint p\,dq
=\frac23G^*\sqrt{2\pi m}\,\lambda^{-1/4}E^{3/4},
\qquad \frac{dJ}{dE}=T(E).
\tag{9}
\]

The identity (dJ/dE=T) is the exact bridge between the orbit family and
equilibrium phase-space weighting:

\[
Z_{\rm family}(\beta)=\int T(E)e^{-\beta E}\,dE.
\tag{10}
\]

This is a valid ensemble relation. It does not assert that quantum terminal
records obey Born frequencies.

### 3.2 Exact CM gearbox, conditional on the selected clock

With `x=q/A` and `y=p/(sqrt(2m lambda) A^2)`, the positive-energy shell is

\[
 y^2=1-x^4.
\]

FTD-0827 proves the exact rational map

\[
 (x,y)\longmapsto
 \left(u=\frac1{x^2},\ v=-\frac{y}{x^3}\right),
 \qquad v^2=u^3-u,
\]

and the differential identity

\[
 \frac{du}{2v}=\frac{dx}{y}.
\]

The opposite sign of `v` reverses the differential. The quartic Hamiltonian's
signed swept area is strictly negative on every positive-energy orbit, so the
declared forward flow fixes this orientation. The smooth quartic curve is
birational to `32a1` and degree-two isogenous to the fixed `32a2` curve;
therefore its archimedean clock period and finite-prime CM calendar are one
compatible conductor-32 system. See
[`DERIV_QUARTIC_CLOCK_CM_GEARBOX_v1.md`](../derivations/native_time_carrier_programme/DERIV_QUARTIC_CLOCK_CM_GEARBOX_v1.md).

This is `[CONDITIONAL THEOREM]` content because the quartic clock is the input.
It does not repair the open native-maintenance and operational-realization
debts.

---

## 4. Why the quartic appears at an edge

For a reflection-symmetric one-coordinate system,

\[
V(q;\mu)=\frac12\mu q^2+\lambda q^4+O(q^6).
\tag{11}
\]

At a generic stable point $\mu>0$, the local clock is harmonic and π-valued.
At the codimension-one surface $\mu=0$, reflection symmetry forbids odd
terms, so a positive quartic is the generic leading term. Thus:

- sitting at the threshold requires one condition, $\mu=0$;
- given the threshold and reflection symmetry, the quartic shape is generic;
- $\lambda>0$ is essential; a subcritical negative quartic is not a stable
  clock without higher-order stabilization.

G* is therefore the period coefficient of **supercritical marginal
stability**, not of every system containing a quartic interaction.

---

## 5. The detuned clock and exact distance from threshold

For $\mu\ne0$, define the dimensionless detuning

\[
\boxed{\delta=\frac{\mu}{2\lambda A^2}}.
\tag{12}
\]

This corrects the dimensionful shorthand $\mu/A^2$. The elliptic modulus is

\[
k^2=\frac{2\lambda A^2}{\mu+4\lambda A^2}
=\frac{1}{\delta+2}.
\tag{13}
\]

The critical clock is the self-dual point $k^2=1/2$. Across the symmetric
orbit family,

\[
T A\sqrt{\frac{2\lambda}{m}}=4kK(k).
\tag{14}
\]

Equations (13)--(14) turn a defect—amplitude-dependent period—into a sensor
for $\mu$. They also show why the controller must estimate a dimensionless
detuning rather than merely rename a coefficient.

---

## 6. Exact signatures of a quartic clock

These signatures test the selected Hamiltonian, not FTD provenance.

| channel | exact critical result | meaning |
|---|---|---|
| period | $TA\sqrt{2\lambda/m}=\sqrt\pi G^*$ | dynamic period coefficient |
| log slope | $d\log\omega/d\log A=1$ | quartic rather than harmonic clock |
| waveform | $\mathcal B_4=48\pi/G^{*4}$ | dimensionless orbit-shape discriminator |
| thermal variance | $\langle q^2\rangle=(\beta\lambda)^{-1/2}/G^*$ | static G* recovery |
| Binder ratio | $\langle q^4\rangle/\langle q^2\rangle^2=G^{*2}/4$ | calibration-free critical value |
| heat capacity | $C=3k_B/4$ | generalized equipartition |
| semiclassical spectrum | $E_n\propto[(n+1/2)/G^*]^{4/3}$ | widening pure-quartic ladder |

Observing all of these in a tabletop or circuit system would strongly verify
that the device realizes a critical quartic mode. It would not show that FTD
space-time uses that mode; every correctly engineered quartic oscillator must
obey the same mathematics.

---

## 7. What maintenance actually means

The lightweight v2 contextual-actualization witness tracks amplitude,
detuning, phase, and a nominal work accumulator. It is adequate as an API
separation test but not as a physical energy audit: its
`operational_duration` is incremented by phase, and its prior work counter
adds a detuning-coordinate correction to an amplitude-coordinate correction.
Those quantities do not share units.

The maintained reference model added with this audit evolves (1) directly.
Its two controls are:

1. **criticality loop:** change $\mu$ toward zero;
2. **shell loop:** at the positive-going (q=0) Poincare section, restore the
   energy corresponding to target amplitude $A_0$.

Changing $\mu$ at fixed $q$ performs exact parameter work

\[
\Delta W_\mu=\frac12q^2\Delta\mu.
\tag{15}
\]

Damping is an exact momentum split, and the shell correction is booked as
the kinetic-energy change. Every step satisfies

\[
\Delta H
=W_{\rm disturbance}+W_{\mu,\rm controller}+W_{A,\rm controller}
-E_{\rm dissipated}+R_{\rm integrator}+\epsilon_{\rm balance}.
\tag{16}
\]

The tests require $\epsilon_{\rm balance}$ to remain at floating-point
roundoff scale.

The recorded controller work is work exchanged with the oscillator. It is a
lower bound on the controller's own energetic cost: actuator inefficiency,
sensing, computation, and energy spent while changing a parameter at a node
where $q=0$ are not modeled. “Energy closure” here means closure of the
oscillator ledger, not a thermodynamic model of the complete apparatus.

### 7.1 Maintenance cost is conditional, not automatic

If a perfectly isolated clock is placed exactly at $\mu=0$ with no damping,
it needs no continuing controller work. Persistent cost arises only when
there is persistent detuning drift, damping, measurement backaction, or
noise. Therefore the statement “a G*-clock must be actively maintained” is
physically appropriate for a real critical device but is not a theorem of the
autonomous quartic Hamiltonian. The disturbance model must be named.

### 7.2 Gate semantics

A compliant gate is a positive-going (q=0) section crossing satisfying both

\[
|\delta|\le\epsilon_\delta,
\qquad
\left|\frac{A_{\rm est}}{A_0}-1\right|\le\epsilon_A.
\tag{17}
\]

This makes the gate physical and phase-local. The clock determines when an
event is eligible; it has no access to measurement settings, effects,
probabilities, selector state, or outcomes.

---

## 8. Where G* cancels—and where it can survive

Even the isolated period law does not give G* a non-rescalable physical role
when the quartic coupling is free. Defining

\[
\lambda_{\rm eff}=\lambda/G^{*2}
\]

rewrites (7) as

\[
T=\frac{\sqrt\pi}{A}\sqrt{\frac{m}{2\lambda_{\rm eff}}}.
\]

Thus a measured period identifies the combination $G^*/\sqrt\lambda$, not
G* separately, unless $\lambda$ is fixed by an independent construction.

For a homogeneous even-power clock, any dimensionless comparison built only
from its own cycle, energy, and a coupling expressed as a ratio to that energy
loses the normalization constant. In the registered coupled-clock field,

\[
\left(\frac{c_{\rm clock}}{\Omega\ell}\right)^2
=d_R\eta\frac{m_{\rm power}-2}{2m_{\rm power}},
\tag{18}
\]

so the quartic specialization is $d_R\eta/4$ and contains no G*. This is the
exact `GSTAR_LINEAR_SIGNATURE_ABSENT` result.

Comparing the imposed quartic clock with an independently named substrate
interval does retain G*:

\[
d_4=\frac{\text{one substrate interval}}{T_4}
=\frac{\rho}{u}\frac{(2E)^{1/4}}{\sqrt\pi G^*}
\tag{19}

under the normalization of FTD-0771. But $\rho$, the shell $E$, Hamiltonian
coefficients, and support/transport speed role are not fixed by P1--P5.

The decision rule is:

> A non-rescalable physical role for G* requires a second structure whose
> normalization is fixed independently of the quartic oscillator.

Setting that second structure using G* would be circular.

---

## 9. Discrete time and the resolution ratio

For the pure quartic velocity-Verlet map, the transformation

\[
(q,p,\Delta t)\mapsto(sq,s^2p,\Delta t/s)
\tag{20}
\]

is an exact equivariance. Because (T(sA)=T(A)/s), all raw timestep effects
depend on

\[
\rho_{\rm disc}=\frac{\Delta t}{T(A)}.
\tag{21}
\]

The existing exploratory campaign measures instability bands near the
quartic point, not one exact universal cutoff. The initial apparent
two-clock constants $1/G^*$ and $1/3$ were both killed by finer resolution
and phase controls; resonance-channel fine structure survived. This is a
useful anti-numerology result. The next analysis should derive stability
tongues from the exact lemniscatic carrier rather than hunt a special number.

Equation (21) is the clean interface to a discrete ontology: a substrate
candidate must operate in a registered stable region of $\rho_{\rm disc}$
and must show convergence or a native discrete law. A continuous auxiliary
clock sampled at integer ticks is not yet a discrete clock derivation.

---

## 10. The CM local/global bridge

The curve $E_i:y^2=x^3-x$ has complex multiplication by $\mathbb Z[i]$.
G* is an archimedean period ratio associated with that CM object. Rational
primes expose its finite-place behavior:

| prime | Gaussian behavior | norm/trace data | normalized phase |
|---|---|---|---|
| (p=2) | ramified | special bad/ramified place | finite exceptional case |
| $p\equiv1\pmod4$ | split $p=\pi\bar\pi$, $\pi=a+ib$ | $N\pi=p$, $a_p=\pi+\bar\pi$ up to curve convention | $\pi/\sqrt p=e^{i\theta_p}$ |
| $p\equiv3\pmod4$ | inert | $N(p)=p^2$, $a_p=0$ at good reduction | $\widehat\Pi_p^2=-1$ |

For an inert prime,

\[
\mathbb Z[i]/(p)\cong\mathbb F_{p^2},
\qquad
(a+bi)^p=a-bi.
\tag{22}
\]

Thus residue Frobenius is conjugation and has order two. On the CM elliptic
curve, trace zero gives

\[
\Pi_p^2+p=0,
\qquad
\widehat\Pi_p:=\Pi_p/\sqrt p,
\qquad
\widehat\Pi_p^2=-1,
\quad \widehat\Pi_p^4=1.
\tag{23}
\]

This exact four-phase normalized cycle is genuinely clock-like. Split phases
$e^{\pm i\theta_p}$ are generally not finite-order and supply the dispersive
angular bulk.

The useful distinction is therefore **phase rigidity versus angular
dispersion**. A split prime chooses a Gaussian factor $a+ib$ and hence a
variable angle $\theta_p$; an inert prime has no representation
$p=a^2+b^2$ and therefore no freely varying Gaussian hand. Its visible
residue action alternates $z\leftrightarrow\bar z$ with period two, while the
oriented normalized CM lift retains the hidden quarter-turn period four.
This makes the inert sector a natural arithmetic phase reference, not yet a
physical oscillator.

### 10.1 What the CM bridge does not yet say

Equations (2)--(4) and (22)--(23) are two localizations of one CM arithmetic
object: archimedean period and finite-prime Frobenius. They do **not** supply
a physical law transporting a local FTD state through those primes, nor a
map synchronizing Frobenius iteration with global ticks or quartic phase.

A valid synchronization result would need one substrate-defined operator
whose finite reductions yield the split/inert characteristic polynomials and
whose archimedean realization yields the quartic flow, without choosing the
operator because it reproduces G*. That construction is open.

---

## 11. The Gaussian cone interpretation

The arithmetic cone uses

\[
\tau_G=\sqrt{N(a+ib)}.
\tag{24}
\]

Inert primes lie on axial null rays with $\tau_G=p$; split primes lie off-axis
with $\tau_G=\sqrt p$. Hence inert points form an integral axial skeleton
while split points dominate the large-norm angular bulk.

This geometry is not the raw FTD causal cone. For the 26-neighbor Moore
graph,

\[
d_{26}(x,y,z)=\max(|x|,|y|,|z|),
\tag{25}
\]

whose frontier is cubic. A Euclidean Gaussian cone can be an abstract
potentiality geometry or an operationally recovered cone only after an
isotropy/Lorentz bridge is demonstrated.

---

## 12. G*, actualization, and Born separation

The only admissible present role is:

\[
G^*\ \text{sets the selected quartic period factor}
\ \Longrightarrow\ \text{clock crossing decides eligibility}.
\tag{26}
\]

The selector separately decides which record is written. Neither the period
identity nor the quadratic norm identity derives physical Born frequencies.

The legacy constants

\[
k_{\rm crit}=4/G^*,
\qquad x_{\rm degenerate}=2G^*
\tag{27}
\]

are exact facts about a parameterized algebraic quadratic at zero
discriminant. Naming the second quantity `X_BORN`, or saying a null cone
“derives the Born rule,” does not establish normalization, additivity,
context overlap, equilibrium measure, or terminal-basin pushforward. Those
claims are superseded on the v2 branch by the explicit
`PREREG_CONTEXTUAL_BORN_RECOVERY_v1.md` debt.

---

## 13. Claim audit

| claim | strongest justified status | evidence/guard |
|---|---|---|
| $G^*=\Gamma(1/4)/\Gamma(3/4)$ and equivalent period identities | `[THEOREM]` | gamma reflection, beta/elliptic identities |
| every pure quartic oscillator has the G* period factor | `[THEOREM]` | quadrature (6)--(7) |
| signed pair energy admits a unique reversible energy-closed discrete recursion | `[THEOREM — CONDITIONAL ON ADOPTED COUPLING]` | FTD-0840 exact certificate `24/24` |
| local self-pair tensor admits a unique reversible energy/angular-momentum-closed vector recursion | `[THEOREM — CONDITIONAL ON ADOPTED RADIAL COUPLING]` | FTD-0841 exact certificate `26/26`; only polarized sectors inherit scalar `G*` |
| positive edge plus onsite self-pair admits exact simultaneous energy closure | `[THEOREM — CONDITIONAL GLOBAL REFERENCE MAP]` | FTD-0842 exact certificate `26/26`; exact inverse dense, not a one-tick P4 update |
| positive edge plus positive onsite quartic supplies a bounded exact critical mode | `[CLOSED NEGATIVE — REGISTERED SINGLE-FIELD ARCHITECTURE]` | FTD-0842: connected edge kernel is spatially constant; bounded nonzero profiles have positive quadratic stiffness |
| rank-one common propagation plus onsite relative quarticity supplies a compact P4-local carrier | `[THEOREM — SELECTED TWO-CHANNEL CONSTRUCTION]` | FTD-0844 repaired `28/28`; production `b=0`, selected boundary `b=a`; formation/readout/cadence open |
| the FTD-0840 recursion is the exact finite-time quartic flow | `[CLOSED NEGATIVE — EXACT SERIES CONTROL]` | turning-point coefficients differ at `h^3/h^4` |
| a symmetric supercritical threshold is generically quartic once $\mu=0$ | `[THEOREM/standard normal form]` | reflection forbids odd terms; $\lambda>0$ required |
| G* is global time | **rejected wording** | global time is integer order (n) |
| G* is energy processed per tick | `[SELECTION/INTERPRETATION]` only | no dimensional/native rate derivation |
| G* supplies a selected local clock cadence | `[SELECTION]` | exact after Hamiltonian, shell, coefficients, and rate are selected |
| current P1--P5 derive a G*-clock | `[OPEN]`; prior scoped attempts negative | no native positive action/shell/rate |
| G* has a non-rescalable internal linear-clock signature | `[CLOSED NEGATIVE — scoped]` | exact cancellation (18) |
| maintained reference controller is energetically coherent | `[IMPOSED MODEL + TESTED ACCOUNTING]` | equation (16), focused tests |
| inert primes have order-two residue and order-four normalized CM phase | `[THEOREM]` | equations (22)--(23) |
| selected quartic clock and conductor-32 CM calendar share one oriented algebraic operator | `[CONDITIONAL THEOREM]` | FTD-0827 direct map and differential pullback, exact certificate `22/22` |
| production substrate physically realizes the CM calendar | `[OPEN]` | native critical maintenance and prime-indexed operational channels absent |
| G* derives Born frequencies | `[OPEN/NOT ESTABLISHED]` | clock gate and outcome selector are separate |

---

## 14. Canonical reconciliation status

The following conflicts found during this audit are now reconciled without
changing numerical APIs:

1. `engine/include/ftd/ontic/lemniscate.h`, its JS/Python mirrors, and the
   engine audit now identify G* as a lemniscatic gamma ratio and exact quartic
   period coefficient. `GSTAR_FLUX`, `GSTAR_TIME`, `GSTAR_ACTION`, `K_CRIT`,
   and `X_BORN` remain compatibility names, with adjacent nonclaim guards.
2. The generalized-quadratic comments and proof labels now state only the
   exact real/repeated/complex discriminant trichotomy. They no longer claim
   to derive $i$, fermions, measurement, or the Born rule.
3. The lightweight contextual-actualization clock explicitly labels
   `operational_duration`/`local_duration` as an accumulated phase parameter
   and its feedback fields as dimensionless diagnostics. Physical mechanical
   accounting is delegated to `maintained_gstar_clock_v1.py`.
4. `gstar_compendium_verify.py` no longer presents the retired
   $x_-\leftrightarrow N_c$ identification as a live result.
5. The two-clock note's stale “derive $\chi_c=1/3$” queue item is replaced by
   resonance-channel/KAM analysis; both pretty-constant claims remain closed
   negative.
6. The legacy Hilbert/Born checker now states its actual scope: conditional
   state-effect and toy-model consistency, not target-free substrate
   frequency recovery.

Broader pre-v2 scripts whose filenames preserve historical language remain
provenance artifacts. They are not admissible evidence for a G* clock unless
their live output carries the guards above. This reconciliation changes no
physical claim and supplies no new clock carrier.

---

## 15. Successor experiments, in order

### G1 — Maintained-reference disturbance campaign

Freeze damping, deterministic detuning disturbances, controller gains,
integration resolution, and tolerances. Measure detuning recovery, gate
jitter, amplitude stability, signed controller work, dissipation, and
balance residual. Include harmonic and sextic controls. This establishes the
engineering cost of the selected clock, not native FTD recovery.

### G2 — Native phase/action search

Before looking for G*, require a substrate observable to provide:

1. a state-functional periodic phase;
2. a positive conjugate action or invariant shell label;
3. persistence under transport and interaction;
4. a target-blind formation law;
5. a frequency-amplitude exponent discriminating harmonic, quartic, and
   sextic behavior.

Only after the exponent selects quartic may the coefficient be compared with
the G* period law.

FTD-0840 supplies the reference recursion against which such a native carrier
must now be compared. It removes numerical-instability and free-maintenance
ambiguities from the reference side; it does not supply the production-local
`lambda u^2` interaction or a finite-tick rotation-number theorem.

FTD-0841 sharpens that comparison: the local phase-space variables are already
present in production, so the next missing type is not another local register.
It is a target-blind local energy rule that combines the selected onsite
`lambda|J|^4` term with the existing spatial gradient energy, plus a native
mechanism for bounded support and polarization. A split integrator does not
inherit exact conservation of the combined Hamiltonian automatically.

FTD-0842 now resolves the first clause at global mathematical scope and
refutes its simplest physical reading. Exact simultaneous closure requires a
globally dependent solve, while the positive edge form excludes a bounded
zero-stiffness mode. The next reference must therefore test a genuinely local
energy-current/edge-register architecture and a relative-mode mechanism whose
softness is structural rather than tuned from the `G*` target.

FTD-0844 supplies the minimal exact reference: its rank-one channel metric is
the unique positive boundary with a soft relative mode in the registered
two-channel quadratic class. What is missing is no longer compact recurrence
or P4-local accounting at selected scope. It is physical provenance and
coupling: why production selects `b=a`, how the relative carrier forms, and
how its phase is read without destroying the sector ledger or support.

### G3 — Tick-matching campaign

With a candidate native clock, preregister the relative rate, occupied shell,
support/transport cone, and held-out perturbations. Reject any “derivation”
that fixes $\rho$, $A$, $m$, or $\lambda$ by demanding the G* answer.

### G4 — CM local/global operator

Seek one algebraically defined, substrate-motivated transfer operator whose
finite reductions reproduce split/inert Frobenius classes and whose real
period problem is lemniscatic. This is a structural existence problem, not a
numerical prime-pattern search. Failure leaves the CM connection arithmetic
only.

### G5 — Operational cone and Born firewall

Test preferred-tick hiding and common-cone recovery independently. Only after
the gate passes those tests may it be coupled to the contextual selector.
Outcome frequencies remain governed by the separate, locked Born-recovery
campaign; the clock may not read target weights.

---

## 16. Stop conditions

Demote G* to a quartic period normalization, with no actualization role, if:

1. no native phase/action carrier survives the target-blind gate;
2. all admissible dimensionless observables cancel G* or reduce to free
   normalization choices;
3. maintained gating leaks context or enables signalling;
4. tick matching requires setting a rate or shell from the desired answer;
5. the Euclidean cone cannot be operationally recovered from Moore dynamics;
6. CM synchronization exists only as post-hoc prime plotting;
7. Born recovery requires target-coded weights.

The exact quartic and CM mathematics survives every such failure. What would
close is the physical identification.

---

## 17. Reproduction

```text
python scripts/proofs/proof_gstar_clock_deep_dive.py
python -m pytest scripts/tests/test_maintained_gstar_clock_v1.py scripts/tests/test_gaussian_prime_cone.py scripts/tests/test_contextual_actualization_v2.py -q
python scripts/proofs/proof_coupled_quartic_clock_field.py
python scripts/proofs/proof_quartic_clock_rod_synchronization.py
python scripts/experiments/temporal_interior/derive_gstar_degeneracy_price.py
ctest --test-dir engine/build -C Release -j 24 --output-on-failure -R "(ontic_chain|coupled_quartic_clock_field|contextual_actualization)"
python scripts/verification/verify_index_links.py
git diff --check
```

The first certificate covers 22 exact identities. The maintained model tests
period recovery at three amplitudes, a harmonic amplitude-independent
control, dimensionless detuning, feedback recovery, damping compensation,
persistent and sparse held-out disturbance patterns, compliant gates, and
energy balance. None of these runs is a numerical near-miss search.

---

## 18. Completion audit for the deep-dive deliverable

This table audits the research deliverable, not the still-open claim that the
FTD substrate physically contains a G* clock.

| requirement | authoritative evidence | verdict |
|---|---|---|
| define G* and firewall neighboring constants | Sections 1--2; 64/64 fixed compendium identities | **proved/checked** |
| derive the critical quartic period and action law | Sections 3--4; 22/22 exact certificate; three-amplitude integration | **proved/checked** |
| state the correct dimensionless detuning and crossover | Section 5; symbolic modulus identity; focused test | **proved/checked** |
| separate global tick, local phase, and compliant gate count | Sections 1 and 7; isolated Python/C++ interfaces | **implemented/checked** |
| make maintenance energetics dimensional | Section 7; Hamiltonian ledger; damping, persistent, and sparse-pulse tests | **implemented reference model** |
| include controls and falsify normalization-only readings | harmonic control; coupling absorption; internal linear cancellation; clock--rod underdetermination | **closed negative at registered scope** |
| decode split/inert Gaussian periods and cone geometry | Sections 10--11; residue order 2, normalized CM order 4; exact visualizer tests | **proved arithmetically** |
| distinguish the arithmetic cone from the Moore causal cone | equation (25) and visualization note | **proved distinction; recovery open** |
| keep clock eligibility separate from Born outcome weights | Section 12; contextual-actualization tests; retagged legacy interfaces | **architecture checked; physical Born recovery open** |
| reconcile live G*/time/Born labels without breaking APIs | Section 14; rebuilt engine targets; ontic-chain test | **completed for audited surfaces** |
| publish falsifiers and the next target-blind programme | Sections 15--16 | **completed** |
| verify code, proofs, navigation, and whitespace | commands in Section 17 | **all passing on 2026-08-10** |

### Final boundary

The deep dive is complete at its honest altitude. It establishes what G* is
the constant of, which periods the inert sector carries, how an imposed clock
can be maintained and audited, and why current internal observables do not
give G* a non-rescalable physical role. It does **not** establish a native
substrate carrier, independent tick matching, CM-to-dynamics synchronization,
operational Lorentz hiding, or target-free Born frequencies. Those are the
explicit next research programme, not silent assumptions of this result.
