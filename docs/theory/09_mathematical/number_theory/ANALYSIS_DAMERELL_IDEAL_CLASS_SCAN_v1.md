# FTD-0321 — The full per-ideal-class CM scan: d = −4's dual-match privilege is a range artifact

**Status:** `[NUMERICAL FACT — EXHAUSTIVE OVER STATED DOMAIN]` +
`[CLOSED NEGATIVE — d = −4 PER-CLASS UNIQUENESS FAILS BEYOND THE REGISTERED RANGE]` +
`[MEASURED — THE CRITERION HAS NO DISCRIMINATING POWER AT SCALE]`
**Verdict:** `UNIQUENESS_HOLDS_ON_ITS_REGISTERED_DOMAIN_AND_NOWHERE_ELSE`
**Executes:** `PREREG_DAMERELL_SCAN_v1.md` (design-locked 2026-06-24, run deferred; LEDGER row FTD-0321 reserved)
**Runner:** `scripts/proofs/proof_damerell_ideal_class_scan.py`
**Production impact:** none.

---

## 1. What was open

`TRACKER_ONTIC_TRUTH.md`, honest caveat on OT-1.9:

> the Γ-product analogue `G*_d` reproduces canonical G\* exactly at `d = −4` but
> at `h ≥ 2` it is a single-number analogue, not the full per-ideal-class
> Damerell formula. **A full Damerell scan at h ≥ 2 has not been run. Reviewer
> pressure point.**

Estimated effort in `EXPLR_CHOWLA_SELBERG_HIGHER_H.md` §4: *2–6 weeks*. This is
that scan.

## 2. A necessary departure from the letter of the pre-registration

§3 step 2 of the lock says to compute the per-class period "via the
Chowla–Selberg / Damerell formula (Γ-products weighted by the Kronecker
character, per ideal class)". **That is not possible as stated.** The h ≥ 2
Chowla–Selberg formula, as given in `EXPLR_CHOWLA_SELBERG_HIGHER_H.md` §3, is

```
prod_{[a] in Cl(K)} Omega_a^2 = (1/(2 pi |D|))^{h/2} prod_{a=1}^{|D|-1} Gamma(a/|D|)^{chi(a)}
```

which determines only the **product** over ideal classes. Γ-products cannot
separate the individual per-class periods — which is precisely why the FTD-0123
single-number scan "projects away the ideal-class structure" in the first place.

The per-class periods are taken instead from the Dedekind η function at each
class's CM point. For a reduced form `(a,b,c)` of discriminant `d < 0`, with
`tau = (−b + sqrt(d))/(2a)`:

```
G*_[a] := sqrt( 8 pi * |eta(tau)|^4 * Im(tau) )
```

`|η(τ)|⁴ Im(τ)` is SL₂(ℤ)-invariant, so this depends only on the ideal class.
The Γ-product then becomes an **independent check** rather than the definition.

## 3. Correctness gates (both passed before any verdict was read)

| Gate | Result |
|---|---|
| **G1** — `d = −4, h = 1` reproduces canonical `G* = Γ(1/4)/Γ(3/4)` | **PASS**, error `5.3e-51` at dps 50 |
| **G2** — Chowla–Selberg identity `prod_j (G*_j)^2 == (2/sqrt\|d\|)^h * Gamma_d^(w/2)`, h = 1 (9 fields) | **PASS**, worst `3.2e-50` |
| **G2** — same identity, **h ≥ 2** (16 fields, h ≤ 7) | **PASS**, worst `8.1e-50` |

The normalisation was fixed by the nine `h = 1` fields **only** and then required
to hold, unmodified, at `h ≥ 2`. It does. Chowla–Selberg therefore certifies the
per-class computation independently — a strictly stronger gate than §5's, which
checked `d = −4` alone. The gate was additionally re-run *at the counterexample
discriminant* (§5 below).

## 4. Registered result (§2 domain) — UNIQUE-CONFIRMED

| | |
|---|---|
| fields scanned | **279** (270 with h ≥ 2, plus the 9 Heegner controls) |
| ideal classes | **2,558** |
| dual-matchers | **1** — `d = −4`, form `(1,0)`, `x₊ = 137.0361714581555` |

**Domain note.** §2 names "the 54 fundamental discriminants with h ≥ 2 and
|d| ≤ 907" — that is the h ≥ 2 subset of FTD-0123's *truncated* 63-element set
(h = 4 was cut at the 20 smallest). This scan covered **all 270** fundamental
h ≥ 2 discriminants with |d| ≤ 907, a strict superset including the 216
FTD-0123 had truncated away. The §4 outcome letter is therefore
**UNIQUE-CONFIRMED**, on a larger domain than the lock required.

## 5. Deep extension (declared; beyond the lock, cannot change the §4 letter)

All reduced forms with `|d| ≤ 500,000`: float64 sieve (half-width `1e-4`,
~54× the match window and ~10¹¹× the float64 error, so it cannot hide a
matcher) followed by dps-50 confirmation.

| | |
|---|---|
| reduced forms enumerated | **61,582,891** |
| survived the sieve | 114,209 |
| confirmed dual-matchers | 2,954 |
| — **fundamental (genuine CM fields)** | **1,271** (1,270 beyond `d = −4`) |
| — `d = −4f²` conductor family | 352 — all `τ = i`, i.e. `d = −4` in disguise |
| — other non-fundamental | 1,331 |
| distinct fundamental discriminants with a matching class | **696** |

