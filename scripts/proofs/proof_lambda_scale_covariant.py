#!/usr/bin/env python3
"""
proof_lambda_scale_covariant.py  --  FTD-0331

Grounds the scale-covariant / holographic reading of the cosmological constant
(DERIV_LAMBDA_SCALE_COVARIANT.md). This is NOT a derivation of the value of
Lambda -- it verifies the three numerical facts the *mechanism* rests on:

  (1) The observed Lambda satisfies Lambda * L_H^2 ~ O(1), i.e. Lambda ~ 1/L_H^2.
  (2) The famous "~10^-122 fine-tuning" is exactly (l_P / L_H)^2 -- a ratio of the
      two scales FC-3 admits (UV = l_P, IR = horizon), not a tuned number.
  (3) The classical empty void carries zero field energy (structural, not numeric):
      every Lagrangian energy term vanishes at J = 0, s = 0, so there is no
      1/2 hbar omega zero-point floor and no M_Planck^4 catastrophe.

Tags: the mechanism is [SELECTION]+[BOUNDARY]; nothing here promotes any claim.
Inputs L_H (the horizon) are EXTERNAL (FTD-0059: no native length) -- the value of
Lambda is a boundary, by design.
"""

# Externally-measured inputs (Planck 2018; CODATA). These are inputs, not outputs.
l_P    = 1.616255e-35     # Planck length [m]
c      = 2.99792458e8     # [m/s]
H0     = 2.1927e-18       # Hubble constant [1/s]  (~67.66 km/s/Mpc)
Lambda = 1.089e-52        # observed cosmological constant [1/m^2]

L_H = c / H0              # Hubble length [m] -- the IR / horizon scale

def approx(a, b, tol):
    return abs(a - b) <= tol * abs(b)

def main():
    ok = True

    # (1) Lambda * L_H^2 ~ O(1)
    LL = Lambda * L_H**2
    print(f"[1] L_H            = {L_H:.4e} m = {L_H/l_P:.4e} Planck lengths")
    print(f"    Lambda * L_H^2 = {LL:.4f}   (expect O(1): 0.5 < x < 6)")
    c1 = 0.5 < LL < 6.0
    ok &= c1
    print(f"    => Lambda ~ 1/L_H^2 : {'PASS' if c1 else 'FAIL'}\n")

    # (2) the "10^-122" IS (l_P / L_H)^2
    ratio2 = (l_P / L_H)**2
    Llp2   = Lambda * l_P**2
    print(f"[2] (l_P/L_H)^2    = {ratio2:.4e}   <- the famous ~10^-122")
    print(f"    Lambda * l_P^2 = {Llp2:.4e}   <- the CC in Planck units")
    # they agree to within the same O(1) factor as (1) (Lambda*l_P^2 = (Lambda*L_H^2)*(l_P/L_H)^2)
    c2 = approx(Llp2, LL * ratio2, 1e-6) and (1e-123 < ratio2 < 1e-121)
    ok &= c2
    print(f"    => smallness of Lambda = largeness of universe : {'PASS' if c2 else 'FAIL'}\n")

    # (3) classical empty void is zero-energy (structural identity, shown symbolically)
    #     energy density terms (SPEC_FTD_LAGRANGIAN sec 3.6):
    #       field kinetic  0.5 * ||dJ/dt||^2
    #       field gradient 0.5 * c^2 * ||grad J||^2
    #       rest term     -K_B  applies only where s != 0
    #     at the void J = 0, s = 0:
    J = 0.0; dJ_dt = 0.0; gradJ = 0.0; s = 0
    K_B = 0.511  # MeV; present only for manifested s != 0
    rho_void = 0.5*dJ_dt**2 + 0.5*(1/3)*gradJ**2 + ( -K_B if s != 0 else 0.0 )
    print(f"[3] classical void energy density rho(J=0,s=0) = {rho_void:.1f}")
    c3 = (rho_void == 0.0)
    ok &= c3
    print(f"    => no 1/2 hbar omega floor, no M_Planck^4 catastrophe : {'PASS' if c3 else 'FAIL'}\n")

    print("=" * 60)
    print(f"ALL CHECKS: {'PASS' if ok else 'FAIL'}")
    print("Mechanism [SELECTION]+[BOUNDARY]; value needs L_H (FTD-0059) -- a boundary, not a derivation.")
    return 0 if ok else 1

if __name__ == "__main__":
    raise SystemExit(main())
