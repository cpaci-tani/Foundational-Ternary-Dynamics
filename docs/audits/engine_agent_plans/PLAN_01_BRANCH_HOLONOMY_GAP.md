# Plan 01 — Branch Holonomy Gap Module

## Objective

Implement the theorem-level branch-holonomy gap result as a repo-native C++ module and test.

Target result:

\[
H_x=-1
\Rightarrow
\lambda_{\min}=4\sin^2\left(\frac{\pi}{2N}\right)
\]

for a periodic cubic \(N^3\) lattice with one anti-periodic branch twist.

For \(k\) independent twisted cycles:

\[
\lambda_{\min}
=
k\,4\sin^2\left(\frac{\pi}{2N}\right).
\]

## Status label

THEOREM for the finite periodic cubic graph.

Not a production physics change.

## Add file

`engine/include/ftd/branch_holonomy.h`

## Required API

```cpp
#pragma once
#include <array>
#include <cmath>
#include <cstdint>

namespace ftd {

enum class BranchTwist : uint8_t {
    Periodic = 0,
    AntiPeriodic = 1
};

struct BranchTwist3 {
    BranchTwist x = BranchTwist::Periodic;
    BranchTwist y = BranchTwist::Periodic;
    BranchTwist z = BranchTwist::Periodic;
};

inline int holonomy(BranchTwist t) {
    return t == BranchTwist::AntiPeriodic ? -1 : +1;
}

inline std::array<int, 3> holonomy_vector(const BranchTwist3& t) {
    return {holonomy(t.x), holonomy(t.y), holonomy(t.z)};
}

inline double branch_momentum_shift(BranchTwist t) {
    return t == BranchTwist::AntiPeriodic ? M_PI : 0.0;
}

inline double torus_laplacian_eigenvalue_1d(int N, int m, BranchTwist t, double a = 1.0) {
    const double theta = branch_momentum_shift(t);
    const double p = (2.0 * M_PI * static_cast<double>(m) + theta) / static_cast<double>(N);
    return 4.0 * std::pow(std::sin(0.5 * p), 2.0) / (a * a);
}

inline double torus_laplacian_eigenvalue_3d(
    int N,
    int mx,
    int my,
    int mz,
    const BranchTwist3& twists,
    double a = 1.0
) {
    return torus_laplacian_eigenvalue_1d(N, mx, twists.x, a)
         + torus_laplacian_eigenvalue_1d(N, my, twists.y, a)
         + torus_laplacian_eigenvalue_1d(N, mz, twists.z, a);
}

inline int twist_count(const BranchTwist3& twists) {
    return (twists.x == BranchTwist::AntiPeriodic ? 1 : 0)
         + (twists.y == BranchTwist::AntiPeriodic ? 1 : 0)
         + (twists.z == BranchTwist::AntiPeriodic ? 1 : 0);
}

inline double exact_torus_branch_gap(int N, const BranchTwist3& twists, double a = 1.0) {
    return static_cast<double>(twist_count(twists))
         * 4.0 * std::pow(std::sin(M_PI / (2.0 * static_cast<double>(N))), 2.0) / (a * a);
}

} // namespace ftd
```

If the project avoids `M_PI`, define a local constexpr:

```cpp
inline constexpr double PI_BRANCH = 3.141592653589793238462643383279502884;
```

and use that instead.

## Add test

`engine/tests/test_branch_holonomy_gap.cpp`

## Test content

Test exact gaps for several \(N\):

```cpp
#include <cmath>
#include <iostream>
#include <stdexcept>
#include "ftd/branch_holonomy.h"

static void require_close(double actual, double expected, double tol, const char* msg) {
    if (std::abs(actual - expected) > tol) {
        std::cerr << msg << ": actual=" << actual << " expected=" << expected << "\n";
        throw std::runtime_error(msg);
    }
}

int main() {
    using namespace ftd;

    for (int N : {8, 10, 12, 16, 18, 20, 32, 64}) {
        BranchTwist3 none{};
        BranchTwist3 x{BranchTwist::AntiPeriodic, BranchTwist::Periodic, BranchTwist::Periodic};
        BranchTwist3 xy{BranchTwist::AntiPeriodic, BranchTwist::AntiPeriodic, BranchTwist::Periodic};
        BranchTwist3 xyz{BranchTwist::AntiPeriodic, BranchTwist::AntiPeriodic, BranchTwist::AntiPeriodic};

        require_close(exact_torus_branch_gap(N, none), 0.0, 1e-15, "untwisted gap");
        require_close(exact_torus_branch_gap(N, xy), 2.0 * exact_torus_branch_gap(N, x), 1e-14, "two twist additivity");
        require_close(exact_torus_branch_gap(N, xyz), 3.0 * exact_torus_branch_gap(N, x), 1e-14, "three twist additivity");

        const double mode = torus_laplacian_eigenvalue_3d(N, 0, 0, 0, x);
        require_close(mode, exact_torus_branch_gap(N, x), 1e-14, "lowest anti-periodic mode");

        const double asym = M_PI * M_PI / (static_cast<double>(N) * static_cast<double>(N));
        const double ratio = exact_torus_branch_gap(N, x) / asym;
        if (N >= 32 && std::abs(ratio - 1.0) > 0.01) {
            throw std::runtime_error("large-N asymptotic ratio too far from 1");
        }
    }

    return 0;
}
```

## CMake edit

Prefer `ftd_add_test`:

```cmake
ftd_add_test(test_branch_holonomy_gap
             tests/test_branch_holonomy_gap.cpp
             CTEST_NAME branch_holonomy_gap
             NO_CORE
             LABELS unit physics graph theorem)
```

Use `NO_CORE` because the module is header-only and should not link production engine code.

If labels are not accepted by your local macro in that exact form, use the nearest existing pattern.

## Acceptance criteria

- Test passes.
- No `RenderBridge` changes.
- Golden-tick test remains unchanged.
- Documentation says this is a finite graph theorem, not a new physical mass prediction by itself.

## Failure modes

- If compile fails on `M_PI`, replace with a local constexpr.
- If `NO_CORE` fails, link `ftd_core` only as fallback.
- If labels syntax fails, strip labels but keep the CTest target.

## Documentation target

Create:

`docs/theory/03_derivations/DERIV_BRANCH_HOLONOMY_GAP.md`

Minimum sections:

1. Statement
2. Proof
3. Relation to signed incidence
4. FTD interpretation
5. Status: THEOREM
6. Non-claims
