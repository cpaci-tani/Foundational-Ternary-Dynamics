#include "ftd/eft/conserved_charge_basis.h"

#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <stdexcept>

namespace ftd {
namespace eft {

namespace {

struct Rational {
    std::int64_t n = 0;
    std::int64_t d = 1;

    Rational() = default;
    Rational(std::int64_t value) : n(value) {}
    Rational(std::int64_t num, std::int64_t den) : n(num), d(den) {
        normalize();
    }

    void normalize() {
        if (d == 0) throw std::runtime_error("zero rational denominator");
        if (d < 0) { n = -n; d = -d; }
        const auto g = std::gcd(std::llabs(n), d);
        if (g != 0) { n /= g; d /= g; }
    }
};

Rational operator+(const Rational& a, const Rational& b) {
    return {a.n * b.d + b.n * a.d, a.d * b.d};
}
Rational operator-(const Rational& a, const Rational& b) {
    return {a.n * b.d - b.n * a.d, a.d * b.d};
}
Rational operator*(const Rational& a, const Rational& b) {
    return {a.n * b.n, a.d * b.d};
}
Rational operator/(const Rational& a, const Rational& b) {
    return {a.n * b.d, a.d * b.n};
}
bool is_zero(const Rational& a) { return a.n == 0; }

std::int64_t lcm_checked(std::int64_t a, std::int64_t b) {
    if (a == 0 || b == 0) return 0;
    return std::llabs(a / std::gcd(a, b) * b);
}

}  // namespace

std::vector<ChargeTransition> frozen_native_charge_transitions() {
    // Movement has zero global delta and is omitted from rank because it adds
    // no constraint. Evaporation and annihilation are the negatives of the
    // corresponding creation rows and therefore do not change the row space.
    return {
        {"single_genesis_plus",  {1,  1,  0, 0}},
        {"single_genesis_minus", {1, -1,  0, 0}},
        {"dual_genesis_plus",    {1,  1,  1, 1}},
        {"dual_genesis_minus",   {1, -1, -1, 1}},
        {"pair_production",      {2,  0,  0, 0}},
        {"weak_single_plus",     {0, -2,  0, 0}},
        {"weak_single_minus",    {0,  2,  0, 0}},
        {"weak_dual_plus",       {0, -2, -2, 0}},
        {"weak_dual_minus",      {0,  2,  2, 0}},
    };
}

ConservedChargeBasis solve_conserved_charge_basis(
    const std::vector<ChargeTransition>& transitions) {
    const int rows = static_cast<int>(transitions.size());
    constexpr int cols = NATIVE_CHARGE_FEATURES;
    std::vector<std::array<Rational, cols>> a(static_cast<std::size_t>(rows));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) a[r][c] = Rational(transitions[r].delta[c]);
    }

    std::array<int, cols> pivot_row{};
    pivot_row.fill(-1);
    int rank = 0;
    for (int col = 0; col < cols && rank < rows; ++col) {
        int pivot = rank;
        while (pivot < rows && is_zero(a[pivot][col])) ++pivot;
        if (pivot == rows) continue;
        std::swap(a[pivot], a[rank]);
        const Rational divisor = a[rank][col];
        for (int c = col; c < cols; ++c) a[rank][c] = a[rank][c] / divisor;
        for (int r = 0; r < rows; ++r) {
            if (r == rank || is_zero(a[r][col])) continue;
            const Rational factor = a[r][col];
            for (int c = col; c < cols; ++c) {
                a[r][c] = a[r][c] - factor * a[rank][c];
            }
        }
        pivot_row[col] = rank++;
    }

    ConservedChargeBasis out;
    out.rank = rank;
    out.nullity = cols - rank;
    for (int free_col = 0; free_col < cols; ++free_col) {
        if (pivot_row[free_col] != -1) continue;
        std::array<Rational, cols> v{};
        v[free_col] = Rational(1);
        for (int col = 0; col < cols; ++col) {
            if (pivot_row[col] == -1) continue;
            v[col] = Rational(0) - a[pivot_row[col]][free_col];
        }
        std::int64_t common_den = 1;
        for (const auto& x : v) common_den = lcm_checked(common_den, x.d);
        ChargeVector integer{};
        for (int c = 0; c < cols; ++c) integer[c] = v[c].n * (common_den / v[c].d);
        std::int64_t common_num = 0;
        for (const auto x : integer) common_num = std::gcd(common_num, std::llabs(x));
        if (common_num > 1) for (auto& x : integer) x /= common_num;
        out.integer_basis.push_back(integer);
    }
    return out;
}

}  // namespace eft
}  // namespace ftd
