// test_trit_vector.cpp — Tests for TritVector operations
#include <tritium/tritium.h>
#include <cstdio>
#include <vector>

using namespace tritium;

static int failures = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::printf("  FAIL: %s (line %d)\n", msg, __LINE__); \
        failures++; \
    } \
} while(0)

void test_basic_access() {
    std::printf("  Basic access...\n");
    TritVector v(10);
    // Default: all zeros
    for (size_t i = 0; i < 10; ++i)
        CHECK(v[i] == Trit::Zero, "default zero");

    v.set(0, Trit::Pos);
    v.set(5, Trit::Neg);
    v.set(9, Trit::Pos);
    CHECK(v[0] == Trit::Pos, "set/get 0");
    CHECK(v[5] == Trit::Neg, "set/get 5");
    CHECK(v[9] == Trit::Pos, "set/get 9");
    CHECK(v[3] == Trit::Zero, "untouched is zero");
}

void test_construct_from_trits() {
    std::printf("  Construct from Trit array...\n");
    Trit data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Neg, Trit::Pos};
    TritVector v(data, 5);
    CHECK(v.size() == 5, "size");
    CHECK(v[0] == Trit::Pos, "element 0");
    CHECK(v[1] == Trit::Neg, "element 1");
    CHECK(v[2] == Trit::Zero, "element 2");
}

void test_construct_from_doubles() {
    std::printf("  Construct from doubles...\n");
    double data[] = {1.0, -1.0, 0.0, 0.3, -0.8};
    TritVector v(data, 5, 0.5);
    CHECK(v[0] == Trit::Pos, "1.0 -> Pos");
    CHECK(v[1] == Trit::Neg, "-1.0 -> Neg");
    CHECK(v[2] == Trit::Zero, "0.0 -> Zero");
    CHECK(v[3] == Trit::Zero, "0.3 -> Zero");
    CHECK(v[4] == Trit::Neg, "-0.8 -> Neg");
}

void test_dot_product() {
    std::printf("  Dot product...\n");
    // Manual: [+1, -1, 0, +1] . [+1, +1, -1, -1]
    // = (+1)(+1) + (-1)(+1) + (0)(-1) + (+1)(-1) = 1 - 1 + 0 - 1 = -1
    Trit a_data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    Trit b_data[] = {Trit::Pos, Trit::Pos, Trit::Neg, Trit::Neg};
    TritVector a(a_data, 4);
    TritVector b(b_data, 4);
    CHECK(a.dot(b) == -1, "dot product = -1");

    // Dot with self: sum of squares, which for trits is just l0_norm
    CHECK(a.dot(a) == a.l0_norm(), "dot(a,a) == l0_norm(a)");

    // Dot with all zeros = 0
    TritVector z(4);
    CHECK(a.dot(z) == 0, "dot with zeros = 0");

    // Dot with negation = -dot(a,a)
    TritVector neg_a = -a;
    CHECK(a.dot(neg_a) == -a.dot(a), "dot(a, -a) == -dot(a,a)");
}

void test_dot_product_large() {
    std::printf("  Dot product (large, multi-word)...\n");
    // Create vectors larger than 32 trits to test multi-word dot
    size_t n = 100;
    TritVector a(n), b(n);

    // Fill with alternating pattern
    int expected = 0;
    for (size_t i = 0; i < n; ++i) {
        Trit ta = (i % 3 == 0) ? Trit::Pos : (i % 3 == 1) ? Trit::Neg : Trit::Zero;
        Trit tb = (i % 2 == 0) ? Trit::Pos : Trit::Neg;
        a.set(i, ta);
        b.set(i, tb);
        expected += to_int(ta) * to_int(tb);
    }

    CHECK(a.dot(b) == expected, "large dot product matches naive");
}

void test_hamming() {
    std::printf("  Hamming distance...\n");
    Trit a_data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    Trit b_data[] = {Trit::Pos, Trit::Pos, Trit::Zero, Trit::Neg};
    TritVector a(a_data, 4);
    TritVector b(b_data, 4);
    // Positions 1 and 3 differ
    CHECK(a.hamming(b) == 2, "hamming distance = 2");

    // Same vector: distance 0
    CHECK(a.hamming(a) == 0, "hamming(a,a) = 0");
}

void test_l0_norm() {
    std::printf("  L0 norm...\n");
    Trit data[] = {Trit::Pos, Trit::Zero, Trit::Neg, Trit::Zero, Trit::Pos};
    TritVector v(data, 5);
    CHECK(v.l0_norm() == 3, "3 nonzero trits");

    TritVector z(10);
    CHECK(z.l0_norm() == 0, "all-zero vector");
}

void test_serialization() {
    std::printf("  Serialization round-trip...\n");
    size_t n = 37; // deliberately not a multiple of 5 or 32
    TritVector original(n);
    Trit pattern[] = {Trit::Pos, Trit::Neg, Trit::Zero};
    for (size_t i = 0; i < n; ++i)
        original.set(i, pattern[i % 3]);

    // Pack to storage format
    auto packed = original.pack_to();

    // Unpack back
    TritVector restored = TritVector::unpack_from(packed.data(), n);

    // Verify
    for (size_t i = 0; i < n; ++i)
        CHECK(original[i] == restored[i], "serialization round-trip element");
}

void test_negate_vector() {
    std::printf("  Vector negation...\n");
    Trit data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    TritVector v(data, 4);
    TritVector neg = -v;
    CHECK(neg[0] == Trit::Neg, "negate Pos -> Neg");
    CHECK(neg[1] == Trit::Pos, "negate Neg -> Pos");
    CHECK(neg[2] == Trit::Zero, "negate Zero -> Zero");
    CHECK(neg[3] == Trit::Neg, "negate Pos -> Neg");

    // Double negation
    TritVector nn = -(-v);
    for (size_t i = 0; i < 4; ++i)
        CHECK(nn[i] == v[i], "double negation is identity");
}

int main() {
    std::printf("=== Tritium: Vector Tests ===\n");

    test_basic_access();
    test_construct_from_trits();
    test_construct_from_doubles();
    test_dot_product();
    test_dot_product_large();
    test_hamming();
    test_l0_norm();
    test_serialization();
    test_negate_vector();

    if (failures == 0) {
        std::printf("All vector tests PASSED\n");
        return 0;
    } else {
        std::printf("%d vector test(s) FAILED\n", failures);
        return 1;
    }
}
