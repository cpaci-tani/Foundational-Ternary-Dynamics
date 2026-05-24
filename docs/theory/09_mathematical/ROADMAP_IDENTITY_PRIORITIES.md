# ROADMAP -- Identity-verification priorities (synonymy graph C4)

**Tag:** [INFRASTRUCTURE / METHODOLOGY] -- a research-navigation tool, not new mathematics.
**LEDGER row:** FTD-0202.
**Date:** 2026-05-23 (Path III Session A3 of `.claude/plans/let-s-proceed-on-the-eager-rocket.md`).
**Sources:**
- Graph data: [`scripts/verification/results/synonymy_graph.json`](../../../scripts/verification/results/synonymy_graph.json)
- Graph extractor: [`scripts/verification/extract_synonymy_graph.py`](../../../scripts/verification/extract_synonymy_graph.py)
- Source-of-truth identities: [`scripts/verification/verify_gstar_paper.py`](../../../scripts/verification/verify_gstar_paper.py) (100 distinct `check()` call sites; some loops generate multiple runtime checks)
- G\* paper (uncommitted, pending arXiv upload): [`docs/papers/PAPER_GSTAR_INTRODUCTION.tex`](../../papers/PAPER_GSTAR_INTRODUCTION.tex)

> **Scope discipline.** This roadmap is **descriptive, not generative.** It identifies which mathematical objects are well-connected in the verified-identity corpus and which are isolated; it does **not** produce new theorems. Its value is methodological: prioritising where future verification effort lands. **No FTD claim is promoted or demoted by this document.**

---

## §1 -- What the graph is and is not

The **synonymy graph** is the bipartite graph extracted from the 100 `check()` call sites in `verify_gstar_paper.py`. One side carries identity-nodes (one per `check`, labelled `A1`, `A2`, ..., `Q6`); the other side carries object-nodes (named mathematical constants, functions, and derived expressions that appear in any check). An edge `(c, o)` records that object `o` participates in identity `c`. A separate **pairwise** representation collapses each identity into all unordered pairs of its participants, with the identity ID as witness.

The graph is **not**:
- A proof system. An "edge" only records that the AST mentions the object; it does not assert that the identity reduces to a known relation between just those objects.
- An exhaustive catalogue. It reflects only the 100 currently-verified identities in this one script. Other verification scripts under `scripts/verification/` add their own (unindexed-here) edges.
- A novelty detector. High valence reflects how often a constant appears in the existing corpus; it does not by itself indicate either deep significance or simple convenience.

The graph **is**:
- A read of the currently-encoded network of synonymies between mathematical objects.
- A surface for spotting isolated nodes (single-identity entries) where a new identity would add the most cross-connection.
- A starting point for "where should we verify next" decisions when adding to `verify_gstar_paper.py` or to a successor script.

---

## §2 -- Current graph state (extraction 2026-05-23)

| Metric | Value |
|---|---|
| `n_checks` (distinct `check()` call sites) | 100 |
| `n_objects` (named participants after EXCLUDE filter) | 72 |
| `n_edges_bipartite` (identity ↔ object) | 216 |
| `n_edges_pairwise` (object ↔ object, per identity) | 181 |
| Mean valence per object | 3.00 |
| Median valence per object | 1 (long isolated tail) |

### §2.1 -- Top-valence objects (the connective tissue)

| Rank | Object | Valence | Role |
|---|---|---|---|
| 1 | `G_G` (Gauss constant 1/AGM(1, √2)) | 29 | the AGM hub -- modular, theta, Watson, period |
| 2 | `pi` | 25 | universal background |
| 3 | `sqrt` (function) | 18 | universal background |
| 4 | `Gstar` (G\*) | 13 | the spine constant; bridges G_G and the master quadratic |
| 5 | `G_rho` (equianharmonic Gauss analog) | 9 | cubic-AGM hub; the τ = ρ counterpart of G_G |
| 6 | `gamma` (Γ function) | 9 | the underlying Γ machinery |
| 7 | `varpi` (lemniscate constant ϖ) | 8 | π · G_G; classical lemniscate calculus |
| 8 | `eta_i` (Dedekind η(i)) | 7 | the τ = i modular hub |
| 9 | `K_half` (complete elliptic K at k = 1/√2) | 5 | period-of-E elliptic-integral hub |
| 10 | `d_n` (asymptotic R_n coefficients) | 5 | the §O asymptotic-expansion family |

