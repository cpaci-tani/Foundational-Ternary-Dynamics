#!/usr/bin/env python3
"""Independent numpy reproduction of the total-momentum stress-ledger algebra.

`PREREG_TOTAL_MOMENTUM_STRESS_LEDGER_v1.md` Sec 12 item 2 requires this script
to reproduce, in the repository, every fixture closure quoted in its Sec 2.11
table before the protocol hash may be taken.  This file is that verifier.

It is **not** a wrapper around the C++/CUDA engine and it reads no engine
result artifact.  The operators of Sec 0 are re-implemented from scratch on
small synthetic periodic lattices, transcribed from the frozen engine sources
(`matched_gauss_transport.cpp:183-243` for `C`/`C^T`,
`matched_face_momentum_transaction.h:45-73` for `D_i`,
`matched_face_energy_transaction.h:150-172` for the staggered step), and every
identity of Sec 2 is closed a second way, in this project's proof-script
language.  No fit, no search, no tolerance tuning.

Groups reproduce the Sec 2.11 rows one for one:

  A   `C^T` adjoint of `C`; `D_i` skew; `[D_i,C] = [D_i,C^T] = 0`   (Sec 2.1)
  B   global identity (G), source-free and sourced, `L=9`           (Sec 2.1)
  C   M1 regional identity, static mask, `L=9`                      (Sec 2.4)
  D   M2 regional identity + global density/source agreement        (Sec 2.5)
  D'  negative control: L1's flux pair against L2's masked change   (Sec 2.5)
  E   chord census and true reach, `L=11`                           (Sec 2.6)
  F   unit-bond `(T,S)` construction vs direct bilinear, `L=11`     (Sec 2.3)
  G   cumulative moving-mask ledger (L), 24 ticks, `L=15`           (Sec 2.7)
  H   shell corollary (H); source enclosure                         (Sec 2.8)
  J   `eta - tau = 1` and its shell form                            (Sec 2.9)
  U   `interaction_scale` unit convention, byte-checked             (Sec 3)

Group D' is an intentional negative control: an `O(1)` failure there is the
correct outcome, and the script asserts the failure is large rather than small.

Residual convention.  Every closure is reported twice: `abs`, the raw absolute
residual, and `rel`, the residual divided by `max(1, |LHS|, |RHS|, |terms|)` --
the normalization the pre-registration's own G0 gate uses (Sec 6.1) and the
convention of `close()` across `scripts/proofs/`.  Gating is on `rel` against
the Sec 2.11 tolerance column; `abs` is printed so a reader can compare
directly against the Sec 2.11 worst-residual column, whose fixture amplitudes
this script does not attempt to reproduce.
"""

from __future__ import annotations

import hashlib
import re
from collections import Counter
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
PREREG = (
    ROOT / "docs" / "theory" / "10_eft_program" / "preregistrations"
    / "PREREG_TOTAL_MOMENTUM_STRESS_LEDGER_v1.md"
)
CUDA_PIPELINE = ROOT / "engine" / "cuda" / "cuda_matched_field_pipeline.cu"
GAUGE_COUPLINGS = ROOT / "engine" / "include" / "ftd" / "ontic" / "gauge_couplings.h"
FACE_NORMALIZATION = (
    ROOT / "engine" / "include" / "ftd" / "eft" / "face_flux_normalization.h"
)
CAMPAIGN_HARNESS = (
    ROOT / "engine" / "tests" / "campaign_total_momentum_stress_ledger.cpp"
)

# --- Sec 0 frozen constants -------------------------------------------------
# Transcribed from Sec 0 verbatim:
#   lambda = C_SPEED * dt = 0.5773502691896258 * 0.25 = 0.14433756729740646
C_SPEED_PREREG = 0.5773502691896258
DT = 0.25
LAMBDA = C_SPEED_PREREG * DT

# --- Sec 3 frozen unit convention -------------------------------------------
INTERACTION_SCALE = 0.021892057692994273

# --- Sec 2.11 tolerance column ----------------------------------------------
TOLERANCE = {
    "A": 1e-11,
    "B": 1e-11,
    "C": 1e-12,
    "D": 1e-12,
    "E": 1e-13,
    "F": 1e-11,
    "G": 1e-12,
    "H": 1e-12,
    "J": 1e-9,
    "U": 1e-12,
    "L": 0.0,
}

# --- Sec 2.6 census of record (the table this script must reproduce) --------
# columns: displacements, (r,a,b) entries, R+ classes (None = not skew),
#          max |r|_1, max |r|_inf
CENSUS = {
    "D_i": (2, 6, 3, 1, 1),
    "D_i C C^T": (25, 74, 37, 3, 2),
    "D_i C^T C": (25, 74, 37, 3, 2),
    "D_i C^T": (8, 24, None, 2, 2),
    "C C^T": (13, 39, None, 2, 1),
}
SKEW_OPERATORS = ("D_i", "D_i C C^T", "D_i C^T C")
# Sec 2.6: displacements split by |r|_1 as 1,6,10,8 and R+ classes as 2,11,20,4
DISPLACEMENT_SPLIT = {0: 1, 1: 6, 2: 10, 3: 8}
CLASS_SPLIT = {0: 2, 1: 11, 2: 20, 3: 4}


# ===========================================================================
# Ledger
# ===========================================================================
class Ledger:
    """Check counter with per-group worst-residual tracking."""

    def __init__(self) -> None:
        self.checks = 0
        self.failures: list[str] = []
        self.worst: dict[str, tuple[float, float, str]] = {}
        self.notes: list[str] = []

    def check(self, group: str, label: str, condition: bool) -> None:
        self.checks += 1
        if not condition:
            self.failures.append(f"[{group}] {label}")

    def record(self, group: str, label: str, absolute: float, relative: float) -> None:
        prior = self.worst.get(group)
        if prior is None or relative > prior[0]:
            self.worst[group] = (relative, absolute, label)

    def close(
        self,
        group: str,
        label: str,
        lhs: float,
        rhs: float,
        *scale: float,
    ) -> float:
        """Assert lhs == rhs at the Sec 2.11 tolerance for `group`."""
        absolute = abs(lhs - rhs)
        denominator = max(1.0, abs(lhs), abs(rhs), *(abs(s) for s in scale)) \
            if scale else max(1.0, abs(lhs), abs(rhs))
        relative = absolute / denominator
        self.record(group, label, absolute, relative)
        self.check(group, label, relative <= TOLERANCE[group])
        return absolute

    def zero(self, group: str, label: str, value: float, *scale: float) -> float:
        return self.close(group, label, value, 0.0, *scale)

    def note(self, text: str) -> None:
        self.notes.append(text)


LEDGER = Ledger()
NEGATIVE_CONTROL_GAPS: list[float] = []


# ===========================================================================
# Sec 0 operators, transcribed from the frozen engine sources
# ===========================================================================
def shift(field: np.ndarray, r: tuple[int, int, int]) -> np.ndarray:
    """`T_r f(v) = f(v + r)` on the periodic torus.  Shape is (3, L, L, L)."""
    out = field
    for axis, amount in enumerate(r):
        if amount:
            out = np.roll(out, -amount, axis=axis + 1)
    return out


def d_minus(field: np.ndarray, axis: int) -> np.ndarray:
    """`d_a^- = I - T_{-e_a}`."""
    r = [0, 0, 0]
    r[axis] = -1
    return field - shift(field, tuple(r))


def d_plus(field: np.ndarray, axis: int) -> np.ndarray:
    """`d_a^+ = T_{e_a} - I`."""
    r = [0, 0, 0]
    r[axis] = 1
    return shift(field, tuple(r)) - field


def curl(magnetic: np.ndarray) -> np.ndarray:
    """`(C B)_a = eps_{abc} d_b^- B_c`  (`matched_curl`)."""
    out = np.empty_like(magnetic)
    out[0] = d_minus(magnetic, 1)[2] - d_minus(magnetic, 2)[1]
    out[1] = d_minus(magnetic, 2)[0] - d_minus(magnetic, 0)[2]
    out[2] = d_minus(magnetic, 0)[1] - d_minus(magnetic, 1)[0]
    return out


