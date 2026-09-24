"""Extract authored documentation; public mode reads Git objects, never worktree files.

python scripts/assistant/build_knowledge.py --mode local
python scripts/assistant/build_knowledge.py --mode public --revision HEAD --output DIRECTORY
"""
from __future__ import annotations
import argparse
import hashlib
from html import escape
from html.parser import HTMLParser
import io
import json
from pathlib import Path, PurePosixPath
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
EXTENSIONS = {".md", ".qmd", ".tex", ".txt", ".html", ".ipynb", ".pdf"}
PDF_VERSION = "6.18.1"
MAX_SOURCE_BYTES = 8 * 1024 * 1024
MAX_PDF_PAGES = 2000
EXCLUDED = {"node_modules", ".git", "thirdparty", "vendor", "validation", "_freeze", "_book", "_webbook", "__pycache__", "site-packages", ".cache"}
SOURCE_ROOTS = {"docs", "dissemination", "engine", "scripts"}
TAG = re.compile(r"\[([A-Z][A-Z0-9 _–—-]{1,70})\]")
HEADING = re.compile(r"^(?:#{1,6}\s+(.+)|\\(?:sub)*section\*?\{([^}]+)\})")


def allowed(path):
    p = PurePosixPath(path)
    if p.suffix.lower() not in EXTENSIONS or any(part in EXCLUDED or part.startswith("build") for part in p.parts):
        return False
    if len(p.parts) == 1:
        return p.suffix.lower() in {".md", ".txt"}
    if p.parts[0] not in SOURCE_ROOTS:
        return False
    # Application HTML is not repository documentation; authored demos are.
    if p.parts[:2] == ("engine", "web") and p.suffix == ".html" and not any(part in ("docs", "demos") for part in p.parts):
        return False
    return True


class TextHTML(HTMLParser):
    def __init__(self):
        super().__init__(); self.skip = 0; self.text = []
    def handle_starttag(self, tag, attrs):
        if tag in ("script", "style"): self.skip += 1
        elif tag in ("p", "div", "h1", "h2", "h3", "li", "br"): self.text.append("\n")
    def handle_endtag(self, tag):
        if tag in ("script", "style"): self.skip = max(0, self.skip - 1)
    def handle_data(self, data):
        if not self.skip: self.text.append(data)


def extract(path, data):
    suffix = PurePosixPath(path).suffix.lower()
    if suffix == ".pdf":
        return extract_pdf(data)[0]
    text = data.decode("utf-8", errors="replace")
    if suffix == ".ipynb":
        book = json.loads(text)
        text = "\n\n".join("".join(cell.get("source", [])) for cell in book.get("cells", []) if cell.get("cell_type") == "markdown")
        return text, "extracted"
    if suffix == ".html":
        parser = TextHTML(); parser.feed(text)
        return "".join(parser.text), "extracted"
    return text, "source"


class PDFCoverageError(ValueError):
    def __init__(self, status, detail):
        super().__init__(detail)
        self.status = status


def pdf_library():
    try:
        import pypdf
    except ImportError as error:
        raise RuntimeError("Install PDF extraction dependencies: python -m pip install --require-hashes -r scripts/assistant/requirements.txt") from error
    if pypdf.__version__ != PDF_VERSION:
        raise RuntimeError(f"Corpus builds require pypdf=={PDF_VERSION}; install scripts/assistant/requirements.txt")
    return pypdf


def extract_pdf(data):
    """Extract existing text only; never execute PDF actions or perform implicit OCR."""
    reader = pdf_library().PdfReader(io.BytesIO(data), strict=False)
    if reader.is_encrypted:
        raise PDFCoverageError("encrypted", "Password-protected PDF was not extracted")
    if len(reader.pages) > MAX_PDF_PAGES:
        raise PDFCoverageError("page-limit", f"PDF exceeds {MAX_PDF_PAGES} pages")
    parts, pages, empty = [], [], []
    line, extracted_bytes = 1, 0
    for number, page in enumerate(reader.pages, 1):
        text = (page.extract_text() or "").strip()
        extracted_bytes += len(text.encode("utf-8"))
        if extracted_bytes > MAX_SOURCE_BYTES:
            raise PDFCoverageError("text-limit", "Extracted PDF text exceeds 8 MiB")
        if text:
            pages.append({"page": number, "startLine": line, "endLine": line + len(text.splitlines()) - 1})
        else:
            empty.append(number)
        parts.append(text)
        line += text.count("\n") + 2
    if not pages:
        raise PDFCoverageError("empty", "No extractable text; scanned/image-only PDFs need explicit OCR")
    return ("\n\n".join(parts), "extracted"), {"pageCount": len(reader.pages), "textPages": len(pages), "emptyPages": empty, "pageRanges": pages}


