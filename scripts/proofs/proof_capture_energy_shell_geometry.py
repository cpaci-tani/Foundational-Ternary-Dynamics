"""Independent analytic/data certificate for FTD-0733."""

from __future__ import annotations

import csv
import hashlib
from decimal import Decimal, getcontext
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations" / (
    "PREREG_CAPTURE_ENERGY_SHELL_GEOMETRY_v1.md"
)
ROWS = ROOT / "engine/results/ftd_0732" / (
    "ftd_0732_captured_state_perturbation_survival_v1.csv"
)

PROTOCOL = "E4C639DC897DEF3B0395F8CC8335B004ED8D9D3E40EAB29F61651D2EAC6E26DB"
ROWS_HASH = "15926F9E64B8DE3A633CCE4794B07DAF40E6293D29D97DE63C89493980C2E2AD"
VERDICT = "SELECTED_CAPTURE_ENERGY_SHELL_DERIVED"

getcontext().prec = 80
D = Decimal(1) / Decimal(100)
THREE_QUARTERS = Decimal(3) / Decimal(4)
ONE = Decimal(1)
THREE_HALVES = Decimal(3) / Decimal(2)
ROOT_WIDTH_GATE = Decimal("1e-30")
CLASSIFICATION_GATE = Decimal("1e-12")
RECONSTRUCTION_GATE = Decimal("1e-13")


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, label: str, checks: list[str]) -> None:
    assert condition, label
    checks.append(label)


def potential(distance_squared: Decimal) -> Decimal:
    if distance_squared >= THREE_HALVES:
        return Decimal(0)
    return (
        -Decimal(16)
        * D
        * (distance_squared - THREE_HALVES) ** 2
        * (distance_squared - THREE_QUARTERS)
    )


def energy(distance_squared: Decimal, kinetic: Decimal) -> Decimal:
    return kinetic + potential(distance_squared)


def isolate_root(
    kinetic: Decimal, left: Decimal, right: Decimal
) -> tuple[Decimal, Decimal]:
    """Isolate one root on a preregistered monotone interval."""
    f_left = energy(left, kinetic)
    f_right = energy(right, kinetic)
    assert f_left * f_right < 0
    for _ in range(240):
        midpoint = (left + right) / 2
        f_midpoint = energy(midpoint, kinetic)
        if f_left * f_midpoint <= 0:
            right = midpoint
        else:
            left = midpoint
            f_left = f_midpoint
    return left, right


def first_value(series: str) -> Decimal:
    return Decimal(series.split(";", maxsplit=1)[0])


