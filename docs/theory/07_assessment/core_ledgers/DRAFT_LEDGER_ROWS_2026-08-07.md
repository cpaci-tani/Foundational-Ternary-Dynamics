# DRAFT LEDGER ROWS — 2026-08-07 session (BOOKED 2026-08-08)

**Status:** `[BOOKED — FTD-0804 … FTD-0808]`. Retained for provenance
rather than deleted: this file records what each row said at drafting
time, before the wording that entered the LEDGER. The id-collision
condition it waited on is resolved — `edge-clock-dissemination` tops at
FTD-0803, the same maximum as `main`, so the merge decision does not move
these ids.

| draft row (in order below) | booked as |
|---|---|
| minimum viable clock carrier | `FTD-0804` |
| native single-scale `n = 4` screen / hexagon wheel | `FTD-0805` |
| temporal-interior programme charter + criteria specs | `FTD-0806` |
| Born-density upcrossing / saturation preregistration arc | `FTD-0807` |
| T2 first screen: the geometric bit | `FTD-0808` |

The 2026-08-08 continuation booked separately as `FTD-0809` … `FTD-0818`;
see [`INDEX.md`](../../10_eft_program/temporal_interior_programme/INDEX.md).
Booking moved no tag beyond what each row states.

---

**| FTD-NNNN |** *Does a minimum viable clock carrier exist, and at what
price?* **|** [ANALYSIS — EXACT CONDITIONAL RESULTS WITHIN A DECLARED
TWO-SCALE EXTENSION] + [SELECTED MODEL — CONDITIONAL THEOREM: the
collinear two-scale 4-chain is prestress-stable at zero tension with
n = 4 on every flex] + [MEASURED — G\* recovered to 2e-6 relative, no
fitted scale] **|** NEW 2026-08-07 —
`ANALYSIS_MINIMUM_VIABLE_CLOCK_CARRIER_v1.md` +
`mvc_fourchain_clock.py`. A⁺B⁻C⁺D⁻ collinear, three unit bonds (k₁) plus
ONE adopted species-2 bond at range 3r₀ (k₂); stress (1,1,1,−1); blocking
form eigenvalues {2, 10/3, 0, 0}, kernel = trivials;
λ_eff = 8k₁k₂/(k₁+3k₂), m_eff = 4;
T·A = √π·G\*·√(k₁+3k₂)/(2√(k₁k₂)); 12-DOF ringdown recovers
G\* = 2.958675 to 2e-6 (A→0). Minimality exact: polarity alternation
forces 4 bodies + odd (= 3r₀) closure. Price: 1 interaction species + C2
scale parity (ε·A_max² > 1.0556 field / 2.786 wave ⇒ ε > 4.22/11.15 at
A_max = 0.5). C5 unevaluated; C11 fails BY CONSTRUCTION (that is the
definition of minimum-viable); FTD-0794 ("G\* enters by choice") and
FTD-0784 (surd external) unaffected. **|**

---

