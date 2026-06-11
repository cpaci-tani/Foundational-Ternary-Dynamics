# DERIV — A1 + A2: Dirac-Kähler Identification + Mass-Ratio No-Go

**Tag:** A1 = [STRUCTURAL IDENTIFICATION]; A2 = [THEOREM NEGATIVE]
**Ledger row:** FTD-0089
**Filed:** 2026-04-25
**Companions:**
- FTD-0086 — F-prime bivector matching signature
- FTD-0088 — Path 1 multi-grade decomposition (12/12 PASS)
- FTD-0073 — Mode-erasure theorem
- [DERIV_NC_FROM_TOPOLOGY.md](../03_derivations/standard_model/DERIV_NC_FROM_TOPOLOGY.md) — N_c topology argument

---

## Executive statement

Two structural results from the Branch-B intuition exploration:

**A1 (POSITIVE):** FTD's measured 4-grade structure $(S, V_i, P_{ij}, T)$ on the $3^3$ Moore block is **exactly a Dirac-Kähler field** on the cubic lattice. This is a clean identification with a known mathematical object — a Dirac-Kähler field's exterior-algebra components $(0, 1, 2, 3)$-forms map directly to FTD's grades $(0, 1, 2, 3)$.

**A2 (NEGATIVE):** Cl(3,0) by itself cannot predict fermion **mass ratios**. The bivector subalgebra $\mathfrak{su}(2)$ has exactly one Casimir invariant; one Casimir gives one mass scale, not a hierarchy. SM mass ratios ($m_\mu/m_e \approx 207$, $m_\tau/m_\mu \approx 17$) require **flavor structure beyond Cl(3,0)** — multiple representations or symmetry-breaking Yukawa-like couplings.

**Net implication for Branch-B accounting:** FTD's Cl(3,0) skeleton provides the *spinor structure* (Dirac-Kähler identification) but NOT the *mass hierarchy* (no-go). The Branch-B selection must provide:
1. A flavor index (3 generations).
2. A Yukawa-like coupling that breaks Casimir degeneracy.
3. Connection to G* / α for the absolute mass scale.

---

## A1 — Dirac-Kähler Identification

### A1.1 What the Dirac-Kähler field is

The Dirac-Kähler (DK) field on a $D$-dimensional manifold is a section of the exterior algebra:
$$
\Phi = \phi^{(0)} + \phi^{(1)}_\mu dx^\mu + \phi^{(2)}_{\mu\nu} dx^\mu \wedge dx^\nu + \cdots + \phi^{(D)}_{\mu_1 \ldots \mu_D} dx^{\mu_1} \wedge \cdots \wedge dx^{\mu_D}
$$

For $D = 4$ (spacetime): $1 + 4 + 6 + 4 + 1 = 16$ components $= 4 \times 4$ = four Dirac spinors.
For $D = 3$ (spatial slice): $1 + 3 + 3 + 1 = 8$ components $= 2 \times 4$ = two Dirac spinors.

The DK equation:
$$
(d - \delta)\Phi = m \Phi
$$
where $d$ is exterior derivative ($d^2 = 0$) and $\delta = (-)^? \star d \star$ is codifferential. After basis change, DK is equivalent to four decoupled Dirac equations (Becher-Joos 1982, Rabin 1982).

DK is the **canonical staggered-fermion construction on a cubic lattice** — it provides fermions without doubling, using the lattice's natural form structure.

### A1.2 The mapping to FTD's measured grades

From FTD-0088, the 4-grade decomposition of FTD's 2-injection response on the $2^3$ block:

| Cl(3,0) grade | FTD observable | Form on $3^3$ lattice | DK component |
|---|---|---|---|
| Grade 0 (scalar) | $S = \sum_x \|J(x)\|^2$ | 0-form $\phi^{(0)}(x)$ | 1 component |
| Grade 1 (vector) | $V_i = \sum_x J_i(x)$ | 1-form $\phi^{(1)}_i(x)$ on edges | 3 components |
| Grade 2 (bivector) | $P_{ij} = \sum_x [J_i(x) J_j(x+\hat{e}_i) - J_i(x+\hat{e}_j) J_j(x)]$ | 2-form $\phi^{(2)}_{ij}(x)$ on plaquettes | 3 components |
| Grade 3 (pseudoscalar) | $T = \sum_x J_x(x) J_y(x) J_z(x)$ | 3-form $\phi^{(3)}_{xyz}(x)$ on cubes | 1 component |

