# Plan 06 — Nonlinear Scalar Source-Law Solver

## Objective

Implement the nonlinear graph source law as an isolated module:

\[
D_\sigma^\dagger W\sinh(D_\sigma U)=\kappa_\psi J_\chi.
\]

Weak limit:

\[
L_\sigma U=\kappa_\psi J_\chi.
\]

This should begin as a standalone theory/test module, not as a production gravity replacement.

## Status labels

- Equation definition: DEFINITION / CANDIDATE PRINCIPLE depending on doc context
- Weak graph solve: THEOREM once implemented as finite equation
- Physical \(G_N,\ell_F,\kappa_\psi\) interpretation: OPEN / SELECTION

## Add file

`engine/include/ftd/source_law_graph.h`

## Required API

Initial API should be scalar and small:

```cpp
struct SourceLawConfig {
    double kappa = 1.0;
    double tolerance = 1e-10;
    int max_iterations = 100;
};

struct SourceLawResult {
    std::vector<double> U;
    int iterations;
    double residual_norm;
    bool converged;
};
```

But avoid adding general sparse matrix infrastructure unless the repo already has it.

## Initial test strategy

Use small graphs where exact behavior is obvious:

1. Two-node graph with source-sink.
2. Three-node path with net-zero source.
3. Periodic ring with net-zero source.

For weak mode:

\[
L U = J
\]

can be solved by direct small dense Gaussian elimination.

For nonlinear mode:

Newton iteration:

\[
F(U)=D^\top W\sinh(DU)-\kappa J
\]

Jacobian:

\[
J_F(U)=D^\top W\operatorname{diag}(\cosh(DU))D.
\]

## Acceptance tests

- At small source amplitude, nonlinear result matches weak result to \(O(J^3)\).
- Residual norm decreases under Newton.
- Net source must be zero for periodic graph or gauge fixed by mean-zero constraint.
- No modification to `RenderBridge`.

## Recommended implementation sequence

1. Implement a dense small-graph test helper in the test file.
2. Prove the formulas on tiny graphs.
3. Only then decide whether to integrate with `Lattice`.
4. Do not assign physical \(G_N\) yet.

## Documentation target

Create:

`docs/theory/03_derivations/EXPLR_NONLINEAR_SOURCE_LAW_GRAPH_SOLVER.md`

Required non-claim:

This implements a finite scalar source-law model. It is not full GR and does not derive \(G_N\) unless \(\kappa_\psi,\ell_F\) are independently fixed.