**| FTD-NNNN |** *Does the registered single-scale law natively host an
n = 4 mechanism?* **|** [CONSTRUCTIVE — FIRST NATIVE ZERO-TENSION
SELF-STRESSED EQUILIBRIUM EXHIBITED (s = 2 hexagon wheel, N = 19)] +
[CLOSED NEGATIVE — AS A CLOCK: compressed-chain buckling opens a
zero-energy mechanism cone] + [CRITERION — NEW, THREE CONDITIONS] +
[OPEN — the integer unit-strut tensegrity question] **|** NEW 2026-08-07
— `ANALYSIS_MINIMUM_VIABLE_CLOCK_CARRIER_v1.md` §3.1 +
`native_chain_network_{search,verify}.py`, `native_wheel_clock_sim.py`,
`native_unitedge_stress_screen.py`. Unit-edge bipartite screen (17
classes, N ≤ 7 + Q₃): 0 stressed embeddings; SC blocks L = 2,3,4
self-stress dim 0 EXACT (explains FTD-0800's clamped-only quartics).
Contraction analysis: interior-K₄ capped at 30° joint angle vs the law's
28.96° clearance floor (support q < 3/2) — exhaustive integer search to
diameter 60 tops at 26.14°. The hexagon wheel passes G1–G3 (24 unit
bonds; nearest opposite non-bonded pair at q = 3 = 2× support edge;
coker(R) = 1, chain-uniform rim +t / spokes −t) — then fails G5: the
blocking form is sign-indefinite on the 28-dim flex space
(E₄ ∝ (Σκ_rim − Σκ_sp)²; the escape δ = u found empirically and
structurally). **Criterion of record: native n = 4 requires (1) a
self-stress, (2) blocking-form definiteness on the FULL flex space,
(3) every compressed member a single bond** (free-hinge chains buckle;
all-tension stress impossible by the extreme-vertex balance). First
mixed family (axial lens) killed exactly: s² − 4k² = −1 impossible
mod 4. FTD-0783's bracket updated: one identified mechanism class,
refuted; C3 remains unrealized natively. **|**

---

**| FTD-NNNN |** *Temporal-interior programme registered (charter +
criteria specs).* **|** [SCOPE / PROGRAMME CHARTER — DRAFT FOR OWNER
RATIFICATION] + [SYNTHESIS — three criteria specs, no candidate scored,
nothing promoted] **|** NEW 2026-08-07 —
`docs/theory/10_eft_program/temporal_interior_programme/`:
`archive/superseded/ARCH_SCOPE_TEMPORAL_INTERIOR_PROGRAM_V1.md` (historical unratified v1; two set types — succession P2,
potentiality P3; three purchased structures — clock/register/tracker;
fronts T1–T5; the binding ceiling sentence: scaffold-to-interior is an
IDENTIFICATION, `[CONJECTURE — INTERPRETIVE]` at best),
`SPEC_REGISTER_CRITERIA_v1.md` (R1–R9; FTD-0777's reference-lift as
checklist; coupled-escape gate mandatory),
`SPEC_DISPOSITION_TRACKER_LADDER_v1.md` (rungs a/b/c; b hard-gated by
T2; experiential vocabulary QUARANTINED until a+b close under lock),
`SPEC_OBSERVERS_COMPLETION_MAP_v1.md` (six-row native-seed vs
observer-completion table at tags of record; thesis at [SYNTHESIS]:
Hilbert space as the observer's completion of substrate potentiality;
collapse native / unitarity imported). **|**

---

**| FTD-NNNN |** *Which weighting do threshold upcrossings follow —
amplitude, occupation (Born), or energy?* **|** [MEASURED —
MECHANISM-LEVEL, IMPOSED ENSEMBLE, QUICK-CHECK PLATFORM; preregistered,
outcomes verbatim] + [CORRECTION — FTD-0798's density drift was
normalisation, not physics] + [OPEN — substrate level unchanged;
FTD-0200 engine closure pending] **|** NEW 2026-08-07 —
`PREREG_BORN_DENSITY_UPCROSSING_v1.md` (v1 → OUTCOME D EXECUTION
INVALID, instrument defect caught by its own gates; v1.1 re-locked →
**OUTCOME E**, Born-fraction 0.024 [0.014, 0.036] → 0.099 [0.090, 0.109]
across Ω·τ 0.45–0.91 → 1.81–3.56) +
`PREREG_BORN_DENSITY_SATURATION_v2.md` (five arms, Ω·τ 0.64 → 78, fresh
seeds → **OUTCOME B — APPROACHES**: BF 0.049 → 0.102 → 0.274 → 0.469 →
0.836 [0.813, 0.861], strictly monotone, crossover Ω·τ ≈ 17, descriptive
B_∞ = 0.86). **Within the FTD-0187/0356 mechanism class, Born/occupation
weighting is the fast-mode/slow-noise asymptote of threshold-crossing
statistics — the 1/√(2Ω) normalisation is performed by the threshold
itself in the Ω·τ ≫ 1 regime.** Retrodicts FTD-0200's 6-neighbour Rice
failure (wrong regime). Stage A sharpening: occupation conserved to
7.1e-14 with the discrete-exact two-step invariant (FTD-0798's reported
1.05e-2 was continuum-normalisation convention). Riders: arm-1
phase-draw systematic ~0.02 (v3 averaging target); asymptote exactly-1
vs ≈ 0.86 registered residual; NEVER cite as substrate Born. Engine
campaign design target: Ω·τ ≳ 30. **|**

---

**| FTD-NNNN |** *T2 first screen: the geometric bit.* **|**
[STATICS-LEVEL SCREEN — SELECTED SCAFFOLD; R2 PASS EXACT] **|** NEW
2026-08-07 — `ANALYSIS_GEOMETRIC_BIT_REGISTER_SCREEN_v1.md` +
`register_geometric_bit_screen.py`. One ⁺ body on three pinned ⁻
anchors (side s < √3): two zero-tension first-order-rigid mirror states
(Hessian = 96ε·Σûûᵀ ≻ 0), separation 2√(1 − s²/3), **true barrier = ε
exactly** (one bond depth; watershed flood-fill over the full 3D
configuration space; hinge path 1.000ε; through-centre ~30ε),
geometry-independent over s ∈ [0.9, 1.3]. Structural note: register and
clock occupy opposite corners of the same rigidity criterion; ε prices
binding, clock rate (C2), and retention (Arrhenius ε/T) at once. R1/R7
open pending the self-holding composite (registered next constructions:
self-holding bit → clock–register composite prereg → tracker rung b).
**|**
