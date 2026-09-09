"""Restore exact discovery order after the pinned pytest fixture permutation.

The outer coordinator explicitly selects this plugin for its top-level run.
The frozen regression component and all test sources remain unchanged.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path

import pytest
from _pytest import fixtures
from scripts.phi_v2_lattice import recovery_regression as R

pytest_plugins = ("scripts.phi_v2_lattice.recovery_regression",)
FIXTURE_SHA256 = "eff80aa15502a5fa38b022f2dd3d3ba760efff16b9c761e7283299bbcf380de6"
_DISCOVERED = []


def pytest_sessionstart(session):
    _DISCOVERED.clear()


def pytest_itemcollected(item):
    _DISCOVERED.append(item)


def restore_order(discovered, items, reorder, fixture_path, fixture_sha256):
    """Pure list validation/restoration plus an explicit source-byte precheck.

    Returns the restored item list and serializable complete order evidence.
    Does not modify either supplied list, item records, fixtures or node IDs.
    """
    before = list(items)
    original = list(discovered)
    evidence = {"schema": "strict-regression-collection-order-1", "status": "REJECTED",
                "discovered": [item.nodeid for item in original],
                "core_order": [item.nodeid for item in before],
                "expected_core_order": [], "restored_order": [],
                "fixture_path": str(Path(fixture_path).resolve()),
                "fixture_sha256": fixture_sha256,
                "scientific_completion_evaluated": False, "canonical_adoption": False}
    try:
        if hashlib.sha256(Path(fixture_path).read_bytes()).hexdigest() != fixture_sha256:
            raise ValueError("pinned pytest fixture source changed")
        nodes = evidence["discovered"]
        if (not original or any(type(node) is not str or not node for node in nodes)
                or len(set(nodes)) != len(nodes) or len({id(item) for item in original}) != len(original)):
            raise ValueError("discovery is not a nonempty unique node/object inventory")
        if (len(before) != len(original) or len({id(item) for item in before}) != len(before)
                or {id(item) for item in before} != {id(item) for item in original}
                or len({item.nodeid for item in before}) != len(before)):
            raise ValueError("core collection is not the exact discovered object/node permutation")
        expected = list(reorder(list(original)))
        evidence["expected_core_order"] = [item.nodeid for item in expected]
        if ([id(item) for item in before] != [id(item) for item in expected]
                or {id(item) for item in expected} != {id(item) for item in original}):
            raise ValueError("unexpected collection permutation beyond pinned fixture regrouping")
        if hashlib.sha256(Path(fixture_path).read_bytes()).hexdigest() != fixture_sha256:
            raise ValueError("pytest fixture source changed during permutation validation")
        evidence.update(status="PASS", restored_order=list(nodes),
                        changed_positions=sum(a is not b for a, b in zip(before, original)))
        return original, evidence
    except BaseException as error:
        evidence["error"] = type(error).__name__ + ": " + str(error)
        error.order_evidence = evidence
        raise


@pytest.hookimpl(trylast=True)
def pytest_collection_modifyitems(session, config, items):
    output = os.environ.get(R.PREFIX + "OUTPUT")
    if output is None:
        raise ValueError("collection-order adapter requires the accepted regression output")
    destination = Path(output) / "collection-order.json"
    try:
        restored, evidence = restore_order(_DISCOVERED, items, fixtures.reorder_items,
                                           fixtures.__file__, FIXTURE_SHA256)
    except BaseException as error:
        if hasattr(error, "order_evidence"):
            R._write(destination, error.order_evidence)
        raise
    R._write(destination, evidence)
    items[:] = restored
