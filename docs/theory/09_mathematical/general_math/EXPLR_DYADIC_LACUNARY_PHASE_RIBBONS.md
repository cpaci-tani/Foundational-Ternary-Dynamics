# EXPLR - Dyadic Lacunary Phase-Ribbon Geometry

**Document type:** Exploratory mathematical / visualization note
**Status:** [EXPLORATORY] for FTD; exact claims are curve-local or renderer-local only
**Primary curve note:** [EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md](EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md)
**Interactive workbench:** `dissemination/interactive/dyadic_lacunary_curve_lab.html`

---

## 0. Purpose

This document records the geometry seen in the interactive dyadic lacunary
curve lab:

```text
finite lacunary Fourier curve
  -> hidden rotating clocks
  -> lossy 2D projection
  -> high-frequency phase braiding
  -> ribbon / tesseract-shadow visual readout.
```

The seed curve `C_3` is already documented as a precise four-mode analytic
parametrization with a degree-16 algebraic image, exact signed area, exact
regularity check, and projective singularity budget. This note does not
replace that proof-oriented document. It explains the mutable visual family:

- why the curve changes so dramatically under mode edits;
- why the animation can feel "alive";
- why the ribbon view looks like a 3D object or a tesseract-like shadow;
- how the optional 3D phase lift chooses a diagnostic `z` channel;
- how phase fibers expose stacked hidden times at one visible location;
- why an absolute grid is needed for stable perception;
- how the regime classifier names the current visual state;
- which observations are mathematical facts, which are renderer facts, and
  which remain visual intuition only.

No FTD physics claim is promoted here. The note is an intuition and
visualization ledger for a finite Fourier readout.

---

## 1. General mutable curve family

The interactive lab works with finite curves of the form

```text
C(t) = (x(t), y(t)),       0 <= t <= 2*pi,

x(t) = A sum_{k in E} a_k cos(n_k(t + phi) + p^x_k),
y(t) = A sum_{k in E} b_k sin(n_k(t + phi) + p^y_k).
```

Here:

```text
E        = enabled set of modes,
n_k      = B^k, usually B in {2, 3, 4},
a_k,b_k  = x/y amplitudes,
p^x_k    = x-phase offset,
p^y_k    = y-phase offset,
phi      = global phase,
A        = global amplitude.
```

The seed `C_3` is the special four-mode dyadic case:

```text
B = 2,
E = {0,1,2,3},
(a_0,a_1,a_2,a_3) = (1, 1/2, 1/2, 3/8),
b_k = 2(-1)^k a_k.
```

The lab then deliberately breaks the seed conditions:

- it changes `B`;
- it changes `E`;
- it changes amplitudes and phases;
- it changes chirality signs;
- it adds or removes high modes;
- it renders phase-thickened ribbons.

Thus most lab states are not the exact `C_3` theorem object. They are members
of the surrounding finite Fourier sandbox.

---

## 2. Exact facts inherited from the seed

For the unmutated seed `C_3`, the primary document verifies:

| Feature | Status |
|---|---|
| Finite dyadic support `1,2,4,8` | definition |
| Real analytic parametrization | immediate from finite trigonometric sum |
| Regular immersion, no cusps | exact resultant check |
| Reflection symmetry | exact |
| Signed area `3*pi/4` | exact Fourier orthogonality |
| Signed centroid `(0,0)` | exact |
| Turning number `-2` | verified |
| Algebraic image degree `16` | exact elimination |
| Projective genus budget `75 + 15 + 15 = 105` | verified |
| Weierstrass tail thresholds | analytic extension, not a seed singularity |

The lab preserves these facts only while the seed coefficients and mode set are
preserved. Once the user mutates the modes, the display becomes an exploratory
member of the finite family.

---

## 3. Hidden clock interpretation

Each enabled mode is an elliptical clock:

```text
k-mode clock:
  x_k(t) = a_k cos(n_k t + p^x_k),
  y_k(t) = b_k sin(n_k t + p^y_k).
```

The visible curve is the sum of these clocks:

```text
C(t) = sum_k C_k(t).
```

The important point is that the curve is a low-dimensional readout of a larger
phase state. One can write the hidden phase vector as

