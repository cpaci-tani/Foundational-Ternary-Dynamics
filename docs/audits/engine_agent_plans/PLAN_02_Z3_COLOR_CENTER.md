# Plan 02 — Z3 Color-Center Closure Module

## Objective

Implement the finite \(\mathbb Z_3\) center-closure scaffold in C++.

This module demonstrates center-neutral observability:

\[
\sum_i c_i\equiv 0 \pmod 3
\]

with:

\[
q\bar q: 1+2\equiv 0
\]

\[
qqq: 1+1+1\equiv 0
\]

and non-neutral examples:

\[
q: 1\not\equiv0
\]

\[
qq: 1+1\equiv2\not\equiv0.
\]

## Status labels

- Center-neutral arithmetic: THEOREM
- Observable center-neutral selection scaffold: CONDITIONAL THEOREM inside finite-center model
- Flux penalty: CANDIDATE PRINCIPLE
- Full QCD confinement: OPEN

## Add file

`engine/include/ftd/color_center.h`

## Required API

```cpp
#pragma once
#include <array>
#include <cstdint>
#include <initializer_list>
#include <vector>
#include <complex>
#include <cmath>

namespace ftd {

enum class Z3Charge : uint8_t {
    Neutral = 0,
    Q = 1,
    QBar = 2
};

inline int z3_value(Z3Charge c) {
    return static_cast<int>(c);
}

inline int z3_mod(int x) {
    int r = x % 3;
    return r < 0 ? r + 3 : r;
}

inline int center_total(std::initializer_list<Z3Charge> charges) {
    int s = 0;
    for (auto c : charges) s += z3_value(c);
    return z3_mod(s);
}

inline int center_total(const std::vector<Z3Charge>& charges) {
    int s = 0;
    for (auto c : charges) s += z3_value(c);
    return z3_mod(s);
}

inline bool is_center_neutral(std::initializer_list<Z3Charge> charges) {
    return center_total(charges) == 0;
}

inline bool is_center_neutral(const std::vector<Z3Charge>& charges) {
    return center_total(charges) == 0;
}

struct Z3ProjectorCheck {
    double idempotent_error;
    int rank_expected;
};

// Optional simple check without external linear algebra.
// P0 = (I + Z + Z^2)/3 in regular representation.
inline Z3ProjectorCheck check_center_projector() {
    using C = std::complex<double>;
    std::array<std::array<C, 3>, 3> I{};
    std::array<std::array<C, 3>, 3> Z{};
    std::array<std::array<C, 3>, 3> Z2{};
    std::array<std::array<C, 3>, 3> P{};
    std::array<std::array<C, 3>, 3> P2{};

    for (int i = 0; i < 3; ++i) I[i][i] = C{1.0, 0.0};
    Z[0][2] = C{1.0, 0.0};
    Z[1][0] = C{1.0, 0.0};
    Z[2][1] = C{1.0, 0.0};

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
                Z2[i][j] += Z[i][k] * Z[k][j];

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            P[i][j] = (I[i][j] + Z[i][j] + Z2[i][j]) / C{3.0, 0.0};

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
                P2[i][j] += P[i][k] * P[k][j];

    double err2 = 0.0;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            err2 += std::norm(P2[i][j] - P[i][j]);

    return {std::sqrt(err2), 1};
}

inline double toy_flux_energy(double length, bool center_neutral, double sigma_c = 1.0, double open_penalty = 0.0) {
    return sigma_c * length + (center_neutral ? 0.0 : open_penalty);
}

} // namespace ftd
```

## Add test

`engine/tests/test_z3_color_center.cpp`

## Test content

```cpp
#include <cmath>
#include <iostream>
#include <stdexcept>
#include "ftd/color_center.h"

static void require_true(bool b, const char* msg) {
    if (!b) throw std::runtime_error(msg);
}

static void require_close(double a, double b, double tol, const char* msg) {
    if (std::abs(a - b) > tol) throw std::runtime_error(msg);
}

int main() {
    using namespace ftd;

    require_true(is_center_neutral({Z3Charge::Q, Z3Charge::QBar}), "q qbar must be neutral");
    require_true(is_center_neutral({Z3Charge::Q, Z3Charge::Q, Z3Charge::Q}), "qqq must be neutral");
    require_true(!is_center_neutral({Z3Charge::Q}), "single q must be non-neutral");
    require_true(!is_center_neutral({Z3Charge::Q, Z3Charge::Q}), "qq must be non-neutral");

    require_true(center_total({Z3Charge::Q, Z3Charge::QBar}) == 0, "q qbar total");
    require_true(center_total({Z3Charge::Q, Z3Charge::Q, Z3Charge::Q}) == 0, "qqq total");
    require_true(center_total({Z3Charge::Q, Z3Charge::Q}) == 2, "qq total");

    const auto P = check_center_projector();
    require_close(P.idempotent_error, 0.0, 1e-14, "P0 idempotent");

    const double neutral_E = toy_flux_energy(5.0, true, 2.0, 100.0);
    const double open_E = toy_flux_energy(5.0, false, 2.0, 100.0);
    require_close(neutral_E, 10.0, 1e-12, "neutral flux energy");
    require_close(open_E, 110.0, 1e-12, "open flux energy");

    return 0;
}
```

## CMake edit

```cmake
ftd_add_test(test_z3_color_center
             tests/test_z3_color_center.cpp
             CTEST_NAME z3_color_center
             NO_CORE
             LABELS unit physics graph color theorem)
```

## Acceptance criteria

- The test passes.
- The module does not claim full confinement.
- Ledger distinguishes center-closure theorem from confinement dynamics.

## Documentation target

Create:

`docs/theory/03_derivations/DERIV_Z3_CENTER_GRAPH_CLOSURE.md`

Minimum sections:

1. Finite center model
2. Neutrality theorem
3. Projector \(P_0^{(c)}\)
4. Meson/baryon closure
5. Open-flux scaffold
6. Non-claims: no \(\Lambda_{\rm QCD}\), no full confinement derivation
