"""Preparations for the actual compiled staged lattice, with test provenance.

Tests remain verification code. Only explicitly registered, law-compatible
finite states enter the worker; analytical scores and successor laws do not.
"""
from __future__ import annotations

import ast
from dataclasses import asdict, dataclass
from functools import lru_cache
from hashlib import sha256
from pathlib import Path

import numpy as np

from . import channels as C, geometry as G, native_codec as N, prepare as A
from . import staged as P, state as S
from ._proofs import encode
from . import recovery_carriers as R, recovery_mixed_response as M, recovery_mixed_scattering as X

ROOT = Path(__file__).resolve().parents[2]
SIZES = (3, 4, 7, 9)


@dataclass(frozen=True)
class Scenario:
    id: str
    title: str
    family: str
    sizes: tuple[int, ...]
    source: str
    tests: tuple[str, ...]
    observation: str = "field_tokens"
    law_id: str = P.LAW_ID
    status: str = "selected_finite_preparation"


@lru_cache(maxsize=1)
def scenarios():
    basic = (
        ("empty", "Empty record control", "state.py:blank", "test_state.py", "field_tokens"),
        ("relation", "One relation token", "prepare.py:isolated_relation", "test_prepare.py", "relation_tokens"),
        ("sparse", "Sparse records", "prepare.py:sparse_material", "test_native_parity.py", "field_tokens"),
        ("r5", "R5 stationary-background preparation", "prepare.py:r5_vacuum", "test_wasm_parity.py", "field_tokens"),
        ("seam", "SC/FCC seam and lagged manifestation", "test_native_parity.py:preparation", "test_native_parity.py", "relation_tokens"),
        ("witness-control", "Causal witness · control", "test_staged.py:test_baseline_distance_two_witness_respects_elapsed_candidate_ticks", "test_staged.py", "field_tokens"),
        ("witness", "Causal witness · intervention", "test_native_parity.py:preparation", "test_causal_witness.py", "field_tokens"),
        ("expiry-a", "Expiry presentation · A", "test_staged.py:test_expiry_erases_distinct_channel_presentations_from_complete_state", "test_staged.py", "field_tokens"),
        ("expiry-b", "Expiry presentation · B", "test_staged.py:test_expiry_erases_distinct_channel_presentations_from_complete_state", "test_staged.py", "field_tokens"),
    )
    rows = [Scenario(key, title, "Core and regression controls", SIZES, source, (test,), observation)
            for key, title, source, test, observation in basic]
    for case in R.cases():
        title = (f"{case.family} · orientation {case.orientation} · polarity {case.polarity:+d}"
                 f" · phase {case.phase} · slot {case.slot} · placement {case.placement}"
                 f" · layer {case.layer} · second polarity {case.second_polarity:+d}")
        rows.append(Scenario(case.case_id, title, "Registered carrier controls", (R.L,),
                             "recovery_carriers.py:prepare", ("test_recovery_carriers.py",),
                             "relation_tokens" if case.family == "relation" else "field_tokens"))
    for module, family, test in ((M, "Registered mixed response", "test_recovery_mixed_response.py"),
                                 (X, "Registered mixed scattering", "test_recovery_mixed_scattering.py")):
        for case in module.cases():
            rows.append(Scenario(case.case_id, f"{case.defect} · {'probe' if case.probe else 'control'}",
                                 family, (module.L,), module.__name__.split('.')[-1]+".py:prepare", (test,),
                                 "field_tokens" if case.probe else "relation_tokens"))
    if len({row.id for row in rows}) != len(rows):
        raise ValueError("duplicate lattice scenario identifier")
    return tuple(rows)


def prepare(scenario_id, size):
    spec = next((row for row in scenarios() if row.id == scenario_id), None)
    if spec is None or type(size) is not int or size not in spec.sizes:
        raise ValueError("unregistered scenario or lattice size")
    for module in (R, M, X):
        case = next((case for case in module.cases() if case.case_id == scenario_id), None)
        if case is not None:
            state = module.prepare(case)
            P.validate(state)
            return state
    if scenario_id == "relation":
        lattice = A.isolated_relation(size)
    elif scenario_id == "sparse":
        lattice = A.sparse_material(size, seed=11, n_tokens=12, field_occupation=.005)
    elif scenario_id == "r5":
        lattice = A.r5_vacuum(size, seed=11)
    else:
        lattice = S.blank(size)
        if scenario_id == "seam":
            lattice.s[:] = np.arange(size**3) % 3-1
            lattice.ell[:] = np.arange(size**3) % 3
            owner = G.site_index(size, size-1, size-1, size-1)
            lattice.sc[owner, 0, 1] = S.idx_of(encode(0, -1))
            lattice.fcc[owner, 2, 0, 0] = S.idx_of(encode(0, 1))
            lattice.fcc[owner, 2, 1, 1] = S.idx_of(encode(0, -1))
            lattice.bank[owner, [0, 34, 194]] = True
        elif scenario_id in ("witness", "witness-control"):
            owner = G.site_index(size, 1, 1, 1)
            lattice.bank[owner, [0, 34]] = True
            if scenario_id == "witness":
                lattice.bank[G.shift(size, owner, (1, 0, 0)), 2] = True
        elif scenario_id in ("expiry-a", "expiry-b"):
            channels = [c for c in range(192) if C.phase(c) == 2 and C.tangent(c) == (1, 0, 0)]
            lattice.bank[0, channels[int(scenario_id == "expiry-b")]] = True
    return P.initialize(lattice)


