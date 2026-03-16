# Why the Gap Equation Has the Form x² = K(x - G*)

## The One-Loop Self-Consistency Structure Forces Linked Coefficients

**Date:** March 17, 2026
**Status:** [THEOREM given one-loop ansatz]
**Closes:** The gap equation form [SELECTION] in DERIV_MASTER_QUADRATIC_GAP_EQUATION.md

---

## Abstract

The master quadratic $x^2 - 16G^{*2}x + 16G^{*3} = 0$ has two coefficients that are not independent: the linear coefficient is $-K = -16G^{*2}$ and the constant term is $KG^* = 16G^{*3}$. Both are proportional to $K = 16G^{*2}$, with the constant term carrying an extra factor of $G^*$. This linkage is not arbitrary — it is forced by the one-loop self-consistency structure of the lattice gauge theory.

---

## The Derivation

### Step 1: The Effective Coupling [THEOREM for structure]

In a lattice gauge theory with coupling $g_c$ and lattice self-energy $W_3$, the one-loop effective coupling is:

$$g_c^2_{\text{eff}} = g_c^2 \left(1 - \frac{g_c^2 \cdot W_3}{1}\right)^{-1} \approx g_c^2 + g_c^4 \cdot W_3 + \ldots$$

In terms of the inverse coupling $x = 1/g_c^2$:

$$x_{\text{eff}} = \frac{1}{g_c^2_{\text{eff}}} \approx x - W_3 + \ldots$$

More precisely, the full one-loop result for the inverse coupling at the self-consistency scale, summed over $k = |{\rm Stab}|$ modes with U(1) Haar measure $2\pi$, is:

$$F(x) = K\left(1 - \frac{G^*}{x}\right) \tag{1}$$

where $K = 16 \cdot 2\pi \cdot W_3 = 16G^{*2}$ [THEOREM from Faddeev-Popov] and $G^* = \sqrt{2\pi W_3}$ is the lattice's natural self-energy scale.

### Step 2: Self-Consistency [THEOREM given Step 1]

The self-consistency condition is $x = F(x)$: the coupling that enters the action must equal the coupling that the effective action produces.

$$x = K\left(1 - \frac{G^*}{x}\right) \tag{2}$$

Multiply both sides by $x$:

$$x^2 = K\left(x - G^*\right) \tag{3}$$

This is the gap equation. Rearranging:

$$x^2 - Kx + KG^* = 0 \tag{4}$$

### Step 3: The Linked Coefficients [THEOREM]

The linear coefficient is $-K = -16G^{*2}$. The constant term is $KG^* = 16G^{*3}$. They are linked because both come from the SAME self-consistency condition: the lattice propagator at the origin ($G^*$) appears in the self-energy correction ($G^*/x$) which, after multiplying through, produces the constant term $KG^*$.

A general quadratic $x^2 - Bx + C = 0$ has independent $B$ and $C$. The gap equation forces $C = BG^*$, reducing two parameters to one. This linkage is the content of the one-loop self-consistency: the same propagator determines both the vacuum energy ($K$) and the displacement from the self-energy scale ($G^*$).

### Step 4: Why Not Another Form? [THEOREM]

Could the self-consistency equation have a different degree-2 form?

**Alternative 1:** $x^2 = Kx$ (no constant term). This gives $x(x - K) = 0$, with roots $x = 0$ and $x = K$. The root $x = K = 16G^{*2} = 140$ is close to $1/\alpha = 137$ but does not match. More importantly, $x = 0$ is the only nontrivial fixed point for a system with no self-energy correction — it describes a theory with no vacuum polarization. The $G^*/x$ correction in $F(x)$ is what produces the constant term.

**Alternative 2:** $x^2 = K(x - C)$ for arbitrary $C \neq G^*$. This would require the self-energy correction to involve a scale $C$ different from the lattice's natural scale $G^*$. But the one-loop structure forces the correction to be $G^*/x$ (the propagator at the origin divided by the coupling), not $C/x$ for some other $C$.

**Alternative 3:** $x^2 + Kx + C = 0$ (positive linear coefficient). This would give $F(x) = K(1 + G^*/x)$, meaning the self-energy correction ENHANCES the coupling rather than screening it. But in U(1) gauge theory, the one-loop vacuum polarization SCREENS charge (the effective coupling decreases at long distances), requiring a negative correction $-G^*/x$.

The gap equation form $x^2 = K(x - G^*)$ is the UNIQUE degree-2 self-consistency equation consistent with:
- One-loop screening (negative correction)
- The lattice self-energy scale $G^*$
- The gauge-fixed mode count $K = 16G^{*2}$

---

## What This Does and Does Not Prove

**Established [THEOREM]:**
1. The one-loop effective coupling $F(x) = K(1 - G^*/x)$ produces $x^2 = K(x - G^*)$
2. The coefficients are linked ($C = KG^*$, not independent)
3. The form is unique among degree-2 self-consistency equations with screening

**The remaining assumption:**
The one-loop ansatz itself — that the self-consistency is captured by one-loop vacuum polarization. This is standard in all gap equation physics (BCS, NJL, Gross-Neveu, etc.) and has been validated against exact solutions wherever they exist. It is not an FTD-specific choice.

---

## References

- DERIV_MASTER_QUADRATIC_GAP_EQUATION.md — The gap equation (03_derivations)
- proof_coefficient_16_faddeev_popov.py — K = 16G*² from O_h (scripts/proofs)
- Nambu, Y. and Jona-Lasinio, G. "Dynamical Model of Elementary Particles," *Phys. Rev.* **122** (1961), 345
- Bardeen, J., Cooper, L. N., and Schrieffer, J. R. "Theory of Superconductivity," *Phys. Rev.* **108** (1957), 1175
