# Plan 03 — Generation Graph Candidate Module

## Objective

Implement the candidate generation graph rule as a C++ diagnostic module.

Candidate law:

\[
\Gamma_F(d)=K_3(q^{d+1},1,q^d;\phi=\pi+\pi/d)
\]

with:

\[
q^*=\frac{G^*-\sqrt{G^{*2}-4}}{2}.
\]

Candidate sector pairing:

\[
\Gamma_U=\Gamma_F(3),\qquad \Gamma_D=\Gamma_F(2).
\]

Expected absolute eigenbasis overlap:

\[
\begin{pmatrix}
0.973536 & 0.228440 & 0.006537\\
0.228336 & 0.972678 & 0.041952\\
0.009485 & 0.041385 & 0.999098
\end{pmatrix}
\]

## Status label

CANDIDATE RECONSTRUCTION.

Do not label as theorem until \(d_U=3\), \(d_D=2\), and \(\phi(d)=\pi+\pi/d\) are derived from finite internal closure.

## Add file

`engine/include/ftd/generation_graph.h`

## Implementation constraints

This is a 3x3 Hermitian eigenproblem. Avoid adding heavyweight dependencies. Options:

1. Preferred: implement a small deterministic Jacobi diagonalizer for Hermitian 3x3.
2. Alternative: for first pass, store the expected overlap produced by the Python audit and test rule inputs/outputs separately.
3. Best long-term: add a small matrix utility module if one already exists in the repo.

The agent should first search the repo for existing small matrix/eigenvalue utilities before writing a new one.

## Required API

At minimum:

```cpp
#pragma once
#include <array>
#include <complex>
#include <cmath>
#include "ftd/ontic.h" // or the correct existing constant header

namespace ftd {

struct GenerationRule {
    int d;
    std::array<int, 3> powers; // p12, p23, p13
    double phi;
};

inline double q_star_from_gstar(double G) {
    return (G - std::sqrt(G * G - 4.0)) / 2.0;
}

inline GenerationRule generation_rule(int d) {
    constexpr double PI = 3.141592653589793238462643383279502884;
    return GenerationRule{
        d,
        {d + 1, 0, d},
        std::fmod(PI + PI / static_cast<double>(d), 2.0 * PI)
    };
}

struct GenerationWeights {
    double w12;
    double w23;
    double w13;
    double phi;
};

inline GenerationWeights generation_weights(int d, double q) {
    auto r = generation_rule(d);
    return {
        std::pow(q, r.powers[0]),
        std::pow(q, r.powers[1]),
        std::pow(q, r.powers[2]),
        r.phi
    };
}

} // namespace ftd
```

For the first C++ test, it is acceptable to test:

- `generation_rule(2)` returns powers `(3,0,2)` and \(\phi=3\pi/2\).
- `generation_rule(3)` returns powers `(4,0,3)` and \(\phi=4\pi/3\).
- \(q^*+1/q^*=G^*\).
- Edge weight hierarchy matches \(q^{d+1},1,q^d\).
- If eigen solver is added, test overlap matrix.

## Full eigen-solver option

If implementing the Hermitian matrix:

\[
L =
\begin{pmatrix}
w_{12}+w_{13} & -w_{12} & -w_{13}e^{i\phi}\\
-w_{12} & w_{12}+w_{23} & -w_{23}\\
-w_{13}e^{-i\phi} & -w_{23} & w_{13}+w_{23}
\end{pmatrix}
\]

Need eigenvectors \(U_d\) and overlap:

\[
|U_3^\dagger U_2|.
\]

The test tolerance can be \(5\times10^{-4}\) for first implementation.

## Add test

`engine/tests/test_generation_graph.cpp`

## Minimal test content

```cpp
#include <cmath>
#include <stdexcept>
#include "ftd/generation_graph.h"

static void require_close(double a, double b, double tol, const char* msg) {
    if (std::abs(a - b) > tol) throw std::runtime_error(msg);
}

int main() {
    constexpr double PI = 3.141592653589793238462643383279502884;
    constexpr double G = 2.9586751191886389;

    const double q = ftd::q_star_from_gstar(G);
    require_close(q + 1.0/q, G, 1e-13, "q* root relation");

    const auto d2 = ftd::generation_rule(2);
    const auto d3 = ftd::generation_rule(3);

    if (d2.powers != std::array<int,3>{3,0,2}) throw std::runtime_error("d=2 powers");
    if (d3.powers != std::array<int,3>{4,0,3}) throw std::runtime_error("d=3 powers");

    require_close(d2.phi, 1.5 * PI, 1e-14, "d=2 phi");
    require_close(d3.phi, 4.0 * PI / 3.0, 1e-14, "d=3 phi");

    const auto w2 = ftd::generation_weights(2, q);
    const auto w3 = ftd::generation_weights(3, q);

    require_close(w2.w23, 1.0, 1e-15, "d=2 unsuppressed edge");
    require_close(w3.w23, 1.0, 1e-15, "d=3 unsuppressed edge");
    require_close(w2.w12, std::pow(q, 3), 1e-15, "d=2 w12");
    require_close(w2.w13, std::pow(q, 2), 1e-15, "d=2 w13");
    require_close(w3.w12, std::pow(q, 4), 1e-15, "d=3 w12");
    require_close(w3.w13, std::pow(q, 3), 1e-15, "d=3 w13");

    return 0;
}
```

## CMake edit

```cmake
ftd_add_test(test_generation_graph
             tests/test_generation_graph.cpp
             CTEST_NAME generation_graph
             NO_CORE
             LABELS unit physics graph flavor reconstruction)
```

## Acceptance criteria

- Minimal rule test passes.
- If eigen solver is implemented, overlap matrix matches the Python audit within tolerance.
- Documentation labels the result as CANDIDATE RECONSTRUCTION.
- No CKM-solved claim.

## Documentation target

Create:

`docs/theory/05_particles/EXPLR_GENERATION_GRAPH_GAMMA_D.md`

Minimum sections:

1. Definition of \(\Gamma_F(d)\)
2. Origin from graph search compression
3. \(d=2,d=3\) candidate interpretation
4. CKM-like overlap
5. Mass-proxy audit summary
6. Required hardening: derive \(d_U,d_D,\phi(d)\)
7. Status and non-claims