```text
Theta(t) =
  (n_k t + p^x_k, n_k t + p^y_k)_{k in E}
  mod 2*pi.
```

This lives on a product of circles:

```text
Theta(t) in (S^1)^(2|E|).
```

The visible canvas applies a readout map:

```text
Pi(Theta) =
(
  A sum_k a_k cos(theta^x_k),
  A sum_k b_k sin(theta^y_k)
).
```

So the drawing is not merely "a line in the plane." It is a 2D projection of a
coordinated multi-clock phase state. Since the frequencies are integers, the
orbit is periodic, but its projection can still look intricate, braided, and
high-dimensional.

This is the rigorous core behind the "tesseract-like" intuition:

```text
hidden torus orbit -> projected 2D shadow.
```

It is not literally a tesseract. A tesseract is a 4D cube. This object is a
Fourier torus shadow. The perceptual resemblance comes from the same visual
principle: a higher-dimensional organized object is being seen through a
lower-dimensional projection.

---

## 4. Why it feels alive

The lab can feel alive for structural reasons:

1. Low modes set the carrier body.

   The `n=1` and `n=2` clocks usually define the large-scale loop, stalk,
   oval, or backbone.

2. High modes ride on the carrier.

   A high clock such as `n=64`, `n=2048`, or higher creates ribs, teeth,
   braided edges, and fast local oscillations.

3. Lacunarity separates scales.

   With `n_k = B^k`, adjacent modes are not close frequencies. They occupy
   distinct scales. This makes the shape read as nested machinery rather than
   as a single smooth deformation.

4. Projection hides the phase state.

   The state is many angles. The display is two numbers. When the hidden
   angles drift, a small phase change can reorganize the visible projection.

5. Color provides phase-depth cues.

   Phase coloring turns the curve into a pseudo-depth object. Nearby visible
   points can have far-apart parameter values, and the color gradient lets the
   eye infer an unseen ordering.

6. The ribbon renderer thickens time.

   The ribbon view draws a short phase offset beside the curve. The eye reads
   the offset and struts as cross-sections of a rotating surface.

The animation is deterministic. The "alive" quality is the result of a
structured phase projection whose scale hierarchy is visible.

---

## 5. Signed area and energy in the mutable family

For a finite family with distinct frequencies and matching x/y frequency
inside each mode,

```text
x_k(t) = A a_k cos(n_k t + p^x_k),
y_k(t) = A b_k sin(n_k t + p^y_k),
```

Fourier orthogonality gives the signed area contribution

```text
A_k = pi n_k (A a_k)(A b_k) cos(p^x_k - p^y_k).
```

Thus

```text
Area(C) = pi A^2 sum_{k in E} n_k a_k b_k cos(p^x_k - p^y_k).
```

Consequences:

- A high mode can dominate area because of the factor `n_k`.
- If a high mode has `b_k = 0`, it contributes no signed area even if it
  strongly corrugates the visible curve.
- If the x/y phase difference is near `pi/2`, its signed area contribution is
  suppressed.
- Alternating signs produce cancellation ledgers rather than monotone area.

The derivative-energy hierarchy has the schematic form

```text
E_j ~ pi A^2 sum_{k in E} n_k^(2j) (a_k^2 + b_k^2).
```

So high modes can dominate acceleration and higher derivative energy even when
their visual amplitude is small. This is why a tiny high-frequency endpoint
can make the object look tense, jagged, or electrically alive.

---

## 6. Bookend mode regime

A particularly revealing experiment is the "bookend" state:

```text
E = {0, M}.
```

Then the curve is

```text
x(t) = A(a_0 cos t + a_M cos(Nt + p^x_M)),
y(t) = A(b_0 sin t + b_M sin(Nt + p^y_M)),

N = B^M.
```

This is a carrier plus a rider:

```text
slow clock n=1       -> large body
fast clock n=N       -> ribs / scallops / corrugation
```

### 6.1 X-only rider

If the last mode is mostly x-only,

```text
b_M ~= 0,
```

then the high clock moves points horizontally while the slow y-coordinate
still controls the vertical carrier. The result is a ribbed oval or double
column. Signed area remains dominated by the slow mode, because the fast mode
has little or no y-area contribution.

### 6.2 Two-axis rider

