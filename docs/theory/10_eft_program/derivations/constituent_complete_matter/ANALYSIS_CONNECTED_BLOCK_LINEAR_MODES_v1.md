# Analysis — FTD-0629 connected-block linear modes

**Status:** `[SELECTED ACTION] + [DERIVED FINITE-AMPLITUDE SECANT PREDICTION] + [MEASURED —
FOUR SYMMETRY-PRESERVING CLASSICAL RESPONSES] + [OPEN — FULL TANGENT/FIELD
SPECTRUM / PHYSICAL POLE]`  
**Verdict:** `CONNECTED_BLOCK_ADIABATIC_LINEAR_MODES_CONSTRUCTIVE`  
**Protocol SHA-256:**
`BF823BB629BFAB7FA385E39AB83E4BDCC2DCA3E857EE424FAF65C3280898CB4F`

> **Correction after FTD-0634--0637:** the FTD-0628 reduced Hessian stencil
> crosses quadratic-B-spline knots. The registered trajectories and recurrence
> measurements remain valid finite-amplitude reversible responses, but they
> are not qualified infinitesimal normal modes. Analytic small-amplitude modes
> must be rebuilt about the FTD-0638 exact center.

## 1. Registered secant prediction

FTD-0628 supplies the static four-coordinate Hessian `H`. The unchanged
production dispersion fixes the small-velocity kinetic metric for the
symmetry coordinates:

\[
M=M_{\rm inertial}\,\operatorname{diag}(8,8,16,16),
\qquad M_{\rm inertial}=0.511.
\]

The generalized eigenproblem

\[
H v_m=\omega_m^2 M v_m
\]

has four positive eigenvalues. Because the transaction is an implicit
midpoint/discrete-gradient step, the linear per-tick phase prediction is

\[
\Omega_m=2\arctan(\omega_m/2).
\]

| mode | `omega^2` | predicted `Omega` | predicted period |
|---:|---:|---:|---:|
| 0 | 4.97720345 | 1.67986634 | 3.74028884 ticks |
| 1 | 22.45450515 | 2.34283035 | 2.68187806 ticks |
| 2 | 61.30026622 | 2.64139756 | 2.37873518 ticks |
| 3 | 67.79199412 | 2.66500817 | 2.35766080 ticks |

The implicit-time map is essential. The continuous frequencies of the upper
three modes exceed or approach the naive discrete Nyquist scale; the midpoint
map compresses every finite positive `omega` into a phase below `pi`.

## 2. Measured response

Each mass-normalized eigenvector was displaced separately at registered
amplitudes and redressed from scratch. The fixed recurrence estimator returns:

| mode | measured `Omega` (`A=1e-4,+`) | relative error | maximum leakage |
|---:|---:|---:|---:|
| 0 | 1.68275831 | 0.1722% | 0.0205% |
| 1 | 2.34293262 | 0.00437% | 0.0971% |
| 2 | 2.64145146 | 0.00204% | 0.1866% |
| 3 | 2.66501961 | 0.000429% | 0.1179% |

All 16 arms pass. The worst aggregate diagnostics are:

- generalized-mode orthogonality residual `5.56e-16`;
- amplitude residual `2.53e-4`;
- signed-trajectory residual `1.03e-4`;
- cyclic trajectory residual `7.28e-11`;
- common-action residual `1.94e-11`;
- energy drift `8.44e-15`;
- state-only recovery `5.13e-13`.

Initial excess energy scales quadratically between `A=1e-4` and `2e-4`, and
the measured phase is amplitude-independent within the locked gate. The
negative-amplitude arms mirror the positive arms, and cyclic x/y copies agree.

## 3. Relation to the earlier breathing record

At 256 samples the new linear prediction places mode 0 near DFT bin `68.44`
and modes 2/3 near `107.62/108.58`. FTD-0627 had broad dominant bands around
`68/69` and `107/109`. This is descriptively consistent with the rigid start
being a mixture of the newly isolated modes. It is not counted as an
independent FTD-0629 gate because that earlier spectrum was known when this
campaign was designed.

## 4. Ontological consequence

Within the selected research action, the dressed connected object now has:

1. a stationary constituent-plus-field configuration;
2. a positive symmetry-reduced energy basin;
3. four separately excitable, weakly mixing, reversible internal oscillations;
4. predictable discrete-time phases fixed by its static stiffness and existing
   constituent inertia.

This strengthens “matter as a dynamical pattern.” The pattern is not merely
persistent; it has a local state space with reproducible internal response.
Its dressing participates self-consistently without destroying the adiabatic
mode prediction at the tested amplitudes.

## 5. Limits

These are classical, symmetry-preserving shape modes in one finite-volume
selected model. The frequencies depend on the imposed `M_INERTIAL=0.511` and
selected binding stiffness `kappa=1`; they are not parameter-free particle
mass predictions. The campaign does not diagonalize:

- the other 44 constituent tangent directions;
- independent dynamical face/edge field perturbations;
- translation, rotation, shear, or symmetry-breaking sectors in a complete
  irreducible basis;
- the infinite-volume transfer operator.

Consequently it does not yet establish full stability, a physical spectral
pole, quantum energy levels, a clock standard, or a particle identity.

## 6. Next discriminator

Before boosting the state, construct the complete 48-coordinate adiabatic
matter Hessian at the refined dressing. Remove no directions post hoc. Classify
its cubic irreducible sectors, identify every negative/zero/positive direction,
and explicitly separate pinned translations from internal modes. Only if the
full matter tangent is nonnegative should the program add independent field
perturbations and then boost the dressed object.
