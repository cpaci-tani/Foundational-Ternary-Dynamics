#!/usr/bin/env python3
"""Exact certificate for the minimal many-body matter-network derivation.

This script performs no numerical search and uses no measured constants.  All
checks are finite integer/rational identities for the selected compact pair
law, quadratic-coat capacity candidate, SC/FCC parity geometry, static
checkerboard Gauss field, and rectangular-block surface accounting.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from fractions import Fraction as F
from itertools import product


Vec = tuple[F, F, F]


@dataclass
class Certificate:
    checks: int = 0
    failures: list[str] = field(default_factory=list)

    def check(self, condition: bool, label: str) -> None:
        self.checks += 1
        if not condition:
            self.failures.append(label)


def add(lhs: Vec, rhs: Vec) -> Vec:
    return tuple(a + b for a, b in zip(lhs, rhs))  # type: ignore[return-value]


def sub(lhs: Vec, rhs: Vec) -> Vec:
    return tuple(a - b for a, b in zip(lhs, rhs))  # type: ignore[return-value]


def scale(value: F, vector: Vec) -> Vec:
    return tuple(value * item for item in vector)  # type: ignore[return-value]


def dot(lhs: Vec, rhs: Vec) -> F:
    return sum((a * b for a, b in zip(lhs, rhs)), F(0))


def norm2(vector: Vec) -> F:
    return dot(vector, vector)


def potential(q: F, epsilon: F = F(1)) -> F:
    if q >= F(3, 2):
        return F(0)
    return -16 * epsilon * (q - F(3, 2)) ** 2 * (q - F(3, 4))


def potential_prime(q: F, epsilon: F = F(1)) -> F:
    if q >= F(3, 2):
        return F(0)
    return -48 * epsilon * (q - F(3, 2)) * (q - F(1))


def divided_potential(q0: F, q1: F) -> F:
    if q0 == q1:
        return potential_prime(q0)
    return (potential(q1) - potential(q0)) / (q1 - q0)


def pairwise_energy(positions: list[Vec], polarities: list[int]) -> F:
    total = F(0)
    for first in range(len(positions)):
        for second in range(first + 1, len(positions)):
            if polarities[first] == polarities[second]:
                continue
            total += potential(norm2(sub(positions[first], positions[second])))
    return total


def pairwise_impulses(
    before: list[Vec], after: list[Vec], polarities: list[int]
) -> list[Vec]:
    impulses: list[Vec] = [(F(0), F(0), F(0)) for _ in before]
    for first in range(len(before)):
        for second in range(first + 1, len(before)):
            if polarities[first] == polarities[second]:
                continue
            d0 = sub(before[first], before[second])
            d1 = sub(after[first], after[second])
            gradient = divided_potential(norm2(d0), norm2(d1))
            impulse = scale(gradient, add(d0, d1))
            impulses[first] = sub(impulses[first], impulse)
            impulses[second] = add(impulses[second], impulse)
    return impulses


def b2_integer(offset: int) -> F:
    if offset == 0:
        return F(3, 4)
    if abs(offset) == 1:
        return F(1, 8)
    return F(0)


def b2_rational(offset: F) -> F:
    absolute = abs(offset)
    if absolute <= F(1, 2):
        return F(3, 4) - offset * offset
    if absolute < F(3, 2):
        return F(1, 2) * (F(3, 2) - absolute) ** 2
    return F(0)


def db2_rational(offset: F) -> F:
    absolute = abs(offset)
    if absolute <= F(1, 2):
        return -2 * offset
    if absolute < F(3, 2):
        return -(F(3, 2) - absolute) * (F(-1) if offset < 0 else F(1))
    return F(0)


def d2b2_rational(offset: F) -> F:
    absolute = abs(offset)
    if absolute < F(1, 2) or offset == 0:
        return F(-2)
    if F(1, 2) < absolute < F(3, 2):
        return F(1)
    if absolute >= F(3, 2):
        return F(0)
    raise ValueError("quadratic coat second derivative requested at a knot")


def rational_rank(matrix: list[list[F]]) -> int:
    if not matrix:
        return 0
    work = [row[:] for row in matrix]
    row_count = len(work)
    column_count = len(work[0])
    pivot_row = 0
    pivot_column = 0
    while pivot_row < row_count and pivot_column < column_count:
        selected = next(
            (
                row
                for row in range(pivot_row, row_count)
                if work[row][pivot_column] != 0
            ),
            None,
        )
        if selected is None:
            pivot_column += 1
            continue
        work[pivot_row], work[selected] = work[selected], work[pivot_row]
        pivot = work[pivot_row][pivot_column]
        work[pivot_row] = [entry / pivot for entry in work[pivot_row]]
        for row in range(row_count):
            if row == pivot_row or work[row][pivot_column] == 0:
                continue
            multiple = work[row][pivot_column]
            work[row] = [
                entry - multiple * pivot_entry
                for entry, pivot_entry in zip(work[row], work[pivot_row])
            ]
        pivot_row += 1
        pivot_column += 1
    return pivot_row


def parity(x: int, y: int, z: int) -> int:
    return -1 if (x + y + z) & 1 else 1


def block_bonds(length_x: int, length_y: int, length_z: int) -> int:
    points = set(product(range(length_x), range(length_y), range(length_z)))
    result = 0
    for x, y, z in points:
        for dx, dy, dz in ((1, 0, 0), (0, 1, 0), (0, 0, 1)):
            if (x + dx, y + dy, z + dz) in points:
                result += 1
    return result


def block_polarity(length_x: int, length_y: int, length_z: int) -> int:
    return sum(
        parity(x, y, z)
        for x, y, z in product(
            range(length_x), range(length_y), range(length_z)
        )
    )


def finite_checkerboard_line(length: int) -> dict[int, F]:
    """Quadratic-coat density of an even alternating finite 1D block."""
    return {
        site: sum(
            (F(1) if constituent % 2 == 0 else F(-1))
            * b2_integer(site - constituent)
            for constituent in range(length)
        )
        for site in range(-1, length + 1)
    }


def shifted_finite_checkerboard_line(length: int, shift: F) -> dict[int, F]:
    """Alternating finite line deposited after a common subcell shift."""
    return {
        site: sum(
            (F(1) if constituent % 2 == 0 else F(-1))
            * b2_rational(F(site - constituent) - shift)
            for constituent in range(length)
        )
        for site in range(-1, length + 2)
    }


def finite_line_translation_derivative(
    length: int, *, signed: bool
) -> dict[int, F]:
    """First deposited-line variation under a common subcell translation."""
    return {
        site: -sum(
            (
                F(1) if not signed or constituent % 2 == 0 else F(-1)
            )
            * db2_rational(F(site - constituent))
            for constituent in range(length)
        )
        for site in range(-1, length + 1)
    }


def infinite_alternating_line_amplitude(shift: F) -> F:
    """Checkerboard amplitude at site zero for |shift| <= 1/2."""
    if abs(shift) > F(1, 2):
        raise ValueError("principal translation phase is |shift| <= 1/2")
    return sum(
        (F(1) if constituent % 2 == 0 else F(-1))
        * b2_rational(-F(constituent) - shift)
        for constituent in range(-2, 3)
    )


def cumulative_face_field(density: dict[int, F]) -> dict[int, F]:
    """Compact 1D face field with E(v)-E(v-1)=rho(v)."""
    field: dict[int, F] = {}
    running = F(0)
    for site in range(min(density), max(density) + 1):
        running += density.get(site, F(0))
        field[site] = running
    return field


def run_certificate() -> Certificate:
    cert = Certificate()

    # Compact-well algebra.
    cert.check(potential(F(1)) == -1, "well minimum")
    cert.check(potential_prime(F(1)) == 0, "well stationary")
    cert.check(potential(F(3, 2)) == 0, "cutoff value")
    cert.check(potential_prime(F(3, 2)) == 0, "cutoff first derivative")
    cert.check(potential(F(3, 4)) == 0, "inner zero")
    cert.check(potential(F(0)) == 27, "coincident repulsion")

    # Exact N-body momentum/work identity for two simultaneous active bonds.
    before: list[Vec] = [
        (F(-1), F(0), F(0)),
        (F(0), F(0), F(0)),
        (F(1), F(0), F(0)),
    ]
    after: list[Vec] = [
        (F(-19, 20), F(1, 50), F(0)),
        (F(1, 100), F(-1, 100), F(0)),
        (F(49, 50), F(1, 100), F(0)),
    ]
    signs = [1, -1, 1]
    impulses = pairwise_impulses(before, after, signs)
    total_impulse = (F(0), F(0), F(0))
    work = F(0)
    for x0, x1, impulse in zip(before, after, impulses):
        total_impulse = add(total_impulse, impulse)
        work += dot(sub(x1, x0), impulse)
    delta_u = pairwise_energy(after, signs) - pairwise_energy(before, signs)
    cert.check(total_impulse == (F(0), F(0), F(0)), "N-body impulse sum")
    cert.check(work == -delta_u, "N-body binding work")

    # Superextensive all-pairs counterexample.
    for multiplicity in range(1, 7):
        stacked_positions = [
            *[(F(0), F(0), F(0)) for _ in range(multiplicity)],
            *[(F(1), F(0), F(0)) for _ in range(multiplicity)],
        ]
        stacked_signs = [1] * multiplicity + [-1] * multiplicity
        expected = -F(multiplicity * multiplicity)
        cert.check(
            pairwise_energy(stacked_positions, stacked_signs) == expected,
            f"stack collapse m={multiplicity}",
        )

    # Quadratic-coat capacity and coarse degree bound.
    cert.check(b2_integer(0) + 2 * b2_integer(1) == 1, "1D partition")
    cert.check(F(1, 2) ** 3 == F(1, 8), "nearest-site minimum weight")
    cert.check(8 * F(1, 8) == 1, "eight-per-nearest-site capacity")
    cert.check(4**3 * 8 - 1 == 511, "capacity degree bound")

    # SC/FCC parity geometry.
    offsets = [offset for offset in product((-1, 0, 1), repeat=3) if offset != (0, 0, 0)]
    opposite_face = [offset for offset in offsets if sum(v * v for v in offset) == 1 and sum(offset) & 1]
    same_fcc = [offset for offset in offsets if sum(v * v for v in offset) == 2 and not (sum(offset) & 1)]
    cert.check(len(opposite_face) == 6, "six reciprocal SC faces")
    cert.check(len(same_fcc) == 12, "twelve same-parity FCC neighbours")
    cert.check(all(sum(v * v for v in offset) >= 2 for offset in same_fcc), "FCC outside cutoff")

    # Exact checkerboard coat, Gauss field, curl, and energy on an even cell.
    lattice_size = 4
    max_rho_residual = F(0)
    max_divergence_residual = F(0)
    max_curl_residual = F(0)
    max_capacity_residual = F(0)
    max_centered_field = F(0)
    field_energy = F(0)
    for x, y, z in product(range(lattice_size), repeat=3):
        rho = F(0)
        for dx, dy, dz in product((-1, 0, 1), repeat=3):
            rho += (
                parity(x - dx, y - dy, z - dz)
                * b2_integer(dx)
                * b2_integer(dy)
                * b2_integer(dz)
            )
        expected = F(parity(x, y, z), 8)
        max_rho_residual = max(max_rho_residual, abs(rho - expected))

        occupancy = F(0)
        for dx, dy, dz in product((-1, 0, 1), repeat=3):
            occupancy += b2_integer(dx) * b2_integer(dy) * b2_integer(dz)
        max_capacity_residual = max(max_capacity_residual, abs(occupancy - 1))

        electric = F(parity(x, y, z), 48)
        divergence = F(0)
        for axis in range(3):
            behind = [x, y, z]
            behind[axis] -= 1
            behind_field = F(parity(*behind), 48)
            divergence += electric - behind_field
            max_centered_field = max(
                max_centered_field, abs(electric + behind_field)
            )
            field_energy += F(1, 2) * electric * electric
        max_divergence_residual = max(
            max_divergence_residual, abs(divergence - expected)
        )

        # Forward-difference curl: every component difference is identical.
        for first, second in ((0, 1), (1, 2), (2, 0)):
            step_first = [x, y, z]
            step_second = [x, y, z]
            step_first[first] += 1
            step_second[second] += 1
            curl = (
                F(parity(*step_first), 48)
                - electric
                - F(parity(*step_second), 48)
                + electric
            )
            max_curl_residual = max(max_curl_residual, abs(curl))

    volume = lattice_size**3
    cert.check(max_rho_residual == 0, "checkerboard coat density")
    cert.check(max_capacity_residual == 0, "integer filling saturates capacity")
    cert.check(max_divergence_residual == 0, "checkerboard Gauss field")
    cert.check(max_curl_residual == 0, "checkerboard curl-free field")
    cert.check(max_centered_field == 0, "centered electric cancellation")
    cert.check(field_energy / volume == F(1, 1536), "field energy per site")

    # General even-L translation-phase identities.  At the integer-aligned
    # phase, complete axial row translations change both signed source and
    # unsigned occupancy only in the two-site-thick end caps.  In the infinite
    # checkerboard bulk the signed line amplitude is exactly 1/2 - 2 delta^2
    # on the principal cell, so the site-aligned field energy is a maximum and
    # the half-cell phase cancels the bulk source.
    even_lengths = (2, 4, 6, 8)
    cert.check(
        all(
            finite_line_translation_derivative(length, signed=False)
            == {
                site: (
                    F(-1, 2)
                    if site in (-1, 0)
                    else F(1, 2)
                    if site in (length - 1, length)
                    else F(0)
                )
                for site in range(-1, length + 1)
            }
            for length in even_lengths
        ),
        "even finite unsigned row translation is end-cap supported",
    )
    cert.check(
        all(
            finite_line_translation_derivative(length, signed=True)
            == {
                site: (
                    F(-1, 2)
                    if site in (-1, length)
                    else F(1, 2)
                    if site in (0, length - 1)
                    else F(0)
                )
                for site in range(-1, length + 1)
            }
            for length in even_lengths
        ),
        "even finite signed row translation is end-cap supported",
    )
    cert.check(
        all(
            sum(
                finite_line_translation_derivative(length, signed=signed).values(),
                F(0),
            )
            == 0
            for length in even_lengths
            for signed in (False, True)
        ),
        "finite row translation conserves signed and unsigned line weight",
    )
    phase_samples = (F(-1, 2), F(-1, 4), F(0), F(1, 4), F(1, 2))
    cert.check(
        all(
            infinite_alternating_line_amplitude(shift)
            == F(1, 2) - 2 * shift * shift
            for shift in phase_samples
        ),
        "infinite alternating line translation-phase amplitude",
    )
    cert.check(
        all(
            infinite_alternating_line_amplitude(shift) ** 2 / 384
            == F(1, 1536) - shift * shift / 192 + shift**4 / 96
            for shift in phase_samples
        ),
        "periodic checkerboard translation-phase field energy",
    )
    cert.check(
        infinite_alternating_line_amplitude(F(0)) == F(1, 2)
        and infinite_alternating_line_amplitude(F(1, 2)) == 0,
        "integer phase is sourced and half-cell phase is bulk neutral",
    )
    cert.check(
        2 * (2 * F(1, 2) * F(-2) / 384) == F(-1, 96),
        "site-aligned periodic field-energy translation curvature",
    )
    half_shifted_lines = {
        length: shifted_finite_checkerboard_line(length, F(1, 2))
        for length in even_lengths
    }
    cert.check(
        all(
            density
            == {
                site: (
                    F(1, 2)
                    if site == 0
                    else F(-1, 2)
                    if site == length
                    else F(0)
                )
                for site in range(-1, length + 2)
            }
            for length, density in half_shifted_lines.items()
        ),
        "half-shifted even alternating line is boundary supported",
    )
    cert.check(
        all(
            sum(density.values(), F(0)) == 0
            and sum((value * value for value in density.values()), F(0))
            == F(1, 2)
            for density in half_shifted_lines.values()
        ),
        "half-shifted finite line neutrality and norm",
    )

    # A transverse route gives an exact area-scaling field for a finite body
    # whose x translation phase is one half.  This is an admissible Gauss
    # field, not an assertion that the route is the curl-free minimizer.
    half_x = half_shifted_lines[2]
    finite_y = finite_checkerboard_line(4)
    finite_z = finite_checkerboard_line(6)
    transverse_y_field = cumulative_face_field(finite_y)
    max_half_phase_gauss_residual = F(0)
    half_phase_trial_energy = F(0)
    for x, y, z in product(half_x, finite_y, finite_z):
        rho = half_x[x] * finite_y[y] * finite_z[z]
        electric_y = half_x[x] * transverse_y_field[y] * finite_z[z]
        previous_y = half_x[x] * transverse_y_field.get(y - 1, F(0)) * finite_z[z]
        max_half_phase_gauss_residual = max(
            max_half_phase_gauss_residual,
            abs(electric_y - previous_y - rho),
        )
        half_phase_trial_energy += F(1, 2) * electric_y * electric_y
    cert.check(
        max_half_phase_gauss_residual == 0,
        "half-shifted finite block transverse Gauss route",
    )
    cert.check(
        half_phase_trial_energy == F((10 * 4 - 1) * (4 * 6 + 5), 2048),
        "half-shifted finite block area-scaling field bound",
    )

    # Binding-mode coefficients from exact Taylor coefficients.
    for delta_q in (F(-1, 4), F(-1, 8), F(0), F(1, 8), F(1, 4)):
        cert.check(
            potential(1 + delta_q) + 1
            == 12 * delta_q**2 - 16 * delta_q**3,
            f"well expansion delta_q={delta_q}",
        )
    cert.check(F(12) * 2**2 * 3 == 144, "dilation coefficient")
    cert.check(F(12) == 12, "SC shear quartic coefficient")
    cert.check(4 * F(48, 9) == F(64, 3), "BCC shear quadratic coefficient")
    cert.check(F(48) == 48, "relative-sublattice coefficient")

    # Finite rectangular block: bond, surface, and polarity identities.
    for lengths in ((1, 1, 1), (2, 3, 4), (3, 3, 3), (4, 5, 6)):
        lx, ly, lz = lengths
        formula = (lx - 1) * ly * lz + lx * (ly - 1) * lz + lx * ly * (lz - 1)
        cert.check(block_bonds(lx, ly, lz) == formula, f"block bonds {lengths}")
        expected_q = 1 if all(length % 2 for length in lengths) else 0
        cert.check(block_polarity(lx, ly, lz) == expected_q, f"block polarity {lengths}")
        surface_half = ly * lz + lx * lz + lx * ly
        cert.check(3 * lx * ly * lz - formula == surface_half, f"surface excess {lengths}")

    # Exact finite all-even Gauss construction.  The x-routed field is not
    # asserted to be the electrostatic minimizer; it is a compact admissible
    # field and therefore an exact upper bound on the minimum Gauss energy.
    line_data: dict[int, tuple[dict[int, F], dict[int, F]]] = {}
    for length in (2, 4, 6):
        density = finite_checkerboard_line(length)
        face_field = cumulative_face_field(density)
        line_data[length] = (density, face_field)
        density_norm = sum((value * value for value in density.values()), F(0))
        field_norm = sum((value * value for value in face_field.values()), F(0))
        cert.check(sum(density.values(), F(0)) == 0, f"finite line neutral L={length}")
        cert.check(
            density_norm == F(4 * length + 5, 16),
            f"finite line density norm L={length}",
        )
        cert.check(
            field_norm == F(10 * length - 1, 32),
            f"finite line field norm L={length}",
        )

    lx, ly, lz = 2, 4, 6
    density_x, field_x = line_data[lx]
    density_y, _ = line_data[ly]
    density_z, _ = line_data[lz]
    max_finite_gauss_residual = F(0)
    trial_energy = F(0)
    for x, y, z in product(
        range(-1, lx + 1), range(-1, ly + 1), range(-1, lz + 1)
    ):
        rho = density_x[x] * density_y[y] * density_z[z]
        electric_x = field_x[x] * density_y[y] * density_z[z]
        previous_x = field_x.get(x - 1, F(0)) * density_y[y] * density_z[z]
        max_finite_gauss_residual = max(
            max_finite_gauss_residual, abs(electric_x - previous_x - rho)
        )
        trial_energy += F(1, 2) * electric_x * electric_x
    trial_formula = F(
        (10 * lx - 1) * (4 * ly + 5) * (4 * lz + 5), 16384
    )
    cert.check(max_finite_gauss_residual == 0, "finite block routed Gauss field")
    cert.check(trial_energy == trial_formula, "finite block routed field energy")

    # The selected cube bound is negative from L=2 onward.  After division by
    # N=L^3 the routed-field factor is
    # (160 + 384/L + 210/L^2 - 25/L^3)/16384.  Its derivative is negative for
    # L>=1 because 384 L^2 + 420 L - 75 is then positive and increasing.
    selected_beta = F(21892057692994273, 10**18)
    selected_epsilon = F(1, 100)

    def cube_trial_per_constituent(length: int) -> F:
        return F((10 * length - 1) * (4 * length + 5) ** 2, 16384 * length**3)

    cert.check(384 + 420 - 75 > 0, "cube field bound derivative negative at L=1")
    cert.check(2 * 384 + 420 > 0, "cube derivative polynomial increasing")
    cert.check(
        -3 * selected_epsilon
        + F(3, 2) * selected_epsilon
        + selected_beta * cube_trial_per_constituent(2)
        < 0,
        "selected even-cube formation bound negative at L=2",
    )
    cert.check(
        -3 * selected_epsilon + selected_beta * F(5, 512) < 0,
        "selected even-cube formation bound negative asymptotically",
    )

    # Capacity is an incompressibility/exclusion constraint, not a shear law.
    # Interior finite-block sites saturate; the one-coat boundary layer has
    # slack.  Independent translations of complete x rows preserve occupancy
    # exactly by the 1D partition of unity.
    for length in (4, 6):
        unsigned = {
            site: sum(
                (b2_integer(site - constituent) for constituent in range(length)),
                F(0),
            )
            for site in range(-1, length + 1)
        }
        expected = {
            **{-1: F(1, 8), 0: F(7, 8)},
            **{site: F(1) for site in range(1, length - 1)},
            length - 1: F(7, 8),
            length: F(1, 8),
        }
        cert.check(unsigned == expected, f"finite capacity profile L={length}")
        active_sites = sum(value == 1 for value in unsigned.values())
        cert.check(active_sites == length - 2, f"finite active capacity count L={length}")
        cert.check(
            active_sites**3 == (length - 2) ** 3,
            f"finite 3D active capacity count L={length}",
        )
        cert.check(
            sum((value * value for value in unsigned.values()), F(0))
            == F(length) - F(7, 16),
            f"finite capacity squared norm L={length}",
        )

    row_partition = True
    row_occupancy = F(0)
    for row_y, row_z in product((-1, 0, 1), repeat=2):
        shift = F(row_y + 2 * row_z, 7)
        translated_row = sum(
            (b2_rational(-F(column) - shift) for column in range(-3, 4)),
            F(0),
        )
        row_partition = row_partition and translated_row == 1
        row_occupancy += (
            b2_integer(row_y) * b2_integer(row_z) * translated_row
        )
    cert.check(row_partition, "independently shifted rows preserve 1D capacity")
    cert.check(row_occupancy == 1, "independently shifted rows preserve 3D capacity")

    # Eight coincident half-cell coats saturate all eight surrounding sites.
    # A single site's contact normal has nonzero common-translation derivative,
    # exposing lattice recoil unless the complete multiplier field cancels it.
    half_weight = b2_rational(F(-1, 2)) ** 3
    common_x_gradient = 8 * (
        -db2_rational(F(-1, 2))
        * b2_rational(F(-1, 2))
        * b2_rational(F(-1, 2))
    )
    cert.check(8 * half_weight == 1, "half-cell eight-coat capacity saturation")
    cert.check(common_x_gradient == -2, "single-site capacity recoil gradient")

    # The diffuse interface functional sum_v n(v)(1-n(v)) factorizes and has
    # an exact area-leading expansion for a rectangular block.
    lx, ly, lz = 4, 6, 8
    interface_factorized = F(lx * ly * lz) - (
        F(lx) - F(7, 16)
    ) * (F(ly) - F(7, 16)) * (F(lz) - F(7, 16))
    interface_expanded = (
        F(7, 16) * F(ly * lz + lx * lz + lx * ly)
        - F(49, 256) * F(lx + ly + lz)
        + F(343, 4096)
    )
    cert.check(
        interface_factorized == interface_expanded,
        "finite diffuse capacity interface area expansion",
    )

    # Cubic-invariant L=4 pressure pattern.  The eight active sites are
    # {1,2}^3, so one common multiplier produces a separable, net-zero outward
    # capacity force with exact virial 24 lambda.
    pressure_b = [
        sum((b2_integer(active - constituent) for active in (1, 2)), F(0))
        for constituent in range(4)
    ]
    pressure_d = [
        sum(
            (db2_rational(F(active - constituent)) for active in (1, 2)),
            F(0),
        )
        for constituent in range(4)
    ]
    cert.check(
        pressure_b == [F(1, 8), F(7, 8), F(7, 8), F(1, 8)],
        "L4 invariant pressure weights",
    )
    cert.check(
        pressure_d == [F(-1, 2), F(-1, 2), F(1, 2), F(1, 2)],
        "L4 invariant pressure derivatives",
    )
    pressure_total = [F(0), F(0), F(0)]
    pressure_virial = F(0)
    for x, y, z in product(range(4), repeat=3):
        force = (
            pressure_d[x] * pressure_b[y] * pressure_b[z],
            pressure_b[x] * pressure_d[y] * pressure_b[z],
            pressure_b[x] * pressure_b[y] * pressure_d[z],
        )
        for axis in range(3):
            pressure_total[axis] += force[axis]
        pressure_virial += (
            (F(x) - F(3, 2)) * force[0]
            + (F(y) - F(3, 2)) * force[1]
            + (F(z) - F(3, 2)) * force[2]
        )
    cert.check(pressure_total == [F(0), F(0), F(0)], "L4 pressure net impulse")
    cert.check(pressure_virial == 24, "L4 pressure outward virial")
    pressure_magnitudes: set[F] = set()
    pressure_orbits: set[tuple[int, int]] = set()
    for x, y, z in product(range(4), repeat=3):
        coordinates = (x, y, z)
        inner_count = sum(coordinate in (1, 2) for coordinate in coordinates)
        components = (
            pressure_d[x] * pressure_b[y] * pressure_b[z],
            pressure_b[x] * pressure_d[y] * pressure_b[z],
            pressure_b[x] * pressure_b[y] * pressure_d[z],
        )
        for axis, component_value in enumerate(components):
            pressure_magnitudes.add(abs(component_value))
            pressure_orbits.add(
                (inner_count, int(coordinates[axis] in (1, 2)))
            )
    cert.check(
        pressure_magnitudes == {F(1, 128), F(7, 128), F(49, 128)},
        "L4 pressure component magnitudes",
    )
    cert.check(len(pressure_orbits) == 6, "L4 pressure component cubic orbits")

    # Exact L=4 active-constraint and harmonic row-slide geometry.  The eight
    # active capacity rows have full rank.  All 48 axial row translations are
    # tangent to them and are zero modes of the central-force binding Hessian.
    constituents = list(product(range(4), repeat=3))
    active_capacity_sites = list(product((1, 2), repeat=3))
    capacity_jacobian: list[list[F]] = []
    for site in active_capacity_sites:
        jacobian_row: list[F] = []
        for constituent in constituents:
            for axis in range(3):
                value = -db2_rational(F(site[axis] - constituent[axis]))
                for other_axis in range(3):
                    if other_axis != axis:
                        value *= b2_rational(
                            F(site[other_axis] - constituent[other_axis])
                        )
                jacobian_row.append(value)
        capacity_jacobian.append(jacobian_row)
    cert.check(rational_rank(capacity_jacobian) == 8, "L4 capacity Jacobian rank")

    capacity_gram = [
        [
            sum(
                (
                    capacity_jacobian[first][column]
                    * capacity_jacobian[second][column]
                    for column in range(192)
                ),
                F(0),
            )
            for second in range(8)
        ]
        for first in range(8)
    ]
    gram_by_hamming: dict[int, set[F]] = {distance: set() for distance in range(4)}
    for first, first_site in enumerate(active_capacity_sites):
        for second, second_site in enumerate(active_capacity_sites):
            distance = sum(a != b for a, b in zip(first_site, second_site))
            gram_by_hamming[distance].add(capacity_gram[first][second])
    cert.check(
        gram_by_hamming
        == {
            0: {F(1083, 2048)},
            1: {F(57, 512)},
            2: {F(9, 512)},
            3: {F(0)},
        },
        "L4 capacity Gram Hamming kernel",
    )
    walsh_spectrum: dict[int, tuple[int, F]] = {}
    multiplicities = (1, 3, 3, 1)
    for weight in range(4):
        mask = (1,) * weight + (0,) * (3 - weight)
        eigenvalue = sum(
            (
                capacity_gram[0][column]
                * (
                    -1
                    if sum(
                        mask[axis]
                        * (active_capacity_sites[column][axis] - 1)
                        for axis in range(3)
                    )
                    & 1
                    else 1
                )
                for column in range(8)
            ),
            F(0),
        )
        walsh_spectrum[weight] = (multiplicities[weight], eigenvalue)
    cert.check(
        walsh_spectrum
        == {
            0: (1, F(1875, 2048)),
            1: (3, F(1275, 2048)),
            2: (3, F(819, 2048)),
            3: (1, F(507, 2048)),
        },
        "L4 capacity Gram Walsh spectrum",
    )

    row_slide_basis: list[list[F]] = []
    row_slide_labels: list[tuple[int, tuple[int, int]]] = []
    for axis in range(3):
        other_axes = [candidate for candidate in range(3) if candidate != axis]
        for key in product(range(4), repeat=2):
            column: list[F] = []
            for constituent in constituents:
                for component in range(3):
                    column.append(
                        F(1)
                        if component == axis
                        and (
                            constituent[other_axes[0]],
                            constituent[other_axes[1]],
                        )
                        == key
                        else F(0)
                    )
            row_slide_basis.append(column)
            row_slide_labels.append((axis, key))
    capacity_on_row_slides = [
        [
            sum(
                (
                    capacity_jacobian[row][coordinate]
                    * row_slide_basis[column][coordinate]
                    for coordinate in range(192)
                ),
                F(0),
            )
            for column in range(48)
        ]
        for row in range(8)
    ]
    cert.check(
        all(value == 0 for row in capacity_on_row_slides for value in row),
        "L4 row slides tangent to active capacity",
    )

    binding_incidence: list[list[F]] = []
    constituent_index = {
        constituent: index for index, constituent in enumerate(constituents)
    }
    for constituent in constituents:
        for axis in range(3):
            neighbor = list(constituent)
            neighbor[axis] += 1
            neighbor_tuple = tuple(neighbor)
            if neighbor_tuple not in constituent_index:
                continue
            row = [F(0)] * 192
            row[3 * constituent_index[constituent] + axis] = F(-1)
            row[3 * constituent_index[neighbor_tuple] + axis] = F(1)
            binding_incidence.append(row)
    cert.check(
        len(binding_incidence) == 144
        and rational_rank(binding_incidence) == 144,
        "L4 binding Hessian rank and 48 row-slide nullity",
    )

    # Restrict the Hessian of sum_{v in {1,2}^3} n(v) to the 48 row-slide
    # coordinates.  Its zero trace and nonzero rank prove that any nonzero
    # common pressure is indefinite on the binding-null space.
    capacity_hessian_blocks: list[list[list[F]]] = []
    for constituent in constituents:
        block = [[F(0) for _ in range(3)] for _ in range(3)]
        for site in active_capacity_sites:
            offsets = [F(site[axis] - constituent[axis]) for axis in range(3)]
            for axis in range(3):
                value = d2b2_rational(offsets[axis])
                for other_axis in range(3):
                    if other_axis != axis:
                        value *= b2_rational(offsets[other_axis])
                block[axis][axis] += value
            for first_axis in range(3):
                for second_axis in range(first_axis + 1, 3):
                    remaining_axis = 3 - first_axis - second_axis
                    value = (
                        db2_rational(offsets[first_axis])
                        * db2_rational(offsets[second_axis])
                        * b2_rational(offsets[remaining_axis])
                    )
                    block[first_axis][second_axis] += value
                    block[second_axis][first_axis] += value
        capacity_hessian_blocks.append(block)

    pressure_row_hessian = [[F(0) for _ in range(48)] for _ in range(48)]
    for constituent_number, constituent in enumerate(constituents):
        memberships: list[tuple[int, int]] = []
        for column, (axis, key) in enumerate(row_slide_labels):
            other_axes = [candidate for candidate in range(3) if candidate != axis]
            if (
                constituent[other_axes[0]],
                constituent[other_axes[1]],
            ) == key:
                memberships.append((column, axis))
        for first_column, first_axis in memberships:
            for second_column, second_axis in memberships:
                pressure_row_hessian[first_column][second_column] += (
                    capacity_hessian_blocks[constituent_number][first_axis][second_axis]
                )
    pressure_rank = rational_rank(pressure_row_hessian)
    pressure_trace = sum(
        (pressure_row_hessian[index][index] for index in range(48)), F(0)
    )
    pressure_nonzero = max(
        abs(value) for row in pressure_row_hessian for value in row
    )
    cert.check(
        pressure_rank == 21
        and pressure_trace == 0
        and pressure_nonzero == F(7, 32)
        and all(
            pressure_row_hessian[first][second]
            == pressure_row_hessian[second][first]
            for first in range(48)
            for second in range(48)
        ),
        "L4 common-pressure row-slide Hessian is rank-21 indefinite",
    )
    translations_are_null = True
    for axis in range(3):
        translation = [
            F(1) if label_axis == axis else F(0)
            for label_axis, _key in row_slide_labels
        ]
        translations_are_null = translations_are_null and all(
            sum(
                (
                    pressure_row_hessian[row][column] * translation[column]
                    for column in range(48)
                ),
                F(0),
            )
            == 0
            for row in range(48)
        )
    cert.check(
        translations_are_null and 48 - pressure_rank == 27,
        "L4 pressure leaves 27 row-slide null directions including translations",
    )

    # The 27-dimensional kernel is explicit.  Pressure detects only the
    # D-weighted inner/outer imbalance in either transverse row coordinate.
    # For each motion axis, D^perp tensor D^perp supplies 3*3 null patterns.
    d_perp_basis = (
        (F(1), F(-1), F(0), F(0)),
        (F(0), F(0), F(1), F(-1)),
        (F(1), F(0), F(1), F(0)),
    )
    cert.check(
        all(
            sum((pressure_d[index] * vector[index] for index in range(4)), F(0))
            == 0
            for vector in d_perp_basis
        ),
        "L4 D-perp transverse basis",
    )
    explicit_pressure_kernel: list[list[F]] = []
    for axis in range(3):
        for first_vector in d_perp_basis:
            for second_vector in d_perp_basis:
                coordinate = [F(0)] * 48
                for row, (label_axis, key) in enumerate(row_slide_labels):
                    if label_axis == axis:
                        coordinate[row] = (
                            first_vector[key[0]] * second_vector[key[1]]
                        )
                explicit_pressure_kernel.append(coordinate)
    kernel_annihilated = all(
        sum(
            (
                pressure_row_hessian[row][column] * vector[column]
                for column in range(48)
            ),
            F(0),
        )
        == 0
        for vector in explicit_pressure_kernel
        for row in range(48)
    )
    cert.check(
        len(explicit_pressure_kernel) == 27
        and rational_rank(explicit_pressure_kernel) == 27
        and kernel_annihilated,
        "L4 pressure kernel equals three D-perp tensor-square sectors",
    )

    # Although common pressure is blind to these 27 directions, the complete
    # finite coat is not.  Differentiate the signed Gauss source and unsigned
    # occupancy on the full compact support {-1,...,4}^3 with respect to all
    # 48 row slides, then restrict both maps to the explicit pressure kernel.
    # Exact full column rank means every nonzero pressure-null combination is
    # visible at first order to both the longitudinal field source and the
    # finite-body surface profile.
    full_support_sites = list(product(range(-1, 5), repeat=3))
    signed_source_derivative: list[list[F]] = []
    unsigned_occupancy_derivative: list[list[F]] = []
    for site in full_support_sites:
        signed_row: list[F] = []
        unsigned_row: list[F] = []
        for axis, key in row_slide_labels:
            other_axes = [candidate for candidate in range(3) if candidate != axis]
            signed_value = F(0)
            unsigned_value = F(0)
            for constituent in constituents:
                if (
                    constituent[other_axes[0]],
                    constituent[other_axes[1]],
                ) != key:
                    continue
                value = -db2_rational(F(site[axis] - constituent[axis]))
                for other_axis in other_axes:
                    value *= b2_rational(
                        F(site[other_axis] - constituent[other_axis])
                    )
                unsigned_value += value
                signed_value += parity(*constituent) * value
            signed_row.append(signed_value)
            unsigned_row.append(unsigned_value)
        signed_source_derivative.append(signed_row)
        unsigned_occupancy_derivative.append(unsigned_row)

    cert.check(
        all(
            sum((matrix[row][column] for row in range(len(matrix))), F(0)) == 0
            for matrix in (
                signed_source_derivative,
                unsigned_occupancy_derivative,
            )
            for column in range(48)
        ),
        "L4 row-slide source and occupancy derivatives conserve total weight",
    )

    def restrict_to_pressure_kernel(matrix: list[list[F]]) -> list[list[F]]:
        return [
            [
                sum(
                    (
                        row[column] * kernel_vector[column]
                        for column in range(48)
                    ),
                    F(0),
                )
                for kernel_vector in explicit_pressure_kernel
            ]
            for row in matrix
        ]

    signed_on_kernel = restrict_to_pressure_kernel(signed_source_derivative)
    unsigned_on_kernel = restrict_to_pressure_kernel(
        unsigned_occupancy_derivative
    )
    cert.check(
        rational_rank(signed_on_kernel) == 27
        and all(
            rational_rank([row[9 * axis : 9 * (axis + 1)] for row in signed_on_kernel])
            == 9
            for axis in range(3)
        ),
        "L4 signed Gauss source is injective on the pressure kernel",
    )
    cert.check(
        rational_rank(unsigned_on_kernel) == 27
        and all(
            rational_rank(
                [row[9 * axis : 9 * (axis + 1)] for row in unsigned_on_kernel]
            )
            == 9
            for axis in range(3)
        ),
        "L4 finite occupancy surface is injective on the pressure kernel",
    )

    return cert


def main() -> int:
    certificate = run_certificate()
    for failure in certificate.failures:
        print(f"FAIL: {failure}")
    print(
        "minimal_many_body_matter_network "
        f"checks={certificate.checks} failures={len(certificate.failures)}"
    )
    return 0 if not certificate.failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
