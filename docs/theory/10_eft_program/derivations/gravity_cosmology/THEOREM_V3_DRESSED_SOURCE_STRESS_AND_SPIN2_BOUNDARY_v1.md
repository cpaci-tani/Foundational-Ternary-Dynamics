# V3 dressed-source stress and spin-2 boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — CHARGE-EVEN DRESSED-SOURCE $E_g$ STRESS]** +
**[SCOPED NO-GO — ONE-RECORD $T_{2g}$ SHEAR ABSENT]** + **[SCOPED NO-GO —
SELECTED COLLISION DOES NOT PROTECT EVEN THE $E_g$ QUADRATURE]** + **[OPEN —
GRAVITY CARRIER, POLE, CONSTRAINTS, UNIVERSAL COUPLING, CONE, AND LENSING]**  
**Carrier price:** no new type for the source readout; a complete tensor carrier
remains unresolved  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[`proof_v3_dressed_source_stress_spin2_boundary.py`](../../../../../scripts/proofs/proof_v3_dressed_source_stress_spin2_boundary.py)
passes 11/11 v3 gates after the parent 128,499-check exact collision
certificate, including 27,648 signed-cubic covariance rows.

---

## 1. Stress carried by one field record

At cotangent layer `ell`, one channel has perpendicular unit polar/axial
readouts `E` and `B`. Define

\[
 \Sigma=EE^{\mathsf T}+BB^{\mathsf T}-{2\over3}I.
\]

It is symmetric and trace free, and under every signed-cubic transformation
`M`,

\[
 \Sigma\mapsto M\Sigma M^{\mathsf T}.
\]

Because `E` and `B` are always coordinate-axis directions, every off-diagonal
entry vanishes. The one-record stress span is therefore exactly the two-
dimensional diagonal representation `E_g`, not the complete

\[
 E_g\oplus T_{2g}
\]

five-dimensional spatial STF representation. C4 phase supplies two
quadratures and raises the rank only to four. It does not manufacture the
three missing shear coordinates.

---

## 2. The dressed source has a common stress record

For the complete eight-channel source packet along `d`, summing through every
one of the twelve C4/C3 frames gives

\[
 \boxed{
 \Sigma_{\rm source}
 =4\left(dd^{\mathsf T}-{1\over3}I\right).}
\]

This source is charge-conjugation even: opposite electric polarities carry
opposite charge/field but the same quadratic stress. Consequently the same
finite object already supplies:

- charge-odd electric incidence; and
- charge-even anisotropic source stress.

That is a genuine common-source result. It does not yet provide a gravitational
degree of freedom.

---

## 3. Collision boundary

The selected three-layer collision preserves exactly number plus the six
electric/magnetic first moments. Applying its exact correction matrices to the
C4-weighted `E_g` stress rows gives leakage rank

\[
 (4,4,4)
\]

on the three C3 layers. Thus even the available rank-four stress quadrature is
not a protected slow sector of `Phi-v2`.

The precise v3 gravity status is therefore:

```text
common charge/stress source: present
diagonal Eg tensor readout: present
T2g shear carrier: absent at one-record level
protected tensor collision invariant: absent
massless spin-2 pole: absent
```

This independently blocks any attempt to invoke the Deser bootstrap. The
bootstrap can complete a massless spin-2 field after it exists; it cannot
create the missing carrier, collision invariant, or constraints.

---

## 4. Next gravity gate

Two honest branches remain:

1. a multi-record composite whose finite carrier spans both `E_g` and `T2g`,
   with a target-blind collision protecting the correct constrained tensor
   slow sector; or
2. a separately priced finite tensor/transport-geometry type.

Either branch must then demonstrate a massless pole, remove scalar/vector
contamination, couple universally to the same conserved stress, share the
matter/radiation cone, and produce equal light response/lensing. Until those
gates pass, gravity is not recovered from v3.

