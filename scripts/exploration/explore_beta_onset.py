import math
import numpy as np

# Grid parameters matching the engine scale
L = 16
T = 10
ALPHA = 1.0 / 18.0
K_GENESIS = 1.533
K_MANIFEST = 0.511
T_L = 0.005 # Langevin temperature
SIGMA_J = math.sqrt(2.0/3.0 * T_L) # thermal flux standard deviation per component

def laplacian18(f):
    """18-point stencil: faces weight 2, edges weight 1, sum-of-weights 24."""
    out = -24.0 * f
    for ax in range(3):
        out += 2.0 * (np.roll(f, 1, axis=ax) + np.roll(f, -1, axis=ax))
    for a1 in range(3):
        for a2 in range(a1 + 1, 3):
            for s1 in (1, -1):
                for s2 in (1, -1):
                    out += np.roll(np.roll(f, s1, axis=a1), s2, axis=a2)
    return out

def solve_poisson_sor(div, state, max_iters=6, omega=1.75):
    """Solve nabla^2 phi = div - state using checkerboard SOR on 18-point stencil."""
    phi = np.zeros_like(div)
    mean_charge = state.mean()
    source = div - (state - mean_charge)
    
    # checkerboard checker
    grid = np.indices(div.shape)
    parity = (grid[0] + grid[1] + grid[2]) % 2
    mask0 = (parity == 0)
    mask1 = (parity == 1)
    
    for _ in range(max_iters):
        for mask in (mask0, mask1):
            face_sum = np.zeros_like(phi)
            edge_sum = np.zeros_like(phi)
            for ax in range(3):
                face_sum += np.roll(phi, 1, axis=ax) + np.roll(phi, -1, axis=ax)
            for ax1, ax2 in [(0, 1), (0, 2), (1, 2)]:
                for s1 in (1, -1):
                    for s2 in (1, -1):
                        edge_sum += np.roll(np.roll(phi, s1, axis=ax1), s2, axis=ax2)
            
            gs = (face_sum / 3.0 + edge_sum / 6.0 - source) / 4.0
            phi[mask] += omega * (gs[mask] - phi[mask])
            
    phi -= phi.mean()
    return phi

