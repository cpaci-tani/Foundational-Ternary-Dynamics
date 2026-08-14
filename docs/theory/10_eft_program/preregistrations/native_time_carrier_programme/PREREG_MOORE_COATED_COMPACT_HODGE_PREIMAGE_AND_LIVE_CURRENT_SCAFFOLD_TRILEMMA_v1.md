# FTD-0921 — Moore-coated compact-Hodge preimage and live-current scaffold trilemma v1

**Identifier:** `FTD-0921`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact finite-support representability of the FTD-0920 coated
return, compact transverse control, and the live `j=s v` source tie; no
numerical search and no engine change

## 1. Question

FTD-0920 proved that the FTD-0577 Moore coat removes all eight real
zero/Nyquist source-cokernel components of the plaquette return. That proves
existence of a relaxed **global periodic** Hodge preimage. Does the same
coated return admit a finite-support preimage under the central gradient/curl
source? If not, can a redesigned transverse carrier be actuated compactly,
and can the live production identity `j=s v` realize that compact curl-only
actuation without a nonlocal manifested scaffold?

The three questions are kept separate:

1. compact relaxed source for the coated scalar-polarized plaquette;
2. compact relaxed source for a transverse/curl carrier; and
3. compact live source with `s in {-1,0,+1}` and `j=s v`.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `AUDIT_NATIVE_FIELD_DISCRETE_ACTION.md` | `5EDC7F8C81456BEE4EEB061168154E8EF4D8347B8948C429BB40B8306FFC8AD8` |
| `AUDIT_NATIVE_HODGE_ENERGY_CONTINUITY.md` | `033985919FAC722F47B09311D51B47E5DDB4E5A3A47D0A3F36B736CFAF481D08` |
| `THEOREM_MINIMAL_MOORE_COMPATIBILITY_COAT.md` | `49F41E31DFA9542B2BD7AB0A224808C48D06164967A71139D9C4B7BFB5EBA7B7` |
| `THEOREM_NONCOMPACT_FACE_COHOMOLOGY.md` | `4F0AA19A00A2A96215031139994AD0AC1AC7C93BBE5620E7F3FF99CCCCB62C70` |
| `THEOREM_NATIVE_CENTRAL_HODGE_SOURCE_COKERNEL_AND_PLAQUETTE_RETURN_BOUNDARY_v1.md` | `BC99B6A5D2D7B75FD2564199C4265ABA8AE5FC87C00637DA38F6D57334004EA8` |

The certificate fails closed on source drift.

## 3. Frozen Laurent source algebra

Work in the finite-support Laurent domain

\[
 R=\mathbb Q(i,\sqrt2)[z_x^{\pm1},z_y^{\pm1},z_z^{\pm1}].
\]

Define central-difference symbols

\[
 d_i={z_i-z_i^{-1}\over2},
 \qquad
 D=d_x^2+d_y^2+d_z^2.
\]

After removing the nonzero factor `G_C`, the relaxed production source is

\[
 U=-d\,s+d\times j.
\]

Taking its algebraic longitudinal projection gives the exact necessary
identity

\[
 \boxed{d\cdot U=-D s.}
\]

Therefore any finite-support source preimage must make `d dot U` divisible by
`D`. Equivalently, `d dot U` must vanish at every complex Laurent point where
`D=0`.

This condition is stronger than the eight real unit-torus corner conditions
of FTD-0920. Passing those corners is necessary but not sufficient for a
finite Laurent preimage.

## 4. Frozen coated-plaquette obstruction

Use the FTD-0920 scalar plaquette symbol and FTD-0577 coat

\[
 f=1-z_xz_y,
 \qquad
 B_M=\prod_i {1+c_i\over2},
 \qquad
 c_i={z_i+z_i^{-1}\over2}.
\]

The production stiffness is

\[
 K={4\over3}-{2\over9}
 (c_x+c_y+c_z+c_xc_y+c_yc_z+c_zc_x).
\]

For a real desired body stiffness `kappa`, take the fixed-polarization return

\[
 U=t e_x,
 \qquad
 t=(K-\kappa)B_Mf.
\]

The certificate must evaluate the exact complex Laurent point

\[
 a=\sqrt2,
 \qquad
 z_x=z_y=1+a,
 \qquad
 z_z=i(1+a).
\]

At this point,

\[
 d=(1,1,ia),
 \qquad
 c=(a,a,i),
 \qquad
 D=1+1+(ia)^2=0.
\]

It must also prove

\[
 B_Mf\ne0,
\]

and

\[
 \operatorname{Im}K=-{2\over9}(1+2a)\ne0.
\]

Since `kappa` is real, `K-kappa` and hence `t` are nonzero. Because
`d dot U=d_x t=t`, the necessary identity would require a nonzero quantity
to vanish at `D=0`. Therefore the Moore-coated scalar-polarized plaquette
return has no finite-support relaxed `(s,j)` preimage for any real `kappa`.

