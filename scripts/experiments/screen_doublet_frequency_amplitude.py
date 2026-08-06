"""Frequency-vs-amplitude test on the FTD-0659 doublet. Read-only.

A quartic clock has Omega ~ A, so cycle count must scale with amplitude.
A harmonic mode has Omega independent of A.
"""
import pandas as pd, numpy as np, re, os

BASE = r"C:\Users\cpaci\Desktop\ftd\engine\results\ftd_0659"
a = pd.read_csv(os.path.join(BASE, "ftd_0659_native_excited_matter_clock_arms_v1.csv"))
t = pd.read_csv(os.path.join(BASE, "ftd_0659_native_excited_matter_clock_ticks_v1.csv"))


def aidx(lab):
    m = re.search(r"_a(\d)_", str(lab))
    return int(m.group(1)) if m else None


a["aidx"] = a.label.map(aidx)
a = a[a.aidx.notna()]

print("modal_amplitude by amplitude index:")
amps = {}
for k, g in a.groupby("aidx"):
    v = g.modal_amplitude.astype(float).mean()
    amps[int(k)] = v
    print(f"   a{int(k)}: modal_amplitude = {v:.6e}   (n={len(g)})")
base = amps[min(amps)]
print("   amplitude ratios:", [f"{amps[k]/base:.3f}" for k in sorted(amps)])

cyc = {}
for lab, g in t.groupby("label"):
    k = aidx(lab)
    if k is None:
        continue
    up = g.unwrapped_phase.to_numpy(float)
    s = np.nanmax(up) - np.nanmin(up)
    if np.isfinite(s) and s > 0:
        cyc.setdefault(k, []).append(s / (2 * np.pi))

print("\ncycles completed by amplitude index:")
for k in sorted(cyc):
    arr = np.array(cyc[k])
    print(f"   a{k}: {arr.mean():.6f} cycles   spread={np.ptp(arr):.3e}   n={len(arr)}")

ks = sorted(cyc)
c0 = np.mean(cyc[ks[0]])
print("\n   cycle ratios  :", [f"{np.mean(cyc[k])/c0:.6f}" for k in ks])
print("   quartic needs :", [f"{amps[k]/base:.6f}" for k in ks], "  (Omega ~ A)")
print("   harmonic needs:", ["1.000000"] * len(ks), "  (Omega independent of A)")
