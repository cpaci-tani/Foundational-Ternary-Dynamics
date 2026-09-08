"""Generate fixed local laboratory preparations into ignored build artifacts."""
from pathlib import Path
import hashlib
import json
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from phi_v2_lattice import native_codec, prepare, staged


def main():
    directory = ROOT / "engine" / "build_strict" / "lab"
    directory.mkdir(parents=True, exist_ok=True)
    records = []
    for size in (3, 4, 7, 9):
        for name in ("relation", "sparse"):
            lattice = (prepare.isolated_relation(size) if name == "relation" else
                       prepare.sparse_material(size, seed=11, n_tokens=12, field_occupation=.005))
            data = native_codec.encode(staged.initialize(lattice))
            filename = f"{name}_{size}.bin"
            (directory / filename).write_bytes(data)
            records.append({"name": name, "L": size, "file": filename,
                            "sha256": hashlib.sha256(data).hexdigest()})
    (directory / "manifest.json").write_text(json.dumps({
        "law_id": staged.LAW_ID, "preparations": records,
        "status": "selected_finite_preparations_not_physical_particle_seeds",
    }, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {len(records)} local preparations to {directory}")


if __name__ == "__main__":
    main()