def curl_adjoint(electric: np.ndarray) -> np.ndarray:
    """`(C^T E)_a = eps_{abc} d_b^+ E_c`  (`matched_curl_adjoint`)."""
    out = np.empty_like(electric)
    out[0] = d_plus(electric, 1)[2] - d_plus(electric, 2)[1]
    out[1] = d_plus(electric, 2)[0] - d_plus(electric, 0)[2]
    out[2] = d_plus(electric, 0)[1] - d_plus(electric, 1)[0]
    return out


def central(field: np.ndarray, axis: int) -> np.ndarray:
    """`D_i = (1/2)(T_{e_i} - T_{-e_i})` componentwise."""
    plus = [0, 0, 0]
    minus = [0, 0, 0]
    plus[axis] = 1
    minus[axis] = -1
    return 0.5 * (shift(field, tuple(plus)) - shift(field, tuple(minus)))


def inner(left: np.ndarray, right: np.ndarray) -> float:
    """`<f, g> = sum_{a,v} f_a(v) g_a(v)`."""
    return float(np.sum(left * right))


def momentum(electric: np.ndarray, magnetic: np.ndarray, axis: int) -> float:
    """`P_i(E,B) = <E, D_i C B>`  (`matched_local_translation_momentum`)."""
    return inner(electric, central(curl(magnetic), axis))


