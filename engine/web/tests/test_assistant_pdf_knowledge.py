"""Real PDF fixtures exercise text extraction, duplicate preference and explicit gaps."""
import hashlib
import io
import json
from pathlib import Path
import sys

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / 'scripts/assistant'))
import build_knowledge as builder


def pdf_bytes(texts, encrypted=False):
    pypdf = builder.pdf_library()
    from pypdf.generic import DictionaryObject, NameObject, DecodedStreamObject
    writer = pypdf.PdfWriter()
    for text in texts:
        page = writer.add_blank_page(width=200, height=200)
        if text:
            font = DictionaryObject({NameObject('/Type'): NameObject('/Font'), NameObject('/Subtype'): NameObject('/Type1'),
                                     NameObject('/BaseFont'): NameObject('/Helvetica')})
            page[NameObject('/Resources')] = DictionaryObject({NameObject('/Font'): DictionaryObject({NameObject('/F1'): font})})
            content = DecodedStreamObject()
            content.set_data(f'BT /F1 12 Tf 10 100 Td ({text}) Tj ET'.encode('ascii'))
            page[NameObject('/Contents')] = writer._add_object(content)
    if encrypted:
        writer.encrypt('test-only-password')
    stream = io.BytesIO(); writer.write(stream)
    return stream.getvalue()


def test_pdf_text_has_correct_page_ranges_and_inert_local_line_citations():
    data = pdf_bytes(['PDF-only-needle [OPEN]', 'Second-page-evidence'])
    extracted, details = builder.extract_pdf(data)
    assert 'PDF-only-needle' in extracted[0] and extracted[1] == 'extracted'
    assert details['pageCount'] == details['textPages'] == 2
    assert details['pageRanges'] == [{'page': 1, 'startLine': 1, 'endLine': 1}, {'page': 2, 'startLine': 3, 'endLine': 3}]
    chunk = next(builder.chunks('docs/only.pdf', data, 'a' * 40, False, extracted=extracted, page_ranges=details['pageRanges']))
    assert chunk['pageStart'] == 1 and chunk['pageEnd'] == 2
    assert chunk['sourceHash'] == hashlib.sha256(data).hexdigest()
    entry, page = builder.source_page('docs/only.pdf', data, extracted)
    assert chunk['sourceUrl'] == '/api/ai/knowledge/' + entry['path'] + '#L1'
    assert b'id="L3"' in page and b'PDF-only-needle' in page


def test_pdf_manifest_covers_source_duplicates_text_and_every_extraction_gap(monkeypatch, tmp_path):
    text_pdf = pdf_bytes(['unmatchedneedle'])
    sources = [
        ('docs/paper.pdf', b'not parsed because authored source wins'),
        ('docs/paper.tex', b'\\section{Authored source}\nAuthoritative authored text'),
        ('docs/only.pdf', text_pdf),
        ('docs/empty.pdf', pdf_bytes([''])),
        ('docs/partial.pdf', pdf_bytes(['some text', ''])),
        ('docs/encrypted.pdf', pdf_bytes(['secret'], encrypted=True)),
        ('docs/corrupt.pdf', b'invalid PDF bytes'),
        ('docs/large.pdf', {'status': 'oversize', 'size': builder.MAX_SOURCE_BYTES + 1}),
        ('docs/empty-source.md', b''),
        ('docs/empty-source.pdf', text_pdf),
    ]
    monkeypatch.setattr(builder, 'local_sources', lambda: iter(sources))
    monkeypatch.setattr(sys, 'argv', ['build_knowledge.py', '--mode', 'local', '--output', str(tmp_path)])
    builder.main()
    manifest = json.loads((tmp_path / 'manifest.json').read_text())
    coverage = {entry['sourcePath']: entry for entry in manifest['pdfExtraction']['sources']}
    assert manifest['pdfExtraction']['version'] == '6.18.1' and manifest['pdfExtraction']['ocr'] is False
    assert coverage['docs/paper.pdf']['status'] == 'covered-by-authored-source'
    assert coverage['docs/paper.pdf']['coveredBy']['sourcePath'] == 'docs/paper.tex'
    assert coverage['docs/only.pdf']['status'] == coverage['docs/empty-source.pdf']['status'] == 'indexed'
    assert coverage['docs/partial.pdf']['emptyPages'] == [2]
    assert {coverage[f'docs/{name}.pdf']['status'] for name in ['empty', 'encrypted', 'corrupt', 'large']} == {'empty', 'encrypted', 'parse-error', 'oversize'}
    issues = {entry['sourcePath'] for entry in manifest['coverageIssues']}
    assert {'docs/empty.pdf', 'docs/encrypted.pdf', 'docs/corrupt.pdf', 'docs/large.pdf'} <= issues
    rows = [row for shard in manifest['shards'] for row in json.loads((tmp_path / shard['chunksPath']).read_text()).values()]
    assert not any(row['sourcePath'] == 'docs/paper.pdf' for row in rows)
    assert any(row['sourcePath'] == 'docs/only.pdf' and 'unmatchedneedle' in row['text'] for row in rows)


def test_authored_source_preference_is_scoped_and_requires_indexed_source():
    assert builder.authored_pdf_source('docs/paper.pdf', {'docs/src/paper.tex'}) == 'docs/src/paper.tex'
    assert builder.authored_pdf_source('docs/paper.pdf', {'archive/paper.tex'}) is None
    assert builder.authored_pdf_source('docs/paper.pdf', set()) is None


def test_oversize_pdf_is_reported_without_reading_source_bytes(monkeypatch, tmp_path):
    docs = tmp_path / 'docs'; docs.mkdir()
    path = docs / 'large.pdf'
    with path.open('wb') as stream:
        stream.truncate(builder.MAX_SOURCE_BYTES + 1)
    monkeypatch.setattr(builder, 'ROOT', tmp_path)
    assert list(builder.local_sources()) == [('docs/large.pdf', {'status': 'oversize', 'size': builder.MAX_SOURCE_BYTES + 1})]


def test_pdf_page_limit_is_explicit(monkeypatch):
    data = pdf_bytes(['one', 'two'])
    monkeypatch.setattr(builder, 'MAX_PDF_PAGES', 1)
    with pytest.raises(builder.PDFCoverageError) as caught:
        builder.extract_pdf(data)
    assert caught.value.status == 'page-limit'
