# AUDIT — 10-specialist red-team dissection of FTD, adversarially self-verified

**Tag:** `[CRITIC SYNTHESIS]` / `[MULTI-SPECIALIST RED-TEAM REPORT]`
**LEDGER id:** FTD-0345 (triggers the remediation logged in the same row; the boundary-framing restructuring below is FTD-0346)
**Status:** the source document the FTD-0345/FTD-0346 remediation fixes against. Golden gate untouched (docs only).

---

## 0 · Mandatory disclosure — read this before anything below `[grounded]`

**This audit, like every prior "adversarial," "referee," or "critic" pass in this corpus, was performed entirely by AI** — ten parallel instances of the same system, each role-playing a specialist, independently verified by ten more instances of the same system role-playing a skeptic of the first ten, synthesized by an eleventh instance acting as chair. **It is a real internal-consistency check. It is not external human validation, and it does not discharge the corpus's still-outstanding external-review item** (tracked structurally as of this remediation — see FTD-0346). Every finding below should be read with that ceiling in mind: these are the failure modes an AI system, prompted adversarially, can find in its own project's prior AI-assisted output. That is a genuinely useful check — and it is a different, weaker thing than a mathematician or physicist outside the project reading this corpus cold.

---

## 1 · Method

Ten independent specialists, each assigned a distinct attack surface and instructed to search the actual corpus before leveling any accusation (a lazy critique re-discovering something the project's own LEDGER already fixed is worthless): **number theory/CM curves**, **statistics/numerology-detection**, **QFT/particle phenomenology**, **GR/quantum-gravity**, **philosophy of science/falsifiability**, **software/engineering audit**, **AI-collusion/narrative-bias** (the specific risk the project's own GTCA discipline calls F9), **citation/literature integrity**, **LEDGER self-consistency**, and **the unfalsifiable-boundary-framing** attack (a sharper, narrower cut than the philosophy-of-science pass). Each specialist's raw findings then went to an *independent* verifier instance whose only job was to try to refute them — re-checking the corpus itself, not trusting the specialist's quoted evidence — rendering CONFIRMED / PARTIALLY_CONFIRMED / REFUTED / ALREADY_ADEQUATELY_ADDRESSED. A chair assembled only what survived. 21 agents total, 447 corpus searches, ~2.1M tokens.

---

## 2 · Confirmed — high severity