**Total: 1 + 3 + 3 + 1 = 8 components** = the spatial slice of a 4D DK field = **2 Dirac spinors per spatial site**.

With time (the FTD tick) added as a fourth dimension, the spatial 8 components become $8 \times 2 = 16$ components = **4 Dirac spinors**.

### A1.3 The discrete DK equation in FTD's lattice language

Define forward and backward lattice differences:
$$
(\nabla^+_i f)(x) = f(x + \hat{e}_i) - f(x), \qquad (\nabla^-_i f)(x) = f(x) - f(x - \hat{e}_i).
$$

Discrete exterior derivative $d$ raises form degree:
- $d \phi^{(0)}$ on edge $i$: $\nabla^+_i \phi^{(0)}$
- $d \phi^{(1)}$ on plaquette $(i,j)$: $\nabla^+_i \phi^{(1)}_j - \nabla^+_j \phi^{(1)}_i$
- $d \phi^{(2)}$ on cube: $\sum_{\text{cyclic}} \nabla^+_k \phi^{(2)}_{ij}$

Discrete codifferential $\delta$ lowers form degree (formal adjoint of $d$):
- $\delta \phi^{(1)}$ on site: $\sum_i \nabla^-_i \phi^{(1)}_i$ (divergence)
- $\delta \phi^{(2)}$ on edge $i$: $\sum_j \nabla^-_j \phi^{(2)}_{ij}$
- $\delta \phi^{(3)}$ on plaquette $(i,j)$: $\sum_k \nabla^-_k \phi^{(3)}_{ijk}$

The discrete Dirac-Kähler equation $(d - \delta)\Phi = m\Phi$ becomes a coupled system:
$$
\begin{aligned}
\text{(0-form):} \quad &-(\delta \phi^{(1)})(x) = m \phi^{(0)}(x) \\
\text{(1-form):} \quad &(d\phi^{(0)})_i(x) - (\delta \phi^{(2)})_i(x) = m \phi^{(1)}_i(x) \\
\text{(2-form):} \quad &(d\phi^{(1)})_{ij}(x) - (\delta \phi^{(3)})_{ij}(x) = m \phi^{(2)}_{ij}(x) \\
\text{(3-form):} \quad &(d\phi^{(2)})_{xyz}(x) = m \phi^{(3)}_{xyz}(x)
\end{aligned}
$$

In FTD notation:
$$
\boxed{
\begin{aligned}
-\sum_i \nabla^-_i V_i &= m \cdot S \\
\nabla^+_i S - \sum_j \nabla^-_j P_{ij} &= m \cdot V_i \\
\nabla^+_i V_j - \nabla^+_j V_i - \nabla^-_k T &= m \cdot P_{ij} \quad (k = \text{third axis}) \\
\sum_{\text{cyclic}} \nabla^+_k P_{ij} &= m \cdot T
\end{aligned}
}
$$

This is the natural Dirac-Kähler equation on the FTD lattice, written in terms of the four measured grade observables.

### A1.4 What this is and isn't

**This IS:**
- A clean structural identification: FTD's 4-grade response on the $3^3$ block = the spatial DK field structure.
- The natural lattice fermion construction without doubling, using FTD's geometric form structure.
- Suggesting that FTD's flux dynamics SHOULD satisfy the DK equation if the Cl(3,0) skeleton extends to evolution dynamics.

**This is NOT:**
- A proof that the DK equation is satisfied by FTD's evolution. That's a separate verification test (measure how $S, V_i, P_{ij}, T$ evolve over multiple ticks under non-local dynamics, check whether the DK equation holds with some effective $m$).
- A derivation of the SM Lagrangian. DK gives 4 Dirac fermions per site; the SM has 12 Weyl fermions per generation × 3 generations = 36. The mapping to SM content requires additional selection.

### A1.5 Verification path

A separate test could verify whether FTD's evolution satisfies the DK equation:

