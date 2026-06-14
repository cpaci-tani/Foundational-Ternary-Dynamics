#!/usr/bin/env python3
"""Analyzer for campaign_wave_sectors (FTD-0299)  [hardened v2].

Implements the adversarial-review must-fixes (M1-M8) before the pre-reg lock.

Arm 1 (light): compares the engine's measured omega_eig against the engine's OWN
   18-pt stencil eigenvalue omega_theory (dumped per row) -- the axial law
   2c|sin(k/2)| is only correct on <100> (M6). Reports c_eff isotropy across
   directions as the isotropy evidence; the k^4 anisotropy exponent is NOT
   measurable on L<=256 and is deliberately dropped (M7).

Arm 2 (sound): for each (seed,n) compares the kick arm against the kick=0 CONTROL
   (M3). Detrends + Hann-windows the COMPLEX density-mode series, takes the SIGNED
   FFT peak, and applies a genuine propagation test (one-sided power asymmetry +
   arg phase-ramp) (M1). Primary observable = continuous energy density e_k; the
   conserved state density rho_k must agree (M2). A non-NULL verdict requires a
   >=3-mode fit omega_s(k)=sqrt(Delta^2+(c_s k)^2): the intercept Delta sets
   GAPPED vs COMPRESSION (M4), and a branch-shape comparison vs omega_light(k)
   rejects a light-driven response (M5). Harmonics at ~2*omega_light are rejected.

Usage:  python scripts/exploration/analyze_wave_sectors.py [results_dir]
"""
import csv
import json
import math
import os
import sys
from collections import defaultdict

import numpy as np

C = 1.0 / math.sqrt(3.0)  # C_WAVE

# ---- frozen thresholds (pinned in pre-reg sec5 after the quick-check) ----
LIGHT_RELERR_BOUND = 0.02   # Arm 1: max |omega_eig-omega_theory|/omega_theory
ISO_BOUND = 0.02            # Arm 1: max |c_eff_ir/C - 1| across directions
M_MIN = 0.50                # condensate fraction required
M_DRIFT = 0.15              # |m_end-m_start|/m_start tolerance over the window
EXCLUDE_BINS = 2            # drop the lowest k bins (aperiodic relaxation)
PROM_MARGIN = 3.0           # kick peak prominence must exceed PROM_MARGIN * control peak prominence
ASYM_MIN = 0.30             # one-sided power asymmetry for "propagating"
PHASE_R2 = 0.80             # arg(z) linear-ramp R^2 for "propagating"
SEED_CV = 0.30              # cross-seed CV of omega_s for reproducibility
MIN_MODES = 3               # propagating modes required to fit a branch
LIGHT_CURVE_TOL = 0.10      # branch-shape match to omega_light(k) => light-driven (NULL)
HARM_TOL = 0.15             # reject peak within HARM_TOL of 2*omega_light
GAP_BINS = 2                # gap intercept must exceed GAP_BINS * (FFT resolution) to be GAPPED


def omega_light_axis(k):
    return 2.0 * C * abs(math.sin(k / 2.0))


# ===========================================================================
# Arm 1
# ===========================================================================
def analyze_light(path):
    rows = list(csv.DictReader(open(path)))
    by_dir = defaultdict(list)
    for r in rows:
        by_dir[r["direction"]].append(
            (float(r["kmag"]), float(r["omega_eig"]), float(r["omega_fft"]),
             float(r["omega_theory"]))
        )
    out = {"directions": {}}
    for d, pts in by_dir.items():
        pts.sort()
        kk = np.array([p[0] for p in pts])
        we = np.array([p[1] for p in pts])
        wf = np.array([p[2] for p in pts])
        wt = np.array([p[3] for p in pts])
        rel = np.abs(we - wt) / np.maximum(wt, 1e-12)            # M6: vs OWN stencil
        # leapfrog relation S4: omega_fft should equal 2*asin(omega_eig/2)
        lf_pred = 2.0 * np.arcsin(np.clip(we / 2.0, -1, 1))
        lf_rel = np.abs(wf - lf_pred) / np.maximum(lf_pred, 1e-12)
        out["directions"][d] = {
            "k": kk.tolist(), "omega_eig": we.tolist(), "omega_theory": wt.tolist(),
            "max_rel_err": float(np.max(rel)),
            "c_eff_ir": float(we[0] / kk[0]),
            "vg_ir": float(np.gradient(we, kk)[0]),
            "vg_zone": float(np.gradient(we, kk)[-1]),
            "leapfrog_max_rel": float(np.max(lf_rel)),
        }
    # isotropy: spread of IR phase speed across directions
    ceffs = [v["c_eff_ir"] for v in out["directions"].values()]
    iso_dev = max(abs(ce / C - 1.0) for ce in ceffs) if ceffs else float("nan")
    max_relerr = max(v["max_rel_err"] for v in out["directions"].values())
    out["iso_dev"] = iso_dev
    out["max_rel_err_all"] = max_relerr
    out["token"] = ("LIGHT-CONFIRMED"
                    if (max_relerr < LIGHT_RELERR_BOUND and iso_dev < ISO_BOUND)
                    else "LIGHT-DEVIATION")
    return out


