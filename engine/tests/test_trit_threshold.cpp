// test_trit_threshold.cpp — Tests for hard and stochastic quantization
#include <tritium/tritium.h>
#include <cstdio>
#include <cmath>
#include <random>

using namespace tritium;

static int failures = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::printf("  FAIL: %s (line %d)\n", msg, __LINE__); \
        failures++; \
    } \
} while(0)

void test_hard_quantize() {
    std::printf("  Hard quantize...\n");
    double threshold = 0.5;

    CHECK(hard_quantize(1.0, threshold) == Trit::Pos, "1.0 > 0.5 -> Pos");
    CHECK(hard_quantize(-1.0, threshold) == Trit::Neg, "-1.0 < -0.5 -> Neg");
    CHECK(hard_quantize(0.3, threshold) == Trit::Zero, "0.3 in dead zone -> Zero");
    CHECK(hard_quantize(-0.3, threshold) == Trit::Zero, "-0.3 in dead zone -> Zero");
    CHECK(hard_quantize(0.0, threshold) == Trit::Zero, "0.0 -> Zero");
    CHECK(hard_quantize(0.5, threshold) == Trit::Zero, "exactly at threshold -> Zero");
    CHECK(hard_quantize(0.500001, threshold) == Trit::Pos, "just above -> Pos");
}

void test_batch_quantize() {
    std::printf("  Batch quantize...\n");
    double data[] = {1.0, -1.0, 0.0, 0.3, -0.8, 2.5};
    size_t n = 6;
    TritWord64 words[1];
    batch_quantize(data, words, n, 0.5);

    CHECK(words[0].get(0) == Trit::Pos, "1.0 -> Pos");
    CHECK(words[0].get(1) == Trit::Neg, "-1.0 -> Neg");
    CHECK(words[0].get(2) == Trit::Zero, "0.0 -> Zero");
    CHECK(words[0].get(3) == Trit::Zero, "0.3 -> Zero");
    CHECK(words[0].get(4) == Trit::Neg, "-0.8 -> Neg");
    CHECK(words[0].get(5) == Trit::Pos, "2.5 -> Pos");
}

void test_stochastic_quantize() {
    std::printf("  Stochastic quantize...\n");
    std::mt19937 rng(42);
    double k_b = 0.511;

    // Zero input should always give Zero
    for (int i = 0; i < 100; ++i)
        CHECK(stochastic_quantize(0.0, k_b, rng) == Trit::Zero, "0 always -> Zero");

    // Large positive input should almost always give Pos
    int pos_count = 0;
    for (int i = 0; i < 1000; ++i)
        if (stochastic_quantize(10.0, k_b, rng) == Trit::Pos) pos_count++;
    CHECK(pos_count > 990, "large positive should almost always manifest as Pos");

    // Large negative input should almost always give Neg
    int neg_count = 0;
    for (int i = 0; i < 1000; ++i)
        if (stochastic_quantize(-10.0, k_b, rng) == Trit::Neg) neg_count++;
    CHECK(neg_count > 990, "large negative should almost always manifest as Neg");

    // Small input (0.1) should manifest sometimes but not always
    int nonzero_count = 0;
    for (int i = 0; i < 1000; ++i)
        if (stochastic_quantize(0.1, k_b, rng) != Trit::Zero) nonzero_count++;
    CHECK(nonzero_count > 50 && nonzero_count < 500,
          "small input should manifest occasionally");
}

void test_dequantize() {
    std::printf("  Dequantize...\n");
    CHECK(dequantize(Trit::Pos, 1.0) == 1.0, "Pos -> 1.0");
    CHECK(dequantize(Trit::Neg, 1.0) == -1.0, "Neg -> -1.0");
    CHECK(dequantize(Trit::Zero, 1.0) == 0.0, "Zero -> 0.0");
    CHECK(dequantize(Trit::Pos, 2.5) == 2.5, "Pos scaled -> 2.5");
    CHECK(dequantize(Trit::Neg, 0.511) == -0.511, "Neg scaled -> -0.511");
}

void test_roundtrip() {
    std::printf("  Quantize/dequantize round-trip...\n");
    double data[] = {1.0, -1.0, 0.0, 0.3, -0.8};
    size_t n = 5;
    double threshold = 0.5;
    double scale = 1.0;

    TritWord64 words[1];
    batch_quantize(data, words, n, threshold);

    double restored[5];
    batch_dequantize(words, restored, n, scale);

    // Values above threshold should reconstruct to ±1
    CHECK(restored[0] == 1.0, "1.0 round-trips to 1.0");
    CHECK(restored[1] == -1.0, "-1.0 round-trips to -1.0");
    CHECK(restored[2] == 0.0, "0.0 round-trips to 0.0");
    CHECK(restored[3] == 0.0, "0.3 (below threshold) round-trips to 0.0");
    CHECK(restored[4] == -1.0, "-0.8 round-trips to -1.0");
}

int main() {
    std::printf("=== Tritium: Threshold Tests ===\n");

    test_hard_quantize();
    test_batch_quantize();
    test_stochastic_quantize();
    test_dequantize();
    test_roundtrip();

    if (failures == 0) {
        std::printf("All threshold tests PASSED\n");
        return 0;
    } else {
        std::printf("%d threshold test(s) FAILED\n", failures);
        return 1;
    }
}