This result is scoped to exact compact support. It does not contradict the
FTD-0920 global periodic preimage and does not exclude noncompact tails.

## 5. Frozen transverse compact control

The compact-source obstruction must not be generalized to every carrier.
Let `A in R^3` be any finite-support vector potential and define

\[
 J=d\times A.
\]

Because the scalar convolution `K` commutes with central curl,

\[
 \boxed{
 (K-\kappa)J
 =d\times[(K-\kappa)A].}
\]

Thus the redesigned transverse carrier has an exact compact **relaxed**
return with

\[
 s=0,
 \qquad
 j=(K-\kappa)A.
\]

This is a construction-class witness only. It does not prove a `C4` doublet,
formation, stability, energy closure, or live production realization.

## 6. Frozen live `j=s v` obstruction

Production does not permit an independent current. It fixes

\[
 s\in\{-1,0,+1\},
 \qquad
 j=s v.
\]

For a compact transverse live source, `d dot U=0`. The longitudinal identity
then gives

\[
 D s=0.
\]

The Laurent ring is an integral domain and `D` is nonzero, so compact support
forces

\[
 s=0.
\]

The live support tie then forces `j=s v=0` and hence `U=0`. Therefore:

\[
 \boxed{
 \text{no nonzero compact transverse source exists with the live tied pair}
 (s,sv).}
\]

This is stronger than ternary amplitude quantization: it holds even if the
compact `s` values are allowed to be arbitrary real numbers.

## 7. Frozen periodic-scaffold escape and ontic price

On an even periodic quotient, the symbol of `D` vanishes exactly at the eight
zero/Nyquist modes. Hence

\[
 \ker D
 =\{s:s(x+2e_i)=s(x)\text{ for }i=x,y,z\}.
\]

Such a field is constant on each of the eight site-parity classes. With the
ternary restriction there are exactly `3^8` central-gradient-null periodic
scaffolds. Exactly `2^8` of them are nonzero on all parity classes and can
gate an arbitrary compact current by setting

\[
 v={s j\over G_C}
\]

because `s^2=1` on those scaffolds.

Every nonzero such scaffold is noncompact on the uncontained lattice and is
a globally manifested two-periodic background, not a bounded clock body.
Adopting it as vacuum/current hardware therefore requires an explicit ontic
and energy-background selection. It is not free closure.

The alternative escape—making current independent of `s`—adds a new source
type and must be booked as such. A third route is to accept a longitudinal
source and/or noncompact evanescent carrier tails.

## 8. Frozen trilemma

If all exact gates pass, book the following trilemma:

1. **Coated scalar plaquette:** real blind modes are removed, but no compact
   relaxed Hodge preimage exists.
2. **Transverse carrier with independent relaxed current:** exact compact
   return exists.
3. **Transverse carrier with live `j=s v`:** compact realization is zero;
   nonzero realization requires a noncompact two-periodic ternary scaffold,
   an independent current type, or a different longitudinal/tail mechanism.

## 9. Outcome rules

- **Outcome A — compact-source/live-tie trilemma:** all three branches and
  the periodic-scaffold classification pass. Book the exact boundary and
  advance only the declared escape routes.
- **Outcome B — compact live witness:** an explicit finite-support nonzero
  ternary `s`, velocity `v`, and return source satisfy the frozen equations.
  The witness must identify exactly which premise above fails.
- **Outcome C — invalid execution:** any source lock, Laurent identity,
  complex witness, periodic count, production marker, or scope firewall
  fails. Book no theorem.

## 10. Required certificate gates

The exact certificate must cover:

1. all frozen hashes;
2. central-difference/Cayley identities at the complex point;
3. `D=0`, `B_M f!=0`, and nonreal `K`;
4. the compact coated-return contradiction for arbitrary real `kappa`;
5. exact curl commutation and transverse relaxed construction;
6. integral-domain compact live-current obstruction;
7. all even-periodic `D` null modes on `L=4` and `L=6`;
8. equivalence with eight parity-class constants;
9. exact `3^8` ternary and `2^8` fully supporting scaffold counts;
10. production `-grad s+curl(sv)` and kick--drift markers;
11. unchanged engine/type/import status; and
12. no `G*`, gamma, Born/Bell, context, measurement, fit, sweep, or near-miss
    read.

## 11. Frozen scope ceiling

Success does not derive a physical clock, an acceptable global background,
an independent current type, a compact `C4` transverse body, formation,
stability, reciprocal source reaction, positive storage, finite energy,
mobility, the `G*` gearbox, gamma, Born frequencies, Bell correlations, or
preferred-tick hiding. It only closes the exact compact-source branch and
identifies the minimum ontic alternatives.
