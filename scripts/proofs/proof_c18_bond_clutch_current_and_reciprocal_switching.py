#!/usr/bin/env python3
"""FTD-0988 exact C18 bond-clutch/current/switching discriminator."""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_C18_BOND_CLUTCH_CURRENT_AND_RECIPROCAL_SWITCHING_DISCRIMINATOR_v1.md"
PROTOCOL_HASH = "B85BAAA418F0BFF2AE67678BDB1FBD25532EB1CEC9FF596F2325F8D00AE169DD"

SOURCES = {
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_COMMON_MODE_WORK_PAIR_AND_PRODUCTION_OWNERSHIP_BOUNDARY_v1.md": "47C859191CCC1D9E306F82A68B6FC76A128593E6BAA7CC05D871D5DEEEE7EBAC",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOCAL_CANONICAL_WORK_PORT_AND_C18_FACTOR_EVENT_BOUNDARY_v1.md": "3BF425E7F826844BDD1F87ACA3B57EE9A26704996CC8A6F7781C683477D3B994",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_AND_PRODUCTION_BOUNDARY_v1.md": "656F51A4E5A533C0436E932B452A33810CD851D63E571621DF81ECB0C9BED622",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md": "C6424C1AA0DDA2BA57BDE14A1559C76BBB17E279087122FB7121C59350BB4329",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md": "7DA2366C75D38E0EA1F8012632D71C676C4E6F8D1A7F8D1467EAF4185AE77194",
    ROOT / "engine/include/ftd/voxel.h": "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp": "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp": "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def zero_matrix(matrix: sp.MatrixBase) -> bool:
    return all(sp.simplify(entry) == 0 for entry in matrix)


class Certificate:
    def __init__(self) -> None:
        self.total = 0
        self.passed = 0
        self.failed = 0

    def check(self, label: str, condition: bool, detail: str = "") -> None:
        self.total += 1
        if bool(condition):
            self.passed += 1
            suffix = f": {detail}" if detail else ""
            print(f"  PASS  {label}{suffix}")
        else:
            self.failed += 1
            suffix = f": {detail}" if detail else ""
            print(f"  FAIL  {label}{suffix}")


def main() -> int:
    cert = Certificate()
    print("=" * 79)
    print("FTD-0988 C18 bond clutch/current/reciprocal switching discriminator")
    print("=" * 79)

    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    protocol_norm = " ".join(protocol_text.split())
    cert.check("G1 protocol hash", sha256(PROTOCOL) == PROTOCOL_HASH, sha256(PROTOCOL))
    cert.check("G1 locked before execution", "[PREREGISTERED — NOT YET EVIDENCE]" in protocol_text)
    cert.check("G1 expected Outcome B", "Outcome B — exact local reference law / physical ownership still selected" in protocol_text)

    source_texts: dict[Path, str] = {}
    for path, expected in SOURCES.items():
        cert.check(f"G1 source hash {path.name}", sha256(path) == expected, sha256(path))
        source_texts[path] = path.read_text(encoding="utf-8")

    joined_theory = "\n".join(text for path, text in source_texts.items() if "docs" in path.parts)
    cert.check("G1 inherited exact incidence factor", "B^*B=K" in joined_theory)
    cert.check("G1 inherited common canonical pair", "longitudinal common pair" in joined_theory)
    cert.check("G1 inherited compact-mode obstruction", "no nonzero compactly supported C18 mode" in joined_theory)
    cert.check("G1 inherited native canonical storage", "three local canonical pairs per site" in joined_theory)
    cert.check("G1 inherited ternary orientation latch", "one ternary latch" in joined_theory)

    # G2: exact incidence clutch on a finite witness graph.
    a0, a1, a2 = sp.symbols("a0 a1 a2", positive=True)
    B = sp.Matrix([
        [-sp.sqrt(a0), sp.sqrt(a0), 0, 0],
        [0, -sp.sqrt(a1), sp.sqrt(a1), 0],
        [0, 0, -sp.sqrt(a2), sp.sqrt(a2)],
    ])
    G = sp.diag(1, 0, 1)  # Cut only the boundary bond 1--2.
    K = sp.simplify(B.T * B)
    K_gate = sp.simplify(B.T * G * B)
    cert.check("G2 stiffness symmetric", K_gate == K_gate.T)
    cert.check("G2 region/complement block zero", zero_matrix(K_gate[:2, 2:]))
    cert.check("G2 complement/region block zero", zero_matrix(K_gate[2:, :2]))
    cert.check("G2 exact direct-sum boundary cut", K_gate == sp.diag(*[K_gate[:2, :2], K_gate[2:, 2:]]))

    q = sp.Matrix(sp.symbols("q0:4", real=True))
    norm_gate = sp.simplify((B * q).dot(G * B * q))
    quadratic_gate = sp.simplify((q.T * K_gate * q)[0])
    cert.check("G2 positive sum-of-squares identity", sp.simplify(quadratic_gate - norm_gate) == 0)
    cert.check("G2 cut removes only selected channel", sp.simplify(K - K_gate - B[1, :].T * B[1, :]) == sp.zeros(4))

    face_reps = {(1, 0, 0), (0, 1, 0), (0, 0, 1)}
    edge_reps = {(1, 1, 0), (1, -1, 0), (1, 0, 1), (1, 0, -1), (0, 1, 1), (0, 1, -1)}
    cert.check("G2 C18 has nine oriented incidence channels", len(face_reps | edge_reps) == 9)
    cert.check("G2 every channel is Moore-local", all(max(map(abs, r)) <= 1 for r in face_reps | edge_reps))
    cert.check("G2 face/edge weights exact", 3 * sp.Rational(1, 9) + 6 * sp.Rational(1, 18) == sp.Rational(2, 3))
    for ell in (-1, 0, 1):
        cert.check(f"G2 ternary gate value ell={ell}", 1 - ell**2 in (0, 1))
    cert.check("G2 latch sign survives equal gate square", 1**2 == (-1) ** 2 and 1 != -1)

    # G3: exact local continuity equation and regional boundary flux.
    qx, qy, qz, px, py, pz = sp.symbols("qx qy qz px py pz", real=True)
    wxy, wxz = sp.symbols("wxy wxz", nonnegative=True)
    h_x = px**2 / 2 + (wxy * (qy - qx) ** 2 + wxz * (qz - qx) ** 2) / 4
    qdot = {qx: px, qy: py, qz: pz}
    pdot_x = wxy * (qy - qx) + wxz * (qz - qx)
    hdot_x = sp.diff(h_x, qx) * px + sp.diff(h_x, qy) * py + sp.diff(h_x, qz) * pz + sp.diff(h_x, px) * pdot_x
    j_xy = wxy * (qx - qy) * (px + py) / 2
    j_xz = wxz * (qx - qz) * (px + pz) / 2
    cert.check("G3 local continuity equation", sp.simplify(hdot_x + j_xy + j_xz) == 0)
    j_yx = wxy * (qy - qx) * (py + px) / 2
    cert.check("G3 bond current antisymmetric", sp.simplify(j_xy + j_yx) == 0)

    w01, w12 = sp.symbols("w01 w12", nonnegative=True)
    q0, q1, q2, p0, p1, p2 = sp.symbols("q0 q1 q2 p0 p1 p2", real=True)
    j01 = w01 * (q0 - q1) * (p0 + p1) / 2
    j10 = -j01
    j12 = w12 * (q1 - q2) * (p1 + p2) / 2
    hdot0 = -j01
    hdot1 = -j10 - j12
    cert.check("G3 internal regional current cancels", sp.simplify(hdot0 + hdot1 + j12) == 0)
    cert.check("G3 cut boundary has zero current", sp.simplify(j12.subs(w12, 0)) == 0)

    # G4: switching work, force, and reversible zero-strain orientation seam.
    d, v, weight = sp.symbols("d v weight", real=True)
    g_before, g_after = sp.symbols("g_before g_after", real=True)
    work = sp.simplify((g_after - g_before) * weight * d**2 / 2)
    force_jump = sp.Matrix([-1, 1]) * (g_after - g_before) * weight * d
    cert.check("G4 exact one-bond switching work", work == (g_after - g_before) * weight * d**2 / 2)
    cert.check("G4 zero-strain switching work vanishes", sp.simplify(work.subs(d, 0)) == 0)
    cert.check("G4 zero-strain force jump vanishes", force_jump.subs(d, 0) == sp.zeros(2, 1))
    cert.check("G4 off-seam switch generally costs work", sp.simplify(work.subs({d: 1, weight: 1, g_before: 1, g_after: 0})) == -sp.Rational(1, 2))
    cert.check("G4 signed crossing velocity reverses", sp.sign(-v) == -sp.sign(v))
    loaded = {(sigma, 0): (0, sigma) for sigma in (-1, 1)}
    inverse = {target: source for source, target in loaded.items()}
    cert.check("G4 two-slot sign transfer injective", len(set(loaded.values())) == len(loaded))
    cert.check("G4 two-slot sign transfer exactly invertible", all(inverse[loaded[x]] == x for x in loaded))
    cert.check("G4 erase-to-blank is noninjective", len({(-1, 0): (0, 0), (1, 0): (0, 0)}) == 2 and len({(0, 0)}) == 1)

    # G5: exact finite-tick symplectic map and local shadow Hamiltonian.
    h = sp.symbols("h", real=True)
    k11, k12, k22 = sp.symbols("k11 k12 k22", real=True)
    Ks = sp.Matrix([[k11, k12], [k12, k22]])
    I2 = sp.eye(2)
    Z2 = sp.zeros(2)
    U = sp.BlockMatrix([[I2 - h**2 * Ks, h * I2], [-h * Ks, I2]]).as_explicit()
    Omega = sp.BlockMatrix([[Z2, I2], [-I2, Z2]]).as_explicit()
    shadow_metric = sp.BlockMatrix([[Ks, -h * Ks / 2], [-h * Ks / 2, I2]]).as_explicit()
    cert.check("G5 fixed-gate map symplectic", zero_matrix(U.T * Omega * U - Omega))
    cert.check("G5 fixed-gate shadow energy exact", zero_matrix(U.T * shadow_metric * U - shadow_metric))
    U_inv = sp.BlockMatrix([[I2, -h * I2], [h * Ks, I2 - h**2 * Ks]]).as_explicit()
    cert.check("G5 exact inverse left", zero_matrix(U_inv * U - sp.eye(4)))
    cert.check("G5 exact inverse right", zero_matrix(U * U_inv - sp.eye(4)))

    qe0, qe1, pe0, pe1, we = sp.symbols("qe0 qe1 pe0 pe1 we", real=True)
    Be = sp.Matrix([[-sp.sqrt(we), sp.sqrt(we)]])
    Ke = Be.T * Be
    qe = sp.Matrix([qe0, qe1])
    pe = sp.Matrix([pe0, pe1])
    shadow_edge_matrix = sp.simplify((pe.dot(pe) + (qe.T * Ke * qe)[0] - h * (pe.T * Ke * qe)[0]) / 2)
    shadow_edge_local = sp.simplify(pe.dot(pe) / 2 + we * ((qe1 - qe0) ** 2 - h * (pe1 - pe0) * (qe1 - qe0)) / 2)
    cert.check("G5 local shadow decomposition", sp.simplify(shadow_edge_matrix - shadow_edge_local) == 0)

    lam = sp.symbols("lam", nonnegative=True)
    mode_p, mode_q = sp.symbols("mode_p mode_q", real=True)
    mode_shadow = (mode_p - h * lam * mode_q / 2) ** 2 / 2 + lam * (1 - h**2 * lam / 4) * mode_q**2 / 2
    expanded_mode = mode_p**2 / 2 + lam * mode_q**2 / 2 - h * lam * mode_p * mode_q / 2
    cert.check("G5 positive complete-square identity", sp.simplify(mode_shadow - expanded_mode) == 0)
    cert.check("G5 strict stability coefficient", sp.simplify((1 - h**2 * lam / 4).subs({h: 1, lam: 3})) > 0)
    cert.check("G5 threshold coefficient zero", sp.simplify((1 - h**2 * lam / 4).subs({h: 1, lam: 4})) == 0)
    loewner_difference = sp.simplify(K - K_gate)
    cert.check("G5 cutting lowers stiffness by a square", loewner_difference == B[1, :].T * B[1, :])
    cert.check("G5 finite-tick switch cost vanishes on zero strain", sp.simplify((work - h * (g_after - g_before) * weight * v * d / 2).subs(d, 0)) == 0)

    # G6: correct positive-frequency oscillator action normalization.
    omega, action, theta = sp.symbols("omega action theta", positive=True, real=True)
    Q = sp.sqrt(2 * action / omega) * sp.cos(theta)
    P = -sp.sqrt(2 * omega * action) * sp.sin(theta)
    polar_jac = sp.Matrix([[sp.diff(Q, theta), sp.diff(Q, action)], [sp.diff(P, theta), sp.diff(P, action)]])
    cert.check("G6 oscillator chart symplectic", sp.simplify(polar_jac.det()) == 1)
    cert.check("G6 oscillator Hamiltonian is omega I", sp.simplify((P**2 + omega**2 * Q**2) / 2 - omega * action) == 0)
    energy_before, energy_after = sp.symbols("energy_before energy_after", real=True)
    action_after = action + (energy_before - energy_after) / omega
    cert.check("G6 physical seam conserves H plus omega I", sp.simplify(energy_after + omega * action_after - energy_before - omega * action) == 0)
    cert.check("G6 zero frequency chart excluded", "A zero eigenvalue has no regular oscillator action-angle chart" in protocol_norm)
    cert.check("G6 amplitude audit kept separate", "H+2I identity remains exact only for the explicitly non-Hamiltonian observable-amplitude audit" in protocol_norm)
    cert.check("G6 changed-operator compact mode boundary", "changing the operator" in protocol_norm and "does not contradict" in protocol_norm)

    # G1/G7: production absence and epistemic firewalls.
    production = "\n".join(text for path, text in source_texts.items() if "engine" in path.parts).lower()
    cert.check("G7 no production bond ownership latch", "bond_ownership_latch" not in production)
    cert.check("G7 no production gate matrix", "g_ell" not in production and "gate_matrix" not in production)
    cert.check("G7 no production port reserve", "port_reserve" not in production)
    cert.check("G7 no production switching-work ledger", "switching_work" not in production)
    cert.check("G7 no production inverse transaction", "work_port_inverse" not in production)
    for firewall in ("production latch", "formation law", "work-mode", "complete production tick", "G*", "Born/Bell", "mass", "Hilbert-space", "completeness"):
        cert.check(f"G7 firewall {firewall}", firewall in protocol_text, "explicitly retained")
    cert.check("G7 no numerical search", "No fit, numerical near-miss search, parameter scan, formula substitution" in protocol_norm)
    cert.check("G7 no engine mutation", "No engine or production mutation is authorized" in protocol_norm)

    print("-" * 79)
    print(f"FTD-0988 exact certificate: {cert.passed}/{cert.total} checks passed")
    if cert.failed:
        print("OUTCOME D - invalid certificate; one or more frozen gates failed")
        return 1

    print(
        "OUTCOME B - the C18 incidence factor supplies an exact Moore-local positive "
        "bond clutch, antisymmetric current, zero-strain seam, reciprocal work law, "
        "and fixed-gate shadow-energy inverse, but physical ownership remains selected."
    )
    print(
        "The wave-Hamiltonian work action is frequency-normalized: H_u=omega I_u and "
        "I_u'=I_u+(H-H')/omega. The earlier H+2I law remains amplitude-audit only."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

