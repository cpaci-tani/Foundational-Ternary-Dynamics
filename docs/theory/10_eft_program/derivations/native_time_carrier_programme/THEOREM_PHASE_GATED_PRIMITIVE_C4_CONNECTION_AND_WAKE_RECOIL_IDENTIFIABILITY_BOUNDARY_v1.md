# Theorem — Phase-gated primitive C4 connection and wake-recoil identifiability boundary v1

**Identifiers:** `FTD-0937` parent; `FTD-0938` repaired proof of record  
**Date:** 2026-08-11  
**Status:** `[THEOREM — MINIMUM LOWEST-DEGREE NATIVE-DIRECTION CONNECTION IN THE REGISTERED CLASS]` +
`[THEOREM — EXACT CONDITIONAL SOURCE ENERGY/MOMENTUM/SWITCHING LEDGER]` +
`[SCOPED NO-GO — CLOSED SOURCE CYCLE CANNOT BE DIRECT-SUMMED WITH THE POSITIVE HOP WAKE]` +
`[THEOREM — DIRECTION PLUS WAKE ENERGY DOES NOT IDENTIFY REAL RECIPROCAL IMPULSE]` +
`[BOUNDARY — BACKREACTING LOCAL SOURCE-FIELD ACTION / CARRY OWNER / SCALE OPEN]`  
**Production status:** unchanged

## 1. Result

FTD-0936 closes the orientation input that remained conditional in the
FTD-0904 rectifier. At a declared local gate phase, the already-formed body
carries the live primitive current

\[
 u_n\in\{(-1,1,0),(-1,-1,0),(1,-1,0),(1,1,0)\},
 \qquad u_{n+1}=Su_n.                                  \tag{1}
\]

Inside the registered lowest-degree class, signed-cubic covariance,
linearity in `u`, evenness in the scalar critical-clock coordinate `q`, and
vanishing value and first derivative at `q=0` uniquely give

\[
 \boxed{A_g(q,u)=g\gamma q^2u,\qquad g\in\{0,1\}.}    \tag{2}
\]

The uniqueness is up to one real coefficient `gamma`. Equation (2) is not a
new selected vector: `u` is reconstructed from existing ternary state and
velocity. It is also not a derivation of `gamma`.

The corresponding conditional source action preserves a positive pure
quartic rest clock, exact canonical momentum, branch-paired reversal, and a
directed common displacement. But simply appending that closed source cycle
to the FTD-0933 field does not close a common action. A nonzero local integer
relocation leaves the strictly positive wake

\[
 \overline{\mathcal D}(d)=\|\pi(d)Y-Y\|_4^2>0,        \tag{3}
\]

while the closed source cycle returns its internal energy. With no incoming
or environmental debit, total energy therefore increases by (3).

Finally, the direction `u`, scalar wake (3), and compact C4 character do not
identify a unique real reciprocal impulse. Even if all of (3) is assigned to
an equal-and-opposite quadratic impulse, its magnitude depends on an
independent reduced inertia. The character independently retains the free
conversion `p_*`. The next missing object is therefore a backreacting local
source-field action with a named carry owner and normalization—not another
orientation bit or a post-hoc wake subtraction.

## 2. Minimum connection classification

At one stroboscopically selected phase, let `u` be the live current and
register a polar connection linear in that current:

\[
 A_g(q,u)=gB(q)u.                                      \tag{4}
\]

For every signed permutation `Q`, covariance requires

\[
 B(q)Q=QB(q).                                          \tag{5}
\]

The exact centralizer of all 48 signed cubic matrices in `M_3(R)` is

\[
 \{bI_3:b\in\mathbb R\}.                              \tag{6}
\]

Thus `B(q)=b(q)I_3`. In the registered degree-at-most-two class, write

\[
 b(q)=b_0+b_1q+b_2q^2.                                \tag{7}
\]

Evenness removes `b_1`; `A(0,u)=0` removes `b_0`; and the zero first
derivative at the critical point is then automatic. Renaming `b_2=gamma`
proves (2).

This is a scoped minimum theorem. Higher even powers, nonlinear functions of
additional native tensors, retained incoming currents, and nonanalytic laws
are outside the registered class. Their existence would not determine a
physical normalization either.