Most identities sit in the "G_G + π + sqrt + Gstar" core. `G_rho` and `eta_i` are second-tier hubs that join the cubic-AGM (τ = ρ) and modular (τ = i) sub-networks to the core.

### §2.2 -- Isolated nodes (the long tail)

The graph has 7 objects at valence = 1 that are **single-identity entries** -- they appear in one verified identity each and form leaves of the graph. These are the highest-leverage targets for "enrich this node" work, because adding one more identity per leaf doubles its valence and rewires it into a sub-network.

Notable isolated-node candidates (valence = 1):
- `R2`, `R3`, `R4`, `R5`, `R6` -- the `R_n = Γ(1/n)/Γ((n-1)/n)` family entries (`R4` is the only one with internal cross-link via `R4 = Gstar`). Higher `R_n` values for n ∈ {5, 7, 8, 12} are the natural enrichment target (cf. C3 catalogue work in `.claude/plans/let-s-proceed-on-the-eager-rocket.md`).
- `Delta_i`, `j_i` (j(i) = 1728), `Delta_rho`, `eta_rho` -- the τ = i and τ = ρ modular invariants currently appear in only one identity each. The j-invariant in particular is a natural bridge to the master quadratic via the CM-uniqueness argument (already partially connected through `L11: j(i) = 1728`).
- `E4_i`, `E6_i`, `E8_i`, `E10_i`, `E12_i`, `E14_i`, `E16_i`, `E20_i`, `E24_i` -- the τ = i Eisenstein series. Each appears in exactly one identity in the §L block. Cross-linking them via the von Staudt-Clausen-type recurrence or via the E_k → E_2 quasimodular tower would densify this neighbourhood.
- `W4_raw`, `W4_watson`, `W5_raw` -- higher Watson constants. `W^(3)` has valence 3 (well-connected via G_G); `W^(4)` and `W^(5)` are isolated.
- `omega_E_int`, `omega_rho_direct`, `omega_rho_reduced` -- period-computation variants currently bound to just one check each.

A full leaf list is in `scripts/verification/results/synonymy_graph.json` (search `"valence": 1` and `"valence": 2` in the `objects` array).

---

## §3 -- Prioritised next-target identity bundles

Bundles are ranked by **expected centrality gain** (how many new edges into the high-valence core), with a difficulty band and a one-line value statement. Difficulty: **D** = desk/AGM, **W** = week (literature-sweep + implementation), **M** = month (genuine math beyond existing infrastructure), **FO** = frontier-open (transcendence-theory frontier).

### Bundle 1 -- The Catalan ↔ {G_G, π, x_+, x_-} bridge (4 edges, **FO**-blocked)

**Plan source:** plan §A3 candidate 1.
**Current state:** Catalan G ≈ 0.91596 does not currently appear in the synonymy graph (zero edges to anything). It enters the FTD corpus only via the §19 algebraic-independence conjecture in the G\* paper (Conjecture 19.2; PSLQ to 80 digits, no integer relation at coefficient bound 10^12).
**What would close it:** a proof or strong evidence relating Catalan G to G_G, π, or the master-quadratic roots. The plan §D1 lists this as the optional Catalan PREREG -- documenting the boundary, not closing it.
**Difficulty:** FO (Baker-type lower bounds or Deligne-period machinery; beyond current reach).
**Centrality gain if closed:** +4 edges, joining Catalan to two top-valence nodes (G_G, π) and to both master-quadratic roots. Would also reduce the open-conjectures list in the G\* paper.
**Recommendation:** Author the PREREG (Session D1 of the plan) to lock the falsification basis. Do **not** spend research time attempting to close it without new tooling -- the PSLQ evidence is already the strongest currently-feasible signal.

### Bundle 2 -- Γ(1/5) ↔ {G_G, π, E-series, R_5} (≈12 edges, **W**)

