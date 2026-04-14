/**
 * Test: Tritium algebra (consolidated suite)
 *
 * Merges 7 legacy header-only test files into a single ftd::test-instrumented
 * suite using the Phase 2a NDJSON telemetry API:
 *
 *   test_trit_packing      -> section "packing"
 *   test_trit_arithmetic   -> section "arithmetic"
 *   test_trit_logic        -> section "logic"
 *   test_trit_vector       -> section "vector"
 *   test_trit_matrix       -> section "matrix"
 *   test_trit_threshold    -> section "threshold"
 *   test_trit_convolution  -> section "convolution"
 *
 * Every CHECK(...) from the legacy files is preserved verbatim (same condition,
 * same label) and routed through ftd::test::check for uniform telemetry. The
 * legacy files defined a custom CHECK macro that only printed on failure; this
 * suite uses the standard ftd::test::check which prints PASS on success.
 */

#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

#include <tritium/tritium.h>

#include "ftd/test_telemetry.h"

using namespace tritium;

// --- Helpers (copied verbatim from legacy files) ---

// (from test_trit_arithmetic.cpp / test_trit_logic.cpp)
static TritWord64 single(Trit t) {
    TritWord64 w;
    w.set(0, t);
    return w;
}

// --- Section: packing  (from test_trit_packing.cpp) ---

static void test_tritword_get_set() {
    TritWord64 w;
    for (int i = 0; i < 32; ++i)
        ftd::test::check("default should be Zero", w.get(i) == Trit::Zero);

    w.set(0, Trit::Pos);
    w.set(1, Trit::Neg);
    w.set(31, Trit::Pos);
    ftd::test::check("position 0 should be Pos", w.get(0) == Trit::Pos);
    ftd::test::check("position 1 should be Neg", w.get(1) == Trit::Neg);
    ftd::test::check("position 31 should be Pos", w.get(31) == Trit::Pos);
    ftd::test::check("position 2 should still be Zero", w.get(2) == Trit::Zero);
}

static void test_tritword_splat() {
    TritWord64 z = TritWord64::splat(Trit::Zero);
    TritWord64 p = TritWord64::splat(Trit::Pos);
    TritWord64 n = TritWord64::splat(Trit::Neg);

    for (int i = 0; i < 32; ++i) {
        ftd::test::check("splat Zero", z.get(i) == Trit::Zero);
        ftd::test::check("splat Pos", p.get(i) == Trit::Pos);
        ftd::test::check("splat Neg", n.get(i) == Trit::Neg);
    }
}

static void test_tritword_popcount() {
    TritWord64 w;
    ftd::test::check("empty word has 0 nonzero", w.popcount_nonzero() == 0);
    ftd::test::check("empty word has 0 pos", w.popcount_pos() == 0);
    ftd::test::check("empty word has 0 neg", w.popcount_neg() == 0);

    w.set(0, Trit::Pos);
    w.set(1, Trit::Neg);
    w.set(2, Trit::Pos);
    ftd::test::check("3 nonzero trits", w.popcount_nonzero() == 3);
    ftd::test::check("2 positive trits", w.popcount_pos() == 2);
    ftd::test::check("1 negative trit", w.popcount_neg() == 1);
}

static void test_tritpack_get_set() {
    TritPack p;
    for (int i = 0; i < 5; ++i)
        ftd::test::check("default should be Zero", p.get(i) == Trit::Zero);

    p.set(0, Trit::Pos);
    p.set(1, Trit::Neg);
    p.set(4, Trit::Pos);
    ftd::test::check("position 0", p.get(0) == Trit::Pos);
    ftd::test::check("position 1", p.get(1) == Trit::Neg);
    ftd::test::check("position 2", p.get(2) == Trit::Zero);
    ftd::test::check("position 4", p.get(4) == Trit::Pos);
}

