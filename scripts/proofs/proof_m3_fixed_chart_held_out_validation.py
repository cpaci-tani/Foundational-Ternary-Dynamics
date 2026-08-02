"""Independent artifact certificate for FTD-0758 held-out validation.

The frozen FTD-0758 schema and verdict map are identical to FTD-0755 except
for identifiers and paths.  This certificate loads only the definition prefix
of the hash-locked FTD-0755 certificate, substitutes those identities, and
then applies the inherited checks to the fresh artifact corpus.  It never runs
or repairs dynamics.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_CERTIFICATE = ROOT / "scripts" / "proofs" / "proof_m3_support_invariant_validation.py"
PARENT_CERTIFICATE_HASH = "6C5C4A946C672DD5FFABCBA62F45DFB25AF1A32A8CF788D3F227BAF4F85A9AD4"
PARENT_PROTOCOL_HASH = "1E713DB4B997DAED0D55F098A6E7D63FC0F2D773391CE44FFE03AADD92A504BC"
PROTOCOL_HASH = "16289A34CC23ED39A7EEC2E9479E8C7BC6666ABE284FB08BF69FA58931F17765"
RUNNER_HASH = "B0828EAC7A3FBCA00E10B8180E784C95E860432EA4326E0C1B567A1BD2D1A789"


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest().upper()


if sha256(PARENT_CERTIFICATE) != PARENT_CERTIFICATE_HASH:
    raise SystemExit("FTD-0758 refused: inherited FTD-0755 certificate hash drift")

source = PARENT_CERTIFICATE.read_text(encoding="utf-8")
prefix, marker, _ = source.partition("parser = argparse.ArgumentParser()")
if not marker:
    raise SystemExit("FTD-0758 refused: inherited certificate structure drift")
prefix = prefix.replace("FTD-0755", "FTD-0758")
prefix = prefix.replace("ftd_0755", "ftd_0758")
prefix = prefix.replace(PARENT_PROTOCOL_HASH, PROTOCOL_HASH)
scope = {"__file__": str(Path(__file__).resolve()), "__name__": "ftd0758_certificate"}
exec(compile(prefix, str(PARENT_CERTIFICATE), "exec"), scope)

scope["EXPECTED_HASHES"] = {
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_M3_FIXED_CHART_HELD_OUT_VALIDATION_v1.md": PROTOCOL_HASH,
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_M3_SUPPORT_INVARIANT_VALIDATION_v1.md": PARENT_PROTOCOL_HASH,
    "engine/include/ftd/eft/support_invariant_matter_predicate.h":
        "B11E087E2E7E16375C173185233AD001AB8B9F049E9B9B5A3156D8618CB4F104",
    "engine/src/eft/support_invariant_matter_predicate.cpp":
        "752CE7C3B03A9944C1E7016A62CCA584FAC868EF191D8241ACEE7E6C9C550D21",
    "engine/tests/campaign_m3_fixed_chart_held_out_validation_cuda.cpp":
        RUNNER_HASH,
    "scripts/proofs/proof_m3_support_invariant_validation.py":
        PARENT_CERTIFICATE_HASH,
}
scope["checks"] = 0
scope["failures"] = []
scope["exact_hashes"]()

parser = argparse.ArgumentParser()
parser.add_argument("--preflight", action="store_true")
args = parser.parse_args()
if args.preflight:
    scope["check"]("registered result directory absent",
                   not scope["RESULTS"].exists())
    mode = "preflight"
    verdict = "NOT_RUN"
else:
    scope["check"]("registered result directory exists",
                   scope["RESULTS"].is_dir())
    candidates = [scope["certify_candidate"](arm, variant)
                  for arm in scope["DIRECTIONS"]
                  for variant in scope["VARIANTS"]]
    fibres = [scope["certify_fibre"](arm) for arm in scope["DIRECTIONS"]]
    if not all(item["infrastructure"] for item in candidates + fibres):
        verdict = "M3_VALIDATION_INFRASTRUCTURE_UNRESOLVED"
    elif not all(item["classifier"] for item in candidates + fibres):
        verdict = "M3_STATE_ONLY_CLASSIFIER_INVALID"
    elif not all(item["persists"] for item in candidates):
        verdict = "M3_FINITE_TIME_FAMILY_CLOSED_NEGATIVE"
    elif not all(item["robust"] for item in candidates):
        verdict = "M3_SAMPLED_ROBUSTNESS_ONLY"
    elif all(item["complete"] for item in candidates):
        verdict = "M3_FINITE_TIME_SELECTED_MATTER_FAMILY"
    else:
        verdict = "M3_SAMPLED_ROBUSTNESS_ONLY"
    mode = "artifact"

checks = scope["checks"]
failures = scope["failures"]
if failures:
    print(f"FTD-0758 {mode}: {checks-len(failures)}/{checks} checks")
    for failure in failures:
        print(f"FAIL: {failure}")
    raise SystemExit(1)
print(f"FTD-0758 {mode}: {checks}/{checks} checks")
print(f"verdict={verdict}")
