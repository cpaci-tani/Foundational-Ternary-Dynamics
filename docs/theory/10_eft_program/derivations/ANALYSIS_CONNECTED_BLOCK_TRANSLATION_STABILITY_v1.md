# FTD-0624 — Connected-block translation stability and the half-cell collision surface

> **FTD-0626 successor correction:** this document's “collision surface” and
> reaction interpretation is superseded. The repeated-anchor records remain
> about one cell apart in effective position and become exactly reversible
> under the already selected multiplicity-two chart fibre. FTD-0624 controls
> failure of independent one-record-per-anchor projection only.

**Status:** `[SELECTED DYNAMICS] + [MEASURED — INTEGER-MAXIMUM RUNAWAY AND
ONE-SIDED HALF-CELL RESTORATION] + [CLOSED NEGATIVE — EXACT HALF-CELL
REVERSIBLE REST UNDER INDEPENDENT SITE PROJECTION] + [OPEN — ATOMIC
EXCLUSION/REACTION/DYNAMIC-STABILIZATION LAW]`  
**Protocol SHA-256:**
`CB8AA8843B92F2D8ACB791C5DB01081C6BB2F6AD70E86EC074BBE0EA3E5720A2`  
**Parent:** FTD-0623 result SHA-256
`4E86C850BB1354EC1A9C738FF1C50B94D558528966FED2F0EE40B26B67D69926`  
**Registered verdict:** `CONNECTED_TRANSLATION_STABILITY_EXECUTION_INVALID`  
**Production status:** unchanged

## 1. Result

The FTD-0553 Peierls classification is dynamically visible in every admissible
perturbed arm. Starting at `f=+/-1/64`, the connected object accelerates away
from the integer phase. Starting at `f=+/-(1/2-1/64)`, it accelerates toward
the corresponding half-cell phase. All ten perturbed arms, both exact-integer
controls, and their registered mirror/cyclic comparisons satisfy the unchanged
common-action, energy, coherence, and state-only inverse gates.

The exact half-cell state does not close. Along the body axis, Newton converges
in two iterations with residual `8.70438e-12`, but the accepted continuous
endpoint maps four opposite-polarity pairs onto four common ternary anchors.
The endpoint therefore fails the unchanged unique-site projection at tick one.
In the transverse exact-half arm, eight forward ticks exist and remain
stationary in centre, but the state-only reverse solve fails and supplies no
finite recovery. The registered conjunction is consequently invalid; normal-
mode analysis around a static half-cell centre is not licensed.

## 2. Exact static law

Both registered axes satisfy

\[
U_i(f)=U_0+C_i\left(f^4-\frac12f^2\right),\qquad C_i>0,
\]

to maximum residual `3.87e-16`. The measured coefficients are

| translation axis | `C_i` | `C_i/16` | `U(0)` | `U(1/2)` |
|---|---:|---:|---:|---:|
| parallel `x` | `0.02454649537` | `0.001534155961` | `0.03831279913` | `0.03677864317` |
| transverse `y` | `0.01154006217` | `0.0007212538858` | `0.03831279913` | `0.03759154524` |

Thus the integer state is a genuine stationary maximum and the half-cell state
is the continuous coupling-sidecar minimum. The failed half-cell dynamics does
not invalidate this spectral identity; it shows that its minimum is not an
admissible reversible rest state under the current site projection.

## 3. Perturbed dynamics

After eight ticks:

| arm | centre displacement | first total momentum | classification |
|---|---:|---:|---|
| `x`, `+1/64` | `+0.00150703277` | `+0.000382704009` | away from integer maximum |
| `x`, `-1/64` | `-0.00150703277` | `-0.000382704009` | away from integer maximum |
| `y`, `+1/64` | `+0.000697369271` | `+0.000179641339` | away from integer maximum |
| `y`, `-1/64` | `-0.000697369271` | `-0.000179641339` | away from integer maximum |
| `x`, `+(1/2-1/64)` | `+0.00274182181` | `+0.000731605106` | toward `+1/2` |
| `x`, `-(1/2-1/64)` | `-0.00274182181` | `-0.000731605106` | toward `-1/2` |
| `y`, `+(1/2-1/64)` | `+0.00131299349` | `+0.000344709419` | toward `+1/2` |
| `y`, `-(1/2-1/64)` | `-0.00131299349` | `-0.000344709419` | toward `-1/2` |

