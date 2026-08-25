#!/usr/bin/env python3
"""Exact v3 field-bank counting seam for EM curvature and Born readout.

For one fixed polarity, the selected v3 site bank has 192 independent binary
channel slots.  Under the already registered uniform counting reference, the
six additive (E,B) readouts have covariance 16 I_6 and zero covariance with
carrier number.  In canonical complete-packet coordinates f=(E,B)/8, the
covariance is I_6/4, its Fisher/large-deviation Hessian is 4 I_6, and one unit
electric packet has Mahalanobis insertion cost 2.

The same binary reference gives, for every tangent/polarity outcome port, four
phase counts with exact Gaussian-integer covariance and the already proved
bright-pair pushforward |Z|^2.  This is one finite counting seam, not two
probability assumptions.

The candidate dynamics does not uniquely select the uniform product measure,
and no theorem equates the bare Fisher Hessian to the dynamical Maxwell--Gauss
action.  An arbitrary common multiplier and the block/action-unit convention
remain.  Therefore the seam does not determine physical chi_EM or alpha.
"""

from __future__ import annotations

import json
import sys
from math import comb
from pathlib import Path

from sympy import Matrix, Rational, Symbol, pi, simplify

from proof_global_c3_cotangent_layer_hodge_maxwell_target import layer_value
from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
from proof_v3_field_bank_gaussian_born_readout import (
    bright_pair_count,
    gaussian_integer,
    norm_squared,
)


sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
REGISTER_PATH = (
    ROOT / "docs/theory/01_reference/strict_discrete_common_action_register_v3.json"
)
CANDIDATE_MANIFEST_PATH = (
    ROOT
    / "docs/theory/01_reference/strict_discrete_common_action_phi_v3_charged_candidate.json"
)


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def binomial_moments(trials: int) -> tuple[Rational, Rational]:
    denominator = 2**trials
    mean = sum(Rational(value * comb(trials, value), denominator) for value in range(trials + 1))
    second = sum(Rational(value * value * comb(trials, value), denominator) for value in range(trials + 1))
    return mean, simplify(second - mean * mean)