1. **Unfalsifiability presented as a virtue.** `FOUND_MODULUS_ARGUMENT_FRONTIER.md` (pre-remediation) stated the frontier "does not depend on x₊=1/α being true — it explains the boundary whether or not the central conjecture holds, which is exactly why it is the project's most defensible asset." Three independent specialists converged on this one sentence: a component whose explanatory success is invariant to the truth of the framework's central conjecture has, by Popper's criterion, zero empirical content in that respect — and the corpus named that property its chief strength. **Remediated in FTD-0346** by replacing the invariance-as-virtue framing with a per-instance concrete falsifier.
2. **The failure-ratchet.** A route that works derives α (win). A route that fails "strengthens the boundary" (`AUDIT_BOUNDARY_MAP.md`, pre-remediation: "a sympathetic red-team's attempt at a 5th route also failed (0/5), strengthening the boundary"). No outcome counts against the program. **Remediated in FTD-0346** via the PROVEN-impossibility/attempted-and-failed split.
3. **The retracted ~4×10⁵:1 Bayes figure survives, uncaveated, exactly where readers look first** — the LEDGER's own FTD-0001/FTD-0121 detail rows, a Wilsonian-EFT paper, and a v1 manuscript chapter, despite CLAUDE.md itself already carrying the correct `[NUMERICAL FACT], not a Bayes result` caveat. **Remediated in FTD-0345.**
4. **"Full nonlinear Einstein equations emerge" ships to readers with the fatal caveat stripped** — FTD-0189 (h_μν is posited, not derived) and FTD-0193 (no emergent spin-2 mode, closed negative) are real, internal, effective self-corrections that never reached the manuscript chapters or the public FAQ. **Remediated in FTD-0345.**
5. **Two engine benchmarks are epistemically inverted or vacuous** — the Bell-CHSH "A+" result is the *classical* local bound (a genuinely quantum substrate would exceed it toward Tsirelson's bound), scored as a QM win by a standalone toy that doesn't touch the lattice engine; the hydrogen-spectrum "A+" is a calibrated classical 1/n² identity the engine's own source code disclaims. **Remediated in FTD-0345.**
6. **The adversarial-validation apparatus — including this document** — is the same AI system critiquing its own output, cited forward in prior docs as if it discharged the corpus's repeated "needs external non-AI critique" lines. The one scheduled human review (a number-theory colleague, planned for May 2026) is silently past-due with no referee report anywhere. **Disclosed structurally, not resolved, in FTD-0346** (§0 above, and the tracked open item it adds).

## 3 · Confirmed — medium/low (representative; full detail in the FTD-0345 LEDGER row)

- `m_e`'s exponent n=11 tagged `[DERIVED]` in one doc, `[SELECTION]` in a later, more scrutinized one — unreconciled until FTD-0345.
- Theorem 1's proof misattributed to Chowla-Selberg/L(1,χ₋₄) when it is elementary Euler reflection — the *value* was always correct, the *citation* was wrong.
- Two explicitly-retracted claims (Phase-J "disconfirmed for general L"; Q(G\*) "maximal") survived verbatim in a live compiled book chapter the 2026-06-24 spine audit's file-by-file sweep never reached.
- A biophysics paper presented untagged numerology as demonstrated fact and was simultaneously misclassified as "clean" by the corpus's own hazard-tracking.
- The golden-hash regression gate was described corpus-wide as guarding "physics" while ~14 subsystems sit toggled off in its frozen config.
- The α-precision-formula manuscript chapter blended a genuinely-computed lattice integral with a fit coefficient under one blanket `[THEOREM]` header (confirmed via direct diff of the v2-src and vol1 copies — content-identical, both affected).
- The α-power-ladder chapter carried zero epistemic tags anywhere, indistinguishable from the numerology CLAUDE.md's own discipline bans.
- The keystone transcendence citation (algebraic independence of π and Γ(1/4)) was attributed to different authors/years in prose vs. the Lean formalization; a fabricated author name ("Schipnitzer") appeared in a checklist-closing document; five load-bearing operator-algebra/holographic citations were entirely absent from the "single source of truth" bibliography.
- CLAUDE.md itself — the file every agent reads first — carried one untagged bullet inconsistent with an adjacent, correctly-tagged one.

## 4 · Refuted or already adequately handled (shown for transparency)

- FC-2's emergent-metric claim vs. its preferred-frame commitment: no undemonstrated compatibility found; honestly declared with Lorentz-violation offered as a live falsifier.
- CKN/Hsu 2004 usage in the Λ sector: physics stated correctly, tagged honestly.
- The core algebraic-spine mathematics: genuinely correct to machine precision; the 2026-06-24 adversarial audit was a real, effective self-correction.
- The "zero promotions" LEDGER invariant: genuinely holds in the core ledgers and the flagship whitepaper; no id-collision bug found.
- The G\*/"orphaned" framing: mildly overstates novelty but is heavily hedged and near-self-caught — downgraded, not confirmed as a live overclaim.
- The g\*-paper round-table's self-labeling ("agent in critic mode," "simulated"): honest about what it is — the confirmed problem is only that its verdicts are then cited forward as if they discharged external review.
- The wins=walls §7.1 synthesis (FTD-0344): correctly records its own round table's verdict that a broader QM/GR reading was "mostly relabeling" and explicitly does not adopt it — genuine Popperian hygiene.

## 5 · Cross-cutting patterns (independently found by multiple specialists — stronger evidence than any single finding)

1. **Incomplete propagation of self-corrections** (7 of 10 specialists) — fixes applied to the flagship theory layer, stale everywhere a reader actually looks.
2. **The dissemination layer is the unguarded front door** — manuscripts, papers, the web FAQ, benchmark scorecards consistently lag behind the theory-layer corrections.
3. **Obstruction-as-deliverable / unfalsifiability-as-virtue** — the institutionalized rhetorical move this document's §2 items 1–2 name directly.
4. **Tag-system gaming** — the tag is correct; the surrounding prose's rhetorical force is undiminished by it.
5. **Same-system self-critique counted as independence** — this document's own §0 disclosure exists because of this pattern.
6. **Circular/vacuous validation sold as physics confirmation** — a test harness verifying fits reproduce the values they were fit to, benchmarks grading a classical result as a quantum win.
7. **Attribution instability on the keystone conditional** — the single transcendence input the whole conditional-theorem stack rests on, cited inconsistently across the corpus.

## 6 · Overall verdict

FTD is not a fraud and not crank-numerology dressed as physics — and that is precisely what makes its real failures worth naming. The per-claim epistemic hygiene inside the core theory layer is genuinely, unusually good; the algebraic spine's mathematics is correct to machine precision; the 2026-06-24 spine audit and the FTD-0189/0193 graviton nulls are real, effective self-corrections; a genuine pre-registration apparatus exists. A lazy "it's all coincidence-hunting" critique fails against this corpus. The damage is concentrated in two systematic places the self-audit apparatus does not reach: **propagation** (corrections stop at the `docs/theory/` boundary; the reader-facing dissemination tier is stale) and **framing** (failures become deliverables, unfalsifiability is advertised as strength, and every "external critique" pass is the same AI reviewing itself, cited forward as if that discharged the debt). The epistemic overclaim CLAUDE.md's own discipline bans has not been eliminated by this corpus's considerable internal apparatus — it has been pushed to the periphery and up into the framing, exactly where per-claim tag discipline provides no coverage.

---

## 7 · What this triggered

- **FTD-0345** — the mechanical propagation sweep fixing every item in §2.3/§2.4/§3 above, plus the benchmark re-grading.
- **FTD-0346** — the boundary-framing restructuring (the PROVEN/ATTEMPTED split, per-instance falsifiers replacing invariance-as-virtue, and the structurally-tracked external-review disclosure).

**Zero promotions.** `x₊=1/α` stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; no α is derived anywhere; golden hash `0xb604d81a3d79366e` untouched. This remediation downgrades and discloses; it upgrades nothing.
