# Theorem — Oriented ternary quarter-turn gearbox v1

**Identifier:** `FTD-0872`  
**Status:** `[THEOREM — UNIQUE ORIENTATION-PRESERVING TERNARY TRANSFER]` +
`[THEOREM — EXACT REVERSIBLE READY-PORT EMISSION/ABSORPTION]` +
`[CLOSED NEGATIVE — NAIVE EMPTY-PORT/OTHERWISE-IDENTITY WRAPPER]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[OPEN — PHYSICAL ACTUATION, WORK, PROTECTED RAIL, PRODUCTION, G*]`  
**Date:** 2026-08-11  
**Certificate:** locked first execution `40/40`; no repair

## 1. Result

The actual ternary source and its oriented output port admit a unique minimum
transfer law once three requirements are imposed together:

1. the source value must arrive at a ready output with its sign unchanged;
2. the two-register quadratic label norm must be preserved; and
3. the map must preserve orientation.

The law is the quarter-turn

\[
 R=\begin{pmatrix}0&-1\\1&0\end{pmatrix},
 \qquad R(s,o)=(-o,s).                            \tag{1}
\]

On the ready-port subspace,

\[
 \boxed{R(s,0)=(0,s)}.                            \tag{2}
\]

Thus the completed actual state moves into the output while the local latch
becomes ready. The inverse quarter-turn performs absorption:

\[
 \boxed{R^{-1}(0,s)=(s,0)}.                       \tag{3}
\]

No label is erased. The physical continuous signal energy and controller
work remain separate accounts.

## 2. Exact ternary geometry

Use the FTD-0871 identification

\[
 \{-1,0,+1\}\cong\mathbb F_3.
\]

Then

\[
 R^2=-I,\qquad R^4=I,\qquad
 \det R=1,\qquad R^TR=I.                         \tag{4}
\]

For the canonical representatives define

\[
 Q(s,o)=s^2+o^2.                                  \tag{5}
\]

Direct enumeration of all nine states gives

\[
 Q(Rz)=Q(z).                                      \tag{6}
\]

Equation (5) is a normalized label metric. It becomes a physical energy only
after a separately declared amplitude/action scale is supplied, as in
FTD-0863/0865. It must not be counted as a derivation of event energy.

The ordered one-step area distinguishes the two directions:

\[
 \chi_R(z)=\det[z,Rz]=Q(z)>0,                     \tag{7}
\]

\[
 \chi_{R^{-1}}(z)=\det[z,R^{-1}z]=-Q(z)<0         \tag{8}
\]

for every nonzero `z`. Meanwhile

\[
 \operatorname{Sym}^2(R)
 =\operatorname{Sym}^2(-R),                       \tag{9}
\]

so the ordered actual pair retains exactly the clockwise/counterclockwise
datum that an even symmetric-square projection loses.

## 3. Uniqueness

Let `e1=(1,0)^T` and `e2=(0,1)^T`. Exhaust all `3^4` matrices over `F_3` and
impose

\[
 Me_1=e_2,qquad \det M=1,qquad M^TM=I.           \tag{10}
\]

There is exactly one solution: `M=R`.

The unsigned swap

\[
 K=\begin{pmatrix}0&1\\1&0\end{pmatrix}           \tag{11}
\]

also maps `e1` to `e2` and preserves (5), but `det K=-1`. It is a reflection,
not an oriented quarter-turn. Orientation is therefore the discriminator
between a bare swap and the minimum `i`-like transfer.

Equation (4) is the exact finite-state meaning of the complex structure here:
`R` is a real two-register endomorphism whose square is `-I`. This does not
derive complex potentiality or Hilbert space. FTD-0836 separately proves that,
conditional on the selected quartic clock, the nonlinear physical traversal
of the corresponding self-dual energy circle carries total time weight
`sqrt(pi) G*`. The algebraic quarter-turn supplies the ordered skeleton; it
does not by itself supply Gamma or `G*`.

## 4. Eligibility and backpressure

For the already-defined binary clutch value `a`, set

\[
 G_a=(1-a)I+aR,
 \qquad a\in\{0,1\}.                              \tag{12}
\]

Both branches are permutations of all nine joint states. Readiness is an
operating subspace, not part of the mathematical definition of `R`.

This distinction is mandatory. Consider the tempting rule

\[
 F(s,o)=
 \begin{cases}
 R(s,o),&o=0,\\
 (s,o),&o\ne0.
 \end{cases}                                      \tag{13}
\]

It has the explicit collision

\[
 F(1,0)=(0,1)=F(0,1).                              \tag{14}
\]

Therefore (13) is noninjective. A physical reversible mechanism cannot simply
“fail closed” when it discovers a full output port. It must instead do at
least one of the following:

1. schedule the gate only on a guaranteed-ready port;
2. execute the all-domain reciprocal exchange and retain the incoming value;
3. reflect/export the blocked value through another retained port; or
4. add a reversible control/history coordinate and pay for it explicitly.

FTD-0856's incoming/outgoing reciprocal modes and FTD-0867's eligibility
clutch are precisely the existing interfaces capable of carrying those
distinctions.

## 5. Relation to FTD-0871 and the rail

FTD-0871 used controlled ternary subtraction followed by an empty-port
handoff. On its registered matching/ready subspace, equation (2) supplies the
same endpoint in one joint permutation:

\[
 (s,0)\longmapsto(0,s).                            \tag{15}
\]

The logical source-to-port gearbox is therefore closed. Composing (15) with
the selected FTD-0852/0855 signed rail moves the retained label outward, while
FTD-0863 carries the companion continuous event action. The prior finite-rail
bound remains unchanged: a finite rail cannot retain an indefinitely growing
history unless a complete signed tail is exported, recirculated, or stored.

This composition is exact reference mathematics. It does not prove that the
frozen C18 production field supplies the protected rail; FTD-0858 proves the
literal one-cell identification fails.

## 6. Physical boundary

The theorem closes the discrete permutation, not the actuator. In particular:

- zero change of the endpoint label norm does not imply zero switching work;
- the current production genesis/evaporation map remains many-to-one and is
  not silently replaced;
- the continuous signal still needs an energy/current ledger;
- port readiness, overlap, noise, and controller recovery remain physical
  requirements;
- native cubic transport and production integration remain open;
- `G*` may govern a later eligibility cadence but does not select the outcome
  or generate this gate; and
- Born/Bell, operational Lorentz hiding, and completeness are untouched.

## 7. Certificate and implementation

The frozen certificate
`scripts/proofs/proof_oriented_ternary_quarter_turn_gearbox.py` has SHA-256
`A36BFE041A7ADDA56FF15ECD4959156DE81AD85E727D3E3FA6A48F74B835F395`
and passed `40/40` on its first locked execution. The pre-run protocol hash is
`63462EDAA5970A1EC934F34A1ABF1EB95FC22D1969E6817EE8BA1912FC96E295`.

The isolated reference implementation is in:

- `engine/include/ftd/eft/oriented_ternary_quarter_turn.h`;
- `engine/src/eft/oriented_ternary_quarter_turn.cpp`; and
- `engine/tests/test_oriented_ternary_quarter_turn.cpp`.

It changes no `Voxel`, tick phase, production toggle, or renderer path.