The exact transformation laws are

\[
 A_g(q,Qu)=QA_g(q,u),                                  \tag{8}
\]

\[
 A_g(-q,u)=A_g(q,u),                                   \tag{9}
\]

and, because both the current `u` and common velocity reverse under canonical
time reversal,

\[
 (-A_g)\cdot(-\dot C)=A_g\cdot\dot C.                 \tag{10}
\]

Thus the connection uses the exact information lost by the symmetric square
without introducing a global preferred direction.

## 3. Exact conditional source action

For `M,m,lambda>0` and frozen `g,u`, adopt the **[IMPOSED reference source
law]**

\[
 L_g={M\over2}|\dot C|^2+{m\over2}\dot q^2
     +g\gamma q^2u\cdot\dot C-\lambda q^4.            \tag{11}
\]

Its canonical and mechanical common momenta are

\[
 P=M\dot C+g\gamma q^2u,                              \tag{12}
\]

\[
 K=M\dot C=P-g\gamma q^2u.                            \tag{13}
\]

The positive Hamiltonian is

\[
 \boxed{
 H_g={|P-g\gamma q^2u|^2\over2M}
     +{\pi^2\over2m}+\lambda q^4.}                    \tag{14}
\]

Because `C` is cyclic, `P` is conserved, and therefore

\[
 \boxed{\Delta K=-g\gamma\Delta(q^2)u.}               \tag{15}
\]

At `P=0`, define

\[
 \Lambda_u=\lambda+{g\gamma^2|u|^2\over2M}.           \tag{16}
\]

Every body label in (1) has `|u|^2=2`, so on the active branch

\[
 \Lambda_u=\lambda+{\gamma^2\over M}>0.               \tag{17}
\]

The rest-sector Hamiltonian is exactly

\[
 \boxed{H_{g,P=0}={\pi^2\over2m}+\Lambda_u q^4.}       \tag{18}
\]

The connection contributes no quadratic clock Hessian. It therefore avoids
the continuous-linear-connection detuning obstruction of FTD-0901 while
retaining a directed mechanical exchange.

## 4. Gate work and conditional `G*` gearbox

Changing the gate from `g` to `g'` at fixed state changes (14) by

