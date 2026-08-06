# Audit — connected-block analytic static and dynamical rest

**Campaigns:** FTD-0637 through FTD-0639  
**Verdict:** `[ANALYTIC STATIC BASIN + REVERSIBLE DYNAMICAL REST
CONSTRUCTIVE; PHYSICAL PARTICLE CLAIM OPEN]`

## Findings

1. The FTD-0637 field gradient and Hessian follow from the envelope theorem;
   they do not differentiate through an iterative solver or reuse the failed
   finite-difference estimate.
2. Exact coat derivative sum rules, 49 Poisson response solves per arm, energy
   identity, Hessian symmetry, translation contraction, and cyclic covariance
   pass. The independent certificate recomputes both spectra.
3. FTD-0637 correctly retains a negative verdict on stationarity: the
   `1.1204e-8` analytic force exceeds the locked `1e-8` threshold despite the
   positive Hessian.
4. FTD-0638 changes no action term or tolerance. A deterministic full-space
   Newton step of at most `4.704e-10` per coordinate reduces the force below
   `1.35e-14` and preserves the original spline sector.
5. The energy change is evaluated as a stable Poisson-envelope difference,
   rather than subtracting two nearly equal `0.0354` doubles. The decrement is
   negative in both cyclic arms and agrees with the analytic descent direction.
6. FTD-0639 uses the existing common-action forward and state-only reverse
   solvers. All 512 steps pass; the state is fixed to machine resolution and
   recovers without a stored history.
7. Production remains unchanged. The cap-eight fibre is a selected chart
   capacity, the binding graph is selected dynamics, and the constituent
   phase space is a selected ontic extension. None is reclassified as forced
   by the five postulates.
8. The result licenses `local classical dressed rest state`. It does not
   license `elementary particle`, `quantum eigenstate`, `physical charge`,
   `mass spectrum`, `matter pole`, `Lorentz recovery`, or `unitarity`.

Independent certificates:

- `proof_connected_block_analytic_envelope_hessian.py`;
- `proof_connected_block_analytic_static_refinement.py`;
- `proof_connected_block_analytic_dynamical_rest.py`.

FTD-0640 subsequently closes analytic perturbation dynamics constructively.
The next live gate is an independent field spectrum, followed by coupled-mode
classification and a finite boost ladder.