def staggered_step(
    electric: np.ndarray, magnetic: np.ndarray, current: np.ndarray, lam: float
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """`B' = B - lambda C^T E`, `E' = E + lambda C B'`, `E'' = E' - K`."""
    magnetic_after = magnetic - lam * curl_adjoint(electric)
    face = curl(magnetic_after)
    electric_after = electric + lam * face - current
    return electric_after, magnetic_after, face


def chebyshev_mask(centre: tuple[int, int, int], radius: int, size: int) -> np.ndarray:
    """Component-independent (site) mask for the periodic Chebyshev cube."""
    index = np.indices((size, size, size))
    offset = np.array(centre).reshape(3, 1, 1, 1)
    delta = np.abs(index - offset)
    distance = np.max(np.minimum(delta, size - delta), axis=0)
    inside = (distance <= radius).astype(float)
    return np.stack([inside, inside, inside])


def whole_domain(size: int) -> np.ndarray:
    return np.ones((3, size, size, size))


# ===========================================================================
# Group A -- Sec 2.1 premises
# ===========================================================================
def group_a() -> None:
    size = 9
    rng = np.random.default_rng(0xA)
    electric = rng.standard_normal((3, size, size, size))
    magnetic = rng.standard_normal((3, size, size, size))

    left = inner(electric, curl(magnetic))
    right = inner(curl_adjoint(electric), magnetic)
    LEDGER.close("A", "C^T is the adjoint of C", left, right)

    for axis in range(3):
        first = rng.standard_normal((3, size, size, size))
        second = rng.standard_normal((3, size, size, size))

        LEDGER.close(
            "A",
            f"D_{axis} skew",
            inner(first, central(second, axis)),
            -inner(central(first, axis), second),
        )

        # elementwise commutators
        commutator_c = np.max(
            np.abs(central(curl(magnetic), axis) - curl(central(magnetic, axis)))
        )
        LEDGER.zero(
            "A",
            f"[D_{axis}, C] = 0",
            float(commutator_c),
            float(np.max(np.abs(curl(magnetic)))),
        )
        commutator_ct = np.max(
            np.abs(
                central(curl_adjoint(electric), axis)
                - curl_adjoint(central(electric, axis))
            )
        )
        LEDGER.zero(
            "A",
            f"[D_{axis}, C^T] = 0",
            float(commutator_ct),
            float(np.max(np.abs(curl_adjoint(electric)))),
        )

        # Sec 2.1 uses skewness of D_i M with M = C C^T; Sec 2.5 uses D_i M'
        LEDGER.close(
            "A",
            f"D_{axis} C C^T skew",
            inner(first, central(curl(curl_adjoint(second)), axis)),
            -inner(central(curl(curl_adjoint(first)), axis), second),
        )
        LEDGER.close(
            "A",
            f"D_{axis} C^T C skew",
            inner(first, central(curl_adjoint(curl(second)), axis)),
            -inner(central(curl_adjoint(curl(first)), axis), second),
        )
        # the two consequences Sec 2.1 actually consumes
        face = curl(magnetic)
        LEDGER.zero("A", f"<u, D_{axis} u> = 0", inner(face, central(face, axis)),
                    float(np.sum(np.abs(face * central(face, axis)))))
        LEDGER.zero(
            "A",
            f"<E, D_{axis} M E> = 0",
            inner(electric, central(curl(curl_adjoint(electric)), axis)),
            float(np.sum(np.abs(electric))),
        )


# ===========================================================================
# Group B -- Sec 2.1 global identity (G)
# ===========================================================================
def group_b() -> None:
    size = 9
    rng = np.random.default_rng(0xB)
    electric = rng.standard_normal((3, size, size, size))
    magnetic = rng.standard_normal((3, size, size, size))

    current = np.zeros((3, size, size, size))
    current[:, 3:6, 3:6, 3:6] = rng.standard_normal((3, 3, 3, 3))

    zero_current = np.zeros((3, size, size, size))

    engine_c_speed = read_engine_double(GAUGE_COUPLINGS, "C_SPEED")
    lambdas = (
        ("prereg lambda", LAMBDA),
        ("engine lambda", engine_c_speed * DT),
        ("arbitrary lambda", 0.37),
    )

    for name, lam in lambdas:
        source_free, magnetic_after, _ = staggered_step(
            electric, magnetic, zero_current, lam
        )
        sourced, _, face = staggered_step(electric, magnetic, current, lam)
        for axis in range(3):
            before = momentum(electric, magnetic, axis)
            if name == "prereg lambda":
                LEDGER.close(
                    "B",
                    f"source-free P_{axis} conserved",
                    momentum(source_free, magnetic_after, axis),
                    before,
                )
            after = momentum(sourced, magnetic_after, axis)
            predicted = -inner(current, central(face, axis))
            LEDGER.close(
                "B",
                f"(G) Delta P_{axis} = -<K, D_i C B'> [{name}]",
                after - before,
                predicted,
            )


# ===========================================================================
# Group C -- Sec 2.4 localization L1 and its regional identity M1
# ===========================================================================
def density_l1(electric: np.ndarray, magnetic: np.ndarray, axis: int) -> np.ndarray:
    """`pi^(1)(a,v) = E_a(v) (D_i C B)_a(v)`, masked at E's site."""
    return electric * central(curl(magnetic), axis)


def density_l2(electric: np.ndarray, magnetic: np.ndarray, axis: int) -> np.ndarray:
    """`pi^(2)(a,v) = -B_a(v) (C^T D_i E)_a(v)`, masked at B's site."""
    return -magnetic * curl_adjoint(central(electric, axis))


def m1_terms(
    electric: np.ndarray,
    magnetic: np.ndarray,
    current: np.ndarray,
    mask: np.ndarray,
    axis: int,
    lam: float,
) -> tuple[float, float, float, float]:
    """Return (masked change, flux Phi^(u), flux Phi^(E), source)."""
    electric_after, magnetic_after, face = staggered_step(
        electric, magnetic, current, lam
    )
    change = float(
        np.sum(mask * density_l1(electric_after, magnetic_after, axis))
        - np.sum(mask * density_l1(electric, magnetic, axis))
    )
    phi_u = inner(mask * face, central(face, axis))
    phi_e = inner(mask * electric, central(curl(curl_adjoint(electric)), axis))
    source = inner(mask * current, central(face, axis))
    return change, phi_u, phi_e, source


def m2_terms(
    electric: np.ndarray,
    magnetic: np.ndarray,
    current: np.ndarray,
    mask: np.ndarray,
    axis: int,
    lam: float,
) -> tuple[float, float, float, float]:
    """Return (masked change, flux Phi^(w), flux Phi^(B'), source)."""
    electric_after, magnetic_after, _ = staggered_step(
        electric, magnetic, current, lam
    )
    change = float(
        np.sum(mask * density_l2(electric_after, magnetic_after, axis))
        - np.sum(mask * density_l2(electric, magnetic, axis))
    )
    edge = curl_adjoint(electric)
    phi_w = inner(mask * edge, central(edge, axis))
    phi_b = inner(
        mask * magnetic_after,
        central(curl_adjoint(curl(magnetic_after)), axis),
    )
    source = inner(mask * magnetic_after, central(curl_adjoint(current), axis))
    return change, phi_w, phi_b, source


def component_mask(size: int, centre: tuple[int, int, int]) -> np.ndarray:
    """A genuinely per-component mask: chi_a differs between components."""
    index = np.indices((size, size, size))
    offset = np.array(centre).reshape(3, 1, 1, 1)
    delta = np.abs(index - offset)
    distance = np.max(np.minimum(delta, size - delta), axis=0)
    return np.stack(
        [
            (distance <= 3).astype(float),
            (distance <= 2).astype(float),
            ((distance >= 1) & (distance <= 4)).astype(float),
        ]
    )


def group_c_d() -> None:
    size = 9
    rng = np.random.default_rng(0xCD)
    electric = rng.standard_normal((3, size, size, size))
    magnetic = rng.standard_normal((3, size, size, size))
    current = np.zeros((3, size, size, size))
    current[:, 3:6, 3:6, 3:6] = rng.standard_normal((3, 3, 3, 3))

    centre = (4, 4, 4)
    masks = (
        ("site mask", chebyshev_mask(centre, 3, size)),
        ("per-component mask", component_mask(size, centre)),
        ("whole domain", whole_domain(size)),
    )

    _, magnetic_after, face = staggered_step(electric, magnetic, current, LAMBDA)

    for name, mask in masks:
        for axis in range(3):
            change, phi_u, phi_e, source = m1_terms(
                electric, magnetic, current, mask, axis, LAMBDA
            )
            LEDGER.close(
                "C",
                f"(M1) i={axis} [{name}]",
                change,
                LAMBDA * phi_u - LAMBDA * phi_e - source,
                LAMBDA * phi_u,
                LAMBDA * phi_e,
                source,
            )

            change2, phi_w, phi_b, source2 = m2_terms(
                electric, magnetic, current, mask, axis, LAMBDA
            )
            LEDGER.close(
                "D",
                f"(M2) i={axis} [{name}]",
                change2,
                LAMBDA * phi_w - LAMBDA * phi_b + source2,
                LAMBDA * phi_w,
                LAMBDA * phi_b,
                source2,
            )

    for axis in range(3):
        # global density agreement: sum pi^(1) == sum pi^(2)
        LEDGER.close(
            "D",
            f"global density agreement i={axis}",
            float(np.sum(density_l1(electric, magnetic, axis))),
            float(np.sum(density_l2(electric, magnetic, axis))),
        )
        # global source agreement: <B', D_i C^T K> + <K, D_i C B'> = 0
        LEDGER.zero(
            "D",
            f"global source agreement i={axis}",
            inner(magnetic_after, central(curl_adjoint(current), axis))
            + inner(current, central(face, axis)),
            inner(current, central(face, axis)),
        )

    # ---- Group D' : negative control -------------------------------------
    mask = chebyshev_mask(centre, 3, size)
    print()
    print("Group D' (negative control, an O(1) failure is the correct result):")
    for axis in range(3):
        change_l1, phi_u, phi_e, source_l1 = m1_terms(
            electric, magnetic, current, mask, axis, LAMBDA
        )
        change_l2, _, _, _ = m2_terms(
            electric, magnetic, current, mask, axis, LAMBDA
        )
        l1_right_hand_side = LAMBDA * phi_u - LAMBDA * phi_e - source_l1
        gap = abs(l1_right_hand_side - change_l2)
        scale = max(1.0, abs(change_l2))
        print(
            f"  i={axis}  |L1 flux pair - L2 masked change| = {gap:.4g}"
            f"   |L2 change| = {abs(change_l2):.4g}"
            f"   ratio = {gap / scale:.4g}"
        )
        # The failure must be large: many orders above the (M2) tolerance and
        # commensurate with the quantity it fails to reproduce.
        LEDGER.check(
            "D'",
            f"L1 pair does not close L2 change i={axis} (absolute)",
            gap > 1e-6,
        )
        LEDGER.check(
            "D'",
            f"L1 pair failure is O(1) relative i={axis}",
            gap / scale > 1e-2,
        )
        # Positive control on the very same fixture and mask: L1's pair DOES
        # close L1's own change, so the failure above is a real mismatch of
        # localizations and not a broken flux construction.
        LEDGER.close(
            "C",
            f"positive control: L1 pair closes L1 change i={axis}",
            change_l1,
            l1_right_hand_side,
            LAMBDA * phi_u,
            LAMBDA * phi_e,
            source_l1,
        )
        NEGATIVE_CONTROL_GAPS.append(gap)


# ===========================================================================
# Group E -- Sec 2.6 chord census and true reach
# ===========================================================================
def operator(name: str, axis: int):
    if name == "D_i":
        return lambda f: central(f, axis)
    if name == "D_i C C^T":
        return lambda f: central(curl(curl_adjoint(f)), axis)
    if name == "D_i C^T C":
        return lambda f: central(curl_adjoint(curl(f)), axis)
    if name == "D_i C^T":
        return lambda f: central(curl_adjoint(f), axis)
    if name == "C C^T":
        return lambda f: curl(curl_adjoint(f))
    raise KeyError(name)


def impulse_response(op, size: int, threshold: float = 1e-13) -> dict:
    """Extract `N_r[a][b]` by impulse response: `N_r[a][b] = (N e_b d_0)_a(-r)`."""
    entries: dict[tuple[tuple[int, int, int], int, int], float] = {}
    for b in range(3):
        impulse = np.zeros((3, size, size, size))
        impulse[b, 0, 0, 0] = 1.0
        response = op(impulse)
        for a in range(3):
            for site in np.argwhere(np.abs(response[a]) > threshold):
                raw = tuple(int((-value) % size) for value in site)
                displacement = tuple(
                    value - size if value > size // 2 else value for value in raw
                )
                entries[(displacement, a, b)] = float(response[a][tuple(site)])
    return entries


def representative(key):
    displacement, a, b = key
    mirror = (tuple(-value for value in displacement), b, a)
    return max(key, mirror)


def census(entries: dict) -> tuple[int, int, int, int, int]:
    displacements = {key[0] for key in entries}
    classes = {representative(key) for key in entries}
    l1 = max(sum(abs(v) for v in r) for r in displacements)
    linf = max(max(abs(v) for v in r) for r in displacements)
    return len(displacements), len(entries), len(classes), l1, linf


def skew_defect(entries: dict) -> float:
    """`max |N_{-r} + N_r^T|`, the Sec 2.2 relation (S)."""
    keys = set(entries) | {
        (tuple(-v for v in r), b, a) for (r, a, b) in entries
    }
    worst = 0.0
    for (r, a, b) in keys:
        mirrored = entries.get((tuple(-v for v in r), b, a), 0.0)
        worst = max(worst, abs(mirrored + entries.get((r, a, b), 0.0)))
    return worst


def group_e() -> dict:
    size = 11
    extracted: dict[tuple[str, int], dict] = {}
    for name, expected in CENSUS.items():
        observed = []
        for axis in range(3):
            entries = impulse_response(operator(name, axis), size)
            extracted[(name, axis)] = entries
            observed.append(census(entries))
        LEDGER.check(
            "E",
            f"{name}: census i-independent",
            observed[0] == observed[1] == observed[2],
        )
        displacements, count, classes, l1, linf = observed[0]
        LEDGER.check(
            "E",
            f"{name}: {expected[0]} displacements / {expected[1]} entries / "
            f"max|r|_1 = {expected[3]} / max|r|_inf = {expected[4]}",
            (displacements, count, l1, linf)
            == (expected[0], expected[1], expected[3], expected[4]),
        )
        if expected[2] is not None:
            LEDGER.check(
                "E", f"{name}: {expected[2]} R+ classes", classes == expected[2]
            )

    for name in SKEW_OPERATORS:
        for axis in range(3):
            defect = skew_defect(extracted[(name, axis)])
            LEDGER.zero("E", f"(S) N_-r = -N_r^T for {name} i={axis}", defect)

    # The pre-registration lists exactly three skew operators.  D_i C^T and
    # C C^T must NOT satisfy (S); asserting that keeps the list honest.
    for name in ("D_i C^T", "C C^T"):
        LEDGER.check(
            "E",
            f"{name} is not skew (correctly outside the (S) list)",
            skew_defect(extracted[(name, 0)]) > 1e-9,
        )

    for name in ("D_i C C^T", "D_i C^T C"):
        entries = extracted[(name, 0)]
        displacement_split = Counter(
            sum(abs(v) for v in r) for r in {key[0] for key in entries}
        )
        class_split = Counter(
            sum(abs(v) for v in key[0]) for key in {representative(k) for k in entries}
        )
        LEDGER.check(
            "E",
            f"{name}: |r|_1 displacement split 1,6,10,8",
            dict(displacement_split) == DISPLACEMENT_SPLIT,
        )
        LEDGER.check(
            "E",
            f"{name}: R+ class split 2,11,20,4",
            dict(class_split) == CLASS_SPLIT,
        )

    # Sec 2.6: the l_inf reach is 2 -- and it is 2 only along the D_i axis.
    for name in ("D_i C C^T", "D_i C^T C"):
        for axis in range(3):
            entries = extracted[(name, axis)]
            per_axis = [
                max(abs(r[k]) for r in {key[0] for key in entries}) for k in range(3)
            ]
            expected = [2 if k == axis else 1 for k in range(3)]
            LEDGER.check(
                "E",
                f"{name} i={axis}: reach 2 along i, 1 transverse",
                per_axis == expected,
            )

    # D_i is component-diagonal; this is what makes its S-channel vanish
    # identically (Group F), for any mask whatsoever.
    for axis in range(3):
        LEDGER.check(
            "E",
            f"D_{axis} is component-diagonal",
            all(a == b for (_, a, b) in extracted[("D_i", axis)]),
        )
    return extracted


# ===========================================================================
# Group F -- Sec 2.3 unit-bond (T, S) construction
# ===========================================================================
def lexicographic_path(r: tuple[int, int, int]) -> list[tuple[int, int, int]]:
    """Frozen path convention (Sec 2.3, Banned move B5): x steps, then y, then z."""
    points = [(0, 0, 0)]
    cursor = [0, 0, 0]
    for axis in (0, 1, 2):
        step = 1 if r[axis] > 0 else -1
        for _ in range(abs(r[axis])):
            cursor[axis] += step
            points.append(tuple(cursor))
    return points


def build_stress_arrays(
    entries: dict, field: np.ndarray, size: int
) -> tuple[np.ndarray, np.ndarray]:
    """`T^(i)_{a,d}(v)` and `S^(i)_{a,b}(v)` of Sec 2.3, from the R+ classes.

    Precondition: `N` is skew, i.e. relation (S) of Sec 2.2 holds.  The chord
    form (R) that (U) refines is derived from (S), so pairing `(r,a,b)` with
    `(-r,b,a)` is only meaningful for a skew operator.
    """
    defect = skew_defect(entries)
    if defect > 1e-12:
        raise ValueError(
            "build_stress_arrays requires a skew operator (Sec 2.2 relation "
            f"(S)); max |N_-r + N_r^T| = {defect:.3e}"
        )
    stress = np.zeros((3, 3, size, size, size))
    spin = np.zeros((3, 3, size, size, size))
    classes = {representative(key) for key in entries}
    for (r, a, b) in classes:
        coefficient = entries[(r, a, b)]
        # W_{r,a,b}(v) = N_r[a][b] f_a(v) f_b(v + r)
        weight = coefficient * field[a] * shift(field, r)[b]
        path = lexicographic_path(r)
        for k in range(len(path) - 1):
            step = tuple(path[k + 1][j] - path[k][j] for j in range(3))
            axis = next(j for j in range(3) if step[j] != 0)
            if step[axis] > 0:
                base, sign = path[k], 1.0
            else:
                base, sign = path[k + 1], -1.0
            # T accumulates (+-1) * W(v - base)
            stress[a, axis] += sign * np.roll(weight, base, axis=(0, 1, 2))
        # S accumulates W(v - r)
        spin[a, b] += np.roll(weight, r, axis=(0, 1, 2))
    return stress, spin


def flux_from_arrays(
    stress: np.ndarray, spin: np.ndarray, mask: np.ndarray
) -> tuple[float, float]:
    """Evaluate (U); returns (total Phi, S-channel contribution)."""
    bond = 0.0
    for a in range(3):
        for d in range(3):
            step = [0, 0, 0]
            step[d] = 1
            bond += float(
                np.sum(stress[a, d] * (mask[a] - shift(mask, tuple(step))[a]))
            )
    site = 0.0
    for a in range(3):
        for b in range(3):
            site += float(np.sum(spin[a, b] * (mask[a] - mask[b])))
    return bond + site, site


def group_f(extracted: dict) -> None:
    size = 11
    rng = np.random.default_rng(0xF)
    electric = rng.standard_normal((3, size, size, size))
    magnetic = rng.standard_normal((3, size, size, size))
    magnetic_after = magnetic - LAMBDA * curl_adjoint(electric)
    face = curl(magnetic_after)
    edge = curl_adjoint(electric)

    # ---- the frozen path convention, pinned directly ----------------------
    # Sec 2.3 / Banned move B5 freeze the unit-step order (x, then y, then z).
    # No gate in Sec 2.11 or Sec 6.4 G3 can detect a change to it: Phi is the
    # sum over the whole path and is path-INDEPENDENT, so only the T^(i)/S^(i)
    # split -- the object that carries the stress-ledger interpretation --
    # moves.  This block therefore pins the convention itself, which the
    # flux-closure checks below structurally cannot.
    expected_paths = {
        (1, 1, 1): [(0, 0, 0), (1, 0, 0), (1, 1, 0), (1, 1, 1)],
        (-1, 2, 0): [(0, 0, 0), (-1, 0, 0), (-1, 1, 0), (-1, 2, 0)],
        (0, 0, -2): [(0, 0, 0), (0, 0, -1), (0, 0, -2)],
        (2, 0, 1): [(0, 0, 0), (1, 0, 0), (2, 0, 0), (2, 0, 1)],
    }
    for displacement, expected in expected_paths.items():
        LEDGER.check(
            "F",
            f"frozen lexicographic path for r={displacement} (B5)",
            lexicographic_path(displacement) == expected,
        )
        LEDGER.check(
            "F",
            f"path for r={displacement} has |r|_1 unit steps",
            len(lexicographic_path(displacement)) - 1
            == sum(abs(v) for v in displacement),
        )

    centre = (5, 5, 5)
    masks = (
        ("site mask", chebyshev_mask(centre, 3, size)),
        ("per-component mask", component_mask(size, centre)),
    )
    # the four operators of Sec 4 item 1
    operators = (
        ("Phi^(u)", "D_i", face),
        ("Phi^(E)", "D_i C C^T", electric),
        ("Phi^(w)", "D_i", edge),
        ("Phi^(B')", "D_i C^T C", magnetic_after),
    )

    spin_report: dict[tuple[str, int], float] = {}
    for label, name, field in operators:
        for axis in range(3):
            entries = extracted[(name, axis)]
            # (U) refines the chord form (R), which is derived from (S).  If
            # the operator is not skew the construction is undefined; record
            # that as a failed check rather than aborting the report.
            LEDGER.check(
                "F",
                f"{label} operator {name} is skew, so (U) applies, i={axis}",
                skew_defect(entries) <= 1e-12,
            )
            if skew_defect(entries) > 1e-12:
                continue
            stress, spin = build_stress_arrays(entries, field, size)
            act = operator(name, axis)
            for mask_name, mask in masks:
                direct = inner(mask * field, act(field))
                via, site_channel = flux_from_arrays(stress, spin, mask)
                LEDGER.close(
                    "F",
                    f"{label} via (T,S) vs direct, i={axis} [{mask_name}]",
                    via,
                    direct,
                )
                complement, _ = flux_from_arrays(stress, spin, 1.0 - mask)
                LEDGER.zero(
                    "F",
                    f"{label} complementarity Phi[chi]+Phi[1-chi]=0, "
                    f"i={axis} [{mask_name}]",
                    via + complement,
                    via,
                )
                if mask_name == "site mask":
                    # Sec 2.3 site-mask collapse: the S term drops out exactly
                    LEDGER.check(
                        "F",
                        f"{label} S-channel exactly zero under site mask i={axis}",
                        site_channel == 0.0,
                    )
                else:
                    spin_report[(label, axis)] = site_channel

    # Sec 6.4 G3 requires the per-component pre-check to exercise S.  It can
    # only be exercised through the binding operators: D_i is component-
    # diagonal (Group E), so its S array lives entirely on a == b and the
    # factor chi_a - chi_b annihilates it for every mask.
    for label in ("Phi^(E)", "Phi^(B')"):
        for axis in range(3):
            value = spin_report.get((label, axis))
            LEDGER.check(
                "F",
                f"{label} S-channel non-zero under per-component mask i={axis}",
                value is not None and abs(value) > 1e-6,
            )
    for label in ("Phi^(u)", "Phi^(w)"):
        for axis in range(3):
            value = spin_report.get((label, axis))
            LEDGER.check(
                "F",
                f"{label} S-channel identically zero (D_i is component-diagonal) "
                f"i={axis}",
                value == 0.0,
            )
    LEDGER.note(
        "The Sec 2.3 path convention is invisible to every flux check.  Phi "
        "is the sum over a complete telescoping path, so reordering the unit "
        "steps (x,y,z -> z,y,x) leaves every closure in Sec 2.11 and every "
        "Sec 6.4 G3 flux check EXACTLY unchanged while moving the T^(i) array "
        "itself.  Verified by mutation: reversing the order changes no "
        "residual anywhere in this suite.  Banned move B5 is therefore "
        "load-bearing for the stress-ledger INTERPRETATION of T^(i) and "
        "cannot be enforced by the exactness pre-check; it has to be enforced "
        "by the hash.  This script pins the path directly instead."
    )
    LEDGER.note(
        "Sec 2.3 reports a non-zero S-channel under the per-component mask.  "
        "That is reproduced, but only for the two binding operators "
        "(D_i C C^T, D_i C^T C).  For N = D_i the S array is structurally "
        "confined to a == b, so Phi^(u) and Phi^(w) contribute exactly zero "
        "to the S channel under ANY mask.  The Sec 6.4 G3 requirement 'the "
        "per-component case must produce a non-zero S contribution' is "
        "therefore satisfiable only via the binding pair."
    )
    print()
    print("Group F S-channel under the per-component mask (Sec 2.3):")
    for label in ("Phi^(u)", "Phi^(E)", "Phi^(w)", "Phi^(B')"):
        values = [spin_report.get((label, axis)) for axis in range(3)]
        print(
            f"  {label:9s} i=x,y,z: "
            + ", ".join(
                "n/a" if value is None else f"{value:+.4g}" for value in values
            )
        )


# ===========================================================================
# Groups G, H, J -- Sec 2.7 / 2.8 / 2.9 on the moving-mask fixture
# ===========================================================================
FIXTURE_SIZE = 15
FIXTURE_TICKS = 24
FIXTURE_RADII = (4, 6)
SOURCE_HALF_WIDTH = 1


def fixture_centre(tick: int) -> tuple[int, int, int]:
    """Rounded lattice site of a core moving along a non-axis-aligned track."""
    return (7 + (tick * 7) // FIXTURE_TICKS, 7, 7 + (tick * 5) // FIXTURE_TICKS)


def fixture_current(tick: int, centre: tuple[int, int, int]) -> np.ndarray:
    """A compact current co-moving with the mask centre, half-width 1."""
    current = np.zeros((3, FIXTURE_SIZE, FIXTURE_SIZE, FIXTURE_SIZE))
    values = np.random.default_rng(9000 + tick).standard_normal((3, 3, 3, 3))
    span = range(-SOURCE_HALF_WIDTH, SOURCE_HALF_WIDTH + 1)
    for dx in span:
        for dy in span:
            for dz in span:
                current[
                    :,
                    (centre[0] + dx) % FIXTURE_SIZE,
                    (centre[1] + dy) % FIXTURE_SIZE,
                    (centre[2] + dz) % FIXTURE_SIZE,
                ] = 0.05 * values[:, dx + 1, dy + 1, dz + 1]
    return current


def run_moving_fixture() -> dict:
    """Accumulate (L) tick by tick, both localizations, three region kinds."""
    rng = np.random.default_rng(0x6768)
    electric = rng.standard_normal((3, FIXTURE_SIZE, FIXTURE_SIZE, FIXTURE_SIZE))
    magnetic = rng.standard_normal((3, FIXTURE_SIZE, FIXTURE_SIZE, FIXTURE_SIZE))

    regions = list(FIXTURE_RADII) + [None]  # None = whole domain
    localizations = ("L1", "L2")

    def mask_for(radius, tick):
        if radius is None:
            return whole_domain(FIXTURE_SIZE)
        return chebyshev_mask(fixture_centre(tick), radius, FIXTURE_SIZE)

    def content(radius, tick, localization, e_field, b_field, axis):
        mask = mask_for(radius, tick)
        density = (
            density_l1(e_field, b_field, axis)
            if localization == "L1"
            else density_l2(e_field, b_field, axis)
        )
        return float(np.sum(mask * density))

    initial = {
        (loc, radius, axis): content(radius, 0, loc, electric, magnetic, axis)
        for loc in localizations
        for radius in regions
        for axis in range(3)
    }
    accumulator = {
        (loc, radius, axis, term): 0.0
        for loc in localizations
        for radius in regions
        for axis in range(3)
        for term in ("F", "W", "Q")
    }
    # shell accumulators, built from the difference mask directly (Sec 2.8)
    shell = {
        (loc, axis, term): 0.0
        for loc in localizations
        for axis in range(3)
        for term in ("F", "W")
    }
    worst_tick_residual = 0.0

    for tick in range(FIXTURE_TICKS):
        centre_old = fixture_centre(tick)
        current = fixture_current(tick, centre_old)
        electric_after, magnetic_after, face = staggered_step(
            electric, magnetic, current, LAMBDA
        )
        edge = curl_adjoint(electric)
        bind_e = curl(curl_adjoint(electric))
        bind_b = curl_adjoint(curl(magnetic_after))
        source_l2 = curl_adjoint(current)

        old_masks = {r: mask_for(r, tick) for r in regions}
        new_masks = {r: mask_for(r, tick + 1) for r in regions}

        for axis in range(3):
            d_face = central(face, axis)
            d_bind_e = central(bind_e, axis)
            d_edge = central(edge, axis)
            d_bind_b = central(bind_b, axis)
            d_source_l2 = central(source_l2, axis)
            pi_l1 = density_l1(electric, magnetic, axis)
            pi_l2 = density_l2(electric, magnetic, axis)

            for radius in regions:
                chi_old, chi_new = old_masks[radius], new_masks[radius]
                flux_l1 = LAMBDA * (
                    inner(chi_new * face, d_face) - inner(chi_new * electric, d_bind_e)
                )
                sweep_l1 = float(np.sum((chi_new - chi_old) * pi_l1))
                source_term_l1 = inner(chi_new * current, d_face)
                accumulator[("L1", radius, axis, "F")] += flux_l1
                accumulator[("L1", radius, axis, "W")] += sweep_l1
                accumulator[("L1", radius, axis, "Q")] += source_term_l1

                flux_l2 = LAMBDA * (
                    inner(chi_new * edge, d_edge)
                    - inner(chi_new * magnetic_after, d_bind_b)
                )
                sweep_l2 = float(np.sum((chi_new - chi_old) * pi_l2))
                source_term_l2 = -inner(chi_new * magnetic_after, d_source_l2)
                accumulator[("L2", radius, axis, "F")] += flux_l2
                accumulator[("L2", radius, axis, "W")] += sweep_l2
                accumulator[("L2", radius, axis, "Q")] += source_term_l2

                # per-tick G1 pairing residual (Sec 6.2)
                if radius is not None:
                    after_l1 = density_l1(electric_after, magnetic_after, axis)
                    total = float(
                        np.sum(chi_new * after_l1) - np.sum(chi_old * pi_l1)
                    )
                    material = float(np.sum(chi_new * (after_l1 - pi_l1)))
                    denominator = max(1.0, abs(total), abs(material), abs(sweep_l1))
                    worst_tick_residual = max(
                        worst_tick_residual,
                        abs(total - material - sweep_l1) / denominator,
                    )

            # shell mask, built once from the difference (Sec 2.8 linearity)
            chi_shell_old = old_masks[FIXTURE_RADII[1]] - old_masks[FIXTURE_RADII[0]]
            chi_shell_new = new_masks[FIXTURE_RADII[1]] - new_masks[FIXTURE_RADII[0]]
            shell[("L1", axis, "F")] += LAMBDA * (
                inner(chi_shell_new * face, d_face)
                - inner(chi_shell_new * electric, d_bind_e)
            )
            shell[("L1", axis, "W")] += float(
                np.sum((chi_shell_new - chi_shell_old) * pi_l1)
            )
            shell[("L2", axis, "F")] += LAMBDA * (
                inner(chi_shell_new * edge, d_edge)
                - inner(chi_shell_new * magnetic_after, d_bind_b)
            )
            shell[("L2", axis, "W")] += float(
                np.sum((chi_shell_new - chi_shell_old) * pi_l2)
            )

        electric, magnetic = electric_after, magnetic_after

    final = {
        (loc, radius, axis): content(
            radius, FIXTURE_TICKS, loc, electric, magnetic, axis
        )
        for loc in localizations
        for radius in regions
        for axis in range(3)
    }
    return {
        "initial": initial,
        "final": final,
        "accumulator": accumulator,
        "shell": shell,
        "regions": regions,
        "localizations": localizations,
        "worst_tick_residual": worst_tick_residual,
    }


def group_g_h_j(fixture: dict) -> None:
    initial = fixture["initial"]
    final = fixture["final"]
    acc = fixture["accumulator"]
    shell = fixture["shell"]

    LEDGER.zero(
        "G",
        "G1 per-tick material/sweep pairing residual",
        fixture["worst_tick_residual"],
    )

    whole_change: dict[tuple[str, int], float] = {}
    for loc in fixture["localizations"]:
        for radius in fixture["regions"]:
            for axis in range(3):
                change = final[(loc, radius, axis)] - initial[(loc, radius, axis)]
                flux = acc[(loc, radius, axis, "F")]
                sweep = acc[(loc, radius, axis, "W")]
                source = acc[(loc, radius, axis, "Q")]
                tag = "whole" if radius is None else f"R={radius}"
                LEDGER.close(
                    "G",
                    f"(L) {loc} {tag} i={axis}",
                    change,
                    flux + sweep - source,
                    flux,
                    sweep,
                    source,
                )
                if radius is None:
                    whole_change[(loc, axis)] = change
                    # Sec 2.7 whole-domain limit
                    LEDGER.zero("G", f"F = 0 at chi=1, {loc} i={axis}", flux)
                    LEDGER.check(
                        "G", f"W = 0 identically at chi=1, {loc} i={axis}", sweep == 0.0
                    )
                    LEDGER.zero(
                        "G",
                        f"Delta P + Q = 0 at chi=1, {loc} i={axis}",
                        change + source,
                        source,
                    )

    LEDGER.note(
        "Sensitivity, measured by mutation rather than asserted: (M1), (M2), "
        "(L) and (H) are rearrangements of their own definitions and survive "
        "operator corruption.  Flipping a sign in C^T -- so that C^T is no "
        "longer the adjoint of C and M is no longer symmetric -- leaves every "
        "(M1) and (H) closure intact while breaking groups A, B, D, E, F, G "
        "and J.  This is the quantitative form of the Sec 1 warning that the "
        "regional identity 'can fail only on implementation, never on "
        "physics', and of Banned move B1: a ledger that closes is evidence "
        "about wiring, not about the substrate.  The premises with real "
        "content are the Group A adjoint/skew relations and the Group B "
        "identity (G); everything downstream inherits from those."
    )

    # the two localizations must agree on the whole-domain total (Sec 2.5)
    for axis in range(3):
        LEDGER.close(
            "G",
            f"L1 and L2 agree on Delta P^whole i={axis}",
            whole_change[("L1", axis)],
            whole_change[("L2", axis)],
        )

    # ---- Group H : Sec 2.8 shell corollary -------------------------------
    inner_radius, outer_radius = FIXTURE_RADII
    for loc in fixture["localizations"]:
        for axis in range(3):
            source_inner = acc[(loc, inner_radius, axis, "Q")]
            source_outer = acc[(loc, outer_radius, axis, "Q")]
            LEDGER.check(
                "H",
                f"Q(R2) = Q(R1) exactly, source enclosed, {loc} i={axis}",
                source_inner == source_outer,
            )
            LEDGER.record(
                "H", f"Q(R2)-Q(R1) {loc} i={axis}", abs(source_outer - source_inner), 0.0
            )

            delta_flux = (
                acc[(loc, outer_radius, axis, "F")] - acc[(loc, inner_radius, axis, "F")]
            )
            delta_sweep = (
                acc[(loc, outer_radius, axis, "W")] - acc[(loc, inner_radius, axis, "W")]
            )
            change_outer = (
                final[(loc, outer_radius, axis)] - initial[(loc, outer_radius, axis)]
            )
            change_inner = (
                final[(loc, inner_radius, axis)] - initial[(loc, inner_radius, axis)]
            )
            LEDGER.close(
                "H",
                f"(H) shell identity {loc} i={axis}",
                delta_flux + delta_sweep,
                change_outer - change_inner,
                delta_flux,
                delta_sweep,
            )
            # (L) is linear in chi: the shell accumulators built from the
            # difference mask must equal the difference of the accumulators.
            LEDGER.close(
                "H",
                f"linearity in chi: F[shell] = F(R2)-F(R1) {loc} i={axis}",
                shell[(loc, axis, "F")],
                delta_flux,
            )
            LEDGER.close(
                "H",
                f"linearity in chi: W[shell] = W(R2)-W(R1) {loc} i={axis}",
                shell[(loc, axis, "W")],
                delta_sweep,
            )

    # ---- Group J : Sec 2.9 retention normalization ------------------------
    print()
    print("Group J retention/transfer on the moving fixture (Sec 2.9):")
    for loc in fixture["localizations"]:
        for axis in range(3):
            whole = whole_change[(loc, axis)]
            retentions = {}
            for radius in FIXTURE_RADII:
                change = final[(loc, radius, axis)] - initial[(loc, radius, axis)]
                transfer_numerator = (
                    acc[(loc, radius, axis, "F")] + acc[(loc, radius, axis, "W")]
                )
                eta = change / whole
                tau = transfer_numerator / whole
                retentions[radius] = (eta, tau)
                LEDGER.close(
                    "J",
                    f"(N) eta - tau = 1, {loc} R={radius} i={axis}",
                    eta - tau,
                    1.0,
                    eta,
                    tau,
                )
                if loc == "L1":
                    print(
                        f"  {loc} R={radius} i={axis}: eta = {eta:+.6g}  "
                        f"tau = {tau:+.6g}  |eta|+|tau| = {abs(eta) + abs(tau):.4g}"
                    )
                # Sec 2.9(iii): |eta| + |tau| >= 1 always, once enclosed
                LEDGER.check(
                    "J",
                    f"|eta|+|tau| >= 1 (no 'nothing anywhere'), {loc} "
                    f"R={radius} i={axis}",
                    abs(eta) + abs(tau) >= 1.0 - 1e-9,
                )
            eta_inner, tau_inner = retentions[inner_radius]
            eta_outer, tau_outer = retentions[outer_radius]
            LEDGER.close(
                "J",
                f"shell form eta(R2)-eta(R1) = tau(R2)-tau(R1), {loc} i={axis}",
                eta_outer - eta_inner,
                tau_outer - tau_inner,
                eta_outer,
                eta_inner,
            )


# ===========================================================================
# Group U -- Sec 3 interaction_scale unit convention
# ===========================================================================
def read_engine_double(path: Path, name: str) -> float:
    text = path.read_text(encoding="utf-8")
    match = re.search(
        rf"inline\s+constexpr\s+double\s+{re.escape(name)}\s*=\s*([0-9.eE+-]+)\s*;",
        text,
    )
    if match is None:
        raise LookupError(f"{name} not found in {path}")
    return float(match.group(1))


def group_u(fixture: dict) -> None:
    pipeline = CUDA_PIPELINE.read_text(encoding="utf-8")

    # Sec 3 cites cuda_matched_field_pipeline.cu:1204-1206 for the weighting.
    for component in range(3):
        LEDGER.check(
            "U",
            f"cuda pipeline weights local-momentum axis {component} "
            f"by interaction_scale",
            f"interaction_scale*static_cast<double>(sum[{component}])" in pipeline,
        )
    LEDGER.check(
        "U",
        "cuda pipeline takes interaction_scale as a runtime parameter",
        "double polarity_scale,double interaction_scale,double wave_speed," in pipeline,
    )
    # Honest provenance: the numeric literal is NOT in that translation unit.
    literal_in_pipeline = "0.021892057692994273" in pipeline
    LEDGER.check(
        "U",
        "interaction_scale literal is absent from cuda_matched_field_pipeline.cu "
        "(it is a runtime parameter there)",
        not literal_in_pipeline,
    )

    # Byte-exact reconstruction from the engine's own constants of record.
    normalization = FACE_NORMALIZATION.read_text(encoding="utf-8")
    LEDGER.check(
        "U",
        "face_flux_normalization.h defines the mapped work coefficient as "
        "energy_scale * field_scale * current_scale",
        "result.energy_scale * result.field_scale * result.current_scale"
        in normalization,
    )
    LEDGER.check(
        "U",
        "face_flux_normalization.h sets field_scale = G_C / C_WAVE^2",
        "result.native_susceptibility = G_C / c2;" in normalization
        and "result.field_scale = result.native_susceptibility;" in normalization,
    )
    g_c = read_engine_double(GAUGE_COUPLINGS, "G_C")
    c_wave = read_engine_double(GAUGE_COUPLINGS, "C_WAVE")
    c_speed = read_engine_double(GAUGE_COUPLINGS, "C_SPEED")
    c_squared = c_wave * c_wave
    field_scale = g_c / c_squared
    reconstructed = c_squared * field_scale * field_scale
    LEDGER.check(
        "U",
        "interaction_scale reconstructed byte-exactly from engine G_C and C_WAVE",
        reconstructed == INTERACTION_SCALE,
    )
    LEDGER.check(
        "U",
        "engine C_SPEED and C_WAVE are the same double",
        c_speed == c_wave,
    )

    # Sec 0's lambda literal, and the engine's own.
    LEDGER.check(
        "U",
        "Sec 0 lambda literal is self-consistent",
        C_SPEED_PREREG * DT == 0.14433756729740646,
    )
    engine_lambda = c_speed * DT
    ulp_gap = abs(engine_lambda - LAMBDA)
    LEDGER.check(
        "U",
        "Sec 0 lambda and engine lambda agree to within one ULP",
        0.0 < ulp_gap < 1e-16,
    )
    LEDGER.note(
        "Sec 0 quotes C_SPEED = 0.5773502691896258, the correctly rounded "
        "double for 1/sqrt(3).  The engine literal "
        "(gauge_couplings.h: 0.57735026918962576451) parses to "
        f"{c_speed!r}, one ULP below it, so the engine's lambda is "
        f"{engine_lambda!r} against Sec 0's {LAMBDA!r}.  Every Sec 2 identity "
        "is exact for arbitrary lambda (Group B closes at three different "
        "values), so nothing here is load-bearing -- but the Sec 3 "
        "interaction_scale is byte-exact ONLY against the engine literal: "
        "rebuilding it from Sec 0's C_SPEED gives 0.02189205769299426."
    )
    off_by_one_ulp = (C_SPEED_PREREG * C_SPEED_PREREG)
    rebuilt_from_prereg = off_by_one_ulp * (g_c / off_by_one_ulp) ** 2
    LEDGER.check(
        "U",
        "interaction_scale rebuilt from Sec 0's C_SPEED does NOT reproduce "
        "the Sec 3 literal (disclosed, not hidden)",
        rebuilt_from_prereg != INTERACTION_SCALE,
    )

    # Sec 3: every Sec 2 identity is exactly scale-invariant, so the weighting
    # rescales Pi, F, W, Q by exactly the factor and leaves eta, tau fixed.
    acc = fixture["accumulator"]
    initial, final = fixture["initial"], fixture["final"]
    for axis in range(3):
        whole = final[("L1", None, axis)] - initial[("L1", None, axis)]
        for radius in FIXTURE_RADII:
            change = final[("L1", radius, axis)] - initial[("L1", radius, axis)]
            flux = acc[("L1", radius, axis, "F")]
            sweep = acc[("L1", radius, axis, "W")]
            eta = change / whole
            tau = (flux + sweep) / whole
            weighted_eta = (INTERACTION_SCALE * change) / (INTERACTION_SCALE * whole)
            weighted_tau = (
                INTERACTION_SCALE * (flux + sweep)
            ) / (INTERACTION_SCALE * whole)
            LEDGER.close(
                "U", f"eta invariant under weighting R={radius} i={axis}",
                weighted_eta, eta,
            )
            LEDGER.close(
                "U", f"tau invariant under weighting R={radius} i={axis}",
                weighted_tau, tau,
            )
            LEDGER.close(
                "U",
                f"(L) survives weighting R={radius} i={axis}",
                INTERACTION_SCALE * change,
                INTERACTION_SCALE * (flux + sweep - acc[("L1", radius, axis, "Q")]),
                INTERACTION_SCALE * flux,
            )


# ===========================================================================
# Group L -- Sec 12 lock record: protocol_sha256 self-consistency and the
# engine artifact's compile-time constants, independently re-derived here
# rather than trusted from either source alone.
# ===========================================================================
def group_l() -> None:
    text = PREREG.read_text(encoding="utf-8")
    raw = PREREG.read_bytes()

    marker = b"**Protocol lock:** `protocol_sha256="
    idx = raw.find(marker)
    LEDGER.check("L", "protocol_sha256 marker present in the pre-registration",
                 idx != -1)
    if idx == -1:
        return
    prefix = raw[:idx]
    LEDGER.check("L", "byte-prefix ends on a newline boundary",
                 prefix.endswith(b"\n"))
    recomputed = hashlib.sha256(prefix).hexdigest().upper()

    match = re.search(r"protocol_sha256=([0-9A-Fa-f]{64})`", text)
    LEDGER.check("L", "pre-registration states a 64-hex-digit protocol_sha256",
                 match is not None)
    if match is None:
        return
    declared = match.group(1).upper()
    LEDGER.check(
        "L",
        "declared protocol_sha256 equals SHA-256 of the file's own byte-prefix",
        declared == recomputed,
    )
    LEDGER.check("L", "declared protocol_sha256 is not the UNLOCKED placeholder",
                 declared != "UNLOCKED" and declared != "PENDING")

    id_match = re.search(r"^# (FTD-\d{4}) ", text, re.MULTILINE)
    LEDGER.check("L", "pre-registration title carries a real FTD-NNNN id "
                       "(not the FTD-XXXX placeholder)",
                 id_match is not None and id_match.group(1) != "FTD-XXXX")
    ftd_id = id_match.group(1) if id_match else None

    engine = CAMPAIGN_HARNESS.read_text(encoding="utf-8")

    def engine_string_constant(name: str) -> str | None:
        m = re.search(
            rf'constexpr\s+char\s+{re.escape(name)}\[\]\s*=\s*"([^"]*)"', engine
        )
        return m.group(1) if m else None

    engine_id = engine_string_constant("kMomentumFtdId")
    LEDGER.check("L", "engine kMomentumFtdId is present and not the placeholder",
                 engine_id is not None and engine_id != "FTD-XXXX")
    LEDGER.check("L", "engine kMomentumFtdId matches the pre-registration's id",
                 ftd_id is not None and engine_id == ftd_id)

    engine_slug = engine_string_constant("kMomentumResultSlug")
    expected_slug = f"ftd_{ftd_id.split('-')[1]}" if ftd_id else None
    LEDGER.check("L", "engine kMomentumResultSlug matches the minted id",
                 engine_slug is not None and engine_slug == expected_slug)

    # The constant may be split across lines by the formatter; strip
    # whitespace and quote-concatenation before comparing.
    hash_block = re.search(
        r"kMomentumProtocolSha256\[\]\s*=\s*((?:\"[0-9A-Fa-f]*\"\s*)+);", engine
    )
    LEDGER.check("L", "engine kMomentumProtocolSha256 constant found", hash_block
                 is not None)
    if hash_block is not None:
        engine_hash = "".join(re.findall(r'"([0-9A-Fa-f]*)"', hash_block.group(1)))
        LEDGER.check(
            "L",
            "engine kMomentumProtocolSha256 is not the UNLOCKED placeholder",
            engine_hash != "UNLOCKED",
        )
        LEDGER.check(
            "L",
            "engine kMomentumProtocolSha256 matches the pre-registration's "
            "declared hash",
            engine_hash.upper() == declared,
        )
    LEDGER.note(
        f"Group L re-derives the lock record from scratch: protocol_sha256 "
        f"recomputed as {recomputed} directly from the committed "
        f"pre-registration's own bytes (not trusted from the declared field), "
        f"matched against both the file's own `protocol_sha256=` line and the "
        f"engine's compile-time constant. This is the check Sec 12 item 6 "
        f"requires before the hash may be treated as locked."
    )


# ===========================================================================
# Driver
# ===========================================================================
def main() -> int:
    print("FTD total momentum stress ledger -- independent numpy verification")
    print(f"pre-registration: {PREREG.relative_to(ROOT).as_posix()}")
    print(f"lambda = {LAMBDA!r}   interaction_scale = {INTERACTION_SCALE!r}")

    if not PREREG.is_file():
        print(f"FAIL: pre-registration not found at {PREREG}")
        return 1

    group_a()
    group_b()
    group_c_d()
    extracted = group_e()
    group_f(extracted)
    fixture = run_moving_fixture()
    group_g_h_j(fixture)
    group_u(fixture)
    group_l()

    order = ("A", "B", "C", "D", "D'", "E", "F", "G", "H", "J", "U", "L")
    labels = {
        "A": "adjoint / skew / commutators           (Sec 2.1)",
        "B": "global identity (G), L=9               (Sec 2.1)",
        "C": "M1 regional identity, L=9              (Sec 2.4)",
        "D": "M2 regional identity + agreements      (Sec 2.5)",
        "D'": "NEGATIVE CONTROL: L1 pair vs L2 change (Sec 2.5)",
        "E": "chord census and true reach, L=11      (Sec 2.6)",
        "F": "unit-bond (T,S) construction, L=11     (Sec 2.3)",
        "G": "cumulative moving-mask ledger, L=15    (Sec 2.7)",
        "H": "shell corollary + enclosure            (Sec 2.8)",
        "J": "eta - tau = 1 and shell form           (Sec 2.9)",
        "U": "interaction_scale unit convention      (Sec 3)",
        "L": "protocol_sha256 lock record            (Sec 12)",
    }
    quoted = {
        "A": "5.7e-14", "B": "3.6e-14", "C": "1.6e-14", "D": "2.9e-14",
        "D'": "O(1) failure", "E": "0.0", "F": "3.6e-14", "G": "1.7e-15",
        "H": "6.7e-16", "J": "1.5e-13", "U": "n/a", "L": "n/a",
    }

    print()
    print("=" * 88)
    print(
        f"{'group':6s} {'checks':>7s} {'status':>7s} "
        f"{'worst rel':>11s} {'worst abs':>11s} {'tol':>8s} {'Sec 2.11':>13s}"
    )
    print("-" * 88)
    failed_groups = Counter()
    for failure in LEDGER.failures:
        failed_groups[failure.split("]")[0].lstrip("[")] += 1
    # recount per group from the ledger's own accounting
    for group in order:
        total = GROUP_COUNTS.get(group, 0)
        bad = failed_groups.get(group, 0)
        worst = LEDGER.worst.get(group)
        rel = f"{worst[0]:.2e}" if worst else "-"
        absolute = f"{worst[1]:.2e}" if worst else "-"
        tol = f"{TOLERANCE[group]:.0e}" if group in TOLERANCE else "-"
        status = "PASS" if bad == 0 else "FAIL"
        if group == "D'":
            # An O(1) failure IS the correct outcome for this row; report the
            # smallest observed failure, which is what the assertion binds on.
            rel = "n/a"
            absolute = (
                f"{min(NEGATIVE_CONTROL_GAPS):.2e}" if NEGATIVE_CONTROL_GAPS else "-"
            )
            tol = "-"
        print(
            f"{group:6s} {total:>7d} {status:>7s} {rel:>11s} {absolute:>11s} "
            f"{tol:>8s} {quoted[group]:>13s}"
        )
    print("-" * 88)
    passed = LEDGER.checks - len(LEDGER.failures)
    print(f"TOTAL  {LEDGER.checks:>7d} {'PASS' if not LEDGER.failures else 'FAIL':>7s}")
    print("=" * 88)
    for group in order:
        print(f"  {group:3s} {labels[group]}")

    if LEDGER.notes:
        print()
        print("Recorded observations (no tag moved, nothing promoted):")
        for index, note in enumerate(LEDGER.notes, start=1):
            print(f"  [{index}] {note}")

    print()
    print(f"{passed}/{LEDGER.checks} checks pass")
    if LEDGER.failures:
        for failure in LEDGER.failures:
            print(f"FAIL: {failure}")
        return 1
    return 0


# per-group check counts, filled by a wrapper around Ledger.check
GROUP_COUNTS: Counter = Counter()
_ORIGINAL_CHECK = Ledger.check


def _counting_check(self, group: str, label: str, condition: bool) -> None:
    GROUP_COUNTS[group] += 1
    _ORIGINAL_CHECK(self, group, label, condition)


Ledger.check = _counting_check  # type: ignore[assignment]


if __name__ == "__main__":
    raise SystemExit(main())
