#!/usr/bin/env python3
"""
Replace ASCII-art diagrams in the TRD Quarto manuscript with rendered images.

This tool:
1) Finds fenced code blocks with no language info.
2) Heuristically detects blocks that look like ASCII diagrams.
3) Renders each diagram block to both SVG and PNG (monospace text rendering).
4) Replaces the original fenced block in the .qmd with a Quarto image embed.

Outputs go under: dissemination/manuscript/figures/chXX/
Alongside each generated image, a .txt source file is written for reproducibility.

Usage:
  python figures/replace_ascii_diagrams.py
  python figures/replace_ascii_diagrams.py --dry-run
  python figures/replace_ascii_diagrams.py --check
  python figures/replace_ascii_diagrams.py --force
"""

from __future__ import annotations

import argparse
import hashlib
import re
import sys
from dataclasses import dataclass
from pathlib import Path
import os


@dataclass(frozen=True)
class FencedBlock:
    start_idx: int  # inclusive, 0-based line index
    end_idx: int  # exclusive, 0-based line index (line after closing fence)
    info: str
    content_lines: list[str]  # without line endings


FENCE_RE = re.compile(r"^```(?P<info>.*)$")


def _read_text_roundtrip(path: Path) -> tuple[str, str]:
    raw = path.read_bytes()
    newline = "\r\n" if b"\r\n" in raw else "\n"
    text = raw.decode("utf-8", errors="surrogateescape")
    return text, newline


def _write_text_roundtrip(path: Path, text: str, newline: str) -> None:
    # Normalize newlines to the file's original newline style.
    normalized = text.replace("\r\n", "\n").replace("\r", "\n").split("\n")
    out = newline.join(normalized)
    path.write_bytes(out.encode("utf-8", errors="surrogateescape"))


def _extract_fenced_blocks(lines: list[str]) -> list[FencedBlock]:
    blocks: list[FencedBlock] = []
    i = 0
    while i < len(lines):
        m = FENCE_RE.match(lines[i])
        if not m:
            i += 1
            continue

        info = (m.group("info") or "").strip()
        start = i
        i += 1
        content: list[str] = []
        while i < len(lines) and not lines[i].startswith("```"):
            content.append(lines[i])
            i += 1
        # If we found a closing fence, consume it.
        if i < len(lines) and lines[i].startswith("```"):
            i += 1
        end = i
        blocks.append(FencedBlock(start_idx=start, end_idx=end, info=info, content_lines=content))
    return blocks


def _weirdness_score(text: str) -> float:
    stripped = text.strip()
    if not stripped:
        return 0.0
    total = len(text)
    weird = sum(1 for c in text if (not c.isalnum()) and c not in " \t\n\r")
    box = sum(1 for c in text if 0x2500 <= ord(c) <= 0x259F)
    return (weird / total) + (box / max(1, total))


def _looks_like_ascii_diagram(content_lines: list[str]) -> bool:
    if len(content_lines) < 3:
        return False

    text = "\n".join(content_lines)

    # Strong diagram markers: box drawing, arrows, glyph plots, etc.
    if any(0x2500 <= ord(c) <= 0x259F for c in text):
        return True

    strong_markers = [
        "→",
        "←",
        "↑",
        "↓",
        "↔",
        "⇒",
        "⇐",
        "●",
        "★",
        "☉",
        "∿",
        "╱",
        "╲",
        # CP437-style box drawing frequently found in older ASCII art
        "Ú",
        "Ä",
        "¿",
        "À",
        "Ù",
        "Ã",
        "³",
        "´",
    ]
    if any(m in text for m in strong_markers):
        return True

    # Common ASCII diagram connectors (e.g., molecule sketches) without heavy punctuation density.
    first_chars = []
    for line in content_lines:
        stripped = line.lstrip()
        if stripped:
            first_chars.append(stripped[0])
    connector_leaders = {"|", "‖"}
    if sum(1 for ch in first_chars if ch in connector_leaders) >= 2:
        return True

    # Avoid converting actual code snippets (keep them as code blocks).
    code_like = 0
    code_line = re.compile(r"^\s*(?:def|class)\s+\w+|\b\w+\s*=\s*")
    for line in content_lines:
        if code_line.search(line):
            code_like += 1
    if code_like >= 2:
        return False

    # Heuristic fallback (no strong markers found).
    score = _weirdness_score(text)
    loose_markers = ["|", "/", "\\", "_", ">"]
    return score >= 0.23 and any(m in text for m in loose_markers)


