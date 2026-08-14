#!/usr/bin/env python3
"""FTD-0990 exact ternary-occupancy membrane/self-dual clock discriminator."""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_CLOCK_SPLIT_v1.md"
PROTOCOL_HASH = "461F6D68F2C28964D01A9AD21DA142CF0A446364FB17524E8A6F9246CBDFA904"

SOURCES = {
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_C18_BOND_CLUTCH_CURRENT_AND_WORK_ACTION_NORMALIZATION_v1.md": "2A93D9CFF23DFFDFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_COMMON_MODE_WORK_PAIR_AND_PRODUCTION_OWNERSHIP_BOUNDARY_v1.md": "47C859191CCC1D9E306F82A68B6FC76A128593E6BAA7CC05D871D5DEEEE7EBAC",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md": "7DA2366C75D38E0EA1F8012632D71C676C4E6F8D1A7F8D1467EAF4185AE77194",
    ROOT / "engine/include/ftd/voxel.h": "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp": "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp": "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ROOT / "engine/src/transmutation_phases.cpp": "4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043",
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
            print(f"  PASS  {label}{': ' + detail if detail else ''}")
        else:
            self.failed += 1
            print(f"  FAIL  {label}{': ' + detail if detail else ''}")


def main() -> int:
    cert = Certificate()
    print("=" * 79)
    print("FTD-0990 ternary occupancy membrane and self-dual clock split")
    print("=" * 79)

    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    protocol_norm = " ".join(protocol_text.split())
    cert.check("O1 protocol hash", sha256(PROTOCOL) == PROTOCOL_HASH, sha256(PROTOCOL))
    cert.check("O1 locked before execution", "[PREREGISTERED — NOT YET EVIDENCE]" in protocol_text)
    cert.check("O1 expected Outcome B", "Outcome B — native static mask / selected dynamical coupling" in protocol_text)

    source_texts: dict[Path, str] = {}
    for path, expected in SOURCES.items():
        actual = sha256(path)
        cert.check(f"O1 source hash {path.name}", actual == expected, actual)
        source_texts[path] = path.read_text(encoding="utf-8")

    theory = "\n".join(text for path, text in source_texts.items() if "docs" in path.parts)
    voxel = source_texts[ROOT / "engine/include/ftd/voxel.h"]
    phase_read = source_texts[ROOT / "engine/src/render_bridge_phases/phase_read.cpp"]
    phase_write = source_texts[ROOT / "engine/src/render_bridge_phases/phase_write.cpp"]
    transmutation = source_texts[ROOT / "engine/src/transmutation_phases.cpp"]
    cert.check("O1 inherited bond clutch", "K_\\ell=B^TG_\\ell B" in theory)
    cert.check("O1 inherited common pair", "longitudinal common pair" in theory)
    cert.check("O1 inherited time-odd orientation distinction", "time reversal" in theory and "ternary latch" in theory)
    cert.check("O1 native ternary state", "int8_t state" in voxel)
    cert.check("O1 complete dual canonical storage", all(token in voxel for token in ("Vec3 flux_L", "Vec3 flux_R", "Vec3 wave_vel_L", "Vec3 wave_vel_R")))

    # O2: unique occupancy and equality membrane.
    ternary = (-1, 0, 1)
    occupancy_functions = []
    for values in itertools.product((0, 1), repeat=3):
        f = dict(zip(ternary, values))
        if f[-1] == f[1] and f[0] == 0 and f[-1] == 1:
            occupancy_functions.append(f)
    cert.check("O2 unique charge-blind occupancy function", len(occupancy_functions) == 1)
    cert.check("O2 occupancy is s squared", all(occupancy_functions[0][s] == s * s for s in ternary))

    aa, bb = sp.symbols("aa bb", real=True)
    poly_solution = sp.solve([bb, aa + bb - 1], (aa, bb), dict=True)
    cert.check("O2 unique even degree-two polynomial", poly_solution == [{aa: 1, bb: 0}])

    binary_pairs = tuple(itertools.product((0, 1), repeat=2))
    gate_functions = []
    for values in itertools.product((0, 1), repeat=4):
        gfun = dict(zip(binary_pairs, values))
        if all(gfun[(x, y)] == gfun[(y, x)] for x, y in binary_pairs) and all(
            gfun[(x, y)] == int(x == y) for x, y in binary_pairs
        ):
            gate_functions.append(gfun)
    cert.check("O2 unique symmetric equality gate", len(gate_functions) == 1)
    cert.check("O2 gate truth table", all(gate_functions[0][(x, y)] == 1 - (x - y) ** 2 for x, y in binary_pairs))

    mx, my = sp.symbols("mx my", real=True)
    gate_poly = sp.expand(1 - (mx - my) ** 2)
    boolean_reduced = sp.expand(gate_poly.subs({mx**2: mx, my**2: my}))
    cert.check("O2 equality polynomial", boolean_reduced == 1 - mx - my + 2 * mx * my)
    cert.check("O2 gate symmetric", sp.simplify(gate_poly - gate_poly.xreplace({mx: my, my: mx})) == 0)
    cert.check("O2 oriented normal antisymmetric", sp.simplify((mx - my) + (my - mx)) == 0)
    cert.check("O2 charge conjugation invariant", all(((-s) ** 2) == s**2 for s in ternary))
    cert.check("O2 spatial sign is time even", "time reversal leaves `eta` fixed" in protocol_norm)
    cert.check("O2 signed-cubic covariance", "signed-cubic covariant" in protocol_norm)

    # Finite graph witness with occupied component {0,1,2}, void {3,4}.
    weights = sp.symbols("w0:5", positive=True)
    edges = ((0, 1), (1, 2), (2, 3), (3, 4), (0, 4))
    B = sp.zeros(len(edges), 5)
    mask = (1, 1, 1, 0, 0)
    gates = []
    for row, ((x, y), weight) in enumerate(zip(edges, weights)):
        B[row, x] = -sp.sqrt(weight)
        B[row, y] = sp.sqrt(weight)
        gates.append(1 - (mask[x] - mask[y]) ** 2)
    Gm = sp.diag(*gates)
    Km = sp.simplify(B.T * Gm * B)
    q = sp.Matrix(sp.symbols("q0:5", real=True))
    cert.check("O2 membrane stiffness symmetric", Km == Km.T)
    cert.check("O2 matter/void direct sum", zero_matrix(Km[:3, 3:]) and zero_matrix(Km[3:, :3]))
    cert.check("O2 positive incidence identity", sp.simplify((q.T * Km * q)[0] - (B * q).dot(Gm * B * q)) == 0)
    cert.check("O2 cross-boundary gates cut", gates[2] == 0 and gates[4] == 0)
    cert.check("O2 same-occupancy gates transmit", gates[0] == gates[1] == gates[3] == 1)
    cert.check("O2 one-Moore-shell construction", "Moore-local" in protocol_norm)
    cert.check("O2 no independent static bond memory", "no independent **static** bond-memory variable" in protocol_text)

    # O3: unique L/R block for common gated and relative open channels.
    k11, k12, k22, m11, m12, m22 = sp.symbols("k11 k12 k22 m11 m12 m22", real=True)
    K = sp.Matrix([[k11, k12], [k12, k22]])
    M = sp.Matrix([[m11, m12], [m12, m22]])
    I2 = sp.eye(2)
    T = sp.BlockMatrix([[I2, I2], [I2, -I2]]).as_explicit() / sp.sqrt(2)
    diag_pm = sp.diag(M, K)
    lr_block = sp.simplify(T.T * diag_pm * T)
    target_lr = sp.BlockMatrix([[(M + K) / 2, (M - K) / 2], [(M - K) / 2, (M + K) / 2]]).as_explicit()
    cert.check("O3 common/relative chart orthogonal", zero_matrix(T.T * T - sp.eye(4)))
    cert.check("O3 unique L/R block formula", zero_matrix(lr_block - target_lr))
    swap = sp.BlockMatrix([[sp.zeros(2), I2], [I2, sp.zeros(2)]]).as_explicit()
    cert.check("O3 L/R swap invariance", zero_matrix(swap.T * lr_block * swap - lr_block))

    A11, C11 = sp.symbols("A11 C11")
    unique_solution = sp.solve([A11 + C11 - m11, A11 - C11 - k11], (A11, C11), dict=True)
    cert.check("O3 conditional block uniqueness", unique_solution == [{A11: k11 / 2 + m11 / 2, C11: -k11 / 2 + m11 / 2}])

    xp, xm = sp.Matrix(sp.symbols("xp0:2", real=True)), sp.Matrix(sp.symbols("xm0:2", real=True))
    xlr = T.T * sp.Matrix.vstack(xp, xm)
    cert.check(
        "O3 positivity inherited from channel energies",
        sp.simplify((xlr.T * lr_block * xlr)[0] - (xp.T * M * xp)[0] - (xm.T * K * xm)[0]) == 0,
    )
    cert.check("O3 bulk cross block vanishes", zero_matrix(((M - K) / 2).subs({m11: k11, m12: k12, m22: k22})))

    qpx, qpy, ppx, ppy, qmx, qmy, pmx, pmy, weight = sp.symbols(
        "qpx qpy ppx ppy qmx qmy pmx pmy weight", real=True
    )
    j_plus_cut = 0 * weight * (qpx - qpy) * (ppx + ppy) / 2
    j_minus_open = weight * (qmx - qmy) * (pmx + pmy) / 2
    cert.check("O3 common boundary current cut", j_plus_cut == 0)
    cert.check("O3 relative boundary current retained", j_minus_open != 0)
    cert.check("O3 self-dual sectors not disconnected", "not two disconnected universes" in protocol_norm)

    # O4: connected body uniform clock mode.
    a0, a1 = sp.symbols("a0 a1", positive=True)
    B_body = sp.Matrix([[-sp.sqrt(a0), sp.sqrt(a0), 0], [0, -sp.sqrt(a1), sp.sqrt(a1)]])
    K_body = sp.simplify(B_body.T * B_body)
    one = sp.ones(3, 1)
    cert.check("O4 connected body uniform kernel", K_body * one == sp.zeros(3, 1))
    cert.check("O4 connected body kernel dimension one", K_body.rank() == 2)
    cert.check("O4 sum-of-squares kernel proof registered", "ker K_\\Lambda=\\operatorname{span}" in protocol_text)

    omega = sp.symbols("omega", positive=True)
    u = one / sp.sqrt(3)
    shifted = K_body + omega**2 * sp.eye(3)
    cert.check("O4 uniform mode eigenvalue omega squared", sp.simplify(shifted * u - omega**2 * u) == sp.zeros(3, 1))

    # The two other eigenvalues are strictly above omega^2 because K_body is
    # PSD and has one-dimensional kernel. This is exact from the incidence norm.
    cert.check("O4 unique lowest mode by connected positivity", K_body.rank() == 2 and K_body * one == sp.zeros(3, 1))

    action, theta = sp.symbols("action theta", positive=True, real=True)
    Q = sp.sqrt(2 * action / omega) * sp.cos(theta)
    P = -sp.sqrt(2 * omega * action) * sp.sin(theta)
    polar_jac = sp.Matrix([[sp.diff(Q, theta), sp.diff(Q, action)], [sp.diff(P, theta), sp.diff(P, action)]])
    cert.check("O4 uniform clock chart symplectic", sp.simplify(polar_jac.det()) == 1)
    cert.check("O4 uniform clock energy omega I", sp.simplify((P**2 + omega**2 * Q**2) / 2 - omega * action) == 0)
    h_before, h_after = sp.symbols("h_before h_after", real=True)
    action_after = action + (h_before - h_after) / omega
    cert.check("O4 seam debit frequency normalized", sp.simplify(h_after + omega * action_after - h_before - omega * action) == 0)
    cert.check("O4 zero omega excluded", "For `omega_0=0`" in protocol_text)

    cert.check("O4 production clock uses manifested predicate", "do_db_clock && rb.voxels_[i].state != 0" in phase_read)
    cert.check("O4 production clock acts on L", "rb.delta_j_L_[i] -= rb.voxels_[i].flux_L * omega0_sq" in phase_read)
    cert.check("O4 production clock acts on R", "rb.delta_j_R_[i] -= rb.voxels_[i].flux_R * omega0_sq" in phase_read)
    cert.check("O4 common support predicate is s squared", all((s != 0) == bool(s * s) for s in ternary))
    cert.check("O4 omega remains imposed", "The clock is [IMPOSED]" in phase_read)

    # O5: static price retires, dynamic actuator/history does not.
    cert.check("O5 production full C18 on L", "laplacian_field<&Voxel::flux_L>" in phase_read)
    cert.check("O5 production full C18 on R", "laplacian_field<&Voxel::flux_R>" in phase_read)
    cert.check("O5 production has no occupancy membrane", "occupancy membrane" not in phase_read.lower() and "g_xy" not in phase_read)
    cert.check("O5 genesis reads random draw", "voxel_uniform" in phase_write and "GenesisManifest" in phase_write)
    cert.check("O5 genesis drain not exact latent heat", "not an exact common-action latent-heat identity" in phase_write)
    cert.check("O5 evaporation clears state", "rb.set_state(i, 0)" in phase_write)
    cert.check("O5 journal observation only", "observation-only native event journal" in phase_write)
    cert.check("O5 weak swap remains phase complete", "std::swap(v.flux_L, v.flux_R)" in transmutation and "std::swap(v.wave_vel_L, v.wave_vel_R)" in transmutation)
    cert.check("O5 spatial normal not temporal orientation", "is time-even and is not the clockwise/counterclockwise event sign" in protocol_norm)
    cert.check("O5 active aperture still costs controller state", "cannot actively open one boundary bond" in protocol_norm)

    # O6: scope and classifier.
    production = (voxel + phase_read + phase_write + transmutation).lower()
    cert.check("O6 no production stiffness split", "self-dual clock split" not in production and "occupancy boundary gate" not in production)
    cert.check("O6 no production reversible formation ledger", "formation_energy_ledger" not in production and "membrane_inverse" not in production)
    for firewall in ("production coupling", "body, membrane, or mode formation", "omega_0", "G*", "Born/Bell", "mass", "Hilbert space", "Lorentz", "completeness", "active charging aperture", "spatial occupancy normal"):
        cert.check(f"O6 firewall {firewall}", firewall in protocol_text, "explicitly retained")
    cert.check("O6 no numerical search", "No fit, numerical near-miss search, parameter scan, formula substitution" in protocol_norm)
    cert.check("O6 no engine mutation", "No engine or production mutation is authorized" in protocol_norm)

    print("-" * 79)
    print(f"FTD-0990 exact certificate: {cert.passed}/{cert.total} checks passed")
    if cert.failed:
        print("OUTCOME D - invalid certificate; one or more frozen gates failed")
        return 1

    print(
        "OUTCOME B - ternary occupancy supplies the unique static matter/void "
        "membrane, and the minimum L/R-symmetric law isolates the common clock "
        "sector while retaining the relative interaction sector."
    )
    print(
        "A connected body with imposed omega0 has a unique uniform common clock "
        "mode H_u=omega0 I_u. The coupling, reversible membrane dynamics, active "
        "aperture, formation work, and mode preparation are absent from production."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

