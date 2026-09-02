#!/usr/bin/env python3
"""Build a machine-readable manifest of every tracked code file in engine/.

Output (regenerable):
  engine/docs/ENGINE_FILE_MANIFEST.json   -- AI-consumable catalog (grep/load)
  engine/docs/ENGINE_FILE_MANIFEST.md     -- human-readable companion index

For each tracked code file the manifest records: path, language, line count,
subsystem/category bucket, a one-line purpose (extracted from the leading
comment / module docstring), and a best-effort list of top-level symbols
(classes / exported functions). This is shallow, mechanical metadata -- it does
NOT replace the conceptual docs (ARCHITECTURE.md, CALLSTACKS.md, etc.); it is
the file-level card catalog that complements them.

Regenerate with:
    python engine/tools/build_file_manifest.py
Run from the repository root.
"""
from __future__ import annotations

import json
import re
import subprocess
from collections import defaultdict
from pathlib import Path

CODE_EXTS = {".cpp", ".cc", ".h", ".hpp", ".cu", ".cuh", ".js", ".mjs", ".py"}

LANG = {
    ".cpp": "cpp", ".cc": "cpp", ".h": "c-header", ".hpp": "cpp-header",
    ".cu": "cuda", ".cuh": "cuda-header", ".js": "js", ".mjs": "js", ".py": "python",
}


def run(cmd: list[str]) -> str:
    return subprocess.run(cmd, capture_output=True, text=True, check=True).stdout


def _read_text(fp: Path) -> str:
    """Decode robustly: many engine files carry cp1252 em-dashes, not UTF-8."""
    try:
        data = fp.read_bytes()
    except Exception:
        return ""
    for enc in ("utf-8", "cp1252", "latin-1"):
        try:
            return data.decode(enc)
        except UnicodeDecodeError:
            continue
    return data.decode("utf-8", errors="replace")


def tracked_engine_files() -> list[str]:
    out = run(["git", "ls-files", "engine/"])
    files = []
    for line in out.splitlines():
        p = line.strip()
        if not p:
            continue
        if Path(p).suffix.lower() in CODE_EXTS:
            files.append(p)
    return sorted(files)


def classify(path: str) -> tuple[str, bool]:
    """Return (subsystem, is_primary). is_primary=False for archive/vendor/test."""
    p = path.replace("\\", "/")
    # vendored / third-party
    if "VISUAL_GUIDE_files" in p or "/lib/" in p or "/libs/" in p or p.endswith("/lib.js"):
        return "vendor", False
    if p.startswith("engine/src/render_bridge_phases/"):
        return "src/phases", True
    if p.startswith("engine/src/scenarios/"):
        return "src/scenarios", True
    if p.startswith("engine/src/atom/"):
        return "src/atom", True
    if p.startswith("engine/src/"):
        return "src/core", True
    if p.startswith("engine/include/"):
        return "include", True
    if p.startswith("engine/cuda/"):
        return "cuda", True
    if p.startswith("engine/wasm/"):
        return "wasm", True
    if p.startswith("engine/tests/"):
        return "tests", False
    if p.startswith("engine/sim/"):
        return "sim", True
    if p.startswith("engine/tools/"):
        return "tools", True
    if p.startswith("engine/web/tests/"):
        return "web/tests", False
    m = re.match(r"engine/web/js/scales/(scale\d+)/", p)
    if m:
        return f"web/{m.group(1)}", True
    if p.startswith("engine/web/js/scales/"):
        return "web/scales-shared", True
    if p.startswith("engine/web/js/bridge/"):
        return "web/bridge", True
    if p.startswith("engine/web/js/viewport/") or p == "engine/web/js/viewport.js":
        return "web/viewport", True
    if p.startswith("engine/web/js/ui/"):
        return "web/ui", True
    if p.startswith("engine/web/js/atlas/"):
        return "web/atlas", True
    if p.startswith("engine/web/js/inspector/"):
        return "web/inspector", True
    if p.startswith("engine/web/js/backgrounds/"):
        return "web/backgrounds", True
    if p.startswith("engine/web/js/core/"):
        return "web/core", True
    if p.startswith("engine/web/js/telemetry/"):
        return "web/telemetry", True
    if p.startswith("engine/web/js/config/"):
        return "web/config", True
    if p.startswith("engine/web/js/"):
        return "web/js-toplevel", True
    if p.startswith("engine/web/"):
        return "web/other", True
    return "other", True