The other twelve arms recover below `3.41e-14`, drift total energy by at most
`7.73e-14`, keep shape RMS below `0.001411`, and keep squared-edge strain below
`0.003810`. Their worst common-action residual is below `1.98e-11`. Signed
partners mirror and the accepted cyclic controls rotate within the `1e-8`
registered gates. These conditional controls do not rescue the failed exact-
half conjunction.

## 4. What collides

The body-axis exact-half endpoint contains four duplicate anchors. Every
duplicate is one `+1` and one `-1` constituent at the central interface. For
example, anchor `(8,7,7)` receives effective positions

\[
x_+=7.5007414328,\qquad x_-=8.4992585672.
\]

The two continuous centres remain almost one cell apart, but nearest-site
projection assigns both to site `8`. Three symmetry-related interface pairs do
the same. This is not an exchange of labels between identical constituents.
It is an opposite-polarity occupancy conflict. A primitive ternary site cannot
simultaneously carry `+1` and `-1`.

The numerical solver originally rejected its coordinate-wise derivative probes
at this chart surface. FTD-0624 corrects that solver-only defect: trial probes
may cross the chart, while accepted endpoints still require unique site
projection. After the correction the root converges, and the endpoint—not the
Jacobian probe—fails. No production rule or accepted-state gate changed.

## 5. Ontological consequence

The connected block is a constructive finite-boost carrier but not a static
matter ground state. Electrostatic dressing drives its opposite-polarity
interface toward a manifestation conflict. Under the present reaction-free
action, the continuous minimum is an ontic collision surface.

This leaves three honest candidate continuations:

1. **Reaction:** treat the `+/-` conflict as annihilation into field degrees of
   freedom. This describes decay, not stable matter, and must carry enough
   outgoing state to recover reversibility if low-energy unitarity is retained.
2. **Exclusion/contact:** derive a simultaneous constraint impulse from the
   ternary one-site capacity. It must enter the same common action, do no
   unaccounted work, remain cubic, and be state-only invertible. A post-hoc
   collision correction is inadmissible.
3. **Dynamic stabilization:** seek a closed internal current/orbit whose
   magnetic, binding, and inertial effects keep opposite polarities away from
   the collision surface. The appropriate background would be a periodic
   orbit with Floquet modes, not a static field minimum.

The existing dual substrate does not already solve this. Its `J_L/J_R`
registers are two field vectors at one voxel; they are not two independent
ternary occupancy slots. Reinterpreting them as such would be a new ontology.

## 6. Correct next gate

Do not linearize around `f=1/2`. First construct one versioned atomic
occupancy law that solves endpoint anchors, current, field update, momentum,
constraint/reaction impulse, and energy in the same transaction. The output
must be a unique ternary state, not a fractional or multiply occupied site.

If no local cubic state-only-invertible transaction exists without a new branch
variable, the failure forces an explicit internal occupancy/temporal-phase
fibre. Only after a collision-free fixed point or reversible periodic orbit is
qualified should normal modes, depinning, dressing/wake, or an infrared pole be
measured.

## 7. Reproducibility

- common-action source SHA-256:
  `E242F462F4975030E361DDD2A7181DAD4142D4100831911D9C2CF61029CC2C43`
- test SHA-256:
  `632CC1F3ED49879501A72F440E66804804D3687D7285F267CA25C25A5A1CF15A`
- JSON SHA-256:
  `55D34381B4968653740DF57A0F2330A3D175CC2CFD52012A2C4657D601825653`
- arm CSV SHA-256:
  `761B5F7CC461AC82A51B86D634560C0E44D4CD789B590E59445BE8237854E2BD`
- tick CSV SHA-256:
  `8B4383B394CC18423F9AF6184059E01469A49736217703511A9854D984D573F8`
- independent certificate SHA-256:
  `0971B143D0E25CDCDF5DDE0A030EDCFD2847619B0313B5F321B906F927E08CD1`
- independent certificate: `29/29` checks pass
