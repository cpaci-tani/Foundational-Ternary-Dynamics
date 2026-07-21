"""
proof_dyadic_edge_census.py
===========================

Exact exhaustive census of the C3-tail H(8,3) configuration graph.

The script uses rational arithmetic for signed area and complete enumeration
of all 52,488 undirected one-trit edges. It is a finite combinatorial proof,
not a numerical near-miss search and not an FTD physics claim.
"""

from __future__ import annotations

from collections import Counter
from fractions import Fraction
from itertools import product


N = 8
BETA = Fraction(2)
AMPLITUDES = (
    Fraction(1),
    Fraction(1, 2),
    Fraction(1, 2),
    Fraction(3, 8),
    Fraction(3, 16),
    Fraction(3, 32),
    Fraction(3, 64),
    Fraction(3, 128),
)
WORDS = tuple(product((-1, 0, 1), repeat=N))
WORD_INDEX = {word: index for index, word in enumerate(WORDS)}


def signed_area_over_pi(word: tuple[int, ...]) -> Fraction:
    return sum(
        Fraction(2**k) * BETA * AMPLITUDES[k] ** 2 * abs(value) * value
        for k, value in enumerate(word)
    )


def area_sign(value: Fraction) -> int:
    return (value > 0) - (value < 0)


def sign_char(value: Fraction) -> str:
    return {-1: "-", 0: "0", 1: "+"}[area_sign(value)]


def trace(word: tuple[int, ...]) -> int:
    for k, value in enumerate(word):
        if value:
            return 2**k
    return 1


def classify_pattern(word: tuple[int, ...]) -> str:
    active = [k for k, value in enumerate(word) if value]
    if not active:
        return "void"
    if len(active) == 1:
        return "single clock"
    if len(active) == 2 and active == [0, len(word) - 1]:
        return "bookend"
    contiguous = all(right == left + 1 for left, right in zip(active, active[1:]))
    if contiguous and active[0] == 0:
        return "prefix"
    if contiguous:
        return "block"
    if all(k % 2 == 0 for k in active):
        return "even mask"
    if all(k % 2 == 1 for k in active):
        return "odd mask"
    if len(active) <= (len(word) + 2) // 3:
        return "sparse"
    return "mixed"


def lowest_other(word: tuple[int, ...], changed: int) -> int:
    for k, value in enumerate(word):
        if k != changed and value:
            return k
    return -1


def enumerate_edges():
    areas = tuple(signed_area_over_pi(word) for word in WORDS)
    traces = tuple(trace(word) for word in WORDS)
    patterns = tuple(classify_pattern(word) for word in WORDS)
    edges = []
    classes = Counter()
    counters = Counter()

    for source, word in enumerate(WORDS):
        for mode, value in enumerate(word):
            for target_value in (-1, 0, 1):
                if target_value <= value:
                    continue
                target_word = word[:mode] + (target_value,) + word[mode + 1 :]
                target = WORD_INDEX[target_word]
                source_area = areas[source]
                target_area = areas[target]
                kind = "chirality" if (value, target_value) == (-1, 1) else "toggle"
                area_wall = source_area == 0 or target_area == 0 or source_area * target_area < 0
                quotient_wall = traces[source] != traces[target]
                support_wall = kind == "toggle"
                chirality_wall = kind == "chirality"
                barriers = (
                    (1 if area_wall else 0)
                    | (2 if quotient_wall else 0)
                    | (4 if support_wall else 0)
                    | (8 if chirality_wall else 0)
                )
                fixed_support = sum(bool(x) for x in word) - int(value != 0)
                key = (
                    mode,
                    kind,
                    fixed_support,
                    lowest_other(word, mode),
                    sign_char(source_area) + sign_char(target_area),
                    int(area_wall),
                    int(quotient_wall),
                    patterns[source] + ">" + patterns[target],
                )
                classes[key] += 1
                counters["area"] += int(area_wall)
                counters["quotient"] += int(quotient_wall)
                counters["support"] += int(support_wall)
                counters["chirality"] += int(chirality_wall)
                edges.append((source, target, barriers))

    return edges, classes, counters


def chamber_count(edges: list[tuple[int, int, int]], barrier_mask: int) -> int:
    parent = list(range(len(WORDS)))
    size = [1] * len(WORDS)

    def find(value: int) -> int:
        while parent[value] != value:
            parent[value] = parent[parent[value]]
            value = parent[value]
        return value

    def union(left: int, right: int) -> None:
        left = find(left)
        right = find(right)
        if left == right:
            return
        if size[left] < size[right]:
            left, right = right, left
        parent[right] = left
        size[left] += size[right]

    for source, target, barriers in edges:
        if not barriers & barrier_mask:
            union(source, target)
    return len({find(index) for index in range(len(WORDS))})


def main() -> None:
    edges, classes, counters = enumerate_edges()
    assert len(WORDS) == 6561
    assert len(edges) == 52488
    assert len(classes) == 2510
    assert counters == Counter(
        area=10132,
        quotient=6558,
        support=34992,
        chirality=17496,
    )
    assert chamber_count(edges, 1) == 5
    assert chamber_count(edges, 2) == 8
    assert chamber_count(edges, 4) == 256
    assert chamber_count(edges, 4 | 8) == 6561

    print("PASS  6,561 ternary configurations")
    print("PASS  52,488 one-trit edges")
    print("PASS  2,510 exact invariant fingerprint bins")
    print("PASS  10,132 area walls; 6,558 quotient walls")
    print("PASS  chamber counts: area=5, quotient=8, support=256, support+chirality=6,561")
    print("No numerical near-miss search. No FTD physics claim promoted.")


if __name__ == "__main__":
    main()