If both `a_M` and `b_M` are active, the fast endpoint becomes a small ellipse
riding around the slow carrier. Depending on phase, it can generate:

- scalloped tubes;
- braided bands;
- looped edges;
- self-intersection webs;
- apparent knots in the projection.

These are projection phenomena. The parametrized curve is still one closed
loop unless it becomes singular or decomposes through a degenerate setting.

### 6.3 Parity effects

Parity of `N` matters for mirror branch relations. For example, comparing
points related by

```text
t -> pi - t
```

gives different behavior for `cos(Nt)` depending on whether `N` is even or
odd. This can determine whether the fast rider reinforces a mirror limb or
pushes it toward collision. In the lab, switching base and mode count can
therefore wake up or suppress apparent node formation.

---

## 7. Nodes, crossings, and why counts jump

A plane self-intersection is a pair

```text
t_1 != t_2 mod 2*pi
```

such that

```text
C(t_1) = C(t_2).
```

The seed curve has exact algebraic branch structure, but the lab's node count
is an approximate segment-intersection estimate. It is useful as a live
visual diagnostic, not as a theorem.

In mutable states, node counts jump because:

- high modes add many fast folds;
- phase offsets move near-collisions into or out of exact crossing;
- projection can create crossings even when the hidden phase orbit is
  perfectly regular;
- finite screen sampling can miss or invent near-crossings when
  `samples/n` is too low.

The lab exposes `samples/n` for this reason. A low value means the fastest
active clock is under-resolved.

Rule of thumb:

```text
samples/n < 4      -> mostly aliasing risk
samples/n ~ 8      -> rough curve shape visible
samples/n >= 16    -> much better line readout
samples/n >= 32    -> better for ribbon / node inspection
```

These are visual sampling heuristics, not formal error bounds.

---

## 8. The ribbon / "3D shape" mechanism

The curve itself is a one-dimensional parametrized loop in the plane. The
ribbon renderer constructs a phase-thickened strip.

For a small lag `delta`, define a phase-neighbor curve

```text
C_delta(t) = C(t + delta).
```

The renderer connects corresponding points:

```text
C(t)  <->  C(t + delta).
```

A mathematical surface behind the visual is the ruled strip

```text
R(t,s) = (1-s) C(t) + s C(t + delta),
0 <= s <= 1.
```

This is still drawn in the plane. It becomes visually 3D because the renderer
adds:

- translucent quads;
- struts between phase-neighbor points;
- phase coloring;
- a moving phase marker;
- overlap and occlusion cues from the linework.

The perceived 3D object is therefore a visual lift of a 2D phase strip. It is
not a physical 3D embedding, but it is a valid picture of hidden phase
adjacency.

The lab now includes an optional true 3D visualization lift:

```text
L(t) = ( C_x(t), C_y(t), sigma Z(t) ),
```

where `sigma` is the user-controlled `Z scale` and `Z(t)` is chosen from a
renderer menu:

| Lift mode | `Z(t)` source |
|---|---|
| phase lag | normal component of `C(t + delta) - C(t)` |
| clock phase | sine of the highest active clock phase |
| speed | normalized `|C'(t)|` |
| curvature | normalized signed curvature |
| dominant mode | selected high-energy mode coordinate |
| area sweep | normalized instantaneous wedge `C(t) x C'(t)` |

When `Lift surface` is enabled, the renderer draws a translucent curtain from
the lifted curve back down to the `z=0` floor curve. This gives the eye a
surface to inspect, but it is still renderer-local. Different `Z(t)` choices
are different visual probes of the same finite phase readout, not different
theorem-grade embeddings.

---

## 9. Why the object can look like a tesseract

The tesseract comparison is useful if handled carefully.

What a tesseract rendering usually shows:

```text
4D object -> 3D/2D projection -> apparent impossible motion.
```

What this curve lab shows:

```text
multi-clock torus state -> 2D projection -> apparent impossible surface.
```

The shared mechanism is projection from a larger organized state space. The
difference is the hidden object:

| Visual intuition | Actual object here |
|---|---|
| tesseract | finite Fourier torus orbit |
| rotating 4D cube | rotating phase vector |
| 3D solid | phase-thickened ribbon |
| impossible folding | projection overlap |
| depth | color/strut/overlap cue |

