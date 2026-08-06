# Pre-registration — Common Moore worldline action (FTD-0578)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-26  
**Parents:** FTD-0478, FTD-0542, FTD-0543, FTD-0550--0555,
FTD-0574--0577.  
**Production changes permitted:** none. Observer code, exact proofs, tests,
theorem, audit, and documentation reconciliation are permitted.

## 1. Question

FTD-0577 supplies an exact local central current for a noncardinal Moore-coated
ternary carrier. This campaign asks whether that density/current pair belongs
to one common native Hodge worldline action, whether the reciprocal gather is
the adjoint of the same coupling, whether the time-exact action supplies the
FTD-0576 energy-centered source, and whether its isolated point carrier is
free of lattice self-force.

No force amplification, self-field subtraction, wider coat, tolerance change,
production toggle, scenario, or fitted coefficient is permitted.

## 2. Frozen coated spacetime current

Let `rho_CIC(X(t))` and the instantaneous oriented-face current `k_f(t)` be
the FTD-0478 straight-path density/current, with

```text
d rho_CIC/dt + d_f k_f = 0.
```

Use the FTD-0577 operators

```text
B_i=(T_i^-1+2+T_i)/4,
B_M=B_x B_y B_z,
A_i=(1+T_i^-1)/2,

rho_M=B_M rho_CIC,
q_i=A_i product_(j!=i) B_j k_f,i.                 (1)
```

Then the native central continuity equation must hold at every regular path
point and after integration:

```text
d rho_M/dt + D_c q=0.                             (2)
```

For temporal hats `w_0=1-t`, `w_1=t`, define

```text
T_a=integral_0^1 w_a rho_M dt,
Q_a=integral_0^1 w_a q dt,
T=T_0+T_1,
Q=Q_0+Q_1.                                       (3)
```

Integration of (2) must give the exact split identities

```text
D_c Q_0=rho_0-T,
D_c Q_1=T-rho_1.                                 (4)
```

`Q` must equal the aggregate FTD-0577 bridged current. The temporal splits are
transaction records derived from the straight path, not new persistent
primitive variables.

## 3. Common energy-coordinate action

Use the unique FTD-0576 work coordinate `R=J-W/2`. For a linear field history

```text
R(t)=(1-t)R_0+t R_1,
```

define the candidate common interaction

```text
I_M=G_C integral_0^1 [<rho_M(t),D R(t)>
                      +<q(t),C R(t)>] dt

   =G_C[<T_0,D R_0>+<T_1,D R_1>
        +<Q_0,C R_0>+<Q_1,C R_1>].               (5)
```

Because `D^T=-G` and `C^T=C`, its endpoint field derivatives are

```text
S_0=-G_C G T_0+G_C C Q_0,
S_1=-G_C G T_1+G_C C Q_1.                        (6)
```

Their aggregate is

```text
S_0+S_1=-G_C G T+G_C C Q.                        (7)
```

The deposited action must equal the straight-orbit action obtained by
gathering `D R` through the adjoint scalar coat and `C R` through the adjoint
current bridge. Thus source deposition and reciprocal path gather are
derivatives of (5), not independent rules. The path Euler--Lagrange force has
the Hodge Lorentz form; its magnetic term performs zero scalar work.

The use of `R` is the FTD-0576 energy-coordinate lift. It is not a claim that
production presently evaluates (5).

## 4. Energy-centering discriminator

FTD-0576 exact endpoint-energy exchange uses

```text
rho_bar=(rho_0+rho_1)/2,
S_E=-G_C G rho_bar+G_C C Q.                       (8)
```

The time-exact action (5) instead supplies (7). Therefore its aggregate source
matches the FTD-0576 source if and only if

```text
T=rho_bar.                                       (9)
```

For a complete one-cell axial CIC path, (9) holds exactly. For complete edge
and body-diagonal paths it fails before and after the Moore coat. For a
positive path starting at the origin, the uncoated temporal averages are

```text
edge:
T_00=T_11=1/3, T_10=T_01=1/6,
rho_bar_00=rho_bar_11=1/2;

body diagonal:
T_000=T_111=1/4,
the other six corner weights=1/12,
rho_bar_000=rho_bar_111=1/2.                     (10)
```

