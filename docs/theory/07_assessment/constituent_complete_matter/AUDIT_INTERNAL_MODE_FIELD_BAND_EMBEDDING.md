# Audit — internal-mode field-band embedding

**Ledger ID:** FTD-0663  
**Status:** theorem for kinematic embedding; measured finite-volume transfer;
resonance remains an inference/open discriminator

The matched field dispersion ranges continuously over `[0,pi]`. The first
internal doublet phase is `1.0911648733663635` and is matched already on the
axis branch at `k=2.2339983325737203`. Frequency-gap protection is therefore
excluded exactly.

FTD-0662 independently excludes complete decoupling for the prepared finite-
volume excitation. It does not exclude a symmetry-protected embedded mode or
prove an infinite-volume resonance. The theorem is deliberately scoped to
kinematic embedding.

**Addendum 2026-08-14 (FTD-1003).** The symmetry escape named above is now
closed in its spatial point-group sense:
`THEOREM_NO_POINT_GROUP_PROTECTION_v1.md` proves that no point-group
selection rule, under `O_h` or any subgroup at any site symmetry, can force
the on-shell coupling to vanish identically on the isofrequency surface, for
any frequency strictly inside `(0, pi)` — because generic points of that
surface have trivial stabilizer, and sections of the rank-2 transverse
channel bundle over a free orbit carry every irrep. Of the four mechanisms
this audit left open, that closes "symmetry"; "destructive interference" and
"bound state in the continuum" are recharacterized there as accidental
on-shell vanishing at **codimension exactly one** (tuning, not protection);
and **"topological invariant" remains open**. That document also carries a
correction of record affecting this audit's neighbourhood: the operative
band-clearance edge is the band top `pi`, not the `<100>` axis top
`2 asin(1/sqrt 3) = 1.2310`. No tag in this audit moves.

Independent certificate:
`scripts/proofs/proof_internal_mode_field_band_embedding.py`, SHA-256
`08A3E34CDC1CF8420EDA921C2D2CF142B797EABF5BFF5B0C329E20BC04CD8E92`.
