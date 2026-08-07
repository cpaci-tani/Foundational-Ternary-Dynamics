#!/usr/bin/env python3
"""FTD-0419 full-Brillouin-zone one-loop matching checks.

The default run performs a small independent NumPy quadrature, verifies the
Clifford-trace reduction against explicit 4x4 gamma matrices, checks exact
finite-grid Ward transversality, and audits the high-N CUDA convergence table.
It does not search parameters or fit a physical target.

The quoted high-N coefficient belongs to the explicitly frozen QED_L-like
finite-volume step scheme (xi=1, all directions antiperiodic for fermions,
periodic photons with the single zero mode removed, first bosonic external
mode).  It is not advertised as a scheme-independent on-shell observable.
"""

from __future__ import annotations

import csv
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))

from scripts.constants import ALPHA_EFT  # noqa: E402


DATA = Path(__file__).with_name("_lorentz_full_bz_matching.csv")
C2 = 1.0 / 7.0
C = np.sqrt(C2)
R = np.array([1.0, C, C, C])
NU = R.copy()
PHOTON_WEIGHT = np.array([C2, 1.0, 1.0, 1.0])


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS  {label}")


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def momentum_grid(n: int, antiperiodic: bool) -> np.ndarray:
    twist = 0.5 if antiperiodic else 0.0
    values = 2 * np.pi * (np.arange(n) + twist) / n - np.pi
    mesh = np.meshgrid(values, values, values, values, indexing="ij")
    return np.array([axis.ravel() for axis in mesh])


def fermion_fields(p: np.ndarray, mass: float = 0.0):
    w = mass + np.sum(R[:, None] * (1.0 - np.cos(p)), axis=0)
    k = NU[:, None] * np.sin(p)
    den = w * w + np.sum(k * k, axis=0)
    return w, k, den


def bubble_trace(p: np.ndarray, q: np.ndarray, mu: int, nu: int,
                 mass: float = 0.0) -> np.ndarray:
    p2 = p + q[:, None]
    average = p + 0.5 * q[:, None]
    w1, k1, d1 = fermion_fields(p, mass)
    w2, k2, d2 = fermion_fields(p2, mass)
    am = R[mu] * np.sin(average[mu])
    bm = NU[mu] * np.cos(average[mu])
    an = R[nu] * np.sin(average[nu])
    bn = NU[nu] * np.cos(average[nu])

    d12 = -k1[mu] * bm
    d13 = np.sum(k1 * k2, axis=0)
    d14 = -k1[nu] * bn
    d23 = -bm * k2[mu]
    d24 = bm * bn if mu == nu else np.zeros_like(bm)
    d34 = -k2[nu] * bn

    trace_over_four = w1 * am * w2 * an
    trace_over_four -= (
        d12 * w2 * an + d13 * am * an + d14 * am * w2
        + d23 * w1 * an + d24 * w1 * w2 + d34 * w1 * am
    )
    trace_over_four += d12 * d34 - d13 * d24 + d14 * d23
    return 4.0 * trace_over_four / (d1 * d2)


def contact_trace(p: np.ndarray, mu: int, mass: float = 0.0) -> np.ndarray:
    w, k, den = fermion_fields(p, mass)
    return 4.0 * (w * R[mu] * np.cos(p[mu]) - k[mu] ** 2) / den


def self_slope_parts(n: int, ext: int) -> tuple[float, float]:
    loop = momentum_grid(n, antiperiodic=False)
    sh = np.sin(0.5 * loop)
    ch = np.cos(0.5 * loop)
    w = np.sum(R[:, None] * (1.0 - np.cos(loop)), axis=0)
    k = -NU[:, None] * np.sin(loop)
    den = w * w + np.sum(k * k, axis=0)
    delta = 4 * np.sin(loop[0] / 2) ** 2
    delta += C2 * 4 * np.sum(np.sin(loop[1:] / 2) ** 2, axis=0)
    mask = delta > 1e-30
    den = np.where(mask, den, 1.0)
    delta = np.where(mask, delta, 1.0)

    dw = -R[ext] * np.sin(loop[ext])
    dk = NU[ext] * np.cos(loop[ext])
    dden = 2 * w * dw + 2 * k[ext] * dk
    exchange = np.zeros(loop.shape[1])

    for mu in range(4):
        a = -R[mu] * sh[mu]
        b = NU[mu] * ch[mu]
        if mu == ext:
            da = R[mu] * ch[mu]
            db = NU[mu] * sh[mu]
            numer = (b * b - a * a) * k[ext] + 2 * a * b * w
            dnumer = (
                (2 * b * db - 2 * a * da) * k[ext]
                + (b * b - a * a) * dk
                + 2 * ((da * b + a * db) * w + a * b * dw)
            )
        else:
            numer = -(a * a + b * b) * k[ext]
            dnumer = -(a * a + b * b) * dk
        derivative = (dnumer * den - numer * dden) / den**2
        exchange += PHOTON_WEIGHT[mu] * derivative / delta

    exchange = np.where(mask, exchange, 0.0)
    tadpole = np.where(mask, PHOTON_WEIGHT[ext] / delta, 0.0)
    seagull_part = -0.5 * tadpole.mean()
    exchange_part = -exchange.mean() / NU[ext]
    return seagull_part, exchange_part