Thus a precise phrase is:

```text
tesseract-like shadow of coupled lacunary clocks.
```

An even more technical phrase is:

```text
phase-ribbon projection of a finite Fourier torus orbit.
```

---

## 10. Absolute grid and jitter

The early lab view refit the camera to the curve's sampled bounding box. That
is attractive for static screenshots, but it is bad for animation:

```text
phase changes -> curve bounds change -> camera rescales/recenters
              -> grid moves -> scene jitters.
```

That creates the illusion that the whole world is shaking.

The corrected view uses a stable world reach computed from active amplitudes:

```text
reach_x = A sum_{k in E} |a_k|,
reach_y = A sum_{k in E} |b_k|.
```

The camera scale is then based on this reach, not on the current sampled
phase. During phase animation:

```text
active amplitudes fixed -> world frame fixed -> grid fixed.
```

This is why the grid now behaves as an absolute reference frame. The shape can
move inside the grid, but the grid no longer breathes with it.

---

## 11. Shape catalogue

The mutable family tends to produce several recurring visual regimes.

### 11.1 Carrier loop

Low modes dominate. The curve looks like an oval, lobe, stalk, or simple
closed loop. Turning number is often stable.

### 11.2 Ribbed carrier

One high mode is enabled with moderate amplitude. The curve remains one large
body but gains teeth, ribs, or corrugated edges.

### 11.3 Phase braid

Two or more high modes interact. The curve looks like a woven band. The ribbon
view makes this especially clear.

### 11.4 Column pair

An x-heavy high endpoint on an elliptical carrier makes two vertical limbs
with horizontal rungs. This is common in bookend mode.

### 11.5 Rosette / gear

Comparable x/y high-mode amplitudes create many small loops around a central
carrier, especially when phases align.

### 11.6 Node web

Large high-mode amplitude or multiple active high modes create many visible
self-intersections. Some are real at the current sampling scale; some may be
near misses or aliasing artifacts.

### 11.7 Rough tail illusion

Many small high modes create visual roughness. With finitely many modes this
is still analytic. True Weierstrass roughness belongs to an infinite-tail
limit and requires separate hypotheses on the decay `lambda`.

---

## 12. Finite analytic versus fractal-looking

Every finite lab state is analytic:

```text
finite trigonometric sum -> real analytic curve.
```

It can look fractal because high modes create scale-separated structure, but
finite mode count never creates an actual fractal curve.

The infinite extension

```text
sum_{k>=0} lambda^k cos(B^k t)
```

has different regularity. The primary curve note records the relevant
Weierstrass-style thresholds:

```text
lambda < 1/B          -> differentiable tail regime,
lambda >= 1/B         -> derivative roughness,
lambda < 1/sqrt(B)    -> area better controlled,
lambda >= 1/sqrt(B)   -> area / rough-path warning regime.
```

For the dyadic case `B=2`:

```text
1/B = 1/2,
1/sqrt(B) = 1/sqrt(2).
```

The lab's "rough, area-controlled" and "rough, area-unstable" labels are
therefore tail-regime warnings, not claims that the finite display has become
non-analytic.

---

## 13. What the lab makes mutable

The interactive workbench exposes:

| Control class | Meaning |
|---|---|
| Presets | seed `C_3`, Fibonacci continuation, geometric tail, QCR-like state, bookend endpoints |
| Mode count | number of stored modes |
| Frequency base | `B` in `n_k = B^k` |
| Mode enable flags | selects the active subset `E` |
| Per-mode `a,b` | x/y amplitudes |
| Per-mode `p^x,p^y` | x/y phase offsets |
| Global phase | moves the point through the hidden clock orbit |
| Rotation/zoom/offset | view transform |
| Samples | discretization resolution |
| Color mode | phase, speed, curvature, or solid |
| Ribbon shadow | phase-thickened strip |
| Rib lag | phase separation `delta` used by the ribbon |
| Ribs | approximate strip density |
| Rib alpha | strip opacity |
| Absolute grid | stable frame based on active amplitude reach |
| 3D lift | Three.js view of `(x,y,z)` phase diagnostics |
| Lift mode | source of `Z(t)`: phase lag, phase, speed, curvature, dominant mode, or area sweep |
| Z scale | signed vertical exaggeration of the chosen lift channel |
| Yaw / pitch / depth | 3D camera orientation and distance |
| Lift surface | translucent curtain from the lifted curve to the floor curve |
| Lift floor / depth fade | absolute 3D floor grid and depth coloring |
| Fiber probe | click-selected target point in the visible readout |
| Probe radius | neighborhood used to collect nearby preimage times |
| Fiber links | lines from the visible target to sampled preimage markers |
| Node genealogy | prefix-mode chart of approximate crossings after adding modes |
| Genealogy samples / cap | renderer sampling budget for prefix node estimates |

