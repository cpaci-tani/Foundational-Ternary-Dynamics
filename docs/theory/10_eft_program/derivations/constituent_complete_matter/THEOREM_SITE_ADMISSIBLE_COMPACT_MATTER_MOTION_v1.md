# FTD-0607 — Site-admissible compact matter motion v1

**Status:** `[MEASURED — FIVE SITE-INTERIOR STATIC CORES]` +
`[NUMERICALLY UNRESOLVED — CAMPAIGN COVERAGE AND MOTION]`
**Protocol:**
[`PREREG_SITE_ADMISSIBLE_COMPACT_MATTER_MOTION_v1.md`](../../preregistrations/constituent_complete_matter/PREREG_SITE_ADMISSIBLE_COMPACT_MATTER_MOTION_v1.md),
prefix SHA-256 `CA37FB9700A2416FE293B26A903A9DCA5233091C215E0AEB83D92BA802D871E9`
**Production status:** unchanged

## 1. Registered question

The campaign imposed the ternary transaction's one-label-per-site capacity
inside the `SO(3) x strain` objective rather than checking it after continuous
minimization. It asked for a qualified compact static core at every one of 32
translation phases and allowed autonomous motion only from the locked
phase-zero result.

The constraint was hard: duplicate anchors were rejected, not assigned an
adjustable penalty. Every reported minimum also required a `5e-3` distance
from every remainder-chart face, six positive tangent modes, a gradient below
`5e-7`, and an independently rebuilt minimum-energy Gauss field.

## 2. Static result

All 24 proper-cubic starts were admissible at every phase, and the optimizer
termination threshold passed throughout. The repeatability cluster gate,
however, passed at only ten phases. Five phases passed every static gate:

```text
phase indices 14, 15, 16, 17, 26
fractional phases 0.4375, 0.46875, 0.5, 0.53125, 0.8125
```

Across those five qualified cores:

- the minimum chart margin is `5.8076451586e-3`;
- the maximum tangent-gradient norm is `1.0703866613e-7`;
- the minimum tangent-Hessian eigenvalue is `6.3106137255e-4`;
- every tangent Hessian has six positive modes;
- the maximum direct-field gate is `8.7872874094e-16`.

This is direct numerical evidence that finite one-site capacity does not, by
itself, eliminate the selected compact matter family. Site-interior stable
cores exist in the registered continuous constituent variables.

## 3. Why the motion question remains unanswered

The phase-zero optimizer returned a minimum only `2.22e-14` from a site-chart
face, far inside the prohibited `5e-3` margin. Its best-energy cluster also
contained only one registered start. It therefore was not qualified and could
not be selected for motion.

The locked protocol required phase zero specifically and required qualified
coverage across the static campaign before a negative or constructive motion
verdict could apply. Both autonomous-motion records were consequently left
empty. No forward step, reverse step, hop count, trajectory, energy drift, or
integer-translation covariance result was measured.

The registered verdict is

```text
SITE_ADMISSIBLE_COMPACT_MATTER_NUMERICALLY_UNRESOLVED
```

It is not a dynamics failure and not a no-go for compact matter.

## 4. Ontological consequence

FTD-0606's duplicate-anchor observation was a genuine mismatch between its
unconstrained continuous minima and the site transaction, but it was not
evidence that all nearby site-valid compact states are absent. FTD-0607 finds
five such states.

The next clean discriminator is therefore dynamical: lock one of the already
qualified interior phases independently of motion outcome, initialize the
same minimum-energy field, and execute the unchanged common-action transaction
through the first site boundary. This tests whether a valid material pattern
can transport its organization between site charts. It adds no new primitive
and licenses no physical particle, production toggle, electromagnetic
ontology, pole, Lorentz, or unitarity claim.

