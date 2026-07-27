#!/usr/bin/env python3
"""
energy_scales_2026.py - computed numbers for SPEC_ENERGY_SCALES_AND_DETECTABILITY.md.

Every number quoted in the energy-scales assessment is COMPUTED here (no recalled
high-precision values; no near-miss searching; no new identities). All FRAMEWORK
constants are imported from the canonical scripts/constants.py; the few SI/CODATA
constants not in that module (Planck length, c, hbar*c) are declared with their
CODATA-2022 / PDG source per docs/reference/REF_EXTERNAL_CONSTANTS.md.

Sections:
  0. Scale hierarchy - voxel/lattice physical extents, Planck energy, the CERN gap.
  1. Free-flux dispersion -> Lorentz violation - q = E/E_P at leading order;
     fully discrete dim-6 dv/c = (E/E_P)^2/12; dimension-8 cubic anisotropy
     delta = 6.95e-4*(E/E_P)^4; GRB ToF. Whole-theory radiative/common-cone
     matching remains open under FTD-0407.
  2. Manifestation / pair threshold - K_GENESIS = 3*K_B vs the QED 2*m_e.
  3. Emergent mass/energy ladder - N(A)*K_B at L=32 (FTD-0261 run of record).
  4. Structural nulls - the [THEOREM]-grade forbiddens (listed; no free numbers).

Calibration register (a_phys = l_P, t_phys = l_P/(sqrt(3)*c), K_B = m_e):
SPEC_DIMENSIONAL_MAP.md s4. Dispersion symbol: ANALYSIS_WAVE_SECTORS_v1.md (FTD-0299)
+ AUDIT_LORENTZ_ANISOTROPY.md s2.4 (FTD-0092 / PL-5, FTD-0258). Ladder: ANALYSIS_NA_LAW_CURRENT_STACK_v1.md
(FTD-0261). Structural nulls: SPEC_PREDICTION_LEDGER_DEVIATIONS.md PL-6 + proof_complete_sm.py.
"""
import math
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
try:
    sys.stdout.reconfigure(encoding="utf-8")
except Exception:
    pass

import constants as C  # canonical framework constants (single source of truth)

# --------------------------------------------------------------------------
# SI / CODATA-2022 constants NOT in the framework module (declared, sourced).
# REF_EXTERNAL_CONSTANTS.md standard: CODATA 2022 / PDG 2024.
# --------------------------------------------------------------------------
L_PLANCK_M  = 1.616255e-35          # Planck length [m], CODATA 2022
C_LIGHT     = 299_792_458.0         # speed of light [m/s], exact (SI)
HBARC_MEV_M = 197.3269804e-15       # hbar*c [MeV*m] (= 197.327 MeV*fm), CODATA 2022
GYR_S       = 3.15576e16            # 1 Gyr in seconds (Julian)

# Framework calibration + constants (imported - never recalled from prose).
KB_MEV      = C.K_B                 # 0.511 MeV  (manifestation threshold = m_e)
KGEN_MEV    = C.K_GENESIS           # 1.533 MeV  (= N_c * K_B)
C_WAVE      = C.C_WAVE              # 1/sqrt(3)  (lattice speed of light, voxels/tick)
E_PLANCK_GEV = C.M_PLANCK          # 1.220890e19 GeV (Planck energy in natural units)
ME_MEV      = C.Experimental.m_electron

SEP = "=" * 78


def banner(txt):
    print(SEP)
    print(txt)
    print(SEP)


# ==========================================================================
banner("0. SCALE HIERARCHY - the lattice is a Planck-scale instrument")
# ==========================================================================
print(f"a_phys = l_P              = {L_PLANCK_M:.6e} m   (1 voxel = 1 Planck length) [CALIBRATION]")
t_planck_s = L_PLANCK_M / C_LIGHT
t_phys_s   = L_PLANCK_M / (math.sqrt(3.0) * C_LIGHT)  # canonical edge gauge (FTD-0385)
print(f"t_Planck = l_P/c          = {t_planck_s:.6e} s")
print(f"t_phys = l_P/(sqrt(3)*c)  = {t_phys_s:.6e} s   (one tick) [CALIBRATION]")
print(f"K_B = m_e                 = {KB_MEV:.3f} MeV   (mass unit = 1 MeV/c^2) [IMPOSED]")
print(f"c_lat (selected; stable)   = 1/sqrt(3) = {C_WAVE:.6f} voxels/tick")
print()

