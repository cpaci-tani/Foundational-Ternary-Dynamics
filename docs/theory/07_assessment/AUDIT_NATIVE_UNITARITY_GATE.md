# FTD-0425 — Native injectivity and unitarity gate

**Date:** 2026-07-22  
**Status:** `[THEOREM — source-free linear transfer sector]` + `[ENGINE COUNTEREXAMPLE — full tick injectivity]` + `[OPEN — manifested spectral positivity]`  
**Verdict:** `LINEAR-SECTOR-REVERSIBLE; FULL-TICK-NON-INJECTIVE; EMERGENT-LOW-ENERGY-UNITARITY-UNRESOLVED`

## 1. Linear sector

For one production 18-point eigenmode, let `x=c^2 M(q)`. The source-free
kick-drift update on `(J,W)` has transfer matrix

$$
T(x)=\begin{pmatrix}1-x&1\\-x&1\end{pmatrix},
\qquad \det T=1.
$$

The production band has `0<=M<=16/3` and `c^2=1/3`, hence
`0<=x<=16/9<4`. Away from the constant zero mode, both transfer roots have
unit modulus. The exact invariant metric

$$
H(x)=\begin{pmatrix}x&-x/2\\-x/2&1\end{pmatrix}
$$

is positive because `det H=x(1-x/4)>0`. The free linear wave sector is
therefore reversible and has positive oscillator residue throughout its band.

## 2. Full tick counterexamples

The full production tick is not injective. Engine witnesses show:

1. distinct `s=+1` and `s=-1` zero-field manifested states undergo the same
   accepted evaporation event and reach the same void state;
2. annihilation erases distinct spin/color assignments and produces identical
   post-collision states.

Independent structural routes include Gauss projection, speed projection,
genesis/pair creation without a retained event register, triad locking, and
nonperiodic absorbing/dispersal boundaries. The machine-readable phase catalog
distinguishes these from conditionally invertible kicks and from the locally
involutive weak flip plus L/R swap.

## 3. Correct scope

Microscopic non-injectivity does not by itself prove that every coarse low-
energy sector violates experimental unitarity. That stronger conclusion needs
the preregistered manifested-state spectral-density matrix and information-loss
rate per oscillation. Those quantities are not measured here because FTD-0421
found no native conserved charge with which to define the charged spectral
sector.

Consequently the free flux sector passes its narrow reversibility gate, the
full tick fails fundamental injectivity, and emergent low-energy unitarity
remains an unresolved blocking requirement. No SME comparison is licensed:
FTD-0424 did not produce a gauge-independent on-shell coefficient.

**Artifacts:** `scripts/proofs/_native_tick_injectivity.json`,
`scripts/proofs/proof_native_unitarity_gate.py`, and
`engine/tests/test_native_injectivity_gate.cpp`.