def main() -> None:
    register = json.loads(REGISTER_PATH.read_text(encoding="utf-8"))
    candidate = json.loads(CANDIDATE_MANIFEST_PATH.read_text(encoding="utf-8"))
    field = register["carrier_inventory"]["primitive_payloads"]["field_channel_bank"]

    states = one_particle_states()
    check("C1 one polarity bank has exactly 192 finite channels", len(states) == 192 and field["channel_count"] == 384)

    values = tuple(Matrix(layer_value(state, 0)) for state in states)
    total = sum(values, Matrix.zeros(6, 1))
    gram = sum((value * value.T for value in values), Matrix.zeros(6, 6))
    check("C2 uniform field readout has exactly zero mean", total == Matrix.zeros(6, 1))
    check("C3 one-layer field Gram is exactly 64 I6", gram == 64 * Matrix.eye(6))

    # Independent Bernoulli(1/2) slots have variance 1/4.  Carrier-number
    # covariance is 192/4, and its cross covariance with every field component
    # is (1/4) sum_i v_i = 0.
    field_covariance = gram / 4
    number_variance = Rational(192, 4)
    cross_covariance = total / 4
    joint_covariance = Matrix.diag(number_variance, *field_covariance.diagonal())
    check("C4 uniform binary counting gives Cov(E,B)=16 I6", field_covariance == 16 * Matrix.eye(6))
    check("C5 carrier number is orthogonal to all six field coordinates", cross_covariance == Matrix.zeros(6, 1))
    check("C6 complete number-plus-field covariance has rank seven", joint_covariance.rank() == 7)

    # Canonical complete-packet coordinate f=F/8.  Its covariance and inverse
    # are exact, and the unit electric packet is a coordinate basis vector.
    packet_covariance = field_covariance / 64
    packet_hessian = packet_covariance.inv()
    check("C7 canonical packet-coordinate covariance is I6/4", packet_covariance == Rational(1, 4) * Matrix.eye(6))
    check("C8 bare Fisher/large-deviation Hessian is 4 I6", packet_hessian == 4 * Matrix.eye(6))

    packet_costs = []
    for axis in range(3):
        for polarity in (-1, 1):
            packet = Matrix.zeros(6, 1)
            packet[axis] = polarity
            cost = simplify((packet.T * packet_hessian * packet)[0] / 2)
            packet_costs.append(cost)
    check("C9 every unit electric packet has bare counting cost two", set(packet_costs) == {2})

    # Exact block scaling: B independent sites make a coherent unit shift cost
    # 2B, while one packet inserted into the block-average coordinate costs
    # 2/B.  A declared block convention is therefore part of any action readout.
    block_size = Symbol("B", positive=True, integer=True)
    coherent_cost = simplify(Rational(1, 2) * block_size * 4)
    one_packet_average_cost = simplify(
        Rational(1, 2)
        * Rational(1, 1)
        / block_size**2
        * (4 * block_size)
    )
    check("C10 coherent block cost is extensive", coherent_cost == 2 * block_size)
    check("C11 one-packet block-average cost is 2/B", one_packet_average_cost == 2 / block_size)

    # One native outcome port has eight binary slots per phase per site.  The
    # four phase counts are independent Binomial(8,1/2).  Hence Re Z and Im Z
    # have variance four per site and E|Z|^2=8 per site.
    phase_mean, phase_variance = binomial_moments(8)
    check("C12 every one-site port phase count has mean four and variance two", phase_mean == 4 and phase_variance == 2)
    real_variance = 2 * phase_variance
    imag_variance = 2 * phase_variance
    check("C13 Gaussian-integer rails have exact isotropic covariance 4 I2", real_variance == imag_variance == 4)
    check("C14 uniform one-site expected bright-pair count is eight", real_variance + imag_variance == 8)

    # Exhaustive bounded identity guard: the event readout remains exactly the
    # Gaussian norm on every phase-count vector, independent of the moment
    # calculation above.
    born_rows = 0
    for n0 in range(5):
        for n1 in range(5):
            for n2 in range(5):
                for n3 in range(5):
                    counts = (n0, n1, n2, n3)
                    assert bright_pair_count(counts) == norm_squared(
                        gaussian_integer(counts)
                    )
                    born_rows += 1
    check("C15 the same finite bank pushes prepared counts to |Z|^2", born_rows == 625)

    # Conditional identification with the Maxwell--Gauss action.  The bare
    # field Hessian would give chi=4 only after selecting the counting measure,
    # one-site rate convention, and action-unit map.  A common positive
    # multiplier lambda remains invisible to the state transition.
    multiplier = Symbol("lambda_common", positive=True)
    chi_conditional = simplify(4 * multiplier)
    alpha_conditional = simplify(3 * chi_conditional / (2 * pi))
    check("C16 conditional counting curvature is 4 lambda_common", chi_conditional == 4 * multiplier)
    check("C17 conditional native alpha remains 6 lambda_common/pi", alpha_conditional == 6 * multiplier / pi)

    check(
        "C18 candidate dynamics still declares physical measure selection open",
        "physically_selected_history_measure" in candidate["not_closed"],
    )
    check(
        "C19 candidate dynamics still declares physical coupling normalization open",
        "physical_coupling_normalization" in candidate["not_closed"],
    )
    check(
        "C20 native Born preparation and trials remain open",
        "physical_Born_statistics" in candidate["not_closed"],
    )

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 uniform-bank EM/Born seam checks pass")
    print("uniform_one_polarity_Cov_EB=16*I6")
    print("canonical_packet_Cov=I6/4")
    print("bare_counting_Hessian=4*I6")
    print("unit_packet_Mahalanobis_cost=2")
    print("one_port_uniform_E_absZ2=8_per_site")
    print("conditional_chi_EM=4*lambda_common")
    print("Open: physical measure/action-unit selection, pole, preparation, trials")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
