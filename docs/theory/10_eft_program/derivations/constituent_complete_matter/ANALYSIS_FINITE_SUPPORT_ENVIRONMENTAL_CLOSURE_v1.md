# Analysis — finite-support environmental closure v1

**Identifier:** FTD-0745  
**Status:** `[SELECTED DYNAMICS — REGISTERED M2 LADDER CLOSED NEGATIVE AT E5]`  
**Verdict:** `ENVIRONMENTAL_CLOSURE_ARRIVAL_LAW_FAIL`  
**Date:** 2026-07-29  
**Production status:** unchanged

## 1. Question answered

FTD-0745 replayed the constructive FTD-0739 compact-support formation witness
on a held-out larger periodic quotient, preserving the entire smaller-volume
causal prefix. It asked whether the negative core and localized near field
would persist while a source-free outgoing component crossed six shells before
the earliest possible periodic return.

The exact transaction, causal-prefix, bound-control, polarity, core, and
near-field gates pass. The ordered shell-arrival conjunction fails because no
unbound history exceeds the locked `1e-8` threshold at radius 32 or 48 by tick
184. The formal verdict is therefore the first failed branch, E5:

```text
ENVIRONMENTAL_CLOSURE_ARRIVAL_LAW_FAIL.
```

## 2. Run of record

| item | value |
|---|---:|
| held-out volume | `L=193` |
| compact support radius | `R0=4` |
| forward horizon | `184` ticks |
| earliest periodic self-contact | tick `185` |
| histories | `5` |
| persisted forward rows | `925` |
| causal-prefix scalar difference | `2.200e-14` |
| unbound 64-tick core passes | `4/4` |
| unbound late near-field passes | `4/4` |
| bound controls | `1/1` |
| polarity scalar difference | `0` |
| ordered six-shell arrival passes | `0/4` |
| registered no-return passes | `4/4` |

The clean Release execution took approximately 85 minutes. The shell watchdog
that launched it expired, but the original child process remained live and was
monitored to its own completion. It was not restarted.

## 3. What survived the larger environment

| unbound arm | negative-core onset | late radius-8 minimum | late radius-8 maximum | first ticks at `R=8,12,16,24` |
|---|---:|---:|---:|---|
| face `<001>` | `80` | `2.224e-3` | `2.692e-3` | `22,48,75,130` |
| edge `<01-1>` | `96` | `1.847e-3` | `2.263e-3` | `22,48,75,130` |
| body `<111>`, either polarity | `115` | `1.494e-3` | `1.800e-3` | `22,48,75,130` |

Every unbound arm remains continuously graph-inside with negative pair energy
for at least the registered 64-tick tail. Every late radius-eight near field is
well above the `5e-4` floor and has dynamic range below four. The two body
polarity histories remain exactly conjugate. The bound face control remains
bound and negative through tick 184.

This is stronger than FTD-0739's 136-tick statement: the localized core and
near field survive the held-out larger causal buffer and longer precontact
horizon. It is not an invariant-basin, asymptotic-stability, or particle claim.

## 4. Where environmental closure failed

The largest exterior field energies across the four unbound histories are:

| radius | maximum outside energy | locked threshold | result |
|---:|---:|---:|---|
| `8` | `3.713e-4` | `1e-8` | crossed |
| `12` | `1.936e-4` | `1e-8` | crossed |
| `16` | `6.682e-5` | `1e-8` | crossed |
| `24` | `5.587e-6` | `1e-8` | crossed |
| `32` | `7.983e-9` | `1e-8` | not crossed |
| `48` | `5.165e-18` | `1e-8` | not crossed |

The failure is not loss of the core, collapse of the near field, source leakage,
or measured inward return. It is failure to complete the registered spatial
ladder within the precontact time budget.

As a post-result descriptive diagnostic only, the common first-passage ticks
`(R,t)=(8,22),(12,48),(16,75),(24,130)` give the least-squares line

\[
t_{\rm thr}(R)\simeq -32.71+6.764R.
\]

