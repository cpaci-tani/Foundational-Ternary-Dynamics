"""Generate the finite-seed property-effect coverage map; no physics execution.

Renders ``COVERAGE_MAP`` from ``scripts/tests/phi_v2_lattice/test_web_seed_property_effects.py``
(the single source of truth) to JSON. A row's presence here is not a
certification of physical correctness -- it records which named test proves
that editing a given (kind, property) / region key / activation flag / recipe
field actually changes the compiled finite record, and how (exact value,
boundary, or set/clear). Use --check in focused verification.
"""
import argparse
import importlib.util
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

from phi_v2_lattice import web_seed_presets as WP  # noqa: E402

_TEST_MODULE_PATH = ROOT / "scripts" / "tests" / "phi_v2_lattice" / "test_web_seed_property_effects.py"
_spec = importlib.util.spec_from_file_location("_finite_seed_effects_tests", _TEST_MODULE_PATH)
_tests = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_tests)
COVERAGE_MAP = _tests.COVERAGE_MAP

OUTPUT = ROOT / "engine/config/finite_seed_effects.json"


def _rows():
    return [dict(category=cat, kind=kind, key=key, test=test)
            for cat, kind, key, test in COVERAGE_MAP]


def build():
    rows = _rows()
    return dict(
        schemaVersion=1,
        status="binding-effect-coverage",
        testModule="scripts/tests/phi_v2_lattice/test_web_seed_property_effects.py",
        boundaries=[
            "A row proves one named pytest asserts a real, exact effect on the compiled "
            "record (set/clear, exact channel/slot/value, or boundary), not merely a "
            "changed hash or count.",
            "Coverage is keyed to web_seed_presets.PROPERTY_META / COLLECTION_META / "
            "REGION_META / KINDS as of generation time; test_coverage_map_matches_every_"
            "registered_binding fails closed if a new binding is added without a row.",
            "This is software perturbation-testing evidence, not a physics claim.",
        ],
        countKinds=len(WP.KINDS),
        countPropertyBindings=len({(k, key) for c, k, key, _t in COVERAGE_MAP if c == "property"}),
        countCollectionBindings=len({(k, key) for c, k, key, _t in COVERAGE_MAP if c == "collection"}),
        countRegionBindings=len({key for c, _k, key, _t in COVERAGE_MAP if c == "region"}),
        countRecipeBindings=len({key for c, _k, key, _t in COVERAGE_MAP if c == "recipe"}),
        countActivationBindings=len({k for c, k, key, _t in COVERAGE_MAP if c == "activation"}),
        rows=rows,
    )


def serialized():
    data = build()
    rows = data.pop("rows")
    head = json.dumps(data, ensure_ascii=False, indent=2)
    return head[:-2] + ',\n  "rows": [\n' + ",\n".join(
        "    " + json.dumps(row, ensure_ascii=False) for row in rows) + "\n  ]\n}\n"


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    text = serialized()
    if args.check:
        if not OUTPUT.is_file() or OUTPUT.read_text(encoding="utf-8") != text:
            raise SystemExit("finite_seed_effects.json is stale; regenerate with this script.")
        print("finite_seed_effects.json matches the live coverage map.")
    else:
        OUTPUT.write_text(text, encoding="utf-8", newline="\n")
        print(OUTPUT)