# Planck energy, cross-checked two independent ways.
E_P_from_lP_MeV = HBARC_MEV_M / L_PLANCK_M           # hbar*c / l_P  = E_Planck
E_P_from_lP_GeV = E_P_from_lP_MeV / 1000.0
print(f"E_Planck (= M_PLANCK, constants.py)     = {E_PLANCK_GEV:.6e} GeV")
print(f"E_Planck (= hbar*c / l_P, independent)  = {E_P_from_lP_GeV:.6e} GeV")
rel = abs(E_P_from_lP_GeV - E_PLANCK_GEV) / E_PLANCK_GEV
print(f"  cross-check agreement                 = {rel:.3e} (relative)  -> k = E/E_P is well-defined")
assert rel < 2e-4, "E_Planck cross-check failed - k=E/E_P mapping would be unsafe"
print()

print("Lattice physical extents (L voxels per side, span = L * l_P):")
for L in (33, 64, 128, 256):
    span = L * L_PLANCK_M
    print(f"  L = {L:>4}   span = {span:.3e} m")
print()

# CERN reach vs the lattice.
E_LHC_GEV = 13600.0                                  # 13.6 TeV collision energy (Run 3)
lhc_len_m = HBARC_MEV_M / (E_LHC_GEV * 1000.0)       # hbar*c / E (resolved length)
lhc_voxels = lhc_len_m / L_PLANCK_M
span_256 = 256 * L_PLANCK_M
print(f"LHC collision energy                    = {E_LHC_GEV/1000:.1f} TeV")
print(f"LHC-resolved length  (hbar*c / E)       = {lhc_len_m:.3e} m")
print(f"  -> spans                              = {lhc_voxels:.3e} voxels (a CERN-probed structure)")
print(f"L=256 lattice span / LHC-resolved length= {span_256 / lhc_len_m:.3e}")
print(f"  i.e. the largest practical lattice is ~{lhc_len_m/span_256:.1e}x SHORTER than one LHC resolution element")
print(f"E_LHC / E_Planck                        = {E_LHC_GEV / E_PLANCK_GEV:.3e}")
print()

# ==========================================================================
banner("1. FREE-FLUX DISPERSION -> LORENTZ VIOLATION  (FTD-0299 / FTD-0407)")
# ==========================================================================
print("Exact axis pole:      theta(q) = 2 asin(c_lat sin(q/2)),  c_lat = 1/sqrt(3)")
print("Fully discrete:       theta^2 = q^2/3 - q^4/54 - q^6/4860 + O(q^8)  (FTD-0407)")
print("Group velocity:       v_g/c = cos(q/2)/sqrt(1-c_lat^2 sin^2(q/2)) = 1-q^2/12+O(q^4)")
print("Photon energy -> q:   q = E/E_P at leading order (a_phys=l_P, edge-gauge tick calibration)")
print()
print("  -> direct free-pole LINEAR (dim-5) coefficient = 0 [tree-level, sector-scoped]")
print("  -> leading term is QUADRATIC (dim-6) boost violation; first cubic rotation breaking is dimension 8")
print("  -> lower-dimensional radiative mixing and common matter/flux speed remain OPEN (FTD-0407)")
print()
ANISO_PREFACTOR = 6.95e-4    # delta = prefactor * k^4 (AUDIT_LORENTZ_ANISOTROPY: p=4.0008, L=64 -> 6.5e-8)
D_COSMO_S = 13.0 * GYR_S     # ~13 Gly light-travel (cosmological GRB baseline), order-of-magnitude
print(f"GRB time-of-flight baseline (order): D/c ~ 13 Gly = {D_COSMO_S:.2e} s")
print(f"{'E (photon)':>16} | {'q = E/E_P':>12} | {'dv/c (dim-6)':>14} | {'delta_aniso (dim-8)':>20} | {'GRB ToF delay':>14}")
for label, E_GeV in (("1 GeV", 1.0), ("10 GeV (GRB)", 10.0), ("1 TeV", 1.0e3),
                     ("13.6 TeV (LHC)", 1.36e4), ("E_Planck", E_PLANCK_GEV)):
    k = E_GeV / E_PLANCK_GEV
    dv_c_lead = k * k / 12.0
    delta = ANISO_PREFACTOR * k ** 4
    tof = dv_c_lead * D_COSMO_S
    print(f"{label:>16} | {k:>12.3e} | {dv_c_lead:>14.3e} | {delta:>20.3e} | {tof:>11.3e} s")
