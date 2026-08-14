# Native Phase/Action Carrier and CM Realization Operator v1

**Status:** `[PRE-REGISTRATION v1.2 — PRE-EXECUTION NORMALIZATION CORRECTION;
LOCAL HASH LOCK; OWNER COMMIT PENDING]`  
**Programme:** native time carrier / G* clock  
**Date:** 2026-08-10  
**Question:** Can one already-defined substrate operator simultaneously provide
a target-blind phase/action carrier with independently fixed coupling and tick,
and realize the same Gaussian-CM object whose archimedean period contains `G*`
and whose finite-prime realizations are Frobenius operators?

## 0. Epistemic firewall

This campaign is an exact structural test, not a numerical search. It may use
symbolic algebra, fixed-curve point counts, exact recurrence relations, and
fixed-source engine checks. It must not:

- fit a stencil, coupling, tick rate, CM discriminant, elliptic curve, prime
  subset, period normalization, or local-factor sign after seeing `G*`;
- treat a numerical near miss as evidence;
- call the BCC operator production-native when its production coefficient is
  zero;
- call a standard CM elliptic motive substrate-derived merely because its
  period equals a lattice Green value;
- identify a modal free-field phase with a localized maintained physical clock;
  or
- evaluate the Euler product naively at `s=1`; the central value is defined by
  analytic continuation (or an independently certified convergent formula).

The possible outcome is deliberately asymmetric: one candidate may supply the
native carrier while another supplies the arithmetic realization. That would
be a useful decomposition, but it is not the requested single-operator
closure.

## 1. Candidate order locked before construction

The operator candidates are tested in the following order. Failure may not be
used to alter the order or introduce a tuned mixture.

### C18 — production 18-point free-field operator (primary)

Use exactly the source-free production kick-drift map

\[
 W_{n+1}=W_n-K_{18}J_n,\qquad
 J_{n+1}=J_n+W_{n+1},
\]

with `K_18=-C_WAVE^2 L_18`, face weight `1/3`, edge weight `1/6`,
corner weight `0`, and `C_WAVE=1/sqrt(3) [SELECTED]`. The global tick is
the primitive engine update `Delta n=1`.

### C26 — equal-weight full Moore operator (control)

Use all 26 neighbours with one common neighbour weight and the corresponding
center weight fixed by annihilation of constants. Normalize the nonzero weight
only by the same stability convention used for the kick-drift carrier. No
weight may be changed to improve a period match. C26 is a Moore-ontology
control, not the production wave operator.

### CBCC — pure body-diagonal operator (structural control)

Use the eight offsets `(+-1,+-1,+-1)` and the normalized symbol

\[
 \sigma_{\rm BCC}(k)=1-\cos k_x\cos k_y\cos k_z.
\]

CBCC is registered third because it is structurally present in the Moore
neighbourhood but excluded from production propagation. It may pass the CM
gate only as a structural/imported operator unless a pre-existing production
coupling is found without changing the engine.

No SC/FCC/BCC interpolation and no fitted linear combination is admissible in
v1.

## 2. Locked native carrier construction

For any registered symmetric nonnegative spatial operator `K` and one real
nonzero mode with eigenvalue `0<a<4`, use the exact one-tick matrix

\[
 U_a=\begin{pmatrix}1-a&1\\-a&1\end{pmatrix},\qquad
 \cos\theta_a=1-\frac a2,\qquad
 \sin\theta_a=\sqrt{a(1-a/4)}.
\]

The phase/action coordinates are locked as

\[
 Q_a=\sqrt{\sin\theta_a}\,J,
 \qquad
 P_a=\frac{W-aJ/2}{\sqrt{\sin\theta_a}},
\]

\[
 Z_a=Q_a+iP_a,\qquad
 I_a=\frac{|Z_a|^2}{2},\qquad
 \phi_a=\arg Z_a.
\]

The certificate must establish, without reading `G*`,

\[
 \{Q_a,P_a\}=1,\qquad
 Z_a(n+1)=e^{-i\theta_a}Z_a(n),\qquad
 I_a(n+1)=I_a(n).
\]

Equivalently, with

\[
 H_a=\frac12\left(W^2-aJW+aJ^2\right),
\]

the action must be `I_a=H_a/sin(theta_a)`. The zero mode and band edges are
excluded because they do not define this elliptic phase coordinate.

### Carrier acceptance gates

A candidate passes the carrier gate only if all hold:

1. `J` and `W` are pre-existing state variables and their pairing follows from
   a discrete Legendre transform, not from an inserted complex amplitude;
2. `I_a` is positive and exactly invariant for every stable nonzero mode;
3. the phase advances by the operator's own `theta_a`, with no target phase;
4. the coupling entering `a(k)` is fixed by the registered source operator
   before `G*` or the CM curve is consulted;
5. the tick is the primitive update `Delta n=1`; and
6. exact engine-mode fixtures agree with the analytic map within floating-point
   roundoff.