**Smallest counterexample: `d = −7895`** (h = 112, ideal class `(40, ±35)`,
`x₊` rel. dev. `3.62e-7` against the pre-registered `1.26e-6` gate). The
registered domain `|d| ≤ 907` is **~8.7× too small** to contain it.

The first few, all fundamental, all inside the locked tolerance:

| d | h | class | x₊ rel. dev. |
|---:|---:|---|---:|
| −7895 | 112 | (40, ±35) | 3.62e-7 |
| −15231 | 128 | (57, ±27) | 6.06e-7 |
| −15743 | 124 | (58, ±27) | 1.20e-6 |
| −32312 | 72 | (81, ±68) | 6.57e-7 |
| −36095 | 192 | (90, ±25) | 1.18e-6 |

`d = −15743` was verified independently and exhaustively: the form `(58,27,71)`
has discriminant exactly −15743 and is reduced; −15743 ≡ 1 (mod 4) and
15743 = 7·13·173 is squarefree, so the discriminant is **fundamental**; h = 124
and the form is a genuine member; **the Chowla–Selberg gate passes at that very
discriminant** (rel. error `1.2e-28`); and the match is stable across
dps 30 / 50 / 80 / 120 at `x₊` rel. dev. `1.19824415881e-6`.

## 6. The finding that matters — the criterion has no discriminating power

Counting counterexamples understates the result. Applying the FTD-0791 /
FTD-0802 base-rate method to the scan's own output, over `|d| ≤ 200,000`
(3,557,300 values of `G*` in (2.90, 2.98)):

| | |
|---|---|
| match window half-width in `G*` | `1.843e-6` |
| local density of `G*` near the target | **106,847,000 per unit `G*`** |
| expected matchers by chance | 394 |
| observed | 860 |
| **Monte Carlo, displaced targets** (n = 20,000) | **mean 203.8 matchers per random target, `P(≥1) = 1.0000`** |

**A randomly chosen target in this range is matched by ~204 ideal classes with
probability 1.** The dual-match criterion cannot discriminate anything once the
domain is large enough to be representative.

The mechanism is visible in the invariant. `G*² = 8π|η(τ)|⁴Im(τ)` behaves like
`8π · y · e^{−πy/3}` for `y = Im(τ)`, which is maximised near `y = 3/π ≈ 0.955`
— just above the reduced-form floor `y ≥ √3/2 ≈ 0.866`. The canonical
`G* = 2.9587` sits essentially **at that maximum**, i.e. in the densest region
of achievable values. The real target is not in a sparse place where a match
would be surprising; it is in the most crowded place available.

## 7. What this does and does not touch

**Does NOT touch — OT-1.9's Tier-1 content.** OT-1.9 as a Tier-1 entry is the
*arithmetic* statement that ℚ(i) is the unique imaginary quadratic field with
`|μ_K| = |disc(K)|`. That is pure arithmetic, verified independently, and is
completely unaffected by anything here.

**Does close — the OT-1.9 honest caveat.** The full per-ideal-class Damerell
scan has now been run. The "reviewer pressure point" is resolved, negatively.

**Does falsify — the "dual-match privilege" of d = −4** (already demoted to
`[NUMERICAL FACT]` by the 2026-06-24 spine audit, and already known to flip
under a rational-multiplier criterion per FTD-0124). It is now shown to be a
**range artifact**: true on its registered domain, false at |d| = 7,895, and
meaningless at scale.

**Consequence for OT-5.1.** After FTD-0791 (FTD-0319 leg) and FTD-0802 (OT-3.3
leg), the tracker recorded the remaining support for `x₊ = 1/α` as "OT-1.9 and
OT-1.5, both structural". The OT-1.9 leg as cited there is the *Chowla–Selberg
h-scan / d = −4 uniqueness* reading — and that reading does not survive this
scan. What remains is the Tier-1 arithmetic coincidence `|μ_K| = |disc(K)|`,
which is a statement about unit groups and discriminants and carries no
dual-match content. **`x₊ = 1/α` is not weakened as arithmetic — the 1.26 ppm
agreement is untouched — but its CM-uniqueness support is now the same kind of
finite-scan artifact the other two legs turned out to be.**

## 8. Banned moves (§5) — compliance

- **No criterion-switching.** The trivial-multiplier criterion (q = 1) fixed in
  §3 was used throughout; no alternative criterion was run or selected.
- **No tolerance tuning.** The `1.26e-6` / `0.80%` tolerances are those of the
  FTD-0123 runner, copied verbatim. The smallest counterexample clears the gate
  at 29% of tolerance and the largest reported at 95% — none was admitted or
  excluded by adjusting anything.
- **Correctness gate first.** G1/G2 ran and passed before any verdict was read.
- **No promotion.** The registered UNIQUE-CONFIRMED stays `[NUMERICAL FACT]`.

## 9. Cross-references

FTD-0123 (single-number h ≥ 2 scan, superseded at the per-class level),
FTD-0124 (criterion bifurcation), FTD-0318 (demoted the dual-match privilege),
FTD-0355 (scan-domain restatement), FTD-0791 / FTD-0802 (the base-rate method
applied here), `EXPLR_CHOWLA_SELBERG_HIGHER_H.md`,
`SPEC_ALGEBRAIC_SPINE.md` Theorem 3, `TRACKER_ONTIC_TRUTH.md` OT-1.9.