def main() -> None:
    checks: list[str] = []
    check(digest(PREREG) == PROTOCOL, "locked protocol hash", checks)
    check(digest(ROWS) == ROWS_HASH, "FTD-0732 CSV hash", checks)

    # Exact polynomial/algebra certificate.  The expanded normalized potential
    # is -16 d^3 + 60 d^2 - 72 d + 27.
    d_symbolic = Fraction(1)
    normalized_at_one = (
        -16 * d_symbolic**3
        + 60 * d_symbolic**2
        - 72 * d_symbolic
        + 27
    )
    check(normalized_at_one == -1, "exact well minimum", checks)
    check(potential(THREE_QUARTERS) == 0, "exact inner zero", checks)
    check(potential(ONE) == -D, "exact depth D", checks)
    check(potential(THREE_HALVES) == 0, "exact cutoff zero", checks)
    # dE/dd = -48 D (d-3/2)(d-1): negative on (3/4,1), positive
    # on (1,3/2).  The sign products below are exact rational witnesses.
    inner_probe = Fraction(7, 8)
    outer_probe = Fraction(5, 4)
    inner_sign = -(
        inner_probe - Fraction(3, 2)
    ) * (inner_probe - Fraction(1))
    outer_sign = -(
        outer_probe - Fraction(3, 2)
    ) * (outer_probe - Fraction(1))
    check(inner_sign < 0, "exact inner monotone-decrease sign", checks)
    check(outer_sign > 0, "exact outer monotone-increase sign", checks)
    check(
        -Fraction(1, 1) / inner_sign > 0,
        "inner root moves outward as kinetic rises",
        checks,
    )
    check(
        -Fraction(1, 1) / outer_sign < 0,
        "outer root moves inward as kinetic rises",
        checks,
    )

    with ROWS.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    check(len(rows) == 84, "84 frozen FTD-0732 rows", checks)
    check(
        {row["volume"] for row in rows} == {"33", "65"},
        "two registered volumes",
        checks,
    )
    check(
        {row["direction"] for row in rows}
        == {"0_0_1", "0_1_-1", "1_1_1"},
        "three registered directions",
        checks,
    )
    check(
        {row["polarity"] for row in rows} == {"plus_minus", "minus_plus"},
        "two polarity orders",
        checks,
    )

    groups: dict[tuple[str, str, str], list[dict[str, str]]] = {}
    for row in rows:
        groups.setdefault(
            (row["volume"], row["direction"], row["polarity"]), []
        ).append(row)
    check(len(groups) == 12, "12 volume-direction-polarity groups", checks)

    expected_stage_a = {
        "center",
        "separation_minus",
        "separation_plus",
        "radial_impulse_minus",
        "radial_impulse_plus",
        "tangent1_impulse_minus",
        "tangent1_impulse_plus",
        "tangent2_impulse_minus",
        "tangent2_impulse_plus",
        "dynamic_field_minus",
        "dynamic_field_plus",
    }
    expected_stage_b = {"center", "radial_impulse_plus", "dynamic_field_minus"}
    reports: dict[tuple[str, str, str], dict[str, Decimal | str]] = {}

    for key, group in sorted(groups.items()):
        volume, direction, polarity = key
        expected = expected_stage_a if volume == "33" else expected_stage_b
        check(
            {row["variant"] for row in group} == expected,
            f"{key} exact variant set",
            checks,
        )
        center = next(row for row in group if row["variant"] == "center")
        parent_r = first_value(center["separation_history"])
        parent_d = parent_r * parent_r

        kinetic_by_variant: dict[str, Decimal] = {}
        for row in group:
            radius = first_value(row["separation_history"])
            pair_energy = Decimal(row["initial_pair_energy"])
            kinetic = pair_energy - potential(radius * radius)
            kinetic_by_variant[row["variant"]] = kinetic
            check(kinetic > 0, f"{key}/{row['variant']} positive kinetic", checks)
            check(kinetic < D, f"{key}/{row['variant']} kinetic below depth", checks)

            inner_left, inner_right = isolate_root(
                kinetic, THREE_QUARTERS, ONE
            )
            outer_left, outer_right = isolate_root(
                kinetic, ONE, THREE_HALVES
            )
            check(
                inner_right - inner_left < ROOT_WIDTH_GATE,
                f"{key}/{row['variant']} inner root isolated",
                checks,
            )
            check(
                outer_right - outer_left < ROOT_WIDTH_GATE,
                f"{key}/{row['variant']} outer root isolated",
                checks,
            )
            inside_shell = inner_right < radius * radius < outer_left
            initialized = row["initialized"] == "1"
            check(
                inside_shell == initialized,
                f"{key}/{row['variant']} shell equals initialization class",
                checks,
            )
            if initialized:
                check(
                    pair_energy < -CLASSIFICATION_GATE,
                    f"{key}/{row['variant']} certified negative",
                    checks,
                )
            else:
                check(
                    pair_energy > CLASSIFICATION_GATE,
                    f"{key}/{row['variant']} certified positive",
                    checks,
                )

        center_kinetic = kinetic_by_variant["center"]
        for variant in expected & {
            "separation_minus",
            "separation_plus",
            "dynamic_field_minus",
            "dynamic_field_plus",
        }:
            check(
                abs(kinetic_by_variant[variant] - center_kinetic)
                < RECONSTRUCTION_GATE,
                f"{key}/{variant} preserves kinetic level",
                checks,
            )

        maximum_variant = max(kinetic_by_variant, key=kinetic_by_variant.get)
        maximum_kinetic = kinetic_by_variant[maximum_variant]
        check(
            maximum_variant == "radial_impulse_plus",
            f"{key} registered maximum kinetic selector",
            checks,
        )
        inner = isolate_root(maximum_kinetic, THREE_QUARTERS, ONE)
        outer = isolate_root(maximum_kinetic, ONE, THREE_HALVES)
        inner_root = (inner[0] + inner[1]) / 2
        outer_root = (outer[0] + outer[1]) / 2
        parent_energy_at_maximum = energy(parent_d, maximum_kinetic)
        check(
            parent_energy_at_maximum < -CLASSIFICATION_GATE,
            f"{key} parent lies in common shell",
            checks,
        )

        old_inner_d = (Decimal("0.95") * parent_r) ** 2
        old_outer_d = (Decimal("1.05") * parent_r) ** 2
        check(
            energy(old_inner_d, maximum_kinetic) > CLASSIFICATION_GATE,
            f"{key} old inward probe outside common shell",
            checks,
        )
        check(
            energy(old_outer_d, maximum_kinetic) < -CLASSIFICATION_GATE,
            f"{key} old outward probe inside common shell",
            checks,
        )

        inner_r = inner_root.sqrt()
        outer_r = outer_root.sqrt()
        reports[key] = {
            "maximum_variant": maximum_variant,
            "maximum_kinetic": maximum_kinetic,
            "parent_r": parent_r,
            "inner_d": inner_root,
            "outer_d": outer_root,
            "inner_r": inner_r,
            "outer_r": outer_r,
            "inner_scale": inner_r / parent_r,
            "outer_scale": outer_r / parent_r,
            "u": (parent_d - inner_root) / (outer_root - inner_root),
        }

    # Polarity is a mirrored label, not a different energy shell.
    for volume in ("33", "65"):
        for direction in ("0_0_1", "0_1_-1", "1_1_1"):
            plus = reports[(volume, direction, "plus_minus")]
            minus = reports[(volume, direction, "minus_plus")]
            for field in (
                "maximum_kinetic",
                "parent_r",
                "inner_d",
                "outer_d",
                "inner_scale",
                "outer_scale",
                "u",
            ):
                check(
                    abs(plus[field] - minus[field]) < Decimal("1e-40"),
                    f"{volume}/{direction} polarity-mirror {field}",
                    checks,
                )

    # At K=D, d=1 is the unique zero at the well minimum; no strict
    # negative-energy interval remains.  This is the exact collapse endpoint.
    check(energy(ONE, D) == 0, "exact shell-collapse endpoint", checks)
    check(energy(THREE_QUARTERS, D) > 0, "collapse inner flank positive", checks)
    check(energy(THREE_HALVES, D) > 0, "collapse outer flank positive", checks)

    print(f"FTD-0733 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")
    print(
        "volume direction K_max r_parent r_inner r_outer "
        "inner_scale outer_scale u"
    )
    for volume in ("33", "65"):
        for direction in ("0_0_1", "0_1_-1", "1_1_1"):
            report = reports[(volume, direction, "plus_minus")]
            print(
                volume,
                direction,
                f"{report['maximum_kinetic']:.15E}",
                f"{report['parent_r']:.15E}",
                f"{report['inner_r']:.15E}",
                f"{report['outer_r']:.15E}",
                f"{report['inner_scale']:.15E}",
                f"{report['outer_scale']:.15E}",
                f"{report['u']:.15E}",
            )


if __name__ == "__main__":
    main()
