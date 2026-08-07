"""
theory_concepts.py -- concept/keyword vocabulary and matching for the theory
mindmap's semantic layer.

The mindmap's first layer answers "where does this document live". This module
supplies the second: "what is it about, and what else is about the same thing".

VOCABULARY SOURCES (all curated -- none invented here)
-----------------------------------------------------
  A. scripts/verification/parsers/object_aliases.py
       73 mathematical objects with hand-maintained alias lists, written
       precisely so variant spellings (G*, Gstar, G_STAR, GSTAR) collapse to
       one node per concept.
  B. resources/glossary/GLOSSARY.md
       ~55 FTD-specific concepts in "- **Term** - definition" form (Genesis,
       Moore neighborhood, Bandwidth budget, Latency, ...).
  C. Document filename tokens above a frequency floor.
       Filenames are hand-chosen and, unlike headers, free of template
       boilerplate.

WHY NOT AUTOMATIC PHRASE MINING
-------------------------------
An n-gram scan over titles+headers was tried first and is a trap here: the top
results are dominated by pre-registration template boilerplate ("run record",
"banned moves", "locked verdicts", "frozen artifact", "pre-blessed outcomes"),
because 413 of 1,734 documents are pre-registrations sharing one skeleton.
That would have produced a map of document *templates* rather than physics.
Filenames and curated glossaries do not have that failure mode.

PRECISION
---------
Matching short mathematical aliases against English prose is error-prone
("D" and "e" are alias entries; "pi" occurs inside "topology"). Rules:
  - every match is word-boundary anchored;
  - aliases shorter than MIN_ALIAS_LEN are dropped unless they contain a
    non-ASCII symbol or an explicit marker (so alpha/G* survive, "D"/"e" do not);
  - a stoplist removes filename tokens that are structural rather than topical.
`audit_precision()` samples real matches so the rules can be checked against
the corpus instead of assumed.
"""
from __future__ import annotations

import re
import sys
from collections import Counter
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "scripts"))

MIN_ALIAS_LEN = 3
FILENAME_TOKEN_FLOOR = 8   # a token must name >= this many docs to be a topic

# Filename tokens that are structural/scaffolding rather than subject matter.
FILENAME_STOPLIST = {
    "FROM", "WORK", "BLOCK", "CONNECTED", "INTERNAL", "MATCHED", "BOUND",
    "CORE", "AND", "THE", "FOR", "WITH", "VIA", "INTO", "NOT", "ALL", "NEW",
    "PART", "STEP", "FULL", "MAIN", "BASE", "TEST", "RUN", "SET", "MAP",
    "NOTE", "ITEM", "LIST", "ONE", "TWO", "THREE", "FIRST", "SECOND",
    "COMPLETE", "GENERAL", "SIMPLE", "BASIC", "FINAL", "TOTAL", "SINGLE",
    "NATIVE",  # 75 hits but a framing adjective across every programme
}

# Concepts worth pinning even if they fall below the filename floor, because
# they are load-bearing subject matter in this corpus. Kept short and explicit
# rather than mined, so the provenance of every vocabulary entry stays clear.
PINNED_CONCEPTS = {
    "confinement": ["confinement", "confining"],
    "look-elsewhere": ["look-elsewhere", "look elsewhere"],
    "pre-registration": ["pre-registration", "preregistration", "pre-registered"],
    "Bell/CHSH": ["CHSH", "Bell inequality", "Bell test", "Tsirelson"],
    "Born rule": ["Born rule"],
    "Chowla-Selberg": ["Chowla-Selberg", "Chowla Selberg"],
    "CM curve": ["CM curve", "CM-curve", "complex multiplication"],
    "Heegner": ["Heegner"],
    "Gauss projection": ["Gauss projection", "gauss_project", "Gauss-law projection"],
    "gauge invariance": ["gauge invariance", "gauge-invariant", "gauge invariant"],
    "renormalization": ["renormalization", "renormalisation", "RG flow"],
    "asymptotic freedom": ["asymptotic freedom"],
    "Lorentz recovery": ["Lorentz recovery", "Lorentz invariance", "Lorentz-invariance"],
    "dark matter": ["dark matter"],
    "black hole": ["black hole", "Schwarzschild", "horizon"],
    "graviton": ["graviton", "spin-2"],
    "Yukawa": ["Yukawa"],
    "Higgs": ["Higgs"],
    "Wilson loop": ["Wilson loop"],
    "Watson integral": ["Watson integral", "Watson identity"],
    "FC-W": ["FC-W", "FCW"],
    "MC-T4.3": ["MC-T4.3", "MC-T4-3"],
}


