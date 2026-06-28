# DERIV_STOCHASTIC_OPERATOR_ESTIMATOR.md

**Title:** Stochastic Transfer / Koopman Operator Estimator for the FTD Alpha Readout  
**Status:** `[OPEN PROGRAM]` / estimator specification  
**Depends on:** `SPEC_ALPHA_READOUT_PROGRAM.md`, `SPEC_ALPHA_READOUT_CONTRACT.md`, `SPEC_ALPHA_READOUT_OSCILLATORY_CLOUD.md`  
**Purpose:** Define the non-circular statistical-mechanics estimator for the alpha readout after the deterministic Floquet and static loop routes have closed negative.

---

## 0. Executive statement

The deterministic Floquet route failed because the bare hard-threshold phase-law did not yield a robust deterministic breather, and the static loop route failed because integer winding is spectrally gauge-trivial. The alpha readout therefore moves to the stochastic stationary cloud ensemble.

The object to estimate is no longer a tangent-map multiplier. It is the reduced Koopman/transfer resolvent of the Langevin-stabilized cloud:

\[
W_U=\Pi_{\mathbb Z[i]}\,(I-K_{\rm red})^{-1}\,\Pi_{\mathbb Z[i]}.
\]

The fine-structure readout is admissible only if the dominant nontrivial eigenvalue \(\mu_+\) of \(K_{\rm red}\) is fixed by a canonical bath and a fixed observable projection:

\[
\alpha_{\rm FTD}=|1-\mu_+|,
\qquad
\alpha_{\rm FTD}^{-1}=\frac{1}{|1-\mu_+|}.
\]

The target value is

\[
\mu_+ \approx 0.9927026566,
\qquad
|1-\mu_+|\approx0.0072973434.
\]

> [!WARNING]
> **Readout admissibility precondition.** This estimator runs only on
> a trajectory that has first passed the read-only, **$\alpha$-blind** scale-context
> gate $\mathcal{C}_{\rm scale}$ (`SPEC_SCALE_CONTEXT_READOUT.md`, implemented in
> `engine/src/scale_context.cpp`). Note this doc legitimately names the target
> $\alpha$ value above — the **gate** must not, which is exactly why the gate is a
> *separate* module that accepts/rejects a cloud on geometry + self-confinement
> *before* any spectral computation. The estimator math here is unchanged; the gate
> is a precondition, not part of the readout. First measurement of record: the
> canonical $A=14$ / $L=32$ cloud **percolates the box**
> ($R_{\rm eff}\approx L/2$, $\zeta\approx0.50$ ⇒ `REJECTED_SCALE_CONTEXT`), so the
> current $L=32$ trajectory is inadmissible (see `SPEC_ALPHA_READOUT_PROGRAM §3.1`).
> The downstream guard `scripts/proofs/proof_alpha_stochastic_koopman.py` refuses a
> rejected trajectory unless `--allow-rejected` is passed.

---

## 1. Operator orientation

There are two equivalent but direction-sensitive operator descriptions.

### 1.1 Perron-Frobenius transfer operator

The Perron-Frobenius operator \(P\) acts on densities:

\[
\rho_{t+1}=P\rho_t.
\]

This is useful for proving the existence and uniqueness of the invariant measure \(\pi\), but it is not the most convenient object for projected readout channels.

### 1.2 Koopman operator

The Koopman operator \(K\) acts on observables:

\[
Kf(\Omega)=\mathbb E[f(\Omega_{t+1})\mid \Omega_t=\Omega].
\]

Because the alpha readout is built from public observable channels, this specification uses the Koopman orientation by default. If a document uses \(T\), it must state whether \(T=P\) on densities or \(T=K\) on observables.

---

## 2. Canonical stochastic process

Let \(\Omega_t\) denote the full FTD state under the Langevin-stabilized phase law:

\[
\Omega_{t+1}=F_{\rm Lang}(\Omega_t,\xi_t),
\]

where \(\xi_t\) is the stochastic bath increment. The bath parameters must be fixed independently of alpha.

Admissibility conditions:

1. **Canonical bath:** friction, temperature, and noise amplitude are fixed by non-alpha engine physics.
2. **Unique invariant measure:** the \(A\approx14\) cloud has a reproducible stationary distribution \(\pi\).
3. **Fixed projection:** \(\Pi_{\mathbb Z[i]}\) and the observable dictionary are fixed before checking \(137\).
4. **Stable spectral gap:** the leading nontrivial \(\mu_+\) is stable under seed, trajectory length, lattice size, and dictionary refinement.
5. **No target fitting:** no parameter sweep over bath variables may be used to hit \(\alpha^{-1}\).

---

## 3. Observable dictionary

Let