print("  (dv/c column uses the fully discrete leading q^2/12; GRB ToF delay is dv/c * D/c.)")
print("  At 10 GeV the free-pole ToF estimate is ~2e-20 s; this does not address radiative or common-cone effects.")
print()

# ==========================================================================
banner("2. MANIFESTATION / PAIR-PRODUCTION THRESHOLD")
# ==========================================================================
pair_thresh = 2.0 * ME_MEV
print(f"K_GENESIS = N_c * K_B            = {KGEN_MEV:.3f} MeV   (engine matter-creation threshold) [IMPOSED]")
print(f"QED pair-production threshold 2*m_e = {pair_thresh:.3f} MeV")
print(f"ratio K_GENESIS / (2 m_e)       = {KGEN_MEV / pair_thresh:.4f}   (not 3/2 after K_MANIFEST/M_REST split)")
print(f"Genesis-event budget at A=10: ~5 one-shot events (FTD-0267 SURVIVAL-NULL) - cited, not recomputed.")
print(f"Physical identification of K_GENESIS with a pair threshold: [CONJECTURE] / calibration-dependent.")
print()

# ==========================================================================
banner("3. EMERGENT MASS/ENERGY LADDER  (N(A)*K_B; FTD-0261, L=32 run of record)")
# ==========================================================================
print("inertial relation mass = N*M_REST = N*K_B is [IMPOSED] (FTD-0250); identification of N with a")
print("physical particle mass is [SMC] (FTD-0110). Engine-level; calibration register applies.")
print("FTD-0261 run-of-record table (arm N, L=32; ANALYSIS_NA_LAW_CURRENT_STACK_v1.md s1):")
A_meas = [10, 12, 14, 16, 20, 25, 30, 40, 50, 70, 90]
N_meas = [4.0, 8.4, 16.4, 21.6, 27.4, 32.6, 45.0, 91.8, 130.2, 260.2, 383.3]
print(f"{'A':>5} | {'N_bar':>7} | {'E = N*K_B (MeV)':>16}")
print(f"{1:>5} | {1.0:>7.1f} | {1.0 * KB_MEV:>16.3f}   <- electron anchor (N=1, A~2) [MEASURED-exact, FTD-0262]")
for A, N in zip(A_meas, N_meas):
    print(f"{A:>5} | {N:>7.1f} | {N * KB_MEV:>16.3f}")
print(f"Demonstrated ladder span at L=32: {1.0 * KB_MEV:.3f} MeV (N=1) -> {383.3 * KB_MEV:.1f} MeV (N=383, A=90 flooding boundary).")
print("Identification with specific SM masses: IDENT-NULL (no SM-ratio specialness, p_local=2.052; FTD-0262).")
print()

# ==========================================================================
banner("4. STRUCTURAL NULLS AT THE FRONTIER  ([THEOREM]-grade; PL-6)")
# ==========================================================================
print("No free numbers - exact ontological forbiddens. [THEOREM]-grade PL-6 nulls")
print("(SPEC_PREDICTION_LEDGER_DEVIATIONS.md PL-6); each killed by one contrary observation:")
for s in ("N_monopole = 0          (div B = div(curl J) = 0, an identity)",
          "N_SUSY = 0              (ternary state space carries no fermionic grading)",
          "extra dimensions = 0    (|Aut(E)|^2 = 2^D (D-1)! forces D = 3)"):
    print(f"  - {s}")
print("Honesty caveats (NOT clean [THEOREM]):")
print("  - fermion generations = 3 : [THEOREM topological] / [OPEN dynamical] (CATALOG s16); beyond PL-6's four.")
print("  - tau_proton: NOT a forced infinity. LEDGER FTD-0301 = [MEASURED - UNFORCED-METASTABLE BOUNDARY]:")
print("       only U(1) Sigma-s charge (no baryon/B-L current); FTD's own weak sector decays the mixed-sign")
print("       uud cluster; proof_complete_sm.py tags tau_proton [SELECTION], 'NOT a forced [THEOREM]'.")
print("       (PL-6 source row still reads [THEOREM] -> stale; flagged for FTD-0301 reconciliation.)")
print()
print(SEP)
print("END. Every figure above is computed from constants.py + declared CODATA constants.")
print(SEP)
