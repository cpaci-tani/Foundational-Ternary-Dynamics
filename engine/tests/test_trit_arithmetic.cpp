// test_trit_arithmetic.cpp — Tests for balanced ternary arithmetic
#include <tritium/tritium.h>
#include <cstdio>
#include <cstdlib>

using namespace tritium;

static int failures = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::printf("  FAIL: %s (line %d)\n", msg, __LINE__); \
        failures++; \
    } \
} while(0)

// Helper: create a word with a single trit at position 0
TritWord64 single(Trit t) {
    TritWord64 w;
    w.set(0, t);
    return w;
}

// ============================================================================
// Negate
// ============================================================================
void test_negate() {
    std::printf("  Negate...\n");

    // Single trits
    TritWord64 pos = single(Trit::Pos);
    TritWord64 neg = single(Trit::Neg);
    TritWord64 zero = single(Trit::Zero);

    CHECK(negate(pos).get(0) == Trit::Neg, "negate(+1) == -1");
    CHECK(negate(neg).get(0) == Trit::Pos, "negate(-1) == +1");
    CHECK(negate(zero).get(0) == Trit::Zero, "negate(0) == 0");

    // Full word: negate(negate(x)) == x
    TritWord64 w;
    w.set(0, Trit::Pos); w.set(5, Trit::Neg); w.set(10, Trit::Zero);
    w.set(20, Trit::Neg); w.set(31, Trit::Pos);
    TritWord64 nn = negate(negate(w));
    CHECK(nn == w, "double negate is identity");
}

// ============================================================================
// Absolute value
// ============================================================================
void test_abs() {
    std::printf("  Abs...\n");
    TritWord64 w;
    w.set(0, Trit::Pos); w.set(1, Trit::Neg); w.set(2, Trit::Zero);

    TritWord64 a = abs(w);
    CHECK(a.get(0) == Trit::Pos, "abs(+1) == +1");
    CHECK(a.get(1) == Trit::Pos, "abs(-1) == +1");
    CHECK(a.get(2) == Trit::Zero, "abs(0) == 0");
}

// ============================================================================
// Element-wise multiply
// ============================================================================
void test_multiply() {
    std::printf("  Multiply...\n");

    // Exhaustive truth table for single-trit multiply
    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};
    int expected[3][3] = {
        { 1,  0, -1},  // -1 * {-1, 0, +1}
        { 0,  0,  0},  //  0 * {-1, 0, +1}
        {-1,  0,  1},  // +1 * {-1, 0, +1}
    };

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa, wb;
            wa.set(0, vals[a]);
            wb.set(0, vals[b]);
            TritWord64 result = multiply(wa, wb);
            CHECK(to_int(result.get(0)) == expected[a][b],
                  "multiply truth table");
        }
    }

    // Identity: a * splat(+1) == a
    TritWord64 w;
    w.set(0, Trit::Pos); w.set(5, Trit::Neg); w.set(10, Trit::Zero);
    TritWord64 ones = TritWord64::splat(Trit::Pos);
    TritWord64 prod = multiply(w, ones);
    CHECK(prod == w, "multiply by all-ones is identity");

    // Zero: a * splat(0) == splat(0)
    TritWord64 zeros;
    TritWord64 prod2 = multiply(w, zeros);
    CHECK(prod2.bits == 0, "multiply by all-zeros gives zero");
}

// ============================================================================
// Saturating add
// ============================================================================
void test_add_saturate() {
    std::printf("  Add (saturating)...\n");

    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};
    // Saturated addition truth table
    int expected[3][3] = {
        {-1, -1,  0},  // -1 + {-1, 0, +1}
        {-1,  0,  1},  //  0 + {-1, 0, +1}
        { 0,  1,  1},  // +1 + {-1, 0, +1}
    };

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa, wb;
            wa.set(0, vals[a]);
            wb.set(0, vals[b]);
            TritWord64 result = add_saturate(wa, wb);
            CHECK(to_int(result.get(0)) == expected[a][b],
                  "add_saturate truth table");
        }
    }

    // Identity: a + splat(0) == a
    TritWord64 w;
    w.set(0, Trit::Pos); w.set(5, Trit::Neg);
    TritWord64 zeros;
    TritWord64 sum = add_saturate(w, zeros);
    CHECK(sum == w, "add zero is identity");

    // Inverse: a + negate(a) == splat(0)
    TritWord64 inv = negate(w);
    TritWord64 cancel = add_saturate(w, inv);
    for (int i = 0; i < 32; ++i)
        CHECK(cancel.get(i) == Trit::Zero, "a + (-a) == 0");
}

// ============================================================================
// Add with carry
// ============================================================================
void test_add_with_carry() {
    std::printf("  Add with carry...\n");

    // +1 + +1 = -1 carry +1 (in balanced ternary: 1+1 = 3-1 = one carry + (-1))
    TritWord64 wa, wb;
    wa.set(0, Trit::Pos);
    wb.set(0, Trit::Pos);
    auto [sum, carry] = add_with_carry(wa, wb);
    CHECK(sum.get(0) == Trit::Neg, "1+1 sum = -1");
    CHECK(carry.get(1) == Trit::Pos, "1+1 carry = +1 at position 1");

    // -1 + -1 = +1 carry -1
    wa.set(0, Trit::Neg);
    wb.set(0, Trit::Neg);
    auto [sum2, carry2] = add_with_carry(wa, wb);
    CHECK(sum2.get(0) == Trit::Pos, "-1+-1 sum = +1");
    CHECK(carry2.get(1) == Trit::Neg, "-1+-1 carry = -1 at position 1");

    // +1 + -1 = 0, no carry
    wa.set(0, Trit::Pos);
    wb.set(0, Trit::Neg);
    auto [sum3, carry3] = add_with_carry(wa, wb);
    CHECK(sum3.get(0) == Trit::Zero, "1+(-1) sum = 0");
    CHECK(carry3.get(1) == Trit::Zero, "1+(-1) no carry");
}

int main() {
    std::printf("=== Tritium: Arithmetic Tests ===\n");

    test_negate();
    test_abs();
    test_multiply();
    test_add_saturate();
    test_add_with_carry();

    if (failures == 0) {
        std::printf("All arithmetic tests PASSED\n");
        return 0;
    } else {
        std::printf("%d arithmetic test(s) FAILED\n", failures);
        return 1;
    }
}
