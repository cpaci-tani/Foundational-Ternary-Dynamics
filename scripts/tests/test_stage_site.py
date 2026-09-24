"""The Pages archive publishes runtime assets only."""

import io
from pathlib import Path
import sys
import tarfile


sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "assistant"))
from stage_site import stage_web_archive  # noqa: E402


def test_stage_web_archive_excludes_local_files(tmp_path):
    files = {
        "index.html": b"site",
        "js/app.js": b"app",
        "js/assistant/model-manifest.json": b"{}",
        "js/ui/charts/vendor/uPlot.min.css": b"css",
        "css/style.css": b"css",
        "assets/observer/environments/studio.hdr": b"hdr",
        "data/measurements.json": b"{}",
        "wasm/ftd_core.wasm": b"wasm",
        "demos/example.html": b"demo",
        "serve.py": b"private server",
        "ai_service.py": b"private service",
        "tests/dashboard.spec.js": b"test",
        "docs/audits/evidence.json": b"evidence",
        "js/types.d.ts": b"types",
        "js/vendor/source.js.map": b"source map",
    }
    archive = io.BytesIO()
    with tarfile.open(fileobj=archive, mode="w") as output:
        for name, content in files.items():
            info = tarfile.TarInfo(f"engine/web/{name}")
            info.size = len(content)
            output.addfile(info, io.BytesIO(content))

    stage_web_archive(archive.getvalue(), tmp_path)

    staged = {
        path.relative_to(tmp_path).as_posix()
        for path in tmp_path.rglob("*") if path.is_file()
    }
    assert staged == {
        "index.html",
        "js/app.js",
        "js/assistant/model-manifest.json",
        "js/ui/charts/vendor/uPlot.min.css",
        "css/style.css",
        "assets/observer/environments/studio.hdr",
        "data/measurements.json",
        "wasm/ftd_core.wasm",
        "demos/example.html",
    }
