import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))   # makes `phi_v2_lattice` importable


# These tests certify retained local runs or exact historical source closures.
# Their prerequisites and frozen pins remain mandatory when explicitly enabled.
_HISTORICAL_MODULES = {
    "test_composite_interaction_execution_v1.py":
        "historical composite source closure, retained catalogue and qualified native build",
    "test_sparse_q4_native_v1.py":
        "historical topology/source pins and retained native audit/build evidence",
}
_HISTORICAL_FUNCTIONS = {
    "test_recovery_wave7_evidence.py": {
        "test_frozen_census_reduces_every_retained_count",
        "test_rehashed_census_cannot_change_observable_or_law",
        "test_rehashed_census_cannot_omit_a_completed_range",
        "test_rehashed_range_cannot_shorten_actual_coverage",
    },
    "test_full_memory_execution_v2.py": {
        "test_all_original_failed_attempt_bytes_and_longer_native_prefixes_are_retained",
    },
    "test_recovery_full_memory.py": {
        "test_complete_local_tables_match_frozen_independent_actual_count",
        "test_same_SC_pair_is_retained_in_joint_table",
        "test_admission_preserves_every_accepted_and_prerequisite_pin",
    },
    "test_recovery_momentum_memory.py": {
        "test_frozen_inputs_and_registration",
        "test_registered_expectation_not_used_to_construct_result",
        "test_certificate_exact_scope_and_no_unearned_recovery_claim",
    },
}
_HISTORICAL_FIXTURES = {
    "test_recovery_composite_interaction.py": "catalogue",
    "test_full_memory_execution_v2.py": "synthetic_admission",
}


def pytest_addoption(parser):
    parser.getgroup("FTD retained evidence").addoption(
        "--retained-evidence", action="store_true", default=False,
        help="collect retained-run/historical-identity tests; all local prerequisites still apply")


def pytest_configure(config):
    config.addinivalue_line("markers", "retained_evidence: requires retained local runs or historical source identities")
    config._ftd_retained_exclusions = {}


def _relative(path):
    return Path(path).relative_to(Path(__file__).parent).as_posix()


def pytest_ignore_collect(collection_path, config):
    try:
        name = _relative(collection_path)
    except ValueError:
        return None
    if name in _HISTORICAL_MODULES and not config.getoption("--retained-evidence"):
        config._ftd_retained_exclusions[name] = ("module", _HISTORICAL_MODULES[name])
        return True
    return None


@pytest.hookimpl(tryfirst=True)
def pytest_pycollect_makemodule(module_path, parent):
    # Explicit file arguments can bypass pytest_ignore_collect. Reject before
    # importing a historical module with missing or incompatible source pins.
    try:
        name = _relative(module_path)
    except ValueError:
        return None
    if name in _HISTORICAL_MODULES and not parent.config.getoption("--retained-evidence"):
        raise pytest.UsageError(f"{name} requires --retained-evidence: {_HISTORICAL_MODULES[name]}")
    return None


def pytest_collection_modifyitems(config, items):
    kept, excluded = [], []
    for item in items:
        try:
            name = _relative(item.path)
        except ValueError:
            kept.append(item)
            continue
        reason = _HISTORICAL_MODULES.get(name)
        function = getattr(item, "originalname", None) or item.name.split("[", 1)[0]
        if function in _HISTORICAL_FUNCTIONS.get(name, ()):
            reason = "retained census/attempt artifacts or frozen historical source registration"
        fixture = _HISTORICAL_FIXTURES.get(name)
        if fixture and fixture in item.fixturenames:
            reason = f"fixture {fixture} requires retained catalogue/admission artifacts"
        if reason:
            item.add_marker(pytest.mark.retained_evidence)
        if reason and not config.getoption("--retained-evidence"):
            excluded.append(item)
            config._ftd_retained_exclusions[item.nodeid] = ("item", reason)
        else:
            kept.append(item)
    items[:] = kept
    if excluded:
        config.hook.pytest_deselected(items=excluded)


def pytest_terminal_summary(terminalreporter, exitstatus, config):
    excluded = config._ftd_retained_exclusions
    if excluded:
        modules = sum(kind == "module" for kind, _ in excluded.values())
        terminalreporter.section("retained-evidence collection boundary")
        terminalreporter.write_line(
            f"Excluded {modules} named modules and {len(excluded)-modules} items; "
            "opt in with `pytest scripts/tests/phi_v2_lattice --retained-evidence` "
            "(prerequisites remain mandatory).")
        groups = {}
        for name, (kind, reason) in excluded.items():
            key = (name.split("::", 1)[0], kind, reason)
            groups[key] = groups.get(key, 0) + 1
        for (name, kind, reason), count in sorted(groups.items()):
            terminalreporter.write_line(f"  {name}: {count} {kind}(s): {reason}")
