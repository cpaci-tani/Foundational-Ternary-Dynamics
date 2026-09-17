"""Generate the browser-facing v2 finite-seed schema catalog.

Emits one ``describe()`` schema per registered (scenario_id, size) combination
(656 total) plus a category-base schema at each supported size, to
``engine/web/js/seeding/generated/finite-seeds.json``. Use ``--check`` in
focused verification to reject drift between the registries and the
generated file. All default constructor data lives in
``scripts/phi_v2_lattice/web_seed_presets.py``; this script only serializes
it. No physics executes here beyond the deterministic recipe compilation
already covered by the module's own tests.
"""
import argparse
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from phi_v2_lattice import web_seed_presets as WP  # noqa: E402

OUTPUT = ROOT / "engine/web/js/seeding/generated/finite-seeds.json"


def catalog():
    schemas = {}
    for scenario_id, size in WP.all_ids_and_sizes():
        description = WP.describe(scenario_id, size)
        schemas[f"{description['scenarioId']}@{size}"] = description
    for size in WP.SIZES:
        base = WP.describe_base(size)
        schemas[f"{base['scenarioId']}@{size}"] = base
    templates = {str(size): kinds for size, kinds in WP.all_component_templates().items()}
    return {"schemaVersion": 2, "schemaIdentity": WP.SCHEMA_IDENTITY,
            "generator": "scripts/verification/generate_finite_seed_presets.py",
            "source": "scripts/phi_v2_lattice/web_seed_presets.py",
            "count": len(WP.all_ids_and_sizes()),
            "componentLibrary": WP.component_library(),
            "componentTemplates": templates,
            "schemas": schemas}


def serialized():
    data = catalog()
    schemas = data.pop("schemas")
    head = json.dumps(data, ensure_ascii=False, indent=2)
    body = ",\n".join('  "%s": %s' % (key, json.dumps(schemas[key], ensure_ascii=False))
                       for key in schemas)
    return head[:-2] + ',\n  "schemas": {\n' + body + "\n  }\n}\n"


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    text = serialized()
    if args.check:
        if not OUTPUT.is_file() or OUTPUT.read_text(encoding="utf-8") != text:
            raise SystemExit("Finite-seed presets are stale; regenerate with this script.")
        print("Finite-seed presets match every registered (scenario, size) combination.")
    else:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_text(text, encoding="utf-8", newline="\n")
        print(OUTPUT)