static void test_pack_roundtrip_exhaustive() {
    Trit values[3] = {Trit::Neg, Trit::Zero, Trit::Pos};
    int count = 0;

    for (int a = 0; a < 3; ++a)
    for (int b = 0; b < 3; ++b)
    for (int c = 0; c < 3; ++c)
    for (int d = 0; d < 3; ++d)
    for (int e = 0; e < 3; ++e) {
        TritPack p;
        p.set(0, values[a]);
        p.set(1, values[b]);
        p.set(2, values[c]);
        p.set(3, values[d]);
        p.set(4, values[e]);

        ftd::test::check("byte must be < 243", p.byte < 243);

        Trit decoded[5];
        decode_pack(p.byte, decoded);
        ftd::test::check("round-trip pos 0", decoded[0] == values[a]);
        ftd::test::check("round-trip pos 1", decoded[1] == values[b]);
        ftd::test::check("round-trip pos 2", decoded[2] == values[c]);
        ftd::test::check("round-trip pos 3", decoded[3] == values[d]);
        ftd::test::check("round-trip pos 4", decoded[4] == values[e]);
        count++;
    }
    ftd::test::check("should test all 243 combinations", count == 243);
}

static void test_batch_pack_unpack() {
    const size_t N = 100;

    TritWord64 words[4];
    for (size_t i = 0; i < 4; ++i) words[i].bits = 0;

    Trit pattern[3] = {Trit::Neg, Trit::Zero, Trit::Pos};
    for (size_t i = 0; i < N; ++i) {
        size_t wi = i / TritWord64::CAPACITY;
        int bi = static_cast<int>(i % TritWord64::CAPACITY);
        words[wi].set(bi, pattern[i % 3]);
    }

    size_t n_packs = packed_size(N);
    (void)n_packs;
    TritPack packs[20];
    pack(words, packs, N);

    TritWord64 restored[4];
    unpack(packs, restored, N);

    for (size_t i = 0; i < N; ++i) {
        Trit original = words[i / TritWord64::CAPACITY].get(
            static_cast<int>(i % TritWord64::CAPACITY));
        Trit roundtrip = restored[i / TritWord64::CAPACITY].get(
            static_cast<int>(i % TritWord64::CAPACITY));
        ftd::test::check("batch round-trip mismatch", original == roundtrip);
    }
}

static void test_trit_conversion() {
    ftd::test::check("Neg -> -1", to_int(Trit::Neg) == -1);
    ftd::test::check("Zero -> 0", to_int(Trit::Zero) == 0);
    ftd::test::check("Pos -> 1", to_int(Trit::Pos) == 1);
    ftd::test::check("negative -> Neg", to_trit(-5) == Trit::Neg);
    ftd::test::check("zero -> Zero", to_trit(0) == Trit::Zero);
    ftd::test::check("positive -> Pos", to_trit(42) == Trit::Pos);
    ftd::test::check("negate Pos", -Trit::Pos == Trit::Neg);
    ftd::test::check("negate Neg", -Trit::Neg == Trit::Pos);
    ftd::test::check("negate Zero", -Trit::Zero == Trit::Zero);
}

// --- Section: arithmetic  (from test_trit_arithmetic.cpp) ---

static void test_negate() {
    TritWord64 pos = single(Trit::Pos);
    TritWord64 neg = single(Trit::Neg);
    TritWord64 zero = single(Trit::Zero);

    ftd::test::check("negate(+1) == -1", negate(pos).get(0) == Trit::Neg);
    ftd::test::check("negate(-1) == +1", negate(neg).get(0) == Trit::Pos);
    ftd::test::check("negate(0) == 0", negate(zero).get(0) == Trit::Zero);

    TritWord64 w;
    w.set(0, Trit::Pos); w.set(5, Trit::Neg); w.set(10, Trit::Zero);
    w.set(20, Trit::Neg); w.set(31, Trit::Pos);
    TritWord64 nn = negate(negate(w));
    ftd::test::check("double negate is identity", nn == w);
}

static void test_abs() {
    TritWord64 w;
    w.set(0, Trit::Pos); w.set(1, Trit::Neg); w.set(2, Trit::Zero);

    TritWord64 a = abs(w);
    ftd::test::check("abs(+1) == +1", a.get(0) == Trit::Pos);
    ftd::test::check("abs(-1) == +1", a.get(1) == Trit::Pos);
    ftd::test::check("abs(0) == 0", a.get(2) == Trit::Zero);
}