After `B_M`, require the exact rational norms

```text
||T-rho_bar||_2^2 = 0          axial,
||T-rho_bar||_2^2 = 1/1536     edge diagonal,
||T-rho_bar||_2^2 = 5/3072     body diagonal.     (11)
```

Signed directions, both polarities, translations, and cubic rotations must
preserve the classification. An endpoint-trapezoid correction, temporal
variation, or separate discrete-gradient energy equation is a selected
extension; it is not the unmodified time-exact action (5).

## 5. Static Peierls self-force discriminator

Use the exact FTD-0575 static Hodge kernel

```text
R(k)=3 sum_i sin(k_i)^2/M(k),
M(k)=4-(2/3)sum_i cos(k_i)
       -(2/3)sum_(i<j) cos(k_i)cos(k_j).          (12)
```

For a carrier displaced by `r in [0,1]` along axis `i`,

```text
rho_hat_r(k)=B_M(k)[(1-r)+r exp(-i k_i)],
B_M(k)=product_j cos^2(k_j/2).                    (13)
```

Eliminating the static field from the same common action gives

```text
V_self(r)=-(G_C^2/(2L^3)) sum_k R(k)|rho_hat_r(k)|^2
         =V_self(0)+C_i r(1-r),                  (14)

C_i=(G_C^2/L^3) sum_k R(k)B_M(k)^2(1-cos k_i)>0. (15)
```

Thus

```text
Delta V_Peierls=C_i/4,
F_self(r)=-C_i(1-2r)                             (16)
```

inside a cell. The force is polarity-even, pins toward integer sites, and
vanishes only at reflection-symmetric points. Exact energy conservation makes
it conservative; it does not cancel it.

## 6. Registered arms

- exact symbolic/rational checks of equations (1)--(16);
- `L in {17,33}`, both polarities, and all 26 signed one-cell Moore paths;
- exact 4-point piecewise Gauss--Legendre evaluation of the temporal moments;
- 104 aggregate/split continuity and current-reconstruction arms;
- deterministic `R_0,R_1` fixtures for deposited-action/orbit-gather equality
  and endpoint field-adjoint checks;
- three translations and all 24 proper-cubic rotations of a generic path;
- axial/edge/body time-centering classification and exact rational norms;
- three axes, both polarities, both volumes, and
  `r in {0,1/8,1/4,3/8,1/2,5/8,3/4,7/8,1}` for the static self-energy law;
- production hash, default, toggle, and scenario non-change checks.

All floating continuity, split, action-adjoint, gather-adjoint, covariance,
and Peierls-law residuals must be `<=1e-12`. Every Peierls coefficient and
half-cell barrier must exceed `1e-8`. Axial time-centering must close below
`1e-12`; every diagonal time-centering norm must exceed `1e-6` and agree with
(11) below `1e-12`.

## 7. Outcome map

Positive registered verdict:

```text
COMMON_MOORE_WORLDLINE_ACTION_DERIVED_ENERGY_CENTERING_MISMATCH_PEIERLS_PINNED
```

It establishes a common action and reciprocal adjoint gather, but closes the
unmodified compact bare-polarity action as the FTD-0481 freely mobile law for
two independent reasons:

1. generic multi-axis time-exact sources do not equal the FTD-0576
   endpoint-energy-centered source;
2. the same reciprocal action contains a nonzero point-carrier Peierls
   self-force.

This does not forbid a selected energy-centered correction, a multistage
implicit action, integer hopping, or an extended native excitation whose
relative Peierls barrier scales away. It licenses no production toggle,
scenario, particle, Coulomb, electromagnetic, Lorentz, or unitarity claim.

## 8. Frozen production provenance

The implementation must verify these SHA-256 hashes and must not edit the
files:

```text
phase_read.cpp                  D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8
phase_write.cpp                 2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4
field_operators.h               25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48
native_energy_contract.h        3DB8F2DC573E7F4A87E17409878915E7B5A52CE1673713998C544516E0175621
```
