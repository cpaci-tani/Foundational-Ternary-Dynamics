#!/usr/bin/env python3
"""FTD-0978 exact source census for the production clock/C4 route."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_PRODUCTION_CLOCK_INDEXED_C4_TWIST_CENSUS_v1.md"
)
EXPECTED_PROTOCOL = "F194A9148909D4C8DDC0057266DC56CA93A1316335180C47541FADA3CE9F4A83"

FROZEN = {
    "engine/include/ftd/voxel.h": "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/include/ftd/term_toggles.h": "2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA",
    "engine/src/transmutation_phases.cpp": "4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043",
    "engine/src/render_bridge.cpp": "BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724",
    "engine/src/render_bridge_phases/phase_read.cpp": "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp": "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/include/ftd/gpu_buffers.h": "92AE9190121D278AE5FBA0A74F708063D566DBF0B4036B2DCADC1CAC41A535DF",
    "engine/cuda/gpu_buffers.cu": "9154CC003D3F8E25FC5BB3EC608417F464C09894440C97572004C9C0489FDDA4",
    "engine/cuda/gpu_engine.cu": "302D93022251F53668BFF556088AECAE3F44D1BF1FF5CCE469B4FDDD98D4A96D",
    "engine/cuda/kernels_aux.cu": "E385FCFC93A2188E094798FC3A2C0A0839A6139313D738EE2E69254C6921739C",
    "engine/cuda/kernels_stencil_dual.cu": "25365B176BB333009333E2B5A596F792E2245719D107E754CE3C6BF5BAE9F1C0",
    "engine/include/ftd/render_bridge_diagnostics.h": "5A9525591D3D818377E4688FBE4A57229B5CB7C36E62FF07D76941D814D57F69",
    "engine/src/energy_ledger_compute.cpp": "2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def normalized(text: str) -> str:
    return " ".join(text.split())


def section(text: str, start: str, end: str) -> str:
    i = text.index(start)
    j = text.index(end, i + len(start))
    return text[i:j]


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


def main() -> int:
    print("=" * 79)
    print("FTD-0978 production clock-indexed C4 twist census")
    print("=" * 79)
    cert = Certificate()

    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    protocol_norm = normalized(protocol_text)
    cert.check("G1 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL, sha256(PROTOCOL))
    cert.check(
        "G1 locked-before-execution marker",
        "[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]" in protocol_text,
        "locked",
    )
    cert.check(
        "G1 narrow-route scope",
        "narrower than a whole-engine no-go" in protocol_norm,
        "named production route only",
    )

    texts: dict[str, str] = {}
    for relative, expected in FROZEN.items():
        path = ROOT / relative
        actual = sha256(path)
        cert.check(f"G1 source hash {relative}", actual == expected, actual)
        texts[relative] = path.read_text(encoding="utf-8")

    voxel = texts["engine/include/ftd/voxel.h"]
    toggles = texts["engine/include/ftd/term_toggles.h"]
    transmutation = texts["engine/src/transmutation_phases.cpp"]
    render = texts["engine/src/render_bridge.cpp"]
    phase_read = texts["engine/src/render_bridge_phases/phase_read.cpp"]
    phase_write = texts["engine/src/render_bridge_phases/phase_write.cpp"]
    gpu_buffers_h = texts["engine/include/ftd/gpu_buffers.h"]
    gpu_buffers_cu = texts["engine/cuda/gpu_buffers.cu"]
    gpu_engine = texts["engine/cuda/gpu_engine.cu"]
    kernels_aux = texts["engine/cuda/kernels_aux.cu"]
    kernels_dual = texts["engine/cuda/kernels_stencil_dual.cu"]
    diagnostics = texts["engine/include/ftd/render_bridge_diagnostics.h"]
    energy = texts["engine/src/energy_ledger_compute.cpp"]

    weak_cpu = section(
        transmutation,
        "void weak_transmutation_cpu(RenderBridge& rb)",
        "void accumulate_proper_time(RenderBridge& rb)",
    )
    clock_cpu = section(
        transmutation,
        "void accumulate_proper_time(RenderBridge& rb)",
        "void pair_production_cpu(RenderBridge& rb)",
    )
    weak_gpu = section(
        kernels_aux,
        "__global__ void weak_transmutation_kernel(",
        "__global__ void pair_production_kernel(",
    )
    weak_launcher = section(
        kernels_aux,
        "void launch_weak_transmutation(",
        "}  // namespace kernels",
    )

    # G2: clock state and ordering.
    cert.check(
        "G2 phase is diagnostic only",
        "implementation\n  // contract, not a substrate theorem of physical covariance. Read-only\n  // diagnostic; NOT mixed into the golden state hash." in voxel,
        "Voxel::phase",
    )
    cert.check(
        "G2 phase update law",
        "v.phase += omega0 * delta_tau;" in clock_cpu,
        "phase += omega0*delta_tau",
    )
    cert.check(
        "G2 weak phase precedes clock accumulation",
        render.index("// Rule 6: Weak transmutation")
        < render.index("weak_transmutation_cpu();", render.index("// Rule 6: Weak transmutation"))
        < render.index("// Rule 8: Proper time accumulation")
        < render.index("accumulate_proper_time();", render.index("// Rule 8: Proper time accumulation")),
        "Rule 6 then Rule 8",
    )
    cert.check(
        "G2 clock toggle is imposed",
        "bool de_broglie_clock = false;  // [IMPOSED]" in toggles,
        "not emergent",
    )

    # G3: CPU predicate and exchange.
    cert.check(
        "G3 CPU predicate inputs",
        all(token in weak_cpu for token in ("compute_stress_left", "WEAK_THRESHOLD", "voxel_uniform", "rb.tick_")),
        "state/stress/RNG/tick",
    )
    cert.check(
        "G3 CPU predicate is clock blind",
        all(token not in weak_cpu for token in ("phase", "tau", "omega0", "delta_tau")),
        "no phase/proper-time read",
    )
    cert.check(
        "G3 CPU exchanges both canonical-looking channel pairs",
        "std::swap(v.flux_L, v.flux_R);" in weak_cpu
        and "std::swap(v.wave_vel_L, v.wave_vel_R);" in weak_cpu,
        "flux and wave velocity",
    )
    cert.check(
        "G3 CPU polarity flip",
        "rb.set_state(i, static_cast<int8_t>(-v.state));" in weak_cpu,
        "s -> -s",
    )

    # G4: CUDA dependency and parity census.
    cert.check(
        "G4 device tau exists",
        "double*   d_tau" in gpu_buffers_h and "cudaMalloc(&d_tau" in gpu_buffers_cu,
        "d_tau",
    )
    cert.check(
        "G4 device phase absent",
        "d_phase" not in gpu_buffers_h and "d_phase" not in gpu_buffers_cu,
        "no device phase buffer",
    )
    cert.check(
        "G4 phase omitted from upload dirty-state comparison",
        "bits_differ(a.tau, b.tau)" in gpu_buffers_cu
        and not re.search(r"bits_differ\(a\.phase\s*,\s*b\.phase\)", gpu_buffers_cu),
        "tau uploaded; phase host-only",
    )
    cert.check(
        "G4 CUDA predicate inputs",
        all(token in weak_gpu for token in ("stress", "weak_threshold", "voxel_uniform", "rng_seed", "tick")),
        "stress/RNG/tick",
    )
    cert.check(
        "G4 CUDA predicate is clock blind",
        all(token not in weak_gpu for token in ("phase", "tau", "omega0"))
        and all(token not in weak_launcher for token in ("phase", "tau", "omega0")),
        "kernel and launcher",
    )
    cert.check(
        "G4 CUDA swaps L/R flux and wave velocity",
        all(
            token in weak_gpu
            for token in (
                "fL_x_mut[i] = fR_x_mut[i]",
                "fL_y_mut[i] = fR_y_mut[i]",
                "fL_z_mut[i] = fR_z_mut[i]",
                "wvL_x_mut[i] = wvR_x_mut[i]",
                "wvL_y_mut[i] = wvR_y_mut[i]",
                "wvL_z_mut[i] = wvR_z_mut[i]",
            )
        ),
        "six component swaps",
    )
    cert.check(
        "G4 CUDA engine supplies only seed and tick controls",
        "launch_weak_transmutation(bufs_, toggles.dual_substrate, rng_seed, tick);" in gpu_engine,
        "no phase argument",
    )

    # G5: exact swap algebra.
    zero = sp.Integer(0)
    one = sp.Integer(1)
    ident4 = sp.eye(4)
    swap = sp.Matrix(
        [[zero, one, zero, zero], [one, zero, zero, zero],
         [zero, zero, zero, one], [zero, zero, one, zero]]
    )
    omega = sp.Matrix.vstack(
        sp.Matrix.hstack(sp.zeros(2), sp.eye(2)),
        sp.Matrix.hstack(-sp.eye(2), sp.zeros(2)),
    )
    cert.check("G5 swap is symplectic", swap.T * omega * swap == omega, "S^T Omega S=Omega")
    cert.check("G5 swap preserves quadratic norm", swap.T * swap == ident4, "S^T S=I")
    cert.check("G5 swap has order two", swap * swap == ident4 and swap != ident4, "S^2=I")

    root2 = sp.sqrt(2)
    r2 = sp.Matrix([[one, one], [one, -one]]) / root2
    change = sp.diag(one, one, one, one)
    change[:2, :2] = r2
    change[2:, 2:] = r2
    common_relative = sp.simplify(change * swap * change.T)
    expected_cr = sp.diag(one, -one, one, -one)
    cert.check("G5 common/relative split", common_relative == expected_cr, common_relative)
    relative = common_relative.extract([1, 3], [1, 3])
    j = sp.Matrix([[zero, -one], [one, zero]])
    cert.check("G5 relative action is half-turn", relative == -sp.eye(2), relative)
    cert.check("G5 oriented quarter generators", j * j == -sp.eye(2) and (-j) * (-j) == -sp.eye(2), "J^2=(-J)^2=-I")
    cert.check(
        "G5 swap loses orientation bit",
        relative != j and relative != -j and relative * relative == sp.eye(2),
        "-I is neither +J nor -J",
    )

    # G6: the present oscillator is symmetric under the swap.
    w = sp.Symbol("omega_sq", commutative=True)
    clock_operator = w * sp.eye(4)
    cert.check("G6 symmetric clock commutes with swap", clock_operator * swap == swap * clock_operator, "[C,S]=0")
    cert.check(
        "G6 CPU clock force is L/R symmetric",
        "delta_j_L_[i] -= rb.voxels_[i].flux_L * omega0_sq;" in phase_read
        and "delta_j_R_[i] -= rb.voxels_[i].flux_R * omega0_sq;" in phase_read,
        "same omega0_sq",
    )
    cert.check(
        "G6 CUDA clock force is L/R symmetric",
        "dLx -= fL_x[i] * omega0_sq" in kernels_dual
        and "dRx -= fR_x[i] * omega0_sq" in kernels_dual,
        "same omega0_sq",
    )
    cert.check(
        "G6 CPU write treats L/R in parallel",
        "v.wave_vel_L += rb.delta_j_L_[i]" in phase_write
        and "v.wave_vel_R += rb.delta_j_R_[i]" in phase_write
        and "v.flux = v.flux_L + v.flux_R;" in phase_write,
        "independent updates then observable sum",
    )

    # G7: conditional inverse versus full predicate-gated map.
    cert.check("G7 conditional swap inverse", swap.inv() == swap, "S^-1=S")
    x = ("L-high-stress", "R-low-stress")
    sx = (x[1], x[0])
    predicate = {x: True, sx: False}

    def gated(value: tuple[str, str]) -> tuple[str, str]:
        return (value[1], value[0]) if predicate[value] else value

    cert.check(
        "G7 predicate-gated non-injectivity witness",
        x != sx and gated(x) == gated(sx) == sx,
        "P(x)=1, P(Sx)=0",
    )
    cert.check(
        "G7 CPU journal is observation only",
        weak_cpu.count("observation-only native event journal") >= 2,
        "optional diagnostic history",
    )

    # G8: production ledger lacks the required transaction channels.
    ledger_struct = section(diagnostics, "struct EnergyLedger {", "// EM field decomposition")
    forbidden_ledger_terms = (
        "omega0", "phase", "tau", "connection", "switching", "history", "clock_momentum", "controller_work"
    )
    cert.check(
        "G8 ledger has no clock/twist transaction fields",
        all(term not in ledger_struct for term in forbidden_ledger_terms),
        "no clock potential, reaction, work, or retained history",
    )
    cert.check(
        "G8 ledger computation omits clock/switch channels",
        all(term not in energy for term in ("omega0", "delta_tau", "flux_L", "flux_R", "weak_transmutation", "switching", "history")),
        "field+wave+particle+strong only",
    )
    cert.check(
        "G8 current swap is invisible to observable-sum norm",
        sp.simplify((sp.Matrix([[one, one, zero, zero], [zero, zero, one, one]]) * swap)
                    - sp.Matrix([[one, one, zero, zero], [zero, zero, one, one]])) == sp.zeros(2, 4),
        "L+R unchanged",
    )

    # G9/G10: no target leakage or production modification.
    audited_route = weak_cpu + clock_cpu + weak_gpu + weak_launcher + phase_read + kernels_dual
    cert.check(
        "G9 no G* cadence in audited route",
        all(token not in audited_route for token in ("GSTAR", "G_STAR", "lemniscatic", "phase_crossing")),
        "omega0 is independently imposed",
    )
    cert.check(
        "G9 no Born/Bell target in audited route",
        not re.search(r"\bBorn\b|\bBell\b|born_target|bell_target", audited_route),
        "target blind",
    )
    cert.check(
        "G10 production firewall",
        "No production file, engine type, toggle, coupling, tick phase" in protocol_norm,
        "source census only",
    )

    print("-" * 79)
    print(f"checks={cert.total} passed={cert.passed} failed={cert.failed}")
    if cert.failed:
        print("FTD-0978 OUTCOME D - certificate invalid")
        return 1

    print("CLOCK_INDEXED_TRANSITION=ABSENT_IN_NAMED_ROUTE")
    print("DUAL_EXCHANGE=SYMPLECTIC_ORDER_TWO_HALF_TURN")
    print("CLOCK_FORCE=LR_SYMMETRIC_AND_SWAP_COMMUTING")
    print("PRODUCTION_REACTION_WORK_HISTORY=UNBOOKED")
    print("FUTURE_EXPLICIT_TWISTED_GLUING=OPEN")
    print("FTD-0978 OUTCOME B - named production route closed negative; capacity retained")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
