# FTD-0711 — Co-moving field Fourier solvability v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parent:** FTD-0710

## Question

Did the FTD-0710 field solve stop because restarted GMRES was too weak for a
near-singular but compatible operator, or because the prescribed translating
source has a nonzero component in the exact co-moving nullspace?

## Frozen inputs

- FTD-0710 protocol:
  `82E52438F5483C5C3A427B31D9B068314778B804C2320EEBFFCA1EA6EE593A4B`;
- summary SHA-256:
  `194AA2AA9AB989CDF2AFED59E71E6565555EB7B639EC8D814160C630E528122A`;
- field RHS/GMRES correction SHA-256:
  `76618236A4F6DB01B27666247245E689D68FBD2CA86A56E051D90DAE38C38A0D`;
- runner SHA-256:
  `88A971564428691FBED81AA5AD0A67CD035CBC827316957891C324BA6E368F8C`.

No source, current, field update, volume, velocity, or trajectory may change.

## Exact Fourier blocks

Use the orthonormal three-dimensional discrete Fourier transform with
`k_i=2*pi*n_i/33` and `d_i=1-exp(-i k_i)`. For the stored ordering `(E,B)`,

\[
C(k)=\begin{pmatrix}
0&-d_z&d_y\\ d_z&0&-d_x\\-d_y&d_x&0
\end{pmatrix},
\qquad C^T(k)=C(k)^\dagger .
\]

One source-free field tick is

\[
U(k)=\begin{pmatrix}
I-\lambda^2CC^\dagger&\lambda C\\
-\lambda C^\dagger&I
\end{pmatrix},\qquad \lambda=1/\sqrt3.
\]

Because translation by `-1` in the FTD-0710 storage convention multiplies a
Fourier coefficient by `exp(+i k_x)`, solve independently at every mode:

\[
A(k)y(k)=b(k),\qquad
A(k)=e^{ik_x}U(k)^2-I_6.
\]

Use an SVD pseudoinverse. A singular value is zero exactly for numerical rank
purposes when

\[
\sigma\le10^{-12}\max(1,\sigma_{\max}).
\]

No Tikhonov term, mode deletion, spectral smoothing, or source adjustment is
permitted.

## Locked validation

1. Apply the Fourier operator to the stored FTD-0710 GMRES correction. Its
   reconstructed Euclidean and infinity residuals must reproduce the parent
   values within `1e-10`.
2. Record the source projection onto every discarded left-singular vector.
3. Reconstruct the minimum-norm field correction and require:
   - Fourier and real-space residual infinity norms `<=1e-9`;
   - imaginary reconstruction residual `<=1e-10`;
   - Parseval norm identity residual `<=1e-10`.
4. Define a source-active mode by
   `||b(k)||_2 > 1e-12 max_q ||b(q)||_2`. Record the smallest retained
   singular value over source-active modes and the global amplification
   `||y||_2/||b||_2`.
5. Classify a compatible solution as ill-conditioned if the smallest retained
   source-active singular value is below `1e-8` or amplification exceeds
   `1e6`; otherwise classify it as regular.

## Verdicts

- `FINITE_VOLUME_COMOVING_FIELD_SOLUTION_REGULAR` if the exact residual gates
  pass and neither ill-conditioning threshold fires;
- `FINITE_VOLUME_COMOVING_FIELD_SOLUTION_ILL_CONDITIONED` if the exact
  residual gates pass and either ill-conditioning threshold fires;
- `FINITE_VOLUME_COMOVING_SOURCE_NULLSPACE_INCOMPATIBLE` if the validated
  minimum-norm solve leaves residual above `1e-9` and the discarded-nullspace
  source projection exceeds `1e-9`;
- `COMOVING_FIELD_FOURIER_SOLVABILITY_EXECUTION_INVALID` if provenance,
  operator cross-check, SVD, reality, or Parseval validation fails.

A compatible result advances the reconstructed field to an unchanged
reciprocal matter replay. An incompatible result closes only the rigid
two-tick prescribed source on `L=33`; the next admissible construction is a
deforming or causally formed matter--field history.