Passing this gate licenses **native modal phase/action carrier**, not local
clock, matter clock, actualization gate, or physical SI time.

## 3. Independently fixed coupling and rate

For C18 the exact Fourier symbol is to be derived from the frozen production
weights, not copied from a desired dispersion relation:

\[
 L_{18}(k)=\frac23(c_x+c_y+c_z+c_xc_y+c_yc_z+c_zc_x)-4,
\]

where `c_mu=cos(k_mu)`, and

\[
 a_{18}(k)=-C_{\rm WAVE}^2L_{18}(k),\qquad
 \omega_{18}(k)=\theta_{a_{18}(k)}\;\text{radians/tick}.
\]

The certificate must distinguish:

- **source-fixed:** once production and `C_WAVE` are adopted, `a(k)` and
  `theta(k)` have no remaining fit parameter;
- **postulate-derived:** not claimed, because `C_WAVE=1/sqrt(3)` is currently
  `[SELECTED]`; and
- **physical-unit calibrated:** not claimed, because seconds per tick are not
  fixed by this construction.

An outcome cannot pass this section by absorbing `G*` into a free coupling.

## 4. Locked global CM object

The arithmetic candidate is fixed before local data are evaluated:

\[
 E/\mathbb Q:\quad y^2=x^3-x,
 \qquad \omega_E=\frac{dx}{2y},
 \qquad \operatorname{End}_{\overline{\mathbb Q}}(E)=\mathbb Z[i].
\]

No other curve or quadratic field may be substituted in v1. The genuine
global object is the rank-two motive `M_E=H^1(E)` and its compatible
realizations, not a list of unrelated prime formulas.

### Archimedean realization

The certificate must distinguish the least positive real cycle period from
the BSD real volume. In the locked Neron-differential normalization it must
prove

\[
 \Omega_{\min}=\varpi=\frac{\Gamma(1/4)^2}{2\sqrt{2\pi}},
 \qquad
 \Omega_{\rm BSD}=2\varpi,
 \qquad
 G^*=\frac{2\Omega_{\min}}{\sqrt\pi}.
\]

The factor two in `Omega_BSD` is required because `E(R)` has two connected
components. The certificate must independently verify `c_2=2`,
`|E(Q)_tors|=4`, and then certify

\[
 L(E,1)=\frac{\Omega_{\rm BSD}c_2}{|E(\mathbb Q)_{\rm tors}|^2}
       =\frac{(2\varpi)(2)}{16}
       =\frac{\varpi}{4},\qquad
 G^*=\frac{8L(E,1)}{\sqrt\pi}.
\]

### Finite-prime realization

For every odd prime `p`, define Frobenius on a fixed integral cohomology basis
up to conjugacy by its characteristic polynomial

\[
 P_p(T)=\det(TI-F_p)=T^2-a_pT+p,
 \qquad
 a_p=p+1-\#E(\mathbb F_p).
\]

The normalized determinant-one companion representative is locked as

\[
 R_p=\frac1{\sqrt p}
 \begin{pmatrix}0&-p\\1&a_p\end{pmatrix}.
\]

It must satisfy

\[
 \det R_p=1,\qquad
 \operatorname{tr}R_p=a_p/\sqrt p,
\]

with conjugate eigenvalues `exp(+-i theta_p)`. The CM split is:

- `p=3 mod 4`: `a_p=0`, hence `R_p^2=-I` and `R_p^4=I`;
- `p=1 mod 4`: `p=pi_p conjugate(pi_p)` in `Z[i]` and
  `a_p=pi_p+conjugate(pi_p)` for the conductor-32 primary Hecke character.

The sign of `a_p` must come from point counting or the fixed primary-character
convention, never from choosing a quadrant after inspecting the result.

### One operator-level bridge

The bridge is accepted only in the following precise sense:

\[
 L(E,s)=\prod_{p\ne2}\det(I-F_pp^{-s})^{-1}
\]

for `Re(s)>3/2`, followed by the certified analytic continuation to `s=1`,
where its central value gives the archimedean period above. Equivalently, the
same weight-two newform/Hecke eigensystem supplies all `a_p` and its Mellin
transform supplies `L(E,s)`.

This is called the **CM realization operator**: the compatible Hecke/Frobenius
action on `M_E`, with the archimedean comparison period. It is not a literal
time evolution of the FTD engine.

### Arithmetic acceptance gates

1. The curve, differential, conductor, bad prime, and period normalization are
   fixed and mutually consistent.
2. Exact point counts reproduce the locked `a_p` recurrence and CM inert/split
   laws for a deterministic prime prefix and held-out primes.
3. Inert primes satisfy the exact order-four relation after normalization.
4. The Euler factors agree with the coefficients of the one fixed newform.
5. The period and central-value identities are certified independently of any
   framework-integer subset.
6. No physical significance is assigned to selected primes such as
   `{3,7,13,47}` in this campaign.

