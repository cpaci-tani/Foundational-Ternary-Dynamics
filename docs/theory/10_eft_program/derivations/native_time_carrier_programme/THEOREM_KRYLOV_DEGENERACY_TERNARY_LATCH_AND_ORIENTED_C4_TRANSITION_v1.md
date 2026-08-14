# Krylov-degeneracy ternary latch and oriented C4 transition

**Identifiers:** `FTD-0971`, `FTD-0972`  
**Status:** `[THEOREM — MINIMUM SELF-DELIMITING TERNARY CROSSING LATCH]` +
`[THEOREM, CONDITIONAL — EXACT REVERSIBLE ORIENTED C4 CARRIER]` +
`[THEOREM — REAL J MODE WITH J^2=-I/TIME-REVERSE -J]` +
`[OPEN — FORMATION/PHYSICAL ENERGY/ACTIVE FIELD COUPLING/G* CADENCE]`  
**Date:** 2026-08-12

## 1. Result

The exact singularity of the FTD-0969 regional body admits a minimum
self-delimiting reversible crossing record using one ternary latch. Retaining
the two crossing directions at the degenerate body produces four composite
states and an exact oriented `C4` transition cycle.

On its real two-dimensional oriented mode, one forward update acts as

\[
 J=\begin{pmatrix}0&-1\\1&0\end{pmatrix},qquad J^2=-I.     \tag{1}
\]

Reverse evolution acts as `-J`. Their squares coincide, so the symmetric
square loses direction. This gives an exact discrete real representation of
multiplication by `i`, but `i` is an eigenstructure of the retained recursive
update—not a fourth ontic value, not a property of a static ternary site, and
not a derivation of Hilbert space.

The carrier is reversible and norm preserving. Its physical formation,
regular-versus-degenerate energy balance, active coupling to fields, and
critical-quartic cadence remain open.

## 2. Exact singular frame transition

For the one-cube family

\[
\begin{array}{c|cccc}
x&(0,0,0)&(1,0,0)&(0,1,0)&(1,1,u)\\ \hline
s_x&+1&+1&-1&-1,
\end{array}                                                   \tag{2}
\]

the exact moments include

\[
 d=(0,-2,-u),qquad \kappa(u)=-{u^5\over256}.               \tag{3}
\]

The one-sided normalized frame limits are

\[
 e_1^+=e_1^-=(0,-1,0),qquad
 e_2^+=e_2^-=(-1,0,0),                                      \tag{4}
\]

\[
 \chi^+=-1,quad e_3^+=(0,0,1),qquad
 \chi^-=+1,quad e_3^-=(0,0,-1).                            \tag{5}
\]

Thus the branch transition is

\[
 S=(F^+)^TF^-=operatorname{diag}(1,1,-1),qquad
 S^2=I,quad\det S=-1.                                      \tag{6}
\]

The two sides are related by an improper reflection. It conjugates one
handed complex structure into the other,

\[
 Q\mathcal I_-Q^T=\mathcal I_+,qquad \mathcal I_+=-\mathcal I_-.
                                                               \tag{7}
\]

Equation (6) is not itself multiplication by `i`: it is an involutive
reflection with square `+I`. The oriented `i` mode appears only after the
crossing direction is retained in the complete transition state.

## 3. Why a ternary latch is minimum here

At `kappa=0`, the spatial body alone cannot distinguish the two incoming
branches. A fail-closed self-delimiting latch must distinguish

\[
 \{\text{blank},\text{incoming }+,\text{incoming }-\}.       \tag{8}
\]

An alphabet with fewer than three states cannot injectively encode (8). The
existing ternary form

\[
 \ell\in\{-1,0,+1\}                                        \tag{9}
\]

is therefore exactly sufficient and minimum within a one-cell interface that
must carry its own blank/loaded status. Modular loading

\[
 L_s:\ell\mapsto\ell+s\pmod3,qquad L_s^{-1}=L_{-s}          \tag{10}
\]

is a permutation and can be cleared reversibly.

This minimum is conditional on self-delimitation. A separately retained
phase/type flag could reduce the latch alphabet while increasing the total
priced state; no universal “one bit” adoption follows.

## 4. The four-state recursive carrier

Let

\[
 A=(R_+,0),\quad B=(D,+1),\quad
 C=(R_-,0),\quad D=(D,-1),                                  \tag{11}
\]

where `R_+` and `R_-` are regular chiral branches and `D` is their common
degenerate spatial body. The two degenerate states differ only in the retained
crossing direction.

The forward update is the exact cycle

