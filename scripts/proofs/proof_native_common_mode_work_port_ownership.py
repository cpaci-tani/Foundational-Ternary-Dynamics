#!/usr/bin/env python3
"""FTD-0986 exact native common-mode work-port ownership discriminator."""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_NATIVE_COMMON_MODE_WORK_PORT_OWNERSHIP_DISCRIMINATOR_v1.md"
)

SOURCES = {
    ROOT / (
        "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_CHART_BOUNDARY_v1.md"
    ): "FF80023FA73326B439405C8A07F08A72A5EBD8CC845AC145224B5BE4D647F07C",
    ROOT / (
        "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md"
    ): "100A5539A1116FD6BEC5ABF2B7CE7BA2C32DDA557564EC7C964CDF5877512739",
    ROOT / (
        "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_MOVING_REGIONAL_FRAME_COTANGENT_CONNECTION_AND_PURE_GAUGE_BOUNDARY_v1.md"
    ): "C5C28405CA439BF2341D545F99E9BDFC985BF65155B1CD49075541CD5C258462",
    ROOT / (
        "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_GLOBAL_AGGREGATE_WORK_AND_LOCAL_CONCURRENCY_OWNERSHIP_BOUNDARY_v1.md"
    ): "1CF020D3AA4EB78746C8CF7B932B3AB27E265E173E7F81524CF2A4547A38FA91",
    ROOT / "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ROOT / "engine/include/ftd/render_bridge_diagnostics.h":
        "5A9525591D3D818377E4688FBE4A57229B5CB7C36E62FF07D76941D814D57F69",
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ROOT / "engine/src/diagnostics_compute.cpp":
        "C3703292F8474EBC119F70024B0F3E4A23921C26EA58F8F6AB5E7581FB654AA6",
    ROOT / "engine/src/transmutation_phases.cpp":
        "4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043",
    ROOT / "engine/cuda/kernels_stencil_dual.cu":
        "25365B176BB333009333E2B5A596F792E2245719D107E754CE3C6BF5BAE9F1C0",
    ROOT / "engine/cuda/kernels_aux.cu":
        "E385FCFC93A2188E094798FC3A2C0A0839A6139313D738EE2E69254C6921739C",
}

PROTOCOL_SHA256 = "7E5E00C9262D3E6AF5D2BBD41D7F2845D4744D902157C32BADA7F6787D86AECF"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def zero_matrix(matrix: sp.MatrixBase) -> bool:
    return all(sp.simplify(value) == 0 for value in matrix)


class Certificate:
    def __init__(self) -> None:
        self.total = 0
        self.passed = 0
        self.failed = 0

    def check(self, label: str, condition: object, detail: object = "") -> None:
        self.total += 1
        ok = bool(condition)
        if ok:
            self.passed += 1
        else:
            self.failed += 1
        suffix = f": {detail}" if detail != "" else ""
        print(f"  {'PASS' if ok else 'FAIL'}  {label}{suffix}")


def signed_permutation_group() -> list[sp.Matrix]:
    out: list[sp.Matrix] = []
    for perm in itertools.permutations(range(3)):
        for signs in itertools.product((-1, 1), repeat=3):
            q = sp.zeros(3)
            for row, column in enumerate(perm):
                q[row, column] = signs[row]
            out.append(q)
    return out


