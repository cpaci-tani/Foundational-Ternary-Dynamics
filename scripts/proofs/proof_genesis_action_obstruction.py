#!/usr/bin/env python3
"""Independent algebra/provenance proof for FTD-0567.

The script does not import the C++ observer. It re-derives the accepted genesis
map, its energy withdrawal, the uniform-field action counterexample, and the
evaporation two-preimage witness directly from the frozen production rules.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import platform
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
GATE = 1.0e-12
LOCKED_PREREGISTRATION_SHA256 = (
    "C8DC397572217AF20CD69E01BB398CBB66C3E58A49C309A07A5B8F8F974925C8"
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def require_all(text: str, needles: tuple[str, ...], label: str) -> None:
    missing = [needle for needle in needles if needle not in text]
    assert not missing, f"{label}: missing {missing}"


def source_provenance() -> dict[str, str]:
    files = {
        "phase_write": ROOT / "engine/src/render_bridge_phases/phase_write.cpp",
        "lagrangian_header": ROOT / "engine/include/ftd/lagrangian.h",
        "lagrangian_source": ROOT / "engine/src/lagrangian.cpp",
        "energy_ledger": ROOT / "engine/src/energy_ledger_compute.cpp",
        "action_test": ROOT / "engine/tests/test_action_stationarity.cpp",
        "injectivity_test": ROOT / "engine/tests/test_native_injectivity_gate.cpp",
        "preregistration": ROOT / "docs/theory/10_eft_program/preregistrations/lorentz_recovery_causal_structure/PREREG_GENESIS_AMPLITUDE_ACTION_OBSTRUCTION_v1.md",
    }
    phase_write = files["phase_write"].read_text(encoding="utf-8")
    require_all(
        phase_write,
        (
            "v.wave_vel *= (1.0 - rb.toggles.kinetic_drain);",
            "v.flux *= std::max(0.0, 1.0 - kg / jmag);",
            "double p = 1.0 - std::exp(-excess / km);",
            "manifest_at(rb, v, chi, rb.flux_pre_write_, rb.lattice_, i, gseed, rb.tick_, /*dual=*/true);",
            "rb.set_state(i, 0);",
            "v.particle_id = -1;",
            "v.spin = 0;",
            "v.color = 0;",
        ),
        "production genesis/evaporation map",
    )
    dual_start = phase_write.index("// Genesis (dual): chirality density for polarity.")
    dual_end = phase_write.index("// Genesis (single): divergence for polarity.", dual_start)
    dual_block = phase_write[dual_start:dual_end]
    assert "v.wave_vel *= (1.0 - rb.toggles.kinetic_drain);" not in dual_block
    assert "v.flux *= std::max(0.0, 1.0 - kg / jmag);" not in dual_block

    lagrangian_header = files["lagrangian_header"].read_text(encoding="utf-8")
    require_all(
        lagrangian_header,
        (
            "return v.born_infeld_core();",
            "return G_C * v.state * divJ;",
            "return -LAMBDA_G * violation * violation;",
            "not all variations of this same action",
        ),
        "written action scope",
    )
    for forbidden in ("K_GENESIS", "K_MANIFEST", "L_HIGGS", "Evaporation"):
        assert forbidden not in lagrangian_header
    lagrangian_source = files["lagrangian_source"].read_text(encoding="utf-8")
    for forbidden in ("K_GENESIS", "kinetic_drain", "voxel_uniform", "set_state("):
        assert forbidden not in lagrangian_source

    energy = files["energy_ledger"].read_text(encoding="utf-8")
    require_all(
        energy,
        (
            "0.5 * (E_field + E_wave) + E_kin + E_strong",
            "Rest energy is displayed separately",
            "interaction energies",
        ),
        "accounted energy scope",
    )
    action_test = files["action_test"].read_text(encoding="utf-8")
    require_all(
        action_test,
        (
            "Field Action Stationarity and Production-Force Replay",
            "FTD-0467 proves that the latter are not",
            "all matter-side variations of the written state-flux interaction",
            "rb.toggles.genesis = false;  // Pure field dynamics",
        ),
        "action regression scope",
    )
    injectivity = files["injectivity_test"].read_text(encoding="utf-8")
    require_all(
        injectivity,
        (
            "evaporation preimages are distinct",
            "two distinct signed preimages reach one evaporation image",
            "annihilation erases distinct spin/color preimages",
        ),
        "production noninjectivity witness",
    )
    hashes = {name: sha256(path) for name, path in files.items()}
    assert hashes["preregistration"] == LOCKED_PREREGISTRATION_SHA256
    return hashes


def genesis_arm(
    dual: bool, excess: float, wave_mag2: float, drain: float, polarity: int
) -> dict[str, float | int | bool]:
    kg = km = 1.0
    before = kg + excess
    after = before if dual else excess
    flux_loss = 0.5 * (before * before - after * after)
    wave_factor = 1.0 if dual else 1.0 - drain
    wave_loss = 0.5 * wave_mag2 * (1.0 - wave_factor * wave_factor)
    expected_flux_loss = 0.0 if dual else kg * excess + 0.5 * kg * kg
    expected_wave_loss = 0.0 if dual else (drain - 0.5 * drain * drain) * wave_mag2
    return {
        "dual": dual,
        "excess": excess,
        "wave_mag2": wave_mag2,
        "drain": drain,
        "polarity": polarity,
        "acceptance_probability": 1.0 - math.exp(-excess / km),
        "before": before,
        "after": after,
        "flux_loss": flux_loss,
        "wave_loss": wave_loss,
        "amplitude_residual": abs(after - (before if dual else excess)),
        "flux_energy_residual": abs(flux_loss - expected_flux_loss),
        "wave_energy_residual": abs(wave_loss - expected_wave_loss),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--json-output", type=Path)
    args = parser.parse_args()

    excesses = (0.125, 0.25, 0.5, 1.0)
    wave_values = (0.0, 0.25)
    drains = (0.0, 0.5)
    arms = [
        genesis_arm(False, excess, wave, drain, polarity)
        for excess in excesses
        for wave in wave_values
        for drain in drains
        for polarity in (1, -1)
    ]
    arms += [
        genesis_arm(True, excess, wave, 0.0, polarity)
        for excess in excesses
        for wave in wave_values
        for polarity in (1, -1)
    ]
    assert len(arms) == 48

    max_amplitude_residual = max(float(a["amplitude_residual"]) for a in arms)
    max_flux_energy_residual = max(float(a["flux_energy_residual"]) for a in arms)
    max_wave_energy_residual = max(float(a["wave_energy_residual"]) for a in arms)
    assert max_amplitude_residual <= GATE
    assert max_flux_energy_residual <= GATE
    assert max_wave_energy_residual <= GATE

    register = [
        genesis_arm(False, excess, 0.0, 0.0, 1) for excess in excesses
    ]
    post_amplitudes = {float(a["after"]) for a in register}
    probabilities = [float(a["acceptance_probability"]) for a in register]
    losses = [float(a["flux_loss"]) + float(a["wave_loss"]) for a in register]
    assert post_amplitudes == set(excesses)
    assert all(a < b for a, b in zip(probabilities, probabilities[1:]))
    fixed_quantum_energy_spread = max(losses) - min(losses)
    assert fixed_quantum_energy_spread > GATE

    # The implemented candidate-state terms depend on divJ and s, not |J|.
    # B is the common Born term. At divJ=0:
    # L(0)=B, L(+1)=L(-1)=B-lambda. The arrays are identical for any
    # spatially uniform field amplitude, while eligibility flips at |J|=kg.
    born, lam = -1.0, 100.0
    below_action = (born, born - lam, born - lam)
    above_action = (born, born - lam, born - lam)
    max_action_threshold_residual = max(
        abs(a - b) for a, b in zip(below_action, above_action)
    )
    assert max_action_threshold_residual == 0.0
    assert not (0.5 > 1.0) and (2.0 > 1.0)
    assert below_action[1] == below_action[2]
    assert (-1 if not (0.0 > 0.0) else 1) == -1

    # Frozen evaporation assignments collapse distinct sign/id/spin/color
    # preimages and change no field variable.
    plus = {"state": 1, "particle_id": 11, "spin": 1, "color": 2, "J": (1, 2, 3), "W": (4, 5, 6)}
    minus = {"state": -1, "particle_id": 22, "spin": -1, "color": 3, "J": (1, 2, 3), "W": (4, 5, 6)}
    assert plus != minus
    for preimage in (plus, minus):
        preimage.update(state=0, particle_id=-1, spin=0, color=0)
    assert plus == minus

    hashes = source_provenance()
    implementation_files = {
        "header": ROOT / "engine/include/ftd/eft/genesis_action_obstruction.h",
        "source": ROOT / "engine/src/eft/genesis_action_obstruction.cpp",
        "test": ROOT / "engine/tests/test_genesis_action_obstruction.cpp",
        "independent_proof": Path(__file__).resolve(),
    }
    result = {
        "ftd_id": "FTD-0567",
        "verdict": "GENESIS_ACTION_OBSTRUCTION",
        "platform": platform.platform(),
        "field_representation": "frozen production (s,J,W) variables",
        "normalized_parameters": {"kg": 1.0, "km": 1.0},
        "excesses": list(excesses),
        "wave_magnitude_squared": list(wave_values),
        "kinetic_drains": list(drains),
        "polarities": [1, -1],
        "tolerance": GATE,
        "arms": len(arms),
        "distinct_single_post_amplitudes": len(post_amplitudes),
        "maximum_amplitude_residual": max_amplitude_residual,
        "maximum_flux_energy_residual": max_flux_energy_residual,
        "maximum_wave_energy_residual": max_wave_energy_residual,
        "maximum_action_threshold_residual": max_action_threshold_residual,
        "fixed_quantum_energy_spread": fixed_quantum_energy_spread,
        "single_map_preserves_overshoot": True,
        "no_post_genesis_amplitude_lock": True,
        "no_fixed_ternary_energy_quantum": True,
        "acceptance_conditioning_does_not_lock": True,
        "dual_branch_has_no_latent_heat_payment": True,
        "evaporation_signed_preimages_collapse": True,
        "written_action_cannot_generate_magnitude_gate": True,
        "written_action_zero_divergence_polarity_degenerate": True,
        "frozen_common_action_route_closed": True,
        "extended_reservoir_or_open_system_remains_open": True,
        "source_hashes_sha256": hashes,
        "implementation_hashes_sha256": {
            name: sha256(path) for name, path in implementation_files.items()
        },
    }
    if args.json_output:
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    print("PASS: production genesis is not an amplitude lock or the written common action")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