def _sanitize_diagram_text(text: str) -> str:
    # Replace common control glyphs used in the manuscript as arrows.
    text = text.replace("\x1a", "→")  # SUB (often appears as a box glyph)
    text = text.replace("∿", "~")  # DejaVu Sans Mono doesn't include U+223F reliably.

    # Replace hard tabs with spaces (monospace alignment is still preserved).
    text = text.replace("\t", "    ")

    # Strip other ASCII control characters (keep newlines).
    cleaned = []
    for ch in text:
        code = ord(ch)
        if ch in "\n":
            cleaned.append(ch)
            continue
        if 0 <= code < 32:
            # Drop other control chars like ESC.
            continue
        cleaned.append(ch)
    return "".join(cleaned).rstrip() + "\n"


def _chapter_dir_for_qmd(qmd_path: Path) -> str:
    if qmd_path.parent.name == "chapters":
        # Expected: <major>.<minor>-<slug>.qmd
        m = re.match(r"^(?P<major>\d+)\.", qmd_path.name)
        if m:
            major = int(m.group("major"))
            return f"ch{major:02d}" if major < 10 else f"ch{major}"
    return "ch00"


def _slugify_stem(stem: str) -> str:
    # Keep digits/letters/hyphen, map dots to hyphens.
    s = stem.replace(".", "-")
    s = re.sub(r"[^A-Za-z0-9\\-]+", "-", s)
    s = re.sub(r"-{2,}", "-", s).strip("-")
    return s or "diagram"


def _render_text_figure(text: str, svg_path: Path, png_path: Path, *, dpi: int, font_size: int) -> None:
    import matplotlib.pyplot as plt

    lines = text.splitlines()
    max_len = max((len(l) for l in lines), default=0)
    n_lines = max(len(lines), 1)

    # Approximate monospace metrics.
    width_in = max(2.5, min(14.0, (max_len * font_size * 0.60 / 72.0) + 0.8))
    height_in = max(1.5, min(10.0, (n_lines * font_size * 1.25 / 72.0) + 0.8))

    fig = plt.figure(figsize=(width_in, height_in), dpi=dpi)
    fig.patch.set_facecolor("white")
    ax = fig.add_axes([0, 0, 1, 1])
    ax.set_axis_off()

    ax.text(
        0.02,
        0.98,
        text,
        va="top",
        ha="left",
        family="DejaVu Sans Mono",
        fontsize=font_size,
        color="#2C3E50",
    )

    svg_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(svg_path, format="svg", bbox_inches="tight", facecolor="white", edgecolor="none")
    fig.savefig(png_path, format="png", bbox_inches="tight", facecolor="white", edgecolor="none", dpi=dpi)
    plt.close(fig)


def _relpath(from_file: Path, to_file: Path) -> str:
    return os.path.relpath(to_file, start=from_file.parent).replace("\\", "/")


def _short_sha1(text: str, *, n: int = 10) -> str:
    return hashlib.sha1(text.encode("utf-8", errors="surrogateescape")).hexdigest()[:n]


_OLD_ASCII_BASE_RE = re.compile(r"^fig-ascii-(?P<stem>.+)-(?P<index>\d{2})$")
_HASHED_ASCII_BASE_RE = re.compile(r"^fig-ascii-(?P<stem>.+)-(?P<hash>[0-9a-f]{10})$")


