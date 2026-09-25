# Corrections to Fifty-Two Faces of the Lemniscatic Bridge Constant

Review date: 24 September 2026. Scope: `PAPER_GSTAR_IDENTITIES.tex`, its complete 52-entry catalogue and summary table, supporting normalisations, references, and verification claims. The existing 52 IDs are retained. No physical identification, project ledger status, or priority claim is promoted.

The repaired collection contains 41 exact entries whose target is G*, ten exact identities for related quantities, and one bounded approximation (E5). This is an inventory, not a count of independent derivations. Some exact entries are limits or integral representations; evaluating an equivalent closed form does not numerically certify an arbitrary quadrature or finite truncation.

## Search and evidence

Before changing the sensitive entries, the review searched the companion monograph, the mathematical theory directory, `scripts/proofs/`, `lean/FTD/`, and `docs/reference/REF_BIBLIOGRAPHY.md`. In particular:

- `DERIV_LFUNCTION_GSTAR_CONNECTION.md` already stated the correct least real period, two real components, and Tamagawa number. The independent point-counting script `scripts/proofs/proof_lvalue_deligne_verification.py` was found and run. Thus the suspected factor-of-two defect in G1/G2 was rejected.
- `lean/FTD/Axioms.lean` explicitly discloses that its relevant `True` declarations are citation bookkeeping. The paper's blanket machine-verification claim was inconsistent with that disclosure.
- DLMF 19.7.2, 19.8.5, 20.9.2, 23.5.5, and 5.11.8 were checked for imaginary-modulus, AGM, theta, lemniscatic-value, and shifted log-Gamma conventions.
- Li, Long and Tu, *SIGMA* 14 (2018), 090, Lemma 3.1, supplies the exact fixed-curve L-value by a Mellin/Beta integral. This is more precise evidence for G1/G2 than a general appeal to Coates–Wiles or Rubin.

