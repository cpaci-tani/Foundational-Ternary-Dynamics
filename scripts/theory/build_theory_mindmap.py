#!/usr/bin/env python3
"""
build_theory_mindmap.py -- generate the docs/theory/ topic mindmap.

Walks every git-tracked markdown file under docs/theory/ and emits a
hierarchical + cross-linked graph:

    root -> sector (01_reference .. 10_eft_program, archive)
         -> group  (programme/topic subdirectory, else doc-type prefix)
         -> document

Each document node carries its title, normalised epistemic tag (with the
tag's canonical colour), any FTD-NNNN LEDGER ids it cites, whether it is
archived, and its outbound intra-corpus links (which become cross-edges).

WHY THIS EXISTS
---------------
`dissemination/interactive/graph.json` + `3d_theory_map.html` were the
previous document mindmap. They were hand-built once (2026-05-25), had no
generator, and were never regenerated: by 2026-08-06 they covered 401 of
1,569 active docs (26%) and 56% of their paths no longer resolved. That is
the same decay pattern that hit LEDGER_INDEX.md and every per-sector INDEX
file in this corpus. The fix is the same: generate it, and guard it with a
--check mode wired into the test suite so drift fails loudly.

This is deliberately NOT the math node map
(`scripts/verification/build_math_node_map.py` ->
`scripts/verification/results/math_node_map.json`). That one maps
mathematical *objects, identities and spine theorems*. This one maps
*documents and their topic structure*. They are complementary; this script
reuses that pipeline's tag normalisation and colour table rather than
inventing a second, divergent taxonomy.

Usage:
    python scripts/theory/build_theory_mindmap.py            # regenerate
    python scripts/theory/build_theory_mindmap.py --check    # drift-only

Exit codes:
    0   regenerated (or --check found no drift)
    2   --check found drift
"""
from __future__ import annotations

import argparse
import json
import math
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "scripts"))

# Reuse the LEDGER pipeline's taxonomy rather than inventing a second one.
# TAG_NORMALISATION checks withdrawal markers (REFUTED/RETRACTED/WITHDRAWN/
# SUPERSEDED/DEMOTED) BEFORE positive tags, so a doc tagged e.g.
# "[DERIVED - LITERAL FORM REFUTED]" normalises to REFUTATION, not DERIVED.
# In a corpus that is majority negative results that ordering is the whole
# ballgame -- getting it backwards turns a navigation aid into an overclaim
# generator.
from verification.parsers.ledger_parser import (  # noqa: E402
    TAG_NORMALISATION,
    EPISTEMIC_COLORS,
)

THEORY_ROOT = "docs/theory"
OUT_JSON = REPO_ROOT / "dissemination" / "interactive" / "theory_mindmap.json"

SCHEMA_VERSION = 1

# Human-facing sector labels. Keys are directory names under docs/theory/.
SECTOR_LABELS = {
    "01_reference": "Reference & canon",
    "02_foundations": "Foundations",
    "03_derivations": "Derivations",
    "04_coupling": "Coupling constants",
    "05_particles": "Particles & masses",
    "06_reference_frames_and_measurement": "Reference frames & measurement",
    "07_assessment": "Assessment & ledgers",
    "08_structural": "Structural / Moore geometry",
    "09_mathematical": "Mathematical connections",
    "10_eft_program": "EFT program",
    "archive": "Archive (corpus-level)",
    "media": "Media",
    "_root": "Top-level navigation",
}

# Document-type prefixes, used as the grouping level when a sector has no
# meaningful subdirectory structure.
DOC_TYPES = [
    "SPEC", "DERIV", "FOUND", "AUDIT", "EXPLR", "THEOREM", "ANALYSIS",
    "PREREG", "INDEX", "REF", "SCOPE", "MONOGRAPH", "LEMMA", "CONJ",
    "CORRECTION", "CATALOG", "TRACKER", "LEDGER", "META", "ARCH",
    "CHANGELOG", "PLAN", "PROTOCOL", "REPORT", "ROADMAP", "SYNTHESIS",
    "NODE", "DRAFT", "MATH", "PROPOSAL", "STATUS", "FALSIFICATION",
]