def _maybe_migrate_ascii_asset(png_path: Path) -> tuple[Path, bool]:
    """
    If `png_path` is an old-style `fig-ascii-...-NN.png`, rename its .txt/.png/.svg
    siblings to hash-based names and return the new png path.

    Returns: (new_png_path, changed)
    """
    base = png_path.with_suffix("").name
    m_old = _OLD_ASCII_BASE_RE.match(base)
    if not m_old:
        return png_path, False

    # Only migrate the 2-digit trailing indices produced by this tool.
    if _HASHED_ASCII_BASE_RE.match(base):
        return png_path, False

    stem_slug = m_old.group("stem")

    txt_path = png_path.with_suffix(".txt")
    if not txt_path.exists():
        return png_path, False

    txt = txt_path.read_text(encoding="utf-8", errors="surrogateescape")
    sanitized = _sanitize_diagram_text(txt)
    digest = _short_sha1(sanitized)

    new_base = f"fig-ascii-{stem_slug}-{digest}"
    new_png = png_path.with_name(f"{new_base}.png")
    new_svg = png_path.with_name(f"{new_base}.svg")
    new_txt = png_path.with_name(f"{new_base}.txt")

    # If the target already exists with the same content, treat this as a
    # de-duplication and remove the old siblings.
    if new_txt.exists():
        existing = _sanitize_diagram_text(new_txt.read_text(encoding="utf-8", errors="surrogateescape"))
        if existing == sanitized:
            for old in [txt_path, png_path, png_path.with_suffix(".svg")]:
                if old.exists() and old not in {new_txt, new_png, new_svg}:
                    old.unlink()
            return new_png, True

        # Extremely unlikely (hash collision or manual edits). Keep both by
        # suffixing with the original 2-digit index.
        idx = m_old.group("index")
        new_base = f"fig-ascii-{stem_slug}-{digest}-{idx}"
        new_png = png_path.with_name(f"{new_base}.png")
        new_svg = png_path.with_name(f"{new_base}.svg")
        new_txt = png_path.with_name(f"{new_base}.txt")

    if txt_path != new_txt:
        txt_path.replace(new_txt)
    if png_path != new_png:
        png_path.replace(new_png)
    svg_path = png_path.with_suffix(".svg")
    if svg_path.exists() and svg_path != new_svg:
        svg_path.replace(new_svg)

    return new_png, True


