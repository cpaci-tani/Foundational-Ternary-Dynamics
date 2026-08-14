#!/usr/bin/env python3
"""FTD-0964 production phase-connection representability classifier.

This is a source-locked, proof-only audit. It does not execute or modify the
production engine and does not perform a numerical search.
"""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_CLASSIFIER_v1.md"
)

FROZEN = {
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/src/transmutation_phases.cpp":
        "4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043",
    "engine/src/render_bridge.cpp":
        "BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724",
    "engine/src/diagnostics_compute.cpp":
        "C3703292F8474EBC119F70024B0F3E4A23921C26EA58F8F6AB5E7581FB654AA6",
    "engine/include/ftd/lagrangian.h":
        "0225C75F34D1154CDF3783E73A86F051A3868E0E9087606E117411D75429350F",
    "engine/tests/test_dual_substrate.cpp":
        "B3DF5B36BD73D339E76609D9B5D1114398A61C804904C1B6FE2D1775071CF948",
    "engine/tests/test_symplectic_wave.cpp":
        "C8465563CADC245B3FB8AA19928E64D9D463BF293D29F1776F835633BA95EFF9",
}

PROTOCOL_SHA256 = (
    "B44C925D56BC66B3C9FCA2781AC29C86D0E8EADCF60DCA90FAA0BAD67B6A3E21"
)


