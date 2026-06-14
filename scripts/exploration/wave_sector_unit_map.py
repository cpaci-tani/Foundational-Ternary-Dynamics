#!/usr/bin/env python3
"""Wave-sector unit map for ANALYSIS_LATTICE_WAVE_SECTORS_v1 (FTD-0298).

Computes (does not assert from memory) the lattice -> physical conversions for
the flux-wave sector under the canonical calibration:

  a_phys  = l_P            (length anchor, [CALIBRATION], dimensional_map.json)
  t_phys  = sqrt(3)*l_P/c  (one tick, [CALIBRATION], = 9.34e-44 s)
  c_lat   = 1/sqrt(3)      (lattice speed of light, [DERIVED] CFL)

Per-axis leapfrog dispersion: omega(k) = 2*c_lat*|sin(k/2)|  [rad/tick],
zone-edge (Nyquist) wavelength = 2 voxels.

The dimensionless k/k_zone ratios depend ONLY on a_phys (not on the t_phys
convention), so they are the load-bearing numbers in the doc.
"""
import math

# Canonical constants (CODATA 2022, per REF_EXTERNAL_CONSTANTS.md / dimensional_map.json)
l_P = 1.616255e-35      # Planck length [m]
c   = 299792458.0       # speed of light [m/s] (exact)
t_P = l_P / c           # Planck time [s]

C_SPEED = 1.0 / math.sqrt(3.0)         # lattice c, voxel/tick [DERIVED]
t_phys  = math.sqrt(3.0) * l_P / c     # one tick [s], canonical calibration

# Zone-edge (per-axis) lattice angular frequency: omega_max = 2*c_lat  [rad/tick]
omega_max_lat = 2.0 * C_SPEED
omega_max_phys = omega_max_lat / t_phys           # [rad/s]
f_max_phys     = omega_max_phys / (2.0 * math.pi)  # [Hz]

omega_P = 1.0 / t_P                  # Planck angular frequency [rad/s]
f_P     = omega_P / (2.0 * math.pi)  # Planck (ordinary) frequency [Hz]

lambda_min = 2.0 * l_P  # zone-edge wavelength = 2 voxels [m]

def k_over_kzone(f_hz):
    """Dimensionless k/k_zone for a physical wave of frequency f (vacuum)."""
    lam = c / f_hz
    return lambda_min / lam   # = (2 l_P)/lambda

print(f"t_phys (sqrt3*l_P/c)      = {t_phys:.4e} s   (canonical 9.34e-44)")
print(f"t_P (Planck time)         = {t_P:.4e} s")
print(f"omega_max (lattice)       = {omega_max_lat:.6f} rad/tick  (= 2/sqrt3)")
print(f"omega_max (physical)      = {omega_max_phys:.4e} rad/s")
print(f"omega_P (Planck)          = {omega_P:.4e} rad/s")
print(f"f_max (physical)          = {f_max_phys:.4e} Hz")
print(f"f_P (Planck, ordinary)    = {f_P:.4e} Hz")
print(f"zone-edge wavelength      = {lambda_min:.4e} m  (= 2 l_P)")
print()
for label, f in [("visible (5e14 Hz)", 5.0e14), ("FM radio (1e8 Hz)", 1.0e8)]:
    r = k_over_kzone(f)
    print(f"{label:20s}: k/k_zone = {r:.3e} ; "
          f"PL-4 (k^2) ~ {r**2:.3e} ; PL-5 (k^4) ~ {r**4:.3e}")
