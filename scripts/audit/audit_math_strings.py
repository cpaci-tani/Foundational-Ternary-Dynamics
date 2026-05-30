"""Heuristic scan for ASCII-math strings that should be promoted to LaTeX.

Scans the user-facing text surfaces listed in the 2026-04-19 math
formatting spec and prints a punch-list of fragments grouped by file.
Output is advisory — a human reviews each suggestion before editing.
"""
from __future__ import annotations
import re
import sys
from pathlib import Path

SURFACE_GLOBS = [
    'engine/web/js/ui/components/faq/data.js',
    'engine/web/js/ui/components/knowledge-base/data.js',
    'engine/web/js/ui/components/tooltips/definitions.js',
    'engine/web/js/config/scenarios.js',
    'engine/web/js/ui/panels/lagrangian-panel/term-row.js',
    'engine/web/js/reference frame context-pedagogy.js',
    'engine/web/data/measurements.json',
]

ASCII_GREEK = r'\b(alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigma|tau|upsilon|phi|chi|psi|omega|Omega|Lambda|Delta|Sigma|Phi|Psi|Gamma|Theta)\b'

SIGNALS = [
    ('multichar-subscript',  re.compile(r'\b[A-Za-z]_[A-Za-z]{2,}\b')),
    ('explicit-caret-sup',   re.compile(r'\^\d+|\^\{[^}]+\}')),
    ('ascii-sqrt',           re.compile(r'\bsqrt\(')),
    ('ascii-greek',          re.compile(ASCII_GREEK)),
    ('numeric-fraction',     re.compile(r'\b\d+\s*/\s*\d+\b')),
    ('single-char-under',    re.compile(r'\b[A-Za-z]_[A-Za-z]\b')),
]

# Lines that should be ignored even if they match a signal.
IGNORE_PATTERNS = [
    re.compile(r'^\s*(import|export|from|require\()'),
    re.compile(r'(//|#)\s'),           # comments
    re.compile(r'https?://'),          # URLs
    re.compile(r'docs/theory/'),       # theory-ref paths
    re.compile(r'\b(id|url|date|source|units)\s*:'),  # structural JSON/JS keys
]

def should_ignore(line: str) -> bool:
    return any(rx.search(line) for rx in IGNORE_PATTERNS)


def scan_file(path: Path) -> list[tuple[int, str, str, str]]:
    """Return list of (lineno, signal_name, matched_fragment, full_line)."""
    hits = []
    try:
        text = path.read_text(encoding='utf-8')
    except FileNotFoundError:
        return hits
    for i, line in enumerate(text.splitlines(), start=1):
        if should_ignore(line):
            continue
        for name, rx in SIGNALS:
            for m in rx.finditer(line):
                frag = m.group(0)
                hits.append((i, name, frag, line.strip()))
    return hits


def main():
    repo_root = Path(__file__).resolve().parents[2]
    total = 0
    per_file = {}
    for rel in SURFACE_GLOBS:
        path = repo_root / rel
        hits = scan_file(path)
        if hits:
            per_file[rel] = hits
            total += len(hits)

    # Encode output as UTF-8 to handle non-ASCII characters
    import io
    stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

    for rel, hits in per_file.items():
        stdout.write(f'\n=== {rel} ({len(hits)} hits) ===\n')
        for lineno, name, frag, line in hits:
            line_short = line if len(line) <= 120 else line[:117] + '...'
            stdout.write(f'  {lineno:5d} [{name:22s}] {frag!r:30s} | {line_short}\n')
    stdout.write(f'\nTOTAL: {total} fragments flagged across {len(per_file)} files\n')
    stdout.flush()

    return 0 if total >= 0 else 1


if __name__ == '__main__':
    sys.exit(main())