class Certificate:
    def __init__(self) -> None:
        self.checks = 0
        self.passed = 0

    def check(self, label: str, condition: bool, detail: object = "") -> None:
        self.checks += 1
        if condition:
            self.passed += 1
            print(f"  PASS  {label}: {detail}")
        else:
            print(f"  FAIL  {label}: {detail}")

    @property
    def failed(self) -> int:
        return self.checks - self.passed


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def text(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def canonical_form(pair_count: int) -> sp.Matrix:
    block = sp.Matrix([[0, 1], [-1, 0]])
    return sp.diag(*([block] * pair_count))


def signed_permutation_group() -> list[sp.Matrix]:
    matrices: list[sp.Matrix] = []
    for perm in itertools.permutations(range(3)):
        for signs in itertools.product((-1, 1), repeat=3):
            matrix = sp.zeros(3)
            for row, column in enumerate(perm):
                matrix[row, column] = signs[row]
            matrices.append(matrix)
    return matrices


def main() -> int:
    cert = Certificate()
    print("=" * 79)
    print("FTD-0964 production phase-connection representability classifier")
    print("=" * 79)

    # G1: frozen source and protocol locks.
    cert.check(
        "G1 protocol hash",
        sha256(PROTOCOL) == PROTOCOL_SHA256,
        sha256(PROTOCOL),
    )
    for relative, expected in FROZEN.items():
        observed = sha256(ROOT / relative)
        cert.check(f"G1 hash {relative}", observed == expected, observed)

    voxel = text("engine/include/ftd/voxel.h")
    phase_read = text("engine/src/render_bridge_phases/phase_read.cpp")
    phase_write = text("engine/src/render_bridge_phases/phase_write.cpp")
    transmutation = text("engine/src/transmutation_phases.cpp")
    render_bridge = text("engine/src/render_bridge.cpp")
    diagnostics = text("engine/src/diagnostics_compute.cpp")
    lagrangian = text("engine/include/ftd/lagrangian.h")
    dual_test = text("engine/tests/test_dual_substrate.cpp")
    symplectic_test = text("engine/tests/test_symplectic_wave.cpp")
    protocol = PROTOCOL.read_text(encoding="utf-8")

    required_markers = {
        "voxel flux_L": (voxel, "Vec3 flux_L;"),
        "voxel flux_R": (voxel, "Vec3 flux_R;"),
        "voxel wave_vel_L": (voxel, "Vec3 wave_vel_L;"),
        "voxel wave_vel_R": (voxel, "Vec3 wave_vel_R;"),
        "observable flux sum": (voxel, "flux = flux_L + flux_R"),
        "read-only phase": (voxel, "Read-only\n  // diagnostic"),
        "Legendre momentum": (lagrangian, "The canonical momentum of the flux field."),
        "left independent acceleration":
            (phase_read, "rb.delta_j_L_[i] = lap_L * cw2;"),
        "right independent acceleration":
            (phase_read, "rb.delta_j_R_[i] = lap_R * cw2;"),
        "left canonical update":
            (phase_write, "v.wave_vel_L += rb.delta_j_L_[i] * rb.dt_;"),
        "right canonical update":
            (phase_write, "v.wave_vel_R += rb.delta_j_R_[i] * rb.dt_;"),
        "left coordinate update":
            (phase_write, "v.flux_L += v.wave_vel_L * rb.dt_;"),
        "right coordinate update":
            (phase_write, "v.flux_R += v.wave_vel_R * rb.dt_;"),
        "weak field swap": (transmutation, "std::swap(v.flux_L, v.flux_R);"),
        "proper phase increment": (transmutation, "v.phase += omega0 * delta_tau;"),
        "global tick increment": (render_bridge, "++tick_;"),
        "dual independent propagation test":
            (dual_test, "Independent wave propagation in L and R substrates"),
        "symplectic wave test":
            (symplectic_test, "Symplectic Leapfrog wave propagation"),
    }
    for label, (source, marker) in required_markers.items():
        cert.check(f"G1 marker {label}", marker in source, marker)

    # G2: exact pair-count capacity.
    production_pairs = 2 * 3
    target_pairs = 1 + 4
    cert.check("G2 dual field pair count", production_pairs == 6, production_pairs)
    cert.check("G2 target pair count", target_pairs == 5, target_pairs)
    cert.check(
        "G2 fixed-frame scalar capacity",
        production_pairs >= target_pairs,
        f"{production_pairs}>={target_pairs}",
    )
    cert.check(
        "G2 one complete pair remains unused",
        production_pairs - target_pairs == 1,
        production_pairs - target_pairs,
    )

    # G3: the frozen projection selects five whole pairs and is symplectic.
    omega_12 = canonical_form(6)
    omega_10 = canonical_form(5)
    projection = sp.zeros(10, 12)
    for i in range(10):
        projection[i, i] = 1
    induced = sp.simplify(projection * omega_12 * projection.T)
    cert.check("G3 packing rank", projection.rank() == 10, projection.rank())
    cert.check("G3 induced canonical form", induced == omega_10, induced)
    cert.check(
        "G3 unused pair is complete",
        projection[:, 10:12] == sp.zeros(10, 2),
        "z_R3",
    )
    cert.check(
        "G3 packing is explicitly selected-frame conditional",
        "for a fixed selected orthonormal frame" in protocol,
        "frame debt retained",
    )

    # G4: exact signed-cubic invariant-scalar obstruction.
    group = signed_permutation_group()
    cert.check("G4 signed-cubic group cardinality", len(group) == 48, len(group))
    average_v = sum(group, sp.zeros(3)) / sp.Integer(len(group))
    average_two_v = sp.diag(average_v, average_v)
    cert.check("G4 vector invariant projector rank", average_v.rank() == 0, average_v.rank())
    cert.check(
        "G4 two-vector invariant projector rank",
        average_two_v.rank() == 0,
        average_two_v.rank(),
    )
    cert.check(
        "G4 no site-local linear invariant scalar",
        average_two_v == sp.zeros(6),
        "Hom_Oh(V+V,1)=0",
    )

    # G5: production semantics establish six field pairs, not the gearbox.
    cert.check(
        "G5 wave velocity has canonical semantics",
        "wave_vel (the conjugate momentum)." in lagrangian,
        "Legendre pair",
    )
    cert.check(
        "G5 dual read step remains channel-separated",
        "lap_L" in phase_read and "lap_R" in phase_read,
        "L/R accelerations",
    )
    cert.check(
        "G5 dual write step remains channel-separated",
        "v.flux_L += v.wave_vel_L" in phase_write
        and "v.flux_R += v.wave_vel_R" in phase_write,
        "L/R kick-drift",
    )
    cert.check(
        "G5 observable register is derived",
        "v.flux = v.flux_L + v.flux_R;" in phase_write,
        "no L/R quotient",
    )

    # G6: phase/tau do not form an additional production canonical pair.
    production_state = voxel + render_bridge
    production_clock = transmutation + render_bridge
    cert.check("G6 diagnostic phase exists", "double phase = 0.0;" in voxel, "phase")
    cert.check("G6 phase momentum absent", "phase_momentum" not in production_state, "absent")
    cert.check("G6 phase is only incremented", "v.phase +=" in production_clock, "one-way write")
    cert.check(
        "G6 no production phase consumer in frozen tick",
        production_clock.count("v.phase") == 1,
        production_clock.count("v.phase"),
    )
    cert.check(
        "G6 tau is an accumulator not a pair",
        "double tau = 0.0;" in voxel and "v.tau += delta_tau;" in transmutation,
        "accumulator",
    )

    # G7: a whole-substrate swap is not the oriented exchange quarter-turn.
    swap = sp.Matrix([[0, 1], [1, 0]])
    quarter_turn = sp.Matrix([[0, -1], [1, 0]])
    identity_2 = sp.eye(2)
    cert.check("G7 weak swap squares to identity", swap**2 == identity_2, swap**2)
    cert.check("G7 weak swap determinant", swap.det() == -1, swap.det())
    cert.check(
        "G7 quarter-turn squares to minus identity",
        quarter_turn**2 == -identity_2,
        quarter_turn**2,
    )
    cert.check("G7 quarter-turn determinant", quarter_turn.det() == 1, quarter_turn.det())
    cert.check("G7 swap differs from quarter-turn", swap != quarter_turn, "distinct maps")

    # G8: connection-specific law is absent from the frozen production paths.
    production_law = phase_read + phase_write + transmutation + render_bridge + lagrangian
    forbidden_markers = (
        "oriented_phase_connection",
        "G_T=b_q",
        "complete-square phase connection",
        "calA(delta)G",
        "token loading",
    )
    for marker in forbidden_markers:
        cert.check(f"G8 production marker absent {marker}", marker not in production_law, "absent")
    cert.check(
        "G8 dual acceleration contains no L/R exchange term",
        "delta_j_L_[i] += rb.voxels_[i].flux_R" not in phase_read
        and "delta_j_R_[i] += rb.voxels_[i].flux_L" not in phase_read,
        "no cross-channel generator",
    )
    cert.check(
        "G8 no reverse production tick",
        "reverse_tick" not in render_bridge,
        "absent",
    )

    # G9: the audit has field norms, but no gearbox interaction/reserve terms.
    cert.check(
        "G9 dual field energies are diagnostic sums of squares",
        "quadratic_field_energy_density(v.flux_L.mag2())" in diagnostics
        and "quadratic_field_energy_density(v.wave_vel_R.mag2())" in diagnostics,
        "quadratic L/R telemetry",
    )
    for marker in ("connection_energy", "gearbox_reserve", "gearbox_backpressure"):
        cert.check(f"G9 energy term absent {marker}", marker not in diagnostics, "absent")

    # G10: explicit epistemic and scope firewall.
    scope_markers = (
        "Reference-state capacity is not production dynamics.",
        "does not license a production implementation",
        "Born-target blindness",
        "No fitted tolerance, numerical search, near-miss scan",
        "no new public continuous storage type is forced by local scalar capacity",
        "no site-local cubic-covariant linear scalar chart exists",
    )
    for marker in scope_markers:
        cert.check(f"G10 scope marker {marker[:42]}", marker in protocol, marker)
    cert.check(
        "G10 isolated EFT types excluded from production",
        "An isolated type or exact analyzer under `ftd::eft` does not count" in protocol,
        "reference/production separation",
    )

    print("-" * 79)
    print(f"checks={cert.checks} passed={cert.passed} failed={cert.failed}")
    if cert.failed:
        print("OUTCOME D - invalid certificate")
        return 1

    print("OUTCOME B - conditional chart capacity / production law absent")
    print("DUAL_FIELD_SCALAR_PAIR_CAPACITY=SUFFICIENT_CONDITIONAL_ON_SELECTED_FRAME")
    print("SITE_LOCAL_CUBIC_COVARIANT_LINEAR_SCALAR_CHART=OBSTRUCTED")
    print("CURRENT_PRODUCTION_COMPLETE_SQUARE_CONNECTION=ABSENT")
    print("NEW_PUBLIC_STORAGE_TYPE=NOT_FORCED_BY_CAPACITY_ALONE")
    print("PRODUCTION_IMPLEMENTATION=NOT_LICENSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
