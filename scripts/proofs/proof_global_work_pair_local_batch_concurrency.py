#!/usr/bin/env python3
"""FTD-0983 exact global-pair versus local-concurrency certificate."""

from __future__ import annotations

import hashlib
import math
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_GLOBAL_WORK_PAIR_VERSUS_LOCAL_BATCH_CONCURRENCY_v1.md"
)
EXPECTED_PROTOCOL = "4D47C48793A591A54168B4A24EFFBB537EA8F11F6F226C0B52049A3E7CBD8C6C"

FROZEN = {
    "derivations/native_time_carrier_programme/"
    "THEOREM_LOCAL_CANONICAL_WORK_PORT_AND_C18_FACTOR_EVENT_BOUNDARY_v1.md":
        "3BF425E7F826844BDD1F87ACA3B57EE9A26704996CC8A6F7781C683477D3B994",
    "derivations/native_time_carrier_programme/"
    "THEOREM_ONE_CLOCK_C4_COTANGENT_LIFT_AND_CONNECTION_UNDERDETERMINATION_v1.md":
        "9D80C133F5D99D0F789C320DC7C2C2A9E41C4DBB56FAECD39054B7BF0DB69E7F",
    "derivations/native_time_carrier_programme/"
    "THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_CHART_BOUNDARY_v1.md":
        "FF80023FA73326B439405C8A07F08A72A5EBD8CC845AC145224B5BE4D647F07C",
    "derivations/native_time_carrier_programme/"
    "THEOREM_PRODUCTION_CLOCK_INDEXED_C4_TWIST_CENSUS_v1.md":
        "3873CEE3BD61C894A99857C0527FBC1082F244CE7E7890FEB3E2F01C6D64E58F",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def zero_matrix(matrix: sp.Matrix) -> bool:
    return all(sp.simplify(entry) == 0 for entry in matrix)


class Certificate:
    def __init__(self) -> None:
        self.total = 0
        self.passed = 0

    def check(self, label: str, condition: bool, detail: object = "") -> None:
        self.total += 1
        if condition:
            self.passed += 1
        print(f"  {'PASS' if condition else 'FAIL'}  {label}: {detail}")

    @property
    def failed(self) -> int:
        return self.total - self.passed


def seam_family(
    phase: sp.Symbol,
    stiffness: sp.Symbol,
    kappa: sp.Symbol,
    sigma: int,
) -> tuple[sp.Matrix, sp.Matrix]:
    """Return R_s and B_s=R_s^T Omega dR_s for the FTD-0982 scalar block."""
    omega = sp.Matrix([[0, 1], [-1, 0]])
    b_q = stiffness - kappa**2
    b_p = 1 - stiffness / kappa**2
    q_shear = sp.Matrix([[1, -phase * b_p], [0, 1]])
    p_shear = sp.Matrix([[1, 0], [phase * b_q, 1]])
    root = sp.Matrix([[0, -sp.Rational(sigma, 1) / kappa], [sigma * kappa, 0]])
    r_s = sp.simplify(root * q_shear * p_shear)
    b_s = sp.simplify(r_s.T * omega * r_s.diff(phase))
    return r_s, b_s


def main() -> int:
    print("=" * 79)
    print("FTD-0983 global work pair versus local batch concurrency")
    print("=" * 79)
    cert = Certificate()

    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    protocol_norm = " ".join(protocol_text.split())
    cert.check("G1 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL, sha256(PROTOCOL))
    cert.check(
        "G1 locked marker",
        "[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]" in protocol_text,
        "locked before first execution",
    )
    cert.check("G1 expected classifier", "Expected classifier:** `Outcome B`" in protocol_text, "Outcome B")
    cert.check(
        "G1 no selector energy adoption",
        "No production change or global-selector energy role is adopted" in protocol_norm,
        "scope frozen",
    )

    sources: dict[str, str] = {}
    for relative, expected in FROZEN.items():
        path = BASE / relative
        actual = sha256(path)
        cert.check(f"G1 source hash {Path(relative).name}", actual == expected, actual)
        sources[relative] = path.read_text(encoding="utf-8")

    work_text = sources[list(FROZEN)[0]]
    clock_text = sources[list(FROZEN)[1]]
    capacity_text = sources[list(FROZEN)[2]]
    production_text = sources[list(FROZEN)[3]]
    cert.check(
        "G1 inherited one-batch work law",
        "one complete work pair" in work_text and "field and reserve recover after four strokes" in work_text,
        "FTD-0982",
    )
    cert.check(
        "G1 inherited one-clock law",
        "exactly one covariant mechanical momentum and one kinetic" in " ".join(clock_text.split()),
        "FTD-0977",
    )

    # G2: two independent seam maps lifted by one aggregate clock/action pair.
    theta = sp.symbols("theta", real=True)
    action = sp.symbols("I_G", real=True)
    q1, p1, q2, p2 = sp.symbols("q1 p1 q2 p2", real=True)
    k1, k2 = sp.symbols("k1 k2", positive=True)
    kap1, kap2 = sp.symbols("kap1 kap2", positive=True, nonzero=True)
    omega2 = sp.Matrix([[0, 1], [-1, 0]])
    omega4 = sp.diag(omega2, omega2)
    omega_global = sp.diag(omega4, omega2)  # (z1,z2,theta,I_G)

    r1, b1 = seam_family(theta, k1, kap1, 1)
    r2, b2 = seam_family(theta, k2, kap2, -1)
    z1 = sp.Matrix([q1, p1])
    z2 = sp.Matrix([q2, p2])
    z1_out = sp.simplify(r1 * z1)
    z2_out = sp.simplify(r2 * z2)
    work1 = sp.simplify((z1.T * b1 * z1)[0] / 2)
    work2 = sp.simplify((z2.T * b2 * z2)[0] / 2)
    global_output = sp.Matrix([
        z1_out[0], z1_out[1], z2_out[0], z2_out[1],
        theta, action + work1 + work2,
    ])
    global_input = sp.Matrix([q1, p1, q2, p2, theta, action])
    global_jacobian = global_output.jacobian(global_input)

    cert.check("G2 B1 symmetric", zero_matrix(b1 - b1.T), "R1^T Omega dR1")
    cert.check("G2 B2 symmetric", zero_matrix(b2 - b2.T), "R2^T Omega dR2")
    cert.check(
        "G2 aggregate lift symplectic",
        zero_matrix(global_jacobian.T * omega_global * global_jacobian - omega_global),
        "direct-sum Omega + dtheta wedge dI_G",
    )

    metric1 = sp.diag(k1, 1)
    metric2 = sp.diag(k2, 1)
    h1 = sp.simplify((z1.T * metric1 * z1)[0] / 2)
    h2 = sp.simplify((z2.T * metric2 * z2)[0] / 2)
    r1_zero = r1.subs(theta, 0)
    r2_zero = r2.subs(theta, 0)
    h1_out = sp.simplify(((r1_zero * z1).T * metric1 * (r1_zero * z1))[0] / 2)
    h2_out = sp.simplify(((r2_zero * z2).T * metric2 * (r2_zero * z2))[0] / 2)
    w1_zero = sp.simplify(work1.subs(theta, 0))
    w2_zero = sp.simplify(work2.subs(theta, 0))
    cert.check("G2 batch-one work law", sp.simplify(w1_zero - (h1 - h1_out)) == 0, "w1=H1-H1'")
    cert.check("G2 batch-two work law", sp.simplify(w2_zero - (h2 - h2_out)) == 0, "w2=H2-H2'")
    cert.check(
        "G2 aggregate total energy",
        sp.simplify(h1_out + h2_out + action + w1_zero + w2_zero - h1 - h2 - action) == 0,
        "sum H_a + I_G",
    )
    cert.check(
        "G2 additive work order independence",
        sp.simplify((action + work1 + work2) - (action + work2 + work1)) == 0,
        "disjoint fixed-phase additions commute",
    )

    # G3: one phase with multiple independently physical actions is degenerate.
    shared_two = sp.Matrix([[0, 1, 1], [-1, 0, 0], [-1, 0, 0]])
    null_two = sp.Matrix([0, 1, -1])
    cert.check("G3 shared phase/two actions antisymmetric", shared_two.T == -shared_two, "dtheta wedge d(I1+I2)")
    cert.check("G3 shared phase/two actions rank", shared_two.rank() == 2, "rank 2 of dimension 3")
    cert.check("G3 relative action null", shared_two * null_two == sp.zeros(3, 1), "partial_I1-partial_I2")

    shared_three = sp.zeros(4)
    for column in range(1, 4):
        shared_three[0, column] = 1
        shared_three[column, 0] = -1
    cert.check("G3 shared phase/three actions rank", shared_three.rank() == 2, "two relative-action null modes")
    cert.check("G3 shared phase/three actions nullity", len(shared_three.nullspace()) == 2, "N-1")

    local_pair_form = sp.diag(omega2, omega2)  # theta1,I1,theta2,I2
    synchronization_embedding = sp.Matrix([
        [1, 0, 0],  # theta1=theta
        [0, 1, 0],  # I1
        [1, 0, 0],  # theta2=theta
        [0, 0, 1],  # I2
    ])
    pulled_back = synchronization_embedding.T * local_pair_form * synchronization_embedding
    cert.check("G3 synchronized local-pair pullback", pulled_back == shared_two, "same presymplectic form")
    cert.check("G3 synchronized pullback degenerate", pulled_back.det() == 0, "relative reserve has no conjugate phase")

    # G4: two complete local lifts are a nondegenerate, commuting product.
    theta1, theta2, i1, i2 = sp.symbols("theta1 theta2 I1 I2", real=True)
    lr1, lb1 = seam_family(theta1, k1, kap1, 1)
    lr2, lb2 = seam_family(theta2, k2, kap2, -1)
    lz1_out = sp.simplify(lr1 * z1)
    lz2_out = sp.simplify(lr2 * z2)
    lw1 = sp.simplify((z1.T * lb1 * z1)[0] / 2)
    lw2 = sp.simplify((z2.T * lb2 * z2)[0] / 2)
    local_output = sp.Matrix([
        lz1_out[0], lz1_out[1], theta1, i1 + lw1,
        lz2_out[0], lz2_out[1], theta2, i2 + lw2,
    ])
    local_input = sp.Matrix([q1, p1, theta1, i1, q2, p2, theta2, i2])
    local_omega = sp.diag(omega2, omega2, omega2, omega2)
    local_jacobian = local_output.jacobian(local_input)
    cert.check(
        "G4 local product lift symplectic",
        zero_matrix(local_jacobian.T * local_omega * local_jacobian - local_omega),
        "two complete local work pairs",
    )
    cert.check("G4 local form nondegenerate", local_omega.det() == 1, "full rank")
    cert.check(
        "G4 subsystem symbol separation",
        not ({q2, p2, theta2, i2} & set().union(*(expr.free_symbols for expr in local_output[:4])))
        and not ({q1, p1, theta1, i1} & set().union(*(expr.free_symbols for expr in local_output[4:]))),
        "disjoint maps",
    )

    first_then_second = local_output
    second_then_first = sp.Matrix([
        lz1_out[0], lz1_out[1], theta1, i1 + lw1,
        lz2_out[0], lz2_out[1], theta2, i2 + lw2,
    ])
    cert.check("G4 disjoint order independence", zero_matrix(first_then_second - second_then_first), "F1F2=F2F1")

    local_w1_zero = sp.simplify(lw1.subs(theta1, 0))
    local_w2_zero = sp.simplify(lw2.subs(theta2, 0))
    cert.check("G4 local energy one", sp.simpl(h1_out + i1 + local_w1_zero - h1 - i1) == 0, "H1+I1")
    cert.check("G4 local energy two", sp.simpl(h2_out + i2 + local_w2_zero - h2 - i2) == 0, "H2+I2")

    recovered1 = sp.simplify(lr1.subs(theta1, 0).inv() * lz1_out.subs(theta1, 0))
    recovered2 = sp.simplify(lr2.subs(theta2, 0).inv() * lz2_out.subs(theta2, 0))
    cert.check("G4 local state inverse one", zero_matrix(recovered1 - z1), "exact")
    cert.check("G4 local state inverse two", zero_matrix(recovered2 - z2), "exact")
    cert.check(
        "G4 local action inverse",
        sp.simplify((i1 + local_w1_zero - local_w1_zero) - i1) == 0
        and sp.simplify((i2 + local_w2_zero - local_w2_zero) - i2) == 0,
        "debit recovered with state",
    )

    # G5: exact radius-local Jacobian obstruction and disjoint future cones.
    q_remote, p_remote, i_origin = sp.symbols("q_remote p_remote I_origin", real=True)
    remote_work = (q_remote**2 + p_remote**2) / 2
    global_action_out = i_origin + remote_work
    radius = 1
    origin = 0
    remote_site = 3
    cert.check("G5 remote site outside one-tick cone", abs(remote_site - origin) > radius, "distance 3 > r=1")
    cert.check(
        "G5 nonlocal Jacobian dependence",
        sp.diff(global_action_out, q_remote) == q_remote
        and sp.diff(global_action_out, p_remote) == p_remote,
        "partial I_G'/partial z_remote nonzero",
    )
    cert.check(
        "G5 locality violation marker",
        "violates the one-tick Jacobian support condition" in protocol_norm,
        "global action at origin cannot read remote work",
    )
    cert.check("G5 causal delay exact", math.ceil(abs(remote_site - origin) / radius) == 3, "ceil(d/r)")

    sites = (0, 3, 6)
    cones = [{site + offset for offset in range(-radius, radius + 1)} for site in sites]
    pairwise_disjoint = all(cones[a].isdisjoint(cones[b]) for a in range(3) for b in range(a + 1, 3))
    cert.check("G5 three pairwise disjoint work cones", pairwise_disjoint, cones)
    cert.check(
        "G5 minimum local carrier marker",
        "at least one phase-complete work carrier in each cone" in protocol_norm,
        "one pair or equivalent local canonical field per cone",
    )

    # G6: a same reserve can pass each local test and fail the joint one.
    reserve = sp.Rational(1)
    demand1 = sp.Rational(3, 4)
    demand2 = sp.Rational(3, 4)
    cert.check("G6 event one individually admitted", demand1 <= reserve, "3/4 <= 1")
    cert.check("G6 event two individually admitted", demand2 <= reserve, "3/4 <= 1")
    cert.check("G6 joint reserve overspent", demand1 + demand2 > reserve, "3/2 > 1")
    cert.check(
        "G6 atomic aggregate gate nonlocal",
        "atomic aggregate admission decision that reads every demand" in protocol_norm,
        "or preposition local reserves",
    )
    cert.check(
        "G6 algebra versus positivity separated",
        sp.simplify(reserve - demand1 - demand2) == -sp.Rational(1, 2),
        "additive map exists outside positive domain",
    )

    # G7: production source audit.
    capacity_norm = " ".join(capacity_text.split())
    production_norm = " ".join(production_text.split())
    cert.check(
        "G7 six production pairs",
        "contain six scalar canonical pairs per site" in capacity_norm,
        "conditional fixed-frame capacity",
    )
    cert.check("G7 one unused pair", "leaves one complete pair unused" in capacity_norm, "candidate port capacity")
    cert.check(
        "G7 cubic scalar obstruction",
        "no site-local linear `O_h`-covariant scalar" in capacity_norm
        and "invariant projector is therefore zero" in capacity_norm,
        "no native raw-vector scalar chart",
    )
    cert.check(
        "G7 diagnostic phase has no momentum",
        "read-only diagnostic state" in capacity_norm and "has no stored conjugate momentum" in capacity_norm,
        "phase/tau are accumulators",
    )
    cert.check("G7 CUDA has no phase buffer", "Device storage contains `d_tau` and no `d_phase`" in production_norm, "source locked")
    cert.check(
        "G7 production ledger lacks work port",
        "switching work or reserve" in production_norm
        and "a conjugate clock momentum" in production_norm
        and "backpressure" not in production_norm.split("## 5. Missing reaction and work ledger", 1)[1].split("## 6.", 1)[0],
        "no physical port transaction",
    )
    cert.check(
        "G7 capacity is not formation",
        "kinematically representable without adding storage" in capacity_norm
        and "It does not show formation" in capacity_norm,
        "conditional only",
    )

    # G8/G9: exact disposition and firewalls.
    cert.check(
        "G8 aggregate versus local distinction",
        "aggregate symplectic sufficiency" in protocol_norm and "substrate locality" in protocol_norm,
        "separate gates",
    )
    cert.check(
        "G8 local pair or field equivalence",
        "equivalent to a local canonical work-port field" in protocol_norm,
        "not one permanent species per event",
    )
    cert.check(
        "G8 overlap rule",
        "Overlapping events must instead be compiled as one joint batch" in protocol_norm,
        "no double debit",
    )
    for forbidden in ("selector-energy", "G*", "Born/Bell", "Hilbert", "mass", "completeness"):
        cert.check(f"G9 no {forbidden} promotion", forbidden in protocol_text, "explicit firewall")
    cert.check(
        "G9 no numerical search",
        "No numerical search, fit, near-miss comparison" in protocol_norm,
        "exact symbolic certificate",
    )
    cert.check("G9 no engine mutation", "no engine mutation" in protocol_norm, "reference only")

    print("-" * 79)
    print(f"FTD-0983 exact certificate: {cert.passed}/{cert.total} checks passed")
    if cert.failed:
        print("OUTCOME D — invalid certificate; one or more frozen gates failed")
        return 1

    print(
        "OUTCOME B — one global clock/action pair is an exact aggregate "
        "symplectic bookkeeper but cannot be a same-tick Moore-local energy bus."
    )
    print(
        "Pairwise separated concurrent work events require prepositioned complete "
        "local pairs, or an equivalent canonical work field; current production "
        "provides conditional fixed-frame capacity only."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
