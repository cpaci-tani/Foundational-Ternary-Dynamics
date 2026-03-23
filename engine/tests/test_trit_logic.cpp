// test_trit_logic.cpp — Tests for Kleene strong three-valued logic
#include <tritium/tritium.h>
#include <cstdio>

using namespace tritium;

static int failures = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::printf("  FAIL: %s (line %d)\n", msg, __LINE__); \
        failures++; \
    } \
} while(0)

// Helper: make a word with a single trit at position 0
TritWord64 single(Trit t) {
    TritWord64 w;
    w.set(0, t);
    return w;
}

void test_not() {
    std::printf("  NOT...\n");
    CHECK(ternary_not(single(Trit::Pos)).get(0) == Trit::Neg, "NOT(+1) = -1");
    CHECK(ternary_not(single(Trit::Neg)).get(0) == Trit::Pos, "NOT(-1) = +1");
    CHECK(ternary_not(single(Trit::Zero)).get(0) == Trit::Zero, "NOT(0) = 0");

    // Double negation
    TritWord64 w;
    w.set(0, Trit::Pos); w.set(1, Trit::Neg); w.set(2, Trit::Zero);
    CHECK(ternary_not(ternary_not(w)) == w, "NOT(NOT(x)) = x");
}

void test_and() {
    std::printf("  AND (min)...\n");
    // Exhaustive 3x3 truth table
    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};
    // AND = min: -1 < 0 < +1
    int expected[3][3] = {
        {-1, -1, -1},  // min(-1, {-1, 0, +1})
        {-1,  0,  0},  // min( 0, {-1, 0, +1})
        {-1,  0,  1},  // min(+1, {-1, 0, +1})
    };

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa = single(vals[a]);
            TritWord64 wb = single(vals[b]);
            TritWord64 result = ternary_and(wa, wb);
            CHECK(to_int(result.get(0)) == expected[a][b], "AND truth table");
        }
    }

    // Commutativity
    TritWord64 x, y;
    x.set(0, Trit::Pos); x.set(1, Trit::Neg); x.set(2, Trit::Zero);
    y.set(0, Trit::Neg); y.set(1, Trit::Zero); y.set(2, Trit::Pos);
    CHECK(ternary_and(x, y) == ternary_and(y, x), "AND is commutative");
}

void test_or() {
    std::printf("  OR (max)...\n");
    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};
    // OR = max
    int expected[3][3] = {
        {-1,  0,  1},  // max(-1, {-1, 0, +1})
        { 0,  0,  1},  // max( 0, {-1, 0, +1})
        { 1,  1,  1},  // max(+1, {-1, 0, +1})
    };

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa = single(vals[a]);
            TritWord64 wb = single(vals[b]);
            TritWord64 result = ternary_or(wa, wb);
            CHECK(to_int(result.get(0)) == expected[a][b], "OR truth table");
        }
    }

    // Commutativity
    TritWord64 x, y;
    x.set(0, Trit::Pos); x.set(1, Trit::Neg);
    y.set(0, Trit::Neg); y.set(1, Trit::Pos);
    CHECK(ternary_or(x, y) == ternary_or(y, x), "OR is commutative");
}

void test_consensus() {
    std::printf("  Consensus...\n");
    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa = single(vals[a]);
            TritWord64 wb = single(vals[b]);
            TritWord64 result = consensus(wa, wb);
            if (a == b) {
                CHECK(result.get(0) == vals[a], "consensus: agree -> keep");
            } else {
                CHECK(result.get(0) == Trit::Zero, "consensus: disagree -> 0");
            }
        }
    }
}

void test_demorgan() {
    std::printf("  De Morgan's laws...\n");
    // NOT(AND(a,b)) == OR(NOT(a), NOT(b))
    // NOT(OR(a,b)) == AND(NOT(a), NOT(b))
    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa = single(vals[a]);
            TritWord64 wb = single(vals[b]);

            // NOT(AND(a,b)) == OR(NOT(a), NOT(b))
            TritWord64 lhs1 = ternary_not(ternary_and(wa, wb));
            TritWord64 rhs1 = ternary_or(ternary_not(wa), ternary_not(wb));
            CHECK(lhs1.get(0) == rhs1.get(0), "De Morgan 1");

            // NOT(OR(a,b)) == AND(NOT(a), NOT(b))
            TritWord64 lhs2 = ternary_not(ternary_or(wa, wb));
            TritWord64 rhs2 = ternary_and(ternary_not(wa), ternary_not(wb));
            CHECK(lhs2.get(0) == rhs2.get(0), "De Morgan 2");
        }
    }
}

int main() {
    std::printf("=== Tritium: Logic Tests ===\n");

    test_not();
    test_and();
    test_or();
    test_consensus();
    test_demorgan();

    if (failures == 0) {
        std::printf("All logic tests PASSED\n");
        return 0;
    } else {
        std::printf("%d logic test(s) FAILED\n", failures);
        return 1;
    }
}
