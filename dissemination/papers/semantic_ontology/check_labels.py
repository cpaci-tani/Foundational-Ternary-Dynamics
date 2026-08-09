"""Label-graph check for semantic_ontology.tex.

Reports dangling references (fatal), labels defined and never referenced
(a smell, not always a defect -- navigation anchors are legitimate), and
forward references, which the genetic ordering cares about specifically:
a section that cites something defined later is spending what it has not
bought.

Usage:
    python check_labels.py                 # summary
    python check_labels.py --forward       # also list every forward ref
    python check_labels.py --section NAME  # forward refs out of one section
"""

import re
import sys
from pathlib import Path

TEX = Path(__file__).resolve().parent / "semantic_ontology.tex"
BS = chr(92)

# A \label on a section is a hyperref anchor and is legitimately
# uncited; a \label on an equation, figure, table or proposition is a
# promise that something points at it.  Only the latter are loose ends.
NAV_PREFIXES = ("sec:", "app:")


def scan(text):
    labels, refs = {}, []
    for m in re.finditer(BS + BS + r"label\{([^}]*)\}", text):
        labels[m.group(1)] = m.start()
    pat = BS + BS + r"(?:eq|c|C|auto|name|page)?ref\{([^}]*)\}"
    for m in re.finditer(pat, text):
        for key in m.group(1).split(","):
            refs.append((key.strip(), m.start()))
    return labels, refs


def section_at(text, pos):
    """Which \\section the given offset falls in."""
    best = ("(preamble)", -1)
    for m in re.finditer(BS + BS + r"section\{([^}]*)\}", text):
        if m.start() <= pos and m.start() > best[1]:
            best = (m.group(1), m.start())
    return best[0]


def main():
    text = TEX.read_text(encoding="utf8")
    labels, refs = scan(text)
    used = {k for k, _ in refs}

    dangling = sorted({k for k, _ in refs if k not in labels})
    unref = sorted(k for k in labels if k not in used)
    forward = [(k, p) for k, p in refs if k in labels and labels[k] > p]

    print("labels defined   : %d" % len(labels))
    print("distinct referenced: %d" % len(used))
    print()
    print("DANGLING (fatal) : %d %s" % (len(dangling), dangling or ""))
    real_unref = [k for k in unref if not k.startswith(NAV_PREFIXES)]
    print("unreferenced     : %d  (%d excluding section/appendix anchors)"
          % (len(unref), len(real_unref)))
    for k in real_unref:
        print("    %s" % k)
    print()
    print("forward refs     : %d" % len(forward))

    if "--forward" in sys.argv or "--section" in sys.argv:
        want = None
        if "--section" in sys.argv:
            want = sys.argv[sys.argv.index("--section") + 1]
        for k, p in forward:
            src = section_at(text, p)
            if want and want.lower() not in src.lower():
                continue
            dst = section_at(text, labels[k])
            print("    %-22s  %-34s -> %s" % (k, src[:34], dst[:40]))

    return 1 if dangling else 0


if __name__ == "__main__":
    sys.exit(main())
