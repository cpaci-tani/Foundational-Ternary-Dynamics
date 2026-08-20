"""P2: category taxonomy for docs/theory/07_assessment/core_ledgers/LEDGER.md.

Derived bottom-up from all 747 Quick-index row titles on 2026-08-04, replacing
the keyword-driven `classify_sector` in ledger_parser.py, which left 278 of 746
rows (37%) in `pure-math/unclassified` after the corpus grew 3.5x past the 216
rows it was tuned on.

Assignment is ID-range driven because the corpus is organised by *research
campaign*, and campaigns occupy contiguous id blocks (e.g. FTD-0434-0597 is one
common-action mechanics arc; FTD-0599-0768 is constituent-complete matter).
Keyword matching fails here precisely because rows in an arc share a programme,
not a vocabulary.  EXCEPTIONS overrides the ranges for ids that sit inside one
arc's numeric block but belong to another programme; each was identified by
reading the row title, not by pattern match.

Maintenance: when a new FTD id is registered, extend the last range (or add an
EXCEPTIONS entry).  `scripts/tests/test_ledger_index.py` fails if any ledger id
is unassigned, so the taxonomy cannot silently fall behind the ledger again.

Used by scripts/theory/build_ledger_index.py to group LEDGER_INDEX.md.
"""

from __future__ import annotations

import re

# ---------------------------------------------------------------------------
# Category register: key -> (display title, one-line scope note)
# ---------------------------------------------------------------------------
CATEGORIES = {
    "spine-master-quadratic": (
        "Algebraic spine — master quadratic",
        "The polynomial x^2-16G*^2x+16G*^3, its roots, the coefficient 16, D=3, "
        "minimality/uniqueness scans, and the x+ = 1/alpha identification."),
    "spine-gstar-cm-modular": (
        "Algebraic spine — G*, CM curves, modular",
        "G* itself, lemniscatic/CM-curve theory, Chowla-Selberg, modular and "
        "quasi-modular values, the chi_-4 character, FQCR, Sym^k period algebra."),
    "spine-periods-transcendence": (
        "Algebraic spine — periods, Watson, transcendence",
        "Watson integrals, lattice Green-function periods, the native closure N, "
        "delta-independence, E1/E2 transcendence, the period-import frontier."),
    "framework-postulates": (
        "Framework — postulates & constitution",
        "P1-P5, the Framework Commitments (FC-0/1/2/3/W), the axiom register, "
        "calibrations, adoption-pricing rules."),
    "framework-boundary-imports": (
        "Framework — boundary, imports, consumption",
        "The modulus/argument frontier, type-priority, the priced-import ledger, "
        "the consumption programme, act-counts, what the ontology cannot self-set."),
    "framework-audits": (
        "Framework — audits, red-teams, reconciliation",
        "Adversarial audits, red-team remediation, retractions, tag-honesty and "
        "corpus-wide reconciliation passes, rigidity / look-elsewhere audits."),
    "qm-foundations": (
        "Quantum foundations",
        "Born rule, measurement and the declined map M, CHSH/Bell, Spekkens, "
        "the deviation-prediction ledger, frame-relative projection."),
    "sm-mass-flavor": (
        "SM constants — mass & flavour",
        "m_e, m_p/m_e, Higgs mass, mixing angles, PMNS/CKM, Yukawa prefactors, "
        "the cluster-size-mass identification and its N(A) law."),
    "alpha-readout": (
        "Alpha readout programme (MC-T4.3)",
        "The alpha-readout contract, the ARC-A/B/C campaigns, observable "
        "selection, FC-W and the carrier-narrowing theorem, engine alpha probes."),
    "qcd-ew": (
        "QCD, colour & electroweak",
        "Confinement, colour charge and singlets, SU(3)/Z3 structure, "
        "hadrodynamics, electroweak rank, generations, no-4th-generation."),
    "gravity-cosmology": (
        "Gravity & cosmology",
        "Newton's law from the substrate, graviton/spin-2 provenance, "
        "Kerr-Newman, strong-field signatures, Lambda, dark matter."),
    "engine-infrastructure": (
        "Engine infrastructure & RG",
        "Langevin/thermostat, operator-mixing matrices, RG flow and blocking, "
        "the bridge-contract gates, Ward identities, GPU/CUDA ports and parity."),
    "engine-emergence": (
        "Engine emergence campaigns",
        "Fermion-emergence phases, genesis/evaporation and thermal phase maps, "
        "atomic and bound-state spectra, wave sectors and dispersion."),
    "lorentz-causal": (
        "Lorentz recovery & causal structure",
        "The discrete flux pole, anisotropy exponents, the common cone, CFL and "
        "causal normalisation, preferred-frame operators, anisotropic-QED RG."),
    "em-charge-emergence": (
        "Charge, Gauss & native EM emergence",
        "Native additive charge, Gauss projection and dressing, face-current "
        "sidecars, longitudinal susceptibility, dressed hazards, monopoles."),
    "common-action-mechanics": (
        "Common-action mechanics & reciprocity",
        "Forces, work and recoil for a hop; the worldline/Legendre action; "
        "charts, collisions and quotients; energy closure and Peierls barriers."),
    "constituent-matter": (
        "Constituent-complete matter",
        "Compact cores, trimers and connected blocks; rest states and Hessians; "
        "transport, gait, capture/binding, wakes, causal-horizon persistence."),
    "native-time-carrier": (
        "Native time & the carrier programme",
        "The quartic action-angle clock, G* as a temporal invariant, the C1/C2/C3 "
        "carrier conditions and every carrier candidate opened against them."),
    "meta-process": (
        "Meta — papers, tooling, project process",
        "Paper splits and referee rounds, monographs, node maps and synonymy "
        "graphs, trackers, pre-registration registries, project policy."),
}

