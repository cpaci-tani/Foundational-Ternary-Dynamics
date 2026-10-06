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

## Addendum 2026-10-06: gravity status check (same session)
- g_00: FTD-native via the clock law, conditional on the flagged clock hypothesis (FTD-0131). [DERIVED, conditional]
- g_rr: weak field from the adopted Einstein-Hilbert action (Sep 24 reconstruction: U = V, PPN gamma = 1); exact Schwarzschild under vacuum Einstein closure. EH itself rests on the adopted hypersurface-deformation algebra (Hojman-Kuchar-Teitelboim). [DERIVED, conditional on adopted EH]
- Engine: latency_field solves grad^2 phi = 4 pi G (rho - mean rho) by SOR each tick and sets latency = sqrt(-phi). The field variable is L^2, which is harmonic outside sources (1/R), so the engine is consistent with the reciprocal exterior profile; the retracted FTD-0361 route used L. Gravity in the engine is an elliptic constraint with no tensor sector, so c_T is undefined and an Einstein-aether fit cannot run. [checked in code]
- P2's global clock fixes a foliation, so the full hypersurface algebra can only be emergent; the generic outcome of a preferred-time substrate is Einstein-aether/khronometric, with GR at vanishing aether couplings. [SELECTION PRINCIPLE]
- Spin-2: FTD-0209 [THEOREM] none in the free theory with canonical toggles. FTD-0193 [CLOSED NEGATIVE, pre-registered] already ran the interacting test: connected TT rank-2 correlator of two J-bilinears (flux-quadrupole, stress) on the canonical engine at L = 32, 64. Result: continuum, no pole; the quadrupole channel only carries the spin-1 mode through, and both spin-2 channels sit 7 to 9 orders below the spin-1 control. The composite door is closed in the probed regime; L = 128 and other coupling/temperature regimes remain untested. Effective gravity is at most scalar plus vector.
- Audit of FTD-0193 (2026-10-06, CPU reproduction of tt_correlator_L32.csv to 4.8e-6): the canonical run never manifests matter (amp 0.02 gives max|J| 0.149, genesis needs |J| > K_GENESIS = 1.516; campaign line 202, phase_write.cpp:354), so the measured dynamics are linear and FTD-0193 only repeats the free-theory result FTD-0209. "7 to 9 orders below control" is amp^4 scaling (0.02^4 = 1.6e-7); normalized ratios are 0.1 to 2.7. The TT projector is sound. Frequency bins are 0.0245 (256 lags), so "identical to 7 digits" means same bin; control error is up to 8.4% (L=32) and 13.4% (L=64), not 0.02 to 3%. The report's L=128 "resolved" claim has no tracked file or code. Unregistered CPU rerun at amp 0.4 (matter manifests every tick, window 2048): quadrupole TT peak equals spin-1 control within 4e-4 at 12/12 points, no separable helicity-2 line. Composite door: still untested under a lock; first interacting look shows nothing. [AUDIT FINDING; no tag moved]
