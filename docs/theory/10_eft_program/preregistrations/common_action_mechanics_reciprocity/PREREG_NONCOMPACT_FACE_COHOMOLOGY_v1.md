# Pre-registration — Noncompact face cohomology and localized-carrier gate (FTD-0583)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-26  
**Parents:** FTD-0427, FTD-0478, FTD-0563, FTD-0564, FTD-0574,
FTD-0576, FTD-0582.  
**Production changes permitted:** none. Observer records, an exact independent
proof, one native C++ test, a theorem, an audit, and registry reconciliation
only.

## 1. Question

After FTD-0582 closes native `(J,W)` backreaction in the frozen tick, the live
manifested-matter tracker retains a possible protected defect or bundle class.
Before introducing a compact connection, singular branch, or nonlinear core,
this campaign asks what topology the already selected matched face/edge fields
actually possess:

```text
Can the existing real, noncompact periodic face/edge complex carry a
localized topologically protected matter sector, or are its only cohomology
classes global continuously-valued fluxes?                              (1)
```

The answer is scoped to the current matched operators. It is not a theorem
about compact U(1), singular connections, nonlinear defects, boundaries, or
new carrier variables.

## 2. Exact theorem targets

Let `C` be `matched_curl` from periodic edge cochains to oriented face
cochains and `D` the backward face divergence. Prove:

1. `D C=0` exactly.
2. At Fourier momentum `k`, write
   `q_i=1-exp(-i k_i)`. For every `k!=0`, `rank C(k)=2` and
   `im C(k)=ker D(k)`. At `k=0`, both symbols vanish and the face quotient has
   dimension three.
3. Therefore

   ```text
   H^2(T_L^3;R)=ker D/im C = R^3,                         (2)
   ```

   represented by constant `x`, `y`, and `z` face fluxes. The complete real
   periodic cubical complex has Betti dimensions `(1,3,3,1)`.
4. The three invariants are the fluxes through noncontractible coordinate
   two-tori. They are plane-independent when `D E=0`, unchanged by any local
   curl update, spatially global, and continuously valued because the field
   coefficients live in `R`.
5. Every divergence-free face field with zero three plane fluxes is a curl.
   Hence every localized zero-harmonic field has the explicit contraction

   ```text
   E_t=t E,  0<=t<=1,                                    (3)
   ```

   inside the same divergence-free sector, with energy `U(E_t)=t^2 U(E)`.
6. Periodicity forces `sum_x D E(x)=0`. A real dipole field and its local
   Gauss charges scale continuously under `E_t=tE`; the existing face field
   therefore supplies no charge quantization or nonzero topological floor by
   itself.

The theorem must explicitly distinguish a discrete ternary source value from
a topological class of the continuous field responding to it.

## 3. Locked observer campaign

### 3.1 Fourier and harmonic classification

- volumes `L in {3,4,5,8}`;
- all `L^3` Fourier momenta, for `728` mode arms total;
- verify the zero/nonzero-momentum rank classification and Betti dimensions;
- constant harmonic basis in all three axes and amplitudes
  `{-1,-1/2,+1/2,+1}`: `48` harmonic arms;
- require plane-flux normalization and plane independence below `1e-12`;
- add deterministic local curl fields and require all three harmonic fluxes
  remain unchanged below `1e-12`.

### 3.2 Local contraction

- two deterministic single-edge potentials for each volume and edge axis:
  `24` localized-curl arms;
- contraction parameters `t in {0,1/4,1/2,3/4,1}`: `120` samples;
- require divergence, harmonic flux, support containment, curl construction,
  and `U(tE)-t^2 U(E)` residuals below `1e-12`;
- every nonzero localized fixture must have positive energy at `t=1` and zero
  energy at `t=0`.

### 3.3 Continuous Gauss-dipole scaling

- all four volumes, three axes, both polarities, and amplitudes
  `t in {0,1/4,1/2,3/4,1}`: `120` charge-scaling arms;
- one source/sink pair is seeded by the exact matched face path;
- require total periodic charge exactly zero, source and sink values linear in
  `t`, all other divergence zero, and the discrete surface-telescope identity
  below `1e-12`.

### 3.4 Cubic covariance

- apply all `24` proper cubic rotations to one local curl representative and
  one harmonic vector;
- require rotated divergence, energy, and harmonic-flux vector covariance
  below `1e-12`.

## 4. Independent proof and frozen provenance

The independent Python proof must derive the Fourier Koszul-complex ranks,
enumerate every registered momentum, verify `(1,3,3,1)`, prove the contraction
and continuous-charge statements algebraically, and reject the inference from
real noncompact flux to an integral compact-U(1) bundle class.

The following production/selected-operator files are read-only during the
campaign and their pre-run SHA256 values must be recorded in the audit:

- `engine/include/ftd/eft/matched_gauss_transport.h`;
- `engine/src/eft/matched_gauss_transport.cpp`;
- `engine/include/ftd/voxel.h`;
- `engine/src/render_bridge_phases/phase_read.cpp`;
- `engine/src/render_bridge_phases/phase_write.cpp`.

## 5. Outcome map

If all theorem and campaign gates pass, record:

```text
MATCHED_NONCOMPACT_COHOMOLOGY_GLOBAL_ONLY_LOCAL_PROTECTED_DEFECT_CLOSED
```

This closes a localized protected bundle/defect carrier made solely from the
current real matched face/edge variables. It leaves open a genuinely nonlinear
deforming `(s,J,W)` core and explicitly new compact/singular variables.

If a nonzero localized cohomology class, noncontractible zero-harmonic field,
quantized real-field dipole, or unexplained rank excess is found, the negative
verdict is forbidden and the branch remains open.

No result licenses FTD-0481, a scenario, a particle identification, charge
quantization, compact U(1), or an infrared/Lorentz claim.