def small_quadrature(n: int) -> dict[str, float]:
    t_seag, t_exchange = self_slope_parts(n, 0)
    s_seag, s_exchange = self_slope_parts(n, 1)
    zt = t_seag + t_exchange
    zs = s_seag + s_exchange

    p = momentum_grid(n, antiperiodic=True)
    h = 2 * np.pi / n
    khat2 = 4 * np.sin(h / 2) ** 2
    qt = np.array([h, 0.0, 0.0, 0.0])
    qs = np.array([0.0, 0.0, h, 0.0])
    contact = contact_trace(p, 1).mean()
    ze = (bubble_trace(p, qt, 1, 1).mean() - contact) / khat2
    zb = (bubble_trace(p, qs, 1, 1).mean() - contact) / (C2 * khat2)
    ward_t = bubble_trace(p, qt, 0, 0).mean() - contact_trace(p, 0).mean()
    ward_s = bubble_trace(p, qs, 2, 2).mean() - contact_trace(p, 2).mean()
    return {
        "zt": zt,
        "zs": zs,
        "self_difference": zs - zt,
        "seagull_difference": s_seag - t_seag,
        "exchange_difference": s_exchange - t_exchange,
        "ze": ze,
        "zb": zb,
        "photon_difference": zb - ze,
        "match": (zs - zt) - 0.5 * (zb - ze),
        "ward_t": ward_t,
        "ward_s": ward_s,
    }


def gamma_matrices() -> list[np.ndarray]:
    identity = np.eye(2, dtype=complex)
    sigma1 = np.array([[0, 1], [1, 0]], dtype=complex)
    sigma2 = np.array([[0, -1j], [1j, 0]], dtype=complex)
    sigma3 = np.array([[1, 0], [0, -1]], dtype=complex)
    return [
        np.kron(sigma2, identity),
        np.kron(sigma1, sigma1),
        np.kron(sigma1, sigma2),
        np.kron(sigma1, sigma3),
    ]


def explicit_matrix_trace(p: np.ndarray, q: np.ndarray,
                          mu: int, nu: int) -> complex:
    gamma = gamma_matrices()
    identity = np.eye(4, dtype=complex)

    def propagator(momentum: np.ndarray) -> np.ndarray:
        w = np.sum(R * (1.0 - np.cos(momentum)))
        k = NU * np.sin(momentum)
        numerator = w * identity
        for rho in range(4):
            numerator -= 1j * gamma[rho] * k[rho]
        return numerator / (w * w + np.dot(k, k))

    average = p + 0.5 * q
    vm = R[mu] * np.sin(average[mu]) * identity \
        + 1j * NU[mu] * np.cos(average[mu]) * gamma[mu]
    vn = R[nu] * np.sin(average[nu]) * identity \
        + 1j * NU[nu] * np.cos(average[nu]) * gamma[nu]
    return np.trace(propagator(p) @ vm @ propagator(p + q) @ vn)


def load_rows() -> list[dict[str, str]]:
    with DATA.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def extrapolate(rows: list[dict[str, str]], cut: int) -> float:
    selected = [row for row in rows
                if row["scheme"] == "qedl_step" and int(row["N"]) >= cut]
    n = np.array([float(row["N"]) for row in selected])
    y = np.array([float(row["delta_match"]) for row in selected])
    design = np.column_stack([
        np.ones_like(n), np.log(n) / n**2, 1.0 / n**2, 1.0 / n**4
    ])
    return float(np.linalg.lstsq(design, y, rcond=None)[0][0])