_LICENSE_HINTS = ("copyright", "license", "spdx", "all rights reserved")


def extract_purpose(text: str, lang: str) -> str:
    """Best-effort one-line purpose from the leading comment / docstring."""
    lines = text.splitlines()
    # Python module docstring. NOTE: use [^\n] (not .) inside the leading
    # comment/shebang groups -- under re.DOTALL a repeated `.*\n` group causes
    # catastrophic backtracking. \A + [^\S\n] anchors without eating newlines.
    if lang == "python":
        m = re.match(
            r'\A[^\S\n]*(?:#![^\n]*\n)?(?:[^\S\n]*#[^\n]*\n)*[^\S\n]*'
            r'[rRbBuU]*("""|\'\'\')(.*?)\1',
            text, re.DOTALL)
        if m:
            body = m.group(2).strip()
            if body:
                return _first_sentence(body)
        # fall back to leading # block
        block = []
        for ln in lines:
            s = ln.strip()
            if s.startswith("#!"):
                continue
            if s.startswith("#"):
                block.append(s.lstrip("#").strip())
            elif s == "":
                if block:
                    break
            else:
                break
        if block:
            return _first_sentence(" ".join([b for b in block if b]))
        return ""
    # C-family / JS: first block comment or // run
    # block comment /* ... */ or /** ... */
    m = re.search(r"/\*+(.*?)\*/", text, re.DOTALL)
    block_pos = m.start() if m else len(text)
    # leading // lines
    line_block = []
    line_pos = None
    for i, ln in enumerate(lines[:40]):
        s = ln.strip()
        if s.startswith("//"):
            if line_pos is None:
                # position in text approx by index; we only need ordering vs block
                line_pos = i
            line_block.append(s.lstrip("/").strip())
        elif s == "" and line_block:
            break
        elif s == "" or s.startswith("#"):
            continue
        else:
            if line_block:
                break
            # hit code before any comment -> no leading // block
            break
    # prefer whichever comes first in the file
    candidate = ""
    if m and (line_pos is None or block_pos < (len("\n".join(lines[:line_pos])) if line_pos else 0)):
        candidate = re.sub(r"^\s*\*+", "", m.group(1), flags=re.MULTILINE)
        candidate = " ".join(c.strip() for c in candidate.splitlines() if c.strip())
    if not candidate and line_block:
        candidate = " ".join(b for b in line_block if b)
    candidate = candidate.strip()
    if candidate and not any(h in candidate.lower()[:60] for h in _LICENSE_HINTS):
        return _first_sentence(candidate)
    return ""


def _first_sentence(s: str) -> str:
    s = re.sub(r"\s+", " ", s).strip()
    # cut at first sentence end, cap length
    m = re.search(r"(.+?[.!?])(\s|$)", s)
    out = m.group(1) if m else s
    if len(out) > 200:
        out = out[:197].rstrip() + "..."
    return out


def extract_symbols(text: str, lang: str) -> list[str]:
    syms: list[str] = []
    if lang == "python":
        syms += re.findall(r"^class\s+(\w+)", text, re.MULTILINE)
        syms += [f"{n}()" for n in re.findall(r"^def\s+(\w+)", text, re.MULTILINE)]
    elif lang in ("js",):
        syms += re.findall(r"^export\s+class\s+(\w+)", text, re.MULTILINE)
        syms += re.findall(r"^class\s+(\w+)", text, re.MULTILINE)
        syms += [f"{n}()" for n in
                 re.findall(r"^export\s+(?:async\s+)?function\s+(\w+)", text, re.MULTILINE)]
        syms += [f"{n}()" for n in
                 re.findall(r"^export\s+function\s*\*?\s*(\w+)", text, re.MULTILINE)]
        syms += re.findall(r"^export\s+const\s+(\w+)", text, re.MULTILINE)
        syms += re.findall(r"^export\s+default\s+class\s+(\w+)", text, re.MULTILINE)
    else:  # c-family
        syms += re.findall(r"^\s*(?:class|struct)\s+([A-Z]\w+)", text, re.MULTILINE)
    # dedupe, preserve order, cap
    seen = set()
    out = []
    for s in syms:
        if s not in seen:
            seen.add(s)
            out.append(s)
        if len(out) >= 12:
            break
    return out