1. Initialize $S(x), V_i(x), P_{ij}(x), T(x)$ from a 2-injection protocol (FTD-0088 grade decomposition).
2. Run engine for $N$ ticks with full non-local dynamics.
3. Compute $\nabla^-_i V_i$ vs $S$ at each tick; check whether $\frac{dS}{dt} \approx -\sum_i \nabla^-_i V_i$ (continuity-like).
4. Similarly for the higher-form equations.

If $S$ evolves like $-\sum_i \nabla^-_i V_i$ to within the same ~1% deviation we measured for the static skeleton, **DK evolution is verified**. If not, the DK identification is a static-skeleton match without dynamical content.

---

## A2 — Mass-Ratio No-Go

### A2.1 The structural argument

**Theorem (Cl(3,0) mass-ratio no-go):** A field theory with a Cl(3,0) Clifford structure and no additional flavor/family index predicts ONE mass scale, not a hierarchy.

**Proof sketch:**

1. Cl(3,0) has bivector subalgebra $\mathfrak{su}(2) \cong \mathfrak{so}(3)$.

2. $\mathfrak{su}(2)$ has rank 1 — exactly one Cartan generator. Equivalently, exactly one independent Casimir invariant: $C_2 = \sum_i b_i^2$.

3. By Schur's lemma, on each irreducible representation of $\mathfrak{su}(2)$, the Casimir $C_2$ acts as a scalar. The mass operator (commuting with the algebra) is therefore proportional to identity within each irrep.

4. The natural mass formula has the form $m \propto \sqrt{C_2(\text{rep})}$ — a single-scale formula determined by the Casimir eigenvalue.

5. To produce a HIERARCHY $m_1 \ne m_2 \ne m_3$, you need either:
   - **(i)** Three different irreducible representations of Cl(3,0) (different spin quantum numbers $j = 1/2, 1, 3/2, \ldots$). But these irreps would have masses in the ratio $\sqrt{j(j+1)}$, giving $1 : \sqrt{2} : \sqrt{15/4}$ etc. — none of which match SM lepton ratios $1 : 207 : 3477$.
   - **(ii)** A symmetry-breaking term in the Lagrangian that distinguishes flavors. This is the Yukawa structure in the SM. But Yukawa requires a flavor index AND a coupling function, neither of which is provided by Cl(3,0) alone.

