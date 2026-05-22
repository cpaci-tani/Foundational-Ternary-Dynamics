# Self-Dual Half-Shell Exploration

**Status:** [EXPLORATORY]  
**Script:** `scripts/exploration/ftd_self_dual_half_shell.py`  
**Date:** 2026-04-22

---

## Question

The number `1/2` appears in several places:

```text
elliptic self-duality: parameter m = k^2 = 1/2
dual-cell geometry:    edge-center radius squared r^2 = 1/2
leapfrog dynamics:     wave velocity lives on half-time steps
```

This note asks whether the dual-cell `r^2 = 1/2` shell is the FTD-native
geometric shadow of the lemniscatic self-dual point.

This is not a derivation of alpha. It is a fixed structural audit.

---

## Exact Mathematical Side

For elliptic integrals, self-duality is:

```text
k = k'
k^2 = 1 - k^2
k^2 = 1/2
```

So the self-dual parameter is:

```text
m = k^2 = 1/2
```

The complete elliptic integral at this point reconstructs `G*`:

```text
K(1/2) = Gamma(1/4)^2 / (4 sqrt(pi))
G*     = 2 sqrt(2) K(1/2) / sqrt(pi)
```

The theta-function route gives the same constant at the self-dual nome:

```text
q = e^-pi
G* = sqrt(2 pi) theta_3(q)^2
```

Classification:

```text
[THEOREM] m=1/2 is the elliptic self-dual parameter.
[THEOREM] K(1/2) reconstructs G*.
[THEOREM] q=e^-pi is the Fourier self-dual theta nome.
```

---

## Dual-Cell Geometric Side

For the unit voxel centered at the origin, the dual-cell boundary has natural
half-offset shells:

```text
dual face centers: (+/-1/2, 0, 0)             r^2 = 1/4
dual edge centers: (+/-1/2, +/-1/2, 0)        r^2 = 1/2
dual corners:      (+/-1/2, +/-1/2, +/-1/2)   r^2 = 3/4
```

With `c_FTD = 1/sqrt(3)`, their geometric travel times are:

```text
dual face:   t = sqrt(3)/2
dual edge:   t = sqrt(3/2)
dual corner: t = 3/2
```

Classification:

```text
[THEOREM] the dual edge shell has r^2 = 1/2.
[THEOREM] r^2 = 1/2 is the exact midpoint between dual face r^2=1/4
          and dual corner r^2=3/4.
[THEOREM] under the unit-radius complement map r^2 -> 1 - r^2, the dual edge
          shell is the fixed point: 1 - 1/2 = 1/2.
[THEOREM] for trilinear interpolation of a primal center impulse to half-offset
          boundary points, face and edge shells carry equal total linear
          weight.
[SELECTION] identifying this shell with the elliptic self-dual datum is a
            bridge hypothesis, not yet a theorem.
```

---

## Current Bridge Hypothesis

The cleanest current hypothesis is:

```text
G* is not primarily a BCC correction.
G* is the self-dual normalization of the primal/dual FTD bridge.
```

In this reading:

```text
primal voxel lattice       = manifestation lattice
dual-cell boundary         = J*J / flux-response geometry
dual edge r^2 = 1/2        = geometric self-dual shell
elliptic m = 1/2           = analytic self-dual point
G*                         = normalization connecting the two descriptions
BCC/stella                 = k=3 Moore causal/parity layer
```

This keeps the BCC result from the Moore audits, but it does not force BCC to
carry all of `G*`.

---

## Audit Output

Run:

```text
python scripts/exploration/ftd_self_dual_half_shell.py
```

Expected checks:

```text
[THEOREM] Elliptic self-dual parameter
  parameter m                 = 0.5
  modulus k=sqrt(m)           = 0.707106781186547524400844
  complement k'=sqrt(1-m)     = 0.707106781186547524400844

[THEOREM] K(1/2) reconstructs G*
  G* from K(1/2)              = 2.9586751191886388...

[THEOREM] Dual-cell half-offset shells
  dual face centers           r^2 = 1/4
  dual edge centers           r^2 = 1/2
  dual corners                r^2 = 3/4

[THEOREM] Dual-edge self-complement
  midpoint(face r^2, corner r^2) = 0.5
  1 - edge r^2                   = 0.5

[THEOREM] Trilinear half-offset impulse weights
  face shell:   6 * 1/2 = 3
  edge shell:  12 * 1/4 = 3
  corner shell: 8 * 1/8 = 1
  total = 7
```

The final integer `7` is recorded as an exact outcome of this interpolation
audit. It is not treated here as a derivation of any physics constant.

---

## Open Dynamical Test

The next fixed test should ask:

```text
Does the engine give the dual-edge shell a special role under primal/dual
exchange, Gauss closure, or action balance?
```

Possible fixed audits:

```text
1. Compare face/edge/corner dual-cell flux energy after exact dual-cell Gauss.
2. Test whether primal-to-dual and dual-to-primal projection commute most
   cleanly at the r^2 = 1/2 shell.
3. Measure half-step leapfrog energy exchange and ask whether the midpoint
   shell is where J and wave_vel balance.
```

None of these should use alpha or a target numerical match.

---

## Dynamical Half-Shell Audit

Implemented:

```text
engine/tests/test_native_dual_half_shell.cpp
ctest --test-dir engine/build -C Release -R "^native_dual_half_shell$" --output-on-failure
```

Method:

```text
1. Seed a neutral +/- source pair.
2. Solve exact finite-volume dual-cell Gauss.
3. Reconstruct cell-centered projected flux from adjacent face fluxes.
4. Trilinearly sample the half-offset shells around each source.
```

Raw result:

```text
shell         r^2     count   mean|J|        mean|J|^2      total|J|^2     mean s*(J.rad)
dual face     0.25       12   1.26682335e-01 1.75611074e-02 2.10733288e-01 5.27060522e-02
dual edge     0.50       24   7.74118831e-02 6.75617975e-03 1.62148314e-01 4.61015745e-02
dual corner   0.75       16   4.77855221e-02 2.63144831e-03 4.21031730e-02 3.60843918e-02

edge energy fraction = 0.390733163
face:edge:corner total energy =
  0.210733288 : 0.162148314 : 0.0421031730
```

Interpretation:

```text
[MEASURED] the dual-edge r^2=1/2 shell carries resolved projected energy.
[MEASURED] in this fixed source-pair audit, the edge shell carries about 39%
           of the sampled half-shell energy.
[OPEN] this does not yet prove that r^2=1/2 is the unique dynamical origin of
       G*. It promotes the half-shell from pure geometry to a measurable
       engine response channel.
```

---

## Ledger Impact

This closes the weak open item:

```text
Is the r^2 = 1/2 shell only a visual/geometric analogy?
```

Answer:

```text
No. It is an exact dual-cell shell and a measured projected-flux response
channel in the current engine audit.
```

It does not close the stronger open item:

```text
Does FTD dynamically derive G* from the r^2 = 1/2 shell?
```

Current status:

```text
[THEOREM] elliptic self-duality has m = 1/2.
[THEOREM] the dual-cell edge shell has r^2 = 1/2.
[MEASURED] the dual-edge shell carries native response energy.
[CONJECTURE] G* is the primal/dual bridge normalization.
[OPEN] derive or falsify that conjecture from an FTD action, projection
       commutator, or half-step energy-balance principle.
```
