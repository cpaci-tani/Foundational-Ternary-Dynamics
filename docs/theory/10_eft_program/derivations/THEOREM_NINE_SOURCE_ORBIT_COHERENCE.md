# Theorem — Nine-Source Removal-Time Orbit Coherence

**FTD ID:** FTD-0592  
**Status:** `[THEOREM — NINE-SOURCE FIRST-EVENT COROLLARY]` +
`[NUMERICAL FACT — EXHAUSTIVE FOUR-VOLUME PARTITION MAXIMA]` +
`[CLOSED NEGATIVE — ENDOGENOUS AUTOCATALYSIS FOR N <= 9]` +
`[OPEN — N >= 10]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_9_CLOSED_BY_ORBIT_COHERENCE`

## 1. Scope

Adopt the frozen FTD-0590 sector and theorem. There are nine distinct,
stationary, initially present ternary sources at arbitrary sites and with
arbitrary signs. Each source may be removed at most once at an arbitrary
integer tick. The claim concerns the history before a hypothetical first
descendant genesis event on the registered odd periodic quotients
`L={9,17,33,65}`.

No source geometry, polarity assignment, removal schedule, observation time,
or observation site is selected or searched.

## 2. Parent inequality

FTD-0590 proves that if `r` original sources have been removed, then

\[
 |J(x,n)|\le H_L^{\rm orb}(N,r)
 =C_L\sqrt{N-r}
 +Q_L\sqrt{r+\mu_Lr(r-1)}.
\]

Here `C_L` is the exact common-step coefficient from FTD-0588, while `Q_L`
and `mu_L` are the pulse operator norm and exhaustive cubic-orbit coherence
constant from FTD-0590. The inequality is uniform in every source and history
choice listed above.

For `N=9`, the registered finite maximum is

\[
 \boxed{
 H_L^{(9)}=\max_{0\le r\le9}
 \left[C_L\sqrt{9-r}
 +Q_L\sqrt{r+\mu_Lr(r-1)}\right].}
\]

## 3. Exhaustive removal-partition evaluation

All ten integer partitions `r=0,...,9` were evaluated on every registered
quotient. No stronger shared-eigenshell relaxation was used.

| `L` | `Q_L` | `mu_L` | `C_L` | maximizing `r` | `H_L^(9)` | `K_GENESIS-H_L^(9)` |
|---:|---:|---:|---:|---:|---:|---:|
| 9 | 0.21340660233910125 | 0.36102817687951227 | 0.30397065730643719 | 8 | 1.4375912274277709 | 0.078794831724206871 |
| 17 | 0.21677610473116635 | 0.36250597734262191 | 0.30909222934825492 | 8 | 1.4622990285954853 | 0.054087030556492444 |
| 33 | 0.21844645449354122 | 0.36267617904631827 | 0.31182601851585356 | 8 | 1.4741144289304433 | 0.042271630221534462 |
| 65 | 0.21929447438975708 | 0.36273662797281120 | 0.31324294475519981 | 8 | 1.4801131737725799 | 0.036272885379397879 |

The production threshold is

\[
 K_{\rm GENESIS}=1.5163860591519780.
\]

Every maximum is strict. The least margin is the `L=65`, `r=8` value
`0.036272885379397879`.

## 4. First-event corollary

Assume a first descendant genesis event occurs from nine original sources.
Immediately before that event, every possible field source is still an
original stationary site with either its common step history or one finite
removal pulse. For some `r`, FTD-0590 and the exhaustive maximum give

\[
 |J|\le H_L^{\rm orb}(9,r)
 \le H_L^{(9)}<K_{\rm GENESIS}.
\]

The production genesis predicate is therefore false, contradicting the
assumed first event. FTD-0591 already closes every smaller count. Hence no
first descendant event exists in the frozen sector for `N<=9` on any
registered quotient.

## 5. Verification and boundary

- preregistration SHA-256:
  `DDAA7FC084C3F8F146E722F15E1089FDDA83D095EB5C55D2B31823A20BD41DE8`;
- every one of the 40 registered `(L,r)` partitions evaluated;
- exact parent mode/orbit coverage retained;
- independent Python reconstruction: 126/126 PASS;
- geometry and schedule searches: none;
- production/default/toggle/scenario changes: none.

This is a theorem about the inability of the frozen source histories to cross
one threshold. It is not a positive genesis mechanism, a particle model, or a
reciprocal matter-field law. `N=10` is the next unevaluated integer and
requires a separate preregistration. FTD-0593 performs that evaluation; the
ordinary orbit bound is inconclusive and supplies no witness history.