def _migrate_existing_embeds(repo_root: Path, qmd_files: list[Path], *, dry_run: bool) -> int:
    # Find referenced fig-ascii PNGs, migrate assets, then update the embeds.
    img_re = re.compile(
        r"!\[(?P<alt>[^\]]*)\]\((?P<path>[^)]+fig-ascii-[^)]+?\.png)\)(?P<attrs>\{[^}]*\})?"
    )

    unique_pngs: dict[Path, Path] = {}
    migrated = 0

    for qmd in qmd_files:
        text, _newline = _read_text_roundtrip(qmd)
        for m in img_re.finditer(text):
            rel = m.group("path").strip()
            old_png = (qmd.parent / rel).resolve()
            if old_png in unique_pngs:
                continue
            if not old_png.exists():
                unique_pngs[old_png] = old_png
                continue
            new_png, changed = _maybe_migrate_ascii_asset(old_png) if not dry_run else (old_png, False)
            unique_pngs[old_png] = new_png
            migrated += int(changed)

    updated_qmd = 0
    for qmd in qmd_files:
        text, newline = _read_text_roundtrip(qmd)

        def _repl(match: re.Match[str]) -> str:
            rel = match.group("path").strip()
            old_png = (qmd.parent / rel).resolve()
            new_png = unique_pngs.get(old_png, old_png)
            if new_png == old_png:
                return match.group(0)
            new_rel = _relpath(qmd, new_png)
            return f"![{match.group('alt')}]({new_rel}){match.group('attrs') or ''}"

        new_text, n = img_re.subn(_repl, text)
        if n and (new_text != text):
            updated_qmd += 1
            if not dry_run:
                _write_text_roundtrip(qmd, new_text, newline)

    if dry_run:
        print(f"[dry-run] Would migrate {migrated} ASCII assets and update {updated_qmd} .qmd files.")
    else:
        print(f"Migrated {migrated} ASCII assets; updated {updated_qmd} .qmd files.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true", help="Show changes without writing files.")
    parser.add_argument("--check", action="store_true", help="Exit non-zero if ASCII diagrams remain.")
    parser.add_argument("--force", action="store_true", help="Regenerate images and .txt sources.")
    parser.add_argument("--regenerate-from-txt", action="store_true", help="(Re)render PNG/SVG from existing fig-ascii-*.txt sources.")
    parser.add_argument(
        "--migrate-existing-embeds",
        action="store_true",
        help="Rename old fig-ascii-*-NN assets to hash-based names and update .qmd embeds.",
    )
    parser.add_argument("--dpi", type=int, default=150, help="PNG DPI (SVG is resolution-independent).")
    parser.add_argument("--font-size", type=int, default=10, help="Monospace font size for rendered diagrams.")
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]  # dissemination/manuscript
    chapters_dir = repo_root / "chapters"
    figure_root = repo_root / "figures"

    if args.regenerate_from_txt:
        txt_files = sorted(figure_root.rglob("fig-ascii-*.txt"))
        for txt_path in txt_files:
            base = txt_path.with_suffix("")
            svg_path = base.with_suffix(".svg")
            png_path = base.with_suffix(".png")
            txt = txt_path.read_text(encoding="utf-8", errors="surrogateescape")
            sanitized = _sanitize_diagram_text(txt)
            if not args.dry_run:
                _render_text_figure(sanitized, svg_path, png_path, dpi=args.dpi, font_size=args.font_size)
            else:
                print(f"[dry-run] Would render: {png_path.relative_to(repo_root).as_posix()}")
        return 0

    qmd_files = [repo_root / "index.qmd", repo_root / "preface.qmd", repo_root / "about.qmd", repo_root / "symbols-glossary.qmd"]
    qmd_files += sorted(chapters_dir.glob("*.qmd"))
    qmd_files = [p for p in qmd_files if p.exists()]

    if args.migrate_existing_embeds:
        return _migrate_existing_embeds(repo_root, qmd_files, dry_run=args.dry_run)

    total_found = 0
    total_replaced = 0

    for qmd in qmd_files:
        text, newline = _read_text_roundtrip(qmd)
        lines = text.replace("\r\n", "\n").replace("\r", "\n").split("\n")

        blocks = _extract_fenced_blocks(lines)
        # Only consider no-language blocks.
        ascii_blocks = [b for b in blocks if b.info == "" and _looks_like_ascii_diagram(b.content_lines)]
        if not ascii_blocks:
            continue

        total_found += len(ascii_blocks)

        chapter_dir = _chapter_dir_for_qmd(qmd)
        stem_slug = _slugify_stem(qmd.stem)

        # Build replacements in one pass over lines.
        out_lines: list[str] = []
        cursor = 0
        ascii_block_iter = iter(enumerate(ascii_blocks, start=1))
        next_ascii = next(ascii_block_iter, None)

        while cursor < len(lines):
            if next_ascii and cursor == next_ascii[1].start_idx:
                _block_index, block = next_ascii
                # Write lines before this block (none here because cursor == start)

                original = "\n".join(block.content_lines)
                sanitized = _sanitize_diagram_text(original)
                digest = _short_sha1(sanitized)

                # Prepare output paths (hash-based to avoid collisions across runs)
                base = f"fig-ascii-{stem_slug}-{digest}"
                out_dir = figure_root / chapter_dir
                txt_path = out_dir / f"{base}.txt"
                svg_path = out_dir / f"{base}.svg"
                png_path = out_dir / f"{base}.png"

                if not args.dry_run:
                    out_dir.mkdir(parents=True, exist_ok=True)
                    if args.force or not txt_path.exists():
                        txt_path.write_text(sanitized, encoding="utf-8", errors="surrogateescape")
                    if args.force or (not svg_path.exists()) or (not png_path.exists()):
                        _render_text_figure(sanitized, svg_path, png_path, dpi=args.dpi, font_size=args.font_size)

                # Replace with image embed (no caption; alt text for accessibility)
                rel_img = _relpath(qmd, png_path)
                alt = f"Diagram ({qmd.stem})"
                out_lines.append(f'![{alt}]({rel_img}){{width="90%"}}')
                # Ensure a blank line after the embed for Markdown separation.
                out_lines.append("")
                total_replaced += 1

                cursor = block.end_idx
                next_ascii = next(ascii_block_iter, None)
                continue

            out_lines.append(lines[cursor])
            cursor += 1

        if args.dry_run:
            print(f"[dry-run] Would update: {qmd.relative_to(repo_root).as_posix()} ({len(ascii_blocks)} blocks)")
        else:
            new_text = "\n".join(out_lines)
            _write_text_roundtrip(qmd, new_text, newline)

    if args.check:
        if total_found == 0:
            return 0
        print(f"ASCII diagrams detected: {total_found}")
        return 1

    if args.dry_run:
        print(f"Found {total_found} ASCII blocks; would replace {total_replaced}.")
        return 0

    print(f"Replaced {total_replaced} ASCII diagram blocks (found {total_found}).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
