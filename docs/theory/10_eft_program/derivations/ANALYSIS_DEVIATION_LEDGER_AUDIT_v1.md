# FTD-0795 — The Deviation Ledger Audited Against Experiment

**Status:** `[AUDIT — FTD-0258 SIX ROWS]` +
`[CONDITIONAL CLOSED NEGATIVE — PL-1 PHYSICAL READING]` +
`[EXACT — LOCAL-FIELD DETECTION BOUND alpha >= 1]` +
`[MAINTENANCE — PL-6 STALE AGAINST ITS OWN LEDGER]`
**Verdict:** `SPINE_HAS_NO_ROW_THAT_IS_DEVIANT_LIVE_AND_TESTABLE`
**Parents:** `FTD-0200`, `FTD-0243`, `FTD-0255` (FC-1), `FTD-0258`, `FTD-0355`, `FTD-0564`
**Production impact:** none

## 1. The bound, proved and verified

FTD's detection ontology is a single continuous local flux `J(x,t)` with
manifestation on `|J| > K_B`. Split one excitation 50/50; both arms receive the
**same** half-amplitude. Then for the anticorrelation parameter
`alpha = P_coinc / (P_A * P_B)`:

- **Deterministic threshold** (FTD's actual rule): the arms fire together or
  not at all, so `P_A = P_B = P_coinc = P` and `alpha = 1/P >= 1`. In the
  rare-event (single-excitation) regime `alpha >> 1` — maximal **bunching**.
- **Any intensity-monotone rule**, Rice upcrossing included: the arms are
  perfectly correlated, so `alpha = <p^2>/<p>^2 >= 1` by Cauchy–Schwarz, with
  equality only if `p` is almost surely constant.

Verified numerically across field statistics and rules — hard threshold at the
90th percentile gives `alpha = 10.0` for Gaussian, lognormal and near-constant
fields; a Rice-like rule gives `2.69`, `5.04`, `1.00`. **No local field with a
local detection rule reaches `alpha < 1`.**

Measured: `alpha = 0.0188 +/- 0.0067` — **146 sigma below the floor**.
Grangier–Roger–Aspect 1986: `0.18 +/- 0.06`, 14 sigma below.

**Why this cannot be deflected.** `alpha` is coincidences over singles at one
beamsplitter with one detector type: efficiency, gain and threshold **cancel in
the ratio**. It invokes no Hilbert space, no Born rule, no quantum formalism at
all. It is a raw counting ratio. A programme may reject the entire quantum
formalism as ad hoc and still owe this number.

**Scope.** Conditional on the manifestation ≡ detection identification, which
FTD tags `[CONJECTURE]`. That is the only escape, and taking it empties PL-1 of
physical content.

## 2. The six rows

| row | verdict | reason |
|---|---|---|
| **PL-1** Rice vs Born | **DEAD (physical reading)** | §1 above |
| **PL-2** CHSH `S <= 2` | **VACUOUS** | the row itself states it is not a lab prediction; its only sharp falsifier (substrate `S > 2`) is entailed **false** by FTD-0243 |
| **PL-3** `[q,p] = 0` | **DEAD as physics / VACUOUS as written** | 15 dB squeezing with matched anti-squeezing, and the Arthurs–Kelly 3 dB joint-readout floor, exclude co-measurable quadratures; the row escapes via "demonstrably M-free readout", an undefined predicate no apparatus has instantiated |
| **PL-4** IR-emergent gamma | **LIVE, untestable** | effective `E_QG,2 = sqrt(24) E_P = 6.0e19 GeV` against LHAASO `> 1.2e12 GeV` — `5e7` in energy scale, `2.5e15` in observable delay |
| **PL-5** `k^4` anisotropy | **LIVE, untestable** | `(E/E_P)^4 = 4.5e-57` at 100 TeV against best isotropy sensitivity `~1e-18` — about 39 orders |
| **PL-6** structural nulls | **VACUOUS + STALE** | predicts the Standard Model's own nulls (zero discriminating power), **and** two of its three cited `[THEOREM]` nulls were retracted internally without the row being updated |

**No row is simultaneously deviant, unexcluded, and testable.**

## 3. PL-2's circularity, recorded

`AUDIT_BELL_ANALYSIS.md` derives the observer-level `S = 2 sqrt 2` from
complexification — the Gauss constraint removing one flux mode, leaving
`psi = J_x + i J_y` — plus joint coupling, and **concedes that this
complexification is an instance of the measurement map `M` that FC-1
declines**. The framework's account of the best-established fact in quantum
foundations therefore imports the very object it declared itself complete
without.

## 4. PL-6 is stale against the canonical LEDGER

The `proof_complete_sm.py` line numbers cited in PL-6 no longer resolve. Live
text:

- **monopole null** — downgraded to `[COND. THM]` with the universal claim
  `[OPEN]` by **FTD-0564**: `div(curl J) = 0` is conditional on a regular
  global potential and does not exclude nontrivial `U(1)` bundles;
- **extra dimensions** — the code itself cites `D = 3 [SELECTION], not forced`
  per **FTD-0355**, which makes "no extra dimensions" a restatement of the
  choice rather than a theorem;
- **SUSY null** — the only survivor, and it needs reconciling with the
  maintained `derive_fermion_sector.py`: "the ternary algebra has no grading"
  versus "FTD has a fermion sector" is a live internal-consistency question.

Under the ledger-precedence rule the row is currently wrong on paper and must
be re-annotated.

## 5. The decisive test, and the fork it forces

**Run FTD's own manifestation rule on a lattice beamsplitter and report
`alpha`.** Cost is comparable to the existing FTD-0200 run; pre-register the
kill at `alpha < 1`. Structurally it must return `alpha >= 1`.

That leaves exactly one fork, and it is the same fork Bell forces:

- **Accept explicit nonlocality with a preferred frame.** Then `alpha < 1` and
  `S = 2 sqrt 2` become reachable — the programme becomes a neo-Lorentzian
  hidden-variable theory — and the whole difficulty moves to the GR side, where
  FTD-0412 already registers `LIVE-COMMON-CONE-FAILS` and the framework's own
  analysis says the common cone cannot emerge by RG flow but must be exact by
  symmetry.
- **Refuse it.** Then FTD predicts `alpha >= 1` and `S <= 2`, and both are
  experimentally dead — by 146 sigma, and by the 2015 loophole-free Bell tests.

The programme's two stated halves — replacing quantum mechanics, and connecting
semantically to general relativity — pull against each other through this
single joint.
