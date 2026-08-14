# Theorem — Common/relative catalytic clock growth and quiescent-seam boundary v1

**Identifier:** `FTD-0997`  
**Date:** 2026-08-12  
**Status:** `[THEOREM — EXISTING RELATIVE-PAIR CAPACITY / NO NEW CONTINUOUS TYPE]` +
`[THEOREM, CONDITIONAL — EXACT SYMPLECTIC CATALYTIC SWAP–REFILL]` +
`[THEOREM, CONDITIONAL — ENERGY/INVERSE/MISMATCH RETENTION]` +
`[THEOREM — CATALYTIC RECURSION IFF FORMATION-CLOCK COMPLIANCE]` +
`[THEOREM — SEPARABLE MEMBRANE HAMILTONIAN DOES NOT FORCE COMPLIANCE]` +
`[CLOSED NEGATIVE — QUIESCENT MEMBRANE AS RECURSIVE REFILL SOURCE]` +
`[OPEN — PORT PREPARATION/OWNERSHIP/POWER/CONTROL/PRODUCTION]`  
**Parent:** `FTD-0995/0996`

## Result

The existing dual substrate contains enough phase-space capacity for a local
recursive clock-growth transducer. No seventh continuous pair is required.
The relative `L-R` pair can act as a catalytic phase-and-energy port for the
common `L+R` body clock.

Let the donor common clock, donor relative port, and prospective receiver
common clock be complete canonical pairs

\[
C=(q_C,p_C),\qquad R=(q_R,p_R),\qquad Y=(q_Y,p_Y).       \tag{1}
\]

At a kinetic crossing, prepare

\[
C=R=z=(0,\sigma\sqrt{2me}),
\qquad Y=0,
\qquad e>0,quad\sigma=\pm1.                            \tag{2}
\]

Apply two local operations:

\[
\text{swap:}\qquad(R,Y)\longmapsto(Y,R),                \tag{3}
\]

followed by the FTD-0994 formation-work refill of the emptied relative pair,

\[
R=0\longmapsto z_U=(0,\sigma\sqrt{2mU}).                \tag{4}
\]

The exact output is