# ===========================================================================
# Arm 2
# ===========================================================================
def _detrend_window(re, im):
    n = len(re)
    t = np.arange(n)
    # remove a quadratic trend from each component (condensate relaxation)
    for arr in (re, im):
        coef = np.polyfit(t, arr, 2)
        arr -= np.polyval(coef, t)
    w = np.hanning(n)
    return (re * w) + 1j * (im * w)


def _peak(z):
    """Signed-FFT peak of complex series -> (omega_s, prominence, asym)."""
    n = len(z)
    sp = np.fft.fft(z)
    freqs = np.fft.fftfreq(n)
    power = np.abs(sp) ** 2
    # zero DC + lowest EXCLUDE_BINS on both sides
    for b in range(EXCLUDE_BINS + 1):
        power[b] = 0.0
        power[-b] = 0.0
    peak = int(np.argmax(power))
    pos = power[power > 0]
    med = np.median(pos) if pos.size else 0.0
    prom = (power[peak] / med) if med > 0 else 0.0
    omega_s = abs(2.0 * math.pi * freqs[peak])
    # one-sided asymmetry: a traveling mode is single-signed in the spectrum
    pp = power[1:n // 2].sum()
    pn = power[n // 2 + 1:].sum()
    asym = abs(pp - pn) / (pp + pn) if (pp + pn) > 0 else 0.0
    return omega_s, prom, asym


def _phase_ramp_r2(z):
    """R^2 of a linear fit to unwrapped arg(z) -> propagation signature."""
    n = len(z)
    mag = np.abs(z)
    if np.median(mag) < 1e-9:
        return 0.0
    ph = np.unwrap(np.angle(z))
    t = np.arange(n)
    coef = np.polyfit(t, ph, 1)
    resid = ph - np.polyval(coef, t)
    ss_res = np.sum(resid ** 2)
    ss_tot = np.sum((ph - ph.mean()) ** 2)
    return 1.0 - ss_res / ss_tot if ss_tot > 0 else 0.0


def _series(rows, arm, n, re_key, im_key):
    s = [(int(r["tick"]), float(r[re_key]), float(r[im_key]))
         for r in rows if r["arm"] == arm and int(r["n"]) == n]
    s.sort()
    return (np.array([x[1] for x in s], dtype=float),
            np.array([x[2] for x in s], dtype=float))


def analyze_sound(path):
    rows = list(csv.DictReader(open(path)))
    seeds = sorted(set(r["seed"] for r in rows))
    modes = sorted(set(int(r["n"]) for r in rows))
    Nfft_res = None

    per_mode = defaultdict(list)  # n -> list of dict per seed
    for seed in seeds:
        srows = [r for r in rows if r["seed"] == seed]
        # m(t) stability over the kick window (per seed, any mode)
        for n in modes:
            ek_re, ek_im = _series(srows, "kick", n, "e_re", "e_im")
            ec_re, ec_im = _series(srows, "ctrl", n, "e_re", "e_im")
            rk_re, rk_im = _series(srows, "kick", n, "rho_re", "rho_im")
            mk = [float(r["m"]) for r in srows if r["arm"] == "kick" and int(r["n"]) == n]
            if len(ek_re) < 16 or len(ec_re) < 16:
                continue
            Nfft_res = 2.0 * math.pi / len(ek_re)
            m_arr = np.array(mk)
            invalid = (m_arr.min() < M_MIN or
                       (m_arr[0] > 0 and abs(m_arr[-1] - m_arr[0]) / m_arr[0] > M_DRIFT))
            kval = next(float(r["k"]) for r in srows if int(r["n"]) == n)

            zk = _detrend_window(ek_re.copy(), ek_im.copy())
            zc = _detrend_window(ec_re.copy(), ec_im.copy())
            zk_rho = _detrend_window(rk_re.copy(), rk_im.copy())

            om_k, prom_k, asym_k = _peak(zk)
            om_c, prom_c, _ = _peak(zc)
            r2 = _phase_ramp_r2(zk)
            om_rho, prom_rho, _ = _peak(zk_rho)

            om_light = omega_light_axis(kval)
            harmonic = abs(om_k - 2.0 * om_light) < HARM_TOL * max(2.0 * om_light, 1e-9)

            propagating = (prom_k > PROM_MARGIN * max(prom_c, 1.0)
                           and asym_k > ASYM_MIN and r2 > PHASE_R2 and not harmonic)
            rho_agrees = (prom_rho > PROM_MARGIN and
                          abs(om_rho - om_k) < 0.20 * max(om_k, 1e-9))

            per_mode[n].append({
                "seed": seed, "k": kval, "omega_s": om_k, "prom_kick": prom_k,
                "prom_ctrl": prom_c, "asym": asym_k, "phase_r2": r2,
                "omega_light": om_light, "harmonic": harmonic, "invalid": invalid,
                "propagating": propagating, "rho_agrees": rho_agrees,
                "m_min": float(m_arr.min()),
            })

    # aggregate
    if not per_mode:
        return {"verdict": "INVALID", "reason": "no usable series"}
    all_recs = [r for recs in per_mode.values() for r in recs]
    if np.mean([r["m_min"] for r in all_recs]) < M_MIN:
        return {"verdict": "INVALID",
                "reason": f"mean min-m {np.mean([r['m_min'] for r in all_recs]):.3f} < {M_MIN}",
                "per_mode": {k: v for k, v in per_mode.items()}}

    branch = []  # (k, omega_s) for modes that are reproducibly propagating + rho-agreeing
    for n, recs in sorted(per_mode.items()):
        oms = [r["omega_s"] for r in recs]
        prop_frac = np.mean([r["propagating"] for r in recs])
        rho_frac = np.mean([r["rho_agrees"] for r in recs])
        cv = (np.std(oms) / np.mean(oms)) if np.mean(oms) > 1e-9 else 9.9
        if prop_frac > 0.5 and rho_frac > 0.5 and cv < SEED_CV:
            branch.append((recs[0]["k"], float(np.median(oms))))

    detail = {"per_mode": {str(k): v for k, v in per_mode.items()},
              "branch": branch, "fft_res": Nfft_res}

    if len(branch) < MIN_MODES:
        detail["verdict"] = "NULL"
        detail["reason"] = f"{len(branch)} propagating modes < {MIN_MODES} required"
        return detail

    # multi-mode fit omega_s = sqrt(Delta^2 + (c_s k)^2)
    ks = np.array([b[0] for b in branch])
    ws = np.array([b[1] for b in branch])

    def model(k, delta, cs):
        return np.sqrt(np.maximum(delta ** 2 + (cs * k) ** 2, 0.0))
    try:
        from scipy.optimize import curve_fit
        popt, _ = curve_fit(model, ks, ws, p0=[0.0, C], maxfev=10000)
        delta, cs = float(abs(popt[0])), float(abs(popt[1]))
        resid = ws - model(ks, delta, cs)
        r2_fit = 1.0 - np.sum(resid ** 2) / np.sum((ws - ws.mean()) ** 2)
    except Exception:
        # linear fallback omega = a + b k
        b, a = np.polyfit(ks, ws, 1)
        delta, cs, r2_fit = float(abs(a)), float(abs(b)), float("nan")

    # branch-shape light discriminator (M5): does omega_s(k) track 2c|sin(k/2)|?
    wl = np.array([omega_light_axis(k) for k in ks])
    light_curve_dev = float(np.mean(np.abs(ws - wl) / np.maximum(wl, 1e-9)))
    light_driven = light_curve_dev < LIGHT_CURVE_TOL

    detail.update(delta=delta, c_s=cs, r2_fit=r2_fit,
                  light_curve_dev=light_curve_dev, light_driven=light_driven)

    if light_driven:
        detail["verdict"] = "NULL"
        detail["reason"] = f"branch tracks light (curve dev {light_curve_dev:.3f} < {LIGHT_CURVE_TOL})"
    elif delta > GAP_BINS * (Nfft_res or 0.0246):
        detail["verdict"] = "GAPPED-MODE"
    else:
        detail["verdict"] = "COMPRESSION-FOUND"
    return detail


# ===========================================================================
def main():
    rdir = sys.argv[1] if len(sys.argv) > 1 else "engine/results/wave_sectors"
    light_token = sound_token = "NOT-RUN"
    result = {}
    listing = os.listdir(rdir) if os.path.isdir(rdir) else []

    for fn in sorted(f for f in listing if f.startswith("wave_sectors_light")):
        r = analyze_light(os.path.join(rdir, fn))
        result[fn] = r
        light_token = r["token"]
        print(f"--- LIGHT {fn} : {r['token']} ---")
        for d, dd in r["directions"].items():
            print(f"  <{d}> max_rel_err(vs stencil)={dd['max_rel_err']:.2e} "
                  f"c_eff_ir={dd['c_eff_ir']:.5f} leapfrog_rel={dd['leapfrog_max_rel']:.2e}")
        print(f"  isotropy dev={r['iso_dev']:.2e}  (c=1/sqrt3={C:.5f})")

    for fn in sorted(f for f in listing if f.startswith("wave_sectors_sound")):
        r = analyze_sound(os.path.join(rdir, fn))
        result[fn] = r
        sound_token = r["verdict"]
        print(f"--- SOUND {fn} : {r['verdict']} ---"
              + (f"  ({r.get('reason','')})" if r.get("reason") else ""))
        if "branch" in r:
            print(f"  propagating modes: {r['branch']}")
        if "c_s" in r:
            print(f"  fit: Delta={r['delta']:.4f} c_s={r['c_s']:.4f} r2={r.get('r2_fit')}"
                  f" light_curve_dev={r.get('light_curve_dev'):.3f}")

    print()
    print(f"FTD-0299 SUMMARY: LIGHT={light_token}  SOUND={sound_token}")
    if os.path.isdir(rdir):
        with open(os.path.join(rdir, "wave_sectors_verdict.json"), "w") as jf:
            json.dump({"light_token": light_token, "sound_token": sound_token,
                       "detail": result}, jf, indent=2, default=str)


if __name__ == "__main__":
    main()
