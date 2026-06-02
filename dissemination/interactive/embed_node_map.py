"""
Build the standalone math_node_map.html by inlining the JSON.

Reads:
  - dissemination/interactive/math_node_map.template.html
  - scripts/verification/results/math_node_map.json

Writes:
  - dissemination/interactive/math_node_map.html  (self-contained)

The standalone HTML works on double-click (file://) without an HTTP
server, because the JSON is embedded inline in a
`<script type="application/json" id="map-data">` block.

The template still falls back to fetch() if the inline block is empty
or contains the placeholder, so an HTTP-served deployment also works.

Usage:
    python dissemination/interactive/embed_node_map.py
"""
from __future__ import annotations

import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parents[2]
TEMPLATE = REPO_ROOT / "dissemination" / "interactive" / "math_node_map.template.html"
JSON_IN  = REPO_ROOT / "scripts" / "verification" / "results" / "math_node_map.json"
HTML_OUT = REPO_ROOT / "dissemination" / "interactive" / "math_node_map.html"

PLACEHOLDER = "NODE_MAP_DATA_JSON_PLACEHOLDER"


def main():
    if not TEMPLATE.exists():
        print(f"ERROR: template missing: {TEMPLATE}")
        return 1
    if not JSON_IN.exists():
        print(f"ERROR: data missing: {JSON_IN}")
        print("  Run first: python scripts/verification/build_math_node_map.py")
        print("             python dissemination/interactive/math_node_map_layout.py")
        return 1

    template = TEMPLATE.read_text(encoding="utf-8")
    data = JSON_IN.read_text(encoding="utf-8")

    # Defensive: don't crash if a future template lacks the placeholder.
    if PLACEHOLDER not in template:
        print(f"WARNING: placeholder '{PLACEHOLDER}' not found in template;")
        print(f"  inlining will not happen.  Copying template verbatim to {HTML_OUT.name}.")
        HTML_OUT.write_text(template, encoding="utf-8")
        return 0

    # The JSON might contain stray "</script>" tokens inside string values
    # (unlikely here, but a known XSS vector).  Escape any such occurrence by
    # splitting the closing tag with a no-op JSON-comment-equivalent.
    data_safe = data.replace("</script>", r"<\/script>")

    # Replace ONLY the first occurrence -- the placeholder string lives only
    # in the inline <script type="application/json"> data block.  Any literal
    # mention of the placeholder text elsewhere (e.g. in JS comments or
    # diagnostic strings) must NOT be substituted, or the JSON payload would
    # bloat the JS source and break parsing (this is exactly what bit us once).
    n_occur = template.count(PLACEHOLDER)
    if n_occur > 1:
        print(f"WARNING: placeholder appears {n_occur} times in template;")
        print("  substituting only the first occurrence.  Audit the template.")
    out = template.replace(PLACEHOLDER, data_safe, 1)
    HTML_OUT.write_text(out, encoding="utf-8")

    template_kb = len(template) / 1024
    data_kb = len(data) / 1024
    out_kb = len(out) / 1024
    print(f"Embedded {JSON_IN.relative_to(REPO_ROOT)} ({data_kb:.1f} KB)")
    print(f"  template:   {template_kb:.1f} KB ({TEMPLATE.relative_to(REPO_ROOT)})")
    print(f"  standalone: {out_kb:.1f} KB ({HTML_OUT.relative_to(REPO_ROOT)})")
    print(f"  Open with: double-click {HTML_OUT.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