def authored_pdf_source(path, indexed_sources):
    """Prefer an indexed authored sibling, including a conventional src/ folder."""
    p = PurePosixPath(path)
    for folder in (p.parent, p.parent / "src"):
        for extension in (".tex", ".qmd", ".md", ".txt"):
            candidate = (folder / (p.stem + extension)).as_posix()
            if candidate in indexed_sources:
                return candidate
    return None


def source_page(path, data, extracted=None):
    """An inert snapshot of precisely the text used for local index line ranges."""
    text, line_basis = extracted if extracted is not None else extract(path, data)
    source_hash = hashlib.sha256(data).hexdigest()
    lines = "\n".join(f'<span id="L{number}"><a href="#L{number}">{number}</a> {escape(line)}</span>'
                      for number, line in enumerate(text.splitlines(), 1))
    body = ("<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">"
            "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; base-uri 'none'; form-action 'none'\">"
            f"<title>{escape(path)} — indexed source</title></head><body><h1>{escape(path)}</h1>"
            f"<p>Indexed working-tree snapshot. Source SHA256: <code>{source_hash}</code>. "
            f"Line basis: {line_basis}. Rebuild the local index to reflect later edits.</p>"
            f"<pre>{lines}</pre></body></html>\n").encode("utf-8")
    page_hash = hashlib.sha256(body).hexdigest()
    return {"path": f"sources/{page_hash}.html", "sha256": page_hash, "size": len(body),
            "kind": "source-document", "sourcePath": path, "sourceHash": source_hash,
            "lineBasis": line_basis}, body


def write_source_page(destination, path, data, extracted=None):
    entry, body = source_page(path, data, extracted)
    target = (destination / entry["path"]).resolve()
    if not target.is_relative_to(destination.resolve()):
        raise ValueError("Source page escapes index destination")
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(body)
    return entry


def chunks(path, data, revision, public, local_source_path=None, extracted=None, page_ranges=None):
    text, line_basis = extracted if extracted is not None else extract(path, data)
    source_hash = hashlib.sha256(data).hexdigest()
    lines = text.splitlines()
    title = next((m.group(1) or m.group(2) for line in lines[:100] if (m := HEADING.match(line))), PurePosixPath(path).stem)
    historical = any(part in {"archive", "historical", "retired"} for part in PurePosixPath(path).parts)
    # A ledger's introductory retraction notice applies to its named claim,
    # not every entry in that ledger. Only explicit document-status fields
    # (and a title tag) can supply document-wide tags or historical status.
    status_lines = [line for line in lines[:30] if re.match(r"^\s*(?:\*\*)?(?:status|epistemic status|epistemic tag)\b", line, re.I)]
    top_tags = set(TAG.findall("\n".join([title, *status_lines])))
    historical = historical or any(tag.startswith(("RETRACTED", "CLOSED NEGATIVE", "CLOSED -- RESOLVED")) for tag in top_tags)
    authority = 3 if path.endswith("core_ledgers/LEDGER.md") else 2 if "SPEC_FTD_FRAMEWORK_V3_" in path else 1
    if not public and local_source_path is None:
        local_source_path = source_page(path, data, extracted)[0]["path"]
    section = title; pending = []; first = 1; length = 0
    def emit(end):
        body = "\n".join(pending).strip()
        if not body: return None
        ident = hashlib.sha256(f"{path}:{source_hash}:{first}:{end}".encode()).hexdigest()[:24]
        source_url = ("https://github.com/cpaci-tani/Foundational-Ternary-Dynamics/blob/" + revision + "/" + path
                      if public else "/api/ai/knowledge/" + local_source_path)
        if not public or line_basis == "source": source_url += f"#L{first}"
        source_pages = [page["page"] for page in page_ranges or [] if page["endLine"] >= first and page["startLine"] <= end]
        return {"id": ident, "title": title, "heading": section, "text": body, "sourcePath": path,
            "sourceHash": source_hash, "sourceRevision": revision if public else "working-tree",
            "sourceUrl": source_url, "startLine": first, "endLine": end, "lineBasis": line_basis,
            **({"pageStart": min(source_pages), "pageEnd": max(source_pages)} if source_pages else {}),
            "statusTags": sorted(set(TAG.findall(body)) | top_tags),
            "statusStatements": [line.strip() for line in pending if re.search(r"\b(?:tag|status)[_* .:]*[:.]", line, re.I)][:6],
            "historical": historical, "authority": authority,
            "claimIds": sorted(set(re.findall(r"FTD-\d{4,}", body)))}
    for number, line in enumerate(lines, 1):
        heading = HEADING.match(line)
        if pending and (heading or length + len(line) > 2200 or len(pending) >= 45):
            row = emit(number - 1)
            if row: yield row
            pending = []; length = 0
        if not pending: first = number
        if heading: section = heading.group(1) or heading.group(2)
        pending.append(line); length += len(line) + 1
    if pending:
        row = emit(len(lines))
        if row: yield row