**Plan source:** plan §A3 candidate 2 + Session B2 (C3 higher-Γ catalogue).
**Current state:** `R5 = Γ(1/5) Γ(4/5)`-like entries currently appear at valence 1 (single check `D4`). The §L (Eisenstein) and §M (quasimodular) blocks are saturated at τ = i (G_G hub) and τ = ρ (G_ρ hub); the τ corresponding to disc -20 / level 5 is not yet in the corpus.
**What would close it:** the standard quartic-AGM identity Γ(1/5)^5 = (constant) × π^k × AGM_4(...) or the level-5 modular-Lambert identities (Borwein-Borwein, Cooper-Guillera-Straub-Zudilin); add R_5, R_7, R_8, R_12 to the `R_n` family with cross-checks against the Γ-product machinery already in `proof_quartic_quarter_constants.py`.
**Difficulty:** W (literature-sweep; AGM machinery already exists in the project).
**Centrality gain:** ≈12 edges -- adds `Gamma_1_5`, `Gamma_1_7`, `Gamma_1_8`, `Gamma_1_12` as mid-valence nodes; cross-links to G_G via the higher AGM tower; joins the isolated `W5_raw` node back to the core if combined with bundle 5.
**Recommendation:** Schedule for Session B2 if pursued. The catalogue is well-trodden in the literature; extending `verify_gstar_paper.py` with ≈20 new checks is mechanical once the references are pinned. The novelty for a *paper* is low; the novelty for the *graph* is high.

### Bundle 3 -- η(i/2), η(2i), η(i/3) ↔ {G_G, θ-functions} (≈6 edges, **D**)

**Plan source:** plan §A3 candidate 3.
**Current state:** `eta_i_half` and `eta_2i` appear at valence 2 each; the cross-link to θ-functions is partial (the θ block uses `theta2_i`, `theta3_i`, `theta4_i` with valence 3, 1, 1). The Hauptmodul identity `eta(i/2) eta(2i) = ...` -- a standard CM tower identity -- would join all of these.
**What would close it:** ≈6 standard Ramanujan-Selberg identities, all verifiable at machine precision via mpmath; the modular-transformation derivations are textbook (Cohen, *Number Theory I*, Chapter 5).
**Difficulty:** D (desk work; one afternoon).
**Centrality gain:** ≈6 edges -- doubles the valence of `eta_i_half`, `eta_2i`, `theta3_i`, `theta4_i`; tightens the τ = i modular sub-network.
**Recommendation:** Highest-leverage **D**-difficulty target. Convert to ≈6 new `check()` entries with cross-references to the textbook source. Excellent first-extension target for any future session on the corpus.

### Bundle 4 -- j(i) = 1728 ↔ Master quadratic (≈3 edges, **D**)

**Plan source:** plan §A3 candidate 4.
**Current state:** `j_i` at valence 1 (`L11: j(i) = 1728`). The CM-uniqueness argument in `EXPLR_CM_RATIO_TOWER.md` connects j(i) = 1728 to the master-quadratic coefficient 16 (via |Aut(E)| = 4 and the (j, |Aut|) correspondence), but no explicit identity links `j_i` to `Gstar`, `x_plus`, or `x_minus`.
**What would close it:** make the j-↔-coefficient-16 connection explicit as a verifiable identity (e.g. `16 = |Aut(E_{j=1728})|^2`; cross-check that `Gstar = R_4 = Γ(1/4)/Γ(3/4)` is the CM ratio at the τ = i lattice). Some of this is already in the §C and §L blocks; the missing edge is the explicit `j_i ↔ coefficient_16` cross-reference.
**Difficulty:** D (a single afternoon, mostly book-keeping).
**Centrality gain:** ≈3 edges; pulls `j_i` into the master-quadratic neighbourhood and reduces the isolated-leaf count by 1.
**Recommendation:** Schedule alongside Bundle 3.

### Bundle 5 -- W^(5) ↔ Γ(1/5) (≈2 edges, **W**)

**Plan source:** plan §A3 candidate 5.
**Current state:** `W5_raw` at valence 1. The Bailey-Borwein-Crandall closed form for W^(5) involves Γ(1/5)-products (analogous to W^(3) ↔ G_G^2 and W^(4) ↔ AGM-quartic forms).
**What would close it:** the BBC formula plus its mpmath verification. **Combine with Bundle 2** -- the Γ(1/5) edges are needed before W^(5) can connect through them.
**Difficulty:** W (when bundled with Bundle 2).
**Centrality gain:** ≈2 edges; pulls `W5_raw` into the Γ(1/5)-extended core.
**Recommendation:** Roll into Bundle 2.

---

## §4 -- Currently-unverified-but-load-bearing edges