static void test_multiply() {
    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};
    int expected[3][3] = {
        { 1,  0, -1},
        { 0,  0,  0},
        {-1,  0,  1},
    };

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa, wb;
            wa.set(0, vals[a]);
            wb.set(0, vals[b]);
            TritWord64 result = multiply(wa, wb);
            ftd::test::check("multiply truth table",
                             to_int(result.get(0)) == expected[a][b]);
        }
    }

    TritWord64 w;
    w.set(0, Trit::Pos); w.set(5, Trit::Neg); w.set(10, Trit::Zero);
    TritWord64 ones = TritWord64::splat(Trit::Pos);
    TritWord64 prod = multiply(w, ones);
    ftd::test::check("multiply by all-ones is identity", prod == w);

    TritWord64 zeros;
    TritWord64 prod2 = multiply(w, zeros);
    ftd::test::check("multiply by all-zeros gives zero", prod2.bits == 0);
}

static void test_add_saturate() {
    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};
    int expected[3][3] = {
        {-1, -1,  0},
        {-1,  0,  1},
        { 0,  1,  1},
    };

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa, wb;
            wa.set(0, vals[a]);
            wb.set(0, vals[b]);
            TritWord64 result = add_saturate(wa, wb);
            ftd::test::check("add_saturate truth table",
                             to_int(result.get(0)) == expected[a][b]);
        }
    }

    TritWord64 w;
    w.set(0, Trit::Pos); w.set(5, Trit::Neg);
    TritWord64 zeros;
    TritWord64 sum = add_saturate(w, zeros);
    ftd::test::check("add zero is identity", sum == w);

    TritWord64 inv = negate(w);
    TritWord64 cancel = add_saturate(w, inv);
    for (int i = 0; i < 32; ++i)
        ftd::test::check("a + (-a) == 0", cancel.get(i) == Trit::Zero);
}

static void test_add_with_carry() {
    // +1 + +1 = -1 carry +1
    TritWord64 wa, wb;
    wa.set(0, Trit::Pos);
    wb.set(0, Trit::Pos);
    auto [sum, carry] = add_with_carry(wa, wb);
    ftd::test::check("1+1 sum = -1", sum.get(0) == Trit::Neg);
    ftd::test::check("1+1 carry = +1 at position 1", carry.get(1) == Trit::Pos);

    // -1 + -1 = +1 carry -1
    wa.set(0, Trit::Neg);
    wb.set(0, Trit::Neg);
    auto [sum2, carry2] = add_with_carry(wa, wb);
    ftd::test::check("-1+-1 sum = +1", sum2.get(0) == Trit::Pos);
    ftd::test::check("-1+-1 carry = -1 at position 1", carry2.get(1) == Trit::Neg);

    // +1 + -1 = 0, no carry
    wa.set(0, Trit::Pos);
    wb.set(0, Trit::Neg);
    auto [sum3, carry3] = add_with_carry(wa, wb);
    ftd::test::check("1+(-1) sum = 0", sum3.get(0) == Trit::Zero);
    ftd::test::check("1+(-1) no carry", carry3.get(1) == Trit::Zero);
}

// --- Section: logic  (from test_trit_logic.cpp) ---

static void test_not() {
    ftd::test::check("NOT(+1) = -1",
                     ternary_not(single(Trit::Pos)).get(0) == Trit::Neg);
    ftd::test::check("NOT(-1) = +1",
                     ternary_not(single(Trit::Neg)).get(0) == Trit::Pos);
    ftd::test::check("NOT(0) = 0",
                     ternary_not(single(Trit::Zero)).get(0) == Trit::Zero);

    TritWord64 w;
    w.set(0, Trit::Pos); w.set(1, Trit::Neg); w.set(2, Trit::Zero);
    ftd::test::check("NOT(NOT(x)) = x", ternary_not(ternary_not(w)) == w);
}

static void test_and() {
    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};
    int expected[3][3] = {
        {-1, -1, -1},
        {-1,  0,  0},
        {-1,  0,  1},
    };

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa = single(vals[a]);
            TritWord64 wb = single(vals[b]);
            TritWord64 result = ternary_and(wa, wb);
            ftd::test::check("AND truth table",
                             to_int(result.get(0)) == expected[a][b]);
        }
    }

    TritWord64 x, y;
    x.set(0, Trit::Pos); x.set(1, Trit::Neg); x.set(2, Trit::Zero);
    y.set(0, Trit::Neg); y.set(1, Trit::Zero); y.set(2, Trit::Pos);
    ftd::test::check("AND is commutative", ternary_and(x, y) == ternary_and(y, x));
}