def public_sources(revision):
    revision = subprocess.check_output(["git", "rev-parse", "--verify", revision + "^{commit}"], cwd=ROOT, text=True).strip()
    listing = subprocess.check_output(["git", "ls-tree", "-r", "--name-only", revision], cwd=ROOT).decode("utf-8").splitlines()
    proc = subprocess.Popen(["git", "cat-file", "--batch"], cwd=ROOT, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    try:
        for path in sorted(filter(allowed, listing)):
            if path.startswith("docs/internal/"):
                continue
            proc.stdin.write((revision + ":" + path + "\n").encode()); proc.stdin.flush()
            header = proc.stdout.readline().split()
            if len(header) != 3 or header[1] != b"blob": raise ValueError("Invalid Git object")
            size = int(header[2])
            if size <= MAX_SOURCE_BYTES:
                data = proc.stdout.read(size)
            else:
                remaining = size
                while remaining:
                    block = proc.stdout.read(min(remaining, 1024 * 1024))
                    if not block: raise ValueError("Truncated Git object")
                    remaining -= len(block)
                data = {"status": "oversize", "size": size}
            proc.stdout.read(1)
            yield path, data
    finally:
        proc.stdin.close(); proc.stdout.close(); proc.wait()


def local_sources():
    paths = list(ROOT.glob("*.md")) + list(ROOT.glob("*.txt"))
    for directory in sorted(SOURCE_ROOTS):
        # os.walk prunes large generated trees before visiting them.
        import os
        for folder, dirs, names in os.walk(ROOT / directory):
            dirs[:] = sorted(d for d in dirs if d not in EXCLUDED and not d.startswith("build") and not d.startswith("."))
            paths.extend(Path(folder) / name for name in names if Path(name).suffix.lower() in EXTENSIONS)
    for path in sorted(set(paths)):
        relative = path.relative_to(ROOT).as_posix()
        if allowed(relative) and path.resolve().is_relative_to(ROOT.resolve()) and path.is_file():
            size = path.stat().st_size
            yield relative, path.read_bytes() if size <= MAX_SOURCE_BYTES else {"status": "oversize", "size": size}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("public", "local"), required=True)
    parser.add_argument("--revision", default="HEAD")
    parser.add_argument("--output", type=Path, default=ROOT / "engine/.cache/local-ai/knowledge")
    args = parser.parse_args()
    pdf_library()  # Missing/wrong dependency must not silently shrink the corpus.
    revision = subprocess.check_output(["git", "rev-parse", "--verify", args.revision + "^{commit}"], cwd=ROOT, text=True).strip()
    sources = public_sources(revision) if args.mode == "public" else local_sources()
    count = total = 0
    skipped = []
    source_documents = []
    coverage_issues, pdf_coverage = [], []
    indexed_sources = {}
    destination = args.output.resolve()
    with tempfile.TemporaryDirectory(prefix="ftd-knowledge-") as temporary:
        rows = Path(temporary) / "chunks.jsonl"
        pending_pdfs = []
        with rows.open("w", encoding="utf-8") as output:
            def index_source(path, data, extracted=None, page_ranges=None):
                nonlocal count, total
                extracted = extracted if extracted is not None else extract(path, data)
                if not extracted[0].strip():
                    coverage_issues.append({"sourcePath": path, "status": "empty", "detail": "No authored text"})
                    return False
                source_entry = write_source_page(destination, path, data, extracted) if args.mode == "local" else None
                for chunk in chunks(path, data, revision, args.mode == "public", source_entry["path"] if source_entry else None,
                                    extracted, page_ranges):
                    output.write(json.dumps(chunk, ensure_ascii=False) + "\n"); total += 1
                if source_entry: source_documents.append(source_entry)
                indexed_sources[path] = hashlib.sha256(data).hexdigest()
                count += 1
                return True

            for path, data in sources:
                is_pdf = PurePosixPath(path).suffix.lower() == ".pdf"
                if isinstance(data, dict):
                    record = {"sourcePath": path, **data, "detail": "Source exceeds 8 MiB input bound"}
                    coverage_issues.append(record)
                    if is_pdf: pdf_coverage.append(record)
                    continue
                if is_pdf:
                    # Only unmatched PDFs are extracted after authored sources succeed.
                    saved = Path(temporary) / f"pdf-{len(pending_pdfs)}.bin"
                    saved.write_bytes(data)
                    pending_pdfs.append((path, saved))
                    continue
                try:
                    index_source(path, data)
                except (ValueError, KeyError, TypeError) as error:
                    skipped.append(path)
                    coverage_issues.append({"sourcePath": path, "status": "parse-error", "detail": type(error).__name__})
            for path, saved in pending_pdfs:
                data = saved.read_bytes()
                record = {"sourcePath": path, "sourceHash": hashlib.sha256(data).hexdigest(), "size": len(data)}
                companion = authored_pdf_source(path, indexed_sources)
                if companion:
                    pdf_coverage.append({**record, "status": "covered-by-authored-source", "coveredBy": {
                        "sourcePath": companion, "sourceHash": indexed_sources[companion]}})
                    continue
                try:
                    extracted, details = extract_pdf(data)
                    index_source(path, data, extracted, details["pageRanges"])
                    pdf_coverage.append({**record, "status": "indexed", **details})
                except PDFCoverageError as error:
                    record.update(status=error.status, detail=str(error))
                    pdf_coverage.append(record); coverage_issues.append(record)
                except Exception as error:
                    # Parser failures are explicit corpus gaps; never reinterpret PDF bytes as text.
                    record.update(status="parse-error", detail=type(error).__name__)
                    skipped.append(path); pdf_coverage.append(record); coverage_issues.append(record)
        metadata = Path(temporary) / "metadata.json"
        metadata.write_text(json.dumps({"corpus": args.mode, "sourceRevision": revision if args.mode == "public" else "working-tree",
            "baseRevision": revision, "sourceFiles": count, "chunks": total, "skippedSources": skipped,
            "sourceDocuments": source_documents,
            "coverageIssues": coverage_issues,
            "pdfExtraction": {"library": "pypdf", "version": PDF_VERSION, "ocr": False,
                "maxInputBytes": MAX_SOURCE_BYTES, "maxPages": MAX_PDF_PAGES, "sources": pdf_coverage,
                "summary": {status: sum(row["status"] == status for row in pdf_coverage) for status in sorted({row["status"] for row in pdf_coverage})}},
            "formats": sorted(EXTENSIONS), "scope": "Authored documentation and text-bearing PDFs; matching authored sources preferred; notebook markdown only; generated/vendor files excluded"}), encoding="utf-8")
        subprocess.run(["node", str(ROOT / "scripts/assistant/build_knowledge.mjs"), str(rows), str(metadata), str(args.output.resolve())], check=True, cwd=ROOT)


if __name__ == "__main__": main()