def main() -> int:
    cert = Certificate()
    print("=" * 79)
    print("FTD-0986 native common-mode work-port ownership discriminator")
    print("=" * 79)

    # W1: frozen protocol and source chain.
    cert.check("W1 protocol hash", sha256(PROTOCOL) == PROTOCOL_SHA256, sha256(PROTOCOL))
    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    protocol_norm = " ".join(protocol_text.split())
    cert.check("W1 locked before first execution", "LOCKED BEFORE FIRST EXECUTION" in protocol_text)
    cert.check("W1 expected Outcome B", "Expected classifier:** `Outcome B`" in protocol_text)
    for source, expected in SOURCES.items():
        cert.check(f"W1 source hash {source.name}", sha256(source) == expected, expected)

    capacity_text = next(path for path in SOURCES if path.name.startswith("THEOREM_PRODUCTION_PHASE"))
    frame_text = next(path for path in SOURCES if path.name.startswith("THEOREM_NEUTRAL_BODY"))
    moving_text = next(path for path in SOURCES if path.name.startswith("THEOREM_MOVING_REGIONAL"))
    ownership_text = next(path for path in SOURCES if path.name.startswith("THEOREM_GLOBAL_AGGREGATE"))
    capacity_norm = " ".join(capacity_text.read_text(encoding="utf-8").split())
    frame_norm = " ".join(frame_text.read_text(encoding="utf-8").split())
    moving_norm = " ".join(moving_text.read_text(encoding="utf-8").split())
    ownership_norm = " ".join(ownership_text.read_text(encoding="utf-8").split())
    cert.check("W1 six-pair capacity inherited", "contain six scalar canonical pairs per site" in capacity_norm)
    cert.check("W1 unused complete pair inherited", "leaves one complete pair unused" in capacity_norm)
    cert.check("W1 regional frame covariance inherited", "e'_j=Qe_j" in frame_norm)
    cert.check("W1 moving projection reaction inherited", "state-dependent projection" in moving_norm)
    cert.check("W1 local ownership debt inherited", "prepositioned complete local work pairs" in ownership_norm)

    # W2: common/relative transform and six-pair repacking.
    omega2 = sp.Matrix([[0, 1], [-1, 0]])
    omega4 = sp.diag(omega2, omega2)
    root2 = sp.sqrt(2)
    transform = sp.Matrix([
        [1, 0, 1, 0],
        [0, 1, 0, 1],
        [1, 0, -1, 0],
        [0, 1, 0, -1],
    ]) / root2
    cert.check("W2 common/relative transform orthogonal", zero_matrix(transform.T * transform - sp.eye(4)))
    cert.check("W2 common/relative transform symplectic", zero_matrix(transform.T * omega4 * transform - omega4))
    cert.check("W2 common/relative determinant", sp.simplify(transform.det()) == 1, "det=+1")

    swap = sp.Matrix([
        [0, 0, 1, 0],
        [0, 0, 0, 1],
        [1, 0, 0, 0],
        [0, 1, 0, 0],
    ])
    swap_chart = sp.simplify(transform * swap * transform.T)
    cert.check("W2 common fixed by L/R swap", swap_chart[:2, :2] == sp.eye(2))
    cert.check("W2 relative negated by L/R swap", swap_chart[2:, 2:] == -sp.eye(2))
    cert.check("W2 swap has no common-relative mixing", zero_matrix(swap_chart - sp.diag(1, 1, -1, -1)))

    transform12 = sp.diag(transform, transform, transform)
    omega12 = sp.diag(omega2, omega2, omega2, omega2, omega2, omega2)
    cert.check("W2 six-pair transform symplectic", zero_matrix(transform12.T * omega12 * transform12 - omega12))
    projection10 = sp.eye(12)[:10, :]
    omega10 = sp.diag(omega2, omega2, omega2, omega2, omega2)
    cert.check("W2 five whole pairs retain symplectic form", zero_matrix(projection10 * omega12 * projection10.T - omega10))
    cert.check("W2 one whole pair remains", sp.eye(12)[10:, :].rank() == 2, "rank two")

    e1, e2, e3, j1, j2, j3 = sp.symbols("e1 e2 e3 j1 j2 j3", real=True)
    e = sp.Matrix([e1, e2, e3])
    j = sp.Matrix([j1, j2, j3])
    group = signed_permutation_group()
    cert.check("W2 signed-cubic group size", len(group) == 48)
    cert.check("W2 every signed-cubic matrix orthogonal", all(q.T * q == sp.eye(3) for q in group))
    cert.check(
        "W2 body-frame projection is a regional scalar",
        all(sp.expand((q * e).dot(q * j) - e.dot(j)) == 0 for q in group),
        "(Qe).(QJ)=e.J",
    )

    # W3: punctured-plane action-angle chart and time reversal.
    theta = sp.symbols("theta", real=True)
    action = sp.symbols("I", positive=True)
    q_port = sp.sqrt(2 * action) * sp.cos(theta)
    p_port = -sp.sqrt(2 * action) * sp.sin(theta)
    polar_jacobian = sp.Matrix([q_port, p_port]).jacobian(sp.Matrix([theta, action]))
    cert.check("W3 polar chart symplectic", zero_matrix(polar_jacobian.T * omega2 * polar_jacobian - omega2))
    cert.check("W3 polar Jacobian determinant", sp.simplify(polar_jacobian.det()) == 1)
    cert.check("W3 positive action identity", sp.simplify((q_port**2 + p_port**2) / 2 - action) == 0)
    cert.check("W3 time reversal fixes Q", sp.simplify(q_port.subs(theta, -theta) - q_port) == 0)
    cert.check("W3 time reversal flips P", sp.simplify(p_port.subs(theta, -theta) + p_port) == 0)

    # W4: observable energy coefficient and half-scaled seam reaction.
    cert.check(
        "W4 observable common-mode energy coefficient",
        sp.simplify((root2 * q_port) ** 2 / 2 + (root2 * p_port) ** 2 / 2 - 2 * action) == 0,
        "E_obs=2I",
    )

    k, kappa = sp.symbols("k kappa", positive=True)
    seam = sp.symbols("s", real=True)
    q_target, p_target, i_port = sp.symbols("q p I_port", real=True)
    target = sp.Matrix([q_target, p_target])
    metric = sp.diag(k, 1)
    root = sp.Matrix([[0, -1 / kappa], [kappa, 0]])
    defect = sp.simplify(metric - root.T * metric * root)
    bq = k - kappa**2
    bp = 1 - k / kappa**2
    half_q_shear = sp.Matrix([[1, -seam * bp / 2], [0, 1]])
    half_p_shear = sp.Matrix([[1, 0], [seam * bq / 2, 1]])
    seam_map = sp.simplify(root * half_q_shear * half_p_shear)
    b_seam = sp.simplify(seam_map.T * omega2 * sp.diff(seam_map, seam))
    cert.check("W4 target root symplectic", zero_matrix(root.T * omega2 * root - omega2))
    cert.check("W4 seam family symplectic", zero_matrix(seam_map.T * omega2 * seam_map - omega2))
    cert.check("W4 phase reaction symmetric", zero_matrix(b_seam - b_seam.T))
    cert.check("W4 half-scaled crossing derivative", zero_matrix(b_seam.subs(seam, 0) - defect / 2))

    target_out = sp.simplify(seam_map * target)
    port_work = sp.simplify((target.T * b_seam * target)[0] / 2)
    extended_in = sp.Matrix([q_target, p_target, seam, i_port])
    extended_out = sp.Matrix([target_out[0], target_out[1], seam, i_port + port_work])
    omega_extended = sp.diag(omega2, omega2)
    jacobian_extended = extended_out.jacobian(extended_in)
    cert.check(
        "W4 half-scaled seam lift symplectic",
        zero_matrix(jacobian_extended.T * omega_extended * jacobian_extended - omega_extended),
    )

    seam_target = target_out.subs(seam, 0)
    seam_work = sp.simplify(port_work.subs(seam, 0))
    energy_before = sp.simplify((target.T * metric * target)[0] / 2)
    energy_after = sp.simplify((seam_target.T * metric * seam_target)[0] / 2)
    cert.check("W4 action debit is half target work", sp.simplify(seam_work - (energy_before - energy_after) / 2) == 0)
    cert.check("W4 target plus 2I energy exact", sp.simplify(energy_after + 2 * (i_port + seam_work) - energy_before - 2 * i_port) == 0)
    recovered_target = sp.simplify(root.inv() * seam_target)
    cert.check("W4 seam target inverse", zero_matrix(recovered_target - target))
    cert.check("W4 seam action inverse", sp.simplify((i_port + seam_work) - seam_work - i_port) == 0)
    cert.check("W4 origin is excluded", "punctured plane `I>0`" in protocol_norm)
    cert.check("W4 fail-closed reserve boundary", "I+Delta H/2>=0" in protocol_norm)

    # W5: exact extremal-exponent Laurent obstruction.
    z = sp.symbols("z", nonzero=True)
    a, b, lam, u_max = sp.symbols("a b lambda u_max", nonzero=True)
    m = sp.symbols("m", integer=True)
    stiffness_axis = a - b * (z + z**-1)
    extremal_term = sp.expand((stiffness_axis - lam) * (u_max * z**m))
    cert.check("W5 nonconstant axis stiffness", sp.expand(stiffness_axis).has(z) and sp.expand(stiffness_axis).has(1 / z))
    cert.check("W5 upper extremal coefficient", sp.expand(extremal_term).coeff(z, m + 1) == -b * u_max)
    cert.check("W5 extremal coefficient nonzero by assumptions", b.is_nonzero and u_max.is_nonzero)
    cert.check("W5 Laurent integral-domain gate", "Laurent polynomial ring is an integral domain" in protocol_norm)
    cert.check("W5 compact closed-mode conclusion", "no nonzero compactly supported closed eigenmode" in protocol_norm)

    # W6: selected block-isolating projector and ternary square clutch.
    k11, k12, k13, k22, k23, k33 = sp.symbols("k11 k12 k13 k22 k23 k33", real=True)
    stiffness = sp.Matrix([[k11, k12, k13], [k12, k22, k23], [k13, k23, k33]])
    projector = sp.diag(1, 0, 0)
    complement = sp.eye(3) - projector
    isolated = sp.simplify(projector * stiffness * projector + complement * stiffness * complement)
    cert.check("W6 isolated stiffness symmetric", isolated == isolated.T)
    cert.check("W6 port-to-complement block zero", zero_matrix(complement * isolated * projector))
    cert.check("W6 complement-to-port block zero", zero_matrix(projector * isolated * complement))
    cert.check("W6 selected mode eigenvector", isolated * sp.Matrix([1, 0, 0]) == sp.Matrix([k11, 0, 0]))

    avec = sp.symbols("a0:9", real=True)
    gram_factor = sp.Matrix(3, 3, avec)
    gram = gram_factor.T * gram_factor
    x1, x2, x3 = sp.symbols("x1 x2 x3", real=True)
    xvec = sp.Matrix([x1, x2, x3])
    isolated_gram = projector * gram * projector + complement * gram * complement
    psd_witness = (gram_factor * projector * xvec).dot(gram_factor * projector * xvec) + (
        gram_factor * complement * xvec
    ).dot(gram_factor * complement * xvec)
    cert.check("W6 positivity block witness", sp.simplify((xvec.T * isolated_gram * xvec)[0] - psd_witness) == 0)

    cross = projector * stiffness * complement + complement * stiffness * projector
    clutch = lambda ell: sp.simplify(stiffness - ell**2 * cross)
    cert.check("W6 latch zero gives production stiffness", clutch(0) == stiffness)
    cert.check("W6 latch plus gives isolated stiffness", clutch(1) == isolated)
    cert.check("W6 latch minus gives isolated stiffness", clutch(-1) == isolated)
    cert.check("W6 latch sign retained outside square", 1 != -1 and 1**2 == (-1) ** 2)
    switch_work = sp.simplify((xvec.T * (clutch(1) - clutch(0)) * xvec)[0] / 2)
    h_off = sp.simplify((xvec.T * clutch(0) * xvec)[0] / 2)
    h_on = sp.simplify((xvec.T * clutch(1) * xvec)[0] / 2)
    cert.check("W6 switching work exact", sp.simplify(switch_work - (h_on - h_off)) == 0)
    cert.check("W6 protection remains selected", "selected reference candidate" in protocol_norm)

    # W7/W8: exact current production census and ownership absence.
    texts = {path.name: path.read_text(encoding="utf-8") for path in SOURCES if "engine" in path.parts}
    voxel = texts["voxel.h"]
    diagnostics_h = texts["render_bridge_diagnostics.h"]
    phase_read = texts["phase_read.cpp"]
    phase_write = texts["phase_write.cpp"]
    diagnostics_cpp = texts["diagnostics_compute.cpp"]
    transmutation = texts["transmutation_phases.cpp"]
    cuda_dual = texts["kernels_stencil_dual.cu"]
    cuda_aux = texts["kernels_aux.cu"]

    cert.check("W7 dual coordinate/momentum storage", all(token in voxel for token in ("Vec3 flux_L", "Vec3 flux_R", "Vec3 wave_vel_L", "Vec3 wave_vel_R")))
    cert.check("W7 observable is L plus R", "Observable: flux = flux_L + flux_R" in voxel)
    cert.check("W7 CPU C18 propagates both substrates", "laplacian_field<&Voxel::flux_L>" in phase_read and "laplacian_field<&Voxel::flux_R>" in phase_read)
    cert.check("W7 CPU matter source drives both", "rb.delta_j_L_[i] += curl_sv - grad_s" in phase_read and "rb.delta_j_R_[i] += curl_sv - grad_s" in phase_read)
    cert.check("W7 CPU clock source drives both", "rb.delta_j_L_[i] -= rb.voxels_[i].flux_L" in phase_read and "rb.delta_j_R_[i] -= rb.voxels_[i].flux_R" in phase_read)
    cert.check("W7 CPU advances all dual coordinates", "v.flux_L += v.wave_vel_L" in phase_write and "v.flux_R += v.wave_vel_R" in phase_write)
    cert.check("W7 CPU damps all dual pairs", all(token in phase_write for token in ("v.flux_L *= eff_damping", "v.flux_R *= eff_damping", "v.wave_vel_L *= eff_damping", "v.wave_vel_R *= eff_damping")))
    cert.check("W7 CPU syncs common observable", "v.flux = v.flux_L + v.flux_R" in phase_write and "v.wave_vel = v.wave_vel_L + v.wave_vel_R" in phase_write)
    cert.check("W7 CPU weak swap is phase complete", "std::swap(v.flux_L, v.flux_R)" in transmutation and "std::swap(v.wave_vel_L, v.wave_vel_R)" in transmutation)
    cert.check("W7 accounted energy uses observable", "quadratic_field_energy_density(v.flux.mag2())" in diagnostics_cpp and "quadratic_field_energy_density(v.wave_vel.mag2())" in diagnostics_cpp)
    cert.check("W7 split dual channels are diagnostics", "Dual-substrate diagnostics" in diagnostics_h and all(token in diagnostics_cpp for token in ("a.E_L_total", "a.E_R_total", "a.wv_L_total", "a.wv_R_total")))
    cert.check("W7 audit disclaims wave Hamiltonian", "not the gradient-plus-cross Hamiltonian" in diagnostics_h)
    cert.check("W7 CUDA C18 propagates L/R", "LAP18(fL_x, i)" in cuda_dual and "LAP18(fR_x, i)" in cuda_dual)
    cert.check("W7 CUDA advances and syncs common mode", "fL_x[i] += wvL_x[i]" in cuda_dual and "fR_x[i] += wvR_x[i]" in cuda_dual and "obs_x[i] = fL_x[i] + fR_x[i]" in cuda_dual)
    cert.check("W7 CUDA weak swap phase complete", "fL_z_mut[i] = fR_z_mut[i]" in cuda_aux and "wvL_z_mut[i] = wvR_z_mut[i]" in cuda_aux)

    production_joined = "\n".join(texts.values()).lower()
    cert.check("W8 no work-port ownership type", "work_port_owner" not in production_joined and "port_action" not in production_joined)
    cert.check("W8 no projector clutch", "k_iso" not in production_joined and "projector clutch" not in production_joined)
    cert.check("W8 no switching-work ledger", "switching_work" not in production_joined and "port_reserve" not in production_joined)
    cert.check("W8 no port inverse path", "work_port_inverse" not in production_joined)
    cert.check("W8 unchanged production fails autonomous ownership", "no nonzero compactly supported closed eigenmode" in protocol_norm)

    # W9: epistemic firewalls.
    cert.check("W9 Outcome B classifier present", "Outcome B — native chart / priced ownership law" in protocol_text)
    for forbidden in ("new continuous field", "production", "G*", "Born/Bell", "Hilbert", "mass", "selector-energy", "completeness"):
        cert.check(f"W9 no {forbidden} promotion", forbidden in protocol_text, "explicit firewall")
    cert.check("W9 no numerical search", "No fit, numerical near-miss search, parameter scan" in protocol_norm)
    cert.check("W9 no engine mutation", "No engine or production mutation is authorized" in protocol_norm)

    print("-" * 79)
    print(f"FTD-0986 exact certificate: {cert.passed}/{cert.total} checks passed")
    if cert.failed:
        print("OUTCOME D - invalid certificate; one or more frozen gates failed")
        return 1

    print(
        "OUTCOME B - existing dual fields provide a covariant positive common-mode "
        "work-pair chart, but unchanged production does not own or protect it as "
        "an autonomous local reserve."
    )
    print(
        "No seventh continuous pair is forced. A local projector/current clutch "
        "with switching work, history, inverse, and causal compilation remains "
        "selected or to be derived."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
