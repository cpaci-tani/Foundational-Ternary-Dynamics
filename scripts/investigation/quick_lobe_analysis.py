"""Quick lobe topology analysis of the Fourcier curve."""
import numpy as np
from scipy.signal import argrelextrema

FREQS = [1, 2, 4, 8, 16]
CX = [1.0, 0.5, 0.5, 0.4, 0.0625]
CY = [1.0, -0.5, 0.5, -0.35, 0.0625]

def fc(t, cx, cy):
    x = np.zeros_like(t)
    y = np.zeros_like(t)
    for k in range(len(cx)):
        x += cx[k] * np.cos(FREQS[k] * t)
        y += cy[k] * np.sin(FREQS[k] * t)
    return x, y

t = np.linspace(0, 2*np.pi, 8000)
K = np.ones(20) / 20

print("=== LOBE STRUCTURE PER HARMONIC LEVEL ===")
for n in range(1, 6):
    x, y = fc(t, CX[:n], CY[:n])
    r = np.sqrt(x**2 + y**2)
    rs = np.convolve(r, K, mode='same')
    mi = argrelextrema(rs, np.less, order=40)[0]
    deep = [i for i in mi if rs[i] < 0.25 * np.max(rs)]
    angles = [t[i] * 180 / np.pi for i in deep]
    angle_strs = [f"{a:.0f}" for a in angles]
    print(f"  {n} harmonics: {len(deep)} deep minima, angles = {angle_strs}")

print()
print("=== FULL CURVE DEEP MINIMA ANALYSIS ===")
x, y = fc(t, CX, CY)
r = np.sqrt(x**2 + y**2)
rs = np.convolve(r, K, mode='same')
mi = argrelextrema(rs, np.less, order=40)[0]
deep = [i for i in mi if rs[i] < 0.25 * np.max(rs)]

if len(deep) >= 2:
    angles = [t[i] * 180 / np.pi for i in deep]
    print(f"  {len(deep)} deep minima at angles: {[f'{a:.1f}' for a in angles]}")
    
    if len(deep) == 6:
        # Pair analysis: are these 3 pairs?
        for i in range(0, 6, 2):
            gap = angles[i+1] - angles[i]
            print(f"  Pair {i//2+1}: {angles[i]:.1f} - {angles[i+1]:.1f} (gap {gap:.1f} deg)")
        
        # Inter-pair gaps
        for i in range(2):
            gap = angles[2*(i+1)] - angles[2*i+1]
            print(f"  Between pair {i+1} and {i+2}: {gap:.1f} deg")
        wrap = 360 + angles[0] - angles[5]
        print(f"  Between pair 3 and 1: {wrap:.1f} deg")
        
        print()
        print("  HEXAGONAL: 6 = 2 x 3")
        print("  Z/6Z = Z/2Z x Z/3Z")
        print("  Each of 3 color lobes has 2 sub-domains (matter/antimatter)")
    elif len(deep) == 4:
        for i in range(len(deep)):
            for j in range(i+1, len(deep)):
                gap = angles[j] - angles[i]
                print(f"  Min {i+1} to {j+1}: {gap:.1f} deg")
else:
    print("  Only 1 or 0 deep minima (single lobe)")

# Also check winding number
print()
print("=== WINDING ANALYSIS ===")
theta = np.arctan2(y, x)
dtheta = np.diff(theta)
# Fix wrapping
dtheta = np.where(dtheta > np.pi, dtheta - 2*np.pi, dtheta)
dtheta = np.where(dtheta < -np.pi, dtheta + 2*np.pi, dtheta)
winding = np.sum(dtheta) / (2 * np.pi)
print(f"  Winding number: {winding:.4f}")
print(f"  (Should be integer for closed curve)")

# Self-intersection count
print()
print("=== SELF-INTERSECTION ANALYSIS ===")
# Simple approach: count how many times the curve passes through r < threshold
r_thresh = 0.10 * np.max(r)
below = r < r_thresh
transitions_down = np.sum(np.diff(below.astype(int)) == 1)
print(f"  Near-origin passages (r < {r_thresh:.3f}): {transitions_down}")
print(f"  Visual lobes ~ {transitions_down}")