static void test_or() {
    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};
    int expected[3][3] = {
        {-1,  0,  1},
        { 0,  0,  1},
        { 1,  1,  1},
    };

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa = single(vals[a]);
            TritWord64 wb = single(vals[b]);
            TritWord64 result = ternary_or(wa, wb);
            ftd::test::check("OR truth table",
                             to_int(result.get(0)) == expected[a][b]);
        }
    }

    TritWord64 x, y;
    x.set(0, Trit::Pos); x.set(1, Trit::Neg);
    y.set(0, Trit::Neg); y.set(1, Trit::Pos);
    ftd::test::check("OR is commutative", ternary_or(x, y) == ternary_or(y, x));
}

static void test_consensus() {
    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa = single(vals[a]);
            TritWord64 wb = single(vals[b]);
            TritWord64 result = consensus(wa, wb);
            if (a == b) {
                ftd::test::check("consensus: agree -> keep",
                                 result.get(0) == vals[a]);
            } else {
                ftd::test::check("consensus: disagree -> 0",
                                 result.get(0) == Trit::Zero);
            }
        }
    }
}

static void test_demorgan() {
    Trit vals[3] = {Trit::Neg, Trit::Zero, Trit::Pos};

    for (int a = 0; a < 3; ++a) {
        for (int b = 0; b < 3; ++b) {
            TritWord64 wa = single(vals[a]);
            TritWord64 wb = single(vals[b]);

            TritWord64 lhs1 = ternary_not(ternary_and(wa, wb));
            TritWord64 rhs1 = ternary_or(ternary_not(wa), ternary_not(wb));
            ftd::test::check("De Morgan 1", lhs1.get(0) == rhs1.get(0));

            TritWord64 lhs2 = ternary_not(ternary_or(wa, wb));
            TritWord64 rhs2 = ternary_and(ternary_not(wa), ternary_not(wb));
            ftd::test::check("De Morgan 2", lhs2.get(0) == rhs2.get(0));
        }
    }
}

// --- Section: vector  (from test_trit_vector.cpp) ---

static void test_vec_basic_access() {
    TritVector v(10);
    for (size_t i = 0; i < 10; ++i)
        ftd::test::check("default zero", v[i] == Trit::Zero);

    v.set(0, Trit::Pos);
    v.set(5, Trit::Neg);
    v.set(9, Trit::Pos);
    ftd::test::check("set/get 0", v[0] == Trit::Pos);
    ftd::test::check("set/get 5", v[5] == Trit::Neg);
    ftd::test::check("set/get 9", v[9] == Trit::Pos);
    ftd::test::check("untouched is zero", v[3] == Trit::Zero);
}

static void test_construct_from_trits() {
    Trit data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Neg, Trit::Pos};
    TritVector v(data, 5);
    ftd::test::check("size", v.size() == 5);
    ftd::test::check("element 0", v[0] == Trit::Pos);
    ftd::test::check("element 1", v[1] == Trit::Neg);
    ftd::test::check("element 2", v[2] == Trit::Zero);
}

static void test_construct_from_doubles() {
    double data[] = {1.0, -1.0, 0.0, 0.3, -0.8};
    TritVector v(data, 5, 0.5);
    ftd::test::check("1.0 -> Pos", v[0] == Trit::Pos);
    ftd::test::check("-1.0 -> Neg", v[1] == Trit::Neg);
    ftd::test::check("0.0 -> Zero", v[2] == Trit::Zero);
    ftd::test::check("0.3 -> Zero", v[3] == Trit::Zero);
    ftd::test::check("-0.8 -> Neg", v[4] == Trit::Neg);
}

static void test_dot_product() {
    Trit a_data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    Trit b_data[] = {Trit::Pos, Trit::Pos, Trit::Neg, Trit::Neg};
    TritVector a(a_data, 4);
    TritVector b(b_data, 4);
    ftd::test::check("dot product = -1", a.dot(b) == -1);

    ftd::test::check("dot(a,a) == l0_norm(a)",
                     a.dot(a) == static_cast<int>(a.l0_norm()));

    TritVector z(4);
    ftd::test::check("dot with zeros = 0", a.dot(z) == 0);

    TritVector neg_a = -a;
    ftd::test::check("dot(a, -a) == -dot(a,a)",
                     a.dot(neg_a) == -a.dot(a));
}