The controls are intentionally permissive. They are for exploring shape space,
not for preserving a theorem-grade object at every setting.

---

## 14. Recommended exploration protocols

### 14.1 Seed audit

```text
Preset: C3
Ribbon shadow: off
Color: phase
Nodes: on
Curvature: on
Speed: very low
```

Use this to compare the visual against the exact seed ledger.

### 14.2 Bookend carrier/rider

```text
Preset: Ends
Mode count: 12 or higher
Only modes: first and last
Ribbon shadow: optional
Samples/n: keep >= 16 if possible
```

Use this to study how a slow carrier and a fast endpoint clock interact.

### 14.3 Tesseract-shadow view

```text
Ribbon shadow: on
Rib alpha: low to medium
Rib lag: 1 to 2
Ribs: 300 to 500
Speed: 0.001 to 0.05
Grid: on
```

Use this to see hidden phase adjacency. The absolute grid is important here:
without it, camera breathing can masquerade as object motion.

### 14.4 3D lift probe

```text
3D lift: on
Lift mode: phase lag first, then speed / curvature / dominant mode
Lift surface: on
Lift floor: on
Depth fade: on
Z scale: 0.5 to 1.1
Yaw / pitch: adjust slowly
```

Use this to inspect which diagnostic channel is creating the apparent solid.
The lift is most informative when compared against the planar ribbon view.

### 14.5 Phase-fiber probe

```text
3D lift: off first
Fibers: on
Fiber links: on
Node genealogy: on
Probe radius: small, then widen
Preset: C3, then Ends
```

Click near a crossing or near the fast rider region. Multiple reported
`t`-values mean multiple hidden phase times are landing in the same visible
neighborhood. Compare the node genealogy birth pulse against the exact seed
ledger before interpreting a sampled node family.

### 14.6 Rough-tail probe

```text
Preset: Tail
Mode count: high
lambda: near 1/2 or 1/sqrt(2)
Samples: high
Ribbon shadow: off first, then on
```

Use this to separate true finite high-mode detail from display aliasing.

---

## 15. Regime classifier

The lab includes a regime classifier. It is a renderer-local diagnostic, not a
mathematical classification theorem. Its job is to give names to the visual
state the user is currently steering through.

The classifier reads:

| Signal | Interpretation |
|---|---|
| Active mode count | single clock, bookend, rider/carrier, or mixed family |
| Highest active frequency | visible clock scale |
| Samples per highest cycle | alias risk / inspection quality |
| Approximate node count | web or branch-web tendency |
| `E2/E0` | high-frequency energy concentration |
| Tail ratio lambda | rough-tail warning zone |
| Ribbon state | ribbon or shell-like phase thickening |

The most useful labels are:

| Label | Intended reading |
|---|---|
| `bookend` | only the first and last clocks are enabled |
| `carrier` | a slow large clock is carrying higher detail |
| `rider` | a small high-frequency clock is riding a lower path |
| `braid` | multiple high clocks are interfering visibly |
| `web` | the plane projection has many apparent crossings |
| `branch-web` | high crossing count plus ribbon readout |
| `ribbon` | phase offset is being rendered as a strip |
| `shell` | ribbon opacity/density/lag makes the strip read as a surface |
| `rough-tail` | geometric tail is near a roughness threshold |
| `alias-risk` | the display is under-sampling the highest active clock |

These names are deliberately phenomenological. They help preserve the user's
geometric intuition while keeping epistemic status clear:

```text
visual label
  != exact singularity type
  != proof of a physical mechanism
  != invariant of the underlying curve family.
```

---

## 16. Phase fibers and node genealogy

The click probe treats the visible curve as a many-to-one readout:

