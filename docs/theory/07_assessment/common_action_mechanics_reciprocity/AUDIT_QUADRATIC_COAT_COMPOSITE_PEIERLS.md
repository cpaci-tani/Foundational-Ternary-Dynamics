# AUDIT — Quadratic-coat neutral-composite Peierls gate

**Date:** 2026-07-26  
**Identifier:** `FTD-0553`  
**Status:** `[THEOREM — RIGID INTEGER-OFFSET COMPOSITE PEIERLS LAW] +
[NUMERICAL FACT — LOCKED FINITE-VOLUME VERIFICATION] + [CLOSED NEGATIVE —
RIGID NEUTRAL-COMPOSITE SELF-FORCE CURE]`  
**Verdict:** `RIGID_NEUTRAL_COMPOSITE_PEIERLS_OBSTRUCTION`  
**Pre-registration:**
[`PREREG_QUADRATIC_COAT_NEUTRAL_COMPOSITE_PEIERLS_v1.md`](../10_eft_program/preregistrations/PREREG_QUADRATIC_COAT_NEUTRAL_COMPOSITE_PEIERLS_v1.md)  
**Theorem:**
[`THEOREM_QUADRATIC_COAT_COMPOSITE_PEIERLS.md`](../10_eft_program/derivations/THEOREM_QUADRATIC_COAT_COMPOSITE_PEIERLS.md)  
**Run of record:** `engine/results/ftd_0553/windows_msvc_cpu.json`

## Result

The exact spectral law and independent real-space Poisson reconstruction agree
for every locked localized neutral structure:

```text
registered arms                         96
volumes                                 L=17,33
minimum positive spectral terms         4284
maximum Poisson iterations              119
worst identity residual                 9.9995086064059549e-14
worst polarity residual                 0
worst integer-translation residual      2.6671373443143409e-17
worst cyclic-rotation residual          1.2338220722885040e-16
smallest Peierls coefficient            0.0011781204382135288
smallest barrier                        7.3632527388345549e-05
largest barrier                         4.3065509757911038e-04
failures                                0
```

The smallest barrier exceeds the locked `1e-12` cancellation gate by more
than seven orders of magnitude. The residual is instead limited by the locked
`1e-13` Poisson solve. No admissible cancellation witness appears.

## Why this is structural

For a common subcell translation, the coat transform satisfies exactly

```text
|B(k,f)|^2=|B(k,0)|^2+(1-cos k)^2(f^4-f^2/2).
```

The resulting coefficient is a sum of nonnegative Fourier terms. It can vanish
only when the primitive composite source is invariant along the translated
axis. A nonzero localized source is not. Global polarity reversal changes
`A(k)` by a sign and therefore leaves the energy and force unchanged, exactly
as measured.

The summed exact face current also closes

```text
Delta U+beta<Ebar,K>=0
```

below the registered tolerance. This is a conservative Peierls force, not
energy drift, current leakage, broken Gauss transport, or a covariance bug.

## Verdict and program consequence

Neutrality cancels the zero mode but not the nonzero lattice aliases. It does
not rescue the bare-polarity transaction by itself. The rigid distinct-site
neutral-composite carrier is closed negative for the compact quadratic coat.

The FTD-0481 toggle, FTD-0482 scenario, and FTD-0483 infrared claims remain
unlicensed. An internally deforming carrier or noncompact band-limited coat is
a new registered model branch, not a repair implied by this result. No
production state, default, force, phase, toggle, scenario, or tolerance changed.

## Reproducibility

- test: `quadratic_coat_composite_peierls`, `96` registered arms;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- focused CTest: `1/1` passed in `12.40 s`;
- preregistration SHA256:
  `250FF3639823DE3218B4F5B024A6976531B606F526BD2FA708E9B1B875D1AFC8`;
- header SHA256:
  `2ED6831AC6575DD6D8A3F7B158A49EADE514492A8E8BBD793E9CF7942CD98274`;
- source SHA256:
  `99CEDEB7A02EFCBC92DFA617F8A34958A3187BAC5BCC3C16C00ED35931D4BB99`;
- test SHA256:
  `1D24C2C39133BE8EAE14FA268689F2533985C2F4A29B7BD5035DF7F2FFA4C74D`.