def _load_object_aliases() -> dict[str, list[str]]:
    from verification.parsers.object_aliases import OBJECTS
    out: dict[str, list[str]] = {}
    for canonical_id, _kind, label, aliases in OBJECTS:
        keep = [a for a in aliases
                if len(a) >= MIN_ALIAS_LEN or not a.isascii() or "*" in a]
        if keep:
            out[label or canonical_id] = keep
    return out


def _load_glossary_terms() -> dict[str, list[str]]:
    path = REPO_ROOT / "resources" / "glossary" / "GLOSSARY.md"
    if not path.is_file():
        return {}
    text = path.read_text(encoding="utf-8", errors="ignore")
    out: dict[str, list[str]] = {}
    for m in re.finditer(r"^-\s+\*\*(.+?)\*\*", text, re.M):
        term = re.sub(r"[`\\]", "", m.group(1)).strip()
        # Strip a trailing inline symbol, e.g. "Flux field `J(x)`".
        term = re.sub(r"\s*\(.*?\)\s*$", "", term).strip()
        if len(term) < MIN_ALIAS_LEN:
            continue
        if term.startswith("["):          # epistemic tags, already a facet
            continue
        out[term] = [term]
    return out


def _filename_tokens(doc_stems: list[str], prefixes: set[str]) -> dict[str, list[str]]:
    tok = Counter()
    for stem in doc_stems:
        for w in stem.split("_"):
            w = w.strip().upper()
            if not w or w in prefixes or w in FILENAME_STOPLIST:
                continue
            if re.fullmatch(r"V?\d+[A-Z]?", w):
                continue
            if len(w) < MIN_ALIAS_LEN:
                continue
            tok[w] += 1
    return {t.title(): [t] for t, c in tok.items() if c >= FILENAME_TOKEN_FLOOR}


# Filename tokens that name the same concept as a curated multi-word entry but
# share no alias string with it, so exact-alias merging cannot catch them.
# Without this the strongest "connections" in the graph are synonym pairs:
# the first build's top edges were Alpha<->α (199), Master<->Master quadratic
# (148), Horizon<->black hole (84) -- i.e. the vocabulary talking to itself.
MERGE_HINTS = {
    "Master": "Master quadratic",
    "Quadratic": "Master quadratic",
    "Lemniscatic": "G*",
    "Lemniscate": "G*",
    "Confinement": "confinement",
    "Yukawa": "Yukawa",
    "Higgs": "Higgs",
    "Graviton": "graviton",
    "Wilson": "Wilson loop",
    "Watson": "Watson integral",
    "Heegner": "Heegner",
    "Bell": "Bell/CHSH",
    "Chsh": "Bell/CHSH",
    "Born": "Born rule",
    "Prereg": "pre-registration",
    "Preregistration": "pre-registration",
}


def merge_synonyms(vocab: dict[str, dict]) -> tuple[dict[str, dict], dict[str, str]]:
    """Collapse duplicate concepts arising from different vocabulary sources.

    Returns (merged_vocab, canonical_map). Curated entries (object_aliases /
    glossary / pinned) always win over filename tokens, since they carry the
    hand-maintained alias lists.
    """
    priority = {"object_aliases": 0, "glossary": 1, "pinned": 2, "filename": 3}
    alias_owner: dict[str, str] = {}
    for label, meta in sorted(vocab.items(), key=lambda kv: priority[kv[1]["source"]]):
        if meta["source"] == "filename":
            continue
        for a in meta["aliases"] + [label]:
            alias_owner.setdefault(a.lower(), label)

    # Singular/plural pairs are pure artifacts wherever they occur (Mode/Modes
    # were a top-20 "connection"), so collapse them across all sources first.
    # NOTE: this is only applied to same-source pairs differing by a trailing
    # 's'; genuinely distinct related concepts such as Genesis/K_GENESIS (a
    # process and its threshold) or C_SPEED/c must survive, because their
    # co-occurrence is real signal rather than vocabulary noise.
    plural_of: dict[str, str] = {}
    lowered = {l.lower(): l for l in vocab}
    for label in vocab:
        low = label.lower()
        if low.endswith("s") and low[:-1] in lowered:
            singular = lowered[low[:-1]]
            if vocab[singular]["source"] == vocab[label]["source"]:
                plural_of[label] = singular

    canonical: dict[str, str] = {}
    for label, meta in vocab.items():
        if label in plural_of:
            canonical[label] = plural_of[label]
            continue
        if meta["source"] != "filename":
            canonical[label] = label
            continue
        low = label.lower()
        target = (
            MERGE_HINTS.get(label)
            or alias_owner.get(low)
            or alias_owner.get(low.rstrip("s"))      # Modes -> Mode
            or alias_owner.get(low + "s")
        )
        canonical[label] = target if target and target in vocab else label

    merged: dict[str, dict] = {}
    for label, meta in vocab.items():
        tgt = canonical[label]
        if tgt not in merged:
            merged[tgt] = {
                "aliases": list(vocab[tgt]["aliases"]),
                "source": vocab[tgt]["source"],
            }
        merged[tgt]["aliases"] = sorted(set(merged[tgt]["aliases"]) | set(meta["aliases"]))
    return merged, canonical


