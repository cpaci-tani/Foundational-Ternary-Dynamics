// test_trit_matrix.cpp — Tests for TritMatrix operations
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

void test_basic_access() {
    std::printf("  Basic access...\n");
    TritMatrix m(3, 4);
    CHECK(m.rows() == 3, "rows");
    CHECK(m.cols() == 4, "cols");

    // Default: all zeros
    for (size_t r = 0; r < 3; ++r)
        for (size_t c = 0; c < 4; ++c)
            CHECK(m.get(r, c) == Trit::Zero, "default zero");

    m.set(0, 0, Trit::Pos);
    m.set(1, 2, Trit::Neg);
    m.set(2, 3, Trit::Pos);
    CHECK(m.get(0, 0) == Trit::Pos, "set/get");
    CHECK(m.get(1, 2) == Trit::Neg, "set/get");
}

void test_construct_from_array() {
    std::printf("  Construct from array...\n");
    // 2x3 matrix:
    //  +1  -1   0
    //   0  +1  -1
    Trit data[] = {
        Trit::Pos, Trit::Neg, Trit::Zero,
        Trit::Zero, Trit::Pos, Trit::Neg
    };
    TritMatrix m(data, 2, 3);
    CHECK(m.get(0, 0) == Trit::Pos, "(0,0)");
    CHECK(m.get(0, 1) == Trit::Neg, "(0,1)");
    CHECK(m.get(1, 1) == Trit::Pos, "(1,1)");
    CHECK(m.get(1, 2) == Trit::Neg, "(1,2)");
}

void test_matvec() {
    std::printf("  Matrix-vector multiply...\n");
    // M = [[+1, -1], [0, +1]], v = [+1, +1]
    // M*v = [1*1 + (-1)*1, 0*1 + 1*1] = [0, 1]
    Trit m_data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    TritMatrix m(m_data, 2, 2);

    Trit v_data[] = {Trit::Pos, Trit::Pos};
    TritVector v(v_data, 2);

    auto result = m.matvec(v);
    CHECK(result[0] == 0, "row 0 dot = 0");
    CHECK(result[1] == 1, "row 1 dot = 1");
}

void test_matvec_identity() {
    std::printf("  Matvec with identity-like matrix...\n");
    // 3x3 identity (diagonal +1)
    TritMatrix eye(3, 3);
    eye.set(0, 0, Trit::Pos);
    eye.set(1, 1, Trit::Pos);
    eye.set(2, 2, Trit::Pos);

    Trit v_data[] = {Trit::Pos, Trit::Neg, Trit::Zero};
    TritVector v(v_data, 3);

    auto result = eye.matvec(v);
    CHECK(result[0] == 1, "identity preserves element 0");
    CHECK(result[1] == -1, "identity preserves element 1");
    CHECK(result[2] == 0, "identity preserves element 2");
}

void test_matmul() {
    std::printf("  Matrix-matrix multiply...\n");
    // A = 2x2, B = 2x2
    // A = [[+1, -1], [0, +1]]
    // B = [[+1, 0], [-1, +1]]
    // C = A*B = [[1*1+(-1)*(-1), 1*0+(-1)*1], [0*1+1*(-1), 0*0+1*1]]
    //         = [[2, -1], [-1, 1]]
    Trit a_data[] = {Trit::Pos, Trit::Neg, Trit::Zero, Trit::Pos};
    Trit b_data[] = {Trit::Pos, Trit::Zero, Trit::Neg, Trit::Pos};
    TritMatrix A(a_data, 2, 2);
    TritMatrix B(b_data, 2, 2);

    auto C = A.matmul(B);
    CHECK(C[0][0] == 2, "C[0][0] = 2");
    CHECK(C[0][1] == -1, "C[0][1] = -1");
    CHECK(C[1][0] == -1, "C[1][0] = -1");
    CHECK(C[1][1] == 1, "C[1][1] = 1");
}

void test_transpose() {
    std::printf("  Transpose...\n");
    // 2x3 -> 3x2
    Trit data[] = {
        Trit::Pos, Trit::Neg, Trit::Zero,
        Trit::Zero, Trit::Pos, Trit::Neg
    };
    TritMatrix m(data, 2, 3);
    TritMatrix mt = m.transpose();

    CHECK(mt.rows() == 3, "transposed rows");
    CHECK(mt.cols() == 2, "transposed cols");
    CHECK(mt.get(0, 0) == Trit::Pos, "T(0,0)");
    CHECK(mt.get(1, 0) == Trit::Neg, "T(1,0)");
    CHECK(mt.get(0, 1) == Trit::Zero, "T(0,1)");
    CHECK(mt.get(2, 1) == Trit::Neg, "T(2,1)");

    // Double transpose = original
    TritMatrix mtt = mt.transpose();
    for (size_t r = 0; r < 2; ++r)
        for (size_t c = 0; c < 3; ++c)
            CHECK(mtt.get(r, c) == m.get(r, c), "double transpose identity");
}

int main() {
    std::printf("=== Tritium: Matrix Tests ===\n");

    test_basic_access();
    test_construct_from_array();
    test_matvec();
    test_matvec_identity();
    test_matmul();
    test_transpose();

    if (failures == 0) {
        std::printf("All matrix tests PASSED\n");
        return 0;
    } else {
        std::printf("%d matrix test(s) FAILED\n", failures);
        return 1;
    }
}
