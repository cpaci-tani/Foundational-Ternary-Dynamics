# FTD-0839 — `i`/Gamma/quartic-square split probe v1

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXACT CERTIFICATE 24/24]`  
**Date:** 2026-08-10  
**Scope:** exact source-locked operator, determinant, and square-field controls  
**Production impact:** none

## 1. Registered question

Does the primitive orientation law

\[
J^2=-I
\]

derive the lemniscatic ratio

\[
G^*=\frac{\Gamma(1/4)}{\Gamma(3/4)}
\]

without further spectral structure? Can the same complex square that supplies a
quartic energy retain the clockwise/counterclockwise distinction required by
that ratio? If not, which assumptions are doing the mathematical work, and
what exact type of gearbox remains missing?

## 2. Epistemic firewall

This is an exact discriminator, not a numerical search. It contains no fitted
parameter, tolerance, prime subset, near-miss criterion, or target-dependent
choice made after execution. Standard Hurwitz-zeta and Gamma identities may be
used as mathematical inputs, but their physical realization is not thereby
derived from the substrate.

The campaign must distinguish:

- what follows from `J^2=-I` alone;
- what follows only after selecting a circle, twisted boundary condition,
  chiral half-line, spectral origin, scale, operator order, and multiplicity;
- what follows from the complex square `U=psi^2`; and
- what the square necessarily forgets.

Passing the exact certificate cannot establish a native clock, physical
positive-frequency rule, substrate determinant, or coupling between the
unsquared orientation carrier and the squared quartic carrier.

## 3. Frozen source inputs

The script must fail closed unless these SHA-256 values match:

| Input | SHA-256 |
|---|---|
| `docs/theory/03_derivations/foundational_mechanics/DERIV_GSTAR_QUARTER_CONJUGACY.md` | `52196EDE252C4DF772C3943B8EEDB459B805AAA027E74548F3F779C4D74C6C33` |
| `docs/theory/03_derivations/foundational_mechanics/DERIV_GSTAR_FINITE_APPROX.md` | `F6002D358CE0F832ECBF6D6FE33E67F96BF0BAAEB22604CC1B2E85AF2FF5DBBE` |
| `docs/theory/01_reference/SPEC_FQCR.md` | `C840E0C63A098CA8DDEC6B9D558817B9767E752F173D1AE4C47E2AC3E2887C72` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md` | `779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md` | `2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD` |

## 4. Frozen mathematics

### 4.1 Orientation lift

Use the real complex structure

\[
J=\begin{pmatrix}0&-1\\1&0\end{pmatrix},\qquad
J^2=-I.
\]

Its eigenvalues are `+i` and `-i`. Quarter shifts arise only after adding the
twisted-circle domain

\[
\psi(\phi+2\pi)=J\psi(\phi),
\]

which gives `a=1/4` and `a=3/4` in the two eigensectors.

### 4.2 Chiral determinant and controls

For the selected positive half-line spectrum

\[
D_a=\{n+a:n\geq0\},
\]

use Lerch's identity

\[
\det_\zeta D_a=\frac{\sqrt{2\pi}}{\Gamma(a)}.
\]

The primary ratio is therefore `G*`. The locked controls are:

1. shift the origin from `n=0` to `n=1`;
2. use `r` identical copies;
3. square the positive operator;
4. apply a common spectral scale `c`; and
5. replace the chiral half-line by the orientation-blind full-line Laplacian
   with spectrum `{(n+a)^2:n in Z}`.

For the last control,

\[
\det_\zeta\Delta_a=4\sin^2(\pi a).
\]

### 4.3 Square field

Let `psi=x+iy` and `U=psi^2`. Test exactly

\[
|U|^2=|\psi|^4,
\]

and the collision

\[
(+i\psi)^2=(-i\psi)^2=-\psi^2.
\]

The two quarter twists must both map to the same half-twist `a=1/2`.

## 5. Frozen exact checks

The implementation must run exactly 24 checks:

1. all five frozen source hashes;
2. order-four oriented real complex structure;
3. eigenvalues `+i,-i`;
4. quarter shifts under the added twisted-circle boundary;
5. anchored chiral determinant ratio `G*`;
6. origin-shift control `G*/3`;
7. multiplicity control `G*^r`;
8. squared-operator control `G*^2`;
9. Hurwitz-zeta values at zero;
10. common-scale anomaly `c^(-1/2)G*`;
11. both full-line quarter determinants equal `2`;
12. full-line orientation-blind ratio `1`;
13. square-field quartic norm;
14. cubic force from quartic energy;
15. collision of the two oriented lifts under squaring;
16. doubling of both quarter twists to `1/2`;
17. half-twist half-line determinant `sqrt(2)`;
18. squared-sector ratio `1`;
19. symmetric-square identification of `J` and `-J`;
20. impossibility of recovering an orientation witness from the square image;
21. nonclosure of the primitive alphabet `{0,i}`;
22. closure of `{0,+1,+i,-1,-i}` under multiplication by `i`;
23. common-scale cancellation at equal finite truncation; and
24. the combined split-architecture discriminator.

No check may be removed, reinterpreted, or tolerance-relaxed after execution.

## 6. Locked implementation

```text
scripts/proofs/proof_i_gamma_square_gearbox_probe.py
```

Script SHA-256:
`65AEFE108A1A6CB1630D695FF31E4621B4007C9BCB6C9C253A6327FFC3030DD0`

After the pre-run hash of this protocol is entered in
`REF_PREREGISTER_MANIFEST.md`, run exactly:

```text
python scripts/proofs/proof_i_gamma_square_gearbox_probe.py
```

## 7. Outcomes

- **Outcome A — automatic unified gearbox:** all controls show that `J^2=-I`
  alone fixes the physical domain, chirality, scale, origin, operator order,
  and multiplicity, while the square retains the orientation needed for
  `G*`. This would license a candidate direct gearbox.
- **Outcome B — exact split architecture:** all 24 checks pass. Book that `i`
  fixes only the oriented quarter eigenphases; `G*` additionally requires the
  selected anchored chiral determinant; and `psi^2` supplies quartic energy
  while erasing the orientation. The lift-to-pair gearbox remains `[OPEN]`.
- **Outcome C — invalid:** any exact or source-hash check fails without
  establishing Outcome A. Book no theorem and repair under a new lock.

The expected result is Outcome B. That expectation is frozen before the run
and does not weaken the controls.

## 8. Recorded outcome

The frozen implementation ran once and returned `24/24 PASS`. Registered
Outcome B is selected:

```text
I_FORCES_ORIENTED_QUARTER_EIGENPHASES_ONLY
GSTAR_REQUIRES_TWISTED_DOMAIN_CHIRAL_HALF_LINE_SCALE_AND_ORIGIN
COMPLEX_SQUARE_SUPPLIES_QUARTIC_ENERGY_AND_ERASES_ORIENTATION
LIFT_TO_PAIR_GEARBOX_STATUS=OPEN
```

The conditional determinant identity and square-field quarticity are exact.
The full-line control returns ratio `1`; the square maps both quarter sectors
to the same half-twist. See
[`THEOREM_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md).
