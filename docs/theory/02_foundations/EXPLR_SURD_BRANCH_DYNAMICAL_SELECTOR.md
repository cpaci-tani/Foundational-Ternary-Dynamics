# EXPLR: the surd branch as an order datum, and a conditional dynamical selector

**Tag:** [EXPLORATORY]. Promotes nothing. FC-W stays [AXIOM]; x+ = 1/alpha stays [SMC]. No LEDGER row yet (owner decision).
**Verifier:** `scripts/exploration/surd_branch_checks.py` (sympy + mpmath, 60 digits).

## Results
1. [THEOREM] Over Q(G*) the roots x+- = 8G*^2 +- 4G* delta are Galois conjugates (already FTD-0244/0326). In the ordered reals "the larger root" is definable. The branch is an order datum, not an algebraic one.
2. [THEOREM] With p_N = x+^N + x-^N and r_N = p_{N+1}/p_N (in Q(G*) for every integer N): (x+ - r_N)/(r_N - x-) = (x-/x+)^N. N = 0 gives the midpoint 8G*^2; N >= 1 lies nearer x+; N <= -1 nearer x- (r_{-1} = 2G*). Clock reversal N -> -N acts on this coordinate exactly as the Galois swap.
3. [THEOREM] Forward dominance selects x+ iff the one-tick factor increases with 1/alpha. Multiplier 1/alpha, damping (1 - alpha), decay rate alpha: forward gives x+. Amplitude factor alpha, decay rate 1/alpha: forward gives x-.
4. [THEOREM, conditional] Premise P_Gamma [CONJECTURE]: coarse-grained Phi has a two-mode damped sector with tr Gamma = 1/G*, det Gamma = 1/(16G*^3). With SP-surv [SELECTION PRINCIPLE: observed coupling = damping of the forward survivor] and the arrow (forced given FC-2, FTD-0324), 1/alpha = x+. Reversed arrow gives x-.
5. [THEOREM] P_Gamma cannot hold at the primitive level: x+- are transcendental, finite algebraic operators have algebraic spectra, finite deterministic transfer spectra are {0} and roots of unity (agrees with Lemma 0, FTD-0368).

## Scope notes
- FTD-0784 section 5 two-mode shape: a reversible pair only labels; a damped pair selects.
- Four-walls Lemma G holds for static scalars only; FTD-0326 section 0 covers algebraic invariants only.
- Trap: r_4 lies nearer CODATA than x+. N is a free integer; not evidence.

## Next (pre-register first)
Lock a test: derive from Phi (no alpha input) a coarse-grained two-mode damped sector; check tr and det in epsilon-L form. Falsifier: a derived sector where 1/alpha is a decay rate selects x- and conflicts with FC-2.

## Errata found (verified numerically, not yet fixed)
- FTD-1026 "amplification 44.3 = x+/x-": x+/x- = 45.3167; 44.3 = x+/x- - 1.
- 8G*^2(1 +- sqrt(1 - 1/G*)) gives 127.009 / 13.051; radicand must be 1 - 1/(4G*). Sites: SPEC_QUADRATIC_PHYSICS_BRIDGE.md:425-426, FOUND_THE_COMPLETE_ALGEBRA_OF_i.md:541, DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md:84,91, scripts/proofs/proof_alpha_from_lattice.py:39-40.
- DERIV_ALPHA_FROM_PHASE_STRUCTURE.md:89 says x- < G*; x- - G* = +0.0653.
