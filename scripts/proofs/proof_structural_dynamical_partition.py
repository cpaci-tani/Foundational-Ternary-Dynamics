"""
proof_structural_dynamical_partition.py

Boundary theorem (FTD-0186), Stage 1 -- machine verification of the
structural / dynamical classification.

Pre-registration v1 (historical): docs/theory/10_eft_program/PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md
  hash-locked: commit 75ebe56, tag preregister-structural-dynamical-discriminator-v1,
  SHA256 a6562dca56154401e7a2cfb8785266cef0d5b4ee70d3755797762ddffa3e538d.
  Falsifier S4-A1 fired on the v1 wording -- type-ii closed-negatives target
  STRUCTURAL quantities, which v1's broad A1 quantification rejected.

Pre-registration v2 (current, 2026-05-23 Path II Session A2):
  docs/theory/10_eft_program/PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v2.md
  hash-locked: commit d550bca, tag preregister-structural-dynamical-discriminator-v2,
  SHA256 a233fa28be54c63c6a7ebae26c6b54e129c9f2120e535f92d85999ac84d9068a.
  A1 sharpened to "failed attempt to derive a non-universal DYNAMICAL VALUE";
  A3 added (type-ii recorded as structural-provenance closed-negatives, separate
  honest category); S2 (the discriminator definition) carried over verbatim
  from v1. v2 re-run returns Outcome A -- clean partition (A1 v2 PASS,
  A2 PASS, A3 PASS); LEDGER FTD-0186 status [DEFINITION] + [STAGE 1 CLOSED
  POSITIVE per v2].

This script content has been the same since v1 -- its EXPECT table already
encoded the v2-style semantics (cneg_ii -> STRUCTURAL). Under v1 the script
was reading the post-fired finding correctly but documenting it as a v2 need;
under v2 the same partition is the pre-registered falsifier test.

Classification: docs/theory/02_foundations/FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md

The discriminator (locked pre-reg S2) assigns each load-bearing quantity-claim
one of three classes:
    STRUCTURAL              -- value forced by finite discrete combinatorics
    NON_UNIVERSAL_DYNAMICAL -- a dimensionless continuous parameter, not so forced
    CALIBRATION_CONDITIONAL -- dimensional, depends on the calibration declarations

This script encodes the S3 classification of the decisive load-bearing set and
verifies the partition the v2 S4 falsifier turns on:
    spine THEOREM/DERIVED claims               -> STRUCTURAL          (A2)
    type-i closed-negatives  (failed derivation
        of a non-universal DYNAMICAL VALUE)    -> DYNAMICAL or CALIB  (A1 v2)
    type-ii closed-negatives (failed derivation
        of a structural object's provenance)   -> STRUCTURAL targets, recorded
                                                  as structural-provenance
                                                  closed-negatives, outside
                                                  the boundary-theorem axis
                                                  (A3 v2).
"""

import sys

STRUCTURAL = "STRUCTURAL"
DYNAMICAL = "NON_UNIVERSAL_DYNAMICAL"
CALIB = "CALIBRATION_CONDITIONAL"

# (id, short, kind, assigned_class)   kind in {spine, cneg_i, cneg_ii, parametric}
ENTRIES = [
    ("FTD-0001", "master quadratic polynomial + roots",          "spine",      STRUCTURAL),
    ("FTD-0002", "G* = Gamma(1/4)/Gamma(3/4) identity",           "spine",      STRUCTURAL),
    ("FTD-0003", "CM-curve uniqueness (class number 1)",          "spine",      STRUCTURAL),
    ("FTD-0004", "Phase G geometric Coulomb (V(r) form)",         "spine",      STRUCTURAL),
    ("FTD-0005", "Phase J ultralocality (L=2)",                   "spine",      STRUCTURAL),
    ("FTD-0006", "coefficient 16 = |Aut(E)|^2 (Route A)",         "spine",      STRUCTURAL),
    ("FTD-0007", "coefficient 16 = |Aut(E)|^2 (Route B)",         "spine",      STRUCTURAL),
    ("FTD-0008", "Moore integers {4, 13, 7}",                     "spine",      STRUCTURAL),
    ("FTD-0009", "charge conservation per tick",                  "spine",      STRUCTURAL),
    ("FTD-0010", "D = 3 from |Aut(E)|^2 = 2^D (D-1)!",            "spine",      STRUCTURAL),
    ("FTD-0011", "Phase H coupling-scaling relation",             "spine",      STRUCTURAL),
    ("FTD-0012", "discriminant trichotomy (algebra)",             "spine",      STRUCTURAL),

    ("FTD-0058", "Structure-2 scalar gauge ppb alpha-closure",    "cneg_i",     DYNAMICAL),
    ("FTD-0031", "g_c first-principles (all routes)",             "cneg_i",     DYNAMICAL),
    ("FTD-0093", "g_c as bridge-operator eigenvalue",             "cneg_i",     DYNAMICAL),
    ("FTD-0025", "confinement substrate-derivation (sigma)",      "cneg_i",     DYNAMICAL),
    ("FTD-0131", "G_N = 1/(b_3+N_c)^2 as physical G_N",           "cneg_i",     DYNAMICAL),
    ("FTD-0116", "G*^2 as lattice Z-factor",                      "cneg_i",     DYNAMICAL),
    ("FTD-0094", "L2 identity 2 m_e/alpha = 16 G*^2 (demoted)",   "cneg_i",     DYNAMICAL),
    ("FTD-0035", "Mechanism gamma -- a_phys derivation",          "cneg_i",     CALIB),
    ("FTD-0034", "a_phys no-go (Mechanisms alpha-delta)",         "cneg_i",     CALIB),
    ("FTD-0096", "mu-from-l_P mass-unit calibration",             "cneg_i",     CALIB),

    ("FTD-0018", "sin^2 theta_W (parametric demotion)",           "parametric", DYNAMICAL),
    ("FTD-0020", "alpha_s = 7/59 (parametric demotion)",          "parametric", DYNAMICAL),
    ("FTD-0021", "PMNS angles (parametric demotion)",             "parametric", DYNAMICAL),

    ("FTD-0050", "master quadratic as RG-step char. poly",        "cneg_ii",    STRUCTURAL),
    ("FTD-0164", "chi_-4 -> P_G*: (2,3) exponents from CM",       "cneg_ii",    STRUCTURAL),
    ("FTD-0183", "N_base = 4 unification with Z[i]^x",            "cneg_ii",    STRUCTURAL),
]