static void test_dot_product_large() {
    size_t n = 100;
    TritVector a(n), b(n);

    int expected = 0;
    for (size_t i = 0; i < n; ++i) {
        Trit ta = (i % 3 == 0) ? Trit::Pos : (i % 3 == 1) ? Trit::Neg : Trit::Zero;
        Trit tb = (i % 2 == 0) ? Trit::Pos : Trit::Neg;
        a.set(i, ta);
        b.set(i, tb);
        expected += to_int(ta) * to_int(tb);
    }

    ftd::test::check("large dot product matches naive", a.dot(b) == expected);
}

static void test_hamming() {
    Trit a_data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    Trit b_data[] = {Trit::Pos, Trit::Pos, Trit::Zero, Trit::Neg};
    TritVector a(a_data, 4);
    TritVector b(b_data, 4);
    ftd::test::check("hamming distance = 2", a.hamming(b) == 2);
    ftd::test::check("hamming(a,a) = 0", a.hamming(a) == 0);
}

static void test_l0_norm() {
    Trit data[] = {Trit::Pos, Trit::Zero, Trit::Neg, Trit::Zero, Trit::Pos};
    TritVector v(data, 5);
    ftd::test::check("3 nonzero trits", v.l0_norm() == 3);

    TritVector z(10);
    ftd::test::check("all-zero vector", z.l0_norm() == 0);
}

static void test_serialization() {
    size_t n = 37;
    TritVector original(n);
    Trit pattern[] = {Trit::Pos, Trit::Neg, Trit::Zero};
    for (size_t i = 0; i < n; ++i)
        original.set(i, pattern[i % 3]);

    auto packed = original.pack_to();

    TritVector restored = TritVector::unpack_from(packed.data(), n);

    for (size_t i = 0; i < n; ++i)
        ftd::test::check("serialization round-trip element",
                         original[i] == restored[i]);
}

static void test_negate_vector() {
    Trit data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    TritVector v(data, 4);
    TritVector neg = -v;
    ftd::test::check("negate Pos -> Neg", neg[0] == Trit::Neg);
    ftd::test::check("negate Neg -> Pos", neg[1] == Trit::Pos);
    ftd::test::check("negate Zero -> Zero", neg[2] == Trit::Zero);
    ftd::test::check("negate Pos -> Neg", neg[3] == Trit::Neg);

    TritVector nn = -(-v);
    for (size_t i = 0; i < 4; ++i)
        ftd::test::check("double negation is identity", nn[i] == v[i]);
}

// --- Section: matrix  (from test_trit_matrix.cpp) ---

static void test_mat_basic_access() {
    TritMatrix m(3, 4);
    ftd::test::check("rows", m.rows() == 3);
    ftd::test::check("cols", m.cols() == 4);

    for (size_t r = 0; r < 3; ++r)
        for (size_t c = 0; c < 4; ++c)
            ftd::test::check("default zero", m.get(r, c) == Trit::Zero);

    m.set(0, 0, Trit::Pos);
    m.set(1, 2, Trit::Neg);
    m.set(2, 3, Trit::Pos);
    ftd::test::check("set/get", m.get(0, 0) == Trit::Pos);
    ftd::test::check("set/get", m.get(1, 2) == Trit::Neg);
}

static void test_mat_construct_from_array() {
    Trit data[] = {
        Trit::Pos, Trit::Neg, Trit::Zero,
        Trit::Zero, Trit::Pos, Trit::Neg
    };
    TritMatrix m(data, 2, 3);
    ftd::test::check("(0,0)", m.get(0, 0) == Trit::Pos);
    ftd::test::check("(0,1)", m.get(0, 1) == Trit::Neg);
    ftd::test::check("(1,1)", m.get(1, 1) == Trit::Pos);
    ftd::test::check("(1,2)", m.get(1, 2) == Trit::Neg);
}

static void test_matvec() {
    Trit m_data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    TritMatrix m(m_data, 2, 2);

    Trit v_data[] = {Trit::Pos, Trit::Pos};
    TritVector v(v_data, 2);

    auto result = m.matvec(v);
    ftd::test::check("row 0 dot = 0", result[0] == 0);
    ftd::test::check("row 1 dot = 1", result[1] == 1);
}

