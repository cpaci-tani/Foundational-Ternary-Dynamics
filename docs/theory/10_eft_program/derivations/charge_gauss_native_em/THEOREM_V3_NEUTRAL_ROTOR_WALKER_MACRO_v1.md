# V3 neutral rotor/walker macro v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — CARRIER-COMPLETE NEUTRAL SAMPLER]** +
**[THEOREM — STATE-ONLY UNIQUE ROTOR/MARKER ROLES]** +
**[THEOREM — RADIUS-ONE ZERO-FIELD CUBIC-COVARIANT TRANSACTION]** +
**[SELECTION — CANDIDATE MACRO, NOT PHI]** +
**[OPEN — CHARGED A9 COMPOSITION, RENEWAL, SINK WORK, MULTI-WALKER
SCHEDULING, INVERSE, FIELD WRITEBACK, AND NORMALIZATION]**  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Parent Green theorem:**
[`THEOREM_V3_NEUTRAL_ROTOR_HARMONIC_GREEN_SEAM_v1.md`](THEOREM_V3_NEUTRAL_ROTOR_HARMONIC_GREEN_SEAM_v1.md)  
**Exact certificate:**
[`proof_v3_neutral_rotor_walker_macro.py`](../../../../../scripts/proofs/proof_v3_neutral_rotor_walker_macro.py)

---

## 1. Carrier question

The parent theorem encoded one zero-field rotor in an opposite-polarity Hodge
record pair and proved the deterministic Green bounds. It left the moving
sampler token abstract. This theorem asks whether the sampler itself can be
represented locally without adding a carrier type or contaminating $(E,B)$.

Let $R$ denote the selected period-twelve Hodge/C4 internal tick. At an
unmarked site, the two opposite-polarity slots controlled by $q$ store the
rotor:

\[
 U(q)=\{(q,+),(q,-)\}.                                  \tag{1}
\]

At the sampler site, add a second neutral pair at offset four:

\[
 M(q)=U(q)\cup U(R^4q).                                \tag{2}
\]

The offset cannot be six. The unordered set
$\{q,R^6q\}$ is symmetric under exchanging its two controllers, so no
state-only predicate can distinguish rotor from marker. For offset four,

\[
 R^8q\notin\{q,R^4q\},                                 \tag{3}
\]

and exactly one controller $u$ in equation (2) satisfies
$R^4u$ equal to the other controller. The rotor/marker roles are therefore
unique functions of the instantaneous four occupied bits.

Both equations (1) and (2) have exact zero electric-magnetic readout on every
C3 layer.

---

## 2. Local transaction

Suppose site $x$ has the marked pattern $M(q_x)$ and the served neighbor $y$
has the clean unmarked pattern $U(q_y)$. One local transaction performs

\[
 q_x\longmapsto Rq_x,
 \qquad
 d=r(Rq_x),
 \qquad
 y=x+d,                                                 \tag{4}
\]

and writes

\[
 M(q_x)+U(q_y)
 \longmapsto
 U(Rq_x)+M(q_y).                                       \tag{5}
\]

Thus:

- the departure rotor advances once;
- the marker moves exactly one SC edge;
- the destination rotor is not advanced until the next visit;
- six field records exist before and after;
- every site remains exactly $(E,B)=0$; and
- every dependency and writer lies on one SC edge.

The native internal tick commutes with signed-cubic transformations and the
served direction is polar. Equation (5) therefore commutes with all 48
signed-cubic maps. No coordinate color, external direction, random branch, or
target quantity is read.

The exact certificate checks all $192^2$ clean departure/destination
controller pairs, giving 294,912 local transaction rows, plus 27,648 cubic
covariance rows.

---

## 3. Relation to the Green theorem

On a prepared absorbing box, externally insert the marker pair $U(R^4q_s)$
beside the current source rotor $U(q_s)$. Apply equation (5) until the served
edge exits the box, then clear the marker. Repeating this preparation while
retaining the rotor background reproduces the parent controller-only paths
step for step.

The certificate compares four finite-box injection histories exactly. All
visits and directed traversal counts agree. Consequently the carrier-complete
neutral macro inherits

\[
 \operatorname{div}J_N=\delta_s,
 \qquad
 \|L_DG_N-\delta_s\|_\infty\le {8\over N},
 \qquad
 \|J_N-\nabla_DG_N\|_\infty\le {8\over3N}.             \tag{6}
\]

This removes the abstract neutral-token carrier price from the parent theorem.
It does not make the sampler a physical electric charge.

---

## 4. Noninjectivity and current

The destination marker is rewritten relative to the destination rotor. It
does not retain the departure site or incoming direction. Different clean
prestates can therefore share an output. This is a named noninjective routing
transaction, not a reversible Hamiltonian map.

The one-hop current is nevertheless determined by the actual prestate and
output of each tick. Its cumulative traversal count is a blocked history
readout; no unbounded counter is stored in one voxel. A reciprocal physical
action would still have to retain or price the erased path information and
book the source/sink work.

---

## 5. Exact boundary

Established:

1. two existing bits encode an unmarked zero-field rotor;
2. four existing bits encode a uniquely recognizable marked site;
3. offset four gives unique roles while the offset-six control fails;
4. the clean local move is total on the declared one-marker sector;
5. six records and zero $(E,B)$ are retained;
6. the move is radius one and signed-cubic covariant; and
7. finite-box histories reproduce the harmonic-Green rotor exactly.

Open or selected:

1. composition with an A9 charged endpoint and its dressed Gauss string;
2. integration into the complete Phi schedule beside collision, streaming,
   charged frames, and malformed-state fallback;
3. autonomous source renewal and an owned absorbing sink;
4. reciprocal work or a retained inverse/path record;
5. conflict-free multiple-walker arbitration;
6. formation and persistence of the rotor medium;
7. instantaneous field writeback or a fully operational blocked-field rule;
8. physical coupling normalization; and
9. stable matter, Born apparatus, and tensor response.

The neutral history pole now has both local storage and a local moving sampler.
The decisive remaining charged step is to make the same transaction move an
actual A9 endpoint while maintaining its finite dressed Gauss history and
common work ledger.
