"""Exact rational certificate for FTD-0667 block elimination."""

from __future__ import annotations

from fractions import Fraction


def add(a, b):
    return [x + y for x, y in zip(a, b)]


def mv(matrix, vector):
    return [sum((value * vector[j] for j, value in enumerate(row)), Fraction())
            for row in matrix]


def mm(a, b):
    columns = list(zip(*b))
    return [[sum((x * y for x, y in zip(row, column)), Fraction())
             for column in columns] for row in a]


def identity(n):
    return [[Fraction(i == j) for j in range(n)] for i in range(n)]


def power(matrix, n):
    result = identity(len(matrix))
    for _ in range(n):
        result = mm(result, matrix)
    return result


A = [[Fraction(2, 3), Fraction(-1, 5)],
     [Fraction(1, 7), Fraction(3, 4)]]
B = [[Fraction(1, 2), Fraction(0), Fraction(-2, 9)],
     [Fraction(1, 11), Fraction(3, 8), Fraction(1, 6)]]
C = [[Fraction(-1, 3), Fraction(2, 5)],
     [Fraction(1, 4), Fraction(1, 9)],
     [Fraction(2, 7), Fraction(-3, 10)]]
D = [[Fraction(1, 2), Fraction(1, 3), Fraction(0)],
     [Fraction(-1, 5), Fraction(2, 3), Fraction(1, 7)],
     [Fraction(1, 11), Fraction(0), Fraction(3, 5)]]
x0 = [Fraction(2, 7), Fraction(-1, 3)]
y0 = [Fraction(1, 5), Fraction(2, 9), Fraction(-1, 4)]

xs = [x0]
ys = [y0]
for _ in range(10):
    xs.append(add(mv(A, xs[-1]), mv(B, ys[-1])))
    ys.append(add(mv(C, xs[-2]), mv(D, ys[-1])))

for n in range(11):
    reconstructed_y = mv(power(D, n), y0)
    for m in range(n):
        reconstructed_y = add(
            reconstructed_y, mv(mm(power(D, n - 1 - m), C), xs[m]))
    assert reconstructed_y == ys[n]
    if n < 10:
        reconstructed_x = add(mv(A, xs[n]), mv(B, mv(power(D, n), y0)))
        for m in range(n):
            kernel = mm(mm(B, power(D, n - 1 - m)), C)
            reconstructed_x = add(reconstructed_x, mv(kernel, xs[m]))
        assert reconstructed_x == xs[n + 1]

# Same matter state, different hidden field, different next matter state.
delta_y = [Fraction(1), Fraction(0), Fraction(0)]
assert mv(B, delta_y) != [Fraction(0), Fraction(0)]

# Invertible norm-preserving rotation transfers and revives the projection.
rotation = [[Fraction(0), Fraction(1)],
            [Fraction(-1), Fraction(0)]]
state = [Fraction(1), Fraction(0)]
matter = []
energies = []
for _ in range(5):
    matter.append(state[0])
    energies.append(state[0] * state[0] + state[1] * state[1])
    state = mv(rotation, state)
assert matter == [1, 0, -1, 0, 1]
assert energies == [1, 1, 1, 1, 1]

print("FTD-0667 matter-field memory-kernel certificate: PASS")