# Document-level tags the LEDGER taxonomy does not cover. TAG_NORMALISATION is
# claim-oriented (it classifies FTD-NNNN claim verdicts); document headers also
# carry structural tags like [REFERENCE] that have no claim analogue and would
# otherwise all collapse to UNKNOWN. This supplement is applied ONLY after the
# shared taxonomy has already failed to match, so the withdrawal-markers-first
# ordering in TAG_NORMALISATION always wins -- a doc tagged
# "[REFERENCE - SUPERSEDED]" still normalises to SUPERSEDED, not REFERENCE.
# Editing the shared table instead would silently change the math node map's
# output too, so the supplement is kept local on purpose.
DOC_TAG_SUPPLEMENT = [
    (re.compile(r"\bREFERENCE\b", re.I),              "REFERENCE"),
    (re.compile(r"\bINSTRUMENT\s+SPEC", re.I),        "INSTRUMENT_SPEC"),
    (re.compile(r"\bPREDICTION\s+SPEC", re.I),        "INSTRUMENT_SPEC"),
    (re.compile(r"\bMETHODOLOG", re.I),               "METHODOLOGICAL_CLARIFICATION"),
    (re.compile(r"\bVOCABULARY\b", re.I),             "REFERENCE"),
    (re.compile(r"\bCANONICAL\b", re.I),              "REFERENCE"),
    (re.compile(r"\bDRAFT\b", re.I),                  "DRAFT"),
    (re.compile(r"\bCANDIDATE\b", re.I),              "CANDIDATE_RECONSTRUCTION"),
    (re.compile(r"\bPURE\s+MATHEMATICS\b", re.I),     "EXACT"),
    (re.compile(r"\bPROTOCOL\b", re.I),               "INSTRUMENT_SPEC"),
    # DOCUMENT-CLASS tags, deliberately NOT mapped to epistemic grades.
    # "[DERIVATION]" means "this document is a derivation write-up"; it does
    # NOT mean the claim inside is [DERIVED]. Likewise "[THEORY]" is not
    # [THEOREM]. Collapsing either onto its look-alike grade would invent an
    # epistemic promotion the document never claimed -- the exact failure mode
    # the 2026-08-06 corpus audit spent its time removing. They get their own
    # neutral class and a neutral colour instead.
    (re.compile(r"\bDERIVATION\b", re.I),             "DOC_CLASS_DERIVATION"),
    (re.compile(r"\bTHEORY\b", re.I),                 "DOC_CLASS_THEORY"),
    (re.compile(r"\bCAPSTONE\b", re.I),               "DOC_CLASS_THEORY"),
]

# Colours for supplement-only tags (the shared EPISTEMIC_COLORS has no entry).
SUPPLEMENT_COLORS = {
    "REFERENCE": "#546e7a",
    "INSTRUMENT_SPEC": "#0277bd",
    "DRAFT": "#8d6e63",
    # Neutral greys on purpose: a document-class tag asserts nothing about
    # epistemic standing, so it must not borrow the green of THEOREM/DERIVED.
    "DOC_CLASS_DERIVATION": "#78909c",
    "DOC_CLASS_THEORY": "#90a4ae",
    "UNTAGGED": "#9e9e9e",
    "UNKNOWN": "#757575",
}

TITLE_RE = re.compile(r"^#\s+(.+?)\s*$", re.M)
TAG_LINE_RE = re.compile(
    r"^\*\*(?:Tag|Status|Epistemic Status|Document Classification)[^:]*:\*\*\s*(.+?)\s*$",
    re.M | re.I,
)
FTD_ID_RE = re.compile(r"\bFTD-\d{4}[A-Za-z0-9\-]*\b")
MD_LINK_RE = re.compile(r"(?<!!)\[[^\]]*\]\(([^)\s]+)\)")


def git_tracked_theory_md() -> list[str]:
    out = subprocess.run(
        ["git", "ls-files", THEORY_ROOT],
        cwd=REPO_ROOT, capture_output=True, text=True, check=True,
    ).stdout.splitlines()
    return sorted(f for f in out if f.endswith(".md"))


def git_head_sha() -> str:
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=REPO_ROOT, text=True
        ).strip()
    except Exception:
        return "(unknown)"


def normalise_tag(raw: str) -> tuple[list[str], str]:
    """Return (all matched tags, primary tag).

    Shared LEDGER taxonomy first (so withdrawal markers keep priority), then
    the document-level supplement only for candidates it could not classify.
    """
    bracketed = re.findall(r"\[[^\]]+\]", raw)
    candidates = bracketed if bracketed else [raw]
    tags: list[str] = []
    for cand in candidates:
        norm = None
        for pat, name in TAG_NORMALISATION:
            if pat.search(cand):
                norm = name
                break
        if norm is None:
            for pat, name in DOC_TAG_SUPPLEMENT:
                if pat.search(cand):
                    norm = name
                    break
        if norm and norm not in tags:
            tags.append(norm)
    if not tags:
        tags = ["UNKNOWN"]
    return tags, tags[0]