static void test_matvec_identity() {
    TritMatrix eye(3, 3);
    eye.set(0, 0, Trit::Pos);
    eye.set(1, 1, Trit::Pos);
    eye.set(2, 2, Trit::Pos);

    Trit v_data[] = {Trit::Pos, Trit::Neg, Trit::Zero};
    TritVector v(v_data, 3);

    auto result = eye.matvec(v);
    ftd::test::check("identity preserves element 0", result[0] == 1);
    ftd::test::check("identity preserves element 1", result[1] == -1);
    ftd::test::check("identity preserves element 2", result[2] == 0);
}

static void test_matmul() {
    Trit a_data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    Trit b_data[] = {Trit::Pos, Trit::Zero, Trit::Neg, Trit::Pos};
    TritMatrix A(a_data, 2, 2);
    TritMatrix B(b_data, 2, 2);

    auto C = A.matmul(B);
    ftd::test::check("C[0][0] = 2", C[0][0] == 2);
    ftd::test::check("C[0][1] = -1", C[0][1] == -1);
    ftd::test::check("C[1][0] = -1", C[1][0] == -1);
    ftd::test::check("C[1][1] = 1", C[1][1] == 1);
}

static void test_transpose() {
    Trit data[] = {
        Trit::Pos, Trit::Neg, Trit::Zero,
        Trit::Zero, Trit::Pos, Trit::Neg
    };
    TritMatrix m(data, 2, 3);
    TritMatrix mt = m.transpose();

    ftd::test::check("transposed rows", mt.rows() == 3);
    ftd::test::check("transposed cols", mt.cols() == 2);
    ftd::test::check("T(0,0)", mt.get(0, 0) == Trit::Pos);
    ftd::test::check("T(1,0)", mt.get(1, 0) == Trit::Neg);
    ftd::test::check("T(0,1)", mt.get(0, 1) == Trit::Zero);
    ftd::test::check("T(2,1)", mt.get(2, 1) == Trit::Neg);

    TritMatrix mtt = mt.transpose();
    for (size_t r = 0; r < 2; ++r)
        for (size_t c = 0; c < 3; ++c)
            ftd::test::check("double transpose identity",
                             mtt.get(r, c) == m.get(r, c));
}

// --- Section: threshold  (from test_trit_threshold.cpp) ---

static void test_hard_quantize() {
    double threshold = 0.5;

    ftd::test::check("1.0 > 0.5 -> Pos",
                     hard_quantize(1.0, threshold) == Trit::Pos);
    ftd::test::check("-1.0 < -0.5 -> Neg",
                     hard_quantize(-1.0, threshold) == Trit::Neg);
    ftd::test::check("0.3 in dead zone -> Zero",
                     hard_quantize(0.3, threshold) == Trit::Zero);
    ftd::test::check("-0.3 in dead zone -> Zero",
                     hard_quantize(-0.3, threshold) == Trit::Zero);
    ftd::test::check("0.0 -> Zero",
                     hard_quantize(0.0, threshold) == Trit::Zero);
    ftd::test::check("exactly at threshold -> Zero",
                     hard_quantize(0.5, threshold) == Trit::Zero);
    ftd::test::check("just above -> Pos",
                     hard_quantize(0.500001, threshold) == Trit::Pos);
}

static void test_batch_quantize() {
    double data[] = {1.0, -1.0, 0.0, 0.3, -0.8, 2.5};
    size_t n = 6;
    TritWord64 words[1];
    batch_quantize(data, words, n, 0.5);

    ftd::test::check("1.0 -> Pos", words[0].get(0) == Trit::Pos);
    ftd::test::check("-1.0 -> Neg", words[0].get(1) == Trit::Neg);
    ftd::test::check("0.0 -> Zero", words[0].get(2) == Trit::Zero);
    ftd::test::check("0.3 -> Zero", words[0].get(3) == Trit::Zero);
    ftd::test::check("-0.8 -> Neg", words[0].get(4) == Trit::Neg);
    ftd::test::check("2.5 -> Pos", words[0].get(5) == Trit::Pos);
}