The references are [DLMF](https://dlmf.nist.gov/), [Li–Long–Tu](https://doi.org/10.3842/SIGMA.2018.090), and [LMFDB's curve 32.a3](https://www.lmfdb.org/EllipticCurve/Q/32/a/3). The paper now includes these sources explicitly.

## Findings and disposition

Line numbers below locate the repaired mathematical section; entry IDs remain the stable anchors if later typesetting changes move lines.

| File:line / entry | Severity | Defect, reason, and correction | Verification |
|---|---|---|---|
| `PAPER_GSTAR_IDENTITIES.tex:253`, C2 | Medium | The old coefficient 4 was too large by sqrt(2). Replaced it by 2sqrt(2), consistent with the modulus convention and C1. | Recomputed at 100 decimal working digits; absolute residual below 3e-101. |
| `PAPER_GSTAR_IDENTITIES.tex:256`, C3 | Medium | `K'(i)` denoted an ambiguous complementary integral and did not justify the stated real value. Replaced by the unambiguous imaginary-modulus identity G*=4K(i)/sqrt(pi), defining its real positive integral. | DLMF 19.7.2 and independent evaluation of the integral with parameter -1. |
| `PAPER_GSTAR_IDENTITIES.tex:258`, C4/C5 | Non-issue | The original coefficients **2 outside the square root and 4 in the squared relation are correct**. A provisional suspicion was rejected: K(2E−K)=pi/2, so G*²=8K²/pi=4K/(2E−K). | Recomputed at 100 digits; residuals 0 at working precision and approximately 1.1e-100. The verifier independently caught the same erroneous proposed alteration before finalisation. |
| `PAPER_GSTAR_IDENTITIES.tex:288`, D3 | Medium | AGM homogeneity requires an additional sqrt(2) in the numerator. | Recomputed at 100 digits against the defining Gamma ratio. |
| `PAPER_GSTAR_IDENTITIES.tex:290`, D4 | High | The earlier cosine product lacked a valid, supported convergent formula. Replaced it with the natural telescoping product obtained directly from the stated AGM recurrence: 2sqrt(pi) times the product of 2/(1+r_n). | With a0=1, b0=sqrt(2), each factor is a_n/a_(n+1), so the first N factors equal 1/a_N. Direct iteration agrees at 100 digits. This is an algebraic AGM restatement, not a newly discovered identity. |
| `PAPER_GSTAR_IDENTITIES.tex:334`, theta evaluation | Medium | The prose's Gamma expression for theta3 had denominator 2^(1/4), whereas the correct factor is sqrt(2). The numbered E1–E4 identities themselves were correct. | Theta series gives 1.086434811213308014575316121510…; combine K=(pi/2)theta3² with the lemniscatic K-value. |
| `PAPER_GSTAR_IDENTITIES.tex:321`, E5 | Medium | An approximation was included in blanket statements that every entry agreed exactly. It is now expressly distinguished and has a positive tail bound. | Absolute error is 0.0000379880371667351238…; the proved upper bound is below 3.7989e-5. |
| `PAPER_GSTAR_IDENTITIES.tex:384`, F4 | High | The old denominator was an FCC step symbol inside a purported BCC Watson identity, with an incompatible prefactor. Replaced by the BCC symbol 1−cos(k1)cos(k2)cos(k3) and the factor 1/(4pi²) for G*². | Normalised Fourier averaging produces the BCC return series sum binom(2n,n)^3/64^n; Watson's evaluation gives W3=4K²/pi² and G*²=2pi W3. Singularities are locally integrable. |
| `PAPER_GSTAR_IDENTITIES.tex:416`, G1/G2/G4 | Non-issue | The factors 8L(E,1)/sqrt(pi) and 2Omega+/sqrt(pi) were correct. Their meaning needed the specified curve, differential, and period normalisation. | Point-counting plus the accelerated conductor-32 L-series gave L(E,1)=0.655514388573…=varpi/4 with relative residual 5.254e-41 at 40-digit precision. Li–Long–Tu Lemma 3.1 proves the exact Beta value. |
| `PAPER_GSTAR_IDENTITIES.tex:420`, G3 | Medium | The old 4sqrt(2/pi) coefficient was smaller than the correct coefficient by sqrt(2). Replaced it by 8/sqrt(pi), explicitly a duplicate algebraic form. | Same independently evaluated L-value as G1/G2. |
| `PAPER_GSTAR_IDENTITIES.tex:427`, Family G prose | Medium | `End(E)` did not specify the field of definition, and a general BSD citation concealed normalisation and theorem-scope issues. Full CM endomorphisms are now stated over the algebraic closure; the exact value is cited to a direct fixed-form Mellin evaluation. | Curve is LMFDB 32.a3/Cremona 32a2, least positive real period varpi for dx/(2y), full real volume 2varpi. No statement about twists follows. |
| `PAPER_GSTAR_IDENTITIES.tex:461`, H1 | Medium | The old hypergeometric prefactor was incorrect. K(k)=(pi/2) 2F1(1/2,1/2;1;k²) gives G*=sqrt(2pi) 2F1(1/2,1/2;1;1/2). | Recomputed at 100 digits; residual below 3e-101. |
| `PAPER_GSTAR_IDENTITIES.tex:623`, H5 summary | Low | The table's slash notation placed pi ambiguously outside the denominator. Rewritten as an explicit fraction 16/(sqrt(2)pi). | Compared the summary directly with H5 and Gamma(5/4)=Gamma(1/4)/4. |
| `PAPER_GSTAR_IDENTITIES.tex:530`, J1 | High | The prose mixed the theta and eta nomes and called a nonholomorphic expression a holomorphic modular form. It also overstated uniqueness of a modular fixed point. | Defined Theta(tau)=sum exp(pi i n²tau); stated the S and T² laws on the theta subgroup with multiplier. The fixed point claim is restricted to S in the upper half-plane. G* is a scalar special value, not an involution operator. |
| `PAPER_GSTAR_IDENTITIES.tex:191`, A13/A14 prose | Medium | The products include all odd integers, not just split/inert primes, and “independent races” did not follow from residue classes. Novelty/priority language was unsupported. | Gamma finite-product formula proves both limits. Prime splitting is now restricted to odd primes; no independence or novelty inference is made. |
| `PAPER_GSTAR_IDENTITIES.tex:644`, convergence theorem | High | The stated errors, digits per decade, exact-Gamma correction, and workstation/digit claims were mutually inconsistent. Standardised A13 to exactly N factors and derived the actual asymptotic expansions. | Bernoulli-polynomial computation gives log(Spi/sqrt(pi))=1/(8N)−1/(192N³)+…, log(SG/G*)=1/(64N²)−5/(2048N⁴)+…. Direct products at N=100 and 1000 confirm the leading coefficients. The specified exponential corrections have orders N^-3 and N^-4. |
| `PAPER_GSTAR_IDENTITIES.tex:713`, numerical verification | High | Claims of 52 expressions all equalling G*, global 200-digit agreement, and Lean formalisation were unsupported and sometimes mathematically impossible because targets differ. | Removed the blanket assertions, identified the ten other targets and E5, linked reproducible per-entry checks, and disclosed the `True` placeholders. |
| `MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md:368`, matching Wallis passage | Medium | The balanced N-factor product was assigned the same erroneous 1/N rate; Stirling's series was also described as convergent. | Narrow follow-through repair: the rate is 1/(64N²)+O(N^-4), and Stirling is an asymptotic expansion. Other monograph claims were outside this bounded correction. |

## Verification coverage

The repository now preserves the canonical catalogue and verifier as
`scripts/verification/gstar_identity_catalogue.py` and
`scripts/verification/verify_gstar_identity_catalogue.py`. Run
`python scripts/verification/verify_gstar_identity_catalogue.py` to regenerate
the portable resource copies, atlas and report. Its `--output-dir PATH` option
creates a self-contained verification bundle, including the reviewed paper.
The dissemination directory remains subject to the existing ignore policy;
the canonical Python sources are eligible for normal repository tracking.

Shared-source follow-through corrected the theta coefficient and the paired
period/Tamagawa normalisation in `PAPER_GSTAR_BRIDGE_CONSTANT.tex`. In
`PAPER_MISSING_RATIO.tex`, the fixed-curve L-value now has a direct reference,
prime terminology is restricted correctly, unsupported historical priority and
factor-independence claims were removed, and the unsupported universal product
nonexistence claim was replaced by the exact finite Gamma-product identity.
The three abstracts were synchronised in `MASTER_ABSTRACT_CATALOG.md`, and
classical references were added to `REF_BIBLIOGRAPHY.md`. These companion changes
do not constitute an audit of every physics claim in those papers.

All 52 equation IDs and all 52 summary rows were counted, matched, and checked for duplicate IDs. The TeX remains ASCII-only. `git diff --check` passed for the repaired source.

The complete machine-readable catalogue and executable checks live at `dissemination/interactive/gstar-introduction/identity_catalogue.py` and `verify_catalogue.py`; the per-entry output is `CATALOGUE_VERIFICATION.json`. The verification uses 110-decimal working precision and entry-specific methods. Directly evaluated identities, finite-limit enclosures, proved truncation bounds, symbolically established reductions, and source-normalisation checks are distinguished there. The report should be consulted for the exact final run and source hash; this correction note does not turn those checks into a 52-theorem formal proof.

The independently run existing L-value script also contains unrelated symmetric-square computations. Those computations were not used as evidence for this catalogue's fixed-curve central value. No numerical near-miss or coincidence searches were run.

This is a provisional, AI-simulated review, not a substitute for actual external human mathematical review. An outside mathematician may find errors missed here or dismiss concerns raised here; the thoroughness of these checks is not external mathematical validation.
