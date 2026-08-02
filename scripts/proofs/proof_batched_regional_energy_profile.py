"""Exact-rational certificate for the FTD-0686 batched regional ledger."""

from fractions import Fraction
from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_BATCHED_REGIONAL_ENERGY_PROFILE_v1.md"
EXPECTED = "D1E180A987A24059AB8C49DF97E3A562C91EBE017AA80FA7DB4ED539D46F5E18"


def dot(left, right):
    return sum((a * b for a, b in zip(left, right)), Fraction(0))


def matvec(matrix, vector):
    return [dot(row, vector) for row in matrix]


def transpose(matrix):
    return [list(column) for column in zip(*matrix)]


def main():
    assert sha256(PROTOCOL.read_bytes()).hexdigest().upper() == EXPECTED
    dimension = 8
    c = [[Fraction(((3 * i + 5 * j + 1) % 11) - 5, 7)
          for j in range(dimension)] for i in range(dimension)]
    e = [Fraction(2 * i - 5, 9) for i in range(dimension)]
    b = [Fraction(7 - 3 * i, 10) for i in range(dimension)]
    cb = matvec(c, b)
    cte = matvec(transpose(c), e)
    masks = 0
    for bits in range(1 << dimension):
        mask = [Fraction((bits >> i) & 1) for i in range(dimension)]
        pe = [mask[i] * e[i] for i in range(dimension)]
        pct_e = [mask[i] * cte[i] for i in range(dimension)]
        assert dot(b, matvec(transpose(c), pe)) == dot(cb, pe)
        local = sum((Fraction(1, 2) * mask[i]
                     * (e[i] * e[i] + b[i] * b[i])
                     - Fraction(1, 4) * (e[i] * mask[i] * cb[i]
                                          + b[i] * pct_e[i])
                     for i in range(dimension)), Fraction(0))
        masked = Fraction(1, 2) * (dot(pe, pe)
                                   + dot([mask[i] * b[i]
                                          for i in range(dimension)],
                                         [mask[i] * b[i]
                                          for i in range(dimension)]))
        masked -= Fraction(1, 4) * (
            dot([mask[i] * b[i] for i in range(dimension)], cte)
            + dot(b, matvec(transpose(c), pe)))
        assert local == masked
        masks += 1
    radii = [0, 1, 3, 4, 7]
    contributions = [Fraction(i - 3, 13) for i in range(8)]
    cumulative = [sum((contributions[i] for i in range(len(contributions))
                       if i <= radius), Fraction(0)) for radius in radii]
    for radius, value in zip(radii, cumulative):
        assert value == sum(contributions[:radius + 1])
    assert masks == 256
    print("FTD-0686 batched regional-energy certificate: PASS "
          f"masks={masks} arithmetic=rational")


if __name__ == "__main__":
    main()
