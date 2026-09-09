"""Corrupted-seal controls using tiny synthetic evidence, never real archives."""
import hashlib
import json
from pathlib import Path
import zipfile

import pytest

from scripts.phi_v2_lattice import recovery_evidence_chain as chain


def write_json(path, value):
    path.write_text(json.dumps(value, sort_keys=True), encoding="utf-8")


@pytest.fixture
def sealed(tmp_path):
    root = tmp_path/"repo"
    root.mkdir()
    (root/"law.py").write_bytes(b"finite law\n")
    (root/"contract.md").write_bytes(b"declared finite domain\n")
    evidence = root/"evidence"
    (evidence/"review").mkdir(parents=True)
    (evidence/"range.json").write_bytes(b'{"checked":3}\n')
    (evidence/"review/manifest.json").write_bytes(b'{"retained":true}\n')
    sources = {name: chain.sha256(root/name) for name in ("law.py", "contract.md")}
    with zipfile.ZipFile(evidence/"source.zip", "x") as archive:
        for name in sources:
            archive.writestr(name, (root/name).read_bytes())
    artifacts = {name: chain.sha256(evidence/name) for name in ("range.json", "review/manifest.json", "source.zip")}
    manifest = {"source_sha256": sources, "artifact_sha256": artifacts,
                "source_archive": "source.zip", "source_archive_sha256": artifacts["source.zip"]}
    write_json(evidence/"manifest.json", manifest)
    descriptor = {"manifest": "evidence/manifest.json", "sha256": chain.sha256(evidence/"manifest.json"),
                  "source_count": 2, "artifact_count": 3}
    return root, evidence, manifest, descriptor


def rebind(evidence, manifest, descriptor):
    write_json(evidence/"manifest.json", manifest)
    descriptor["sha256"] = chain.sha256(evidence/"manifest.json")


def test_complete_seal_verifies_current_and_archived_bytes(sealed):
    root, evidence, manifest, descriptor = sealed
    before = {p: p.read_bytes() for p in root.rglob("*") if p.is_file()}
    result = chain.verify_preservation(root, [descriptor])
    assert result["status"] == "PASS" and result["unique_files_verified"] == 6
    assert result["seals"][0]["archive_members_verified"] == 2
    assert result["scientific_result_recomputed"] is False
    assert before == {p: p.read_bytes() for p in root.rglob("*") if p.is_file()}


@pytest.mark.parametrize("target", ["law.py", "evidence/range.json", "evidence/review/manifest.json", "evidence/source.zip"])
def test_changed_source_or_evidence_is_rejected(sealed, target):
    root, evidence, manifest, descriptor = sealed
    (root/target).write_bytes(b"changed")
    with pytest.raises(ValueError, match="identity changed"):
        chain.verify_preservation(root, [descriptor])


@pytest.mark.parametrize("change", ["extra", "missing", "omit_nested"])
def test_complete_artifact_inventory_is_required(sealed, change):
    root, evidence, manifest, descriptor = sealed
    if change == "extra":
        (evidence/"late.json").write_bytes(b"{}")
    elif change == "missing":
        (evidence/"range.json").unlink()
    else:
        del manifest["artifact_sha256"]["review/manifest.json"]
        descriptor["artifact_count"] -= 1
        rebind(evidence, manifest, descriptor)
    with pytest.raises(ValueError, match="inventory differs"):
        chain.verify_preservation(root, [descriptor])


def test_only_declared_rebuildable_cache_directories_are_ignored(sealed):
    root, evidence, manifest, descriptor = sealed
    for name in ("__pycache__", ".pytest_cache"):
        (evidence/name).mkdir()
        (evidence/name/"cache").write_bytes(b"rebuildable")
    assert chain.verify_preservation(root, [descriptor])["status"] == "PASS"


def test_rehashed_archive_cannot_replace_a_source_member(sealed):
    root, evidence, manifest, descriptor = sealed
    with zipfile.ZipFile(evidence/"source.zip", "w") as archive:
        archive.writestr("law.py", b"different law")
        archive.writestr("contract.md", (root/"contract.md").read_bytes())
    digest = chain.sha256(evidence/"source.zip")
    manifest["source_archive_sha256"] = manifest["artifact_sha256"]["source.zip"] = digest
    rebind(evidence, manifest, descriptor)
    with pytest.raises(ValueError, match="archived source bytes"):
        chain.verify_preservation(root, [descriptor])


def test_rehashed_archive_cannot_add_an_unlisted_member(sealed):
    root, evidence, manifest, descriptor = sealed
    with zipfile.ZipFile(evidence/"source.zip", "a") as archive:
        archive.writestr("unlisted", b"unlisted")
    digest = chain.sha256(evidence/"source.zip")
    manifest["source_archive_sha256"] = manifest["artifact_sha256"]["source.zip"] = digest
    rebind(evidence, manifest, descriptor)
    with pytest.raises(ValueError, match="member inventory"):
        chain.verify_preservation(root, [descriptor])


@pytest.mark.parametrize("name", ["../law.py", "C:/law.py", "./law.py", "folder\\law.py"])
def test_source_paths_cannot_escape_or_alias_the_declared_root(sealed, name):
    root, evidence, manifest, descriptor = sealed
    manifest["source_sha256"][name] = manifest["source_sha256"].pop("law.py")
    rebind(evidence, manifest, descriptor)
    with pytest.raises(ValueError, match="canonical repository-relative"):
        chain.verify_preservation(root, [descriptor])


def test_duplicate_or_unapproved_manifests_are_rejected(sealed):
    root, evidence, manifest, descriptor = sealed
    with pytest.raises(ValueError, match="duplicate seal"):
        chain.verify_preservation(root, [descriptor, descriptor])
    descriptor["sha256"] = "0"*64
    with pytest.raises(ValueError, match="manifest identity"):
        chain.verify_preservation(root, [descriptor])


def test_duplicate_json_keys_cannot_hide_a_different_inventory(sealed):
    root, evidence, manifest, descriptor = sealed
    raw = (evidence/"manifest.json").read_bytes()
    raw = raw[:-1]+b',"source_sha256":{}}'
    (evidence/"manifest.json").write_bytes(raw)
    descriptor["sha256"] = hashlib.sha256(raw).hexdigest()
    with pytest.raises(ValueError, match="duplicate JSON key"):
        chain.verify_preservation(root, [descriptor])
