# FTD-0846 — Swap-parity phase readout and the odd-pointer minimum

**Status:** `[THEOREM — COMMON/EVEN READOUT PARITY OBSTRUCTION]` +
`[THEOREM — POSITIVE QUADRATIC CRITICALITY OBSTRUCTION]` +
`[THEOREM — SCOPED DEGREE-FOUR ODD-POINTER MINIMUM]` +
`[THEOREM — EXACT LOCAL REVERSIBLE ENERGY TRANSACTION]` +
`[SELECTION/OPEN — ODD POINTER TYPE, TERNARY RECORD MAP, PRODUCTION COUPLING, AND CADENCE COMPLIANCE]`  
**Date:** 2026-08-10  
**Programme row:** `FTD-0846`  
**Invalid parent:** FTD-0845, `31/32`; sign-reordered squared-expression
comparison defect; no theorem booked  
**Repair protocol:**
[`PREREG_SWAP_PARITY_PHASE_READOUT_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_SWAP_PARITY_PHASE_READOUT_CERTIFICATE_REPAIR_v2.md),
pre-run SHA-256
`C769DFBF4125CCFE864D85B4CD604A8793FE51DF8F03B9672D07E67F0DB025AF`  
**Repaired certificate:**
[`proof_swap_parity_phase_readout_v2.py`](../../../../../scripts/proofs/proof_swap_parity_phase_readout_v2.py),
SHA-256
`F7E7C7D3C901F3CF80F2FB8B4A222DBE2897320664300EC5F868465A46B56C5C`,
`32/32 PASS`  
**Production impact:** none

## 0. Result

FTD-0844's exact decoupling protects its compact relative quartic clock but
prevents physical readout. FTD-0846 gives the first exact type-level answer
to that readout problem.

There are two distinct readout targets:

1. the **symmetric-square phase**, which identifies opposite half-cycles; and
2. the **faithful phase**, which retains the signed relative sheet and hence
   the full cycle.

A common, exchange-even pointer can read the first but not the second. A
positive quadratic coupling to a faithful signed pointer necessarily adds
quadratic stiffness to the clock, destroying exact critical quarticity. In
the registered autonomous polynomial position-coupling class, the first
positive faithful interaction is therefore degree four:

\[
 W_-(r,q)=\frac\kappa4(r-q)^4.                 \tag{1}
\]

With a selected exchange-odd pointer `(r,pi)`, (1) gives a unique,
reversible, onsite discrete update that conserves the complete
clock--pointer--interaction energy exactly. Its pointer force reads the sign
of `q`; its local history reads the velocity/crossing direction. The
interaction is not free: its energy current and phase torque are explicit.
Exact readout without energy destruction is possible; exact readout without
backreaction is not.

This is a reference construction, not a production mechanism. The odd
pointer is a newly selected type. The production ternary alphabet contains a
signed record value, but no law yet maps the continuous pointer into a stable
ternary record.

## 1. The parity boundary

On one fixed-polarization relative clock site, let `(q,p)` be canonical. The
left/right channel exchange acts as

\[
 S(q,p)=(-q,-p).                               \tag{2}
\]

Let `a` be common/even under the same exchange. An analytic position
interaction compatible with exchange symmetry obeys

\[
 V_+(a,q)=V_+(a,-q).                           \tag{3}
\]

Its Taylor expansion contains only even powers of `q`; therefore
`-partial_a V_+` is also even in `q`. No such instantaneous pointer force can
distinguish `(q,p)` from `(-q,-p)`.

The canonical positive control is

\[
 W_+(a,q)=\frac\kappa2(a-q^2)^2.               \tag{4}
\]

At `a=0`,

\[
 -\partial_a W_+=\kappa q^2.                  \tag{5}
\]

Equation (4) preserves the zero `q` Hessian at the origin and reads the
symmetric-square coordinate exactly. It is blind to `q -> -q`. This is the
precise local form of the FTD-0839/BCC orientation loss: a common/even record
can carry phase modulo `pi`, but it cannot carry the faithful `2pi` sheet.

This does not prohibit every momentum-dependent interaction. A gyroscopic
or history-dependent common pointer may read exchange-even currents such as
`q p`. It still cannot recover the signed sheet from common/even
instantaneous data, and it falls outside the registered position-polynomial
minimum theorem.

## 2. Why the bilinear repair fails

For a signed pointer `r`, the most general quadratic position energy is

\[
 V_2(r,q)=\frac a2r^2+b\,rq+\frac c2q^2.      \tag{6}
\]

Its Hessian is

\[
 K_2=\begin{pmatrix}a&b\\b&c\end{pmatrix}.   \tag{7}
\]

Positive semidefiniteness requires

\[
 a\ge0,\qquad c\ge0,\qquad b^2\le ac.         \tag{8}
\]

Exact critical quarticity requires no quadratic clock stiffness, so `c=0`.
Equation (8) then forces

\[
 \boxed{b=0}.                                  \tag{9}
\]

The usual positive lock `(r-q)^2` makes the price visible: it includes a
nonzero `q^2` term. A bilinear faithful readout can be positive or can keep
the exact critical clock, but cannot do both in this class.

Every nonzero homogeneous odd-degree polynomial changes sign under total
inversion and cannot be globally nonnegative. Degree one and degree three
therefore fail positivity/covariance, while degree two fails (9). This proves
the degree-four floor conditional on:

- autonomous polynomial position coupling;
- global nonnegativity;
- joint odd covariance `(r,q)->(-r,-q)`;
- zero clock Hessian at the origin; and
- nonzero signed pointer response.

It is not a no-go for non-polynomial, momentum-dependent, explicitly driven,
or dissipative readouts.

## 3. The degree-four faithful pointer

Select `(r,pi)` to be exchange odd together with `(q,p)`:

\[
 S(r,\pi,q,p)=(-r,-\pi,-q,-p).                \tag{10}
\]

Equation (1) is invariant under (10), nonnegative, and has zero gradient and
Hessian at the origin. Its pointer force is

\[
 F_r=-\partial_rW_-=-\kappa(r-q)^3.            \tag{11}
\]

For a pointer initialized at `r=pi=0`,

\[
 F_r=\kappa q^3.                               \tag{12}
\]

Thus the pointer acceleration is odd in the clock coordinate. Away from a
crossing, the response derivative contains the velocity sign:

\[
 \dot F_r=3\kappa q^2\frac p m.                \tag{13}
\]

At `q=0`, both (12) and (13) vanish. For initial
`r=pi=q=0`, Hamilton's equations instead give

\[
 r^{(5)}(0)=\frac{6\kappa p^3}{M m^3}.         \tag{14}
\]

The exact crossing direction is therefore present in the local pointer
history, not in an instantaneous zero-force sample. This is sufficient for a
finite-history gate but does not yet define a ternary record transition.

## 4. Closed local energy transaction

Use

\[
 H=\frac{p^2}{2m}+\frac{\pi^2}{2M}
   +\lambda q^4+\frac\kappa4(r-q)^4,
 \qquad m,M,\lambda,\kappa>0.                 \tag{15}
\]

The position map `(q,r)->(q,r-q)` is invertible, so the two positive fourth
powers in (15) make its position energy coercive. Every finite-energy shell
is compact in `(q,r,p,pi)`.

Define

\[
 G(x_0,x_1)=(x_1^2+x_0^2)(x_1+x_0),
 \qquad z_j=r_j-q_j.                           \tag{16}
\]

The selected symmetric discrete-gradient rule is

\[
 \frac{q_1-q_0}{h}=\frac{p_1+p_0}{2m},
 \qquad
 \frac{r_1-r_0}{h}=\frac{\pi_1+\pi_0}{2M},   \tag{17}
\]

\[
 \frac{p_1-p_0}{h}
 =-\lambda G(q_0,q_1)+\frac\kappa4G(z_0,z_1), \tag{18}
\]

\[
 \frac{\pi_1-\pi_0}{h}
 =-\frac\kappa4G(z_0,z_1).                    \tag{19}
\]

Since

\[
 G(x_0,x_1)(x_1-x_0)=x_1^4-x_0^4,            \tag{20}
\]

dotting (18)--(19) with the coordinate increments proves

\[
 \boxed{H_1=H_0}.                              \tag{21}
\]

The stronger three-account ledger is

\[
 E_q=\frac{p^2}{2m}+\lambda q^4,
 \quad E_r=\frac{\pi^2}{2M},
 \quad E_I=\frac\kappa4z^4,                   \tag{22}
\]

\[
 \Delta E_q=\frac\kappa4G_z\Delta q,
 \quad
 \Delta E_r=-\frac\kappa4G_z\Delta r,
 \quad
 \Delta E_I=\frac\kappa4G_z(\Delta r-\Delta q).\tag{23}
\]

The three terms in (23) sum identically to zero. The measurement interaction
has a visible energy current; no maintenance work is hidden.

## 5. Well-posedness, reversal, and locality

The secant `G` is symmetric in its endpoints, and

\[
 \partial_{x_1}G(x_0,x_1)
 =3(x_1+x_0/3)^2+2x_0^2/3\ge0.                \tag{24}
\]

After eliminating momenta, the endpoint Jacobian is the positive mass matrix
plus nonnegative rank-one contributions from the two quartics. Its determinant
is

\[
 \frac{4mM}{h^2}+2M\lambda a_q
 +\frac\kappa2(m+M)a_z
 +\frac{h^2\kappa\lambda}{4}a_qa_z>0,         \tag{25}
\]

where `a_q,a_z>=0` are the secant derivatives. The residual is strongly
monotone and coercive, hence has exactly one endpoint. Endpoint symmetry
makes the map physically time reversible.

Every equation is onsite. A carrier plus pointer initially supported at one
site remains supported there. Setting `kappa=0` returns the FTD-0844 relative
quartic recursion exactly and leaves a zero pointer inert.

This locality statement does not couple the pointer to the production common
field. Doing that would allow the record to radiate, and would require a new
combined common-action/energy proof.

## 6. Backreaction is the readout

The isolated clock's discrete orientation witness becomes

\[
 \begin{aligned}
 \chi_q={}&-h\left[
   \frac\lambda2(q_1+q_0)^2(q_1^2+q_0^2)
   +\frac{(p_1+p_0)^2}{4m}
 \right]\\
 &+\frac{h\kappa}{8}(q_1+q_0)G(z_0,z_1).       \tag{26}
 \end{aligned}
\]

The last term is the exact pointer torque. It can have either sign for
unrestricted pointer states. Therefore no `kappa>0` gives a universal strict
orientation theorem for the clock subsystem alone.

A sufficient per-step compliance condition is

\[
 \left|\frac{\kappa}{8}(q_1+q_0)G_z\right|
 <\frac\lambda2(q_1+q_0)^2(q_1^2+q_0^2)
  +\frac{(p_1+p_0)^2}{4m}.                    \tag{27}
\]

Under (27), `chi_q<0` survives that readout step. This is the honest
readout--disturbance trade: the clock remains bounded and total energy closes
for every state, but phase orientation and cadence require a declared
compliance envelope.

During readout, even the initial `r=0` interaction adds
`kappa q^4/4` to the instantaneous quartic coefficient. The `G*` shape factor
of a pure quartic remains, but its rate is shifted. The pointer cannot be
called a clock-neutral gate without a separate weak-coupling or compensated
cadence audit.

## 7. Certificate record

The FTD-0845 parent returned `31/32` because C9 compared
`(-a+q^2)^2` and `(a-q^2)^2` structurally. Their exact difference is zero.
FTD-0846 changed only that comparison and returned:

```text
FTD-0845 swap-parity phase readout: 32/32 PASS
COMMON_EVEN_POINTER_READS_ONLY_THE_SYMMETRIC_SQUARE_QUOTIENT
POSITIVE_BILINEAR_SIGNED_READOUT_DESTROYS_EXACT_CRITICALITY
QUARTIC_ODD_POINTER_IS_THE_SCOPED_DEGREE_MINIMUM_FAITHFUL_BRIDGE
LOCAL_REVERSIBLE_ENERGY_TRANSACTION_EXACT_BACKREACTION_ORIENTATION_COMPLIANCE_REQUIRED
FTD-0846 CERTIFICATE_REPAIR_ONLY_C9_EXACT_SIMPLIFIED_DIFFERENCE
```

## 8. What this means for the substrate

The substrate can distinguish clockwise from counterclockwise internally
because the retained `(q,p)` lift has a signed swept-area current. A
common/even symmetric-square record cannot retain the full phase sheet. The
smallest conservative faithful bridge found here adds an exchange-odd pointer
whose sign transforms with the relative channel.

The actual ternary state already has the right *alphabetic possibility* for
an odd record (`-1,0,+1`). That is not yet a mechanism. The next gate must
derive or select a context-blind transition from the continuous pointer
history to a persistent ternary record, book its dissipated/ exported energy,
and test whether the clock stays within the orientation and cadence envelope.

No claim is made that the pointer is biological, conscious, quantum, Born
distributed, production native, or an actualization selector.
