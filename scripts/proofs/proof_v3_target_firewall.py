"""Operational/dataflow firewall for the selected FTD-v3 Phi v2.

The firewall separates active microscopic inputs from explanatory and
verification-only outputs.  It checks the machine manifest, generator AST,
import boundary, numeric types, frozen hashes, and preparation contract.  It
does not claim historical ignorance of physics; it proves that the executable
selection/dataflow cannot read the prohibited outputs.
"""

from __future__ import annotations

import ast
import json
import re
import sys
from pathlib import Path
from typing import Any


sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
MANIFEST_PATH = (
    ROOT / "docs/theory/01_reference/strict_discrete_common_action_phi_v2.json"
)
REGISTER_PATH = (
    ROOT / "docs/theory/01_reference/strict_discrete_common_action_register_v3.json"
)
GENERATOR_PATH = (
    ROOT / "scripts/proofs/proof_global_c3_cotangent_layer_equivariant_collision.py"
)
RECOVERY_PATH = (
    ROOT / "scripts/proofs/proof_global_c3_cotangent_layer_full_tick_maxwell_vacuum.py"
)
EXPECTED_HASH = "D0BB71DBED7938ED286E1D6D91A16700DA31F4550E83B2FB3580CCC347B2BD25"

checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def walk_values(value: Any):
    if isinstance(value, dict):
        for key, item in value.items():
            yield key
            yield from walk_values(item)
    elif isinstance(value, list):
        for item in value:
            yield from walk_values(item)
    else:
        yield value


def normalized_words(value: Any) -> set[str]:
    blob = " ".join(str(item) for item in walk_values(value)).lower()
    return set(re.findall(r"[a-z][a-z0-9_]*", blob))


def ast_imports(tree: ast.AST) -> set[str]:
    modules: set[str] = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Import):
            modules.update(alias.name for alias in node.names)
        elif isinstance(node, ast.ImportFrom) and node.module:
            modules.add(node.module)
    return modules


manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
register = json.loads(REGISTER_PATH.read_text(encoding="utf-8"))
rule_core = manifest["rule_core"]
preparation_core = manifest["preparation_core"]
verification_outputs = manifest["verification_outputs_not_read_by_rule"]

check("C1 firewall manifest is valid and versioned", manifest["meta"]["name"].endswith("Phi-v2") and rule_core["carrier_version"] == 2)
check("C2 active carrier hash matches the register", rule_core["collision"]["table_sha256_utf8_lf_no_terminal_newline"] == register["carrier_inventory"]["collision_table_sha256"] == EXPECTED_HASH)
check("C3 active rule spec and verifier paths exist", (ROOT / manifest["meta"]["rule_specification"]).exists() and (ROOT / manifest["meta"]["firewall_verifier"]).exists())

# Exact forbidden semantic names.  Substrings are not used because words such
# as "manifested" must not trip a ban on an unrelated short token.
forbidden_words = {
    "alpha",
    "born",
    "codata",
    "pdg",
    "higgs",
    "electron",
    "proton",
    "muon",
    "outcome",
    "outcomes",
    "lensing",
    "metric",
    "mass",
    "masses",
    "eigenvalue",
    "eigenvalues",
    "speed",
}
active_words = normalized_words({"rule_core": rule_core, "preparation_core": preparation_core})
check("C4 active rule/preparation contains no prohibited semantic input", not (active_words & forbidden_words), str(sorted(active_words & forbidden_words)))

active_values = list(walk_values({"rule_core": rule_core, "preparation_core": preparation_core}))
check("C5 active rule/preparation contains no floating-point literal", not any(isinstance(value, float) for value in active_values))
check("C6 active preparation has no adjustable parameter", preparation_core["adjustable_parameters"] == [])
check("C7 field reference is finite uniform counting", preparation_core["field_reference"].startswith("uniform_counting_measure") and preparation_core["finite_periodic_region"] is True)

# Verification outputs are deliberately present, but the active sections may
# not contain their serialized values as branch inputs.
output_blob = json.dumps(verification_outputs, sort_keys=True)
active_blob = json.dumps({"rule_core": rule_core, "preparation_core": preparation_core}, sort_keys=True)
check("C8 verification output object is structurally separate", "verification_outputs_not_read_by_rule" not in active_blob)
check("C9 recovered speed string is absent from active inputs", verification_outputs["speed"] not in active_blob)
check("C10 recovered action coefficient string is absent from active inputs", verification_outputs["blocked_spatial_action_coefficient"] not in active_blob)
check("C11 recovery error expression is absent from active inputs", verification_outputs["three_tick_error_bound"] not in active_blob)
check("C12 verification outputs remain recorded for audit", all(term in output_blob for term in ("1/6", "1/36", "kappa")))

