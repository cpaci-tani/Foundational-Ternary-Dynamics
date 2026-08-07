"""Guards for the generated docs/theory/ mindmap (2026-08-06).

These exist because the predecessor artifact decayed silently:
dissemination/interactive/graph.json was hand-built once, had no generator and
no test, and by the time anyone looked it covered 26% of the corpus with 56%
broken paths. Each test below pins one property that decay violated.
"""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

BUILDER = ROOT / "scripts" / "theory" / "build_theory_mindmap.py"
DATA = ROOT / "dissemination" / "interactive" / "theory_mindmap.json"
VIEWER = ROOT / "dissemination" / "interactive" / "theory_mindmap.html"


@pytest.fixture(scope="module")
def data() -> dict:
    return json.loads(DATA.read_text(encoding="utf-8"))


def test_mindmap_is_in_sync() -> None:
    """The committed JSON must be a faithful rebuild of the live corpus."""
    r = subprocess.run(
        [sys.executable, str(BUILDER), "--check"],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    assert r.returncode == 0, (
        f"mindmap out of sync (exit {r.returncode}):\n"
        f"STDOUT: {r.stdout}\nSTDERR: {r.stderr}\n"
        "Run: python scripts/theory/build_theory_mindmap.py"
    )


def test_covers_every_tracked_theory_doc(data: dict) -> None:
    """Coverage must be total. The predecessor covered 26% and nobody noticed."""
    tracked = {
        f for f in subprocess.run(
            ["git", "ls-files", "docs/theory"],
            capture_output=True, text=True, cwd=str(ROOT), check=True,
        ).stdout.split()
        if f.endswith(".md")
    }
    mapped = {d["path"] for d in data["documents"]}
    missing = sorted(tracked - mapped)
    assert not missing, f"{len(missing)} tracked theory docs absent from the mindmap: {missing[:10]}"


def test_every_document_path_resolves(data: dict) -> None:
    """Every node must point at a file that exists. 56% of the predecessor's did not."""
    broken = [d["path"] for d in data["documents"] if not (ROOT / d["path"]).is_file()]
    assert not broken, f"{len(broken)} mindmap nodes have non-resolving paths: {broken[:10]}"


def test_every_edge_endpoint_is_a_known_node(data: dict) -> None:
    ids = {d["id"] for d in data["documents"]}
    dangling = [e for e in data["edges"] if e["source"] not in ids or e["target"] not in ids]
    assert not dangling, f"{len(dangling)} edges reference unknown nodes: {dangling[:5]}"


def test_tree_leaves_match_document_list(data: dict) -> None:
    """The hierarchy and the flat document list must not disagree."""
    leaves: list[str] = []

    def walk(n: dict) -> None:
        kids = n.get("children") or []
        if not kids:
            leaves.append(n["id"])
        for c in kids:
            walk(c)

    walk(data["tree"])
    assert sorted(leaves) == sorted(d["id"] for d in data["documents"]), (
        "tree leaves and documents[] are out of sync"
    )


def test_withdrawal_markers_outrank_positive_tags() -> None:
    """A withdrawn-but-theorem-shaped tag must not read as a theorem.

    Regression guard for the most damaging failure mode available to this map:
    in a corpus that is majority negative results, normalising
    '[DERIVED - REFUTED]' to DERIVED turns navigation into an overclaim
    generator.
    """
    from theory.build_theory_mindmap import normalise_tag

    for raw, forbidden in [
        ("[DERIVED — LITERAL FORM REFUTED AT I5]", "DERIVED"),
        ("[CONDITIONAL THEOREM — WITHDRAWN]", "THEOREM"),
        ("[DERIVED — SUPERSEDED]", "DERIVED"),
        ("[RETAG — CONDITIONAL THEOREM TO SELECTION]", "THEOREM"),
        ("[THEOREM — DEMOTED]", "THEOREM"),
    ]:
        _tags, primary = normalise_tag(raw)
        assert primary != forbidden, (
            f"{raw!r} normalised to {primary!r}; a withdrawal marker must win"
        )


@pytest.mark.xfail(
    reason=(
        "KNOWN, DELIBERATE LIMITATION -- do not 'fix' by moving RETRACTED into "
        "the withdrawal-first block of TAG_NORMALISATION. That was tried on "
        "2026-08-06 and is strictly worse. Unlike REFUT/RETAG/WITHDRAW/"
        "SUPERSED/DEMOT (all rare as prose), 'retract' appears constantly in "
        "ordinary descriptive text in this corpus precisely because so much "
        "has been retracted. Promoting it mislabels 5 LEDGER rows, including "
        "FTD-0044 -- 'Per-voxel mass gap ... (Theorem 5.1 of retracted YM "
        "paper)', canonically 'THEOREM ... UNAFFECTED', i.e. the claim that "
        "SURVIVED the retraction. Downgrading it to RETRACTED is an underclaim "
        "generator: the exact mirror of the bug being fixed, destroying a real "
        "positive result. Closing this properly needs a status-assertion "
        "pattern (leading/emphasised RETRACTED) that excludes 'of/in/from "
        "retracted X' prose -- a real change to shared infrastructure that "
        "must be measured against all 747 LEDGER rows, not a reordering."
    ),
    strict=True,
)
def test_retracted_inside_a_positive_bracket_is_not_yet_caught() -> None:
    from theory.build_theory_mindmap import normalise_tag

    _tags, primary = normalise_tag("[THEOREM — RETRACTED 2026-06-19]")
    assert primary != "THEOREM"


def test_document_class_tags_are_not_epistemic_grades() -> None:
    """'[DERIVATION]' is a document class, not the grade [DERIVED]."""
    from theory.build_theory_mindmap import normalise_tag

    _t, deriv = normalise_tag("[DERIVATION]")
    assert deriv == "DOC_CLASS_DERIVATION", f"[DERIVATION] must not become {deriv!r}"

    _t, theory = normalise_tag("[THEORY]")
    assert theory == "DOC_CLASS_THEORY", f"[THEORY] must not become {theory!r}"


def test_every_tag_has_a_color(data: dict) -> None:
    colors = data["epistemic_colors"]
    missing = sorted({d["primary_tag"] for d in data["documents"]} - set(colors))
    assert not missing, f"tags with no colour assigned: {missing}"


def test_concept_layer_is_populated(data: dict) -> None:
    assert data["stats"]["concepts"] > 100, "concept vocabulary collapsed"
    assert data["stats"]["concept_edges"] > 200, "concept graph has no connections"


def test_concept_edges_reference_known_concepts(data: dict) -> None:
    ids = {c["id"] for c in data["concepts"]}
    dangling = [e for e in data["concept_edges"]
                if e["source"] not in ids or e["target"] not in ids]
    assert not dangling, f"{len(dangling)} concept edges point at unknown concepts"


def test_no_corpus_stopword_concepts(data: dict) -> None:
    """A concept in most of the corpus carries no connective information.

    Regression guard: the first concept build was dominated by 'Ftd', which
    matched 1,706 of 1,734 documents (98%), making every strongest edge
    'X <-> Ftd'. The document-frequency ceiling exists to prevent that.
    """
    total = data["stats"]["documents_total"]
    hogs = [(c["id"], c["doc_count"]) for c in data["concepts"]
            if c["doc_count"] > 0.40 * total]
    assert not hogs, f"corpus-stopword concepts survived the DF ceiling: {hogs[:5]}"


def test_document_concepts_are_known_and_bounded(data: dict) -> None:
    ids = {c["id"] for c in data["concepts"]}
    for d in data["documents"]:
        cs = d.get("concepts", [])
        assert len(cs) <= 12, f"{d['path']} carries {len(cs)} concepts (cap is 12)"
        unknown = [c for c in cs if c not in ids]
        assert not unknown, f"{d['path']} references unknown concepts {unknown}"


def test_synonyms_are_merged_not_duplicated(data: dict) -> None:
    """'Alpha' and 'α' are one concept, not two.

    Regression guard: before synonym merging, the strongest 'connections' in
    the graph were the vocabulary talking to itself -- Alpha<->α (199),
    Master<->Master quadratic (148), Horizon<->black hole (84).
    """
    ids = {c["id"] for c in data["concepts"]}
    for a, b in [("Alpha", "α"), ("Master", "Master quadratic"),
                 ("Horizon", "black hole"), ("Modes", "Mode")]:
        assert not (a in ids and b in ids), (
            f"{a!r} and {b!r} both present; synonym merge regressed"
        )


def test_viewer_exposes_both_modes() -> None:
    html = VIEWER.read_text(encoding="utf-8")
    for hook in ("modeStruct", "modeConcept", "conceptLayout", "drawConcepts"):
        assert hook in html, f"viewer is missing the {hook} hook"


def test_viewer_has_no_external_dependencies() -> None:
    """Self-contained by design: the predecessor died offline on CDN loads."""
    html = VIEWER.read_text(encoding="utf-8")
    for bad in ("unpkg.com", "cdn.jsdelivr", "cdnjs.cloudflare", "googleapis.com"):
        assert bad not in html, f"viewer pulls an external dependency: {bad}"