It places the `R=32` threshold near tick `184` and the `R=48` threshold near
tick `292`. This is a threshold-front description, not a signal-speed, cone,
dispersion, or continuum fit. It explains why the radius-32 miss is marginal
while radius 48 is causally out of reach for `T=184`. The current periodic
precontact condition therefore cannot test persistence after radius-48 arrival.

No threshold is relaxed and no failed gate is reclassified.

## 5. Exactness, source separation, and reversibility

| diagnostic | measured maximum | gate |
|---|---:|---:|
| common-action residual | `5.147e-14` | `1e-10` |
| total-energy residual | `4.890e-15` | `1e-8` |
| recoil defect | `2.686e-14` | `1e-9` |
| causal-speed excess | `0` | `1e-12` |
| regional-ledger residual | `9.664e-14` | `1e-10` |
| source exchange outside registered support | `1.510e-16` | `1e-10` |
| pair-plus-field endpoint defect | `9.457e-15` | `1e-8` |
| state-only inverse recovery | `4.447e-11` | `1e-8` |

All deposited current remains within radius three. At every shell that crosses
the threshold, cumulative outward transport has no registered inward increment.
For radii 32 and 48 the no-return statement is vacuous because no first passage
occurs; E5 has priority over E6.

## 6. Record defect and independent certificate

The raw summary contains one non-standard bare `inf` token in
`late_inside_8_minimum` for the bound control. That field is initialized only
for the unbound near-field classifier and is unused for the bound arm. All
physics-gate data are finite in the CSV, and all reverse/inverse fields used by
E0 are finite in the summary.

The raw summary is preserved byte-for-byte and hashed. The independent proof
requires exactly one occurrence in exactly that bound-only slot, replaces it
with JSON `null` in memory, and then parses the result. No persisted datum or
verdict is rewritten. This is a serialization defect to fix in the next runner,
not a physics-gate failure under the enumerated E0 conditions.

[`proof_finite_support_environmental_closure.py`](../../../../../scripts/proofs/proof_finite_support_environmental_closure.py)
independently reconstructs all 925 forward rows, five inverse summaries, the
frozen FTD-0739 prefix comparison, every gate, and the ordered verdict.

Result: **`131/131 PASS`**.  
Proof SHA-256:
`C1256E515526121516D7FC6B87AFC5969466426D8AD0D93A94128D581B97F35B`.

| result artifact | SHA-256 |
|---|---|
| CSV | `58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C` |
| raw summary | `B6325EFBC06F486F6135C20E97F78B50752E637138C7B277AC513ED2E761DC2A` |

## 7. Ontological consequence

FTD-0745 rules out one overly strong environmental-closure conjunction. It
does not rule out the matter candidate. The selected `(s,C,F)` action has now
shown, on two causally embedded volumes, that compact initial data can form and
retain a localized negative core plus a noncollapsing near field while exporting
energy source-freely and monotonically across every shell the front actually
reaches.

The honest ontology is therefore:

> The present object candidate is a persistent localized relational core with
> a dynamically maintained near field and a detached outward field component.
> Its closure against an arbitrarily extended environment is not established.

The data do not determine whether the outer component is radiation, a wake, or
generic dispersing background. They only establish a source-separated outward
energy transport record through radius 24 at this horizon.

## 8. Next admissible gate

M3 is not licensed because the registered M2 conjunction failed. The next
admissible work is a fresh, causal-horizon-matched M2 version that:

1. predicts its outer-shell horizon from the frozen FTD-0745 inner-shell record;
2. chooses `L` so every requested shell can arrive before periodic contact;
3. freezes a valid finite/null JSON serializer before execution;
4. separates arrival from post-arrival persistence rather than requiring a
   physically unreachable shell in the same horizon; and
5. preserves the same action, preparation, exactness, source, control,
   polarity, and inverse gates.

This is a new registered candidate, not a tolerance repair or rerun of FTD-0745.

