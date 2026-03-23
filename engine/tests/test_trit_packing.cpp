// test_trit_packing.cpp — Tests for core trit types and pack/unpack conversion
#include <tritium/tritium.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>

using namespace tritium;

static int failures = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::printf("  FAIL: %s (line %d)\n", msg, __LINE__); \
        failures++; \
    } \
} while(0)

// ============================================================================
// TritWord64 basic operations
// ============================================================================
void test_tritword_get_set() {
    std::printf("  TritWord64 get/set...\n");
    TritWord64 w;
    // All positions should start as Zero
    for (int i = 0; i < 32; ++i)
        CHECK(w.get(i) == Trit::Zero, "default should be Zero");

    // Set and read back each position
    w.set(0, Trit::Pos);
    w.set(1, Trit::Neg);
    w.set(31, Trit::Pos);
    CHECK(w.get(0) == Trit::Pos, "position 0 should be Pos");
    CHECK(w.get(1) == Trit::Neg, "position 1 should be Neg");
    CHECK(w.get(31) == Trit::Pos, "position 31 should be Pos");
    CHECK(w.get(2) == Trit::Zero, "position 2 should still be Zero");
}

void test_tritword_splat() {
    std::printf("  TritWord64 splat...\n");
    TritWord64 z = TritWord64::splat(Trit::Zero);
    TritWord64 p = TritWord64::splat(Trit::Pos);
    TritWord64 n = TritWord64::splat(Trit::Neg);

    for (int i = 0; i < 32; ++i) {
        CHECK(z.get(i) == Trit::Zero, "splat Zero");
        CHECK(p.get(i) == Trit::Pos, "splat Pos");
        CHECK(n.get(i) == Trit::Neg, "splat Neg");
    }
}

void test_tritword_popcount() {
    std::printf("  TritWord64 popcount...\n");
    TritWord64 w;
    CHECK(w.popcount_nonzero() == 0, "empty word has 0 nonzero");
    CHECK(w.popcount_pos() == 0, "empty word has 0 pos");
    CHECK(w.popcount_neg() == 0, "empty word has 0 neg");

    w.set(0, Trit::Pos);
    w.set(1, Trit::Neg);
    w.set(2, Trit::Pos);
    CHECK(w.popcount_nonzero() == 3, "3 nonzero trits");
    CHECK(w.popcount_pos() == 2, "2 positive trits");
    CHECK(w.popcount_neg() == 1, "1 negative trit");
}

// ============================================================================
// TritPack basic operations
// ============================================================================
void test_tritpack_get_set() {
    std::printf("  TritPack get/set...\n");
    TritPack p;
    // Default: all zeros (byte = 121)
    for (int i = 0; i < 5; ++i)
        CHECK(p.get(i) == Trit::Zero, "default should be Zero");

    p.set(0, Trit::Pos);
    p.set(1, Trit::Neg);
    p.set(4, Trit::Pos);
    CHECK(p.get(0) == Trit::Pos, "position 0");
    CHECK(p.get(1) == Trit::Neg, "position 1");
    CHECK(p.get(2) == Trit::Zero, "position 2");
    CHECK(p.get(4) == Trit::Pos, "position 4");
}

// ============================================================================
// Exhaustive round-trip: all 243 TritPack values
// ============================================================================
void test_pack_roundtrip_exhaustive() {
    std::printf("  Exhaustive 243-value round-trip...\n");
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

        CHECK(p.byte < 243, "byte must be < 243");

        // Decode via LUT and compare
        Trit decoded[5];
        decode_pack(p.byte, decoded);
        CHECK(decoded[0] == values[a], "round-trip pos 0");
        CHECK(decoded[1] == values[b], "round-trip pos 1");
        CHECK(decoded[2] == values[c], "round-trip pos 2");
        CHECK(decoded[3] == values[d], "round-trip pos 3");
        CHECK(decoded[4] == values[e], "round-trip pos 4");
        count++;
    }
    CHECK(count == 243, "should test all 243 combinations");
}

// ============================================================================
// Batch pack/unpack conversion
// ============================================================================
void test_batch_pack_unpack() {
    std::printf("  Batch pack/unpack...\n");
    const size_t N = 100;

    // Create compute-format data
    TritWord64 words[4]; // 4 words = 128 trit slots, we use 100
    for (size_t i = 0; i < 4; ++i) words[i].bits = 0;

    Trit pattern[3] = {Trit::Neg, Trit::Zero, Trit::Pos};
    for (size_t i = 0; i < N; ++i) {
        size_t wi = i / TritWord64::CAPACITY;
        int bi = static_cast<int>(i % TritWord64::CAPACITY);
        words[wi].set(bi, pattern[i % 3]);
    }

    // Pack to storage format
    size_t n_packs = packed_size(N); // ceil(100/5) = 20
    TritPack packs[20];
    pack(words, packs, N);

    // Unpack back to compute format
    TritWord64 restored[4];
    unpack(packs, restored, N);

    // Verify round-trip
    for (size_t i = 0; i < N; ++i) {
        Trit original = words[i / TritWord64::CAPACITY].get(
            static_cast<int>(i % TritWord64::CAPACITY));
        Trit roundtrip = restored[i / TritWord64::CAPACITY].get(
            static_cast<int>(i % TritWord64::CAPACITY));
        CHECK(original == roundtrip, "batch round-trip mismatch");
    }
}

// ============================================================================
// to_trit / to_int conversion
// ============================================================================
void test_trit_conversion() {
    std::printf("  Trit conversion...\n");
    CHECK(to_int(Trit::Neg) == -1, "Neg -> -1");
    CHECK(to_int(Trit::Zero) == 0, "Zero -> 0");
    CHECK(to_int(Trit::Pos) == 1, "Pos -> 1");
    CHECK(to_trit(-5) == Trit::Neg, "negative -> Neg");
    CHECK(to_trit(0) == Trit::Zero, "zero -> Zero");
    CHECK(to_trit(42) == Trit::Pos, "positive -> Pos");
    CHECK(-Trit::Pos == Trit::Neg, "negate Pos");
    CHECK(-Trit::Neg == Trit::Pos, "negate Neg");
    CHECK(-Trit::Zero == Trit::Zero, "negate Zero");
}

int main() {
    std::printf("=== Tritium: Packing Tests ===\n");

    test_trit_conversion();
    test_tritword_get_set();
    test_tritword_splat();
    test_tritword_popcount();
    test_tritpack_get_set();
    test_pack_roundtrip_exhaustive();
    test_batch_pack_unpack();

    if (failures == 0) {
        std::printf("All packing tests PASSED\n");
        return 0;
    } else {
        std::printf("%d packing test(s) FAILED\n", failures);
        return 1;
    }
}