static void test_stochastic_quantize() {
    std::mt19937 rng(42);
    double k_b = 0.511;

    // Zero input should always give Zero
    for (int i = 0; i < 100; ++i)
        ftd::test::check("0 always -> Zero",
                         stochastic_quantize(0.0, k_b, rng) == Trit::Zero);

    // Large positive input should almost always give Pos
    int pos_count = 0;
    for (int i = 0; i < 1000; ++i)
        if (stochastic_quantize(10.0, k_b, rng) == Trit::Pos) pos_count++;
    ftd::test::check("large positive should almost always manifest as Pos",
                     pos_count > 990);

    // Large negative input should almost always give Neg
    int neg_count = 0;
    for (int i = 0; i < 1000; ++i)
        if (stochastic_quantize(-10.0, k_b, rng) == Trit::Neg) neg_count++;
    ftd::test::check("large negative should almost always manifest as Neg",
                     neg_count > 990);

    // Small input (0.1) should manifest sometimes but not always
    int nonzero_count = 0;
    for (int i = 0; i < 1000; ++i)
        if (stochastic_quantize(0.1, k_b, rng) != Trit::Zero) nonzero_count++;
    ftd::test::check("small input should manifest occasionally",
                     nonzero_count > 50 && nonzero_count < 500);
}

static void test_dequantize() {
    ftd::test::check("Pos -> 1.0", dequantize(Trit::Pos, 1.0) == 1.0);
    ftd::test::check("Neg -> -1.0", dequantize(Trit::Neg, 1.0) == -1.0);
    ftd::test::check("Zero -> 0.0", dequantize(Trit::Zero, 1.0) == 0.0);
    ftd::test::check("Pos scaled -> 2.5", dequantize(Trit::Pos, 2.5) == 2.5);
    ftd::test::check("Neg scaled -> -0.511",
                     dequantize(Trit::Neg, 0.511) == -0.511);
}

static void test_roundtrip() {
    double data[] = {1.0, -1.0, 0.0, 0.3, -0.8};
    size_t n = 5;
    double threshold = 0.5;
    double scale = 1.0;

    TritWord64 words[1];
    batch_quantize(data, words, n, threshold);

    double restored[5];
    batch_dequantize(words, restored, n, scale);

    ftd::test::check("1.0 round-trips to 1.0", restored[0] == 1.0);
    ftd::test::check("-1.0 round-trips to -1.0", restored[1] == -1.0);
    ftd::test::check("0.0 round-trips to 0.0", restored[2] == 0.0);
    ftd::test::check("0.3 (below threshold) round-trips to 0.0",
                     restored[3] == 0.0);
    ftd::test::check("-0.8 round-trips to -1.0", restored[4] == -1.0);
}

// --- Section: convolution  (from test_trit_convolution.cpp) ---

static void test_conv1d_basic() {
    Trit in_data[] = {Trit::Pos, Trit::Neg, Trit::Pos, Trit::Neg, Trit::Pos};
    Trit k_data[] = {Trit::Pos, Trit::Pos};
    TritVector input(in_data, 5);
    TritVector kernel(k_data, 2);

    auto output = conv1d(input, kernel);
    ftd::test::check("output length = 4", output.size() == 4);
    ftd::test::check("conv[0] = 0", output[0] == 0);
    ftd::test::check("conv[1] = 0", output[1] == 0);
    ftd::test::check("conv[2] = 0", output[2] == 0);
    ftd::test::check("conv[3] = 0", output[3] == 0);
}

static void test_conv1d_identity() {
    Trit in_data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    Trit k_data[] = {Trit::Pos};
    TritVector input(in_data, 4);
    TritVector kernel(k_data, 1);

    auto output = conv1d(input, kernel);
    ftd::test::check("output length = 4", output.size() == 4);
    ftd::test::check("identity preserves", output[0] == 1);
    ftd::test::check("identity preserves", output[1] == -1);
    ftd::test::check("identity preserves", output[2] == 0);
    ftd::test::check("identity preserves", output[3] == 1);
}

