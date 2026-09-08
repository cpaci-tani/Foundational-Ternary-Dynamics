# Phase 1: Hydrodynamic Limit of the Strict Φ Law — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Derive, exactly and with rigorous enclosures, the long-wavelength dispersion of the `phi-v2-staged-candidate-1` field sector; classify it by the pre-registered Navier–Stokes verdict rule; verify the Boltzmann closure with one registered WSL2 CUDA campaign; book the results at their tags and record the Phase 2 trigger decision.

**Architecture:** Three exact Python modules under `scripts/phi_v2_lattice/` (invariant census, dispersion operators, verdict clauses) built on the existing `channels`, `recovery_kinetic_response`, and `recovery_kinetic_reference` modules; python-flint `fmpq_mat` for the exact track at proxy coupling `r₀ = 1/695` and `arb_mat` for certified enclosures at physical `r(p)`. A C++ campaign instrument under `engine/strict/recovery_hydro/` calls the accepted CUDA `advance` unchanged and emits projected conserved moments; a Python campaign runner mirrors the carrier-campaign lock protocol. Documents follow the `engine/docs` strict-stack conventions.

**Tech Stack:** Python 3.13, numpy, python-flint 0.8 (`fmpz_mat`, `fmpq_mat`, `arb_mat`, `arb_poly`), pytest; C++17, CUDA 13.0 through WSL2 Ubuntu-22.04 (`/usr/local/cuda/bin/nvcc`, arch 120), CMake, OpenSSL; existing `engine/strict` runtime.

