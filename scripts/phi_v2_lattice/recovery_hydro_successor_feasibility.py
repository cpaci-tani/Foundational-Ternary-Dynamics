"""Exact pre-table feasibility certificate for hydrodynamics design section 4.2.

This checks a proposed law, and never loads, changes, or advances the frozen
Phi law. All arithmetic is integral. The bounded exhaustive domain is every
occupancy with mass <= 5, reduced only by the explicitly constructed cube
group. This is finite constraint analysis, not a physical-constant search.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from functools import lru_cache
import hashlib
from itertools import combinations, permutations, product
import json
from pathlib import Path


# Full four-coordinate labels distinguish the two copies of each face velocity.
LABELS = tuple(v for v in product((-1, 0, 1), repeat=4)
               if sum(x * x for x in v) == 2)
LABEL_INDEX = {v: i for i, v in enumerate(LABELS)}
VELOCITIES = tuple(v[:3] for v in LABELS)
GROUP = tuple((p, s) for p in permutations(range(3))
              for s in product((-1, 1), repeat=3))


def transform_vector(g: int, v: tuple[int, int, int]) -> tuple[int, int, int]:
    """Signed axis permutation: (gv)_i = s_i v_{p_i}."""
    p, s = GROUP[g]
    return tuple(s[i] * v[p[i]] for i in range(3))


CHANNEL_ACTION = tuple(tuple(LABEL_INDEX[transform_vector(g, v[:3]) + (v[3],)]
                            for v in LABELS) for g in range(len(GROUP)))
_ACTION_INDEX = {p: i for i, p in enumerate(CHANNEL_ACTION)}
COMPOSE = tuple(tuple(_ACTION_INDEX[tuple(a[b[i]] for i in range(24))]
                      for b in CHANNEL_ACTION) for a in CHANNEL_ACTION)
IDENTITY = _ACTION_INDEX[tuple(range(24))]
INVERSE = tuple(next(j for j in range(48) if COMPOSE[i][j] == IDENTITY)
                for i in range(48))


def transform_state(g: int, state: tuple[int, ...]) -> tuple[int, ...]:
    return tuple(sorted(CHANNEL_ACTION[g][i] for i in state))


def momentum(state: tuple[int, ...]) -> tuple[int, int, int]:
    return tuple(sum(VELOCITIES[i][a] for i in state) for a in range(3))


def little_group(p: tuple[int, int, int]) -> tuple[int, ...]:
    return tuple(g for g in range(48) if transform_vector(g, p) == p)


@lru_cache(maxsize=6)
def occupancy_classes(mass: int) -> dict:
    """Exhaust the declared small sector; intentionally prohibit a large scan."""
    if type(mass) is not int or not 0 <= mass <= 5:
        raise ValueError("certificate mass must be an integer in 0..5")
    classes = defaultdict(list)
    for state in combinations(range(24), mass):
        classes[momentum(state)].append(state)
    return {p: tuple(states) for p, states in classes.items()}


def _conjugate(g: int, stabilizer: frozenset[int]) -> tuple[int, ...]:
    return tuple(sorted(COMPOSE[COMPOSE[g][h]][INVERSE[g]] for h in stabilizer))


def _orbits(states: tuple, p: tuple[int, int, int]) -> tuple:
    group = little_group(p)
    remaining = set(states)
    rows = []
    while remaining:
        representative = min(remaining)
        orbit = tuple(sorted({transform_state(g, representative) for g in group}))
        remaining.difference_update(orbit)
        stabilizer = frozenset(g for g in group
                              if transform_state(g, representative) == representative)
        kind = min(_conjugate(g, stabilizer) for g in group)
        # Equivariant self-maps of G/H are N_G(H)/H. An involutive
        # fixed-point-free self-map exists exactly when nH has order two.
        flips = tuple(g for g in group if g not in stabilizer
                      and COMPOSE[g][g] in stabilizer
                      and frozenset(_conjugate(g, stabilizer)) == stabilizer)
        rows.append((representative, orbit, stabilizer, kind, flips))
    return tuple(rows)


def minimum_fixed_involution(mass: int, p: tuple[int, int, int]) -> tuple[dict, dict]:
    """Exact minimum and a witnessing equivariant involution for one class.

    Isomorphic little-group orbits may be paired. A remaining orbit either
    admits its own free involution, or must be fixed pointwise. This is a
    proposed relaxation witness on this bounded sector, not an adopted law.
    """
    states = occupancy_classes(mass).get(p)
    if states is None:
        raise ValueError("the requested mass-momentum class is empty")
    group = little_group(p)
    rows = _orbits(states, p)
    kinds = defaultdict(list)
    for row in rows:
        kinds[row[3]].append(row)
    collision = {}
    theoretical_minimum = 0
    for family in kinds.values():
        orbit_size = len(family[0][1])
        internally_free = bool(family[0][4])
        if not internally_free:
            theoretical_minimum += (len(family) % 2) * orbit_size
        # Pair equal orbit types; choose a target with precisely the same
        # stabilizer so transport gA -> gB is independent of representative.
        for j in range(0, len(family) - 1, 2):
            source, _, stabilizer, _, _ = family[j]
            target = next(b for b in family[j + 1][1]
                          if frozenset(g for g in group if transform_state(g, b) == b)
                          == stabilizer)
            for g in group:
                a, b = transform_state(g, source), transform_state(g, target)
                collision[a], collision[b] = b, a
        if len(family) % 2:
            source, orbit, _, _, flips = family[-1]
            if flips:
                target = transform_state(flips[0], source)
                for g in group:
                    collision[transform_state(g, source)] = transform_state(g, target)
            else:
                collision.update((state, state) for state in orbit)
    violations = {"domain": int(set(collision) != set(states)),
                  "mass_momentum": 0, "involution": 0, "equivariance": 0}
    for state in states:
        output = collision[state]
        violations["mass_momentum"] += (len(output) != mass or momentum(output) != p)
        violations["involution"] += collision[output] != state
        violations["equivariance"] += sum(
            collision[transform_state(g, state)] != transform_state(g, output)
            for g in group)
    fixed = sum(collision[a] == a for a in states)
    if any(violations.values()) or fixed != theoretical_minimum:
        raise AssertionError("minimum-fixed involution construction failed")
    summary = {
        "mass": mass, "momentum": list(p), "class_size": len(states),
        "little_group_order": len(group),
        "orbit_size_histogram": {str(k): v for k, v in sorted(Counter(
            len(row[1]) for row in rows).items())},
        "required_fixed_points": len(states) % 2,
        "minimum_equivariant_fixed_points": fixed,
        "maximal_mixing_possible": fixed == len(states) % 2,
        "constructive_relaxation_violations": violations,
    }
    return summary, collision


def small_sector_census() -> list[dict]:
    """Every mass <= 5 state, with momentum classes reduced by all 48 symmetries."""
    result = []
    for mass in range(6):
        classes = occupancy_classes(mass)
        seen = set()
        summaries = []
        for p in sorted(classes):
            if p in seen:
                continue
            seen.update(transform_vector(g, p) for g in range(48))
            summaries.append(minimum_fixed_involution(mass, p)[0])
        if seen != set(classes):
            raise AssertionError("momentum-orbit coverage failed")
        result.append({
            "mass": mass, "occupancy_states": sum(map(len, classes.values())),
            "momentum_classes": len(classes), "momentum_orbits": len(summaries),
            "obstructed_momentum_orbits": sum(not row["maximal_mixing_possible"]
                                             for row in summaries),
            "classes": summaries,
        })
    return result


def isotropy_certificate() -> dict:
    second = [[sum(v[a] * v[b] for v in VELOCITIES)
               for b in range(3)] for a in range(3)]
    fourth_violations = 0
    for a, b, c, d in product(range(3), repeat=4):
        actual = sum(v[a] * v[b] * v[c] * v[d] for v in VELOCITIES)
        expected = 4 * ((a == b) * (c == d) + (a == c) * (b == d)
                        + (a == d) * (b == c))
        fourth_violations += actual != expected
    return {"second_moment": second, "fourth_delta_coefficient": 4,
            "fourth_tensor_violations": fourth_violations,
            "T_xxxx": 12, "T_xxyy": 4,
            "scope": "equal-weight velocity tensors only; not hydrodynamic closure"}


def phase_certificate() -> dict:
    """Concrete failures of phase erasure and the sorted passive-color lift."""
    a = tuple(sorted(LABEL_INDEX[v] for v in ((-1, -1, 0, 0), (1, 1, 0, 0))))
    b = tuple(sorted(LABEL_INDEX[v] for v in ((-1, 1, 0, 0), (1, -1, 0, 0))))
    g = GROUP.index(((1, 0, 2), (1, 1, 1)))
    decorated_a = tuple(zip(a, (0, 1)))
    decorated_b = tuple(zip(b, (0, 1)))

    def acted(state):
        return tuple(sorted((CHANNEL_ACTION[g][i], k) for i, k in state))

    erased_channel = LABEL_INDEX[(1, 0, 0, 1)]
    phase_cycle = [[int(row == (column + 1) % 4) for column in range(4)]
                   for row in range(4)]
    phase_fourth = [[sum(phase_cycle[row][a] * phase_cycle[a][b_]
                        * phase_cycle[b_][c] * phase_cycle[c][column]
                        for a, b_, c in product(range(4), repeat=3))
                     for column in range(4)] for row in range(4)]
    return {
        "independent_phase_bits_states_per_polarity": 2 ** 96,
        "one_phase_per_velocity_states_per_polarity": 5 ** 24,
        "occupancy_table_entries": 2 ** 24,
        "erasure_counterexample": {
            "full_state": [[erased_channel, 0], [erased_channel, 1]],
            "erased_occupancy": [erased_channel],
            "true_mass": 2, "erased_mass": 1,
            "true_momentum": [2, 0, 0], "erased_momentum": [1, 0, 0],
        },
        "sorted_assignment_counterexample": {
            "group_index": g, "input": [list(row) for row in decorated_a],
            "output": [list(row) for row in decorated_b],
            "transformed_input": [list(row) for row in acted(decorated_a)],
            "transformed_output": [list(row) for row in acted(decorated_b)],
            "input_fixed": acted(decorated_a) == decorated_a,
            "output_fixed": acted(decorated_b) == decorated_b,
            "occupancy_mass_momentum_preserved": len(a) == len(b) and momentum(a) == momentum(b),
            "occupancy_pair_symmetry_compatible": all(
                (transform_state(h, a) == a) == (transform_state(h, b) == b)
                for h in range(48)),
        },
        "phase_count_cycle": phase_cycle, "phase_count_cycle_fourth_power": phase_fourth,
        "four_hop_additive_invariant_lower_bound": 7,
        "scope": "phase multiset preserved at collision; every particle hops once per hop event; "
                 "closed absorption-free field sector; passive spatial action on k",
    }


def certificate() -> dict:
    census = small_sector_census()
    witness, collision = minimum_fixed_involution(5, (-2, -2, -2))
    states = occupancy_classes(5)[(-2, -2, -2)]
    group = little_group((-2, -2, -2))
    remaining = set(range(24))
    channel_orbits = []
    while remaining:
        i = min(remaining)
        orbit = sorted({CHANNEL_ACTION[g][i] for g in group})
        channel_orbits.append(orbit)
        remaining.difference_update(orbit)
    cycle = GROUP.index(((1, 2, 0), (1, 1, 1)))
    cycle_orbits = sorted({tuple(sorted({i, CHANNEL_ACTION[cycle][i],
                                        CHANNEL_ACTION[COMPOSE[cycle][cycle]][i]}))
                           for i in range(24)})
    witness.update({
        "states": [list(a) for a in states],
        "little_group_channel_orbits": channel_orbits,
        "three_cycle_group_index": cycle,
        "three_cycle_channel_orbits": [list(row) for row in cycle_orbits],
        "three_cycle_order": 3,
        "three_cycle_fixed_projected_velocities": sum(
            transform_vector(cycle, v) == v for v in VELOCITIES),
        "globally_little_group_fixed_states": sum(
            all(transform_state(g, a) == a for g in group) for a in states),
        "minimum_fixed_witness": [list(a) for a in states if collision[a] == a],
        "relaxed_collision_pairs": [[list(a), list(b)]
                                    for a, b in sorted(collision.items()) if a < b],
    })
    return {
        "certificate_id": "strict-hydro-successor-feasibility-v1",
        "design_law_id": "phi-hydro-staged-candidate-1",
        "verdict": "incompatible_as_written_under_declared_label_action",
        "epistemic_status": "THEOREM -- finite obstruction conditional on specified action",
        "action_independent_scope": "unique-fixed-point obstruction also holds for any honest "
                                    "cube-group lift to these same 24 channels: the three-cycle "
                                    "has no fixed projected velocity and all its channel orbits have size 3; "
                                    "the exact minimum of 3 is certified for the stated lift",
        "label_action": "g(v,t)=(g v,t); g is any signed permutation of three axes",
        "phase_action": "g(k)=k (passive)",
        "labels": [list(v) for v in LABELS], "group_order": len(GROUP),
        "isotropy": isotropy_certificate(), "small_sector_census": census,
        "counterexample": witness, "phase_contract": phase_certificate(),
        "full_2_to_24_table_generated": False,
        "old_law_or_wave3_evidence_modified": False,
    }


def validate_certificate(report: dict) -> bool:
    """Recompute every finite claim; reject altered or incomplete evidence."""
    if report != certificate():
        raise ValueError("certificate differs from the exact recomputation")
    return True


def canonical_bytes(report: dict) -> bytes:
    return (json.dumps(report, sort_keys=True, indent=2) + "\n").encode("utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, help="optional certificate JSON path")
    args = parser.parse_args()
    report = certificate()
    encoded = canonical_bytes(report)
    if args.output is not None:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(encoded)
        print(json.dumps({"verdict": report["verdict"], "output": str(args.output),
                          "sha256": hashlib.sha256(encoded).hexdigest()}, sort_keys=True))
    else:
        print(encoded.decode("utf-8"), end="")


if __name__ == "__main__":
    main()