These are edges the synonymy graph does **not** carry but that load-bearing FTD claims rely on. They are flagged as accidental-graph-omissions rather than mathematical gaps -- the identities are claimed elsewhere in the corpus but not yet in `verify_gstar_paper.py`.

| Missing edge | Where it's claimed | Why it matters | Priority |
|---|---|---|---|
| `G_G^4 ↔ Γ(1/4)^8/(64 π^6)` | G\* paper Theorem 12.5 (after critic fix) | Verified post-fix; verify_gstar_paper.py already covers via B5; redundant edge but worth a duplicate `check()` for the explicit G_G^4 form | D, low |
| `G_rho ↔ cubic-AGM closed form` (Theorem 18.1) | G\* paper §18 | Verified in `proof_quartic_quarter_constants.py` and `investigate_p2_cubic_agm.py`; not yet in `verify_gstar_paper.py` | D, medium -- pull `M_3(1, 2^{-1/3})` in as a new node |
| `Catalan ↔ algebraic-independence basis` | G\* paper §19 Conjecture 19.2 | Conjecture, not theorem; PSLQ-only evidence; PREREG candidate per Bundle 1 | FO, **PREREG-only** per Session D1 |
| `W^(3)_BCC ↔ K(1/√2)^2 / π^2` | already at G2 line 174 | Verified; no action | -- |
| `omega_E ↔ G_G ↔ Gstar` triangle | A3, A4, E2, E3 (all verified) | Verified; no action | -- |
| `E_2(τ) at τ=i ↔ 3/π` | M1 line 365 | Verified; the cross-link to `Delta_i = (E_4^3 - E_6^2)/1728` is implicit but not a separate `check()` -- one D-difficulty addition would tighten | D, low |

---

## §5 -- What this roadmap does NOT do

- Does not promote or demote any FTD claim.
- Does not propose any new theorem.
- Does not assert that high-valence objects are mathematically more important than low-valence ones -- the bias may simply reflect what the paper's authors chose to cross-check, not what the underlying mathematics calls central.
- Does not exhaust the verification corpus -- only `verify_gstar_paper.py` is extracted; the ≈55 other `verify_*.py` scripts under `scripts/verification/` carry their own (separately-buildable) sub-graphs.
- Does not weigh "research production" against "research navigation." Per plan §R3, if the user prefers research production (Bundle 1 closure attempt) over research navigation (this roadmap), the roadmap is a low-priority artefact.

---

## §6 -- Reproducing the graph

```sh
# Re-extract from the current verify script:
python scripts/verification/extract_synonymy_graph.py

# The JSON output:
cat scripts/verification/results/synonymy_graph.json | python -m json.tool | head -50
```

The script reads `verify_gstar_paper.py` with `ast`, walks every `check(label, computed, claim, ...)` call, extracts named identifiers (Name + Attribute leaves) from arguments 2 onwards, applies the `EXCLUDE_NAMES` filter (Python builtins + mpmath plumbing + script-internal helpers), and emits the bipartite + pairwise edge lists plus per-object valence + first-seen-line.

Adding a new `check()` to `verify_gstar_paper.py` automatically regenerates the graph on the next extraction run -- no manual edits to the JSON or this roadmap required (only the §2 statistics and the §3 priority list should be re-curated when the corpus drifts substantively).

---

## §7 -- Cross-references

- `scripts/verification/verify_gstar_paper.py` -- the 100-check verification script the graph extracts from.
- `scripts/verification/extract_synonymy_graph.py` -- the extractor.
- `scripts/verification/results/synonymy_graph.json` -- the canonical machine-readable graph.
- `docs/papers/PAPER_GSTAR_INTRODUCTION.tex` §19 -- Conjecture 19.2 (Catalan algebraic independence) -- Bundle 1's frontier.
- `docs/theory/09_mathematical/REF_GUILLERA_CORPUS_MAP.md` -- the Borwein-Cooper-Guillera-Straub-Zudilin AGM-modularity bridge that powers Bundles 2 and 5.
- `docs/theory/09_mathematical/EXPLR_CM_RATIO_TOWER.md` -- the CM-uniqueness machinery Bundle 4 connects to.
- `.claude/plans/let-s-proceed-on-the-eager-rocket.md` -- Sessions A3 (this work) + B2 (C3 higher-Γ catalogue) + D1 (Catalan PREREG).
