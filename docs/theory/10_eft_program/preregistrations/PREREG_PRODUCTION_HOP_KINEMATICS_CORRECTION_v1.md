# PRE-REGISTRATION — Production hop kinematics correction v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0450`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent correction:** `FTD-0444` selected reversible map  
**Engine artifact:** `engine/tests/campaign_production_hop_kinematics_correction.cpp`  
**Campaign SHA256:** `1923d6cad6f913d9b99fd9a9110b4250b32c5d8bf7cb1bdbc5388aa9e9dabd8e`  
**Helper SHA256:** `4fce830b79cd4590108b7fea28063b489b33cf3ca69925e5405043b78d1c2ebd`

## 1. Defect

FTD-0444 called its selected energy convention the production flat diagnostic,
but registered

$$
E_{0444}(p)=\sqrt{M_{inertial}^2+|p|^2/C_{speed}^2}.
$$

Production actually defines

$$
E=\gamma E_{rest},\qquad p=\gamma M_{inertial}v,
\qquad E_{rest}=M_{inertial}C_{speed}^2,
$$

which implies

$$
E(p)^2=E_{rest}^2+C_{speed}^2|p|^2.
$$

The coefficient and rest term in FTD-0444 are both inconsistent with that
identity.

## 2. Frozen checks

For all 26 Moore directions, use velocity
`0.15 d_hat + 0.03 transverse_hat`:

- convert velocity to production momentum `gamma M v`;
- compare `flat_particle_energy(v^2)` with
  `sqrt(E_REST^2+C_SPEED^2 p^2)`;
- reconstruct velocity as `p C_SPEED^2/E`;
- compare the old FTD-0444 energy against production;
- apply corrected preserved-transverse/same-longitudinal-branch work
  `+1e-4`, then reverse displacement/work;
- verify energy, momentum round trip, reconstructed-velocity energy, and
  global recoil balance.

## 3. Locked gates

- production energy identities and velocity reconstruction `<=1e-13`;
- minimum old-convention relative energy mismatch `>=1.0`;
- corrected forward/reverse work residuals `<=1e-13`;
- corrected momentum round trip and recoil balance `<=1e-12`;
- all 26 updates valid.

## 4. Locked outcomes

- `PRODUCTION_KINEMATICS_CORRECTS_SELECTED_MAP`: all gates pass.
- `OLD_CONVENTION_MATCHES_PRODUCTION`: old mismatch gate fails while the
  corrected map remains reversible.
- `PROTOCOL_INVALID`: any other result.

## 5. Interpretation boundary

FTD-0444's algebraic underdetermination theorem is independent of its energy
example and remains valid. Its old selected map may remain a reversible map for
its synthetic dispersion, but it cannot be cited as production-compatible.

The corrected map still selects preserved transverse momentum and a
longitudinal branch. FTD-0447 derives the isolated force direction, not this
finite nonlinear branch rule. No production dynamics change follows.

## 6. Banned moves

- No velocity family, work, dispersion, conversion, tolerance, or outcome
  label may change after first execution.
- Do not silently rewrite the hash-locked FTD-0444 source or run record.
- No production tick changes.