def color_for(tag: str) -> str:
    return EPISTEMIC_COLORS.get(tag) or SUPPLEMENT_COLORS.get(tag, "#757575")


def doc_type_of(stem: str) -> str:
    for t in DOC_TYPES:
        if stem.startswith(t + "_") or stem == t:
            return t
    return "OTHER"


def group_for(rel: str) -> tuple[str, str]:
    """Return (group_key, group_kind) for a repo-relative theory path.

    Prefer a real subdirectory (programme / topic folder) when one exists,
    since that is the corpus's own organising principle after the 2026-08-06
    reorg. Fall back to the document-type prefix for flat sectors.
    """
    parts = Path(rel).parts  # docs, theory, <sector>, [subdirs...], file
    if len(parts) >= 5:
        sub = parts[3]
        if sub == "archive" and len(parts) >= 6:
            return f"archive/{parts[4]}", "archive-subdir"
        return sub, "subdir"
    return doc_type_of(Path(rel).stem), "doctype"


# Each document is read once and reused by both the structural parse and the
# concept scan; reading twice roughly doubled a full build.
_TEXT_CACHE: dict[str, str] = {}


def doc_text(rel: str) -> str:
    if rel not in _TEXT_CACHE:
        try:
            _TEXT_CACHE[rel] = (REPO_ROOT / rel).read_text(encoding="utf-8", errors="ignore")
        except OSError:
            _TEXT_CACHE[rel] = ""
    return _TEXT_CACHE[rel]


def parse_doc(rel: str) -> dict:
    path = REPO_ROOT / rel
    text = doc_text(rel)
    head = text[:4000]

    m = TITLE_RE.search(head)
    title = m.group(1).strip() if m else Path(rel).stem.replace("_", " ")
    # Strip markdown emphasis/backticks from the title for display.
    title = re.sub(r"[`*]", "", title)

    tm = TAG_LINE_RE.search(head)
    tag_raw = tm.group(1).strip() if tm else ""
    tags, primary = normalise_tag(tag_raw) if tag_raw else (["UNTAGGED"], "UNTAGGED")

    parts = Path(rel).parts
    # docs/theory/<sector>/... -- but 4 files sit directly at docs/theory/*.md
    # (META_INDEX.md, META_STRUCTURE.md, AGENT_THEORY_CUSTODIAN.md,
    # STRATEGY_PAPER_SPLIT_*.md). Without this guard each became its own
    # single-file "sector" named after the file.
    sector = parts[2] if len(parts) > 3 else "_root"
    group, group_kind = group_for(rel)
    archived = "/archive/" in rel or "/archive" in str(Path(rel).parent)

    ftd_ids = sorted(set(FTD_ID_RE.findall(head)))

    links: list[str] = []
    for target in MD_LINK_RE.findall(text):
        if target.startswith(("http://", "https://", "mailto:", "#", "data:", "file:")):
            continue
        t = target.split("#", 1)[0]
        if not t.endswith(".md"):
            continue
        resolved = (path.parent / t).resolve()
        try:
            r = resolved.relative_to(REPO_ROOT).as_posix()
        except ValueError:
            continue
        if r.startswith(THEORY_ROOT + "/") and r != rel:
            links.append(r)

    return {
        "id": rel,
        "path": rel,
        "title": title,
        "stem": Path(rel).stem,
        "sector": sector,
        "sector_label": SECTOR_LABELS.get(sector, sector),
        "group": group,
        "group_kind": group_kind,
        "doc_type": doc_type_of(Path(rel).stem),
        "tag_raw": tag_raw,
        "tags": tags,
        "primary_tag": primary,
        "color": color_for(primary),
        "archived": archived,
        "ftd_ids": ftd_ids,
        "links": sorted(set(links)),
        "bytes": path.stat().st_size if path.exists() else 0,
    }


# --- semantic layer tuning -------------------------------------------------
# Co-occurrence is dense by nature (323 concepts could yield ~52k pairs), so it
# is thresholded twice: a pair must share at least MIN_PAIR_DOCS documents, and
# each concept keeps only its TOP_EDGES_PER_CONCEPT strongest partners. Without
# both, the graph is a hairball and the JSON balloons.
MIN_PAIR_DOCS = 6
TOP_EDGES_PER_CONCEPT = 12
TOP_CONCEPTS_PER_DOC = 12