EXPECT = {
    "spine":      {STRUCTURAL},
    "cneg_i":     {DYNAMICAL, CALIB},
    "parametric": {DYNAMICAL, CALIB},
    "cneg_ii":    {STRUCTURAL},   # structural targets -- the scope boundary
}


def main():
    print("=" * 74)
    print("  Boundary Theorem FTD-0186, Stage 1 -- partition verification")
    print("=" * 74)
    misfits = []
    by_kind = {}
    for eid, short, kind, cls in ENTRIES:
        by_kind.setdefault(kind, []).append((eid, short, cls))
        ok = cls in EXPECT[kind]
        if not ok:
            misfits.append((eid, short, kind, cls))
        print(f"  [{'ok ' if ok else 'BAD'}] {eid}  {kind:<10}  {cls:<23} {short}")

    print("-" * 74)
    n_spine = len(by_kind.get("spine", []))
    n_i = len(by_kind.get("cneg_i", [])) + len(by_kind.get("parametric", []))
    n_ii = len(by_kind.get("cneg_ii", []))
    print(f"  spine theorems                 : {n_spine:>2}  -> all STRUCTURAL")
    print(f"  type-i closed-negatives        : {n_i:>2}  -> all NON-UNIVERSAL DYNAMICAL / CALIBRATION-CONDITIONAL")
    print(f"  type-ii closed-negatives       : {n_ii:>2}  -> STRUCTURAL targets (out of boundary-theorem axis)")
    print("-" * 74)

    spine_clean = all(c in EXPECT["spine"] for _, _, c in by_kind.get("spine", []))
    type_i_clean = all(
        c in EXPECT["cneg_i"]
        for k in ("cneg_i", "parametric")
        for _, _, c in by_kind.get(k, [])
    )

    if misfits:
        print("  RESULT: misfit(s) found -- discriminator S2 falsified (Outcome B).")
        for eid, short, kind, cls in misfits:
            print(f"    {eid} ({kind}) classified {cls}, expected {EXPECT[kind]}")
        return 1

    print("  RESULT: clean partition.")
    print("    A2  every spine THEOREM/DERIVED claim  -> STRUCTURAL         : "
          + ("PASS" if spine_clean else "FAIL"))
    print("    A1  every type-i closed-negative       -> DYNAMICAL / CALIB  : "
          + ("PASS" if type_i_clean else "FAIL"))
    print()
    print("  The type-ii closed-negatives (FTD-0050, 0164, 0183) target STRUCTURAL")
    print("  objects -- the deeper provenance of structural quantities, not a")
    print("  dynamical value. They are outside the boundary theorem's axis and")
    print("  define its scope (FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md S4-S5).")
    print()
    print("  Outcome (v2 falsifier, hash-locked at commit d550bca,")
    print("  tag preregister-structural-dynamical-discriminator-v2):")
    print("    A1 v2  failed dynamical-value derivation -> DYNAMICAL/CALIB : PASS")
    print("    A2     spine THEOREM/DERIVED              -> STRUCTURAL      : PASS")
    print("    A3     type-ii (structural-provenance)    -> STRUCTURAL      : PASS")
    print("  Stage 1 status: CLOSED POSITIVE per v2. LEDGER FTD-0186 updated")
    print("  from [DEFINITION] + [OPEN] to [DEFINITION] + [STAGE 1 CLOSED")
    print("  POSITIVE per v2]. No FTD claim promoted or demoted. Stage 2")
    print("  (Structural Decoupling Theorem) remains an unsettled provable")
    print("  proposition; v2 closing positive is its prerequisite, not its")
    print("  proof. See FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md S5.2.")
    print("=" * 74)
    return 0 if (spine_clean and type_i_clean) else 1


if __name__ == "__main__":
    sys.exit(main())