6. Cl(3,0) has no native flavor index (all generators $e_i$ are equivalent up to $\mathfrak{so}(3)$ rotation; there's no way to distinguish "first-generation $e_1$" from "second-generation $e_1$"). Therefore (i) is unavailable.

7. Cl(3,0) has no native Yukawa structure (the algebra has no scalar Higgs-like field that couples differently to different irreps). Therefore (ii) is unavailable.

8. **Conclusion:** Cl(3,0) alone gives one mass scale. Mass ratios are independent of FTD's algebraic content.  $\square$

### A2.2 What this means

Branch-A derivation of fermion content from FTD's Cl(3,0) skeleton can produce:
- The spinor structure (Dirac-Kähler, A1 above).
- The single mass scale $\propto \sqrt{C_2} \cdot G^* \cdot \alpha$.
- The SU(2) gauge structure (FTD-0086 bivector subalgebra).

It **cannot** produce:
- The three-generation structure.
- The mass ratios $m_e : m_\mu : m_\tau$.
- The CKM/PMNS mixing angles.
- The flavor structure of weak interactions.

These remain Branch-B selections, structurally outside the Cl(3,0)-only derivation.

### A2.3 Why the F-prime structure constants are NOT mass ratios

A naive reading of FTD-0086 might suggest the bivector commutator values $(2.34, 9.00, 3.18)$ are mass-related. They are not:

1. Multi-seed average (FTD-0087): the values are seed-dependent (one is 0.499 ± 0.000, another 11.78 ± 1.12). They reflect injection-protocol normalization, not physical couplings.

2. The protocol: WH amplitude $A = 10$ is arbitrary. Doubling $A$ changes all three values by $A^2 = 4 \times$. Physical mass ratios cannot depend on protocol normalization.

3. The structure constants of $\mathfrak{su}(2)$ are equal up to sign by the algebra itself: $\epsilon_{ijk} = \pm 1$. Any deviation from equality measures *deviation from clean* $\mathfrak{su}(2)$, not a mass hierarchy.

4. **Treating these values as predictions would be numerical fishing** (CLAUDE.md violation).

The 1% pseudoscalar leakage measured in FTD-0088 is the cleanest "deviation from Cl(3,0)" signal, but it's a single number, not three — still no mass hierarchy.

### A2.4 What Branch-B selection must provide for masses

A consistent Branch-B selection that completes FTD's Cl(3,0) skeleton to SM fermion content must specify:

1. **Flavor index**: three generations × two doublet components = 6 flavors. The Moore Layer Theorem (FOUND_PHENOMENAL_NOUMENAL_BRIDGE.md) suggests 6 from face-orbit size. This is consistent with the flavor count but doesn't fix individual masses.

2. **Yukawa coupling structure**: a function $y_i(\text{flavor})$ that breaks Casimir degeneracy. The SM uses $\bar\psi \cdot H \cdot \psi$ with $H$ a Higgs scalar; FTD's analog would be $\bar\psi \cdot \Phi^{(0)} \cdot \psi$ where $\Phi^{(0)}$ is the scalar grade. Connection to FTD's measured scalar Casimir is open.

3. **Absolute mass scale**: $m \cdot$ (dimensional unit). The current $m_e$ formula $m_e = m_P \sqrt{2\pi}(16/3) \alpha^{11}$ provides this, but it's [SELECTION] (FTD-0077) — not derived from Cl(3,0).

4. **Mixing angles**: CKM, PMNS. No structural input from FTD; pure Branch-B selection.

These four selections are the minimum to extend Cl(3,0) → full SM fermion content. None of them are derivable from FTD's lattice geometry alone.

---

## Combined implication

After A1 + A2, the Branch-B accounting is **sharper**:

| Component | FTD provides? | Status |
|---|---|---|
| Spinor structure (Cl(3,0) Dirac-Kähler) |  | Branch-A (A1, FTD-0088) |
| SU(2) gauge structure (bivector subalgebra) |  approximate (1% deviation) | Branch-A (FTD-0086) |
| C_3 ⊂ SU(3) for color |  discrete only | Branch-A (FTD-0077) |
| Single fermion mass scale |  in principle | Branch-A (with G*, α input) |
| Three generations (flavor count) |  structurally consistent | Branch-B selection respecting Moore-26 |
| Mass hierarchy ($m_\mu/m_e$, etc.) |  A2 no-go | Branch-B selection |
| CKM/PMNS mixing |  no structural input | Branch-B selection |
| Yukawa coupling structure |  no structural input | Branch-B selection |

**FTD provides the algebraic and structural skeleton; Branch-B provides the flavor-and-mass content.**

This is more honest than either "FTD derives the SM" (it doesn't, A2) or "FTD says nothing about fermions" (it provides Dirac-Kähler structure, A1).

---

## Status

**A1: STRUCTURAL IDENTIFICATION** — FTD's 4-grade decomposition = Dirac-Kähler field on cubic lattice. The discrete DK equation is written in FTD's lattice language above.

**A2: THEOREM NEGATIVE** — Cl(3,0) alone predicts one mass scale, not a hierarchy. Mass ratios require Branch-B selection.

Both are clean structural results obtained without numerical fishing or substitution identities.

Open follow-ups:
- **Verify** the DK equation is satisfied by FTD's evolution (separate measurement test, multi-tick).
- **Quantify** the 1% deviation: characterize how the approximate-Cl(3,0) breaks into a deformation parameter that connects to FTD's pseudoscalar leakage.
- **Specify** the minimal Branch-B selection (4 components: flavor index, Yukawa structure, mass scale, mixing angles).

---

*Filed 2026-04-25. Responds to A1+A2 from the Branch-B intuition exploration. A1 cleanly identifies FTD's 4-grade decomposition with the Dirac-Kähler field on the cubic lattice — a known mathematical object whose discrete equation is now written in FTD's language. A2 establishes a structural no-go: Cl(3,0) alone has one Casimir, which gives one mass scale, not a hierarchy. Mass ratios require Branch-B input. Together: FTD provides the algebraic skeleton; Branch-B selection provides the flavor-and-mass content.*
