# AUDIT — Hop-mechanics underdetermination

**Date:** 2026-07-24  
**Identifier:** `FTD-0444`  
**Status:** `[THEOREM — SCALAR WORK UNDERDETERMINES LOCAL MECHANICS]` + `[CONSTRUCTIVE EXAMPLE — SELECTED REVERSIBLE MAP]`  
**Verdict:** `REVERSIBLE_SELECTED_MAP_BUT_LOCAL_DYNAMICS_UNDERDETERMINED`  
**Pre-registration:** [`PREREG_HOP_MECHANICS_UNDERDETERMINATION_v1.md`](../10_eft_program/preregistrations/PREREG_HOP_MECHANICS_UNDERDETERMINATION_v1.md)  
**Run of record:** `engine/results/ftd_0444/windows_msvc_cpu.csv`

## 1. Result

FTD-0443's exact scalar hop work is necessary but not sufficient to define a
mechanical update. Three independent ambiguities survive.

For any nonzero hop displacement `d` and scalar work `W`,

$$
F=\frac{W}{|d|^2}d+F_\perp,
\qquad F_\perp\cdot d=0
$$

has `F dot d = W`. The locked campaign constructed five distinct members for
each of the 26 Moore directions. Their worst work residual is
`1.10182e-17`, while the nearest non-longitudinal member remains `0.2` away.
Thus work leaves two transverse force degrees of freedom.

An on-shell energy fixes momentum magnitude, not direction. Six distinct axis
momenta have identical energy with zero measured residual and minimum pairwise
separation `0.0829458`.

Total momentum balance fixes only the sum of field recoil. Depositing the same
recoil on either of two different cells gives identical total vector and
identical quadratic norm, yet the field configurations differ by `0.0433359`.
Even momentum plus one energy-like quadratic constraint does not locate the
recoil.

## 2. Constructive reversible map

An analysis-only map was constructed by selecting three extra rules:

1. preserve momentum transverse to the hop;
2. preserve the sign of the longitudinal branch;
3. assign total field recoil as `p_before-p_after` without choosing its local
   field support.

Using the selected synthetic diagnostic

$$
E(p)=\sqrt{M_{inertial}^2+|p|^2/C_{speed}^2},
$$

the map closes forward/reverse work for all 26 directions. Worst energy
residual is `1.10182e-17`, round-trip momentum residual is `1.92296e-16`, and
global recoil residual is zero.

This is an existence proof, not native emergence. Different transverse rules,
branch rules, and recoil supports satisfy the same scalar work.

**Correction (FTD-0450):** this energy function is not the production flat
diagnostic. With production momentum it is exactly three times the engine's
energy because it uses `M_INERTIAL` instead of `E_REST=M_INERTIAL*C_SPEED^2`
and divides the momentum term by `C_SPEED^2` instead of multiplying it. The
algebraic reversibility example remains, but its production-compatible reading
is retracted and superseded by FTD-0450.

**Successor refinement (FTD-0447):** adding the explicit isolated-hop
assumption package—polar-vector response, no other local directional datum,
and invariance under the full cubic stabilizer of the hop—forces the response
direction to `span(d)`. Under those additional assumptions, the longitudinal
force representative is unique. Momentum-branch and local-recoil ambiguities
remain.

## 3. Ontological consequence

The current site action says how much the interaction changes when a state
moves between sites. It does not say how motion is executed. A scalar at the
endpoints cannot by itself supply an oriented local exchange history.

The missing primitive is therefore not another site force formula. It is a
spacetime-link transaction carrying at least:

- an oriented hop `a -> b`;
- signed transported state/current;
- interaction work on that link;
- particle impulse;
- an explicitly local field impulse/update;
- a reverse transaction that restores the complete prior particle-field
  state.

Those data may be derived from a new link action or introduced as selected
dynamics, but they are not consequences of the frozen site coupling alone.

## 4. Correct claim boundary

FTD now has an exact discrete potential difference and a reversible selected
particle update. It does not yet have native event mechanics, because the
impulse direction and local field recoil remain underdetermined. Calling the
selected longitudinal map derived would hide precisely the new structure that
must be justified.

The next admissible step is an observer-only spacetime-link contract. It must
enumerate its additional choices, conserve the complete registered
particle-field state locally, and close exact forward/reverse transactions
before any production integration.

## 5. Reproducibility

- campaign SHA256: `1f3dfc3874939404eeb805c6a08c8f34b30244b99fb465badbf61ec32da347ba`
- helper SHA256: `c966e6c9963774b941363e03788fadec7f71d17ceaffd24f0fd65307fbe2f045`
- record SHA256: `99742d965b3b931f0500bc61d6216c1c830f66b37c266ad30ba927f9cda2452d`
- compiler: pinned MSVC `14.44.35207`, Release
- execution: algebraic observer, no production tick
- result: `REVERSIBLE_SELECTED_MAP_BUT_LOCAL_DYNAMICS_UNDERDETERMINED`

No production dynamics were changed.