static void test_conv1d_gradient() {
    Trit in_data[] = {Trit::Zero, Trit::Pos, Trit::Pos, Trit::Zero, Trit::Neg};
    Trit k_data[] = {Trit::Neg, Trit::Pos};
    TritVector input(in_data, 5);
    TritVector kernel(k_data, 2);

    auto output = conv1d(input, kernel);
    ftd::test::check("diff[0] = 1", output[0] == 1);
    ftd::test::check("diff[1] = 0", output[1] == 0);
    ftd::test::check("diff[2] = -1", output[2] == -1);
    ftd::test::check("diff[3] = -1", output[3] == -1);
}

static void test_conv2d_basic() {
    Trit in_data[] = {
        Trit::Pos, Trit::Neg, Trit::Zero,
        Trit::Zero, Trit::Pos, Trit::Neg,
        Trit::Neg, Trit::Zero, Trit::Pos
    };
    Trit k_data[] = {
        Trit::Pos, Trit::Pos,
        Trit::Pos, Trit::Pos
    };
    TritMatrix input(in_data, 3, 3);
    TritMatrix kernel(k_data, 2, 2);

    auto output = conv2d(input, kernel);
    ftd::test::check("output height = 2", output.size() == 2);
    ftd::test::check("output width = 2", output[0].size() == 2);
    ftd::test::check("conv2d[0][0] = 1", output[0][0] == 1);
    ftd::test::check("conv2d[0][1] = -1", output[0][1] == -1);
    ftd::test::check("conv2d[1][0] = 0", output[1][0] == 0);
    ftd::test::check("conv2d[1][1] = 1", output[1][1] == 1);
}

static void test_conv3d_basic() {
    Volume3D input;
    input.depth = 2; input.height = 2; input.width = 2;
    input.data = {Trit::Pos, Trit::Neg, Trit::Pos, Trit::Neg,
                  Trit::Neg, Trit::Pos, Trit::Neg, Trit::Pos};

    Volume3D kernel;
    kernel.depth = 2; kernel.height = 2; kernel.width = 2;
    kernel.data = {Trit::Pos, Trit::Pos, Trit::Pos, Trit::Pos,
                   Trit::Pos, Trit::Pos, Trit::Pos, Trit::Pos};

    auto output = conv3d(input, kernel);
    ftd::test::check("single output element", output.size() == 1);
    ftd::test::check("conv3d sum = 0", output[0] == 0);
}

static void test_conv3d_offset() {
    Volume3D input;
    input.depth = 3; input.height = 3; input.width = 3;
    input.data.resize(27, Trit::Pos);

    Volume3D kernel;
    kernel.depth = 2; kernel.height = 2; kernel.width = 2;
    kernel.data.resize(8, Trit::Pos);

    auto output = conv3d(input, kernel);
    size_t expected_size = 2 * 2 * 2;
    ftd::test::check("output size = 8", output.size() == expected_size);
    for (size_t i = 0; i < output.size(); ++i)
        ftd::test::check("all-ones conv = 8", output[i] == 8);
}

// --- main ---

int main() {
    ftd::test::init("test_tritium_algebra");

    ftd::test::section("packing");
    test_trit_conversion();
    test_tritword_get_set();
    test_tritword_splat();
    test_tritword_popcount();
    test_tritpack_get_set();
    test_pack_roundtrip_exhaustive();
    test_batch_pack_unpack();

    ftd::test::section("arithmetic");
    test_negate();
    test_abs();
    test_multiply();
    test_add_saturate();
    test_add_with_carry();

    ftd::test::section("logic");
    test_not();
    test_and();
    test_or();
    test_consensus();
    test_demorgan();

    ftd::test::section("vector");
    test_vec_basic_access();
    test_construct_from_trits();
    test_construct_from_doubles();
    test_dot_product();
    test_dot_product_large();
    test_hamming();
    test_l0_norm();
    test_serialization();
    test_negate_vector();

    ftd::test::section("matrix");
    test_mat_basic_access();
    test_mat_construct_from_array();
    test_matvec();
    test_matvec_identity();
    test_matmul();
    test_transpose();

    ftd::test::section("threshold");
    test_hard_quantize();
    test_batch_quantize();
    test_stochastic_quantize();
    test_dequantize();
    test_roundtrip();

    ftd::test::section("convolution");
    test_conv1d_basic();
    test_conv1d_identity();
    test_conv1d_gradient();
    test_conv2d_basic();
    test_conv3d_basic();
    test_conv3d_offset();

    return ftd::test::finalize();
}