\[
\psi(\Omega)=(\psi_1(\Omega),\ldots,\psi_d(\Omega))^\top
\]

be a fixed vector of canonical cloud observables. The minimal dictionary should contain only non-alpha observables, for example:

- \(\mathbb Z[i]\)-quadrature amplitudes \(q,p\),
- cloud intensity moments \(\rho=|z|^2\),
- shape tensor components,
- breathing/shear/rotation mode amplitudes,
- canonical neutral-source parity channels,
- threshold-crossing counters if they are fixed before spectral extraction.

The dictionary must exclude empirical \(\alpha\), \(x_+\), \(1/137\), QED formulae, and fitted basis functions selected after observing the target.

---

## 4. EDMD estimator

Given a stationary trajectory

\[
\Omega_0,\Omega_1,\ldots,\Omega_N,
\]

compute

\[
X_t=\psi(\Omega_t),\qquad
Y_t=\psi(\Omega_{t+1}).
\]

Define the time-lagged covariance matrices

\[
C_{00}=\frac1N\sum_{t=0}^{N-1} X_tX_t^\top,
\]

\[
C_{01}=\frac1N\sum_{t=0}^{N-1} X_tY_t^\top.
\]

The finite-dimensional Koopman estimate is

\[
K_{\rm EDMD}=C_{00}^{+}C_{01},
\]

where \(C_{00}^{+}\) is a regularized pseudoinverse. With ridge regularization,

\[
K_{\rm EDMD}=(C_{00}+\lambda I)^{-1}C_{01}.
\]

---

## 5. Reduction and projection

Remove trivial modes:

- stationary constant observable,
- global normalization,
- translation modes,
- pure time-shift / phase drift modes,
- gauge-like redundant directions.

Let \(K_{\rm red}\) denote the reduced operator. Then define the resolvent readout

\[
W_U=\Pi_{\mathbb Z[i]}\,(I-K_{\rm red})^{-1}\,\Pi_{\mathbb Z[i]}.
\]

The eigenvalue test can be performed either directly on \(K_{\rm red}\) or on \(W_U\). If

\[
K_{\rm red}v_+=\mu_+v_+,
\]

then the corresponding resolvent eigenvalue is

\[
\lambda_+=\frac{1}{1-\mu_+}
\]

for real \(\mu_+\), and more generally

\[
|\lambda_+|=\frac{1}{|1-\mu_+|}.
\]

---

## 6. Alpha estimator

The alpha estimator is

\[
\widehat{\alpha}^{-1}=\frac{1}{|1-\widehat{\mu}_+|},
\]

where \(\widehat{\mu}_+\) is the leading nontrivial eigenvalue of \(K_{\rm red}\) after removing the invariant eigenvalue \(\mu=1\).

If the reduced two-channel resolvent matrix \(W_U\) is explicitly built, then the stronger master-quadratic test is:

\[
\operatorname{Tr}W_U \stackrel{?}{\to} 16G^{*2},
\]

\[
\det W_U \stackrel{?}{\to} 16G^{*3}.
\]

---

## 7. Bootstrap stability protocol

For a valid run, report:

1. estimated \(\mu_+\),
2. \(\widehat{\alpha}^{-1}\),
3. confidence interval from block bootstrap,
4. seed stability,
5. trajectory-length stability,
6. lattice-size stability,
7. dictionary-refinement stability,
8. bath-parameter immutability declaration.

A result close to \(137\) is not admissible unless all stability checks pass.

---

## 8. Pass/fail logic

### Positive candidate

The stochastic readout remains viable if

\[
\widehat{\alpha}^{-1}\to137.036171458\ldots
\]

under fixed canonical bath, fixed projection, growing data, and dictionary/lattice refinement.

### Closed negative

The stochastic route closes negative if:

- no unique invariant cloud measure exists,
- \(\mu_+\) is bath-tuned,
- \(\mu_+\) is not stable under seed/data/lattice refinement,
- the leading slow mode is a trivial drift or normalization mode,
- \(W_U\) fails the master-quadratic trace/determinant test.

---

## 9. Minimal execution command

Use `proof_alpha_stochastic_koopman.py` on an `.npz` trajectory exported from the FTD engine.

Required `.npz` keys are one of:

1. `features`: array of shape `(N, d)` containing precomputed canonical observables, or
2. raw arrays such as `J`, `p`, `s`, from which the script can build a default diagnostic dictionary.

Example:

```bash
python proof_alpha_stochastic_koopman.py --input trajectory_A14.npz --lag 1 --ridge 1e-8 --bootstrap 200 --block-size 512
```

The script reports \(\mu_+\), \(\alpha^{-1}\), bootstrap intervals, and trace/determinant diagnostics when a two-channel projection is supplied.