## 5. Single-operator closure gate

The full requested closure passes only if one of C18, C26, or CBCC satisfies
both the carrier gates and the arithmetic gates with the *same operator* and
without an added stencil-selection or coupling-selection rule.

The following exact controls are mandatory:

- derive the C18 lattice-period differential operator and test the necessary
  self-duality/polarization conditions for an elliptic or symmetric-power CM
  realization;
- test the equal-weight C26 period against the same invariant conditions;
- verify the production BCC coefficient is exactly zero; and
- distinguish a BCC Green function equal to `G*^2/(2pi)` from the linear
  elliptic period `G*` and state the required symmetric-square/square-root
  relation.

An equality of one special value is insufficient. The local system, Euler
factors, or an exact motive-preserving map must agree.

## 6. Verdict map

- `NATIVE_CM_SINGLE_OPERATOR_CLOSED`: one registered candidate passes every
  carrier, coupling/tick, arithmetic, and same-operator gate.
- `NATIVE_CARRIER_CM_OPERATOR_SPLIT`: C18 passes native carrier/coupling/tick,
  the fixed CM motive passes the arithmetic bridge, but no registered native
  operator realizes that motive.
- `BCC_CM_STRUCTURAL_SELECTION_REQUIRED`: CBCC passes the period/motive gate
  but has zero production weight or requires a new coupling selection.
- `NO_TARGET_BLIND_NATIVE_CARRIER`: no candidate passes the carrier gate.
- `CM_REALIZATION_CERTIFICATE_FAILED`: the fixed curve's period/local-factor
  normalization cannot be certified consistently.
- `PROTOCOL_INVALID`: source hashes, definitions, candidate order, or
  target-blind firewall were changed after execution.

The split verdict is not a failure of the arithmetic construction. It is a
boundary result: a native dynamical phase/action carrier and a genuine global
CM operator exist, but their identification would remain an additional type.

## 7. Frozen source corpus

The v1 lock refers to these current files and SHA-256 values:

| Source | SHA-256 |
|---|---|
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/include/ftd/ontic/gauge_couplings.h` | `BC862D8120E0F3D83B7FAD0201F8D4DF46B5BAD5E7D52CD571AF68BECA3EB0F3` |
| `engine/include/ftd/sublattice.h` | `3D0903987D7FF97AFFE203C0C9C5FCA826BD2FEABB9D457C6660D8B821C689E9` |
| `THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md` | `2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C` |
| `SPEC_ALPHA_DYNAMICAL_BOUNDARY.md` | `EA5295FF581A38669A0573AF1D58AB7C685AA5C9CF951A00DA5D0C278AF10128` |
| `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` | `245A2F97F71BF72C6CA49352E238E65A1A379CC2B10A3E000AE06D76EB9EB5BB` |
| `DERIV_LFUNCTION_GSTAR_CONNECTION.md` | `A7900118651DB1126EAA36B1EA167D24B10D6146A1CF94E9784015D6CB810473` |
| `explr_stencil18_selfduality_derived.py` | `AC0A362810A00929A2388B933A964300D0CFF67916B2B389416F2920F6424B4F` |
| `_stencil18_operator.json` | `22F0809C4EF477E3CCDA874C9466D8BEC522274EEF138A983E9646352FC910DE` |

Hash drift does not make a mathematical theorem false, but it invalidates an
execution claimed under this v1 source lock.

### Pre-execution correction provenance

The initial local prefix hash
`53566A1A5CF5A00CFE21BDCE175DDDD3C16F4BD0B2FAB409DC86F7C812337EA0`
locked `omega_E=dx/y` while also locking the `varpi` period appropriate to the
Neron differential `dx/(2y)`. A fixed-curve Sage audit caught the factor-two
inconsistency before a certificate or campaign output existed. Version 1.1
therefore corrects the differential and separates the least real period from
the two-component BSD real volume. No candidate order, target-blind gate, or
verdict threshold changed.

The v1.1 prefix hash
`6B8DFFA98108140AA31F32E1575E07183421F1E95673D44A134E96CE7696A1C2`
was also pre-execution. Version 1.2 synchronizes the frozen source hash after
the same normalization correction was applied to
`DERIV_LFUNCTION_GSTAR_CONNECTION.md`; it changes no campaign gate.

## 8. Required deliverables

1. an exact independent proof/certificate for the modal action-angle map;
2. a small isolated `engine/include/ftd/eft/` carrier interface and native test;
3. a fixed-curve CM realization certificate covering periods, point counts,
   Frobenius matrices, Hecke recurrence, and Euler factors;
4. exact C18/C26/CBCC same-operator controls;
5. a canonical analysis with one verdict from section 6; and
6. synchronized programme index, preregistration manifest, LEDGER/open-item
   bookkeeping, and link checks.

`protocol_sha256=8BE09323F54424C51EA96B2589D532559CC54C4656DE39DEE0626DD6C5EC09F5`