\[
 T:A\to B\to C\to D\to A,qquad
 T^4=I,quad T^{-1}=T^3.                                    \tag{12}
\]

It is a finite stable recurrence in the exact kinematic sense: no state is
lost, every state has one predecessor and successor, and four forward strokes
return the complete carrier state.

The time-reversal involution fixes the regular snapshots and exchanges the
directed degenerate states,

\[
 \Theta A=A,qquad \Theta C=C,qquad \Theta B=D,qquad
 \Theta D=B,                                                \tag{13}
\]

so

\[
 \Theta T\Theta=T^{-1}.                                    \tag{14}
\]

Direction is therefore relational and temporal: it lives in the ordered
transition through the same degenerate body, not in either regular snapshot
alone.

## 5. Exact real representation of `i`

In the real one-hot carrier space define

\[
 c=A-C,qquad s=B-D.                                        \tag{15}
\]

The plane spanned by `(c,s)` is invariant. On the ordered basis `(c,s)`,

\[
 Tc=s,qquad Ts=-c,qquad
 [T]_{(c,s)}=J.                                             \tag{16}
\]

Hence

\[
 J^2=-I,qquad J^4=I.                                      \tag{17}
\]

Reverse traversal gives `J^{-1}=-J`, but

\[
 J^2=(-J)^2=-I.                                            \tag{18}
\]

This identifies precisely what the symmetric square loses: it records that
two strokes occurred while erasing whether the cycle was traversed forward or
backward.

The full four-state permutation has characteristic polynomial

\[
 \lambda^4-1=(\lambda-1)(\lambda+1)(\lambda-i)(\lambda+i). \tag{19}
\]

The `+/-i` pair is thus forced by the oriented four-cycle. It is not inserted
as an additional ontic state.

## 6. What is and is not stable

As a permutation, `T` preserves the equal-weight one-hot norm and has an exact
inverse. This is rigorous logical stability and losslessness of the carrier.

It is not yet physical dynamical stability. The certificate does not show
that:

- production dynamics forms the regular and degenerate body states;
- their physical energies are equal;
- one transition occurs autonomously per tick;
- perturbations return to the four-state orbit; or
- a reservoir supplies any required deformation work.

Those require an actual action or exact local transition rule with energy,
reaction, reserve, backpressure, and perturbation gates.

## 7. Why this is not yet the field gearbox

The four-cycle changes only the abstract retained body/latch state. No field
coordinate or momentum is acted on. Using the oriented mode to rotate a field
pair would require a separately declared controlled interaction.

That interaction must close:

- field and body/source reaction;
- physical energy and work;
- finite reserve and fail-closed backpressure;
- exact inverse and retained routing history; and
- active rather than passive chart action.

The cycle therefore supplies a natural **control clock representation** for
`i`, not the controlled physical exchange itself.

Likewise, one stroke per global tick gives only a four-tick reference orbit.
Nothing here produces the exact critical-quartic period factor `G*`. Matching
the C4 carrier to a maintained quartic clock still requires the missing
substrate/CM cadence gearbox.

## 8. Certificate

- FTD-0971 protocol SHA-256:
  `85E6BA5B4CEFC7CDBF70A5CB903C19D3E6230632889DE70927A4C1E5FF28C8E5`;
- immutable parent proof SHA-256:
  `F8C44B012A3FDF60B974327B95D8534A5EA1E48351F59DA10CE4C331C11169D0`;
- first parent execution: `60/62`, Outcome D on two Markdown line-wrap
  verifier markers after every substantive gate passed;
- FTD-0972 repair protocol SHA-256:
  `4F69BFBB20BDB277BF76244F1FAAA3752090421CFBC8A22A63B2688F4647D574`;
- repair wrapper SHA-256:
  `AD22721EE499BBDE334FF60AAC8CABA807D8871043D7FA9FAC5E384C89E60869`;
- repaired inherited certificate: `62/62`, Outcome B;
- repair integrity: `14/14`;
- no engine or production file changed.

## 9. Scope firewall

This theorem is not:

- a fourth ontic state or a derivation of complex Hilbert space;
- a derivation of gamma, damping, mass, or spin;
- proof of autonomous body formation or perturbative stability;
- physical energy equality between regular and degenerate phases;
- an active field rotation, token-loading transaction, or positive export;
- a `G*` cadence derivation or CM-prime/substrate gearbox;
- Born/Bell recovery or preferred-tick hiding; or
- production integration or whole-framework completeness.

The next admissible step is to derive the minimum controlled symplectic
coupling from this retained `C4` carrier to one complete field pair and test
its reaction/energy ledger. The carrier itself may not be counted as that
coupling.