def run_simulation(A, case=3, seed=None):
    """
    Simulates wave propagation on a 3D grid with options for back-reaction:
    case=1: Naive (No back-reaction, linear wave)
    case=2: Drains Only (Kinetic + Flux drain at center voxel)
    case=3: Full Back-Reaction (Drains + Gauss projection)
    """
    if seed is not None:
        np.random.seed(seed)
        
    c = L // 2
    Jx = np.zeros((L, L, L)); Jy = np.zeros_like(Jx); Jz = np.zeros_like(Jx)
    Vx = np.zeros_like(Jx); Vy = np.zeros_like(Jx); Vz = np.zeros_like(Jx)
    state = np.zeros((L, L, L), dtype=np.int8)
    
    # Inject flux at center
    Jx[c, c, c] = A * K_GENESIS
    
    center_manifested = False
    peak_J_neighbor = 0.0
    
    for t in range(1, T + 1):
        # 1. Read phase: Laplacians
        delta_jx = ALPHA * laplacian18(Jx)
        delta_jy = ALPHA * laplacian18(Jy)
        delta_jz = ALPHA * laplacian18(Jz)
        
        # 2. Write phase: update velocity and flux
        Vx += delta_jx
        Vy += delta_jy
        Vz += delta_jz
        
        Jx += Vx
        Jy += Vy
        Jz += Vz
        
        # 3. Genesis check
        if not center_manifested and case > 1:
            jmag_center = math.sqrt(Jx[c,c,c]**2 + Jy[c,c,c]**2 + Jz[c,c,c]**2)
            if jmag_center > K_GENESIS:
                p_man = 1.0 - math.exp(-(jmag_center - K_GENESIS) / K_MANIFEST)
                if np.random.random() < p_man:
                    center_manifested = True
                    # Sign convention: if div <= 0, state is -1
                    state[c, c, c] = -1
                    
                    # Apply kinetic velocity drain (50%)
                    Vx[c, c, c] *= 0.5
                    Vy[c, c, c] *= 0.5
                    Vz[c, c, c] *= 0.5
                    
                    # Apply flux drain
                    drain = max(0.0, 1.0 - K_GENESIS / jmag_center)
                    Jx[c, c, c] *= drain
                    Jy[c, c, c] *= drain
                    Jz[c, c, c] *= drain
                    
        # 4. Gauss projection
        if center_manifested and case == 3:
            div = 0.5 * (np.roll(Jx, -1, axis=0) - np.roll(Jx, 1, axis=0)) \
                + 0.5 * (np.roll(Jy, -1, axis=1) - np.roll(Jy, 1, axis=1)) \
                + 0.5 * (np.roll(Jz, -1, axis=2) - np.roll(Jz, 1, axis=2))
                
            phi = solve_poisson_sor(div, state, max_iters=6)
            
            grad_phix = 0.5 * (np.roll(phi, -1, axis=0) - np.roll(phi, 1, axis=0))
            grad_phiy = 0.5 * (np.roll(phi, -1, axis=1) - np.roll(phi, 1, axis=1))
            grad_phiz = 0.5 * (np.roll(phi, -1, axis=2) - np.roll(phi, 1, axis=2))
            
            mask_zero = (state == 0)
            Jx[mask_zero] -= grad_phix[mask_zero]
            Jy[mask_zero] -= grad_phiy[mask_zero]
            Jz[mask_zero] -= grad_phiz[mask_zero]
            
        jmag_neighbor = math.sqrt(Jx[c+1,c,c]**2 + Jy[c+1,c,c]**2 + Jz[c+1,c,c]**2)
        if jmag_neighbor > peak_J_neighbor:
            peak_J_neighbor = jmag_neighbor
            
    # Calculate manifestation probability at neighbor using Monte Carlo over Langevin thermal noise
    samples = 1000
    thermal_x = np.random.normal(0, SIGMA_J, samples)
    thermal_y = np.random.normal(0, SIGMA_J, samples)
    thermal_z = np.random.normal(0, SIGMA_J, samples)
    
    total_x = peak_J_neighbor + thermal_x
    total_y = thermal_y
    total_z = thermal_z
    total_mag = np.sqrt(total_x**2 + total_y**2 + total_z**2)
    
    excess = np.maximum(0.0, total_mag - K_GENESIS)
    p_man_samples = 1.0 - np.exp(-excess / K_MANIFEST)
    P_manifest_neighbor = np.mean(p_man_samples)
    
    return peak_J_neighbor, P_manifest_neighbor

def main():
    print(f"SIGMA_J = {SIGMA_J:.4f} (Langevin T_L = {T_L:.4f})")
    print(f"K_GENESIS = {K_GENESIS:.3f}, K_MANIFEST = {K_MANIFEST:.3f}")
    
    n_seeds = 30
    amplitudes = [4.0, 5.0, 5.62, 6.0, 7.0, 8.0, 8.5, 9.0, 9.5, 10.0]
    
    print("\nManifestation probability of the second shell (r=1 nearest neighbors):")
    print(f"{'A':<5} | {'Case 1 (Naive)':<18} | {'Case 2 (Drains Only)':<22} | {'Case 3 (Full Back-Reaction)':<28}")
    print("-" * 82)
    
    for A in amplitudes:
        p_man_c1, p_man_c2, p_man_c3 = [], [], []
        peak_j_c1, peak_j_c2, peak_j_c3 = [], [], []
        
        for seed in range(n_seeds):
            j1, p1 = run_simulation(A, case=1, seed=seed)
            j2, p2 = run_simulation(A, case=2, seed=seed)
            j3, p3 = run_simulation(A, case=3, seed=seed)
            
            peak_j_c1.append(j1); p_man_c1.append(p1)
            peak_j_c2.append(j2); p_man_c2.append(p2)
            peak_j_c3.append(j3); p_man_c3.append(p3)
            
        print(f"{A:<5.2f} | "
              f"{np.mean(peak_j_c1):.3f} ({np.mean(p_man_c1):7.2%}) | "
              f"{np.mean(peak_j_c2):.3f} ({np.mean(p_man_c2):7.2%}) | "
              f"{np.mean(peak_j_c3):.3f} ({np.mean(p_man_c3):7.2%})")

if __name__ == "__main__":
    main()
