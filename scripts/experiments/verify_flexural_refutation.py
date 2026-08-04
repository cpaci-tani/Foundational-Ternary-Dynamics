"""Independent check of the refutation of FTD-0787: is the trimer bend
EXACTLY flat, making my quartic a chord across a flat valley?"""
import numpy as np
from mpmath import mp, mpf, sqrt, cos, sin, pi

mp.dps = 30
eps = mpf("0.01")

def V(q):
    q = mpf(q)
    return mpf(0) if q >= mpf(3)/2 else -16*eps*(q - mpf(3)/2)**2*(q - mpf(3)/4)

# polarity mask: A-B bonded (opposite), B-C bonded (opposite) => sigma_A = sigma_C
sA, sB, sC = 1, -1, 1
mask = lambda s1, s2: (1 - s1*s2)/2
print("=== THE POLARITY MASK ===")
print(f"  sigma_A={sA}, sigma_B={sB}, sigma_C={sC}  (both bonds opposite-polarity)")
print(f"  A_AB = {mask(sA,sB)}, A_BC = {mask(sB,sC)}, A_AC = {mask(sA,sC)}")
print(f"  => A and C do NOT interact BY POLARITY, at any separation.")
print(f"     (My FTD-0787 said 'outside compact support' - true but weaker.)")
print(f"  => U depends ONLY on the two bond lengths |AB| and |BC|.\n")

print("=== IS THE BEND FLAT? (fixed bond lengths = 1, vary the angle) ===")
print(f"  {'angle':>8} {'|AB|':>9} {'|BC|':>9} {'|AC|':>9} {'U':>16} {'U-(-2eps)':>12}")
for deg in (180, 150, 120, 90, 60, 30, 5):
    th = mpf(deg)*pi/180
    # B at origin, A at (-1,0), C at angle th from A-B direction, both bonds length 1
    A_ = np.array([-1.0, 0.0]); B_ = np.array([0.0, 0.0])
    C_ = np.array([float(cos(pi - th)), float(sin(pi - th))])
    lAB = np.linalg.norm(A_-B_); lBC = np.linalg.norm(B_-C_); lAC = np.linalg.norm(A_-C_)
    U = V(lAB**2) + V(lBC**2)          # A_AC = 0, so no third term
    print(f"  {deg:>8} {lAB:>9.6f} {lBC:>9.6f} {lAC:>9.6f} {float(U):>16.12f} "
          f"{float(U + 2*eps):>12.3e}")
print("  -> FLAT TO MACHINE ZERO at every angle. The bend costs nothing.\n")

print("=== SO WHAT DID MY 'QUARTIC' MEASURE? ===")
print(f"  {'d':>7} {'bond len on my path':>21} {'my V(d)':>14} "
      f"{'relaxed (bend only)':>21}")
for dd in ("0.0", "0.2", "0.4", "0.5525", "0.7071"):
    d = mpf(dd)
    bond = sqrt(1 + d**2)
    Vmine = 2*V(1 + d**2)
    print(f"  {float(d):>7.4f} {float(bond):>21.6f} {float(Vmine):>14.8f} "
          f"{float(-2*eps):>21.8f}")
print("  -> my path STRETCHES both bonds to sqrt(1+d^2). The system can reach the")
print("     same transverse offset by BENDING at fixed bond length, for free.")
print("     The 24 eps d^4 is the cost of a stretch the system need not make.\n")

print("=== HESSIAN OF THE COLLINEAR TRIMER (9x9) ===")
def U_full(x):
    p = x.reshape(3,3)
    u = 0.0
    for (i,j) in ((0,1),(1,2)):          # A-B, B-C only; A_AC = 0
        u += float(V(float(np.sum((p[i]-p[j])**2))))
    return u
x0 = np.array([-1.,0,0, 0.,0,0, 1.,0,0])
h = 1e-5
H = np.zeros((9,9))
for i in range(9):
    for j in range(9):
        xpp=x0.copy(); xpp[i]+=h; xpp[j]+=h
        xpm=x0.copy(); xpm[i]+=h; xpm[j]-=h
        xmp=x0.copy(); xmp[i]-=h; xmp[j]+=h
        xmm=x0.copy(); xmm[i]-=h; xmm[j]-=h
        H[i,j]=(U_full(xpp)-U_full(xpm)-U_full(xmp)+U_full(xmm))/(4*h*h)
ev = np.linalg.eigvalsh((H+H.T)/2)
print("  eigenvalues:", np.round(ev, 6))
nz = np.sum(np.abs(ev) < 1e-6)
print(f"  zero modes: {nz}   nonzero: {np.round(ev[np.abs(ev)>=1e-6],4)}")
print(f"  trivial zeros for a LINEAR molecule = 3 translations + 2 rotations = 5")
print(f"  => {nz-5} EXTRA zero modes = the two degenerate bends.")
print(f"  stiff modes: 96 eps = {float(96*eps):.4f} and 3*96 eps = {float(3*96*eps):.4f}")
print("\n  MAXWELL COUNT: 3N - B = 9 - 2 = 7 zero modes. Matches exactly.")
print("  The trimer is HYPOSTATIC (floppy). Its bend is a finite mechanism,")
print("  not merely an infinitesimal one -- hence flat to ALL orders.")