def build_vocabulary(doc_stems: list[str], doc_type_prefixes: set[str]) -> dict[str, dict]:
    """Return {concept_label: {"aliases": [...], "source": "..."} }."""
    vocab: dict[str, dict] = {}

    def add(label: str, aliases: list[str], source: str) -> None:
        if label in vocab:
            vocab[label]["aliases"] = sorted(set(vocab[label]["aliases"]) | set(aliases))
            return
        vocab[label] = {"aliases": sorted(set(aliases)), "source": source}

    for label, aliases in _load_object_aliases().items():
        add(label, aliases, "object_aliases")
    for label, aliases in _load_glossary_terms().items():
        add(label, aliases, "glossary")
    for label, aliases in PINNED_CONCEPTS.items():
        add(label, aliases, "pinned")
    for label, aliases in _filename_tokens(doc_stems, doc_type_prefixes).items():
        add(label, aliases, "filename")
    merged, _canonical = merge_synonyms(vocab)
    return merged


def _alias_regex(alias: str) -> str:
    """Word-boundary-anchored fragment. \\b is a no-op beside a non-word char
    (G*, alpha-symbol), so those ends are left unanchored deliberately."""
    esc = re.escape(alias)
    lead = r"\b" if re.match(r"\w", alias[0]) else ""
    tail = r"\b" if re.search(r"\w$", alias) else ""
    return f"{lead}{esc}{tail}"


class ConceptMatcher:
    """One combined pattern for the whole vocabulary.

    Alternatives are sorted longest-first because Python's alternation is
    leftmost-first: without it "N_c" would shadow "N_c = 3" and "Master" would
    shadow "Master quadratic".

    PERFORMANCE, measured rather than assumed. A full build is ~80s, and
    profiling puts ~all of it in this scan (reading all 1,734 files is 0.13s;
    vocabulary construction and regex compilation are ~0.01s each). Two
    "obvious" optimisations were tried and rejected on evidence:
      - one combined alternation vs. 258 per-concept patterns: no material
        difference. Kept anyway because it makes longest-first precedence
        explicit, not because it is faster.
      - lowercasing once and dropping re.I: 1.1x. Not worth the case-handling
        subtlety.
    Capping the scan at 20k chars/document would cut the build to ~55s, but was
    rejected: on a 40-document sample of long files only 6/40 kept an identical
    top-12 concept set (mean Jaccard 0.75), i.e. it silently changes what a
    quarter of the long documents appear to be *about*. Full-text scanning is
    the correct trade here -- this regenerates on doc changes, not per keystroke.
    """

    def __init__(self, vocab: dict[str, dict]):
        self.alias_to_label: dict[str, str] = {}
        for label, meta in vocab.items():
            for a in list(meta["aliases"]) + [label]:
                self.alias_to_label.setdefault(a.lower(), label)
        alts = sorted(self.alias_to_label, key=len, reverse=True)
        self.pattern = re.compile("|".join(_alias_regex(a) for a in alts), re.I)

    def scan(self, text: str) -> dict[str, int]:
        hits: Counter = Counter()
        a2l = self.alias_to_label
        for m in self.pattern.finditer(text):
            label = a2l.get(m.group(0).lower())
            if label:
                hits[label] += 1
        return dict(hits)


def compile_matchers(vocab: dict[str, dict]) -> ConceptMatcher:
    return ConceptMatcher(vocab)


def concepts_in(text: str, matcher: "ConceptMatcher") -> dict[str, int]:
    """Concept -> occurrence count within `text`."""
    return matcher.scan(text)


def audit_precision(vocab, matchers, sample_texts, per_concept=2):
    """Collect real matched snippets so precision can be eyeballed, not assumed."""
    out: dict[str, list[str]] = {}
    for label, pat in matchers:
        for txt in sample_texts:
            for m in pat.finditer(txt):
                s = max(0, m.start() - 38)
                snip = txt[s:m.end() + 38].replace("\n", " ")
                out.setdefault(label, [])
                if len(out[label]) < per_concept:
                    out[label].append(f"...{snip}...")
            if len(out.get(label, [])) >= per_concept:
                break
    return out
