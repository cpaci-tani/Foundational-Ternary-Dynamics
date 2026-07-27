# Theorem — Eight-Source Removal-Time Orbit Coherence

**FTD ID:** FTD-0591  
**Status:** `[THEOREM — EIGHT-SOURCE FIRST-EVENT COROLLARY]` +
`[NUMERICAL FACT — EXHAUSTIVE FOUR-VOLUME PARTITION MAXIMA]` +
`[CLOSED NEGATIVE — ENDOGENOUS AUTOCATALYSIS FOR N <= 8]` +
`[BOUNDARY SUPERSEDED BY FTD-0592]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_8_CLOSED_BY_ORBIT_COHERENCE`

## 1. Scope

Adopt the frozen FTD-0590 sector and theorem. There are eight distinct,
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

For `N=8`, the only remaining operation is the registered finite maximum

\[
 \boxed{
 H_L^{(8)}=\max_{0\le r\le8}
 \left[C_L\sqrt{8-r}
 +Q_L\sqrt{r+\mu_Lr(r-1)}\right].}
\]

## 3. Exhaustive removal-partition evaluation

All nine integer partitions `r=0,...,8` were evaluated on every registered
quotient. No stronger eigenshell relaxation was used.

| `L` | `Q_L` | `mu_L` | `C_L` | maximizing `r` | `H_L^(8)` | `K_GENESIS-H_L^(8)` |
|---:|---:|---:|---:|---:|---:|---:|
| 9 | 0.21340660233910125 | 0.36102817687951227 | 0.30397065730643719 | 7 | 1.3086417854242818 | 0.20774427372769599 |
| 17 | 0.21677610473116635 | 0.36250597734262191 | 0.30909222934825492 | 7 | 1.3310542251949447 | 0.18533183395703312 |
| 33 | 0.21844645449354122 | 0.36267617904631827 | 0.31182601851585356 | 7 | 1.3418282589109189 | 0.17455780024105882 |
| 65 | 0.21929447438975708 | 0.36273662797281120 | 0.31324294475519981 | 7 | 1.3473027423603405 | 0.16908331679163724 |

The production threshold is

\[
 K_{\rm GENESIS}=1.5163860591519780.
\]

Every maximum is strict. The least margin is the `L=65`, `r=7` value
`0.16908331679163724`.

## 4. First-event corollary

Assume a first descendant genesis event occurs from `N<=8` original sources.
Immediately before that event, every possible field source is still an
original stationary site with either its common step history or one finite
removal pulse. For some `r`, FTD-0590 and the exhaustive maximum give

\[
 |J|\le H_L^{\rm orb}(N,r)
 \le H_L^{(8)}<K_{\rm GENESIS}.
\]

The production genesis predicate is therefore false, contradicting the
assumed first event. Hence no first descendant event exists in the frozen
sector for `N<=8` on any registered quotient.

## 5. Verification and boundary

- preregistration SHA-256:
  `F6ED8183765BCCC29427DFFBCA6074D916FEDBF7D97B557F38DD3405721D4F70`;
- every one of the 36 registered `(L,r)` partitions evaluated;
- exact parent mode/orbit coverage retained;
- independent Python reconstruction: 122/122 PASS;
- geometry and schedule searches: none;
- production/default/toggle/scenario changes: none.

This is a theorem about the inability of the frozen source histories to cross
one threshold. It is not a positive genesis mechanism, a particle model, or a
reciprocal matter-field law. FTD-0592 subsequently evaluated and closed `N=9`;
`N=10` is the live boundary.