# ---------------------------------------------------------------------------
# Primary assignment: contiguous id ranges (inclusive), first match wins.
# ---------------------------------------------------------------------------
RANGES = [
    ((1, 3),     "spine-master-quadratic"),
    ((4, 5),     "engine-emergence"),
    ((6, 8),     "spine-master-quadratic"),
    ((9, 9),     "em-charge-emergence"),
    ((10, 13),   "spine-master-quadratic"),
    ((15, 17),   "sm-mass-flavor"),
    ((18, 21),   "qcd-ew"),
    ((22, 22),   "sm-mass-flavor"),
    ((23, 24),   "qm-foundations"),
    ((25, 29),   "qcd-ew"),
    ((30, 30),   "framework-postulates"),
    ((31, 31),   "alpha-readout"),
    ((32, 32),   "spine-master-quadratic"),
    ((33, 34),   "engine-infrastructure"),
    ((35, 35),   "gravity-cosmology"),
    ((36, 41),   "framework-postulates"),
    ((42, 49),   "meta-process"),
    ((50, 50),   "spine-master-quadratic"),
    ((51, 59),   "engine-infrastructure"),
    ((60, 63),   "framework-audits"),
    ((64, 70),   "engine-infrastructure"),
    ((71, 78),   "engine-emergence"),
    ((79, 79),   "spine-periods-transcendence"),
    ((80, 84),   "spine-master-quadratic"),
    ((85, 89),   "engine-emergence"),
    ((90, 92),   "engine-infrastructure"),
    ((93, 93),   "alpha-readout"),
    ((94, 96),   "sm-mass-flavor"),
    ((97, 97),   "framework-audits"),
    ((98, 109),  "engine-infrastructure"),
    ((110, 110), "sm-mass-flavor"),
    ((111, 112), "spine-gstar-cm-modular"),
    ((113, 113), "spine-periods-transcendence"),
    ((114, 115), "em-charge-emergence"),
    ((116, 116), "spine-periods-transcendence"),
    ((117, 117), "framework-audits"),
    ((118, 118), "spine-periods-transcendence"),
    ((119, 119), "sm-mass-flavor"),
    ((120, 120), "em-charge-emergence"),
    ((121, 121), "framework-boundary-imports"),
    ((122, 124), "spine-gstar-cm-modular"),
    ((125, 126), "engine-emergence"),
    ((127, 127), "spine-gstar-cm-modular"),
    ((128, 128), "framework-postulates"),
    ((129, 129), "alpha-readout"),
    ((130, 131), "gravity-cosmology"),
    ((132, 132), "spine-gstar-cm-modular"),
    ((133, 135), "sm-mass-flavor"),
    ((136, 137), "framework-boundary-imports"),
    ((143, 143), "spine-gstar-cm-modular"),
    ((152, 152), "alpha-readout"),
    ((153, 153), "framework-boundary-imports"),
    ((154, 169), "spine-gstar-cm-modular"),
    ((170, 170), "qm-foundations"),
    ((171, 174), "meta-process"),
    ((175, 183), "spine-gstar-cm-modular"),
    ((184, 184), "gravity-cosmology"),
    ((185, 185), "alpha-readout"),
    ((186, 186), "framework-boundary-imports"),
    ((187, 188), "qm-foundations"),
    ((189, 193), "gravity-cosmology"),
    ((194, 196), "qcd-ew"),
    ((197, 198), "alpha-readout"),
    ((199, 200), "qm-foundations"),
    ((201, 201), "engine-infrastructure"),
    ((202, 203), "meta-process"),
    ((204, 206), "alpha-readout"),
    ((207, 207), "meta-process"),
    ((208, 209), "gravity-cosmology"),
    ((210, 210), "spine-master-quadratic"),
    ((211, 211), "gravity-cosmology"),
    ((212, 212), "spine-gstar-cm-modular"),
    ((213, 214), "gravity-cosmology"),
    ((219, 222), "sm-mass-flavor"),
    ((223, 224), "qcd-ew"),
    ((225, 228), "qm-foundations"),
    ((229, 229), "gravity-cosmology"),
    ((230, 232), "alpha-readout"),
    ((233, 235), "alpha-readout"),
    ((236, 236), "engine-emergence"),
    ((237, 237), "spine-gstar-cm-modular"),
    ((238, 244), "alpha-readout"),
    ((248, 249), "meta-process"),
    ((250, 252), "constituent-matter"),
    ((253, 257), "framework-postulates"),
    ((258, 258), "qm-foundations"),
    ((259, 269), "sm-mass-flavor"),
    ((270, 271), "engine-emergence"),
    ((272, 277), "engine-emergence"),
    ((278, 279), "engine-emergence"),
    ((284, 286), "alpha-readout"),
    ((298, 299), "engine-emergence"),
    ((300, 301), "framework-audits"),
    ((303, 303), "framework-audits"),
    ((307, 309), "sm-mass-flavor"),
    ((310, 310), "framework-audits"),
    ((311, 311), "framework-boundary-imports"),
    ((312, 312), "spine-master-quadratic"),
    ((313, 315), "alpha-readout"),
    ((316, 317), "engine-emergence"),
    ((318, 320), "framework-audits"),
    ((321, 321), "spine-gstar-cm-modular"),
    ((322, 330), "framework-boundary-imports"),
    ((331, 334), "gravity-cosmology"),
    ((335, 336), "framework-boundary-imports"),
    ((337, 337), "engine-emergence"),
    ((338, 338), "gravity-cosmology"),
    ((339, 344), "framework-boundary-imports"),
    ((345, 348), "framework-audits"),
    ((349, 350), "constituent-matter"),
    ((351, 352), "framework-audits"),
    ((353, 355), "framework-boundary-imports"),
    ((356, 356), "framework-audits"),
    ((357, 358), "framework-boundary-imports"),
    ((359, 359), "qm-foundations"),
    ((360, 361), "framework-audits"),
    ((362, 363), "engine-emergence"),
    ((364, 364), "gravity-cosmology"),
    ((365, 365), "framework-boundary-imports"),
    ((366, 367), "spine-gstar-cm-modular"),
    ((368, 370), "spine-periods-transcendence"),
    ((371, 371), "framework-boundary-imports"),
    ((372, 378), "spine-periods-transcendence"),
    ((379, 380), "engine-emergence"),
    ((381, 382), "spine-gstar-cm-modular"),
    ((383, 388), "framework-boundary-imports"),
    ((390, 390), "sm-mass-flavor"),
    ((392, 394), "constituent-matter"),
    ((395, 396), "framework-boundary-imports"),
    ((397, 397), "sm-mass-flavor"),
    ((398, 399), "constituent-matter"),
    ((400, 400), "qcd-ew"),
    ((401, 425), "lorentz-causal"),
    ((426, 433), "em-charge-emergence"),
    ((434, 483), "common-action-mechanics"),
    ((484, 514), "common-action-mechanics"),
    ((515, 520), "framework-boundary-imports"),
    ((525, 562), "common-action-mechanics"),
    ((563, 564), "em-charge-emergence"),
    ((565, 566), "framework-boundary-imports"),
    ((567, 582), "common-action-mechanics"),
    ((583, 597), "common-action-mechanics"),
    ((598, 598), "framework-boundary-imports"),
    ((599, 768), "constituent-matter"),
    ((769, 769), "constituent-matter"),
    ((770, 784), "native-time-carrier"),
    ((785, 785), "framework-audits"),
    ((786, 790), "native-time-carrier"),
    ((791, 794), "alpha-readout"),
    ((795, 796), "qm-foundations"),
    ((797, 798), "native-time-carrier"),
    ((799, 799), "constituent-matter"),
    ((800, 801), "native-time-carrier"),
    ((802, 802), "framework-audits"),
    ((803, 803), "spine-gstar-cm-modular"),
    # 2026-08-07/08 temporal-interior and cone arc.
    ((804, 806), "native-time-carrier"),        # MVC, native n=4 screen, programme charter
    ((807, 807), "qm-foundations"),             # Born-weighting preregistration arc
    ((808, 808), "native-time-carrier"),        # the geometric bit as a register
    ((809, 809), "qm-foundations"),             # engine regime map (N) + latency gate
    ((810, 813), "lorentz-causal"),             # cone speed, causal cell, massive & composite cones
    ((814, 814), "native-time-carrier"),        # one-energy carrier, universal kink dilation
    ((815, 816), "lorentz-causal"),             # the two owed proofs; the inter-sector cone
    ((817, 817), "native-time-carrier"),        # what G* is the constant of
    ((818, 818), "meta-process"),               # the semantic-ontology manuscript
    ((819, 819), "lorentz-causal"),            # carrier sublattice: body centres suffice
    ((820, 821), "native-time-carrier"),       # period factorisation; threshold forcing
    ((822, 824), "native-time-carrier"),       # register barrier proven; economy
                                               # refuted; MVC reduction corrections
    ((825, 825), "qm-foundations"),            # contextual-actualization successor
    ((826, 826), "native-time-carrier"),        # native modal carrier + CM realization boundary
    ((827, 827), "native-time-carrier"),        # conditional quartic-clock / CM gearbox
    ((828, 828), "native-time-carrier"),        # minimum native-clock requirements + orientation
    ((829, 999), "native-time-carrier"),        # tangent ladder through cumulative growth resource/backpressure law
]

