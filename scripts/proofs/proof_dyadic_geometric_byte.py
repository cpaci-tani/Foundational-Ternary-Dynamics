"""
proof_dyadic_geometric_byte.py
================================

Exact finite-state verification for the eight-mode geometric-byte encoding.

The binary support word belongs to {0,1}^8 and therefore has 256 states.  The
signed mode word belongs to {-1,0,+1}^8 and therefore has 6561 states.  This
script verifies the support projection, its fiber cardinalities, and the
balanced-ternary bijection used by the interactive atlas.

This is an exhaustive finite combinatorial check, not a numerical search, and
it makes no FTD physics claim.
"""

from __future__ import annotations

from collections import Counter
from itertools import product


MODE_COUNT = 8
SUPPORT_COUNT = 2**MODE_COUNT
SIGNED_COUNT = 3**MODE_COUNT
BALANCED_OFFSET = (SIGNED_COUNT - 1) // 2


def support_mask(word: tuple[int, ...]) -> int:
    """Project a signed word to its binary nonzero-support mask."""
    return sum((1 << k) for k, trit in enumerate(word) if trit != 0)


def balanced_value(word: tuple[int, ...]) -> int:
    """Encode k_0-first trits as a balanced-ternary integer."""
    return sum(trit * 3**k for k, trit in enumerate(word))


def ternary_index(word: tuple[int, ...]) -> int:
    """Encode trits -1, 0, +1 as ordinary base-3 digits 0, 1, 2."""
    return sum((trit + 1) * 3**k for k, trit in enumerate(word))


def check_state_space_cardinalities() -> None:
    assert SUPPORT_COUNT == 256
    assert SIGNED_COUNT == 6561
    assert BALANCED_OFFSET == 3280


def check_projection_fibers() -> None:
    fibers: Counter[int] = Counter()
    for word in product((-1, 0, 1), repeat=MODE_COUNT):
        fibers[support_mask(word)] += 1

    assert len(fibers) == SUPPORT_COUNT
    for mask in range(SUPPORT_COUNT):
        weight = mask.bit_count()
        assert fibers[mask] == 2**weight

    assert sum(fibers.values()) == SIGNED_COUNT
    assert sum(2 ** mask.bit_count() for mask in range(SUPPORT_COUNT)) == SIGNED_COUNT


def check_balanced_ternary_bijection() -> None:
    words = list(product((-1, 0, 1), repeat=MODE_COUNT))
    values = {balanced_value(word) for word in words}
    indices = {ternary_index(word) for word in words}

    assert values == set(range(-BALANCED_OFFSET, BALANCED_OFFSET + 1))
    assert indices == set(range(SIGNED_COUNT))
    for word in words:
        assert ternary_index(word) == balanced_value(word) + BALANCED_OFFSET


def check_atlas_reference_word() -> None:
    # The app's default eight-mode extension is (+,-,+,-,0,0,0,0).
    word = (1, -1, 1, -1, 0, 0, 0, 0)
    assert support_mask(word) == 0x0F
    assert balanced_value(word) == -20
    assert ternary_index(word) == 3260


def main() -> None:
    checks = [
        check_state_space_cardinalities,
        check_projection_fibers,
        check_balanced_ternary_bijection,
        check_atlas_reference_word,
    ]
    for check in checks:
        check()
        print(f"PASS  {check.__name__}")
    print(f"PASS  geometric byte: {SUPPORT_COUNT} supports, {SIGNED_COUNT} signed words")


if __name__ == "__main__":
    main()
