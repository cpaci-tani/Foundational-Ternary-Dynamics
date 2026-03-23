// test_trit_convolution.cpp — Tests for 1D/2D/3D ternary convolution
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

void test_conv1d_basic() {
    std::printf("  Conv1D basic...\n");
    // input:  [+1, -1, +1, -1, +1]
    // kernel: [+1, +1]
    // output: [1+(-1), -1+1, 1+(-1), -1+1] = [0, 0, 0, 0]
    Trit in_data[] = {Trit::Pos, Trit::Neg, Trit::Pos, Trit::Neg, Trit::Pos};
    Trit k_data[] = {Trit::Pos, Trit::Pos};
    TritVector input(in_data, 5);
    TritVector kernel(k_data, 2);

    auto output = conv1d(input, kernel);
    CHECK(output.size() == 4, "output length = 4");
    CHECK(output[0] == 0, "conv[0] = 0");
    CHECK(output[1] == 0, "conv[1] = 0");
    CHECK(output[2] == 0, "conv[2] = 0");
    CHECK(output[3] == 0, "conv[3] = 0");
}

void test_conv1d_identity() {
    std::printf("  Conv1D identity kernel...\n");
    // Kernel [+1] is identity
    Trit in_data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    Trit k_data[] = {Trit::Pos};
    TritVector input(in_data, 4);
    TritVector kernel(k_data, 1);

    auto output = conv1d(input, kernel);
    CHECK(output.size() == 4, "output length = 4");
    CHECK(output[0] == 1, "identity preserves");
    CHECK(output[1] == -1, "identity preserves");
    CHECK(output[2] == 0, "identity preserves");
    CHECK(output[3] == 1, "identity preserves");
}

void test_conv1d_gradient() {
    std::printf("  Conv1D gradient (difference kernel)...\n");
    // Kernel [-1, +1] = forward difference
    // input: [0, +1, +1, 0, -1]
    // output: [1-0, 1-1, 0-1, -1-0] = [1, 0, -1, -1]
    Trit in_data[] = {Trit::Zero, Trit::Pos, Trit::Pos, Trit::Zero, Trit::Neg};
    Trit k_data[] = {Trit::Neg, Trit::Pos};
    TritVector input(in_data, 5);
    TritVector kernel(k_data, 2);

    auto output = conv1d(input, kernel);
    CHECK(output[0] == 1, "diff[0] = 1");
    CHECK(output[1] == 0, "diff[1] = 0");
    CHECK(output[2] == -1, "diff[2] = -1");
    CHECK(output[3] == -1, "diff[3] = -1");
}

void test_conv2d_basic() {
    std::printf("  Conv2D basic...\n");
    // 3x3 input, 2x2 kernel
    // Input:  +1 -1  0
    //          0 +1 -1
    //         -1  0 +1
    // Kernel: +1 +1
    //         +1 +1
    // output[0][0] = 1+(-1)+0+1 = 1
    // output[0][1] = -1+0+1+(-1) = -1
    // output[1][0] = 0+1+(-1)+0 = 0
    // output[1][1] = 1+(-1)+0+1 = 1
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
    CHECK(output.size() == 2, "output height = 2");
    CHECK(output[0].size() == 2, "output width = 2");
    CHECK(output[0][0] == 1, "conv2d[0][0] = 1");
    CHECK(output[0][1] == -1, "conv2d[0][1] = -1");
    CHECK(output[1][0] == 0, "conv2d[1][0] = 0");
    CHECK(output[1][1] == 1, "conv2d[1][1] = 1");
}

void test_conv3d_basic() {
    std::printf("  Conv3D basic...\n");
    // 2x2x2 input, 2x2x2 kernel (full overlap = single output value)
    Volume3D input;
    input.depth = 2; input.height = 2; input.width = 2;
    input.data = {Trit::Pos, Trit::Neg, Trit::Pos, Trit::Neg,
                  Trit::Neg, Trit::Pos, Trit::Neg, Trit::Pos};

    Volume3D kernel;
    kernel.depth = 2; kernel.height = 2; kernel.width = 2;
    kernel.data = {Trit::Pos, Trit::Pos, Trit::Pos, Trit::Pos,
                   Trit::Pos, Trit::Pos, Trit::Pos, Trit::Pos};

    auto output = conv3d(input, kernel);
    // Sum of all input elements: 1-1+1-1-1+1-1+1 = 0
    CHECK(output.size() == 1, "single output element");
    CHECK(output[0] == 0, "conv3d sum = 0");
}

void test_conv3d_offset() {
    std::printf("  Conv3D with stride...\n");
    // 3x3x3 input, 2x2x2 kernel -> 2x2x2 output
    Volume3D input;
    input.depth = 3; input.height = 3; input.width = 3;
    input.data.resize(27, Trit::Pos); // all +1

    Volume3D kernel;
    kernel.depth = 2; kernel.height = 2; kernel.width = 2;
    kernel.data.resize(8, Trit::Pos); // all +1

    auto output = conv3d(input, kernel);
    size_t expected_size = 2 * 2 * 2;
    CHECK(output.size() == expected_size, "output size = 8");
    // Each position sums 8 ones = 8
    for (size_t i = 0; i < output.size(); ++i)
        CHECK(output[i] == 8, "all-ones conv = 8");
}

int main() {
    std::printf("=== Tritium: Convolution Tests ===\n");

    test_conv1d_basic();
    test_conv1d_identity();
    test_conv1d_gradient();
    test_conv2d_basic();
    test_conv3d_basic();
    test_conv3d_offset();

    if (failures == 0) {
        std::printf("All convolution tests PASSED\n");
        return 0;
    } else {
        std::printf("%d convolution test(s) FAILED\n", failures);
        return 1;
    }
}