\[
 \Delta H_{g\to g'}=
 {|P-g'\gamma q^2u|^2-|P-g\gamma q^2u|^2\over2M}.     \tag{19}
\]

At `q=0`, equation (19) vanishes exactly. Away from the critical crossing it
is generally nonzero. A dynamically maintained gate must therefore acquire
and release the connection only at a registered zero or pay the exact
switching work.

For a nontrivial `P=0` orbit of turning amplitude `a`, (18) retains the
conditional continuum period law

\[
 Ta=\sqrt\pi G^*\sqrt{m\over2\Lambda_u}.              \tag{20}
\]

Since

\[
 \dot C=-{g\gamma\over M}q^2u,                        \tag{21}
\]

the exact quartic beta integral gives

\[
 \boxed{
 \Delta C_T=-{4\sqrt\pi\,g\gamma a\over M G^*}
 \sqrt{m\over2\Lambda_u}\,u.}                       \tag{22}
\]

Equation (22) is a directed continuum displacement inside the imposed law.
It does not force an integer Moore hop. Matching its magnitude to one lattice
step would constrain `gamma`, `a`, `M`, and `lambda`; none is fixed by `i`,
the character, or `G*`. The body C4 phase and the quartic traversal also
remain distinct clocks until a finite-tick synchronization law is derived.

After a full source cycle, `(q,pi,P,K)` and the source energy return while
`C` may have translated. This closed-cycle property is exactly what exposes
the field-composition obstruction below.

## 5. Direct-composition obstruction

Take a source initially accompanied by the fully formed FTD-0932 C4 field.
Suppose a closed source cycle realizes a nonzero integer displacement `d`.
Strict locality forbids translating the extended companion simultaneously.
Immediately after the local source relocation, the old field differs from
the new formed companion by

\[
 b_Y(d)=\pi(d)Y-Y,                                     \tag{23}
\]

and FTD-0933/0934 give

\[
 \overline{\mathcal D}(d)=\|b_Y(d)\|_4^2>0.           \tag{24}
\]

The local native field subsequently re-dresses the new center and carries
that positive invariant outward as a wake. If the source action (11) has
closed its cycle and no incoming/environmental store changes, then

\[
 \Delta E_{\rm source}=0,
 \qquad
 \Delta E_{\rm field}=\overline{\mathcal D}(d),
 \qquad
 \Delta E_{\rm total}=\overline{\mathcal D}(d)>0.     \tag{25}
\]

Therefore

\[
 \boxed{
 \text{closed source actuator}\ \oplus\
 \text{unchanged formed field}
 \quad\text{is not a common conservative action}.}    \tag{26}
\]

This is not a no-go against motion. It excludes the naive direct sum. An
admissible common action must do at least one of the following by its own
equations:

1. reduce a named source/internal store by (24);
2. consume a named incoming field/environmental store;
3. backreact on the source cycle so it does not return unchanged;
4. spread the translation over a different deforming route; or
5. reject the hop when the available local store is insufficient.

Writing `Delta E_source=-Dbar(d)` after the fact closes a scalar table but
does not supply a conjugate force, inverse transaction, carry update, or
locality proof. It is bookkeeping, not the missing action.

## 6. Recoil-scale identifiability theorem

Give the most favorable simple assumption to the recoil proposal: allocate
all wake energy to equal-and-opposite source and field impulses `+I,-I`
along the already-known direction. With positive quadratic inertias `M_s`
and `M_f`, define the reduced inertia

\[
 {1\over\mu}={1\over M_s}+{1\over M_f}.                \tag{27}
\]

Energy balance would give

\[
 \overline{\mathcal D}(d)
 ={I^2\over2M_s}+{I^2\over2M_f}
 ={I^2\over2\mu},                                     \tag{28}
\]

and hence

\[
 \boxed{|I|=\sqrt{2\mu\,\overline{\mathcal D}(d)}.}  \tag{29}
\]

For every `mu>0`, equation (29) gives a different impulse while preserving:

- the same live direction `u`;
- the same compact character;
- the same scalar wake energy;
- signed-cubic covariance;
- exact equal-and-opposite momentum cancellation; and
- exact quadratic energy balance.

For example, `M_s=M_f=2` gives `mu=1`, while `M_s=M_f=8` gives `mu=4`; the
second impulse is exactly twice the first for the same wake. Thus even the
strongest total-wake-to-kinetic allocation does not identify recoil without
an independent inertial law.

The compact momentum candidate has a separate scaling freedom:

\[
 P_{\rm candidate}=p_*[k+2\pi W].                      \tag{30}
\]

Every positive `p_*` leaves `exp(ik dot d)` and all wrap/carry identities
unchanged. The aggregate carry also admits inequivalent local partitions,
such as `(w_s,w_f)=(0,1),(1,0),(-1,2)` for the same `W=1`.

Consequently

\[
 \boxed{
 (u,\overline{\mathcal D},\Xi)
 \not\Longrightarrow
 (\gamma,\mu,p_*,\text{carry owner},I).}              \tag{31}
\]

`i` and the native current orient the gearbox. They do not normalize it.

## 7. What is closed and open

### Closed

1. `[THEOREM]` the live FTD-0936 primitive current supplies the polar,
   time-odd input of the minimum registered connection.
2. `[THEOREM]` signed-cubic covariance and the declared critical-point
   restrictions uniquely give `A_g=g gamma q^2 u` up to `gamma`.
3. `[THEOREM]` the conditional source action has a positive Hamiltonian,
   exact canonical momentum, exact mechanical impulse, and an undetuned pure
   quartic rest clock.
4. `[THEOREM]` gate-zero switching costs zero while off-phase switching is
   generally nonzero.
5. `[THEOREM]` the conditional full-cycle displacement is parallel to the
   live current and has the exact inverse-`G*` factor (22).
6. `[SCOPED NO-GO]` a closed source cycle cannot be direct-summed with the
   unchanged formed field after a nonzero local hop.
7. `[THEOREM]` direction plus scalar wake plus compact character does not
   identify a real reciprocal impulse, conversion scale, or carry owner.

### Open

1. a local source-centered field action that creates and pays the wake by
   variation rather than target-coded subtraction;
2. a torus-momentum or local-stress derivation of equal/opposite field-source
   transfer;
3. carry ownership, transport, finite capacity, reversal, and energy;
4. `gamma`, `p_*`, inertia, absolute mass, and integer-hop normalization;
5. dynamic acquisition/release of the live `u_n` and recovery after a hop;
6. source formation, autonomous mobility, collision composition, universal
   ternary closure, nonlinear attraction, and production;
7. finite-tick `G*` synchronization and preferred-order hiding; and
8. Born recovery, Bell laboratory recovery, Lorentz recovery, and
   completeness.

No new selected type, adoption currency, fitted value, target-coded weight,
or production integration is added.

## 8. Certificate and repair provenance

The frozen FTD-0937 preregistration has SHA-256
`CDDDC452A94938945728571D8677E5CE4F1BD9A0EAEA840A8F4323D22F0E7823`.
Its immutable parent certificate has SHA-256
`FB14DD1CB379C38AAC9A241E4E2373E2CE511F8C988139ED47EBD50F2315057D`.
The first execution returned `126/128`: all substantive gates passed, while
one algebraically equal SymPy matrix factor comparison and the dependent
Outcome-A flag failed. FTD-0937 issued no theorem.

FTD-0938 froze a single representation-only repair:

- repair protocol SHA-256
  `FC2CF1A51252FA5061206C9831B6C6C97A2C879056878E5B6924F9A6AE8AA10F`;
- in-memory wrapper SHA-256
  `8F3F063A4EF96D99F2797E04E10C9D08A0882F558F319E6ED836167C2A596C84`.

The wrapper replaces only the structural matrix equality with its exact zero-
residual form, preserves the parent file, and passes all inherited `128/128`
gates. FTD-0938 is the proof of record.

## 9. Next acceptance gate

Pre-register one local source-centered field action before attempting another
hop simulation. Two honest routes remain:

1. **compact translation route:** use a discrete source coordinate and exact
   torus momentum, derive the opposite field character increment from the
   interaction, and give reciprocal carry a local owner; or
2. **local stress route:** derive a bond/face impulse state and its exact
   source-field exchange law without promoting Bloch phase to unwrapped
   momentum.

Either route must include the compact source-field coupling before the hop,
generate the wake from its own equations, debit a named local store, possess
an exact inverse, prove causal support, and keep `gamma`, `p_*`, and mass
symbolic unless independently fixed. Reading `Dbar(d)` after the event and
subtracting it from a reservoir is a stop condition.

```text
LIVE_NATIVE_DIRECTION=u_n_FROM_FORMED_BODY_CURRENT
MINIMUM_REGISTERED_CONNECTION=A_g=g*gamma*q^2*u_n
REST_CRITICAL_QUARTIC=PRESERVED_EXACTLY
CANONICAL_TOTAL_MOMENTUM=EXACT
MECHANICAL_COMMON_IMPULSE=-g*gamma*Delta(q^2)*u_n
GATE_ZERO_SWITCHING_WORK=ZERO
OFF_PHASE_SWITCHING_WORK=GENERALLY_NONZERO
CONDITIONAL_CONTINUUM_DISPLACEMENT_DIRECTION=NATIVE_u_n
NAIVE_CLOSED_SOURCE_PLUS_WAKE_COMPOSITION=NOT_ENERGY_CONSERVING
WAKE_REQUIRES=BACKREACTION_OR_NAMED_DEBIT_OR_NO_HOP
DIRECTION_PLUS_WAKE_IDENTIFIES_REAL_IMPULSE=FALSE
GAMMA_MU_PSTAR=OPEN
RECIPROCAL_CARRY_OWNER=OPEN
LOCAL_COMMON_SOURCE_FIELD_ACTION=OPEN
PRODUCTION_CHANGED=FALSE
NO_NEW_SELECTED_TYPE=TRUE
GSTAR_BORN_BELL_CONTEXT_USED=FALSE
```
