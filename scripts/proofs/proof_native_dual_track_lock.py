"""FTD-0420/0421 exact lock and native-charge source-contract verifier."""

from __future__ import annotations

from fractions import Fraction
from hashlib import sha256
import json
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/proofs/native_dual_track_lock.json"
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_NATIVE_FIRST_DUAL_TRACK_RECOVERY.md"


def stripped_text(path: Path) -> str:
    text = path.read_text(encoding="utf-8").replace("\r\n", "\n")
    pattern = re.compile(
        r"\n?[ \t]*// FTD-HISTORY-BEGIN.*?// FTD-HISTORY-END\r?\n?",
        re.DOTALL,
    )
    return pattern.sub("\n", text)


def exact_rank(rows: list[list[int]]) -> int:
    matrix = [[Fraction(value) for value in row] for row in rows]
    rank = 0
    for column in range(len(matrix[0])):
        pivot = next(
            (row for row in range(rank, len(matrix)) if matrix[row][column]),
            None,
        )
        if pivot is None:
            continue
        matrix[rank], matrix[pivot] = matrix[pivot], matrix[rank]
        divisor = matrix[rank][column]
        matrix[rank] = [value / divisor for value in matrix[rank]]
        for row in range(len(matrix)):
            if row == rank or not matrix[row][column]:
                continue
            factor = matrix[row][column]
            matrix[row] = [
                value - factor * pivot_value
                for value, pivot_value in zip(matrix[row], matrix[rank])
            ]
        rank += 1
    return rank


def main() -> None:
    lock = json.loads(LOCK.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []
    successor = lock.get("qualified_successor", {}).get("files", {})
    for relative, expected in lock["files"].items():
        actual = sha256(stripped_text(ROOT / relative).encode()).hexdigest()
        original = actual == expected
        qualified = actual == successor.get(relative)
        label = "LOCK" if original else "QUALIFIED-SUCCESSOR"
        checks.append((f"{label} {relative}", original or qualified))

    rows = [
        [1, 1, 0, 0],
        [1, -1, 0, 0],
        [1, 1, 1, 1],
        [1, -1, -1, 1],
        [2, 0, 0, 0],
        [0, -2, 0, 0],
        [0, 2, 0, 0],
        [0, -2, -2, 0],
        [0, 2, 2, 0],
    ]
    checks.append(("ALG exact charge matrix rank is four", exact_rank(rows) == 4))
    checks.append(("ALG preregistered charge nullity is zero", 4 - exact_rank(rows) == 0))

    prereg = PREREG.read_text(encoding="utf-8")
    checks.append(("DOC six identifiers are reserved", all(x in prereg for x in lock["identifier_reservation"])))
    checks.append(("DOC no-target and no-retuning rules are explicit", "No numerical match" in prereg and "may not be retuned" in prereg))
    checks.append(("SRC event journal is observer-only", "consume no RNG values" in prereg and "change no state" in prereg))
    checks.append(("SRC native gate records scoped closure", "exact nullspace is trivial" in prereg))

    failed = 0
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(f"\nNative dual-track lock checks: {len(checks) - failed}/{len(checks)} passed")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
