import numpy as np
import scipy.integrate as integrate
from scipy.special import gamma

# Constants
G_star = gamma(0.25) / gamma(0.75)
target_val = 16 * G_star**2

def watson_integrand_2d(px, py):
    return 1.0 / (4 - 2*np.cos(px) - 2*np.cos(py))

def watson_integrand_4d(px, py, pz, pw):
    return 1.0 / (8 - 2*np.cos(px) - 2*np.cos(py) - 2*np.cos(pz) - 2*np.cos(pw))

def finite_lattice_trace(L, D, twist=0.0):
    """
    Computes the trace of the inverse Laplacian on a finite L^D lattice 
    with a given boundary phase twist.
    """
    trace = 0.0
    
    # 1D array of momenta
    n = np.arange(L)
    p = (2 * np.pi * n + twist) / L
    
    if D == 2:
        px, py = np.meshgrid(p, p)
        denom = 4 - 2*np.cos(px) - 2*np.cos(py)
        # Avoid division by zero at zero momentum
        denom[denom == 0] = np.inf
        trace = np.sum(1.0 / denom) / (L**2)
    
    elif D == 4:
        px, py, pz, pw = np.meshgrid(p, p, p, p)
        denom = 8 - 2*np.cos(px) - 2*np.cos(py) - 2*np.cos(pz) - 2*np.cos(pw)
        denom[denom == 0] = np.inf
        trace = np.sum(1.0 / denom) / (L**4)
        
    return trace

def main():
    print(f"Target 16*(G^*)^2 = {target_val:.6f}")
    
    # Test finite lattice traces with twist = pi/2 (quarter twist)
    print("\n--- Finite Lattice Trace (D=2), Twist = pi/2 ---")
    for L in [4, 8, 16, 32, 64]:
        tr = finite_lattice_trace(L, 2, twist=np.pi/2)
        print(f"L={L:2d} | Trace = {tr:.6f}")
        
    print("\n--- Finite Lattice Trace (D=4), Twist = pi/2 ---")
    for L in [4, 8, 16]:
        tr = finite_lattice_trace(L, 4, twist=np.pi/2)
        print(f"L={L:2d} | Trace = {tr:.6f}")

if __name__ == '__main__':
    main()