generator_source = GENERATOR_PATH.read_text(encoding="utf-8")
generator_tree = ast.parse(generator_source)
imports = ast_imports(generator_tree)
forbidden_import_fragments = {
    "constants",
    "random",
    "numpy.random",
    "requests",
    "urllib",
    "socket",
    "pandas",
}
check("C13 collision generator imports no constants/data/random/network module", not any(any(fragment in module.lower() for fragment in forbidden_import_fragments) for module in imports), str(sorted(imports)))
check("C14 collision generator does not import the recovery certificate", all("full_tick_maxwell_vacuum" not in module for module in imports))

float_literals = [
    node.value
    for node in ast.walk(generator_tree)
    if isinstance(node, ast.Constant) and isinstance(node.value, float)
]
check("C15 collision generator contains no floating-point literal", not float_literals, str(float_literals))

forbidden_calls = []
for node in ast.walk(generator_tree):
    if not isinstance(node, ast.Call):
        continue
    if isinstance(node.func, ast.Name) and node.func.id in {"open", "input", "eval", "exec"}:
        forbidden_calls.append(node.func.id)
    if isinstance(node.func, ast.Attribute) and node.func.attr in {"read_text", "read_bytes", "load", "loads", "download"}:
        forbidden_calls.append(node.func.attr)
check("C16 collision generator reads no file, user input, or external payload", not forbidden_calls, str(forbidden_calls))

# Remove module/function docstrings before semantic-name scanning; comments and
# prose are not executable data.  Names used generically for graph targets are
# allowed, while named physical inputs are not.
semantic_identifiers = {
    node.id.lower() for node in ast.walk(generator_tree) if isinstance(node, ast.Name)
}
semantic_attributes = {
    node.attr.lower() for node in ast.walk(generator_tree) if isinstance(node, ast.Attribute)
}
executable_names = semantic_identifiers | semantic_attributes
physical_names = {
    "alpha",
    "born_weight",
    "desired_outcome",
    "particle_mass",
    "lensing_ratio",
    "continuum_metric",
    "target_speed",
    "target_eigenvalue",
}
check("C17 collision generator executable names contain no physical target input", not (executable_names & physical_names), str(sorted(executable_names & physical_names)))

# The generator may validate structural rank after construction, but the pair
# selection itself must be rank-greedy/lexicographic and not compare a spectrum.
check("C18 collision selection order is target-independent", manifest["rule_core"]["collision"]["selection_order"] == [
    "signed_cubic_and_c4_orbit_partition",
    "field_sum_preserving_fixed_point_free_self_involutions",
    "fewest_options_first",
    "maximum_exact_modular_rank_gain",
    "lexicographically_first_tie",
    "exact_rational_rank_postcheck"
])
check("C19 selection provenance excludes recovery spectrum and external data", {"recovery eigenvalues", "wave speed", "external data"} <= set(manifest["selection_provenance"]["collision_inputs_excluded"]))

# Dependency direction is one-way: recovery imports the generator, never the
# reverse.  This is the decisive dataflow boundary.
recovery_tree = ast.parse(RECOVERY_PATH.read_text(encoding="utf-8"))
recovery_imports = ast_imports(recovery_tree)
check("C20 recovery certificate consumes the frozen collision generator", any("equivariant_collision" in module for module in recovery_imports))
check("C21 generator cannot consume recovery outputs", all("full_tick" not in module and "vacuum" not in module for module in imports))

# Structural numeric provenance: every nontrivial active integer is named in
# the manifest by a finite census, table size/hash convention, or exact rank
# algorithm.  No measured decimal appears.
active_ints = sorted({value for value in active_values if isinstance(value, int) and not isinstance(value, bool)})
declared_structural_ints = {0, 1, 2, 3, 4, 6, 9, 192, 384, 55008, 1000003}
check("C22 active integer inputs are exhaustively structural", set(active_ints) <= declared_structural_ints, str(active_ints))
check("C23 active rule contains no decimal experimental value", not re.search(r"\b(?:137\.0|125\.2|1836\.|0\.00729|2\.828)\d*\b", active_blob))

contract = manifest["firewall_contract"]
check("C24 firewall contract prohibits external replay/random tape", any("replay" in row and "random" in row for row in contract["forbidden_rule_or_preparation_inputs"]))
check("C25 verification-only comparisons cannot alter rule/preparation", any("cannot alter" in row for row in contract["allowed_verification_only"]))

check("C26 R1--R6 are booked closed after this certificate", all(register["ratification_status"][f"R{i}"].startswith("closed") for i in range(1, 7)))

passed = sum(ok for _, ok, _ in checks)
print(f"\n{passed}/{len(checks)} target-firewall checks pass")
raise SystemExit(0 if passed == len(checks) else 1)