# ---------------------------------------------------------------------------
# Exceptions: ids inside one arc's numeric block that belong to another
# programme.  Derived by reading the row titles, not by keyword match.
# ---------------------------------------------------------------------------
EXCEPTIONS = {
    "FTD-0136-PhaseB-final": "framework-boundary-imports",
    # inside the constituent-matter block (0599-0768) but not about matter objects
    "FTD-0669": "framework-boundary-imports",   # least ontology of matter
    "FTD-0740": "framework-boundary-imports",   # evidence standard for the programme
    "FTD-0741": "framework-boundary-imports",   # minimum retained information
    "FTD-0742": "framework-boundary-imports",   # evidence baseline
    "FTD-0743": "framework-boundary-imports",   # metastable-matter predicate
    "FTD-0744": "framework-boundary-imports",   # how evidence decides among ontologies
    "FTD-0641": "em-charge-emergence",          # face/edge field propagating modes
    "FTD-0747": "engine-infrastructure",        # first CUDA port
    "FTD-0748": "engine-infrastructure",        # CUDA current gate
    "FTD-0749": "engine-infrastructure",        # CUDA determinism
    "FTD-0750": "engine-infrastructure",        # CUDA replay parity
    "FTD-0751": "engine-infrastructure",        # CPU/CUDA divergence stage
    "FTD-0752": "engine-infrastructure",        # explicit-rounding CUDA backend
    "FTD-0759": "engine-infrastructure",        # device-resident pipeline parity
    # inside the common-action block but framework-level
    "FTD-0508": "framework-boundary-imports",   # four imports share one schema
    "FTD-0509": "framework-boundary-imports",   # adopted-bit unit reconciliation
    "FTD-0510": "framework-boundary-imports",   # D=3 forcedness
    "FTD-0517": "qm-foundations",               # what a measuring device must be
    "FTD-0568": "framework-audits",             # is the coat layer FTD-native or re-derived
    "FTD-0203": "sm-mass-flavor",               # FTD-0110 nonlinear-bridge scoping memo
    # inside the engine block but spine maths
    "FTD-0079": "spine-periods-transcendence",
    # alpha-adjacent rows inside other blocks
    "FTD-0011": "alpha-readout",                # g_c^2 scales alpha_r
    "FTD-0013": "spine-master-quadratic",       # stays with the polynomial
    # outside every range (first ids past FTD-0999) but framework-level content
    "FTD-1000": "framework-postulates",         # CLK-1 folded into FC-2, constitution amendment
    "FTD-1001": "native-time-carrier",          # batch certificate relock (0993/0995/0998 chains)
    "FTD-1002": "framework-boundary-imports",   # four-walls/fold adjudication: 4 walls, 3 roofs
    "FTD-1003": "lorentz-causal",               # no point-group protection; C2 escape closed
    "FTD-1004": "native-time-carrier",          # unit-strut tensegrity decision v1, Outcome B
    "FTD-1005": "native-time-carrier",          # axial surd no-go + Pythagorean currency
    "FTD-1006": "native-time-carrier",          # strutted axial class fully closed (v2+v2.1)
    "FTD-1007": "native-time-carrier",          # ternary mask extension owner-adopted
    "FTD-1008": "native-time-carrier",          # ternary-sector first menu, Outcome B
    "FTD-1009": "lorentz-causal",              # two-body bion dilation chain, execution record
    "FTD-1010": "lorentz-causal",              # v2.1 deviation model; surrogate line closed 19/20
    "FTD-1011": "lorentz-causal",              # hiding order (ka)^2 via isotropic curvature; 4/4
    "FTD-1012": "lorentz-causal",              # bath-frame break chain; matter braked, radiation blind
    "FTD-1013": "gravity-cosmology",            # universal free-fall Q0; test-body action in external L
    "FTD-1014": "gravity-cosmology",            # UFF engine alignment; live F/M vs Q0 g_ext, CLOSED-NEGATIVE
    "FTD-1015": "gravity-cosmology",            # GW area-holonomy Q0; kinematic residual not exactly two TT
    "FTD-1016": "gravity-cosmology",            # geometric free-fall integrator; default-off F=M C^2 L grad L
    "FTD-1017": "gravity-cosmology",            # sourced geometric free-fall; Poisson L then FTD-1016 operator
    "FTD-1018": "gravity-cosmology",            # CUDA port of geometric_gravity; CPU/GPU F and v parity
    "FTD-1019": "gravity-cosmology",            # one frozen Poisson well: FC-2 rest clocks + FTD-1016 falling
    "FTD-1020": "gravity-cosmology",            # frozen vacuum well vs live flux characteristics; class 0
    "FTD-1021": "gravity-cosmology",            # live Poisson occupancy; operator reads live L; freeze not test-body at 1/125
    "FTD-1022": "gravity-cosmology",            # 3^3 slow-envelope live Newton; freeze still not test-body at 27/125
}