```text
t in S^1  ->  C(t) in R^2.
```

For a selected target point `p`, the ideal phase fiber is

```text
F_p = { t : C(t) = p }.
```

The lab does not solve this equation exactly. It samples the current curve,
looks for local distance minima to `p`, refines nearby candidates, and reports
the visible neighborhood

```text
F_{p,r} = { t : |C(t) - p| <= r }
```

for the current probe radius `r`. This is a microscope for hidden preimage
structure: self-intersections appear as multiple phase times stacked at one
visible location.

The node genealogy strip asks a different question. For each prefix

```text
C_{<=k}(t) = sum_{j <= k} mode_j(t),
```

it estimates the number of apparent crossings after modes up to `k` have been
included. Large jumps in the strip mark visual "birth pulses" where a newly
enabled octave creates many new sampled crossings.

Both tools are renderer-local:

| Probe | What it reveals | What it is not |
|---|---|---|
| phase fiber | nearby hidden times mapping to one visible point | exact fiber cardinality |
| node genealogy | sampled prefix crossing growth | proof of singularity continuation |
| birth pulse | where a mode visibly adds crossings | invariant bifurcation theorem |

The exact seed singularity ledger remains the proof-oriented reference. The
fiber and genealogy tools are for inspection, intuition, and hypothesis
formation.

---

## 17. Epistemic guardrails

This document permits the following statements:

```text
[curve-local] The seed C_3 has the exact properties verified in the primary
              dyadic lacunary curve note.

[renderer-local] The ribbon view draws a phase-offset strip between C(t)
                 and C(t+delta).

[renderer-local] The 3D lift draws (C_x(t), C_y(t), sigma Z(t)) with a
                 user-selected diagnostic channel Z(t).

[renderer-local] The phase fiber microscope reports sampled nearby preimage
                 times for a clicked visible target.

[renderer-local] The node genealogy strip estimates prefix-mode crossing
                 counts using the current renderer sampling budget.

[visualization] The ribbon can be perceived as a 3D/tesseract-like object
                because it is a projection of hidden phase adjacency.

[open] A future analytic fiber solver could replace the sampled microscope for
       special finite algebraic cases.
```

This document does not permit the following statements:

```text
The lab derives physical 3D space.
The curve proves an FTD particle mechanism.
The tesseract-like visual is evidence of literal 4D geometry.
Mutated coefficient settings are new FTD derivations.
Approximate node counts are theorem-grade singularity counts.
Fiber probe counts are exact preimage cardinalities.
```

The honest interpretation is:

```text
finite multi-clock phase system
  -> lossy two-coordinate readout
  -> visually rich projection geometry
  -> useful intuition for hidden-state/readout separation.
```

---

## 18. Relation to FTD intuition

FTD repeatedly uses the distinction between hidden structure and manifest
readout. This curve lab is not evidence for any physical FTD claim, but it is
a good toy model for that distinction:

```text
hidden phase coordinates:
  many circular clocks

visible manifestation:
  two projected coordinates

observer effect:
  apparent complexity generated by projection, sampling, and rendering
```

The useful lesson is not "this curve is physics." The useful lesson is:

```text
simple hidden rules can generate rich visible forms when the readout is lossy
and phase-structured.
```

That is an intuition worth preserving, provided it stays in the exploratory
mathematical layer.

---

## 19. Verification and artifacts

Primary exact verifiers:

```text
python scripts/proofs/proof_dyadic_lacunary_fourier_curve.py
python scripts/proofs/proof_dyadic_curve_singularity_budget.py
```

Interactive artifact:

```text
dissemination/interactive/dyadic_lacunary_curve_lab.html
dissemination/interactive/vendor/three.global.js
```

Renderer features referenced here:

```text
showRibbon
ribbonLag
ribbonDensity
ribbonAlpha
showLift3d
showLiftSurface
liftMode
liftScale
liftYaw
liftPitch
showFiberProbe
fiberRadius
showNodeGenealogy
stableWorldReach()
maxActiveFrequency()
classifyRegime()
updateRegimeClassifier()
findFiberMatches()
computeNodeGenealogy()
```

This note itself is a documentation synthesis and visualization explanation.
It introduces no new theorem-grade FTD claim.

*End of document.*