def main() -> None:
    repo = Path(run(["git", "rev-parse", "--show-toplevel"]).strip())
    files = tracked_engine_files()
    entries = []
    for rel in files:
        fp = repo / rel
        # `git ls-files` reports index entries that may already be deleted in the
        # working tree.  A generated working-tree manifest must not preserve
        # those paths as zero-line ghost files while cleanup is in progress.
        if not fp.is_file():
            continue
        text = _read_text(fp)
        loc = text.count("\n") + (1 if text and not text.endswith("\n") else 0)
        lang = LANG.get(fp.suffix.lower(), "other")
        subsystem, primary = classify(rel)
        entries.append({
            "path": rel.replace("\\", "/"),
            "lang": lang,
            "loc": loc,
            "subsystem": subsystem,
            "primary": primary,
            "purpose": extract_purpose(text, lang),
            "symbols": extract_symbols(text, lang),
        })

    # rollups
    by_sub: dict[str, dict] = defaultdict(lambda: {"files": 0, "loc": 0})
    total_loc = 0
    for e in entries:
        by_sub[e["subsystem"]]["files"] += 1
        by_sub[e["subsystem"]]["loc"] += e["loc"]
        total_loc += e["loc"]

    manifest = {
        "schema": "ftd-engine-file-manifest/v1",
        "generated_by": "engine/tools/build_file_manifest.py",
        "note": "Mechanical per-file catalog. Conceptual docs: engine/ARCHITECTURE.md, "
                "CALLSTACKS.md, SCENARIO_ARCHITECTURE.md, SPEC_ENGINE.md, "
                "engine/docs/ENGINE_CODE_MAP.md.",
        "totals": {"files": len(entries), "loc": total_loc},
        "subsystems": {k: v for k, v in sorted(
            by_sub.items(), key=lambda kv: -kv[1]["loc"])},
        "files": entries,
    }

    out_json = repo / "engine" / "docs" / "ENGINE_FILE_MANIFEST.json"
    out_json.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    # Markdown companion
    md = []
    md.append("# Engine File Manifest (auto-generated)\n")
    md.append("> Regenerate: `python engine/tools/build_file_manifest.py`  ")
    md.append("> Machine-readable source of truth: "
              "[`ENGINE_FILE_MANIFEST.json`](ENGINE_FILE_MANIFEST.json)  ")
    md.append("> Narrative map: [`ENGINE_CODE_MAP.md`](ENGINE_CODE_MAP.md)\n")
    md.append(f"**{len(entries)} code files, {total_loc:,} LOC** "
              "(tracked `.cpp/.cc/.h/.hpp/.cu/.cuh/.js/.mjs/.py` under `engine/`).\n")

    md.append("## Subsystem rollup\n")
    md.append("| Subsystem | Files | LOC |")
    md.append("|---|--:|--:|")
    for k, v in sorted(by_sub.items(), key=lambda kv: -kv[1]["loc"]):
        md.append(f"| `{k}` | {v['files']} | {v['loc']:,} |")
    md.append("")

    # group files by subsystem
    md.append("## Files by subsystem\n")
    groups: dict[str, list] = defaultdict(list)
    for e in entries:
        groups[e["subsystem"]].append(e)
    for sub in sorted(groups, key=lambda s: -by_sub[s]["loc"]):
        md.append(f"### `{sub}`  ({by_sub[sub]['files']} files, "
                  f"{by_sub[sub]['loc']:,} LOC)\n")
        md.append("| File | LOC | Purpose |")
        md.append("|---|--:|---|")
        for e in sorted(groups[sub], key=lambda x: -x["loc"]):
            purpose = (e["purpose"] or "").replace("|", "\\|")
            if not purpose and e["symbols"]:
                purpose = "_symbols:_ " + ", ".join(e["symbols"][:4])
            md.append(f"| [`{Path(e['path']).name}`]({_rel(e['path'])}) "
                      f"| {e['loc']} | {purpose} |")
        md.append("")

    out_md = repo / "engine" / "docs" / "ENGINE_FILE_MANIFEST.md"
    out_md.write_text("\n".join(md).rstrip() + "\n", encoding="utf-8")

    print(f"Wrote {out_json.relative_to(repo)}")
    print(f"Wrote {out_md.relative_to(repo)}")
    print(f"{len(entries)} files, {total_loc:,} LOC across {len(by_sub)} subsystems")


def _rel(path: str) -> str:
    """Link target relative to engine/docs/."""
    # manifest lives in engine/docs/; files are repo-root-relative under engine/
    return "../../" + path


if __name__ == "__main__":
    main()
