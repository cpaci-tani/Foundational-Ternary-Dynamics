# FTD-0657 — Classical composite pole boundary

**Status:** `[THEOREM — FROZEN SELECTED CLASSICAL ACTION]`  
**Scope:** finite constituent positions/momenta plus matched face/edge fields  
**Production status:** unchanged

## 1. Statement

For the current classical common-action state, a linear retarded response can
identify:

1. translational zero modes;
2. convective structure-factor lines `omega=k·v` on a moving background;
3. internal constituent, field, and hybrid normal modes.

It cannot identify a massive particle pole

\[
\omega^2=\omega_0^2+c^2k^2,\qquad \omega_0=E_{rest},
\]

solely because the Hamiltonian contains the additive rest energy `E_rest`.
The rest term affects the momentum Hessian and hence inertia, but it supplies
no state coordinate whose phase rotates at `E_rest`.

## 2. Proof

### 2.1 Constants do not enter the tangent map

Write the selected classical energy near a dressed state as

\[
H(q,p,F)=H_0+T(p)+V(q,F).
\]

Hamilton's equations, the discrete-gradient equations, and their tangent map
depend on first and second differences of `H`. The additive constant `H_0`
has zero first and second differences. Therefore no eigenvalue of the linear
response can be fixed to `H_0` merely by its presence in the energy ledger.

### 2.2 The collective translation is a zero mode

For one free collective coordinate, linearization about `p=0` gives

\[
\dot q=M^{-1}p,\qquad \dot p=0,
\]

where `M^{-1}=partial_p^2 T(0)=C_SPEED^2/E_rest` for the production
dispersion. A discrete step has the tangent block

\[
D\Phi=\begin{pmatrix}1&\Delta t/M\\0&1\end{pmatrix}.
\]

Its only eigenvalue is one. The discrete retarded resolvent has denominator

\[
\det(I-zD\Phi)=(1-z)^2,
\]

so the response is at zero frequency. `E_rest` changes the off-diagonal
coefficient, hence the response amplitude/inertia, but not the pole location.

### 2.3 Binding supplies internal frequencies, not the rest pole

For quadratic internal coordinates with mass matrix `M` and stiffness `K`,

\[
{d\over dt}\binom{\delta q}{\delta p}
=\begin{pmatrix}0&M^{-1}\\-K&0\end{pmatrix}
\binom{\delta q}{\delta p}.
\]

Nonzero frequencies satisfy

\[
\det(\omega^2I-M^{-1}K)=0.
\]

These are binding/field/hybrid mode frequencies. The constant rest-energy
offset is absent. This is the correct interpretation of the FTD-0640--0642
classical mode spectra.

### 2.4 A moving form factor is convective

For a coherently translated density `rho(x-R(t))` with `R(t)=R_0+vt`,

\[
F(k,t)=F(k,0)e^{-ik\cdot vt}.
\]

Its line is `omega=k·v`. FTD-0656's co-moving matter/field structure factors
measure this convective line and dressing coherence. They do not measure a
mass shell with nonzero rest intercept.

### 2.5 What would be sufficient

A classical massive field coordinate `phi` with quadratic action

\[
{1\over2}\dot\phi^2-{c^2\over2}|\nabla\phi|^2
-{\omega_0^2\over2}\phi^2
\]

does have `omega^2=omega_0^2+c^2k^2`. Alternatively, quantization or an
independently derived statistical phase of the soliton collective coordinate
can produce a quantum two-point particle pole. Neither structure is currently
part of the selected constituent common-action state.

This is not an argument that a new primitive is mandatory. A phase/amplitude
coordinate could still emerge from a native limit cycle, topological sector,
clock variable, or ensemble construction. It must be derived and measured;
the rest-energy ledger alone cannot substitute for it.

## 3. Consequence

The next retarded campaign must be named and graded as a **classical hybrid-mode
response**, not a particle-pole campaign. A physical particle-pole claim is
blocked until one of these gates closes:

1. a native oscillatory phase/amplitude mode with a nonzero rest intercept;
2. a justified quantization/statistical correlator of the collective mode;
3. an equivalent connection/holonomy construction whose observable has the
   required pole and positive residue.

FTD-0656 remains constructive at co-moving-dressing scope. No prior numerical
result is regraded.