# A concept appearing in most of the corpus is a stopword FOR this corpus and
# carries no information about how topics connect. The first build showed why
# this is not optional: "Ftd" matched 1,706 of 1,734 documents (98%), and every
# single strongest edge was "X <-> Ftd". Terms like Only/Result/State/Finite
# behaved the same way. Rather than maintain an ever-growing hand stoplist, any
# concept above this document-frequency ceiling is dropped automatically.
MAX_DOC_FRACTION = 0.35
# Per-document concepts are ranked by tf-idf rather than raw count, so a
# document's "top" concepts are the ones that distinguish it, not the ones
# everybody mentions.


def build_concept_layer(docs: list[dict]) -> tuple[list[dict], list[dict]]:
    """Attach per-document concepts and return (concepts, concept_edges)."""
    from theory.theory_concepts import build_vocabulary, compile_matchers, concepts_in

    vocab = build_vocabulary([d["stem"] for d in docs], set(DOC_TYPES))
    matchers = compile_matchers(vocab)

    # Pass 1: raw hits per document, so document-frequency is known before any
    # ranking decision is made.
    raw: dict[str, dict[str, int]] = {}
    concept_docs: dict[str, set[str]] = {}
    concept_hits: Counter = Counter()

    for d in docs:
        hits = concepts_in(doc_text(d["path"]), matchers)
        raw[d["id"]] = hits
        for k, n in hits.items():
            concept_docs.setdefault(k, set()).add(d["id"])
            concept_hits[k] += n

    # Drop corpus-wide stopwords by document frequency (see MAX_DOC_FRACTION).
    n_docs = max(len(docs), 1)
    ceiling = MAX_DOC_FRACTION * n_docs
    kept = {c for c, ds in concept_docs.items() if 0 < len(ds) <= ceiling}
    dropped = sorted(
        (c for c in concept_docs if c not in kept),
        key=lambda c: -len(concept_docs[c]),
    )

    # Pass 2: rank each document's concepts by tf-idf over the kept vocabulary.
    doc_concepts: dict[str, list[str]] = {}
    for d in docs:
        hits = {k: v for k, v in raw[d["id"]].items() if k in kept}
        scored = sorted(
            hits.items(),
            key=lambda kv: (-(kv[1] * math.log(n_docs / len(concept_docs[kv[0]]))), kv[0]),
        )[:TOP_CONCEPTS_PER_DOC]
        labels = [k for k, _ in scored]
        d["concepts"] = labels
        doc_concepts[d["id"]] = labels

    concept_docs = {c: ds for c, ds in concept_docs.items() if c in kept}
    vocab = {k: v for k, v in vocab.items() if k in kept}

    concepts = [
        {
            "id": label,
            "label": label,
            "source": vocab[label]["source"],
            "doc_count": len(concept_docs.get(label, ())),
            "hits": concept_hits.get(label, 0),
        }
        for label in sorted(vocab)
        if concept_docs.get(label)
    ]

    # Co-occurrence weighted by shared-document count, computed from each
    # document's top concepts (not every mention) so the edges reflect what a
    # document is actually *about* rather than what it name-drops once.
    pair = Counter()
    for labels in doc_concepts.values():
        uniq = sorted(set(labels))
        for i in range(len(uniq)):
            for j in range(i + 1, len(uniq)):
                pair[(uniq[i], uniq[j])] += 1

    strong = [(a, b, w) for (a, b), w in pair.items() if w >= MIN_PAIR_DOCS]
    keep: set[tuple[str, str]] = set()
    per: dict[str, list] = {}
    for a, b, w in strong:
        per.setdefault(a, []).append((w, b))
        per.setdefault(b, []).append((w, a))
    for node, lst in per.items():
        for w, other in sorted(lst, reverse=True)[:TOP_EDGES_PER_CONCEPT]:
            keep.add((min(node, other), max(node, other)))

    edges = [
        {"source": a, "target": b, "weight": pair[(a, b)]}
        for (a, b) in sorted(keep) if (a, b) in pair
    ]
    return concepts, edges


