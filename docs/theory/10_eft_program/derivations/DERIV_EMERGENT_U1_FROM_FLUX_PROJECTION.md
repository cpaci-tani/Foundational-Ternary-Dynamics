# Emergent U(1) from Flux Projection

**Date:** 2026-04-22
**Status:** [PARTIAL] bridge result; emergent redundancy, not primitive gauge ontology
**Purpose:** Decide whether FTD needs microscopic U(1) gauge structure, or whether U(1) is an effective description of projected flux degrees of freedom.

---

## Executive result

FTD does **not** need microscopic U(1) gauge structure to interpret its native degrees of freedom.

The native structure is:

```text
s(x)       signed ternary manifestation state
J_i(x)     physical/dispositional vector flux
div J      source/manifestation constraint
J_T        transverse propagating flux modes
```

U(1)-like gauge structure enters only after projection:

```text
J -> J_T = P_T J
```

where `P_T` is the transverse Helmholtz projector. Once observables are restricted to transverse flux/radiation modes, adding a pure gradient to an auxiliary potential does not change the projected field:

```text
P_T(A + grad chi) = P_T A.
```

That is an emergent redundancy of the **description**, not a microscopic redundancy of the FTD flux field itself.

The bridge claim should therefore be:

```text
FTD native layer: physical flux DoF
EFT layer: U(1)-like redundancy of transverse coarse observables
QED layer: additional matter/gauge completion still required
```

---

## Why microscopic U(1) is not required

The state/flux dictionary in `DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md` showed that the native coupling

```text
L_int = -g_c s div J
```

is not invariant under

```text
J -> J + grad chi
```

for a general ternary source configuration. Therefore `J` should not be treated as a primitive gauge potential.

But this does not damage FTD. A physical vector flux field is already a legitimate set of degrees of freedom. It can have:

- source response
- Gauss projection
- Coulomb-like lattice Green functions
- transverse wave modes
- two propagating polarizations after constraint

Those are DoF facts, not necessarily gauge-ontology facts.

---

## Helmholtz projection on the lattice

On a finite periodic cubic cell, any nonzero Fourier mode of a vector field decomposes as:

```text
J_i(k) = J_T,i(k) + J_L,i(k)
```

with

```text
khat_i J_T,i(k) = 0
J_L,i(k) = khat_i [khat_j J_j(k)] / khat^2.
```

Equivalently:

```text
P_L,ij(k) = khat_i khat_j / khat^2
P_T,ij(k) = delta_ij - P_L,ij(k)
J_L = P_L J
J_T = P_T J
```

The Gauss/source constraint fixes the longitudinal part:

```text
div J = rho
```

so for nonzero modes:

```text
J_L(k) is determined by rho(k).
```

The unconstrained propagating field content is therefore the transverse part `J_T`, with two independent components per nonzero mode.

Status:

- Helmholtz decomposition on periodic lattice: **[THEOREM]**
- Gauss constraint fixing longitudinal flux: **[THEOREM]** given the constraint
- interpretation of transverse modes as photon-like DoF: **[SELECTION]**

---

## Emergent equivalence relation

Define coarse electromagnetic observables to depend only on:

```text
rho = div J
J_T = P_T J
```

Then two microscopic flux configurations are equivalent for those coarse observables if:

```text
J ~ J'  iff  div J = div J'  and  P_T J = P_T J'
```

On a periodic cell without harmonic zero-mode distinctions, this equivalence is narrow: specifying both `div J` and `P_T J` essentially fixes `J`. That is why the native FTD flux field is physical, not gauge-redundant.

The U(1)-like redundancy appears one level higher, when the transverse field is represented by an auxiliary potential `A`:

```text
J_T = P_T A
```

Then:

```text
A ~ A + grad chi
```

because:

```text
P_T grad chi = 0.
```

This is the exact sense in which U(1) is emergent:

> U(1) is not a microscopic freedom to alter physical flux. It is a redundancy in the auxiliary potential used to represent the transverse projected flux.

---

## What this gives FTD

This branch preserves the best parts of the old U(1) story:

1. **Two propagating modes:** Gauss constraint removes the longitudinal mode from propagation.
2. **Coulomb sector:** longitudinal flux is fixed by the source through the lattice Poisson equation.
3. **Radiation sector:** transverse flux carries wave-like propagation.
4. **Gauge language:** auxiliary potentials may be used for the transverse sector, with `A ~ A + grad chi`.
5. **Ward-like constraints:** any EFT written in auxiliary-potential variables must respect the projection redundancy.

But it avoids the false stronger claim:

```text
microscopic J is literally a U(1) gauge potential.
```

---

## What this does not yet give

Emergent projection redundancy is not enough to claim full QED.

Still missing:

1. **Matter representation:** how ternary excitations become scalar, Dirac, or other charged matter fields.
2. **Local matter coupling:** how matter couples locally to the auxiliary potential through phases or covariant derivatives.
3. **Charge quantization:** why the EFT charge unit is the physical electron charge.
4. **Counterterms and regulator:** which projected operator and Brillouin-zone prescription define the renormalized coupling.
5. **Alpha observable:** why `x_+` is the physical Thomson `1/alpha`, rather than an arithmetic/eigenvalue match.

So the proper claim is:

```text
Emergent U(1)-like projection: supported
Full electromagnetic QED matching: still open
```

---

## Consequence for Structure-2

The Structure-2 scalar gauge completion treated U(1) as a full gauge theory with charged scalar matter and Peierls link phases. That was a valid test of one possible EFT completion.

Its negative result now has a clearer interpretation:

```text
Natural scalar gauge completion of the projected-U(1) branch
does not reproduce the Structure-1 ppb alpha closure.
```

It does **not** falsify native FTD flux dynamics, because native FTD was never required to be microscopic scalar QED.

---

## Consequence for alpha

This result weakens the old overclaim and strengthens the bridge discipline:

```text
x_+ = 137.036... as arithmetic root                  intact
x_+ as native FTD flux/eigenvalue scale              plausible
x_+ as physical 1/alpha_EM                           not derived under current projected action
Structure-1 ppb correction as scheme-specific EFT    still scheme-specific
```

After the QED-alpha bridge closure, the next step is not "prove J is primitive U(1)." The better target is:

> define FTD-native source/flux response observables, using the projected transverse description only where it helps.

---

## Updated bridge statement

The bridge should now be stated as:

```text
FTD microscopic ontology:
    ternary state s + physical vector flux J

Constraint decomposition:
    J = J_L[rho] + J_T

Coarse electromagnetic variables:
    source rho, transverse flux J_T

Auxiliary EFT representation:
    J_T = P_T A, with A ~ A + grad chi

Native replacement problem:
    derive C_L^FTD, K_T^FTD, Z_j^FTD, g_sJ^FTD, and native flow laws
```

This is more natural than treating `J` itself as a gauge potential. It also aligns with the quantum-mechanics documents where complex amplitudes arise from the two transverse flux degrees of freedom after Gauss projection.

---

## 2026-04-22 matter-coupling follow-up

`DERIV_PROJECTED_EFT_MATTER_COUPLING.md` records the next bridge span:

```text
native matter       signed source/worldline matter from s
projected coupling  rho fixes Coulomb sector; j_T couples to A_T
QED-facing matter   Dirac completion is preferred but still selected
```

The QED-facing charge-normalization route later closed negative under the current projected action. The active replacement is `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`: native source/flux response observables are primary, and QED comparisons are external diagnostics.
