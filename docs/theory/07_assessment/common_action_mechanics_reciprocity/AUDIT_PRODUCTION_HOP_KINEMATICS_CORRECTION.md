# AUDIT — Production hop kinematics correction

**Date:** 2026-07-24  
**Identifier:** `FTD-0450`  
**Status:** `[CORRECTION — FTD-0444 PRODUCTION-DIAGNOSTIC CLAIM RETRACTED]` + `[THEOREM — EXACT FACTOR-THREE MISMATCH]` + `[CONSTRUCTIVE EXAMPLE — CORRECTED SELECTED MAP]`  
**Verdict:** `PRODUCTION_KINEMATICS_CORRECTS_SELECTED_MAP`  
**Pre-registration:** [`PREREG_PRODUCTION_HOP_KINEMATICS_CORRECTION_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_PRODUCTION_HOP_KINEMATICS_CORRECTION_v1.md)  
**Run of record:** `engine/results/ftd_0450/windows_msvc_cpu.csv`

## 1. Defect

FTD-0444 described

$$
E_{old}(p)=\sqrt{M_{inertial}^2+|p|^2/C_{speed}^2}
$$

as the production flat particle diagnostic. That identification is false.
Production uses

$$
E=\gamma E_{rest},\qquad
p=\gamma M_{inertial}v,\qquad
E_{rest}=M_{inertial}C_{speed}^2,
$$

so

$$
\boxed{E(p)=\sqrt{E_{rest}^2+C_{speed}^2|p|^2}}.
$$

The old expression has both the wrong rest term and the reciprocal power of
`C_SPEED` multiplying momentum.

## 2. Exact factor-three mismatch

In the engine `C_SPEED^2=1/3`. With `p=gamma M v` and
`beta^2=v^2/C_SPEED^2=3v^2`,

$$
E_{old}^2
=M^2+\frac{\gamma^2M^2v^2}{C^2}
=M^2(1+\gamma^2\beta^2)
=\gamma^2M^2.
$$

Therefore `E_old=gamma M`. Production instead gives

$$
E_{production}=\gamma M C^2=\frac{\gamma M}{3},
$$

and hence

$$
\boxed{E_{old}=3E_{production}}.
$$

The locked all-direction campaign measures relative mismatch
`|E_old-E_production|/E_production = 2.0000000000000004` identically across
all 26 cases, exactly as the algebra predicts.

## 3. Corrected constructive map

The successor helper uses the production dispersion, converts
`v -> p=gamma M v`, and reconstructs `v=p C^2/E`. Energy and velocity
identities close to `2.77556e-17`.

Applying the same explicit preserved-transverse/same-longitudinal-branch
selection with work `1e-4` closes:

- forward and reverse work residual: `1.10182e-17`;
- momentum round trip: `2.94392e-17`;
- field-recoil balance: exactly zero;
- reconstructed-velocity energy: exactly zero residual.

This supplies a production-compatible selected finite map. It does not derive
the branch selection.

## 4. Status of FTD-0444

The FTD-0444 theorem that scalar work alone underdetermines force, momentum
direction, and local recoil is unaffected. Its force-family and support
counterexamples do not depend on the faulty dispersion.

The old selected map remains an algebraically reversible construction for its
synthetic energy function, but the claim that it used production flat
kinematics is retracted. FTD-0450 supersedes that example.

FTD-0447 separately derives the isolated force direction from cubic symmetry.
Neither result derives the finite momentum branch or a field Hamiltonian.

## 5. Reproducibility

- campaign SHA256: `1923d6cad6f913d9b99fd9a9110b4250b32c5d8bf7cb1bdbc5388aa9e9dabd8e`
- helper SHA256: `4fce830b79cd4590108b7fea28063b489b33cf3ca69925e5405043b78d1c2ebd`
- record SHA256: `33824de8daf6d7a0aeb56eed1bef938d9048845340b08602cf13bf35ca7c71d9`
- compiler: pinned MSVC `14.44.35207`, Release
- execution: algebraic observer, no production tick
- result: `PRODUCTION_KINEMATICS_CORRECTS_SELECTED_MAP`

No production dynamics were changed.
