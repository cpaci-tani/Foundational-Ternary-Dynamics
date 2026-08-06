# FTD-0716 — Period-three co-moving-field solvability v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parent:** FTD-0715

## Question

Does the constructive FTD-0715 three-phase matter trajectory deposit a source
that lies in the range of the exact three-tick translated face/edge field
operator, or does a field nullspace obstruction still prevent a complete
co-moving dressing?

## Frozen source

Use the FTD-0708 `L=33` rest-qualified constituent geometry and the exact 16
FTD-0713 displacement vectors. For each constituent use the FTD-0715 positions

\[
x_{a0}=x_a,
\quad x_{a1}=x_a+\tfrac13\hat x+\delta_a,
\quad x_{a2}=x_a+\tfrac23\hat x-\delta_a,
\quad x_{a3}=x_a+\hat x.
\]

Deposit all three currents with the unchanged exact quadratic-coat
straight-segment observer. Aggregate by oriented face and apply the unchanged
matched field tick in the order

\[
B\leftarrow B-C_{\rm SPEED}C^TE,
\qquad E\leftarrow E+C_{\rm SPEED}CB-j_t.
\]

Require each segment's continuity residual and causal excess `<=1e-12`.

## Frozen field solve

Let `U(k)` be the exact source-free one-tick Fourier symbol used by FTD-0711.
For one-site translation after three ticks solve independently at every
momentum

\[
A_3(k)F(k)=b_3(k),
\qquad A_3(k)=e^{ik_x}U(k)^3-I.
\]

The C++ observer writes the real-space affine source `b_3`. The Python
certificate performs a full complex `6x6` SVD at every lattice momentum using
relative singular-value threshold `1e-12`, source-active threshold `1e-12`,
orthonormal FFT normalization, and the minimum-norm inverse. No regulator or
mode deletion is allowed.

## Gates and verdicts

Validate reality and Parseval residuals to `1e-10`, current continuity and
causality to `1e-12`, and source reconstruction from the three exact segments.
A field solution requires both spectral and real-space maximum residual
`<=1e-9`.

- `PERIOD_THREE_COMOVING_FIELD_SOLUTION_REGULAR` if the solution exists, the
  minimum retained source-active singular value is at least `1e-8`, and total
  solution amplification is at most `1e6`;
- `PERIOD_THREE_COMOVING_FIELD_SOLUTION_ILL_CONDITIONED` if the residual gates
  pass but either conditioning gate fails;
- `PERIOD_THREE_COMOVING_SOURCE_NULLSPACE_INCOMPATIBLE` if the exact discarded
  left-null projection exceeds `1e-9`;
- `PERIOD_THREE_COMOVING_FIELD_SOLVABILITY_EXECUTION_INVALID` for provenance,
  source, FFT, reality, Parseval, or operator failure.

A regular solution establishes finite-volume field/source compatibility only.
It does not establish field recoil balance, action selection, stability, or a
native mobile particle. The next gate is an atomic three-tick matter-field
replay requiring the field to supply the recorded FTD-0715 tick impulses and
return the complete translated state.
