"""
Rutherford Scattering Verification on FTD Lattice
=================================================

Tests if the discrete gradient of a 1/r potential on the cubic lattice
yields the correct 1/sin^4(theta/2) scattering cross-section.

Method:
1. Construct 1/r potential on a 3D grid.
2. Compute force field F = -grad(Phi) using DISCRETE operators.
3. Integrate trajectories for particles with varying impact parameters (b).
4. Tally scattering angles and compare to Rutherford formula.
"""

import numpy as np
import matplotlib.pyplot as plt
from discrete_operators import discrete_gradient

def run_scattering_experiment(grid_size=100, n_particles=500):
    print("=" * 60)
    print("RUTHERFORD SCATTERING SIMULATION (LATTICE DRIVEN)")
    print("=" * 60)
    
    # 1. Setup Lattice Potential (Coulomb 1/r)
    center = grid_size // 2
    x = np.arange(grid_size) - center
    y = np.arange(grid_size) - center
    z = np.arange(grid_size) - center
    
    X, Y, Z = np.meshgrid(x, y, z, indexing='ij')
    R = np.sqrt(X**2 + Y**2 + Z**2)
    R[center, center, center] = 1e-6 # Avoid singularity
    
    # Potential Phi = 1/r (repulsive)
    Q = 10.0
    Phi = Q / R
    # Clamp core to avoid purely numerical singularities exploding
    Phi[center, center, center] = Phi[center+1, center, center] 
    
    print(f"Lattice initialized: {grid_size}^3")
    print("Computing Discrete Force Field F = -nabla(Phi)...")
    
    # 2. Compute Discrete Forces
    grad = discrete_gradient(Phi)
    # F = -grad(Phi)
    Fx_grid = -grad[0]
    Fy_grid = -grad[1]
    Fz_grid = -grad[2]
    
    # 3. Integrate Trajectories
    # Filter: Avoid b < 2.0 (Core lattice effects) and b > grid/6 (Boundary effects)
    impact_params = np.linspace(2.0, grid_size/6, n_particles)
    scattering_angles = []
    
    dt = 0.1 # Finer time step
    v0 = 1.0 # Slower velocity to feel the potential
    
    print(f"Firing {n_particles} test particles (b={impact_params[0]:.1f} to {impact_params[-1]:.1f})...")
    
    for b in impact_params:
        # Initial State: Start at z=0 edge, nucleus at center (z=grid/2)
        px = b + center
        py = center
        pz = 2.0 # Start slightly inside to avoid index errors
        
        vx = 0.0
        vy = 0.0
        vz = v0
        
        # Integration loop
        max_steps = int(grid_size * 2 / v0 / dt)
        
        for t in range(max_steps):
            # Nearest Neighbor Interpolation
            ix = int(round(px))
            iy = int(round(py))
            iz = int(round(pz))
            
            # Boundary check
            if ix < 1 or ix >= grid_size-1 or \
               iy < 1 or iy >= grid_size-1 or \
               iz < 1 or iz >= grid_size-1:
                break
                
            fx = Fx_grid[ix, iy, iz]
            fy = Fy_grid[ix, iy, iz]
            fz = Fz_grid[ix, iy, iz]
            
            # Newtonian Update (Symplectic Euler)
            vx += fx * dt
            vy += fy * dt
            vz += fz * dt
            
            px += vx * dt
            py += vy * dt
            pz += vz * dt
            
        # Compute final scattering angle
        vf_mag = np.sqrt(vx**2 + vy**2 + vz**2)
        if vf_mag < 1e-9: continue
        
        # Dot product with initial direction (0,0,1)
        # cos_theta = vz / |v|
        cos_theta = vz / vf_mag
        cos_theta = np.clip(cos_theta, -1.0, 1.0)
        theta = np.arccos(cos_theta)
        
        # Filter minimal deflections (no interaction)
        if theta > 1e-4:
            scattering_angles.append((b, theta))
            
    # 4. Analyze Results
    print(f"Captured {len(scattering_angles)} valid scattering events.")
    
    if len(scattering_angles) == 0:
        print("[FAIL] No events captured.")
        return False

    b_vals = np.array([p[0] for p in scattering_angles])
    theta_vals = np.array([p[1] for p in scattering_angles])
    
    # Debug stats
    print(f"Theta Min: {np.degrees(theta_vals.min()):.4f} deg")
    print(f"Theta Max: {np.degrees(theta_vals.max()):.4f} deg")
    
    # Rutherford Check: K = b * tan(theta/2) should be constant
    K_vals = b_vals * np.tan(theta_vals / 2.0)
    
    # IQR Filtering
    if len(K_vals) > 5:
        Q1 = np.percentile(K_vals, 25)
        Q3 = np.percentile(K_vals, 75)
        IQR = Q3 - Q1
        mask = (K_vals >= Q1 - 1.5*IQR) & (K_vals <= Q3 + 1.5*IQR)
        K_filtered = K_vals[mask]
        print(f"Outliers removed: {len(K_vals) - len(K_filtered)}")
    else:
        K_filtered = K_vals
    
    K_mean = np.mean(K_filtered)
    K_std = np.std(K_filtered)
    rel_error = K_std / (K_mean + 1e-9)
    
    print("-" * 40)
    print("RUTHERFORD TEST RESULTS")
    print(f"Theory Constraint: b * tan(theta/2) = Constant")
    print(f"Measured Mean K: {K_mean:.4f}")
    print(f"Measured StdDev: {K_std:.4f}")
    print(f"Relative Deviation: {rel_error*100:.2f}%")
    print("-" * 40)
    
    # Sample Data
    print("Sample (b -> theta_deg -> K):")
    step = max(1, len(b_vals)//5)
    for i in range(0, len(b_vals), step):
        print(f"  {b_vals[i]:.2f} -> {np.degrees(theta_vals[i]):.2f} -> {K_vals[i]:.4f}")
        
    return rel_error < 0.15

if __name__ == "__main__":
    run_scattering_experiment()