def checkpoint(scenario_id, size):
    data = N.encode(prepare(scenario_id, size))
    return data, sha256(data).hexdigest()


def test_inventory(root=ROOT):
    """AST-only inventory: do not import or execute tests while serving a page.

    Law references describe dependencies, not a passing result or admission.
    Every test module is listed, including optional historical and backend tests.
    """
    package = root/"scripts/phi_v2_lattice"
    modules = {"phi_v2_lattice."+str(p.relative_to(package).with_suffix("")).replace("\\", ".").replace("/", "."): p
               for p in package.rglob("*.py")}
    dependencies, laws = {}, {}

    def inspect(path, module):
        tree = ast.parse(path.read_text(encoding="utf-8-sig"))
        refs, ids = set(), set()
        for node in ast.walk(tree):
            if isinstance(node, ast.Assign) and any(isinstance(t, ast.Name) and t.id == "LAW_ID" for t in node.targets):
                if isinstance(node.value, ast.Constant) and isinstance(node.value.value, str):
                    ids.add(node.value.value)
            if isinstance(node, ast.ImportFrom):
                prefix = node.module or ""
                if node.level:
                    prefix = ".".join(module.split(".")[:-node.level]+([prefix] if prefix else []))
                prefix = prefix.removeprefix("scripts.")
                if prefix.startswith("phi_v2_lattice"):
                    refs.add(prefix)
                    refs.update(prefix+"."+alias.name for alias in node.names)
            elif isinstance(node, ast.Import):
                refs.update(alias.name.removeprefix("scripts.") for alias in node.names)
        return tree, refs & modules.keys(), ids

    for module, path in modules.items():
        _, dependencies[module], laws[module] = inspect(path, module)

    def all_laws(refs):
        seen, result, queue = set(), set(), list(refs)
        while queue:
            ref = queue.pop()
            if ref in seen:
                continue
            seen.add(ref)
            result.update(laws[ref])
            queue.extend(dependencies[ref]-seen)
        return sorted(result)

    rows = []
    tests = root/"scripts/tests/phi_v2_lattice"
    for path in sorted(tests.rglob("test_*.py")):
        relative = path.relative_to(tests).as_posix()
        tree, refs, _ = inspect(path, "tests."+relative.removesuffix(".py").replace("/", "."))
        ids = all_laws(refs)
        linked = [row.id for row in scenarios() if relative in row.tests]
        rows.append({"module": relative, "test_functions": sum(isinstance(n, (ast.FunctionDef, ast.AsyncFunctionDef))
                                                              and n.name.startswith("test_") for n in ast.walk(tree)),
                     "law_references": ids, "scenario_ids": linked,
                     "disposition": "playable_fixtures_and_offline_checks" if linked else
                                    "other_law_or_analysis" if any(law != P.LAW_ID for law in ids) else "offline_check",
                     "execution_status": "not_run_by_catalog",
                     "command": "python -m pytest scripts/tests/phi_v2_lattice/"+relative+" -q"})
    return rows


def catalog():
    return {"schema": "web-lattice-record-catalog-1", "law_id": P.LAW_ID,
            "collision_hash": C.COLLISION_HASH, "canonical_adoption": False,
            "scenarios": [asdict(row) for row in scenarios()], "tests": test_inventory(),
            "backend_checks": [
                {"path": "engine/strict/native_selftest.cpp", "command": "ctest --test-dir engine/build_strict_native -j 24 --output-on-failure"},
                {"path": "engine/web/tests/strict-runtime-worker.node.test.mjs", "command": "node --test engine/web/tests/strict-runtime-worker.node.test.mjs"},
                {"path": "engine/web/tests/finite-record-frame.node.test.mjs", "command": "node --test engine/web/tests/finite-record-frame.node.test.mjs"},
                {"path": "engine/web/tests/record-lattice.spec.js", "command": "cd engine/web/tests && npx playwright test --config playwright.record-lattice.config.js"},
                {"path": "engine/web/tests/test_record_routes.py", "command": "python -m pytest engine/web/tests/test_record_routes.py -q"},
            ],
            "excluded_from_this_runtime": [
                {"family": "Staged H2 hydrodynamic campaign", "reason": "The registered 336-case L=32 campaign exceeds this bounded L<=17 interactive profile. Its preparations and test module remain in the offline campaign; shrinking them would change the registration."},
                {"family": "Parameterized transport, pair-sector and arbitrary-bank helpers", "reason": "Verification helpers admit additional parameter choices, not a finite scenario catalog. Their complete test modules remain listed below; no exhaustive trajectory coverage is implied by the interactive controls."},
                {"family": "Thermal candidates 1 and 2", "reason": "Different six-word Boolean bank and collision law; no compatible staged-worker codec. Finite-wavelength scores are analytical perturbations, not finite checkpoints."},
                {"family": "Hydro successor", "reason": "Different collision table and law; use its compiled fluid laboratory.", "url": "/strict/web/hydro/"},
                {"family": "Alignment, routing, matching, credit-exchange and parity successors", "reason": "Distinct complete records/laws. Their tests do not authorize conversion into staged Phi-v2 states."},
                {"family": "Proofs, fitted comparisons, malformed-state and historical-receipt tests", "reason": "Offline verification rather than valid evolving preparations; no test result is inferred from loading."},
            ]}