def main() -> None:
    checks = 0

    gamma = gamma_matrices()
    for mu in range(4):
        for nu in range(4):
            anticommutator = gamma[mu] @ gamma[nu] + gamma[nu] @ gamma[mu]
            expected = 2 * np.eye(4) if mu == nu else np.zeros((4, 4))
            require(np.max(np.abs(anticommutator - expected)) < 1e-14,
                    f"G{mu}{nu} Euclidean gamma anticommutator")
            checks += 1

    p = np.array([0.31, -0.47, 0.28, -0.19])
    q = np.array([0.17, 0.11, -0.23, 0.07])
    scalar = bubble_trace(p[:, None], q, 1, 2)[0]
    matrix = explicit_matrix_trace(p, q, 1, 2)
    require(abs(scalar - matrix) < 2e-14,
            "T1 reduced Clifford trace equals the explicit 4x4 matrix trace")
    checks += 1

    small = small_quadrature(8)
    require(abs(small["zt"] - 0.0417233539969925) < 3e-13,
            "Q1 independent CPU quadrature reproduces CUDA Z_t at N=8")
    checks += 1
    require(abs(small["zs"] + 0.270808889300151) < 3e-13,
            "Q2 independent CPU quadrature reproduces CUDA Z_s at N=8")
    checks += 1
    require(abs(small["ze"] - 0.0473619290453485) < 3e-13
            and abs(small["zb"] - 0.0696692199709315) < 3e-13,
            "Q3 independent CPU quadrature reproduces both photon coefficients")
    checks += 1
    require(abs(small["match"] + 0.323685888759935) < 5e-13,
            "Q4 independent CPU quadrature reproduces delta_match at N=8")
    checks += 1
    require(abs(small["ward_t"]) < 1e-13 and abs(small["ward_s"]) < 1e-13,
            "W1 both longitudinal polarizations vanish on the finite grid")
    checks += 1
    require(small["seagull_difference"] < -0.23
            and small["exchange_difference"] < -0.07,
            "Q5 seagull and exchange independently drive the matter cone negative")
    checks += 1

    rows = load_rows()
    qedl = [row for row in rows if row["scheme"] == "qedl_step"]
    require(len(qedl) == 8 and max(abs(float(row["ward_t"])) for row in qedl) < 1e-13,
            "D1 all eight high-N QED_L rows retain Ward transversality")
    checks += 1
    values = [float(row["delta_match"]) for row in qedl]
    require(all(values[i + 1] < values[i] for i in range(len(values) - 1)),
            "D2 the frozen QED_L sequence converges monotonically from N=64 to 320")
    checks += 1
    limit64 = extrapolate(rows, 64)
    limit96 = extrapolate(rows, 96)
    require(-0.326970 < limit64 < -0.326968
            and -0.326970 < limit96 < -0.326968,
            "D3 independent asymptotic cuts give delta_match/g^2=-0.326969... in the frozen scheme")
    checks += 1
    require(abs(limit64 - limit96) < 5e-8,
            "D4 high-N scheme extrapolation is stable below 5e-8 across cuts")
    checks += 1

    mass_rows = [row for row in rows if row["scheme"] == "mass_witness"]
    mass_matches = [float(row["delta_match"]) for row in mass_rows]
    require(len(mass_rows) == 5
            and all(-0.27 < value < -0.25 for value in mass_matches),
            "D5 covariant positive-mass witnesses keep the threshold negative and O(10^-1)")
    checks += 1
    require(max(abs(float(row["ward_s"])) for row in mass_rows) < 1e-12,
            "D6 spatial Ward residuals remain negligible in the mass-regulated witnesses")
    checks += 1

    selected_threshold = abs(limit96 * ALPHA_EFT)
    require(0.0023 < selected_threshold < 0.0025,
            "P1 selected g^2=alpha wiring translates the scheme coefficient to about 2.39e-3")
    checks += 1
    require(selected_threshold / 2.571353e-9 > 9e5,
            "P2 the bare threshold exceeds FTD-0416's optimistic UV allowance by over 9e5")
    checks += 1

    audit = read("docs/theory/07_assessment/lorentz_recovery_causal_structure/AUDIT_LORENTZ_FULL_BZ_MATCHING.md")
    ledger = read("docs/theory/07_assessment/core_ledgers/LEDGER.md")
    require("scheme-specific" in audit and "not an on-shell observable" in audit,
            "S1 the numerical coefficient is not presented as scheme-independent physics")
    checks += 1
    require("COUNTERTERM-REQUIRED" in audit and "automatic cancellation" in audit,
            "S2 the scoped negative verdict is explicit")
    checks += 1
    require("FTD-0419" in ledger and "Next free ID FTD-0420" in ledger,
            "S3 the canonical ledger records FTD-0419 and advances the identifier")
    checks += 1

    print()
    print(f"FTD-0419 algebra/quadrature/source checks: {checks}/{checks} passed")
    print(f"QED_L XI=1  delta_match/g^2 = {limit96:.9f} (scheme-specific)")
    print(f"g^2=alpha    |delta_match| = {selected_threshold:.9e}")
    print("VERDICT       BARE COMMON CONE IS NOT AUTOMATICALLY ONE-LOOP CLOSED")


if __name__ == "__main__":
    main()
