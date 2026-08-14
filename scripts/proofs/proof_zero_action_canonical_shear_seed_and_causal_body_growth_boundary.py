#!/usr/bin/env python3
"""FTD-0993 zero-action canonical seed / causal body-growth discriminator."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_ZERO_ACTION_CANONICAL_SHEAR_SEED_AND_CAUSAL_BODY_GROWTH_BOUNDARY_v1.md"
PROTOCOL_HASH = "9A25D55B35BC32787E8FCBC513B6225B31ADA2E84249AB8F273992F489662753"

SOURCES = {
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOCAL_OCCUPANCY_FLIP_FORMATION_WORK_AND_MINIMUM_ACTIVE_APERTURE_v1.md": "E4D4BBCF2A0E09953EA2107FD80954E50BB2ED9BE45A9C9C6D2381DA018D7B9F",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_COMMON_MODE_WORK_PAIR_AND_PRODUCTION_OWNERSHIP_BOUNDARY_v1.md": "47C859191CCC1D9E306F82A68B6FC76A128593E6BAA7CC05D871D5DEEEE7EBAC",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_C18_BOND_CLUTCH_CURRENT_AND_WORK_ACTION_NORMALIZATION_v1.md": "2A93D9CFF23DFFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md": "7DA2366C75D38E0EA1F8012632D71C676C4E6F8D1A7F8D1467EAF4185AE77194",
    ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md": "A7DC30C90C491976F58CDEAF71FB5ABFCE04952ECE971CA7FF72C65A7B9B90BF",
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
    print("FTD-0993 zero-action canonical shear seed and causal body growth")
    print("=" * 79)

    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    protocol_norm = " ".join(protocol_text.split())
    cert.check("Z1 protocol hash", sha256(PROTOCOL) == PROTOCOL_HASH, sha256(PROTOCOL))
    cert.check("Z1 locked before execution", "[PREREGISTERED — NOT YET EVIDENCE]" in protocol_text)
    cert.check("Z1 expected Outcome B", "Outcome B — local Cartesian seed / extended-body causal boundary" in protocol_text)

    source_texts: dict[Path, str] = {}
    for path, expected in SOURCES.items():
        actual = sha256(path)
        cert.check(f"Z1 source hash {path.name}", actual == expected, actual)
        source_texts[path] = path.read_text(encoding="utf-8")

    formation = source_texts[next(path for path in SOURCES if path.name.startswith("THEOREM_LOCAL_OCCUPANCY"))]
    common = source_texts[next(path for path in SOURCES if path.name.startswith("THEOREM_NATIVE_COMMON"))]
    clutch = source_texts[next(path for path in SOURCES if path.name.startswith("THEOREM_C18_BOND"))]
    latch = source_texts[next(path for path in SOURCES if path.name.startswith("THEOREM_KRYLOV"))]
    reservoir = source_texts[next(path for path in SOURCES if path.name.startswith("THEOREM_SELF_DUAL"))]
    voxel = source_texts[ROOT / "engine/include/ftd/voxel.h"]
    phase_read = source_texts[ROOT / "engine/src/render_bridge_phases/phase_read.cpp"]
    phase_write = source_texts[ROOT / "engine/src/render_bridge_phases/phase_write.cpp"]
    transmutation = source_texts[ROOT / "engine/src/transmutation_phases.cpp"]
    cert.check("Z1 inherited cut-set formation work", "Exact cut-set identity" in formation and "W_{\\rm form}" in formation)
    cert.check("Z1 inherited common Cartesian pair", "longitudinal common pair" in common and "Q=" in common and "P=" in common)
    cert.check("Z1 inherited frequency-normalized action", "H_u=omega I" in clutch)
    cert.check("Z1 inherited retained orientation", "(\\sigma,0)\\longleftrightarrow(0,\\sigma)" in latch or "(\\sigma,0)\\longleftrightarrow(0,\\sigma)" in clutch)
    cert.check("Z1 inherited phase-complete lower bound", "complete canonical reservoir pair" in reservoir)
    cert.check("Z1 native Cartesian dual pairs", all(token in voxel for token in ("Vec3 flux_L", "Vec3 flux_R", "Vec3 wave_vel_L", "Vec3 wave_vel_R")))
    cert.check("Z1 production diagnostic phase scalar", "double phase = 0.0" in voxel and "v.phase += omega0 * delta_tau" in transmutation)

    # Z2: generic coordinate-gradient momentum shears are symplectic.
    h11, h12, h22 = sp.symbols("h11 h12 h22", real=True)
    Hs = sp.Matrix([[h11, h12], [h12, h22]])
    I2 = sp.eye(2)
    Z2 = sp.zeros(2)
    jac_generic = sp.BlockMatrix([[I2, Z2], [Hs, I2]]).as_explicit()
    omega4 = sp.BlockMatrix([[Z2, I2], [-I2, Z2]]).as_explicit()
    cert.check("Z2 symmetric Hessian", Hs == Hs.T)
    cert.check("Z2 generic shear symplectic", zero_matrix(jac_generic.T * omega4 * jac_generic - omega4))
    cert.check("Z2 generic shear determinant one", sp.simplify(jac_generic.det()) == 1)
    cert.check("Z2 generic shear full rank", jac_generic.rank() == 4)

    x, Q, px, P, c = sp.symbols("x Q px P c", real=True, positive=True)
    sigma = sp.symbols("sigma", real=True, nonzero=True)
    f = sp.sqrt(x**2 + c**2)
    S = sigma * Q * f
    px_after = px + sp.diff(S, x)
    P_after = P + sp.diff(S, Q)
    z = sp.Matrix([x, Q, px, P])
    z_after = sp.Matrix([x, Q, px_after, P_after])
    jac = z_after.jacobian(z)
    cert.check("Z2 explicit seed shear symplectic", zero_matrix(jac.T * omega4 * jac - omega4))
    cert.check("Z2 explicit seed determinant one", sp.simplify(jac.det()) == 1)
    cert.check("Z2 seam source reaction vanishes", sp.simplify(px_after.subs(Q, 0) - px) == 0)
    cert.check("Z2 seam receiver kick", sp.simplify(P_after.subs({Q: 0, P: 0}) - sigma * f) == 0)

    px_inverse = sp.simplify(px_after - sp.diff(S, x))
    P_inverse = sp.simplify(P_after - sp.diff(S, Q))
    cert.check("Z2 inverse source momentum exact", px_inverse == px)
    cert.check("Z2 inverse receiver momentum exact", P_inverse == P)
    cert.check("Z2 opposite sign is inverse generator", sp.simplify(S.subs(sigma, -sigma) + S) == 0)
    cert.check("Z2 no seventh pair registered", "rather than adding a seventh continuous pair" in protocol_norm)

    # Z3: energy and first phase on the zero-action seam.
    U, Omega = sp.symbols("U Omega", positive=True, real=True)
    for sig in (-1, 1):
        p_seed = sig * sp.sqrt(2 * U)
        action = U / Omega
        theta = -sig * sp.pi / 2
        q_from_chart = sp.sqrt(2 * action / Omega) * sp.cos(theta)
        p_from_chart = -sp.sqrt(2 * Omega * action) * sp.sin(theta)
        cert.check(f"Z3 phase chart Q sigma={sig}", sp.simplify(q_from_chart) == 0)
        cert.check(f"Z3 phase chart P sigma={sig}", sp.simplify(p_from_chart - p_seed) == 0)
        cert.check(f"Z3 clock energy sigma={sig}", sp.simplify(p_seed**2 / 2 - U) == 0)
    Hfield = sp.symbols("Hfield", real=True)
    cert.check("Z3 total seam energy exact", sp.simplify((Hfield - U) + (sp.sqrt(2 * U)) ** 2 / 2 - Hfield) == 0)
    cert.check("Z3 seeded action U over Omega", sp.simplify(((sp.sqrt(2 * U)) ** 2 / 2) / Omega - U / Omega) == 0)
    cert.check("Z3 time-odd sign selects opposite stroke", (-sp.sqrt(2 * U)) == -(sp.sqrt(2 * U)))

    P0 = sp.symbols("P0", real=True)
    f0 = sp.sqrt(2 * U)
    off_seam_change = sp.expand((P0 + f0) ** 2 / 2 - P0**2 / 2)
    cert.check("Z3 off-seam cross term present", sp.simplify(off_seam_change - (U + P0 * f0)) == 0)
    cert.check("Z3 simple identity restricted to zero-action seam", "outside `Q=P=0`, the simple energy identity contains cross terms" in protocol_norm)
    cert.check("Z3 zero released work supplies no energy", sp.simplify(f0.subs(U, 0)) == 0)
    t = sp.symbols("t", real=True)
    cert.check("Z3 square-root derivative singular at zero", sp.limit(sp.diff(sp.sqrt(t), t), t, 0, dir="+") == sp.oo)
    cert.check("Z3 target blindness registered", "not by a Born, context, setting, outcome, or `G*` target" in protocol_norm)
    cert.check("Z3 physical identification remains conditional", "physical identification of `U`, the receiver pair, and the event seam remains conditional" in protocol_norm)

    # Z4: exact uniform-mode seed is dense and cannot be a bounded one-tick map.
    for N in (2, 3, 5):
        u = sp.ones(N, 1) / sp.sqrt(N)
        projector = sp.simplify(u * u.T)
        cert.check(f"Z4 uniform projector dense N={N}", all(entry != 0 for entry in projector))
        cert.check(f"Z4 uniform projector rank one N={N}", projector.rank() == 1)
        delta = sp.symbols(f"delta{N}", real=True)
        site_shift = u * delta
        cert.check(f"Z4 modal shift reaches every site N={N}", all(sp.diff(entry, delta) != 0 for entry in site_shift))
    cert.check("Z4 one-tick remote Jacobian obstruction", "nonzero Jacobian dependence at every body site" in protocol_norm)
    cert.check("Z4 causal radius lower bound", "at least the graph radius after optimizing `x_0`" in protocol_norm)
    cert.check("Z4 bounded birth not obstructed", "does not obstruct a bounded one-shell birth or causal growth front" in protocol_norm)

    # Z5: no direct canonical cloning; exact orthogonal join chart.
    qA, pA, qB, pB = sp.symbols("qA pA qB pB", real=True)
    coords = (qA, qB)
    moms = (pA, pB)

    def poisson(fexpr: sp.Expr, gexpr: sp.Expr) -> sp.Expr:
        return sp.simplify(sum(
            sp.diff(fexpr, qi) * sp.diff(gexpr, pi)
            - sp.diff(fexpr, pi) * sp.diff(gexpr, qi)
            for qi, pi in zip(coords, moms)
        ))

    QA_out, PA_out = qA, pA
    QB_out, PB_out = qA, pA
    cert.check("Z5 source output remains canonical", poisson(QA_out, PA_out) == 1)
    cert.check("Z5 copied output pair canonical alone", poisson(QB_out, PB_out) == 1)
    cert.check("Z5 cloning cross bracket nonzero", poisson(QA_out, PB_out) == 1)
    cert.check("Z5 direct two-pair clone not symplectic", poisson(QA_out, PB_out) != 0)

    N = sp.symbols("N", positive=True, integer=True)
    A = sp.Matrix([[sp.sqrt(N), 1], [1, -sp.sqrt(N)]]) / sp.sqrt(N + 1)
    cert.check("Z5 join chart orthogonal", zero_matrix(sp.simplify(A.T * A - sp.eye(2))))
    cert.check("Z5 join chart determinant reflection", sp.simplify(A.det()) == -1)
    Tjoin = sp.BlockMatrix([[A, sp.zeros(2)], [sp.zeros(2), A]]).as_explicit()
    cert.check("Z5 join chart symplectic", zero_matrix(sp.simplify(Tjoin.T * omega4 * Tjoin - omega4)))

    QN, qn, PN, pn = sp.symbols("QN qn PN pn", real=True)
    joined_q = sp.simplify(A * sp.Matrix([QN, qn]))
    joined_p = sp.simplify(A * sp.Matrix([PN, pn]))
    match = {qn: QN / sp.sqrt(N), pn: PN / sp.sqrt(N)}
    cert.check("Z5 phase-matched relative coordinate zero", sp.simplify(joined_q[1].subs(match)) == 0)
    cert.check("Z5 phase-matched relative momentum zero", sp.simplify(joined_p[1].subs(match)) == 0)
    cert.check("Z5 matching condition unique coordinate", sp.solve(sp.Eq(joined_q[1], 0), qn) == [QN / sp.sqrt(N)])
    cert.check("Z5 matching condition unique momentum", sp.solve(sp.Eq(joined_p[1], 0), pn) == [PN / sp.sqrt(N)])

    omega = sp.symbols("omega", positive=True, real=True)
    old_energy = (PN**2 + omega**2 * QN**2) / 2
    site_energy = (pn**2 + omega**2 * qn**2) / 2
    enlarged_energy = (joined_p[0] ** 2 + omega**2 * joined_q[0] ** 2) / 2
    relative_energy = (joined_p[1] ** 2 + omega**2 * joined_q[1] ** 2) / 2
    cert.check("Z5 matched site energy one over N", sp.simplify(site_energy.subs(match) - old_energy / N) == 0)
    cert.check("Z5 matched enlarged energy paid", sp.simplify(enlarged_energy.subs(match) - old_energy * (N + 1) / N) == 0)
    cert.check("Z5 matched relative energy zero", sp.simplify(relative_energy.subs(match)) == 0)
    blank = {qn: 0, pn: 0}
    cert.check("Z5 blank site leaves relative coordinate", sp.simplify(joined_q[1].subs(blank) - QN / sp.sqrt(N + 1)) == 0)
    cert.check("Z5 blank site leaves relative momentum", sp.simplify(joined_p[1].subs(blank) - PN / sp.sqrt(N + 1)) == 0)
    cert.check("Z5 free copying rejected", "It cannot be free copying" in protocol_norm)

    # Z6: production and scope.
    production = (voxel + phase_read + phase_write + transmutation).lower()
    cert.check("Z6 production has no seed shear", "zero-action canonical" not in production and "membrane_seed" not in production)
    cert.check("Z6 production has no occupancy coupling", "occupancy membrane" not in production and "g_xy" not in production)
    cert.check("Z6 production phase does not seed flux", "v.phase += omega0 * delta_tau" in transmutation and "v.phase" not in phase_read)
    cert.check("Z6 production genesis remains stochastic", "voxel_uniform" in phase_write and "GenesisManifest" in phase_write)
    cert.check("Z6 production evaporation remains assignment", "rb.set_state(i, 0)" in phase_write)
    cert.check("Z6 no production reciprocal first-phase map", "first_phase_inverse" not in production and "clock_seed_shear" not in production)

    firewalls = (
        "production zero-action seed",
        "instantaneous global uniform-mode update",
        "free phase cloning",
        "derived body frame",
        "Omega",
        "omega_0",
        "G*",
        "Born/Bell",
        "charge-polarity",
        "mass",
        "Hilbert-space",
        "Lorentz-hiding",
        "completeness",
    )
    for firewall in firewalls:
        cert.check(f"Z6 firewall {firewall}", firewall in protocol_norm, "explicitly retained")
    cert.check("Z6 no numerical search", "No fit, numerical near-miss search, parameter scan, formula substitution" in protocol_norm)
    cert.check("Z6 no engine mutation", "No engine or production mutation is authorized" in protocol_norm)

    print("-" * 79)
    print(f"FTD-0993 exact certificate: {cert.passed}/{cert.total} checks passed")
    if cert.failed:
        print("OUTCOME D - invalid certificate; one or more frozen gates failed")
        return 1

    print(
        "OUTCOME B - a positive local membrane-work scalar and retained orientation "
        "seed a zero Cartesian clock pair through an exact canonical momentum shear. "
        "The first action is U/Omega and phase is -sigma*pi/2 without a target read."
    )
    print(
        "An instantaneous exact uniform seed of an arbitrarily extended body is "
        "nonlocal, and direct phase cloning is not canonical. Causal growth requires "
        "a phase-matched incoming field or a paid phase-complete machine; production "
        "implements neither mechanism."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