**Spec:** `docs/superpowers/specs/2026-09-07-phi-hydrodynamics-design.md` (sections 1–3 and 5–7 govern this plan; section 4 is Phase 2 and is planned separately after gate H1's verdict).

## Global Constraints

- Law under test is `phi-v2-staged-candidate-1`; the frozen collision hash is `D0BB71DBED7938ED286E1D6D91A16700DA31F4550E83B2FB3580CCC347B2BD25`. No kernel, table, or wave-1/wave-2 source file is edited. In particular `engine/strict/staged_cuda.cu`, `staged_runtime.cpp`, `frozen_tables.h`, and every existing `scripts/phi_v2_lattice/*.py` stay byte-identical (existing campaign locks hash them).
- Every derivation acceptance is an exact integer, rational, or a rigorous ball enclosure. No float enters an acceptance test. Floats appear only in the H2 prediction curve and in measured campaign data.
- No near-match search, no fitted target, no retuning of any registered quantity after a result is seen. A failed gate is retained with its witness.
- GPU execution only through `wsl -d Ubuntu-22.04`. CPU work uses `-j 24` or more. Windows native C++ (if any) uses `engine\build_native.bat shell`.
- Commit messages end with the substantive description; **no `Co-Authored-By` trailer and no "Generated with" footer**.
- Work on branch `strict-hydro-wave3` in a git worktree (superpowers:using-git-worktrees). Stage by explicit path only; run `git diff --cached --stat` before every commit; never `git add -A` (a concurrent session holds uncommitted files in the main tree).
- `docs/superpowers/` is gitignored: plan and spec commits use `git add -f`.
- Tests run from the repository root: `python -m pytest scripts/tests/phi_v2_lattice/<file> -q` (the directory's `conftest.py` puts `scripts/` on the path). Evidence runners run from the root with `$env:PYTHONPATH='scripts'` (PowerShell) or `PYTHONPATH=scripts` (bash).
- Epistemic tags in documents are exactly those the spec assigns: H0 `[THEOREM — finite, exact, scoped]`, H1 `[DERIVED — linearized Boltzmann (product) closure at the declared reference; correlation leakage not bounded here]`, H2 `[MEASURED]` or `[CLOSED NEGATIVE at the registered scope]`. Nothing is promoted.

---

## File Structure

**Created**

| Path | Responsibility |
|---|---|
| `scripts/phi_v2_lattice/recovery_hydro_invariants.py` | Gate H0: exact additive-invariant census, covariance check, isotropy table |
| `scripts/tests/phi_v2_lattice/test_recovery_hydro_invariants.py` | H0 tests |
| `scripts/phi_v2_lattice/recovery_hydro_dispersion.py` | Gate H1 operators: period series, bases, resolvent, effective 7×7 operators; exact (`fmpq`) and certified (`arb`) tracks; float64 map for H2 |
| `scripts/tests/phi_v2_lattice/test_recovery_hydro_dispersion.py` | H1 operator tests |
| `scripts/phi_v2_lattice/recovery_hydro_verdict.py` | Verdict clauses (closure block, characteristic polynomials, isotropy comparisons); exact and certified |
| `scripts/tests/phi_v2_lattice/test_recovery_hydro_verdict.py` | Verdict tests on synthetic operators |
| `scripts/phi_v2_lattice/experiments/run_recovery_wave3.py` | Evidence JSON for H0 and H1 |
| `engine/docs/DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md` | H1 contract (before results) and results (after) |
| `scripts/phi_v2_lattice/experiments/probe_strict_cuda_throughput.py` | H2 throughput probe |
| `engine/strict/recovery_hydro/hydro_main.cpp`, `engine/strict/recovery_hydro/CMakeLists.txt` | H2 CUDA campaign instrument |
| `scripts/phi_v2_lattice/recovery_hydro_campaign.py` | H2 cases, preparation, observable, prediction, lock, run, summary |
| `scripts/tests/phi_v2_lattice/test_recovery_hydro_campaign.py` | H2 runner tests |
| `engine/docs/PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md` | H2 preregistration |
| `engine/docs/AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md` | H2 independent audit |
| `engine/docs/DERIV_STRICT_HYDRODYNAMIC_SECTOR.md` | Phase 1 booking document and Phase 2 trigger decision |
| `engine/docs/PROGRAM_STRICT_RECOVERY_WAVE_3.md` | Wave ledger |
| `engine/docs/evidence/strict-recovery-wave3-exact.json`, `strict-hydro-throughput-2026-09.json`, `strict-hydro-response-v1.json` | Evidence |
| `docs/theory/07_assessment/core_ledgers/LEDGER_ROW_DRAFT_phi_hydrodynamics.md` | LEDGER row drafts |

**Modified**

| Path | Change |
|---|---|
| `engine/docs/PROGRAM_STRICT_DISCRETE_STACK.md` | Gate rows C3 and M4: one pointer sentence each to the wave-3 ledger |

---

### Task 1: Gate H0 — invariant census module

**Files:**
- Create: `scripts/phi_v2_lattice/recovery_hydro_invariants.py`
- Test: `scripts/tests/phi_v2_lattice/test_recovery_hydro_invariants.py`

**Interfaces:**
- Consumes: `channels.load_collision_tables()` (list of three dicts, sorted pair tuple → pair tuple), `channels.U(c)`, `channels.tangent(c)`, `channels.layer_value_of(c, layer)`, `channels.COLLISION_HASH`, `channels._hash_tables(tables)`.
- Produces: `u_powers() -> (U, powers)`, `covariance_violations() -> int`, `fixed_kernel() -> tuple[tuple[int,...],...]`, `locked_kernel(l0) -> tuple[tuple[int,...],...]` (sorted primitive integer rows, 192 entries each), `span_membership(basis, vectors) -> bool`, `tangent_rows()`, `layer_rows(layer)`, `fourth_rank_isotropy(velocities) -> Isotropy`, `VELOCITY_SETS`, `census() -> dict`.

- [ ] **Step 1: Write the failing tests**

```python
# scripts/tests/phi_v2_lattice/test_recovery_hydro_invariants.py
"""Exact invariant census (gate H0). No floats, no random draws."""
import json
import pytest

from phi_v2_lattice import recovery_hydro_invariants as H


def test_u_has_period_twelve_on_one_polarity():
    U, powers = H.u_powers()
    assert len(powers) == 12 and powers[0] == list(range(192))
    assert [U[i] for i in powers[11]] == list(range(192))


def test_layer_covariance_holds_exactly():
    assert H.covariance_violations() == 0


def test_fixed_kernel_is_population_only():
    kernel = H.fixed_kernel()
    assert kernel == ((1,) * 192,)


@pytest.mark.parametrize("l0", range(3))
def test_locked_kernel_dimension_is_seven(l0):
    assert len(H.locked_kernel(l0)) == 7


def test_tangent_lies_in_locked_span_but_not_fixed_span():
    assert H.span_membership(H.locked_kernel(0), H.tangent_rows())
    assert not H.span_membership(H.fixed_kernel(), H.tangent_rows())


def test_constant_plus_layer_zero_values_equal_locked_kernel_span():
    rows = ((1,) * 192,) + H.layer_rows(0)
    assert H.span_membership(H.locked_kernel(0), rows)
    assert H.span_membership(rows, H.locked_kernel(0))


@pytest.mark.parametrize("l0,power", [(2, 1), (1, 2)])
def test_locked_kernels_are_pullbacks_of_layer_zero_kernel(l0, power):
    # C_{q-1} = U C_q U^{-1}: the l0 kernel equals {w o U^{-power} : w in kernel(0)}.
    _, powers = H.u_powers()
    inverse = [0] * 192
    for i, j in enumerate(powers[power]):
        inverse[j] = i
    pulled = tuple(tuple(w[inverse[c]] for c in range(192)) for w in H.locked_kernel(0))
    assert H.span_membership(H.locked_kernel(l0), pulled)
    assert H.span_membership(pulled, H.locked_kernel(l0))


@pytest.mark.parametrize("name,xxxx,xxyy,iso", [
    ("body_diagonal_8", 8, 8, False),
    ("face_edge_18", 10, 4, False),
    ("moore_26", 18, 12, False),
    ("fchc_projected_18", 12, 4, True),
])
def test_fourth_rank_isotropy_table(name, xxxx, xxyy, iso):
    result = H.fourth_rank_isotropy(H.VELOCITY_SETS[name])
    assert (result.T_xxxx, result.T_xxyy, result.isotropic4) == (xxxx, xxyy, iso)
    assert result.isotropic2


def test_census_is_exact_and_serializable():
    census = H.census()
    encoded = json.dumps(census, sort_keys=True)
    assert census["fixed_dimension_per_polarity"] == 1
    assert census["locked_dimension_per_polarity"] == {"0": 7, "1": 7, "2": 7}
    assert census["covariance_violations"] == 0
    assert "float" not in encoded and "." not in json.dumps(census["isotropy"])
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `python -m pytest scripts/tests/phi_v2_lattice/test_recovery_hydro_invariants.py -q`
Expected: collection error `ModuleNotFoundError: No module named 'phi_v2_lattice.recovery_hydro_invariants'`.

- [ ] **Step 3: Write the module**

```python
# scripts/phi_v2_lattice/recovery_hydro_invariants.py
"""Exact additive-invariant census of the field sector (gate H0).

Fixed-weight invariants of the three collision layers plus streaming, the
locked-schedule precessing invariants w_t = w_0 o U^{-t} with collision layer
(l0 - t) mod 3, the layer covariance U C_q = C_{q-1} U, and fourth-rank
velocity isotropy. Every result is an exact integer; no float enters any
acceptance. Nothing here evolves a state or changes the law.
"""
from __future__ import annotations

from dataclasses import dataclass
from itertools import product
from math import gcd

import flint
import numpy as np

from . import channels as C

N = 192  # channels per polarity


def _tables():
    tables = C.load_collision_tables()
    if C._hash_tables(tables) != C.COLLISION_HASH:
        raise ValueError("frozen collision identity changed")
    return tables


def u_powers() -> tuple[list[int], list[list[int]]]:
    """U and the maps U^0..U^11 on one polarity; U^12 must be the identity."""
    U = [C.U(c) for c in range(N)]
    powers = [list(range(N))]
    for _ in range(11):
        powers.append([U[i] for i in powers[-1]])
    if [U[i] for i in powers[11]] != list(range(N)):
        raise ValueError("U does not have period 12 on one polarity")
    return U, powers


def covariance_violations() -> int:
    """Rows of the exactly-two sector where U C_q differs from C_{q-1} U."""
    tables = _tables()
    U, _ = u_powers()
    violations = 0
    for q in range(3):
        for (a, b), (c, d) in tables[q].items():
            lhs = tuple(sorted((U[c], U[d])))
            rhs = tuple(sorted(tables[(q - 1) % 3][tuple(sorted((U[a], U[b])))]))
            violations += lhs != rhs
    return violations


def _collision_rows(table, relabel):
    """Sparse constraint rows sum_before w(relabel c) = sum_after w(relabel c)."""
    for before, after in table.items():
        row = {}
        for c in before:
            row[relabel[c]] = row.get(relabel[c], 0) - 1
        for c in after:
            row[relabel[c]] = row.get(relabel[c], 0) + 1
        if any(row.values()):
            yield row


def _gram_kernel(rows) -> tuple[tuple[int, ...], ...]:
    """Primitive integer basis of the common kernel of sparse integer rows.

    The Gram matrix A^T A has the same rational kernel as A, so the 192x192
    integer Gram matrix replaces a matrix with hundreds of thousands of rows.
    """
    gram = np.zeros((N, N), dtype=np.int64)
    for row in rows:
        items = [(i, v) for i, v in row.items() if v]
        for i, a in items:
            for j, b in items:
                gram[i, j] += a * b
    basis, nullity = flint.fmpz_mat(gram.tolist()).nullspace()
    out = []
    for j in range(nullity):
        column = [int(basis[i, j]) for i in range(N)]
        g = 0
        for value in column:
            g = gcd(g, abs(value))
        column = [value // g for value in column]
        if next(value for value in column if value) < 0:
            column = [-value for value in column]
        out.append(tuple(column))
    return tuple(sorted(out))


def fixed_kernel() -> tuple[tuple[int, ...], ...]:
    """Weights conserved by every collision layer and by streaming."""
    tables = _tables()
    U, _ = u_powers()
    identity = list(range(N))
    rows = [row for table in tables for row in _collision_rows(table, identity)]
    rows.extend({c: -1, U[c]: 1} for c in range(N) if U[c] != c)
    return _gram_kernel(rows)


def locked_kernel(l0: int) -> tuple[tuple[int, ...], ...]:
    """Weights w_0 such that w_0 o U^{-t} is conserved by layer (l0-t) mod 3, t=0..11."""
    if l0 not in (0, 1, 2):
        raise ValueError("starting layer must be 0, 1 or 2")
    tables = _tables()
    _, powers = u_powers()
    rows = []
    for t in range(12):
        inverse = [0] * N
        for i, j in enumerate(powers[t]):
            inverse[j] = i  # inverse[c] = U^{-t}(c)
        rows.extend(_collision_rows(tables[(l0 - t) % 3], inverse))
    return _gram_kernel(rows)


def span_membership(basis, vectors) -> bool:
    rows = [list(map(int, row)) for row in basis]
    base_rank = flint.fmpz_mat(rows).rank()
    return flint.fmpz_mat(rows + [list(map(int, v)) for v in vectors]).rank() == base_rank


def tangent_rows() -> tuple[tuple[int, ...], ...]:
    return tuple(tuple(C.tangent(c)[axis] for c in range(N)) for axis in range(3))


def layer_rows(layer: int) -> tuple[tuple[int, ...], ...]:
    return tuple(tuple(C.layer_value_of(c, layer)[k] for c in range(N)) for k in range(6))


@dataclass(frozen=True)
class Isotropy:
    T_xxxx: int
    T_xxyy: int
    T_xx: int
    T_xy: int

    @property
    def isotropic4(self) -> bool:
        return self.T_xxxx == 3 * self.T_xxyy

    @property
    def isotropic2(self) -> bool:
        return self.T_xy == 0


def fourth_rank_isotropy(velocities) -> Isotropy:
    """velocities: iterable of ((x, y, z), multiplicity) with integer entries."""
    xxxx = xxyy = xx = xy = 0
    for (x, y, z), m in velocities:
        xxxx += m * x ** 4
        xxyy += m * x * x * y * y
        xx += m * x * x
        xy += m * x * y
    return Isotropy(xxxx, xxyy, xx, xy)


_ALL = tuple(product((-1, 0, 1), repeat=3))
FACE = tuple(v for v in _ALL if sum(map(abs, v)) == 1)
EDGE = tuple(v for v in _ALL if sum(map(abs, v)) == 2)
CORNER = tuple(v for v in _ALL if sum(map(abs, v)) == 3)
VELOCITY_SETS = {
    "body_diagonal_8": tuple((v, 1) for v in CORNER),
    "face_edge_18": tuple((v, 1) for v in FACE + EDGE),
    "moore_26": tuple((v, 1) for v in FACE + EDGE + CORNER),
    "fchc_projected_18": tuple((v, 2) for v in FACE) + tuple((v, 1) for v in EDGE),
}


def census() -> dict:
    fixed = fixed_kernel()
    locked = {str(l0): locked_kernel(l0) for l0 in range(3)}
    return {
        "law_id": "phi-v2-staged-candidate-1",
        "collision_sha256": C.COLLISION_HASH,
        "covariance_violations": covariance_violations(),
        "fixed_dimension_per_polarity": len(fixed),
        "fixed_basis": fixed,
        "locked_dimension_per_polarity": {k: len(v) for k, v in locked.items()},
        "locked_basis": locked,
        "tangent_in_locked_span": span_membership(locked["0"], tangent_rows()),
        "tangent_in_fixed_span": span_membership(fixed, tangent_rows()),
        "isotropy": {name: {"T_xxxx": r.T_xxxx, "T_xxyy": r.T_xxyy, "T_xx": r.T_xx,
                            "T_xy": r.T_xy, "isotropic4": r.isotropic4, "isotropic2": r.isotropic2}
                     for name, r in ((n, fourth_rank_isotropy(v)) for n, v in VELOCITY_SETS.items())},
        "scope": "field sector on the homogeneous doubly occupied background; linear additive "
                 "invariants with fixed or U-precessing weights; nonlinear and mixed-record "
                 "observables not classified",
    }
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `python -m pytest scripts/tests/phi_v2_lattice/test_recovery_hydro_invariants.py -q`
Expected: `11 passed` in under two minutes (each `locked_kernel` call is a few seconds with the Gram formulation).

- [ ] **Step 5: Commit**

```bash
git add scripts/phi_v2_lattice/recovery_hydro_invariants.py scripts/tests/phi_v2_lattice/test_recovery_hydro_invariants.py
git diff --cached --stat
git commit -m "feat(strict): gate H0 exact invariant census — fixed kernel 1, locked kernel 7, covariance exact, isotropy table"
```

---

### Task 2: Gate H1 — dispersion operators (exact and certified tracks)

**Files:**
- Create: `scripts/phi_v2_lattice/recovery_hydro_dispersion.py`
- Test: `scripts/tests/phi_v2_lattice/test_recovery_hydro_dispersion.py`

**Interfaces:**
- Consumes: `recovery_kinetic_response.collision_operator(layer).integer_correction` (192×192 nested int tuples), `channels.U`, `channels.tangent`, `recovery_hydro_invariants.locked_kernel(l0)`.
- Produces: `PROXY_R`, `DIRECTIONS`, `REFERENCE_PS`, `physical_r(p)`, arithmetic classes `Exact` and `Certified` with `scalar/zeros/identity/from_rows`, `period_series(A, direction, r, l0=0) -> (P0, A1, A2)`, `exact_bases(P0, r, l0=0) -> Bases(W, V)`, `certified_bases(P0, bases_at_proxy) -> Bases`, `reduced_resolvent(A, P0, W, V)`, `effective_operators(series, W, V, resolvent) -> (M1, M2)`, `exact_dispersion(direction, r=PROXY_R) -> Dispersion`, `certified_dispersion(direction, p, proxy_bases, prec=256) -> Dispersion`, `numeric_period_map(k, p) -> np.ndarray`, `Dispersion` dataclass with fields `track, direction, r, p, M1, M2, W, V, rank_complement`.

- [ ] **Step 1: Write the failing tests**

```python
# scripts/tests/phi_v2_lattice/test_recovery_hydro_dispersion.py
"""Exact operator identities for gate H1. Verdict quantities are not classified here."""
from fractions import Fraction

import flint
import numpy as np
import pytest

from phi_v2_lattice import recovery_hydro_dispersion as D
from phi_v2_lattice import recovery_hydro_invariants as H
from phi_v2_lattice import recovery_kinetic_response as R
from phi_v2_lattice import channels as C

N = 192


@pytest.fixture(scope="module")
def exact100():
    return D.exact_dispersion((1, 0, 0))


def test_physical_r_matches_response_module_convention():
    p = Fraction(1, 96)
    assert D.physical_r(p) == p * (1 - p) ** 189 == R.REFERENCE_P * (1 - R.REFERENCE_P) ** 189


def test_proxy_offset_is_declared_and_small():
    r = D.physical_r(Fraction(1, 96))
    assert abs((D.PROXY_R - r) / r) < Fraction(1, 1000)


def test_complement_rank_is_185_at_proxy(exact100):
    assert exact100.rank_complement == 185


def test_locked_kernel_is_left_null_space_and_biorthogonal(exact100):
    P0 = D.period_series(D.Exact, (1, 0, 0), D.PROXY_R)[0]
    I = D.Exact.identity(N)
    assert exact100.W * (I - P0) == D.Exact.zeros(7, N)
    assert exact100.W * exact100.V == D.Exact.identity(7)
    assert (I - P0) * exact100.V == D.Exact.zeros(N, 7)


def test_reduced_resolvent_identities():
    series = D.period_series(D.Exact, (1, 0, 0), D.PROXY_R)
    bases = D.exact_bases(series[0], D.PROXY_R)
    resolvent = D.reduced_resolvent(D.Exact, series[0], bases.W, bases.V)
    I = D.Exact.identity(N)
    Q = I - bases.V * bases.W
    assert (I - series[0]) * resolvent == Q
    assert bases.W * resolvent == D.Exact.zeros(7, N)
    assert resolvent * bases.V == D.Exact.zeros(N, 7)


def test_parity_of_expansion_coefficients():
    plus = D.period_series(D.Exact, (1, 0, 0), D.PROXY_R)
    minus = D.period_series(D.Exact, (-1, 0, 0), D.PROXY_R)
    assert plus[0] == minus[0]
    assert plus[1] == -minus[1]
    assert plus[2] == minus[2]


def test_first_order_coefficient_equals_single_insertion_sum():
    """Independent construction: A1 = sum_t (prod_{s>t} S0 J_s)(PU D J_t)(prod_{s<t} S0 J_s)."""
    A = D.Exact
    PU = D.streaming_permutation(A)
    Dm, _ = D.direction_matrices(A, (1, 1, 0))
    J = [D.collision_jacobian(A, q, D.PROXY_R) for q in range(3)]
    stages = [PU * J[(0 - t) % 3] for t in range(12)]
    total = A.zeros(N, N)
    for t in range(12):
        left = A.identity(N)
        for s in range(t + 1, 12):
            left = stages[s] * left
        right = A.identity(N)
        for s in range(t):
            right = stages[s] * right
        total += left * (PU * Dm * J[(0 - t) % 3]) * right
    assert D.period_series(A, (1, 1, 0), D.PROXY_R)[1] == total


def test_numeric_map_at_zero_wavevector_matches_exact_p0():
    exact = D.period_series(D.Exact, (1, 0, 0), D.PROXY_R)[0]
    numeric = D.numeric_period_map((0.0, 0.0, 0.0), None, r=float(D.PROXY_R))
    dense = np.array([[float(exact[i, j].p) / float(exact[i, j].q) for j in range(N)] for i in range(N)])
    assert np.max(np.abs(numeric - dense)) < 1e-12


def test_certified_track_encloses_exact_track_at_proxy(exact100):
    certified = D.certified_dispersion((1, 0, 0), None, exact100, r=D.PROXY_R)
    for name in ("M1", "M2"):
        ball, exact = getattr(certified, name), getattr(exact100, name)
        for i in range(7):
            for j in range(7):
                assert D.encloses(ball[i, j], exact[i, j]), (name, i, j)
                assert ball[i, j].rad() < flint.arb("1e-40")


def test_certified_physical_bases_are_consistent(exact100):
    certified = D.certified_dispersion((1, 0, 0), Fraction(1, 96), exact100)
    residual = D.certified_residuals(certified)
    assert residual["max_left_null_radius"] < 1e-40
    assert residual["max_biorthogonality_error"] < 1e-40
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `python -m pytest scripts/tests/phi_v2_lattice/test_recovery_hydro_dispersion.py -q`
Expected: collection error, module missing.

- [ ] **Step 3: Write the module**

```python
# scripts/phi_v2_lattice/recovery_hydro_dispersion.py
"""Small-wavevector dispersion of the 12-stage linearized Boltzmann propagator (gate H1).

With the formal variable eps = -i*kappa and wavevector k = kappa*direction, the
streaming phase e^{-i k.d} = e^{eps (direction.d)} expands with real rational
coefficients, so P(eps) = P0 + eps*A1 + eps^2*A2 has real entries. Eigenvalues
read lambda = 1 - i*kappa*mu1 - kappa^2*mu2 and the per-period damping is
mu2 - mu1^2/2. Two tracks: exact fmpq at the declared proxy coupling PROXY_R,
certified arb enclosures at the physical coupling r(p). The contract is
engine/docs/DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction

import flint
import numpy as np

from . import channels as C
from . import recovery_hydro_invariants as H
from . import recovery_kinetic_response as R

N = 192
PROXY_R = Fraction(1, 695)
DIRECTIONS = ((1, 0, 0), (1, 1, 0), (1, 1, 1))
REFERENCE_PS = (Fraction(1, 96), Fraction(1, 192), Fraction(1, 48))
STAGES = 12  # one period = 12 stages = 48 physical microticks


def physical_r(p) -> Fraction:
    p = Fraction(p)
    if not 0 < p < 1:
        raise ValueError("interior reference density required")
    return p * (1 - p) ** 189


class Exact:
    """python-flint fmpq_mat arithmetic (proxy coupling)."""
    name = "exact_fmpq"

    @staticmethod
    def scalar(x):
        x = Fraction(x)
        return flint.fmpq(x.numerator, x.denominator)

    @staticmethod
    def zeros(n, m):
        return flint.fmpq_mat(n, m)

    @staticmethod
    def identity(n):
        I = flint.fmpq_mat(n, n)
        for i in range(n):
            I[i, i] = 1
        return I

    @staticmethod
    def from_rows(rows):
        return flint.fmpq_mat([[Exact.scalar(v) for v in row] for row in rows])


class Certified:
    """python-flint arb_mat ball arithmetic (physical coupling); precision is ctx.prec."""
    name = "certified_arb"

    @staticmethod
    def scalar(x):
        x = Fraction(x)
        return flint.arb(x.numerator) / flint.arb(x.denominator)

    @staticmethod
    def zeros(n, m):
        return flint.arb_mat(n, m)

    @staticmethod
    def identity(n):
        I = flint.arb_mat(n, n)
        for i in range(n):
            I[i, i] = 1
        return I

    @staticmethod
    def from_rows(rows):
        return flint.arb_mat([[Certified.scalar(v) for v in row] for row in rows])


def encloses(ball, exact) -> bool:
    """True when the arb ball contains the exact rational."""
    delta = ball - Certified.scalar(Fraction(int(exact.p), int(exact.q)))
    return abs(delta.mid()) <= delta.rad()


def streaming_permutation(A):
    P = A.zeros(N, N)
    for c in range(N):
        P[C.U(c), c] = 1
    return P


def collision_jacobian(A, layer, r):
    """I + r*K_layer: the first-degree marginal Jacobian of the exactly-two collision."""
    K = R.collision_operator(layer).integer_correction
    rs = A.scalar(r)
    J = A.zeros(N, N)
    for i in range(N):
        row = K[i]
        for j in range(N):
            if row[j]:
                J[i, j] = rs * row[j]
        J[i, i] += 1
    return J


def direction_matrices(A, direction):
    """diag(direction . d(c)) and diag((direction . d(c))^2 / 2)."""
    D1 = A.zeros(N, N)
    D2 = A.zeros(N, N)
    half = A.scalar(Fraction(1, 2))
    for c in range(N):
        value = sum(n * d for n, d in zip(direction, C.tangent(c)))
        D1[c, c] = value
        D2[c, c] = half * (value * value)
    return D1, D2


def _series_mul(X, Y):
    return (X[0] * Y[0], X[0] * Y[1] + X[1] * Y[0], X[0] * Y[2] + X[1] * Y[1] + X[2] * Y[0])


def period_series(A, direction, r, l0=0):
    """(P0, A1, A2): P(eps) = P0 + eps*A1 + eps^2*A2 over one 12-stage period.

    Stage t applies collision layer (l0 - t) mod 3 then streams: M_t = S(eps) J_t,
    S(eps) = P_U (I + eps D1 + eps^2 D2). The period map is M_11 ... M_0.
    """
    PU = streaming_permutation(A)
    D1, D2 = direction_matrices(A, direction)
    S = (PU, PU * D1, PU * D2)
    J = [collision_jacobian(A, q, r) for q in range(3)]
    acc = (A.identity(N), A.zeros(N, N), A.zeros(N, N))
    for t in range(STAGES):
        Jt = J[(l0 - t) % 3]
        acc = _series_mul((S[0] * Jt, S[1] * Jt, S[2] * Jt), acc)
    return acc


@dataclass(frozen=True)
class Bases:
    W: object  # 7 x N conserved weights (rows), stage 0
    V: object  # N x 7 equilibrium modes with W V = I


def exact_bases(P0, r, l0=0) -> Bases:
    """W from the locked kernel; V as the exact right null space normalized to W V = I."""
    W = Exact.from_rows(H.locked_kernel(l0))
    A = Exact.identity(N) - P0
    if W * A != Exact.zeros(7, N):
        raise ValueError("locked kernel is not the left null space of I - P0")
    scale = Exact.scalar(Fraction(r).denominator ** STAGES)
    B = flint.fmpz_mat(N, N)
    for i in range(N):
        for j in range(N):
            q = A[i, j] * scale
            if q.q != 1:
                raise ValueError("entry denominator does not divide den(r)^12")
            B[i, j] = int(q.p)
    basis, nullity = B.nullspace()
    if nullity != 7:
        raise ValueError(f"right nullity {nullity} != 7: undiscovered invariant or defect")
    V = Exact.zeros(N, 7)
    for j in range(7):
        for i in range(N):
            V[i, j] = int(basis[i, j])
    return Bases(W, V * (W * V).inv())


def certified_bases(P0, proxy: Bases) -> Bases:
    """V(r) = T^{-1} V0 with T = (I - P0) + V0 W; solve certifies invertibility."""
    W = Certified.from_rows([[int(proxy.W[i, j].p) for j in range(N)] for i in range(7)])
    V0 = flint.arb_mat([[Certified.scalar(Fraction(int(proxy.V[i, j].p), int(proxy.V[i, j].q)))
                         for j in range(7)] for i in range(N)])
    T = (Certified.identity(N) - P0) + V0 * W
    return Bases(W, T.solve(V0))


def reduced_resolvent(A, P0, W, V):
    """R with (I - P0) R = I - V W, W R = 0, R V = 0."""
    I = A.identity(N)
    VW = V * W
    return ((I - P0) + VW).inv() * (I - VW)


def effective_operators(series, W, V, resolvent):
    P0, A1, A2 = series
    M1 = W * A1 * V
    M2 = W * (A2 + A1 * resolvent * A1) * V
    return M1, M2


@dataclass(frozen=True)
class Dispersion:
    track: str
    direction: tuple
    r: Fraction | None
    p: Fraction | None
    M1: object
    M2: object
    W: object
    V: object
    rank_complement: int | None
    P0: object


def exact_dispersion(direction, r=PROXY_R, l0=0) -> Dispersion:
    series = period_series(Exact, direction, r, l0)
    bases = exact_bases(series[0], r, l0)
    rank = (Exact.identity(N) - series[0]).rank()
    resolvent = reduced_resolvent(Exact, series[0], bases.W, bases.V)
    M1, M2 = effective_operators(series, bases.W, bases.V, resolvent)
    return Dispersion(Exact.name, tuple(direction), Fraction(r), None, M1, M2,
                      bases.W, bases.V, rank, series[0])


def certified_dispersion(direction, p, proxy: Dispersion, r=None, l0=0, prec=256) -> Dispersion:
    """Certified enclosures at physical r(p) (or an explicit r for cross-checks)."""
    saved = flint.ctx.prec
    flint.ctx.prec = prec
    try:
        r = physical_r(p) if r is None else Fraction(r)
        series = period_series(Certified, direction, r, l0)
        bases = certified_bases(series[0], Bases(proxy.W, proxy.V))
        resolvent = reduced_resolvent(Certified, series[0], bases.W, bases.V)
        M1, M2 = effective_operators(series, bases.W, bases.V, resolvent)
        return Dispersion(Certified.name, tuple(direction), r, None if p is None else Fraction(p),
                          M1, M2, bases.W, bases.V, None, series[0])
    finally:
        flint.ctx.prec = saved


def certified_residuals(d: Dispersion) -> dict:
    """Largest ball radii of the defining identities; a rigorous consistency report."""
    I = Certified.identity(N)
    left = d.W * (I - d.P0)
    bio = d.W * d.V - Certified.identity(7)
    right = (I - d.P0) * d.V
    def worst(M, n, m):
        return max(float(abs(M[i, j].mid()) + M[i, j].rad()) for i in range(n) for j in range(m))
    return {"max_left_null_radius": worst(left, 7, N),
            "max_biorthogonality_error": worst(bio, 7, 7),
            "max_right_null_error": worst(right, N, 7)}


def numeric_period_map(k, p, r=None, l0=0) -> np.ndarray:
    """Full complex period map at a finite wavevector k (radians per site), float64.

    Prediction-curve use only. S_k = P_U diag(exp(-i k.d)); J_q = I + r K_q.
    """
    r = float(physical_r(p)) if r is None else float(r)
    d = np.array([C.tangent(c) for c in range(N)], dtype=float)
    phase = np.exp(-1j * d @ np.asarray(k, dtype=float))
    PU = np.zeros((N, N))
    PU[[C.U(c) for c in range(N)], range(N)] = 1.0
    S = PU * phase[None, :]
    J = [np.eye(N) + r * np.array(R.collision_operator(q).integer_correction, dtype=float)
         for q in range(3)]
    P = np.eye(N, dtype=complex)
    for t in range(STAGES):
        P = (S @ J[(l0 - t) % 3]) @ P
    return P
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `python -m pytest scripts/tests/phi_v2_lattice/test_recovery_hydro_dispersion.py -q`
Expected: `10 passed`. Exact operations take seconds; the certified 192×192 solves at 256 bits take under a minute each. If `test_complement_rank_is_185_at_proxy` fails, stop and report: H0 has missed an invariant or the eigenvalue 1 is defective.

- [ ] **Step 5: Commit**

```bash
git add scripts/phi_v2_lattice/recovery_hydro_dispersion.py scripts/tests/phi_v2_lattice/test_recovery_hydro_dispersion.py
git diff --cached --stat
git commit -m "feat(strict): gate H1 dispersion operators — exact fmpq track at proxy r0, certified arb track at physical r, float64 map for prediction"
```

---

### Task 3: Gate H1 — verdict clauses

**Files:**
- Create: `scripts/phi_v2_lattice/recovery_hydro_verdict.py`
- Test: `scripts/tests/phi_v2_lattice/test_recovery_hydro_verdict.py`

**Interfaces:**
- Consumes: `Dispersion` objects (fields `M1`, `M2`, `W`, `direction`) from Task 2; `Exact`, `Certified`.
- Produces: `density_functional(W) -> fmpq_mat(1, 7)`, `closure_block(start_rows, operators) -> fmpq_mat(b, 7)`, `restrict(block, M) -> fmpq_mat(b, b)`, `exact_verdict(dispersions: dict[direction, Dispersion]) -> dict`, `certified_verdict(dispersions, block_rows) -> dict`, `VERDICT_LABELS`.

**Definitions used below (all exact).** For a row basis `B` of the block, `X = restrict(B, M1)` and `Y = restrict(B, M2)`. Direction norm `n2 = |n̂|²`. The sound-pair clause requires `charpoly(X) = x^4 - s x^2` with `s > 0` and `X^3 = s X`. Then `Π_L = X²/s`, `Π_T = I − Π_L`. Transverse second-order operator is `Π_T Y Π_T`; its characteristic polynomial divided by `x²` is the 2-dimensional transverse polynomial `x² + c1 x + c0`; normalized coefficients are `(c1/n2, c0/n2²)`. Longitudinal damping is `γ_L = tr(Π_L Y Π_L)/2 − s/2`, normalized `γ_L/n2`. Sound speed squared normalized `s/n2`.

- [ ] **Step 1: Write the failing tests**

```python
# scripts/tests/phi_v2_lattice/test_recovery_hydro_verdict.py
"""Verdict clause logic on synthetic 7x7 operators; the real operators are classified only by the evidence runner."""
from fractions import Fraction

import flint
import pytest

from phi_v2_lattice import recovery_hydro_dispersion as D
from phi_v2_lattice import recovery_hydro_verdict as V


def _mat(rows):
    return D.Exact.from_rows(rows)


def _w_with_constant_first():
    """A synthetic W whose first row is the constant weight; only its row space matters here."""
    rows = [[1] * 192]
    for k in range(1, 7):
        rows.append([1 if (c % 7) == k else 0 for c in range(192)])
    return _mat(rows)


class Synthetic:
    """Dispersion-like objects with direct 7x7 operators."""
    def __init__(self, direction, M1, M2, W):
        self.direction, self.M1, self.M2, self.W = direction, M1, M2, W


def _isotropic_case(direction, cs2=Fraction(1, 3), nu=Fraction(1, 6), zeta=Fraction(1, 10)):
    """Density + 3 momenta form an NS block; three extra modes decay independently."""
    n = direction
    n2 = sum(x * x for x in n)
    M1 = [[Fraction(0)] * 7 for _ in range(7)]
    # rows act on functionals: a M1 gives the flux functional of a
    for a in range(3):
        M1[0][1 + a] = Fraction(n[a])          # d rho -> n . j
        M1[1 + a][0] = cs2 * n[a]              # d j_a -> cs2 n_a rho
    M2 = [[Fraction(0)] * 7 for _ in range(7)]
    for a in range(3):
        for b in range(3):
            M2[1 + a][1 + b] = nu * n2 * (a == b) + (zeta + nu / 3) * n[a] * n[b]
    for k in range(4, 7):
        M2[k][k] = Fraction(k) * n2
    return _mat(M1), _mat(M2)


def _anisotropic_case(direction):
    M1, M2 = _isotropic_case(direction)
    n = direction
    # cubic term: extra transverse damping proportional to sum n_a^4 breaks isotropy
    rows = [[M2[i, j] for j in range(7)] for i in range(7)]
    for a in range(3):
        rows[1 + a][1 + a] += Fraction(1, 5) * n[a] ** 4
    return M1, _mat(rows)


def test_density_functional_reproduces_constant_weight():
    W = _w_with_constant_first()
    a = V.density_functional(W)
    assert a * W == _mat([[1] * 192])


def test_closure_block_of_isotropic_case_is_four_dimensional():
    W = _w_with_constant_first()
    ops = []
    for n in D.DIRECTIONS:
        M1, M2 = _isotropic_case(n)
        ops += [M1, M2]
    block = V.closure_block(V.density_functional(W), ops)
    assert block.nrows() == 4


def test_isotropic_synthetic_is_ns_class():
    W = _w_with_constant_first()
    disp = {n: Synthetic(n, *_isotropic_case(n), W) for n in D.DIRECTIONS}
    verdict = V.exact_verdict(disp)
    assert verdict["label"] == "NS-class isotropic"
    assert verdict["sound_speed_squared_normalized"] == {str(n): "1/3" for n in D.DIRECTIONS}
    assert verdict["transverse_polynomial_normalized"]["(1, 0, 0)"] == ["-1/3", "1/36"]


def test_anisotropic_synthetic_is_flagged():
    W = _w_with_constant_first()
    disp = {n: Synthetic(n, *_anisotropic_case(n), W) for n in D.DIRECTIONS}
    verdict = V.exact_verdict(disp)
    assert verdict["label"] == "anisotropic momentum hydrodynamics"
    assert verdict["clauses"]["block_dimension_is_four"] is True
    assert verdict["clauses"]["transverse_isotropic"] is False


def test_diffusive_only_synthetic():
    W = _w_with_constant_first()
    disp = {}
    for n in D.DIRECTIONS:
        _, M2 = _isotropic_case(n)
        disp[n] = Synthetic(n, D.Exact.zeros(7, 7), M2, W)
    verdict = V.exact_verdict(disp)
    assert verdict["label"] == "diffusive only"


def test_certified_verdict_marks_proved_consistent_or_undecided():
    W = _w_with_constant_first()
    exact = {n: Synthetic(n, *_anisotropic_case(n), W) for n in D.DIRECTIONS}
    block = V.closure_block(V.density_functional(W), [m for n in D.DIRECTIONS for m in (exact[n].M1, exact[n].M2)])
    flint.ctx.prec = 256
    balls = {}
    for n in D.DIRECTIONS:
        M1, M2 = _anisotropic_case(n)
        balls[n] = Synthetic(n, flint.arb_mat([[D.Certified.scalar(Fraction(int(M1[i, j].p), int(M1[i, j].q))) for j in range(7)] for i in range(7)]),
                             flint.arb_mat([[D.Certified.scalar(Fraction(int(M2[i, j].p), int(M2[i, j].q))) for j in range(7)] for i in range(7)]), W)
    report = V.certified_verdict(balls, block)
    assert report["transverse_isotropic"] == "REFUTED"
    assert report["sound_speed_direction_independent"] == "CONSISTENT"
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `python -m pytest scripts/tests/phi_v2_lattice/test_recovery_hydro_verdict.py -q`
Expected: collection error, module missing.

- [ ] **Step 3: Write the module**

```python
# scripts/phi_v2_lattice/recovery_hydro_verdict.py
"""Pre-registered verdict clauses for gate H1 (spec section 3.2, fixed before any result).

Operators act on moment functionals from the right: a row a (a functional on the
7-space) evolves as a -> a (I + eps M1 + eps^2 M2). The density functional is the
constant weight expressed in the W basis. The closure block is the smallest row
space containing it and invariant under every listed operator.
"""
from __future__ import annotations

from fractions import Fraction

import flint

from . import recovery_hydro_dispersion as D

N = 192
VERDICT_LABELS = ("NS-class isotropic", "anisotropic momentum hydrodynamics", "diffusive only", "other")


def _fraction(q) -> Fraction:
    return Fraction(int(q.p), int(q.q))


def _text(q) -> str:
    return str(_fraction(q))


def density_functional(W):
    """Row a with a W = (1,...,1); raises if the constant weight is outside the row space."""
    ones = D.Exact.from_rows([[1] * N])
    gram = W * W.transpose()
    a = (ones * W.transpose()) * gram.inv()
    if a * W != ones:
        raise ValueError("constant weight is not in the span of W")
    return a


def _rank(rows_mat) -> int:
    return rows_mat.rank()


def _stack(top, extra_row):
    rows = [[top[i, j] for j in range(top.ncols())] for i in range(top.nrows())]
    rows.append([extra_row[0, j] for j in range(extra_row.ncols())])
    return flint.fmpq_mat(rows)


def closure_block(start, operators):
    """Smallest row space containing `start` and closed under right-multiplication."""
    block = start
    changed = True
    while changed:
        changed = False
        for M in operators:
            for i in range(block.nrows()):
                row = flint.fmpq_mat([[block[i, j] for j in range(block.ncols())]]) * M
                candidate = _stack(block, row)
                if candidate.rank() > block.rank():
                    block = candidate
                    changed = True
    return block


def restrict(block, M):
    """X with block M = X block; raises if the block is not invariant."""
    Bt = block.transpose()
    X = (block * M * Bt) * (block * Bt).inv()
    if X * block != block * M:
        raise ValueError("block is not invariant under the operator")
    return X


def _charpoly_coeffs(X):
    return [c for c in X.charpoly().coeffs()]  # low to high


def _direction_norm2(direction) -> int:
    return sum(int(x) * int(x) for x in direction)


def _sound_structure(X):
    """(s, ok): charpoly x^4 - s x^2 with s > 0 and X^3 = s X."""
    coeffs = _charpoly_coeffs(X)
    if len(coeffs) != 5:
        return None, False
    c0, c1, c2, c3, c4 = coeffs
    s = -c2
    ok = (c0 == 0 and c1 == 0 and c3 == 0 and c4 == 1 and s > 0)
    if ok:
        ok = (X * X * X == X * s)
    return s, ok


def _transverse_polynomial(X, Y, s):
    I4 = D.Exact.identity(4)
    PL = (X * X) * flint.fmpq(int(s.q), int(s.p))   # X^2 / s, exact
    PT = I4 - PL
    T = PT * Y * PT
    coeffs = _charpoly_coeffs(T)  # degree 4 with two zero roots from PL's kernel
    # divide by x^2 exactly: coefficients of x^0 and x^1 must vanish
    if coeffs[0] != 0 or coeffs[1] != 0:
        raise ValueError("transverse operator does not annihilate the longitudinal pair")
    return coeffs[2], coeffs[3], coeffs[4]  # c0', c1', leading (=1)


def _longitudinal_damping(X, Y, s):
    PL = (X * X) * flint.fmpq(int(s.q), int(s.p))
    trace = sum((PL * Y * PL)[i, i] for i in range(4))
    return trace / 2 - s / 2


def exact_verdict(dispersions: dict) -> dict:
    """Apply the fixed clauses to exact 7x7 operators keyed by direction."""
    directions = tuple(dispersions)
    W = dispersions[directions[0]].W
    a = density_functional(W)
    operators = [m for n in directions for m in (dispersions[n].M1, dispersions[n].M2)]
    block = closure_block(a, operators)
    dim = block.nrows()
    out = {"block_dimension": dim, "block_rows": [[_text(block[i, j]) for j in range(7)] for i in range(dim)],
           "clauses": {"block_dimension_is_four": dim == 4},
           "charpoly_M1": {str(n): [_text(c) for c in _charpoly_coeffs(dispersions[n].M1)] for n in directions},
           "charpoly_M2": {str(n): [_text(c) for c in _charpoly_coeffs(dispersions[n].M2)] for n in directions}}
    all_first_order_zero = all(dispersions[n].M1 == D.Exact.zeros(7, 7) for n in directions)
    if all_first_order_zero:
        out["label"] = "diffusive only"
        out["density_diffusion_charpoly_on_block"] = {
            str(n): [_text(c) for c in _charpoly_coeffs(restrict(block, dispersions[n].M2))] for n in directions}
        return out
    if dim != 4:
        out["label"] = "other"
        return out
    s_norm, sound_ok, trans, longi = {}, True, {}, {}
    for n in directions:
        X = restrict(block, dispersions[n].M1)
        Y = restrict(block, dispersions[n].M2)
        s, ok = _sound_structure(X)
        sound_ok = sound_ok and ok
        if not ok:
            continue
        n2 = _direction_norm2(n)
        s_norm[str(n)] = _text(s * flint.fmpq(1, n2))
        c0, c1, _ = _transverse_polynomial(X, Y, s)
        trans[str(n)] = [_text(c1 * flint.fmpq(1, n2)), _text(c0 * flint.fmpq(1, n2 * n2))]
        longi[str(n)] = _text(_longitudinal_damping(X, Y, s) * flint.fmpq(1, n2))
    out["clauses"]["sound_pair_all_directions"] = sound_ok
    out["sound_speed_squared_normalized"] = s_norm
    out["transverse_polynomial_normalized"] = trans
    out["longitudinal_damping_normalized"] = longi
    same_speed = sound_ok and len(set(s_norm.values())) == 1
    same_trans = sound_ok and len({tuple(v) for v in trans.values()}) == 1
    same_long = sound_ok and len(set(longi.values())) == 1
    out["clauses"].update({"sound_speed_direction_independent": same_speed,
                           "transverse_isotropic": same_trans,
                           "longitudinal_isotropic": same_long})
    if sound_ok and same_speed and same_trans and same_long:
        out["label"] = "NS-class isotropic"
    elif sound_ok:
        out["label"] = "anisotropic momentum hydrodynamics"
    else:
        out["label"] = "other"
    return out


def _status(delta_ball) -> str:
    """Status of an EQUALITY clause from the enclosure of a difference.

    REFUTED: the enclosure excludes zero, so the inequality is rigorously proved
    (this is how anisotropy is PROVED). CONSISTENT: the enclosure contains zero
    and is narrow. UNDECIDED: the enclosure contains zero but is too wide.
    """
    if abs(delta_ball.mid()) > delta_ball.rad():
        return "REFUTED"
    return "CONSISTENT" if delta_ball.rad() < flint.arb("1e-30") else "UNDECIDED"


def certified_verdict(dispersions: dict, block) -> dict:
    """Ball-arithmetic evaluation of the clauses on the exact block basis."""
    directions = tuple(dispersions)
    Bq = block
    B = flint.arb_mat([[D.Certified.scalar(_fraction(Bq[i, j])) for j in range(7)] for i in range(Bq.nrows())])
    Gi = flint.arb_mat([[D.Certified.scalar(_fraction((Bq * Bq.transpose()).inv()[i, j])) for j in range(Bq.nrows())]
                        for i in range(Bq.nrows())])
    report = {}
    s_values, trans_values, long_values, invariance = {}, {}, {}, {}
    for n in directions:
        M1, M2 = dispersions[n].M1, dispersions[n].M2
        X = (B * M1 * B.transpose()) * Gi
        Y = (B * M2 * B.transpose()) * Gi
        res = X * B - B * M1
        invariance[str(n)] = max(float(abs(res[i, j].mid()) + res[i, j].rad())
                                 for i in range(res.nrows()) for j in range(res.ncols()))
        coeffs = X.charpoly().coeffs()
        s = -coeffs[2]
        n2 = _direction_norm2(n)
        s_values[n] = s / n2
        I4 = flint.arb_mat(4, 4)
        for i in range(4):
            I4[i, i] = 1
        PL = (X * X) * (flint.arb(1) / s)
        PT = I4 - PL
        tc = (PT * Y * PT).charpoly().coeffs()
        trans_values[n] = (tc[3] / n2, tc[2] / (n2 * n2))
        trace = sum((PL * Y * PL)[i, i] for i in range(4))
        long_values[n] = (trace / 2 - s / 2) / n2
    first = directions[0]
    report["block_invariance_max_residual"] = invariance
    report["sound_speed_direction_independent"] = _worst(
        [_status(s_values[n] - s_values[first]) for n in directions[1:]])
    report["transverse_isotropic"] = _worst(
        [_status(trans_values[n][k] - trans_values[first][k]) for n in directions[1:] for k in range(2)])
    report["longitudinal_isotropic"] = _worst(
        [_status(long_values[n] - long_values[first]) for n in directions[1:]])
    report["values"] = {str(n): {"sound_speed_squared_normalized": str(s_values[n]),
                                 "transverse_c1_normalized": str(trans_values[n][0]),
                                 "transverse_c0_normalized": str(trans_values[n][1]),
                                 "longitudinal_damping_normalized": str(long_values[n])}
                        for n in directions}
    return report


def _worst(statuses) -> str:
    if "REFUTED" in statuses:
        return "REFUTED"
    if "UNDECIDED" in statuses:
        return "UNDECIDED"
    return "CONSISTENT"
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `python -m pytest scripts/tests/phi_v2_lattice/test_recovery_hydro_verdict.py -q`
Expected: `6 passed`. If `test_isotropic_synthetic_is_ns_class` fails on the transverse polynomial value, recheck the synthetic: with `ν = 1/6` the transverse operator on the (1,0,0) block has both eigenvalues `ν n2 = 1/6`, so the normalized polynomial is `x² − (1/3) x + 1/36`.

- [ ] **Step 5: Commit**

```bash
git add scripts/phi_v2_lattice/recovery_hydro_verdict.py scripts/tests/phi_v2_lattice/test_recovery_hydro_verdict.py
git diff --cached --stat
git commit -m "feat(strict): gate H1 verdict clauses — closure block, sound pair, transverse/longitudinal isotropy; exact and certified"
```

---

### Task 4: Gate H1 contract document (before any result is inspected)

**Files:**
- Create: `engine/docs/DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md`

**Interfaces:** none (document). The results section is appended in Task 5 and must not edit anything above the marker line.

- [ ] **Step 1: Write the contract**

```markdown
# Exact small-wavevector dispersion of the staged field sector

Date: 2026-09-07. Law: `phi-v2-staged-candidate-1`. Gate: H1 of
[the hydrodynamics design](../../docs/superpowers/specs/2026-09-07-phi-hydrodynamics-design.md).
Status before computation: **[PREREGISTRATION — EXACT LINEAR-RESPONSE CONTRACT]**.
Nothing above the results marker changes after results exist.

## Objects

Per polarity, channel space Q^192. Reference density p (rational). Linearized
collision at layer q: `J_q = I + r K_q`, `r = p (1-p)^189`, `K_q` the integer
correction of `recovery_kinetic_response.collision_operator(q)`. Streaming at
wavevector `k = kappa * n` for an integer direction n: with the formal variable
`eps = -i kappa`, `S(eps) = P_U (I + eps D1 + eps^2 D2)`, `D1 = diag(n . d(c))`,
`D2 = diag((n . d(c))^2 / 2)`. Stage t applies `J_{(l0 - t) mod 3}` then `S`;
the period map over 12 stages (48 physical microticks) is
`P(eps) = P0 + eps A1 + eps^2 A2` with real rational coefficients.

Conserved weights W (7 rows) are the stage-0 locked kernel of gate H0.
Equilibrium modes V (192 x 7) are the right null vectors of `I - P0` with
`W V = I`. The reduced resolvent R satisfies `(I - P0) R = I - V W`, `W R = 0`,
`R V = 0`. Effective operators: `M1 = W A1 V`, `M2 = W (A2 + A1 R A1) V`.
Eigenvalues near 1 read `lambda = 1 - i kappa mu1 - kappa^2 mu2`; per-period
damping is `mu2 - mu1^2/2`.

## Two arithmetic tracks

Exact track: python-flint `fmpq_mat` at the declared proxy coupling
`r0 = 1/695` (relative offset -0.049 % from `r(1/96)`). Certified track:
python-flint `arb_mat` at 256-bit precision at the exact physical `r(p)` for
p in {1/96, 1/192, 1/48}; every clause is reported PROVED, CONSISTENT, or
UNDECIDED from enclosures. The certified track at `r0` must enclose the exact
track (a test). No float enters any clause.

## Directions and normalization

n in {(1,0,0), (1,1,0), (1,1,1)}, unnormalized. First-order quantities are
divided by |n|^2 where they scale as |k|^2 (sound speed squared), second-order
transverse coefficients by |n|^2 and |n|^4, longitudinal damping by |n|^2.

## Verdict rule (fixed)

Let a be the density functional (constant weight in the W basis) and B the
smallest row space containing a and invariant under every M1(n), M2(n).

1. `dim B = 4`. Its 3-dimensional complement within B is "momentum-like"; no
   other definition is admitted.
2. For every n, `charpoly(M1|_B) = x^4 - s x^2` with s > 0, `(M1|_B)^3 = s M1|_B`,
   and `s/|n|^2` equal across the three directions.
3. Transverse second-order polynomial (charpoly of `Pi_T Y Pi_T` divided by
   x^2, `Pi_T = I - X^2/s`), normalized, equal across directions; longitudinal
   damping `tr(Pi_L Y Pi_L)/2 - s/2`, normalized, equal across directions.

All three hold exactly at r0: **NS-class isotropic**. Clause 1 and 2 hold and 3
fails: **anisotropic momentum hydrodynamics**. `M1 = 0` for every n:
**diffusive only**, with the density diffusion polynomial reported. Anything
else: **other**. The full 7x7 operators and their characteristic polynomials
are reported in every case. No fourth label and no reinterpretation.

## Tag

**[DERIVED — linearized Boltzmann (product) closure at the declared reference;
correlation leakage not bounded here].** The wave-2 full-tangent bounds are
loose; this is a prediction for gate H2, not a theorem about Phi.

<!-- RESULTS MARKER: nothing above this line changes after results exist -->
```

- [ ] **Step 2: Commit the contract before running any evidence**

```bash
git add engine/docs/DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md
git diff --cached --stat
git commit -m "docs(strict): H1 dispersion contract registered before computation"
```

---

### Task 5: Gate H1 evidence run and results

**Files:**
- Create: `scripts/phi_v2_lattice/experiments/run_recovery_wave3.py`
- Create: `engine/docs/evidence/strict-recovery-wave3-exact.json`
- Modify: `engine/docs/DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md` (append below the marker only)

**Interfaces:**
- Consumes: `recovery_hydro_invariants.census()`, `recovery_hydro_dispersion.exact_dispersion/certified_dispersion/certified_residuals`, `recovery_hydro_verdict.exact_verdict/certified_verdict/closure_block/density_functional`.
- Produces: evidence JSON with keys `schema`, `law_id`, `collision_sha256`, `invariants`, `dispersion.exact`, `dispersion.certified`, `verdict.exact`, `verdict.certified`, `limits`, `source_sha256`.

- [ ] **Step 1: Write the runner**

```python
# scripts/phi_v2_lattice/experiments/run_recovery_wave3.py
"""Exact wave-3 evidence: invariant census (H0) and dispersion verdict (H1)."""
from __future__ import annotations

import argparse
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import sys

import flint
import numpy as np

from .. import channels as C, staged as P
from .. import recovery_hydro_invariants as H
from .. import recovery_hydro_dispersion as D
from .. import recovery_hydro_verdict as V


def _entries(M, n, m, exact=True):
    if exact:
        return [[str(Fraction(int(M[i, j].p), int(M[i, j].q))) for j in range(m)] for i in range(n)]
    return [[{"mid": str(M[i, j].mid()), "rad": str(M[i, j].rad())} for j in range(m)] for i in range(n)]


def report() -> dict:
    census = H.census()
    exact = {n: D.exact_dispersion(n) for n in D.DIRECTIONS}
    verdict = V.exact_verdict(exact)
    block = V.closure_block(V.density_functional(exact[D.DIRECTIONS[0]].W),
                            [m for n in D.DIRECTIONS for m in (exact[n].M1, exact[n].M2)])
    certified, residuals, certified_verdicts = {}, {}, {}
    for p in D.REFERENCE_PS:
        balls = {n: D.certified_dispersion(n, p, exact[n]) for n in D.DIRECTIONS}
        certified[str(p)] = {str(n): {"M1": _entries(balls[n].M1, 7, 7, False),
                                      "M2": _entries(balls[n].M2, 7, 7, False)} for n in D.DIRECTIONS}
        residuals[str(p)] = {str(n): D.certified_residuals(balls[n]) for n in D.DIRECTIONS}
        certified_verdicts[str(p)] = V.certified_verdict(balls, block)
    # Unit-modulus spectrum of P0 at the proxy coupling (float64 report only, never an acceptance):
    # eigenvalues with |lambda| > 1 - 1e-9 other than the seven at 1 indicate staggered modes.
    P0 = exact[D.DIRECTIONS[0]].P0
    dense = np.array([[float(Fraction(int(P0[i, j].p), int(P0[i, j].q))) for j in range(192)] for i in range(192)])
    values = np.linalg.eigvals(dense)
    unit_modulus = sorted(((float(v.real), float(v.imag)) for v in values if abs(v) > 1 - 1e-9),
                          key=lambda z: (round(z[0], 9), round(z[1], 9)))
    result = {
        "schema": "strict-recovery-wave3-exact-1", "law_id": P.LAW_ID, "collision_sha256": C.COLLISION_HASH,
        "calculation_kind": "exact_finite_operators_and_certified_enclosures_not_a_measurement_campaign",
        "invariants": census,
        "dispersion": {
            "unit_modulus_spectrum_float64_report": {"count": len(unit_modulus), "values": unit_modulus,
                                                     "note": "float64 report; seven eigenvalues at 1 are the conserved modes"},
            "proxy_r": str(D.PROXY_R),
            "physical_r": {str(p): str(D.physical_r(p)) for p in D.REFERENCE_PS},
            "physical_r_float": {str(p): float(D.physical_r(p)) for p in D.REFERENCE_PS},
            "exact": {str(n): {"rank_complement": exact[n].rank_complement,
                               "M1": _entries(exact[n].M1, 7, 7), "M2": _entries(exact[n].M2, 7, 7)}
                      for n in D.DIRECTIONS},
            "certified": certified, "certified_residuals": residuals},
        "verdict": {"exact": verdict, "certified": certified_verdicts},
        "limits": {"closure": "linearized Boltzmann product closure assumed",
                   "correlation_leakage_bounded": False, "physical_units": False,
                   "navier_stokes_clay_claim": False, "canonical_adoption": False},
    }
    root = Path(__file__).resolve().parents[3]
    sources = {}
    for module in tuple(sys.modules.values()):
        path = getattr(module, "__file__", None)
        if path:
            path = Path(path).resolve()
            if path.suffix == ".py" and path.is_relative_to(root):
                sources[path.relative_to(root).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
    result["source_sha256"] = dict(sorted(sources.items()))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report(), indent=2, sort_keys=True) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
```

- [ ] **Step 2: Run the evidence**

Run (PowerShell, repository root):

```powershell
$env:PYTHONPATH='scripts'
python -m phi_v2_lattice.experiments.run_recovery_wave3 --output engine/docs/evidence/strict-recovery-wave3-exact.json
```

Expected: completes in under 30 minutes; the JSON contains `verdict.exact.label` equal to one of the four labels and, for each p, a certified report with statuses.

- [ ] **Step 3: Append results below the marker**

Append to `engine/docs/DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md` a section `## Results` that states, copied from the JSON: the exact verdict label; `block_dimension`; for each direction the characteristic polynomial of `M1` and of `M1|_B` (when dim = 4), `s/|n|^2`, the normalized transverse polynomial and longitudinal damping; the certified statuses per p; the maximum certified residuals; the runtime. Then one paragraph "What this does and does not establish" in the house register: linear-response prediction at the declared reference, closure not bounded, H2 tests it. Do not restate the verdict rule.

- [ ] **Step 4: Commit**

```bash
git add scripts/phi_v2_lattice/experiments/run_recovery_wave3.py engine/docs/evidence/strict-recovery-wave3-exact.json engine/docs/DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md
git diff --cached --stat
git commit -m "evidence(strict): wave-3 exact census and dispersion verdict at proxy r0 with certified enclosures at physical r"
```

---

### Task 6: H2 throughput probe

**Files:**
- Create: `scripts/phi_v2_lattice/experiments/probe_strict_cuda_throughput.py`
- Create: `engine/docs/evidence/strict-hydro-throughput-2026-09.json`

**Interfaces:**
- Consumes: `recovery_kinetic_reference.prepare_bank(bank, L, layer=0)`, `native_codec.encode`, the built `engine/build_strict_cuda/ftd_strict_cuda_cli` (WSL2).
- Produces: JSON `{"device": {...}, "runs": [{"L": L, "sites": N, "microticks": 48, "elapsed_seconds": t, "microticks_per_second": v, "state_bytes": b}]}`.

- [ ] **Step 1: Ensure the CUDA CLI is built (WSL2)**

```bash
wsl -d Ubuntu-22.04 -- bash -lc "cd /mnt/c/Users/cpaci/Desktop/ftd && cmake -S engine/strict -B engine/build_strict_cuda -DFTD_STRICT_CUDA=ON -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc -DCMAKE_CUDA_ARCHITECTURES=120 -DCMAKE_BUILD_TYPE=Release && cmake --build engine/build_strict_cuda --target ftd_strict_cuda_cli --parallel 24 && engine/build_strict_cuda/ftd_strict_cuda_cli --device"
```

Expected: a JSON line with `"backend":"cuda_device_kernels"` and the RTX 5090 name.

- [ ] **Step 2: Write the probe**

```python
# scripts/phi_v2_lattice/experiments/probe_strict_cuda_throughput.py
"""Time the accepted CUDA advance on p=1/96 Bernoulli preparations; sizes L in {16,32,48,64}."""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time

import numpy as np

from .. import native_codec as N_
from .. import recovery_kinetic_reference as Ref

ROOT = Path(__file__).resolve().parents[3]


def _linux(path):
    value = str(Path(path).resolve()).replace("\\", "/")
    return "/mnt/" + value[0].lower() + value[2:] if len(value) > 1 and value[1] == ":" else value


def preparation(L, seed, p=1 / 96):
    rng = np.random.default_rng(seed)
    bank = np.zeros((L ** 3, 384), dtype=bool)
    bank[:, :192] = rng.random((L ** 3, 192)) < p
    return N_.encode(Ref.prepare_bank(bank, L, layer=0))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--sizes", type=int, nargs="+", default=[16, 32, 48, 64])
    parser.add_argument("--ticks", type=int, default=48)
    args = parser.parse_args()
    cli = Path(os.environ.get("FTD_STRICT_CUDA_CLI", ROOT / "engine/build_strict_cuda/ftd_strict_cuda_cli"))
    prefix = ["wsl", "-d", "Ubuntu-22.04", "--", _linux(cli)] if os.name == "nt" else [str(cli)]
    device = json.loads(subprocess.run(prefix + ["--device"], capture_output=True, text=True, timeout=60).stdout)
    runs = []
    with tempfile.TemporaryDirectory(dir=ROOT / "engine") as folder:
        folder = Path(folder)
        for L in args.sizes:
            blob = preparation(L, seed=20260907)
            (folder / "in.bin").write_bytes(blob)
            paths = [_linux(folder / n) if os.name == "nt" else str(folder / n) for n in ("in.bin", "out.bin", "ev.json")]
            started = time.perf_counter()
            result = subprocess.run(prefix + [paths[0], paths[1], str(args.ticks), paths[2]],
                                    capture_output=True, text=True, timeout=3600)
            elapsed = time.perf_counter() - started
            if result.returncode:
                raise RuntimeError(result.stderr)
            runs.append({"L": L, "sites": L ** 3, "microticks": args.ticks, "elapsed_seconds": elapsed,
                         "microticks_per_second": args.ticks / elapsed, "state_bytes": len(blob)})
            print(json.dumps(runs[-1]), flush=True)
    args.output.write_text(json.dumps({"device": device, "runs": runs, "note":
        "wall time includes per-tick host download, decode, validation and event extraction inside advance"},
        indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
```

- [ ] **Step 3: Run it**

```powershell
$env:PYTHONPATH='scripts'
python -m phi_v2_lattice.experiments.probe_strict_cuda_throughput --output engine/docs/evidence/strict-hydro-throughput-2026-09.json
```

Expected: four lines of timing; the L = 64 run may take several minutes. Record the numbers; do not tune anything.

- [ ] **Step 4: Commit**

```bash
git add scripts/phi_v2_lattice/experiments/probe_strict_cuda_throughput.py engine/docs/evidence/strict-hydro-throughput-2026-09.json
git diff --cached --stat
git commit -m "evidence(strict): CUDA advance throughput probe at L=16..64 for the hydrodynamic campaign sizing"
```

---

### Task 7: H2 CUDA campaign instrument

**Files:**
- Create: `engine/strict/recovery_hydro/hydro_main.cpp`
- Create: `engine/strict/recovery_hydro/CMakeLists.txt`

**Interfaces:**
- Consumes: `ftd::strict::decode/encode/work_units`, `ftd::strict::gpu::advance(State&, uint64_t)`, `ftd::strict::gpu::device_json()` (unchanged headers `staged_runtime.h`, `staged_cuda.h`).
- Produces: executable `ftd_strict_hydro_campaign MANIFEST_TSV TRACE_JSONL WEIGHTS_TSV STROBOSCOPES`. Manifest line: `case_id<TAB>path<TAB>L<TAB>mx<TAB>my<TAB>mz<TAB>polarity_offset`. Weights file: 7 lines of 192 tab-separated integers. Trace line per case: `{"case_id":..,"L":..,"k":[mx,my,mz],"polarity_offset":..,"stroboscopes":[{"n":0,"microtick":"0","sha256":"..","population":N,"moments":[[re,im],...]}, ...]}`; final state written next to the input as `<case>.final.bin`.

- [ ] **Step 1: Write the CMake project**

```cmake
# engine/strict/recovery_hydro/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(ftd_strict_hydro_campaign LANGUAGES CXX CUDA)
set(FTD_STRICT_CUDA ON CACHE BOOL "Real CUDA campaign backend" FORCE)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/.." strict_core)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)
add_executable(ftd_strict_hydro_campaign hydro_main.cpp)
target_compile_features(ftd_strict_hydro_campaign PRIVATE cxx_std_17)
target_link_libraries(ftd_strict_hydro_campaign PRIVATE ftd_strict_cuda OpenSSL::Crypto)
```

- [ ] **Step 2: Write the instrument**

```cpp
// engine/strict/recovery_hydro/hydro_main.cpp
// Registered stroboscopic observation of the accepted CUDA evolution. No kernel or law changes.
#include "staged_cuda.h"
#include <openssl/sha.h>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace ftd::strict;
constexpr unsigned MODES = 7, CHANNELS = 192, PERIOD = 48;

std::string digest(const std::vector<std::uint8_t>& bytes) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(bytes.data(), bytes.size(), hash);
    std::ostringstream out; out << std::hex << std::setfill('0');
    for (auto byte : hash) out << std::setw(2) << unsigned(byte);
    return out.str();
}

struct Case { std::string id, path; unsigned L; long mx, my, mz; unsigned offset; };

std::vector<std::vector<long>> read_weights(const std::string& path) {
    std::ifstream in(path); if (!in) throw std::runtime_error("cannot open weights");
    std::vector<std::vector<long>> weights; std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        std::vector<long> row; std::istringstream fields(line); long v;
        while (fields >> v) row.push_back(v);
        if (row.size() != CHANNELS) throw std::runtime_error("weight row must have 192 integers");
        weights.push_back(row);
    }
    if (weights.size() != MODES) throw std::runtime_error("exactly 7 weight rows required");
    return weights;
}

// Prints the observation FIELDS only (no surrounding braces); the caller frames the object.
void observe(std::ostream& out, const State& st, const Case& c, const std::vector<std::vector<long>>& w) {
    const auto L = st.L; const std::size_t n = st.s.size();
    double re[MODES] = {0}, im[MODES] = {0}; std::uint64_t population = 0;
    const double two_pi_over_L = 2.0 * M_PI / double(L);
    for (std::size_t i = 0; i < n; ++i) {
        long site[MODES] = {0}; bool any = false;
        const std::uint8_t* bank = st.bank.data() + i * 384 + c.offset;
        for (unsigned ch = 0; ch < CHANNELS; ++ch) if (bank[ch]) {
            any = true; ++population;
            for (unsigned a = 0; a < MODES; ++a) site[a] += w[a][ch];
        }
        if (!any) continue;
        const long x = long(i / (std::size_t(L) * L)), y = long((i / L) % L), z = long(i % L);
        const double theta = -two_pi_over_L * double(c.mx * x + c.my * y + c.mz * z);
        const double cs = std::cos(theta), sn = std::sin(theta);
        for (unsigned a = 0; a < MODES; ++a) { re[a] += site[a] * cs; im[a] += site[a] * sn; }
    }
    out << "\"microtick\":\"" << st.microtick << "\",\"sha256\":\"" << digest(encode(st))
        << "\",\"population\":" << population << ",\"moments\":[";
    out << std::setprecision(17);
    for (unsigned a = 0; a < MODES; ++a) out << (a ? "," : "") << '[' << re[a] << ',' << im[a] << ']';
    out << "]";
}

Case parse(const std::string& line) {
    std::istringstream fields(line); Case c; std::string L, mx, my, mz, off;
    if (!std::getline(fields, c.id, '\t') || !std::getline(fields, c.path, '\t') || !std::getline(fields, L, '\t')
        || !std::getline(fields, mx, '\t') || !std::getline(fields, my, '\t') || !std::getline(fields, mz, '\t')
        || !std::getline(fields, off, '\t')) throw std::runtime_error("manifest line needs 7 tab-separated fields");
    if (c.id.empty() || c.id.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789_") != std::string::npos)
        throw std::runtime_error("invalid registered caseID");
    c.L = unsigned(std::stoul(L)); c.mx = std::stol(mx); c.my = std::stol(my); c.mz = std::stol(mz);
    c.offset = unsigned(std::stoul(off));
    if (c.offset != 0 && c.offset != 192) throw std::runtime_error("polarity offset must be 0 or 192");
    return c;
}
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 5) throw std::runtime_error("usage: hydro_campaign MANIFEST_TSV TRACE_JSONL WEIGHTS_TSV STROBOSCOPES");
        std::string count = argv[4];
        if (count.empty() || count.find_first_not_of("0123456789") != std::string::npos)
            throw std::runtime_error("invalid stroboscope count");
        const auto stroboscopes = std::stoull(count);
        const auto weights = read_weights(argv[3]);
        std::ifstream manifest(argv[1]); if (!manifest) throw std::runtime_error("cannot open manifest");
        std::filesystem::path target(argv[2]), partial = target.string() + ".part";
        if (std::filesystem::exists(target)) throw std::runtime_error("refuse to overwrite completed campaign trace");
        std::ofstream output(partial); if (!output) throw std::runtime_error("cannot open trace");
        std::cerr << gpu::device_json() << '\n';
        unsigned cases = 0; std::string line;
        while (std::getline(manifest, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            const Case c = parse(line);
            std::ifstream input(c.path, std::ios::binary); if (!input) throw std::runtime_error("cannot read preparation");
            std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
            auto state = decode(bytes);
            if (state.L != c.L || state.microtick != 0) throw std::runtime_error("unregistered domain or preparation tick");
            const auto work = work_units(state);
            output << "{\"case_id\":\"" << c.id << "\",\"L\":" << c.L << ",\"k\":[" << c.mx << ',' << c.my << ',' << c.mz
                   << "],\"polarity_offset\":" << c.offset << ",\"stroboscopes\":[{\"n\":0,";
            observe(output, state, c, weights);
            output << '}';
            for (std::uint64_t s = 1; s <= stroboscopes; ++s) {
                gpu::advance(state, PERIOD);   // events are computed by the accepted backend and discarded here
                if (work_units(state) != work) throw std::runtime_error("work unit drift across a period");
                output << ",{\"n\":" << s << ',';
                observe(output, state, c, weights);
                output << '}';
            }
            output << "]}\n";
            if (!output) throw std::runtime_error("trace write failed");
            auto final_path = std::filesystem::path(c.path); final_path.replace_extension(".final.bin");
            std::ofstream final(final_path, std::ios::binary); auto final_bytes = encode(state);
            final.write(reinterpret_cast<const char*>(final_bytes.data()), std::streamsize(final_bytes.size()));
            if (!final) throw std::runtime_error("endpoint snapshot write failed");
            std::cerr << "completed_cases=" << ++cases << '\n';
        }
        if (!cases) throw std::runtime_error("empty manifest");
        output.close(); if (!output) throw std::runtime_error("trace close failed");
        std::filesystem::rename(partial, target);
        std::cerr << "completed_cases=" << cases << " microticks=" << cases * stroboscopes * PERIOD << '\n';
        return 0;
    } catch (const std::exception& e) { std::cerr << "hydro campaign failed: " << e.what() << '\n'; return 1; }
}
```

- [ ] **Step 3: Build in WSL2 and smoke-test with zero stroboscopes**

```bash
wsl -d Ubuntu-22.04 -- bash -lc "cd /mnt/c/Users/cpaci/Desktop/ftd && cmake -S engine/strict/recovery_hydro -B engine/build_strict_hydro -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc -DCMAKE_CUDA_ARCHITECTURES=120 -DCMAKE_BUILD_TYPE=Release && cmake --build engine/build_strict_hydro --parallel 24 && ls -l engine/build_strict_hydro/ftd_strict_hydro_campaign"
```

Expected: the binary exists. The Python parity test in Task 8 exercises it with `STROBOSCOPES=0`.

- [ ] **Step 4: Commit**

```bash
git add engine/strict/recovery_hydro/hydro_main.cpp engine/strict/recovery_hydro/CMakeLists.txt
git diff --cached --stat
git commit -m "feat(strict): hydrodynamic campaign instrument — stroboscopic conserved-moment projection over the accepted CUDA advance"
```

---

### Task 8: H2 campaign runner (cases, preparation, observable, prediction, locks)

**Files:**
- Create: `scripts/phi_v2_lattice/recovery_hydro_campaign.py`
- Test: `scripts/tests/phi_v2_lattice/test_recovery_hydro_campaign.py`

**Interfaces:**
- Consumes: `recovery_kinetic_reference.prepare_bank`, `native_codec.encode/decode`, `staged.LAW_ID`, `geometry.coords`, `recovery_hydro_invariants.locked_kernel(0)`, `recovery_hydro_dispersion.exact_dispersion/numeric_period_map/physical_r`, `recovery_carriers._linux/instrument_paths`-style closure logic (re-implemented locally; do not import the carrier module's campaign functions).
- Produces: `HydroCase`, `REGISTRATION` (dict built by `registration(L, stroboscopes)`), `cases(L, stroboscopes)`, `prepare(case) -> StagedState`, `weights_rows()`, `observe_moments(state, case) -> list[complex]`, `predict(case, stroboscopes) -> np.ndarray (n+1, 7)`, `prepare_campaign(directory, L, stroboscopes)`, `validate_lock(directory)`, `run_campaign(directory)`, `summarize_campaign(directory)`.

- [ ] **Step 1: Write the failing tests**

```python
# scripts/tests/phi_v2_lattice/test_recovery_hydro_campaign.py
"""Preparation, observable, prediction and lock protocol for gate H2. No GPU trajectory here."""
from fractions import Fraction
import json
import os
from pathlib import Path
import subprocess

import numpy as np
import pytest

from phi_v2_lattice import native_codec as N_
from phi_v2_lattice import recovery_hydro_campaign as Camp
from phi_v2_lattice import recovery_hydro_dispersion as D

ROOT = Path(__file__).resolve().parents[3]


def test_cases_enumerate_registered_factors():
    cases = Camp.cases(L=16, stroboscopes=4)
    assert len(cases) == 3 * 2 * 7 * Camp.SEEDS  # directions x m x modes x seeds
    assert len({c.case_id for c in cases}) == len(cases)


def test_preparation_is_deterministic_and_bounded():
    case = Camp.cases(L=8, stroboscopes=1)[0]
    a, b = Camp.prepare(case), Camp.prepare(case)
    assert N_.encode(a) == N_.encode(b)
    probabilities = Camp.probabilities(case)
    assert probabilities.min() >= 0 and probabilities.max() <= 1
    assert a.lattice.bank[:, 192:].sum() == 0  # other polarity empty


def test_observable_matches_direct_definition():
    case = Camp.cases(L=8, stroboscopes=1)[0]
    state = Camp.prepare(case)
    W = np.array(Camp.weights_rows(), dtype=float)
    L = case.L
    expected = np.zeros(7, dtype=complex)
    for i in range(L ** 3):
        x, y, z = i // (L * L), (i // L) % L, i % L
        phase = np.exp(-2j * np.pi * (case.mx * x + case.my * y + case.mz * z) / L)
        expected += phase * (W @ state.lattice.bank[i, :192].astype(float))
    assert np.allclose(Camp.observe_moments(state, case), expected)


def test_prediction_starts_at_expected_initial_amplitude():
    case = Camp.cases(L=16, stroboscopes=3)[0]
    predicted = Camp.predict(case, 3)
    assert predicted.shape == (4, 7)
    # h0 = N p eps phi / 2 with phi the max-normalized column `mode` of V, so W h0 has a single
    # nonzero component at `mode` equal to N p eps / (2 * max|V column|), since W V = I exactly.
    W = np.array(Camp.weights_rows(), dtype=float)
    projected = W @ Camp.mode_shape(case.mode)
    amplitude = 16 ** 3 * float(Camp.P) * float(Camp.EPSILON) / 2
    assert np.allclose(predicted[0], amplitude * projected, rtol=1e-9, atol=1e-9 * amplitude)
    assert np.count_nonzero(np.abs(projected) > 1e-9) == 1 and abs(projected[case.mode]) > 1e-9


def test_instrument_parity_at_zero_stroboscopes(tmp_path):
    runner = ROOT / "engine/build_strict_hydro/ftd_strict_hydro_campaign"
    if not runner.is_file():
        pytest.skip("optional hydro campaign runner not built")
    case = Camp.cases(L=4, stroboscopes=0)[0]
    state = Camp.prepare(case)
    (tmp_path / "c.bin").write_bytes(N_.encode(state))
    Camp.write_weights(tmp_path / "weights.tsv")
    manifest = f"{case.case_id}\t{Camp._linux(tmp_path / 'c.bin')}\t{case.L}\t{case.mx}\t{case.my}\t{case.mz}\t0\n"
    (tmp_path / "manifest.tsv").write_text(manifest, encoding="utf-8")
    args = [Camp._linux(p) for p in (runner, tmp_path / "manifest.tsv", tmp_path / "trace.jsonl", tmp_path / "weights.tsv")]
    command = (["wsl", "-d", "Ubuntu-22.04", "--"] + args if os.name == "nt" else args) + ["0"]
    result = subprocess.run(command, capture_output=True, text=True, timeout=300)
    assert result.returncode == 0, result.stderr
    trace = json.loads((tmp_path / "trace.jsonl").read_text().splitlines()[0])
    measured = np.array([complex(a, b) for a, b in trace["stroboscopes"][0]["moments"]])
    assert np.allclose(measured, Camp.observe_moments(state, case), atol=1e-6)
    assert trace["stroboscopes"][0]["population"] == int(state.lattice.bank[:, :192].sum())


def test_lock_rejects_tampering(tmp_path, monkeypatch):
    if not (ROOT / "engine/build_strict_hydro/ftd_strict_hydro_campaign").is_file():
        pytest.skip("optional hydro campaign runner required for lock fixture")
    one = Camp.cases(L=4, stroboscopes=1)[:1]
    monkeypatch.setattr(Camp, "cases", lambda L, stroboscopes: one)
    Camp.prepare_campaign(tmp_path, L=4, stroboscopes=1)
    Camp.validate_lock(tmp_path)
    (tmp_path / (one[0].case_id + ".bin")).write_bytes(b"\x00" * 10)
    with pytest.raises(ValueError):
        Camp.validate_lock(tmp_path)
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `python -m pytest scripts/tests/phi_v2_lattice/test_recovery_hydro_campaign.py -q`
Expected: collection error, module missing.

- [ ] **Step 3: Write the module**

```python
# scripts/phi_v2_lattice/recovery_hydro_campaign.py
"""Registered hydrodynamic response campaign (gate H2): preparation, observable, prediction, locks."""
from __future__ import annotations

from dataclasses import asdict, dataclass
from fractions import Fraction
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time
import types

import numpy as np

from . import native_codec as N_
from . import staged as Staged
from . import recovery_hydro_dispersion as D
from . import recovery_hydro_invariants as H
from . import recovery_kinetic_reference as Ref

PROTOCOL_ID = "strict-hydro-response-1"
P_REFERENCE = Fraction(1, 96)
P = P_REFERENCE
EPSILON = Fraction(1, 4)
SEEDS = 8
DIRECTIONS = D.DIRECTIONS
WAVENUMBERS = (1, 2)
PERIOD = 48
ACCEPTANCE = {"relative_rms_max": Fraction(1, 10), "rate_band_sigma": 2}
RUNNER = "engine/build_strict_hydro/ftd_strict_hydro_campaign"


@dataclass(frozen=True)
class HydroCase:
    case_id: str
    L: int
    mx: int
    my: int
    mz: int
    mode: int
    seed: int
    polarity_offset: int = 0


def cases(L: int, stroboscopes: int) -> tuple[HydroCase, ...]:
    out = []
    for direction in DIRECTIONS:
        for m in WAVENUMBERS:
            for mode in range(7):
                for seed_index in range(SEEDS):
                    tag = "".join(str(x) for x in direction)
                    out.append(HydroCase(f"h_d{tag}_m{m}_mode{mode}_s{seed_index}", L,
                                         m * direction[0], m * direction[1], m * direction[2],
                                         mode, 20260907 * 1000 + seed_index))
    return tuple(out)


def _exact_proxy():
    return D.exact_dispersion((1, 0, 0))


_PROXY = None


def _proxy():
    global _PROXY
    if _PROXY is None:
        _PROXY = _exact_proxy()
    return _PROXY


def weights_rows() -> tuple[tuple[int, ...], ...]:
    """Stage-0 conserved weights, in the fixed order used by W in the dispersion module."""
    return H.locked_kernel(0)


def mode_shape(mode: int) -> np.ndarray:
    """Right equilibrium mode `mode` (column of V at the proxy coupling), scaled to max |phi| = 1."""
    V = _proxy().V
    column = np.array([float(Fraction(int(V[c, mode].p), int(V[c, mode].q))) for c in range(192)])
    return column / np.max(np.abs(column))


def probabilities(case: HydroCase) -> np.ndarray:
    L = case.L
    idx = np.arange(L ** 3)
    x, y, z = idx // (L * L), (idx // L) % L, idx % L
    phase = np.cos(2 * np.pi * (case.mx * x + case.my * y + case.mz * z) / L)
    phi = mode_shape(case.mode)
    return float(P) * (1 + float(EPSILON) * np.outer(phase, phi))


def prepare(case: HydroCase) -> Staged.StagedState:
    rng = np.random.default_rng(case.seed)
    L = case.L
    bank = np.zeros((L ** 3, 384), dtype=bool)
    bank[:, case.polarity_offset:case.polarity_offset + 192] = rng.random((L ** 3, 192)) < probabilities(case)
    return Ref.prepare_bank(bank, L, layer=0)


def observe_moments(state: Staged.StagedState, case: HydroCase) -> np.ndarray:
    L = case.L
    W = np.array(weights_rows(), dtype=float)
    idx = np.arange(L ** 3)
    x, y, z = idx // (L * L), (idx // L) % L, idx % L
    phase = np.exp(-2j * np.pi * (case.mx * x + case.my * y + case.mz * z) / L)
    bank = state.lattice.bank[:, case.polarity_offset:case.polarity_offset + 192].astype(float)
    return phase @ (bank @ W.T)


def predict(case: HydroCase, stroboscopes: int) -> np.ndarray:
    """Linear-Boltzmann prediction m(n) = W P(k)^n h0 with h0 = N p eps phi / 2 (float64)."""
    L = case.L
    k = 2 * np.pi * np.array([case.mx, case.my, case.mz], dtype=float) / L
    Pk = D.numeric_period_map(k, P)
    W = np.array(weights_rows(), dtype=float)
    h = (L ** 3) * float(P) * float(EPSILON) / 2 * mode_shape(case.mode).astype(complex)
    out = np.zeros((stroboscopes + 1, 7), dtype=complex)
    for n in range(stroboscopes + 1):
        out[n] = W @ h
        h = Pk @ h
    return out


def slowest_rate(L: int) -> float:
    """Smallest per-period decay -ln|lambda| among the seven slow modes over the registered wavevectors."""
    rates = []
    for direction in DIRECTIONS:
        for m in WAVENUMBERS:
            k = 2 * np.pi * m * np.array(direction, dtype=float) / L
            values = np.linalg.eigvals(D.numeric_period_map(k, P))
            slow = np.sort(np.abs(values))[-7:]
            if np.any(slow > 1 + 1e-9):
                raise ValueError(f"linear instability at k={k}: |lambda|={slow.max()}; report, do not register")
            rates.append(float(np.min(-np.log(np.minimum(slow, 1.0)))))
    return min(rates)


def horizon(L: int, microticks_per_second: float, budget_seconds: float = 7200.0) -> int:
    """At least three e-folds of the slowest mode, capped by the throughput budget."""
    needed = int(np.ceil(3.0 / slowest_rate(L)))
    affordable = int(budget_seconds * microticks_per_second // (len(cases(L, 1)) * PERIOD))
    return max(1, min(needed, affordable))


def registration(L: int, stroboscopes: int) -> dict:
    return {"protocol_id": PROTOCOL_ID, "law_id": Staged.LAW_ID, "p": str(P), "epsilon": str(EPSILON),
            "L": L, "stroboscopes": stroboscopes, "period_microticks": PERIOD, "directions": DIRECTIONS,
            "wavenumbers": WAVENUMBERS, "modes": list(range(7)), "seeds": SEEDS,
            "proxy_r_for_mode_shapes": str(D.PROXY_R), "acceptance": {k: str(v) for k, v in ACCEPTANCE.items()},
            "weights_sha256": _sha("\n".join("\t".join(map(str, r)) for r in weights_rows()).encode()),
            "boundary": "periodic", "background": "homogeneous doubly occupied A9, s=0, ell=0, single polarity"}


def _json(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), default=list)


def _sha(data):
    return hashlib.sha256(data).hexdigest()


def _linux(path):
    value = str(Path(path).resolve()).replace("\\", "/")
    return "/mnt/" + value[0].lower() + value[2:] if len(value) > 1 and value[1] == ":" else value


def write_weights(path):
    Path(path).write_text("\n".join("\t".join(map(str, r)) for r in weights_rows()) + "\n", encoding="utf-8")


def instrument_paths() -> set[str]:
    root = Path(__file__).resolve().parents[2]
    paths = {"engine/strict/recovery_hydro/hydro_main.cpp", "engine/strict/recovery_hydro/CMakeLists.txt",
             "engine/strict/CMakeLists.txt", "engine/strict/cuda.cmake", "engine/strict/staged_runtime.cpp",
             "engine/strict/staged_cuda.cu", "engine/strict/frozen_tables.h", "engine/strict/staged_runtime.h",
             "engine/strict/staged_cuda.h", "engine/docs/PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md"}
    seeds = (__name__, "phi_v2_lattice", "phi_v2_lattice.channels", "phi_v2_lattice.state", "phi_v2_lattice._proofs",
             "phi_v2_lattice.geometry", "phi_v2_lattice.staged", "phi_v2_lattice.tick", "phi_v2_lattice.native_codec",
             "phi_v2_lattice.checkpoint", "phi_v2_lattice.recovery_hydro_dispersion",
             "phi_v2_lattice.recovery_hydro_invariants", "phi_v2_lattice.recovery_kinetic_response",
             "phi_v2_lattice.recovery_kinetic_reference")
    pending = [sys.modules[name] for name in seeds]
    seen = set()
    while pending:
        module = pending.pop()
        if module.__name__ in seen:
            continue
        seen.add(module.__name__)
        raw = getattr(module, "__file__", None)
        if not raw:
            continue
        path = Path(raw).resolve()
        if path.suffix != ".py" or not path.is_relative_to(root):
            continue
        paths.add(path.relative_to(root).as_posix())
        for value in tuple(vars(module).values()):
            if isinstance(value, types.ModuleType):
                pending.append(value)
            elif isinstance(value, (types.FunctionType, type)):
                parent = sys.modules.get(getattr(value, "__module__", ""))
                if parent is not None:
                    pending.append(parent)
    return paths


def prepare_campaign(directory, L: int, stroboscopes: int) -> dict:
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    if (directory / "lock.json").exists():
        raise ValueError("campaign already locked; do not overwrite registration")
    root = Path(__file__).resolve().parents[2]
    runner = root / RUNNER
    if not runner.is_file():
        raise ValueError("build the real CUDA hydro runner before locking")
    write_weights(directory / "weights.tsv")
    manifest, lines, predictions = [], [], {}
    for case in cases(L, stroboscopes):
        path = directory / (case.case_id + ".bin")
        blob = N_.encode(prepare(case))
        path.write_bytes(blob)
        manifest.append({"case": asdict(case), "initial_sha256": _sha(blob)})
        lines.append("\t".join([case.case_id, _linux(path), str(case.L), str(case.mx), str(case.my), str(case.mz),
                                str(case.polarity_offset)]))
        predicted = predict(case, stroboscopes)
        predictions[case.case_id] = [[[float(v.real), float(v.imag)] for v in row] for row in predicted]
    reg = registration(L, stroboscopes)
    (directory / "predictions.json").write_bytes((_json(predictions) + "\n").encode("utf-8"))
    lock = {"registration": reg, "registration_sha256": _sha(_json(reg).encode()),
            "manifest": manifest, "manifest_sha256": _sha(_json(manifest).encode()),
            "predictions_sha256": _sha((directory / "predictions.json").read_bytes()),
            "instrument_sha256": {p: _sha((root / p).read_bytes()) for p in sorted(instrument_paths())},
            "runner_sha256": _sha(runner.read_bytes()),
            "weights_sha256": _sha((directory / "weights.tsv").read_bytes())}
    (directory / "lock.json").write_text(_json(lock) + "\n", encoding="utf-8")
    (directory / "manifest.tsv").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return {"case_count": len(manifest), "manifest_sha256": lock["manifest_sha256"],
            "registration_sha256": lock["registration_sha256"],
            "microticks": len(manifest) * stroboscopes * PERIOD}


def validate_lock(directory) -> dict:
    directory = Path(directory)
    lock = json.loads((directory / "lock.json").read_text())
    root = Path(__file__).resolve().parents[2]
    if set(lock["instrument_sha256"]) != instrument_paths():
        raise ValueError("incomplete instrument source closure")
    if any(_sha((root / p).read_bytes()) != h for p, h in lock["instrument_sha256"].items()):
        raise ValueError("instrument source changed after registration lock")
    if _sha((root / RUNNER).read_bytes()) != lock["runner_sha256"]:
        raise ValueError("CUDA runner changed after registration lock")
    reg = lock["registration"]
    if _sha(_json(reg).encode()) != lock["registration_sha256"] or _json(reg) != _json(registration(reg["L"], reg["stroboscopes"])):
        raise ValueError("registration lock changed")
    if _sha(_json(lock["manifest"]).encode()) != lock["manifest_sha256"]:
        raise ValueError("manifest lock changed")
    if [row["case"] for row in lock["manifest"]] != [asdict(c) for c in cases(reg["L"], reg["stroboscopes"])]:
        raise ValueError("manifest case inventory differs from registration")
    if _sha((directory / "predictions.json").read_bytes()) != lock["predictions_sha256"]:
        raise ValueError("locked predictions changed")
    if _sha((directory / "weights.tsv").read_bytes()) != lock["weights_sha256"]:
        raise ValueError("weights changed")
    expected = []
    for row in lock["manifest"]:
        c = row["case"]
        path = directory / (c["case_id"] + ".bin")
        if _sha(path.read_bytes()) != row["initial_sha256"]:
            raise ValueError("initial preparation hash changed")
        expected.append("\t".join([c["case_id"], _linux(path), str(c["L"]), str(c["mx"]), str(c["my"]), str(c["mz"]),
                                   str(c["polarity_offset"])]))
    if (directory / "manifest.tsv").read_text(encoding="utf-8") != "\n".join(expected) + "\n":
        raise ValueError("executable manifest differs from lock")
    return lock


def run_campaign(directory) -> dict:
    directory = Path(directory).resolve()
    lock = validate_lock(directory)
    root = Path(__file__).resolve().parents[2]
    runner = root / RUNNER
    preflight = {"lock_sha256": _sha((directory / "lock.json").read_bytes()),
                 "registration_sha256": lock["registration_sha256"], "manifest_sha256": lock["manifest_sha256"],
                 "runner_sha256": lock["runner_sha256"], "instrument_sha256": lock["instrument_sha256"],
                 "case_count": len(lock["manifest"]), "validation": "complete preflight passed"}
    (directory / "preflight.json").write_text(_json(preflight) + "\n", encoding="utf-8")
    args = [runner, directory / "manifest.tsv", directory / "trace.jsonl", directory / "weights.tsv"]
    command = (["wsl", "-d", "Ubuntu-22.04", "--"] + [_linux(p) for p in args] if os.name == "nt"
               else [str(p) for p in args])
    started = time.perf_counter()
    result = subprocess.run(command + [str(lock["registration"]["stroboscopes"])], capture_output=True, text=True,
                            timeout=4 * 3600)
    elapsed = time.perf_counter() - started
    (directory / "execution.stdout.txt").write_text(result.stdout, encoding="utf-8")
    (directory / "execution.stderr.txt").write_text(result.stderr, encoding="utf-8")
    if result.returncode:
        raise RuntimeError("GPU hydro runner failed; partial evidence retained: " + result.stderr)
    if validate_lock(directory) != lock or _sha((directory / "lock.json").read_bytes()) != preflight["lock_sha256"]:
        raise ValueError("campaign inputs changed during GPU execution")
    devices = [json.loads(line) for line in result.stderr.splitlines() if line.startswith("{")]
    if len(devices) != 1 or devices[0].get("backend") != "cuda_device_kernels":
        raise ValueError("missing real GPU provenance")
    execution = {"schema": "strict-hydro-execution-1", "preflight_sha256": _sha((directory / "preflight.json").read_bytes()),
                 "postflight": "complete frozen-source/input validation passed", "device": devices[0],
                 "elapsed_seconds": elapsed,
                 "physical_microticks": len(lock["manifest"]) * lock["registration"]["stroboscopes"] * PERIOD,
                 "trace_sha256": _sha((directory / "trace.jsonl").read_bytes())}
    (directory / "execution.json").write_text(_json(execution) + "\n", encoding="utf-8")
    return execution


def _fit_rate(series: np.ndarray) -> float:
    """Declared estimator: least-squares slope of ln|m(n)| against n over the horizon."""
    magnitude = np.abs(series)
    n = np.arange(len(series))
    mask = magnitude > 0
    slope = np.polyfit(n[mask], np.log(magnitude[mask]), 1)[0] if mask.sum() > 1 else float("nan")
    return -float(slope)


def summarize_campaign(directory) -> dict:
    directory = Path(directory)
    lock = validate_lock(directory)
    execution = json.loads((directory / "execution.json").read_text())
    preflight_bytes = (directory / "preflight.json").read_bytes()
    preflight = json.loads(preflight_bytes)
    if execution["preflight_sha256"] != _sha(preflight_bytes) or preflight["lock_sha256"] != _sha((directory / "lock.json").read_bytes()):
        raise ValueError("execution/preflight receipt is not tied to current lock")
    if execution["trace_sha256"] != _sha((directory / "trace.jsonl").read_bytes()):
        raise ValueError("trace changed after execution")
    predictions = json.loads((directory / "predictions.json").read_text())
    reg = lock["registration"]
    traces = {}
    with (directory / "trace.jsonl").open(encoding="utf-8") as stream:
        for expected, line in zip(lock["manifest"], stream, strict=True):
            trace = json.loads(line)
            c = expected["case"]
            if trace["case_id"] != c["case_id"]:
                raise ValueError("case order mismatch")
            initial = N_.decode((directory / (c["case_id"] + ".bin")).read_bytes())
            case = HydroCase(**c)
            measured0 = np.array([complex(a, b) for a, b in trace["stroboscopes"][0]["moments"]])
            if not np.allclose(measured0, observe_moments(initial, case), atol=1e-6):
                raise ValueError("initial GPU observer mismatch")
            final = N_.decode((directory / (c["case_id"] + ".final.bin")).read_bytes())
            if trace["stroboscopes"][-1]["sha256"] != _sha(N_.encode(final)):
                raise ValueError("final checkpoint hash mismatch")
            traces[c["case_id"]] = np.array([[complex(a, b) for a, b in s["moments"]] for s in trace["stroboscopes"]])
    groups = {}
    for row in lock["manifest"]:
        c = row["case"]
        groups.setdefault((c["mx"], c["my"], c["mz"], c["mode"]), []).append(c["case_id"])
    results, passes = [], 0
    for key, ids in sorted(groups.items()):
        stack = np.array([traces[i] for i in ids])          # seeds x (n+1) x 7
        mean = stack.mean(axis=0)
        stderr = stack.std(axis=0, ddof=1) / np.sqrt(len(ids))
        mode = key[3]
        predicted = np.array([[complex(a, b) for a, b in row] for row in predictions[ids[0]]])
        rel_rms = float(np.sqrt(np.sum(np.abs(mean[:, mode] - predicted[:, mode]) ** 2)
                                / np.sum(np.abs(predicted[:, mode]) ** 2)))
        rate_measured = _fit_rate(mean[:, mode])
        rate_seeds = np.array([_fit_rate(t[:, mode]) for t in stack])
        rate_predicted = _fit_rate(predicted[:, mode])
        band = ACCEPTANCE["rate_band_sigma"] * float(np.std(rate_seeds, ddof=1) / np.sqrt(len(ids)))
        rms_ok = rel_rms <= float(ACCEPTANCE["relative_rms_max"])
        rate_ok = abs(rate_measured - rate_predicted) <= band
        passes += rms_ok and rate_ok
        results.append({"k": key[:3], "mode": mode, "seeds": len(ids), "relative_rms": rel_rms,
                        "rate_measured": rate_measured, "rate_predicted": rate_predicted, "rate_band": band,
                        "rms_pass": rms_ok, "rate_pass": rate_ok,
                        "mean_trajectory": [[float(v.real), float(v.imag)] for v in mean[:, mode]],
                        "stderr_trajectory": [[float(v.real), float(v.imag)] for v in stderr[:, mode]]})
    output = {"schema": "strict-hydro-report-1", "protocol_id": PROTOCOL_ID, "law_id": Staged.LAW_ID,
              "registration_sha256": lock["registration_sha256"], "manifest_sha256": lock["manifest_sha256"],
              "predictions_sha256": lock["predictions_sha256"], "trace_sha256": execution["trace_sha256"],
              "runner_sha256": lock["runner_sha256"], "instrument_sha256": lock["instrument_sha256"],
              "execution": execution, "registration": reg, "groups": len(results), "groups_passing": passes,
              "boltzmann_closure_verified_at_registered_scope": passes == len(results),
              "results": results}
    (directory / "report.json").write_text(_json(output) + "\n", encoding="utf-8")
    return output
```

- [ ] **Step 4: Run the tests**

Run: `python -m pytest scripts/tests/phi_v2_lattice/test_recovery_hydro_campaign.py -q`
Expected: `6 passed` when the runner is built, otherwise `4 passed, 2 skipped`. Skips are not evidence; build the runner (Task 7) before proceeding.

- [ ] **Step 5: Commit**

```bash
git add scripts/phi_v2_lattice/recovery_hydro_campaign.py scripts/tests/phi_v2_lattice/test_recovery_hydro_campaign.py
git diff --cached --stat
git commit -m "feat(strict): hydrodynamic response campaign runner — registered cases, Bernoulli mode preparations, locked predictions, lock protocol"
```

---

### Task 9: H2 preregistration document

**Files:**
- Create: `engine/docs/PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md`

**Interfaces:** the document's numbers come from evidence: L from `strict-hydro-throughput-2026-09.json` and `Camp.horizon`, rates from `Camp.slowest_rate`. It is part of the instrument closure: changing it after the lock invalidates the lock.

- [ ] **Step 1: Compute the registered size and horizon**

```powershell
$env:PYTHONPATH='scripts'
python -c "import json; from phi_v2_lattice import recovery_hydro_campaign as C; probe=json.load(open('engine/docs/evidence/strict-hydro-throughput-2026-09.json')); [print(r['L'], C.slowest_rate(r['L']), C.horizon(r['L'], r['microticks_per_second'])) for r in probe['runs']]"
```

Choose the largest L whose `horizon` equals the three-e-fold `needed` value (not budget-capped); if none, choose the largest L whose horizon is at least 12 stroboscopes and record that the three-e-fold target was budget-capped.

- [ ] **Step 2: Write the document**

```markdown
# Registered hydrodynamic response campaign

Date: <date of writing>. Status: **[PREREGISTRATION — LINEAR-RESPONSE VERIFICATION]**.
Law: `phi-v2-staged-candidate-1`. Protocol: `strict-hydro-response-1`.
Prediction source: [H1 dispersion](DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md).
No preparation, horizon, observable, or acceptance band changes after this lock.

## Hypothesis

The linearized Boltzmann period map `P(k)` predicts the stroboscopic evolution
of the seven conserved moments of a small sinusoidal perturbation of the
p = 1/96 counting reference. Failure is retained as "Boltzmann closure fails
for the staged law at this preparation", not corrected.

## Locked matrix

| Factor | Values |
|---|---|
| Lattice | periodic L = <L>, single polarity (offset 0), background A9 code `encode(0,+1)` in both slots, s = 0, ell = 0, phase 0 |
| Directions | (1,0,0), (1,1,0), (1,1,1) |
| Wavenumbers | m in {1, 2}; k = 2 pi m n / L |
| Modes | the seven stage-0 conserved modes, equilibrium shapes from the exact proxy V, scaled to max |phi| = 1 |
| Amplitude | epsilon = 1/4 (relative modulation of p) |
| Seeds | 8 per cell, seed = 20260907000 + index; draws are `numpy.random.default_rng(seed).random((L^3,192)) < p(1+eps phi cos(k.x))` |
| Horizon | <n> stroboscopes of 48 microticks (<n*48> microticks per case); slowest predicted rate <rate> per period, e-folds covered <3 or capped> |
| Cases | 3 x 2 x 7 x 8 = 336; total microticks 336 x <n> x 48 |
| Throughput basis | [probe](evidence/strict-hydro-throughput-2026-09.json), L = <L>: <v> microticks/s |

## Observable and prediction

`m_a(n) = sum_x e^{-i k.x} sum_c w_a(c) N_c(x, 48 n)`, complex, per seed; the
report uses the seed mean and its standard error. Prediction is
`m(n) = W P(k)^n h0`, `h0 = L^3 p eps phi / 2`, computed and hashed into the
lock before execution (`predictions.json`).

## Acceptance (fixed)

Per (k, mode) cell: relative RMS deviation of the seed-mean trajectory from the
locked prediction at most 1/10; declared rate estimator (least-squares slope of
ln|m| versus n) within 2 standard errors of the seed scatter around the
predicted rate. All 42 cells pass: closure verified at this scope. Otherwise
the failing cells are the retained obstruction. No retuning.

## Execution and reproducibility

`recovery_hydro_campaign.prepare_campaign(dir, L, n)` writes preparations,
weights, locked predictions and the SHA256 lock. `run_campaign(dir)` verifies
the complete source closure, runner and inputs before and after the WSL2 GPU
run of `engine/strict/recovery_hydro/hydro_main.cpp`, which calls the accepted
CUDA `advance` unchanged. `summarize_campaign(dir)` checks receipts and writes
`report.json`. Replay directory: `engine/build_strict_hydro/campaign_v1`.
```

Replace every `<...>` with the computed value before committing; the file must contain no angle-bracket fields.

- [ ] **Step 3: Commit**

```bash
git add engine/docs/PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md
git diff --cached --stat
git commit -m "prereg(strict): hydrodynamic response campaign — locked matrix, observable, prediction, acceptance"
```

---

### Task 10: Lock, run, and summarize the campaign

**Files:**
- Create: `engine/docs/evidence/strict-hydro-response-v1.json`
- Build output (not committed): `engine/build_strict_hydro/campaign_v1/`

- [ ] **Step 1: Lock**

```powershell
$env:PYTHONPATH='scripts'
python -c "from phi_v2_lattice import recovery_hydro_campaign as C; print(C.prepare_campaign('engine/build_strict_hydro/campaign_v1', L=<L>, stroboscopes=<n>))"
```

Use the L and n from the preregistration. Expected: `case_count 336`.

- [ ] **Step 2: Run**

```powershell
python -c "from phi_v2_lattice import recovery_hydro_campaign as C; print(C.run_campaign('engine/build_strict_hydro/campaign_v1'))"
```

Expected: an execution receipt with `backend cuda_device_kernels`. If the run fails, keep the partial artifacts, fix nothing in the registration, and report.

- [ ] **Step 3: Summarize and copy the report**

```powershell
python -c "from phi_v2_lattice import recovery_hydro_campaign as C; r=C.summarize_campaign('engine/build_strict_hydro/campaign_v1'); print(r['groups_passing'], '/', r['groups'])"
Copy-Item engine/build_strict_hydro/campaign_v1/report.json engine/docs/evidence/strict-hydro-response-v1.json
```

- [ ] **Step 4: Commit the evidence**

```bash
git add engine/docs/evidence/strict-hydro-response-v1.json
git diff --cached --stat
git commit -m "evidence(strict): hydrodynamic response campaign v1 report"
```

---

### Task 11: Independent audit (reviewer, not the implementer)

**Files:**
- Create: `engine/docs/AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md`

- [ ] **Step 1: Reproduce the summary and locks**

```powershell
$env:PYTHONPATH='scripts'
python -c "from phi_v2_lattice import recovery_hydro_campaign as C; import json; C.validate_lock('engine/build_strict_hydro/campaign_v1'); r=C.summarize_campaign('engine/build_strict_hydro/campaign_v1'); print(json.dumps({k:r[k] for k in ('groups','groups_passing','trace_sha256','registration_sha256')}))"
```

- [ ] **Step 2: Replay two cases against the Python reference for one period**

```python
# run from repository root with PYTHONPATH=scripts (write as scripts/phi_v2_lattice/experiments/replay_hydro_case.py)
import hashlib, json, sys
from pathlib import Path
from phi_v2_lattice import native_codec as N_, staged as P
directory = Path("engine/build_strict_hydro/campaign_v1")
lock = json.loads((directory / "lock.json").read_text())
for row in lock["manifest"][:2]:
    cid = row["case"]["case_id"]
    state = N_.decode((directory / (cid + ".bin")).read_bytes())
    for _ in range(48):
        state, _events = P.step(state)
    replayed = hashlib.sha256(N_.encode(state)).hexdigest()
    trace = next(json.loads(l) for l in (directory / "trace.jsonl").read_text().splitlines() if json.loads(l)["case_id"] == cid)
    print(cid, replayed == trace["stroboscopes"][1]["sha256"])
```

Expected: `True` for both cases (the Python reference and the CUDA path agree on the complete state after 48 microticks). The Python step is slow at large L; if L exceeds 16, replay the first 8 microticks only and compare against a dedicated 8-tick CUDA CLI run of the same preparation (`ftd_strict_cuda_cli`), reporting which comparison was made.

- [ ] **Step 3: Write the audit**

The audit records: lock and receipt identities, the reproduced pass count, the replay comparison and its scope, the acceptance verdict per the preregistration, any defect found (returned to the implementer before acceptance), and the disposition line: closure verified at scope, or obstruction retained. Use the header form of `AUDIT_STRICT_CARRIER_RECOVERY.md`.

- [ ] **Step 4: Commit**

```bash
git add engine/docs/AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md scripts/phi_v2_lattice/experiments/replay_hydro_case.py
git diff --cached --stat
git commit -m "audit(strict): independent reproduction of the hydrodynamic response campaign"
```

---

### Task 12: Booking — sector document, wave ledger, stack pointer, LEDGER drafts

**Files:**
- Create: `engine/docs/DERIV_STRICT_HYDRODYNAMIC_SECTOR.md`
- Create: `engine/docs/PROGRAM_STRICT_RECOVERY_WAVE_3.md`
- Modify: `engine/docs/PROGRAM_STRICT_DISCRETE_STACK.md` (rows C3 and M4 of the gate table)
- Create: `docs/theory/07_assessment/core_ledgers/LEDGER_ROW_DRAFT_phi_hydrodynamics.md`

- [ ] **Step 1: Verify the next free LEDGER id**

```bash
python scripts/audit/check_registry.py
```

Record the reported next-free id in the draft file header; do not mint ids in the LEDGER itself.

- [ ] **Step 2: Write the sector document**

Sections, in order: purpose and scope (spec section 2 wording); H0 result with tag `[THEOREM — finite, exact, scoped to the field sector on the frozen background]` and the census table; H1 result with tag `[DERIVED — linearized Boltzmann (product) closure at the declared reference; correlation leakage not bounded here]`, the verdict label, and the key normalized quantities; H2 result with tag `[MEASURED]` or `[CLOSED NEGATIVE at the registered scope]` and the pass count; **Phase 2 trigger decision**: one sentence, "H1 verdict is `<label>`; per spec section 4.1 Phase 2 is `<built | not built>`", with the alternative branch named; reproduction commands for all three gates.

- [ ] **Step 3: Write the wave ledger**

Mirror `PROGRAM_STRICT_RECOVERY_WAVE_2.md`: ownership table (implementer and reviewer per gate), gate table with dispositions, evidence identities (SHA256 of the three evidence files), integrated test count and time (run `python -m pytest scripts/tests/phi_v2_lattice -q` with `FTD_STRICT_CUDA_REQUIRED=1` in an environment where the CUDA CLI is built and record the exact `N passed in T s`), and the disposition paragraph.

- [ ] **Step 4: Add the stack pointers**

In `engine/docs/PROGRAM_STRICT_DISCRETE_STACK.md`, append to the C3 row's disposition cell: `; wave 3 records the exact linear-response dispersion verdict and its registered CUDA verification ([wave 3](PROGRAM_STRICT_RECOVERY_WAVE_3.md))`, and to the M4 row: `; field-sector hydrodynamic verdict recorded in wave 3`. Change nothing else in that file.

- [ ] **Step 5: Write the LEDGER row drafts**

Three rows in the canonical four-column form `| FTD-NNNN | **question** | [TAGS] | NEW <date> — body |`, ids written as `FTD-NNNN` with the next-free id noted in the header for the owner:

1. *Which additive quantities does the staged field sector conserve, and along which schedule?* `[THEOREM — finite, exact]` — fixed kernel 1, locked kernel 7 (all l0), covariance exact, tangent in span, isotropy table; scope sentence; pointer to `strict-recovery-wave3-exact.json`.
2. *What is the long-wavelength linear-response dispersion of the staged field sector, and is it Navier–Stokes-class?* `[DERIVED — linearized Boltzmann closure at the declared reference]` — verdict label, `s/|n|^2` values, transverse/longitudinal normalized quantities, certified statuses per p; the sentence "this is a prediction for H2, not a theorem about Phi"; no Clay claim.
3. *Does the linearized Boltzmann period map describe the staged law's actual stroboscopic response?* `[MEASURED]` or `[CLOSED NEGATIVE at the registered scope]` — pass count, L, horizon, acceptance band, audit pointer.

Each row ends with the Phase 2 trigger sentence from the sector document.

- [ ] **Step 6: Commit**

```bash
git add engine/docs/DERIV_STRICT_HYDRODYNAMIC_SECTOR.md engine/docs/PROGRAM_STRICT_RECOVERY_WAVE_3.md engine/docs/PROGRAM_STRICT_DISCRETE_STACK.md docs/theory/07_assessment/core_ledgers/LEDGER_ROW_DRAFT_phi_hydrodynamics.md
git diff --cached --stat
git commit -m "docs(strict): wave-3 booking — hydrodynamic sector document, wave ledger, stack pointers, LEDGER row drafts"
```

---

## Verification before hand-back

- [ ] `python -m pytest scripts/tests/phi_v2_lattice -q` passes with no new skips when `FTD_STRICT_CUDA_REQUIRED=1` and the CUDA CLI and hydro runner are built; record the count.
- [ ] `python engine/strict/generate_tables.py --check` still reports the frozen header unchanged.
- [ ] `git diff main --stat` shows only the files listed in the File Structure plus the two plan/spec files.
- [ ] Every document states its tag exactly as the Global Constraints list; grep for `THEOREM` in the new documents and confirm each occurrence is H0's scoped statement.
- [ ] The sector document's Phase 2 trigger sentence is present and matches the evidence JSON's `verdict.exact.label`.