# ---------------------------------------------------------------------------
# Sub-programmes for the two arcs large enough to need a second level.
# ---------------------------------------------------------------------------
SUBRANGES = {
    "common-action-mechanics": [
        ((434, 483), "forces, work & reciprocity"),
        ((484, 514), "action, worldline & Legendre structure"),
        ((525, 542), "contact, collision & charts"),
        ((543, 582), "energy closure & Peierls barriers"),
        ((583, 597), "protected sector, seed bootstrap & removal counts"),
    ],
    "constituent-matter": [
        ((250, 252), "cluster inertia & the moving clock"),
        ((349, 399), "collective-coordinate reduction & particlehood"),
        ((599, 620), "compact cores, trimers & walkers"),
        ((621, 656), "connected blocks & resolution scaling"),
        ((657, 699), "internal modes & matter-field transfer"),
        ((700, 720), "transport, gait & depinning"),
        ((721, 744), "capture, binding & persistence"),
        ((745, 769), "causal-horizon persistence, wake & momentum closure"),
        ((799, 799), "protonucleus & body growth"),
    ],
}


def subcategory_for(ftd_id: str, category: str) -> str | None:
    """Return the sub-programme label, or None if the category has no sub-level."""
    subs = SUBRANGES.get(category)
    if not subs:
        return None
    m = re.match(r"FTD-(\d{4})", ftd_id)
    if not m:
        return None
    n = int(m.group(1))
    for (lo, hi), label in subs:
        if lo <= n <= hi:
            return label
    return None


def category_for(ftd_id: str) -> str | None:
    """Return the category key for an FTD id, or None if unassigned."""
    if ftd_id in EXCEPTIONS:
        return EXCEPTIONS[ftd_id]
    m = re.match(r"FTD-(\d{4})", ftd_id)
    if not m:
        return None
    n = int(m.group(1))
    for (lo, hi), key in RANGES:
        if lo <= n <= hi:
            return key
    return None