\[
\boxed{C'=z,\qquad Y'=z,\qquad R'=z_U.}                 \tag{5}
\]

The complete-pair swap is symplectic, orthogonal, and involutive. The refill
is the previously certified local canonical shear. Their composition is
therefore symplectic on the registered positive-work branch and is inverted
in reverse order: inverse refill, then inverse swap.

If the formation source loses `U`, the full ledger is

\[
\begin{array}{c|ccc|c}
&H_C&H_R&H_Y&H_{\rm source}\\ \hline
\text{before}&e&e&0&E_s\\
\text{after}&e&U&e&E_s-U.
\end{array}                                             \tag{6}
\]

Total energy is exact for every positive `U`. The port retains the mismatch

\[
\boxed{\Delta H_R=U-e.}                                \tag{7}
\]

It returns to its initial phase-bearing state—and is therefore catalytic for
the next growth event—exactly when

\[
\boxed{U=e
\quad\Longleftrightarrow\quad
2mU-p_C^2=0.}                                          \tag{8}
\]

Thus FTD-0996's compliance surface is not merely an energy comparison. It is
the exact condition for a local relative port to copy once, refill itself,
and become recursively ready without losing phase history.

The unchanged FTD-0990 Hamiltonian does not force equation (2) or (8). Its
common and relative sectors are block diagonal, so their initial data are
independent. Formation work is a configuration function while crossing
energy is momentum dependent. At nonzero crossing momentum,

\[
F(q,p_C,m)=2mU(q,m)-p_C^2,
\qquad
{\partial F\over\partial p_C}=-2p_C\ne0.               \tag{9}
\]

Compliance is therefore a regular codimension-one surface, not an identity
on an open local phase space.

At the natural quiescent matching seam, every changed common-field bond has
zero strain. With no additional onsite load,

\[
\boxed{W_y=U=0.}                                       \tag{10}
\]

A positive-energy port can perform the swap once, but the static membrane
cannot refill it. Recursive growth must be powered by pre-existing boundary
strain, an onsite latent term, relative/environmental inflow, or a separate
local reserve. None is forced by the static matter membrane to equal `e`.

## Certificate of record

- Protocol:
  [`PREREG_COMMON_RELATIVE_CATALYTIC_CLOCK_GROWTH_TRANSDUCER_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_COMMON_RELATIVE_CATALYTIC_CLOCK_GROWTH_TRANSDUCER_v1.md),
  SHA-256
  `632A3453B5C4BC166153FA8DF54AAB589563846A78C160531A2A0DDCDC7C0DF1`.
- Exact certificate:
  [`proof_common_relative_catalytic_clock_growth_transducer.py`](../../../../../scripts/proofs/proof_common_relative_catalytic_clock_growth_transducer.py),
  SHA-256
  `ADA5F5872A4CE0F56A9F4197EB80930037B947E0EA6501D27D5800732D3D2BFC`.
- First locked execution: `64/64 computational, 35/35 disclosure`, **Outcome B —
  existing-pair catalytic transducer / native compliance open**, no repair.

## 1. Exact complete-pair swap

Order the port/receiver variables as

\[
Z=(q_R,p_R,q_Y,p_Y)^T
\]

and define

\[
S=\begin{pmatrix}0&I_2\\I_2&0\end{pmatrix}.             \tag{11}
\]

For `Omega_4=diag(J,J)` with
`J=[[0,1],[-1,0]]`,

\[
S^T\Omega_4S=\Omega_4,
\qquad S^TS=I,
\qquad\det S=1,
\qquad S^{-1}=S.                                      \tag{12}
\]

Both canonical coordinates of each pair move together. Swapping only the two
position coordinates while leaving their conjugate momenta fixed has
determinant `-1` and is not symplectic.

Equation (3) transfers the complete port state to the receiver and leaves the
port exactly blank. Equation (4) is then applied on precisely the zero-action
seam for which the FTD-0994 work identity is valid.

## 2. Energy, mismatch, and inverse

The swap changes no oscillator energy. The refill changes the port energy
from zero to `U` while the source loses `U`. This proves equation (6).

For arbitrary mismatch, the map is still invertible. Starting from equation
(5):

1. the inverse refill maps `z_U -> 0` and returns `U` to the source;
2. the inverse swap maps `(R,Y)=(0,z)` back to `(z,0)`.

It recovers every declared initial variable. Catalytic repeatability is a
stronger condition than invertibility. It requires the port after refill to
equal the port before swap, which proves equations (7)--(8).

If `U<e`, the port has donated net energy `e-U`; if `U>e`, it has stored net
surplus `U-e`. No mismatch is erased. Reusing a mismatched port would seed the
next receiver with a different amplitude and, for a critical quartic clock,
the FTD-0996 cadence detuning.

## 3. Why this is not unrestricted cloning

On the compliant prepared surface, equations (2) and (8) make the output
appear to contain three copies of `z`: donor, restored port, and receiver.
This does not contradict the canonical no-cloning result.

- The full map is defined on independent `R,Y` variables and is invertible.
- `C=R` is a prepared correlation, not an identity on the full phase space.
- `R'=C` occurs only on the additional scalar constraint `U=e`.
- The membrane/source state changes and supplies the receiver's energy.
- Off that constrained surface, `R'=z_U` retains the mismatch rather than
  becoming another copy.

The source-plus-port history is exactly the machine state that free copying
lacked.

## 4. Existing common/relative capacity

FTD-0990 already supplies

\[
q_\pm={q_L\pm q_R\over\sqrt2},
\qquad
p_\pm={p_L\pm p_R\over\sqrt2}.                         \tag{13}
\]

This transformation is orthogonal and symplectic. The common and relative
sectors are two existing complete pairs; equation (11) merely assigns the
relative pair a conditional local port role. No new continuous phase-space
dimension is introduced.

Capacity is not ownership. Unchanged production does not protect a relative
pair at each boundary, prepare it equal to the common clock, reserve it for a
single event, or schedule the swap/refill. These remain functional
selections unless derived later.

## 5. Native non-forcing theorem

In the fixed `+/-` chart, the selected FTD-0990 reference Hamiltonian has the
separable form

\[
H_0=H_+(q_+,p_+;m)+H_-(q_-,p_-).                       \tag{14}
\]

Every common/relative mixed second derivative vanishes. Consequently the
Hamiltonian allows arbitrary independent initial data in the two sectors and
does not prepare equation (2).

At fixed field coordinates, FTD-0992 formation work depends on occupancy and
coordinate strains. It contains no `p_C`. The donor crossing energy depends
on `p_C^2`. Equation (9) therefore proves that compliance cannot be an
identity on any open neighborhood with `p_C != 0`. By the implicit-function
theorem its zero set is locally codimension one.

This excludes a claimed automatic consequence of the unchanged separable
membrane Hamiltonian. It does not exclude a new cross coupling, constraint,
feedback law, or dynamically prepared invariant surface.

## 6. Quiescent-seam closed negative

The one-site work law is a sum of changed bond terms

\[
W_y={1\over2}\sum_{b\ni y}(g_b'-g_b)a_bd_b^2
     +W_{\rm onsite}.                                  \tag{15}
\]

At the exact kinetic matching seam, set the receiver and every affected
common-field endpoint coordinate equal to zero. Then every `d_b=0`. If no
extra onsite term changes, equation (15) gives equation (10) independently
of the gate changes.

For `e>0`, the port is emptied by equation (3) and equation (4) leaves it
empty. The current static membrane is therefore closed negative as a
self-powered recursive refill source on the quiescent seam.

The result is an energy statement, not a metaphysical prohibition. Matter
growth may be powered. The allowed sources are explicit:

- stored strain cut from the surrounding field;
- a signed onsite formation/latent-energy term;
- incoming energy in the open relative/environmental channel; or
- a prepositioned local phase-complete reserve.

Each source must carry a positive ledger, locality, ownership, backpressure,
and inverse. Merely naming it does not derive it.

## 7. Epistemic disposition

Established:

- **[THEOREM]** the existing relative pair has sufficient canonical capacity
  and adds no continuous type;
- **[THEOREM, CONDITIONAL]** complete-pair swap plus formation refill is an
  exact local symplectic, energy-conserving, invertible transducer;
- **[THEOREM]** catalytic recursion occurs iff the FTD-0996 compliance holds;
- **[THEOREM]** mismatch remains as relative-port energy `U-e`;
- **[THEOREM]** the unchanged separable common/relative Hamiltonian does not
  force the preparation or compliance surface; and
- **[CLOSED NEGATIVE]** a zero-strain, zero-onsite-load membrane cannot refill
  a positive-energy catalyst.

Selected/open:

- preparation and protection of `C=R`;
- relative-port ownership, swap/refill engagement, aperture scheduling, and
  collision/backpressure;
- a positive local source that repeatedly supplies the per-site energy;
- attraction or robust tolerance around compliance;
- native quarticity, amplitude/scale selection, finite-tick `G*`, and CM
  physical realization;
- production genesis/evaporation, moving boundaries, CPU/CUDA parity, and
  operational hiding; and
- Born/Bell recovery, mass, Lorentz recovery, biology, consciousness, and
  framework completeness.

No production integration follows.

## 8. Next discriminator

The remaining dynamics question has become a resource theorem:

> Can a finite local relative/environmental current replenish one clock-energy
> share per added site with exact backpressure and inverse, or does indefinite
> coherent growth require an explicitly open energy flux?

The next campaign should derive the cumulative energy bound, identify the
minimum positive local inflow/reserve law, and reject any construction that
multiplies clock energy while leaving every source unchanged.
