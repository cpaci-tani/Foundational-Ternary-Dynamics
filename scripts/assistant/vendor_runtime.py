"""Reproduce the checked-in browser bundles from locked NPM provenance records."""
import base64
import hashlib
import io
import json
from pathlib import Path
import tarfile
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
TARGETS = [("web-llm/0.2.85", "package/lib/index.js"), ("minisearch/7.2.0", "package/dist/es/index.js")]


def main():
    for directory, member in TARGETS:
        target = ROOT / "engine/web/js/vendor" / directory
        record = json.loads((target / "provenance.json").read_text(encoding="utf-8"))
        with urllib.request.urlopen(record["tarball"], timeout=60) as response:
            data = response.read(20 * 1024 * 1024)
        sri = "sha512-" + base64.b64encode(hashlib.sha512(data).digest()).decode()
        if sri != record["integrity"]: raise ValueError("NPM tarball integrity mismatch")
        with tarfile.open(fileobj=io.BytesIO(data), mode="r:gz") as package:
            bundle = package.extractfile(member).read()
            if len(bundle) != record["bytes"] or hashlib.sha256(bundle).hexdigest() != record["sha256"]:
                raise ValueError("Bundle differs from checked-in provenance")
            (target / "index.js").write_bytes(bundle)
            license_name = next(name for name in package.getnames() if name in ("package/LICENSE", "package/LICENSE.txt", "package/LICENSE.md"))
            (target / "LICENSE").write_bytes(package.extractfile(license_name).read())
        print(record["package"] + " " + record["version"] + " verified")


if __name__ == "__main__": main()
