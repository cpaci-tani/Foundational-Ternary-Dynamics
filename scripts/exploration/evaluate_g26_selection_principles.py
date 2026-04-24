"""
Evaluate G26 Moore Shell Selection Principles
======================================================
Investigate if there is an FTD-native principle that uniquely
selects the corner weight `c` in the G26 operator family,
or if G18 (c=0) is the unique canonical choice.
"""

import numpy as np

def compute_symbol(kx, ky, kz, c):
    a = 1/3 + 4*c
    b = 1/6 - 2*c
    face = 2*a * (np.cos(kx) + np.cos(ky) + np.cos(kz) - 3)
    edge = 4*b * (np.cos(kx)*np.cos(ky) + np.cos(ky)*np.cos(kz) + np.cos(kz)*np.cos(kx) - 3)
    corner = 8*c * (np.cos(kx)*np.cos(ky)*np.cos(kz) - 1)
    return face + edge + corner

def scan_parameter_space():
    c_vals = np.linspace(0, 1/12, 1000)
    
    spectral_radii = []
    anisotropy_L2 = []
    
    # Test momenta for anisotropy at q = pi/8
    q = np.pi / 8
    dirs = [
        (q, 0, 0),
        (q/np.sqrt(2), q/np.sqrt(2), 0),
        (q/np.sqrt(3), q/np.sqrt(3), q/np.sqrt(3))
    ]
    
    for c in c_vals:
        # Spectral radius calculation (max eigenvalue over BZ)
        # We test the high-symmetry points
        l1 = np.abs(compute_symbol(np.pi, 0, 0, c))
        l2 = np.abs(compute_symbol(np.pi, np.pi, 0, c))
        l3 = np.abs(compute_symbol(np.pi, np.pi, np.pi, c))
        spectral_radius = max(l1, l2, l3)
        spectral_radii.append(spectral_radius)
        
        # Finite-k anisotropy (variance of symbol at fixed q)
        symbols = [compute_symbol(kx, ky, kz, c) for kx, ky, kz in dirs]
        mean_sym = np.mean(symbols)
        var_sym = np.sum((np.array(symbols) - mean_sym)**2)
        anisotropy_L2.append(var_sym)
        
    # Analysis
    min_radius = min(spectral_radii)
    c_min_radius = c_vals[np.argmin(spectral_radii)]
    
    min_aniso = min(anisotropy_L2)
    c_min_aniso = c_vals[np.argmin(anisotropy_L2)]
    
    print("================================================================")
    print("  G26 Corner Weight (c) Selection Principles")
    print("================================================================")
    print(f"c range: [0, {1/12:.4f}]")
    print(f"1. Spectral Radius:")
    print(f"   Min Spectral Radius: {min_radius:.4f}")
    print(f"   Occurs at c = {c_min_radius:.5f} (c <= 1/48 ~ 0.0208)")
    print(f"   G18 (c=0) radius: {spectral_radii[0]:.4f}")
    
    print(f"2. Finite-k Anisotropy (at q=pi/8):")
    print(f"   Min Anisotropy Variance: {min_aniso:.4e}")
    print(f"   Occurs at c = {c_min_aniso:.5f}")
    print(f"   G18 (c=0) anisotropy var: {anisotropy_L2[0]:.4e}")
    
    print("\nConclusion Check:")
    if abs(c_min_aniso - 0) < 1e-4:
        print("-> finite-k anisotropy uniquely selects c = 0 (G18)!")
    elif abs(c_min_aniso - 1/24) < 1e-4:
        print("-> finite-k anisotropy selects c = 1/24 (G26_iso_mid)")
    else:
        print(f"-> finite-k anisotropy selects an arbitrary c = {c_min_aniso:.5f}")
        
    print("================================================================")

if __name__ == "__main__":
    scan_parameter_space()