def build() -> dict:
    files = git_tracked_theory_md()
    docs = [parse_doc(f) for f in files]
    by_id = {d["id"]: d for d in docs}

    # Cross-edges, deduplicated and restricted to resolvable in-corpus targets.
    edges = []
    seen = set()
    for d in docs:
        for t in d["links"]:
            if t in by_id:
                key = (d["id"], t)
                if key not in seen:
                    seen.add(key)
                    edges.append({"source": d["id"], "target": t})

    # Inbound-link counts give a cheap centrality signal for sizing nodes.
    inbound = Counter(e["target"] for e in edges)
    outbound = Counter(e["source"] for e in edges)
    for d in docs:
        d["inbound"] = inbound.get(d["id"], 0)
        d["outbound"] = outbound.get(d["id"], 0)
        # `links` was the raw scrape; `edges` is the same graph filtered to
        # resolvable in-corpus targets. Keeping both stored the link graph
        # twice (~770 KB of pure duplication in a file that regenerates on
        # every doc change). edges[] is the single source of truth; the viewer
        # builds its adjacency map from it at load.
        del d["links"]

    # Hierarchy: sector -> group -> docs
    tree: dict = {"id": "__root__", "name": "docs/theory", "children": []}
    sectors: dict[str, dict] = {}
    for d in sorted(docs, key=lambda x: (x["sector"], x["group"], x["stem"])):
        s = sectors.get(d["sector"])
        if s is None:
            s = {
                "id": f"sector:{d['sector']}",
                "name": d["sector_label"],
                "sector": d["sector"],
                "kind": "sector",
                "children": [],
            }
            sectors[d["sector"]] = s
            tree["children"].append(s)
        gkey = f"group:{d['sector']}/{d['group']}"
        g = next((c for c in s["children"] if c["id"] == gkey), None)
        if g is None:
            g = {
                "id": gkey,
                "name": d["group"],
                "kind": "group",
                "group_kind": d["group_kind"],
                "children": [],
            }
            s["children"].append(g)
        g["children"].append({"id": d["id"], "kind": "doc"})

    concepts, concept_edges = build_concept_layer(docs)

    active = [d for d in docs if not d["archived"]]
    tag_counts = Counter(d["primary_tag"] for d in active)

    return {
        "schema_version": SCHEMA_VERSION,
        "source_commit": git_head_sha(),
        "generator": "scripts/theory/build_theory_mindmap.py",
        "stats": {
            "documents_total": len(docs),
            "documents_active": len(active),
            "documents_archived": len(docs) - len(active),
            "sectors": len(sectors),
            "groups": sum(len(s["children"]) for s in sectors.values()),
            "cross_edges": len(edges),
            "tagged_active": sum(1 for d in active if d["primary_tag"] != "UNTAGGED"),
            "concepts": len(concepts),
            "concept_edges": len(concept_edges),
        },
        "tag_counts": dict(tag_counts.most_common()),
        "epistemic_colors": {**EPISTEMIC_COLORS, **SUPPLEMENT_COLORS},
        "tree": tree,
        "documents": docs,
        "edges": edges,
        "concepts": concepts,
        "concept_edges": concept_edges,
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true",
                    help="verify the committed JSON matches a fresh build; do not write")
    args = ap.parse_args()

    data = build()
    rendered = json.dumps(data, indent=1, ensure_ascii=False) + "\n"

    if args.check:
        if not OUT_JSON.exists():
            print(f"--check: {OUT_JSON} does not exist. Run the generator.", file=sys.stderr)
            return 2
        current = OUT_JSON.read_text(encoding="utf-8")
        # source_commit legitimately changes every commit; compare everything else.
        def strip_commit(s: str) -> str:
            return re.sub(r'"source_commit": "[^"]*"', '"source_commit": "*"', s)
        if strip_commit(current) != strip_commit(rendered):
            print(
                "--check: theory_mindmap.json is out of sync with docs/theory/.\n"
                "Re-run: python scripts/theory/build_theory_mindmap.py",
                file=sys.stderr,
            )
            return 2
        print(f"OK: theory_mindmap.json in sync "
              f"({data['stats']['documents_total']} docs, {data['stats']['cross_edges']} edges).")
        return 0

    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(rendered, encoding="utf-8")
    s = data["stats"]
    print(f"Wrote {OUT_JSON.relative_to(REPO_ROOT)}")
    print(f"  documents:   {s['documents_total']}  "
          f"({s['documents_active']} active, {s['documents_archived']} archived)")
    print(f"  sectors:     {s['sectors']}   groups: {s['groups']}")
    print(f"  cross-edges: {s['cross_edges']}")
    print(f"  tagged:      {s['tagged_active']}/{s['documents_active']} active docs")
    print("  top tags:    " + ", ".join(
        f"{k}={v}" for k, v in list(data["tag_counts"].items())[:8]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
