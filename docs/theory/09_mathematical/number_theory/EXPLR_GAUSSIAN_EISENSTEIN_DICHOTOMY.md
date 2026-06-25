# EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY: Why the Master Quadratic is Gaussian, and What the Odd Term Wants

**Tag:** `[EXPLORATORY MATH]` — mixed status; per-claim tags inline
**Date:** 2026-05-30 (updated 2026-05-30: arithmetic kernel promoted to `[THEOREM]`; updated 2026-06-01: added §5.1 ramified-prime structural synthesis — `[SYNTHESIS]`/`[STRONGLY MOTIVATED]`, promotes nothing)
**LEDGER:** FTD-0237
**Status:** One new `[THEOREM]` — **Theorem GE-1 (Gaussian coefficient coincidence)**, §3.1, a finitely-provable statement about integers ($2^4=4^2$ is the unique $a^b=b^a$ coincidence, hence the master-quadratic coefficient $16$ is canonically defined only for $\mathbb{Q}(i)$) — supporting the existing `[STRUCTURAL OBSERVATION]` (§3, the doubly-sourced coefficient) + a `[CLARIFICATION]` of the $\mathbb{Z}[\omega]\leftrightarrow G^{*3}$ reading (§4) + a reframing of the MC-T4.3 odd-term gap (§5, FTD-0235) + **a new `[SYNTHESIS]`/`[STRONGLY MOTIVATED]` structural synthesis (§5.1): the half-power obstruction is the ramified prime at 2 ($\sqrt2 = |1+i|$); local–global cleanliness; the ontic/epistemic seam on which $\alpha$ sits.** **The new theorem is about integers, not about $\alpha$.** Its FTD significance ("hardens the $d=-4$ selection; no Eisenstein twin") stays `[STRUCTURAL OBSERVATION]`/`[SELECTION]`; §5.1 is interpretive synthesis that **localizes** MC-T4.3 without closing it. **No spine change, no promotion of any physics claim, no new derivation of physics.**

---

## 0 · The question

A conjecture worth testing: do the master quadratic's two coefficients — $16G^{*2}$ (linear) and $16G^{*3}$ (constant) — carry a **Gaussian** ($\mathbb{Z}[i]$) vs **Eisenstein** ($\mathbb{Z}[\omega]$) structure, paralleling the cubic lattice's square (⟨110⟩, 45°) vs triangular (⟨111⟩) geometry? The motivating intuition came from two observations: (a) lattice field lines locking to 45° directions, and (b) the square/cube and $i$/$\omega$ duality of the two distinguished CM points $\tau=i$ ($|\mathrm{Aut}|=4$) and $\tau=\rho$ ($|\mathrm{Aut}|=6$).

**Result, in one line.** The $\mathbb{Z}[i]\leftrightarrow G^{*2}$ half is exactly right and theorem-grade; the $\mathbb{Z}[\omega]\leftrightarrow G^{*3}$ half is the wrong *label* for a real $D=3$ structure; and a unique integer identity ($2^4=4^2$) explains why no clean Eisenstein twin of the master quadratic exists.

---

## 1 · The geometry is the Moore Layer Theorem `[THEOREM, established]`

The 26-neighbour Moore shell decomposes uniquely as $26 = 6 + 12 + 8$ (see [`THEOREM_MOORE_LAYER_DECOMPOSITION.md`](../../08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md), [`DERIV_MOORE_GAUGE_STRUCTURE.md`](../../03_derivations/standard_model/DERIV_MOORE_GAUGE_STRUCTURE.md)):

| Shell | Directions | Angle to axes | Polyhedron | Sublattice | Gauge |
|---|---|---|---|---|---|
| 6 faces | ⟨100⟩ | 0° / 90° | octahedron | SC | U(1) |
| 12 edges | ⟨110⟩ | **45° in-plane** | **cuboctahedron** | FCC | SU(2) |
| 8 corners | ⟨111⟩ | **arccos(1/√3) = 54.7356°** | stella octangula | BCC | SU(3) |

Computed checks: $(1,1,1)$ is at 54.7° to *all three* axes ($\sum\cos^2 = 1$); a single vector at 45° to all three is **impossible** ($\sum\cos^2 = 1.5 \ne 1$). The observed "45° everywhere" is the field riding the ⟨110⟩/FCC shell; the tetrahedral structure is the ⟨111⟩/BCC corners (8 corners = two interlocking tetrahedra = stella octangula).

The **cuboctahedron** is the hinge polyhedron: its 14 faces are **6 squares** (normals along ⟨100⟩, the three $C_4$ axes) **+ 8 triangles** (normals along ⟨111⟩, the four $C_3$ axes $= N_{\text{base}}$). It is the unique Moore polyhedron carrying both 4-fold and 3-fold faces.

---

## 2 · $\mathbb{Z}[i] \leftrightarrow G^{*2}$ — correct and load-bearing `[DERIVED]`

The BCC structure factor is the **triple cosine product** $\sigma_{\text{BCC}}(k) = 1 - \cos k_x\cos k_y\cos k_z$. Two consequences follow from the single fact "BCC offsets have all three components nonzero" (see [`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](../../08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md)):

1. **Propagator:** $G_{\text{BCC}}(0) = \sum_m [\binom{2m}{m}/4^m]^3 = \Gamma(1/4)^4/(4\pi^3) = G^{*2}/(2\pi)$ — Watson's integral. This is the **trace** $16G^{*2}$ of the readout. `[THEOREM]`
2. **Gauge:** the triple-axis displacement excites all three flux components → SU(3).

And the BCC complex structure is literally $V_{\text{complex}} \cong \mathbb{Z}[i]^2$, with $J$ acting as $i$ ($J^2 = -I$) — see [`DERIV_BCC_COMPLEX_STRUCTURE.md`](../general_math/DERIV_BCC_COMPLEX_STRUCTURE.md) (FTD-0122). So $G^{*2}$ genuinely lives on a $\mathbb{Z}[i]^2$, and the **even term is forced by Watson**. The "Z[i] for G\*²" intuition is exactly right.

---

## 3 · The $2^4 = 4^2$ coefficient uniqueness `[STRUCTURAL OBSERVATION]` (new)

The master-quadratic coefficient **16 is doubly-sourced**, by two independent routes the corpus records separately:

- **Automorphism route:** $16 = |\mathrm{Aut}(E)|^2 = 4^2$ for the lemniscatic CM curve $E: y^2 = x^3 - x$ ([`SPEC_ALGEBRAIC_SPINE.md`](../../01_reference/SPEC_ALGEBRAIC_SPINE.md) §4, Theorem 4).
- **Tower route:** $16 = 2^4$, the $(1+i)$-tower base $2^k$ at level $k=4$, where $2 = |1+i|^2$ is the norm of the ramified prime ([`THEOREM_HARMONIC_INVARIANT_TOWER.md`](../../03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md), Theorem 8).

These two routes agree **only because of the unique integer identity**

$$2^4 \;=\; 4^2 \;=\; 16,$$

and $(2,4)$ is the **sole solution** of $a^b = b^a$ with $a \ne b$ (verified; this is now **Theorem GE-1**, §3.1). Phrased structurally: for an imaginary-quadratic CM field with extra automorphisms, the two coefficient routes coincide iff

$$(\text{ramified-prime norm})^{|\text{units}|} \;=\; |\text{units}|^2 .$$

(See §3.2 for the precise — and more limited — sense in which this reduces to Theorem GE-1: the reduction holds for $\mathbb{Q}(i)$ only, and depends on the *separate* field-specific fact that the Gaussian ramified-prime norm equals $2$.)

The only two such fields are $\mathbb{Q}(i)$ and $\mathbb{Q}(\rho)$ (the only ones with $|\text{units}| > 2$). For the **Eisenstein partner** $\mathbb{Z}[\omega]$ ($|\text{units}| = 6$, ramified prime $(1-\omega)$ of norm 3) the two routes **diverge**:

$$\underbrace{3^6 = 729}_{\text{tower route}} \;\;\ne\;\; \underbrace{6^2 = 36}_{|\mathrm{Aut}|^2\text{ route}} .$$

**Consequence.** The hypothetical equianharmonic master quadratic — which [`PAPER_GSTAR_INTRODUCTION.tex`](../../../papers/PAPER_GSTAR_INTRODUCTION.tex) §16 *mentions* as $y^2 - 36R_3^2\,y + 36R_3^3$ but tags `[OPEN CONJECTURE]` and never constructs — **has no canonical form**: its integer prefactor is ambiguous (36 from automorphisms, or 729 from the base-3 tower), whereas the Gaussian coefficient is over-determined. **There is no clean Eisenstein twin of the master quadratic.** This sharpens the corpus's open-conjecture status to a structural reason.

**Computed constants (mpmath, 40 dp):** $G^* = \Gamma(1/4)/\Gamma(3/4) = 2.95867512$; the d=3 Gauss-analog $G_\rho = 2.78265513844626$ (matches the η-tower back-out and the H1 atlas); the ratio-channel analog $R_3 = \Gamma(1/3)/\Gamma(2/3) = 1.97836$, with $|\eta(\rho)|^{12} = R_3^9/(216\pi^3)$.

**Non-result (honesty).** The harmonic invariant $1/y_+ + 1/y_- = 1$ holds **identically for both** the Gaussian and Eisenstein quadratics (and for any "constant $=$ const-factor $\times$ linear" polynomial), so it does **not** discriminate $\mathbb{Z}[i]$ from $\mathbb{Z}[\omega]$ — confirming [`EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md`](EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md) and [`THEOREM_HARMONIC_INVARIANT_TOWER.md`](../../03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md) §"multiplier underdetermination."

---

## 3.1 · The arithmetic kernel as a theorem `[THEOREM]` (new)

The structural observation of §3 has a finitely-provable arithmetic core. We isolate it, prove it, and tag it `[THEOREM]`. **The theorem is a statement about positive integers. It says nothing about $\alpha$, about physics, or about whether the FTD coefficient routes are the *correct* sources of $16$; those remain `[STRUCTURAL OBSERVATION]`/`[SELECTION]` (§3, §3.2 below).**

### Theorem GE-1 (Gaussian coefficient coincidence)

**Statement.** In positive integers, the equation
$$a^{b} \;=\; b^{a}, \qquad a \neq b,$$
has the **unique** solution $\{a,b\} = \{2,4\}$. Equivalently, $2^{4} = 4^{2} = 16$ is the only way a single integer is simultaneously a proper power $a^{b}$ and the "transposed" power $b^{a}$ for distinct positive-integer base/exponent pairs. Consequently the integer $16$ is the unique value at which the two exponential expressions $a^b$ and $b^a$ (over distinct positive integers) collapse to one number.

**Proof.**

*Step 1 — reduce to a one-variable monotonicity problem.* For positive integers $a \neq b$ with $a, b \geq 1$, note $a = 1$ forces $1 = b^{1}$, i.e. $b = 1 = a$, excluded; so $a, b \geq 2$. Take logarithms of $a^{b} = b^{a}$:
$$b \ln a = a \ln b \;\Longleftrightarrow\; \frac{\ln a}{a} = \frac{\ln b}{b}.$$
Thus $a \neq b$ solve $a^b = b^a$ **iff** the function $f(x) := \dfrac{\ln x}{x}$ takes the same value at the two distinct integers $a$ and $b$.

*Step 2 — shape of $f$.* $f$ is differentiable on $(0,\infty)$ with
$$f'(x) = \frac{1 - \ln x}{x^{2}}.$$
Hence $f'(x) > 0$ for $x < e$, $f'(e) = 0$, and $f'(x) < 0$ for $x > e$: $f$ is **strictly increasing on $(0,e)$ and strictly decreasing on $(e,\infty)$**, with a unique global maximum at $x = e \approx 2.71828$. Also $f(1) = 0$ and $f(x) \to 0^{+}$ as $x \to \infty$, with $f(x) > 0$ for all $x > 1$.

*Step 3 — a repeated value must straddle $e$.* Because $f$ is strictly monotone on each of $(1,e)$ and $(e,\infty)$, it is injective on each interval separately. So if $f(a) = f(b)$ with $a \neq b$ and $a, b > 1$, then $a$ and $b$ lie on **opposite sides** of $e$: one in $(1,e)$ and one in $(e,\infty)$. (Both in $(1,e)$ or both in $(e,\infty)$ would force $a=b$ by injectivity.)

*Step 4 — integrality pins the small one.* The only integer strictly between $1$ and $e \approx 2.71828$ is $2$. Hence the smaller solution must be $a = 2$.

*Step 5 — solve for the partner.* With $a = 2$, the partner $b > e$ solves $f(b) = f(2) = \tfrac{\ln 2}{2} = \tfrac{\ln 4}{4}$. Since $\tfrac{\ln 4}{4} = \tfrac{2\ln 2}{4} = \tfrac{\ln 2}{2}$, the value $b = 4$ satisfies $f(4) = f(2)$. By Step 2, $f$ is strictly decreasing on $(e,\infty)$, so the equation $f(b) = f(2)$ has **at most one** root in $(e,\infty)$; therefore $b = 4$ is the unique partner. The only solution with $a \neq b$ is thus $\{2,4\}$, giving $2^{4} = 4^{2} = 16$. $\qquad\blacksquare$

**Numerical confirmation (mpmath, in-session, not recalled).**
- Exhaustive integer search over $1 \le a < b \le 500$: the **only** pair with $a^{b} = b^{a}$ is $(2,4)$.
- $f(2) = \ln 2/2 = 0.34657359027997265470861606072908828$ and $f(4) = \ln 4/4$ agree to $45$ dp.
- $f'$ sign verified: $f$ increasing on $(1,e)$ ($f(1.5) < f(2) < f(2.5)$), decreasing on $(e,\infty)$ ($f(3) > f(4) > f(5)$); unique max at $e = 2.718281828\ldots$; $2$ and $4$ straddle $e$.
- Solving $\ln 2/2 = \ln b / b$ for $b > e$ returns $b = 4.000\ldots$ exactly.

> **Remark (why $16$ and not another doubly-sourced integer).** Theorem GE-1 is the precise sense in which the master-quadratic coefficient $16$ is *canonically* doubly-sourced: among all integers expressible as $a^b$ for $a\neq b$, only $16$ is *also* $b^a$. This is a property of the integer $16$, established by elementary calculus; it requires no FTD input and no choice of CM field. It is the theorem-grade residue of §3's structural observation.

## 3.2 · From the integer theorem to the field statement `[STRUCTURAL OBSERVATION]` / `[SELECTION]`

Theorem GE-1 is about integers. Connecting it to imaginary-quadratic fields requires the two FTD identifications of §3, **both of which carry their pre-existing tags and are not promoted here**:

- **(R1, tower route, `[SELECTION]`)** the coefficient enters as $N_{\mathrm{ram}}^{\,u}$, where $N_{\mathrm{ram}}$ is the norm of the ramified prime and the exponent is the unit count $u = |\mathcal{O}_K^\times|$ identified with the harmonic-tower level $k$ (per [`THEOREM_HARMONIC_INVARIANT_TOWER.md`](../../03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md); the harmonic invariant does **not** force base $2$, so the base$\leftrightarrow N_{\mathrm{ram}}$ and level$\leftrightarrow u$ identifications are `[SELECTION]`);
- **(R2, automorphism route, `[THEOREM]` for the count, `[SELECTION]` for the role)** the coefficient enters as $|\mathrm{Aut}(E)|^{2} = u^{2}$ (the curve's automorphism count is $u$; squaring is `[THEOREM at L=2]` per `SPEC_ALGEBRAIC_SPINE.md` §4 Theorem 4 / the coefficient-16 row).

The two routes give the **same integer** iff $N_{\mathrm{ram}}^{\,u} = u^{2}$.

**Precise scope (honesty — this is the part that is *not* a theorem).** The equation $N_{\mathrm{ram}}^{\,u} = u^{2}$ is *not* in general an instance of $a^b = b^a$. It reduces to Theorem GE-1 **only** for $\mathbb{Q}(i)$, and only because there a **second, independent** numerical fact holds: $N_{\mathrm{ram}}(\mathbb{Q}(i)) = 2$ **equals the fixed squaring exponent $2$** of route R2. With $N_{\mathrm{ram}} = 2$ and $u = 4$, R1 $= 2^{4}$ and R2 $= 4^{2}$ become the literal Theorem-GE-1 pair $2^4 = 4^2$. So the field-level coincidence is *two* facts stacked: (i) Theorem GE-1 ($2^4=4^2$ is the unique transposed-power coincidence), and (ii) the field-specific accident that the Gaussian ramified-prime norm is exactly $2$. Fact (i) is `[THEOREM]`; the *use* of (i) and (ii) together to declare $16$ "canonically the Gaussian coefficient" is `[STRUCTURAL OBSERVATION]`, since it rests on the `[SELECTION]` identifications R1/R2.

**No Eisenstein twin (corollary, `[STRUCTURAL OBSERVATION]`).** For $\mathbb{Q}(\rho)$: $N_{\mathrm{ram}} = 3$, $u = 6$. Route R1 gives $3^{6} = 729$; route R2 gives $6^{2} = 36$; these are unequal. The mismatch is robust to the exponent convention: even reading R2 with the "transposed" exponent $N_{\mathrm{ram}} = 3$ (i.e. $6^{3} = 216$) fails, since $729 \neq 216$. Equivalently, the analogue pair $(3,6)$ is **not** a solution of $a^b=b^a$ (it cannot be — by Theorem GE-1, $(2,4)$ is the *only* one). Hence **no single canonical integer prefactor exists for the would-be equianharmonic master quadratic**: the value is route-dependent ($729$ vs $36$), where the Gaussian value is route-independent ($16$). This is the structural reason the equianharmonic quadratic $y^2 - 36R_3^2\,y + 36R_3^3$ that [`PAPER_GSTAR_INTRODUCTION.tex`](../../../papers/PAPER_GSTAR_INTRODUCTION.tex) §16 mentions stays `[OPEN CONJECTURE]` with an ambiguous prefactor — and it sharpens, but does not change, that open status.

---

## 4 · $\mathbb{Z}[\omega] \leftrightarrow G^{*3}$ — the honest resolution `[CLARIFICATION]`

"Z[ω] for G\*³" is the wrong label, for two reasons, but it points at something real.

1. **$G^{*3}$ is Gaussian, not Eisenstein.** It is a *power of* $G^*$. The genuine Eisenstein constants are different objects ($R_3$, $G_\rho$, built from $\Gamma(1/3)$).
2. **The exponent 3 reads as $D = 3$, not $\omega$.** FTD's own live hint for the odd determinant ([`PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md`](../../10_eft_program/preregistrations/PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md), FTD-0235) is
   $$16G^{*3} \;=\; |\mu_4|^2 \cdot \!\!\prod_{\text{3 planes}}\!\! (\det\nolimits_\zeta \text{ ratio} = G^*),$$
   "the determinant carries **three** $\det_\zeta$ ratios, one per spatial plane, while the trace carries two." Those three planes are the **three coordinate planes — each a square $\mathbb{Z}[i]$ lattice**, not a hexagonal one. So $G^{*3}$ is best read as **$D=3$ copies of the Gaussian source**, not as an Eisenstein object.
3. **$\mathbb{Z}[\omega]$-as-ring is explicitly ruled out in $\mathbb{Z}^3$** ([`MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`](../../01_reference/MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md), `[SELECTION]` anchored in Axiom Zero): "Z[ω] lives on a hexagonal lattice… neither embeds in $\mathbb{Z}^3$ respecting ring structure; the cubic-lattice axiom selects $d=1$."

**What survives of the intuition (and it is real).** The three square $\mathbb{Z}[i]$ planes whose product gives $G^{*3}$ are **cyclically permuted by the $C_3$ rotation about the body diagonal** — the ⟨111⟩/triangular-face/tetrahedral 3-fold axis. So the "3-fold" *is* genuinely present, as the **rotational symmetry organizing the determinant's three planes**, even though the planes themselves are Gaussian. The conjecture conflated the *organizing 3-fold rotation* ($C_3 \subset O_h$, real, ⟨111⟩) with an *Eisenstein CM ring* ($\mathbb{Z}[\omega]$, translational, ruled out). The first is correct; the second is not. The honest slogan: **$G^{*3} = $ three $\mathbb{Z}[i]$ planes, glued by $C_3$ — $D$-fold, not $\omega$-fold.**

---

## 5 · Why this matters: the MC-T4.3 odd-term gap `[OPEN]`

The even/odd asymmetry between $G^{*2}$ and $G^{*3}$ **is** the current foundational obstruction (see [`WHERE_WE_LEFT_OFF.md`](../../../WHERE_WE_LEFT_OFF.md) §0.15; [`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`](../../10_eft_program/derivations/FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md)). The readout is a $2\times2$ transfer operator on $V_{\text{complex}} \cong \mathbb{Z}[i]^2$ with $(\mathrm{Tr}, \mathrm{Det}) = (16G^{*2}, 16G^{*3})$:

- **Trace $16G^{*2}$** = Watson Green's function → **forced** `[DERIVED]`.
- **Determinant $16G^{*3}$** = the odd term, *asserted* as the Vieta target, **not derived** (Watson gives $G^{*2}$, not $G^{*3}$) → `[UNDERDETERMINED]` (W-CRIT-2). A clean odd source exists (the J-twisted $\det_\zeta$ ratio $= G^*$, FTD-0234), but the **det$\det_\zeta$ structural identity** that would compel the determinant is the missing hinge (FTD-0235).

This note's contribution is to **localize and name** that gap: the trace is the 2-component $\mathbb{Z}[i]$ object (forced by Watson); the determinant wants the 3-plane product; and §3 shows why the natural "Eisenstein forcing" *cannot* supply it — the $2^4=4^2$ over-determination that forces the Gaussian side has no Eisenstein analog. So the odd term is underdetermined not by oversight but because the would-be forcing structure ($\mathbb{Z}[\omega]$) is ontologically rotational-only in $\mathbb{Z}^3$. **This reframes, but does not close, MC-T4.3.**

---

## 5.1 · The half-power obstruction is the ramified prime at 2 — a structural synthesis `[SYNTHESIS]` / `[STRONGLY MOTIVATED]`

The §5 gap is "the determinant wants an *odd* power of $G^*$ that the readout cannot force." Seen from the *realizer* side (the weight-½ objects that would supply a clean $\sqrt{G^*}$ to square into the determinant slot — catalogued in [`MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`](../../01_reference/MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md) Derivation 5 and [`DERIV_SPIN_STATISTICS_BRIDGE.md`](../../03_derivations/quantum_mechanics/DERIV_SPIN_STATISTICS_BRIDGE.md) §5.4), the obstruction has a single arithmetic root. This section names it. **Everything here is structural exploration; it promotes no physics claim, adds no spine theorem, and does not close MC-T4.3.**

### 5.1.1 · The stuck $\sqrt 2$ is the ramified prime; the clean $16$ is the units `[STRONGLY MOTIVATED]`

Every native degree-½ $G^*$-object is dressed, never unit-clean (VERIFIED, 16 dp):

| degree-½ object | value | dressing | place |
|---|---|---|---|
| theta-null $\theta_3(0,i) = \sqrt{G^*}/(2\pi)^{1/4}$ | $1.0864348\ldots$ | $(2\pi)^{1/4}$ (modular measure) | archimedean |
| $\det_\zeta(D_{3/4}) = 2^{1/4}\sqrt{G^*}$ | $2.0455313\ldots$ | $2^{1/4}$ (ramified prime) | non-archimedean |
| readout's needed trace $4\sqrt{G^*}$ | $6.8803199\ldots$ | $4 = |\mu_4|$ (units) | — |
| clean reference $\sqrt{G^*}$ | $1.7200800\ldots$ | none | — |

The non-archimedean dressing is **the ramified prime at 2**. In $\mathbb{Z}[i]$,

$$\sqrt 2 \;=\; |1+i|, \qquad 2 \;=\; -\,i\,(1+i)^2, \qquad N(1+i)=(1+i)(1-i)=2,$$

so $2$ is the unique ramified rational prime of $\mathbb{Q}(i)$ (exact, verified). The crucial point is that **ramification at 2 is intrinsic to the entire 2-power cyclotomic tower that $\mathbb{Z}[i]$ lives in** — the same tower that supplies the master quadratic's Gaussian content: the units $|\mu_4|=4$ giving $16 = 4^2 = |\mathrm{Aut}(E)|^2$ (the *trace*'s clean integer), and the complex structure $J^2=-I$ on $V_{\text{complex}}\cong\mathbb{Z}[i]^2$ (FTD-0122, the $i$ that makes the readout 2×2). You cannot reach into that tower for the units without also carrying its ramified prime.

This is why **refining $\mathbb{Z}/4 \to \mathbb{Z}/8$ — the $45^\circ$ diagonal/⟨110⟩ direction the user observes field lines locking to — repackages but cannot remove the obstruction**: $\mathbb{Q}(\zeta_8) \supset \mathbb{Q}(i)$ is a *further 2-power cyclotomic extension*, in which $2$ is *still* (more deeply) ramified ($2 = $ unit $\cdot (1-\zeta_8)^4$ up to the standard factorization). Going to the diagonal changes the *presentation* of the half-turn (a finer root of unity) but stays inside the 2-ramified tower, so the $\sqrt 2 = |1+i|$ dressing is carried along. The slogan:

> **The trace's clean integer $16$ is the UNITS of $\mathbb{Z}[i]$; the half-power's stuck $\sqrt 2$ is the RAMIFIED PRIME of $\mathbb{Z}[i]$. You can keep the units (and get the clean trace $16G^{*2}$) or get a clean $\sqrt{\text{-half}}$, but not both in one CM field** — because in $\mathbb{Q}(i)$ the prime whose square root you want to clean ($2$) is exactly the prime the discriminant ramifies at. (Cf. §3: the $2^4=4^2$ over-determination that *forces* the Gaussian trace has no Eisenstein twin; here, the same ramified-2 structure that forces the trace *obstructs* the clean half-power. One arithmetic fact, two faces.)

### 5.1.2 · Local–global: cleanliness is globality, a square root is a local act `[STRONGLY MOTIVATED]`

The product formula $\prod_v |x|_v = 1$ (over all places $v$ of $\mathbb{Q}$, archimedean and non-archimedean) is the precise statement that **integer-degree, norm-/product-built quantities balance across all places at once** — they are *global*. The forced ingredients of the readout are exactly of this kind: the trace $16G^{*2}$ is a Watson Green's function (a sum/product over the whole lattice), the units $16$ are a global count, $G^{*2} = 2\pi\,G_{\text{BCC}}(0)$ is a product-formula-clean period.

A **square root is an intrinsically local act**: $\sqrt{\cdot}$ does not commute with the global product structure — it must choose a branch, and that choice carries *one place's valuation* as a dressing. That is exactly what the table in §5.1.1 shows: the half-power picks up either the non-archimedean $2^{1/4}$ (the ramified-prime valuation $|\cdot|_2$) or the archimedean $(2\pi)^{1/4}$ (the measure at $|\cdot|_\infty$) — one place's worth of dressing, never zero. **Integer degree is clean because it is global (product-formula-balanced); half degree is dressed because it is local (it reads one valuation).** The determinant the readout needs is the *square* of a half-power, so it inherits the dressing the half-power could not shed.

### 5.1.3 · The ontic/epistemic seam — and α sits on it `[SYNTHESIS]`

The two preceding points line up with FTD's two-layer ontology in a way worth stating plainly (this is interpretive synthesis, `[SYNTHESIS]`, not a derivation):

- **The ontic side is the integer-degree, product-formula-clean menu.** Norms, products, periods, the trace $16G^{*2}$, the units $|\mu_4|^2 = 16$, the time-symmetric Euler-reflection content (cf. "The Ratio and the Arrow," where the *product* $\Gamma(1/4)\Gamma(3/4)=\pi\sqrt2$ is commutative and yields $\pi$). This is the substrate's forced output — what discreteness *determines*.
- **The epistemic side is the chiral half-degree act.** A square root selects one conjugate from the symmetric pair $z\cdot\bar z$; it is the *ratio* $\Gamma(1/4)/\Gamma(3/4)=G^*$ (non-commutative, the arrow), the half-turn, the measurement that breaks the $z\leftrightarrow\bar z$ symmetry the ontic side preserves. Measurement *is* the local, branch-selecting, valuation-carrying operation.
- **$\alpha$ sits on the seam.** The master quadratic's *trace* is ontic (forced, global, clean); its *determinant* requires the epistemic half-degree act (the unforced assembly = "Postulate 6" of [`AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md`](../../07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md) §8). So $\alpha$ is precisely the **output of the epistemic act on the ontic structure** — which is exactly why it is classified *dynamical, not structural* (that audit §5; contrast $N_c=3$, which is purely ontic/structural and forced). The "boundary" of MC-T4.3 is the seam itself: the substrate hands over a globally-clean menu, and reading a definite coupling off it requires one local, chiral, branch-selecting act that the menu does not itself contain.

**Operator-side twin + AGM witness.** This local–global / ontic-epistemic seam is the *arithmetic face* of the operator-side no-go FTD-0318 (`FOUND_MCT43_NATIVE_Z2_PERMANENCE.md`: no native ℤ/2 reaches `δ`, since the δ-selection is a Galois orbit no `ℚ(G*)`-entry operator performs). The two are unified in **FTD-0319** (`FOUND_AGM_PLACE_BRIDGE_AND_DELTA.md`), whose keystone is `G* = 2√π/AGM(1,√2)`: the AGM is the substrate's all-square-roots place-bridge, but its steps are *forced-magnitude* geometric means (FTD-0317), so the tower lands on `G*` (degree 1) and never on `δ` (degree 2) — the concrete witness that the substrate's √-machinery is δ-blind.

**Why this is `[STRONGLY MOTIVATED]` / `[SYNTHESIS]` and not a theorem.** The arithmetic facts (the ramification of $2$, $\sqrt2=|1+i|$, the product formula, the dressings in §5.1.1) are `[THEOREM]`-grade and verified. The *identification* of "global/integer-degree" with the ontic layer and "local/half-degree" with the epistemic/measurement act is a structural reading — compelling and internally consistent, but a `[SELECTION]`-grade interpretive bridge, not a forced consequence of the five postulates. It **localizes** the MC-T4.3 obstruction to the seam and explains the half-power's stubbornness; it does **not** close the obstruction (closing it is still RSI Leg-3 / Postulate-6, `[OPEN]`), does not promote `x_+ = 1/\alpha` (FTD-0013, `[STRONGLY MOTIVATED CONJECTURE]`), and adds no spine theorem.

**Cross-references for the arithmetic kernel.** The integer $4$ as ramified-prime-norm$^2$ is [`DERIV_INTEGER_4_UNIFICATION.md`](DERIV_INTEGER_4_UNIFICATION.md) ($2 = -i(1+i)^2$, $|\mathrm{disc}\,\mathbb{Q}(i)|=4$); the prime-splitting law of $\mathbb{Z}[i]$ that puts $2$ alone in the ramified class is [`DERIV_GSTAR_QUARTER_CONJUGACY.md`](../../03_derivations/foundational_mechanics/DERIV_GSTAR_QUARTER_CONJUGACY.md) §5.2 (residue classes mod 4; $p=2$ Ramified $= -i(1+i)^2$). The time-symmetric product vs. time-asymmetric ratio framing is "The Ratio and the Arrow."

---

## 6 · A falsifiable next step (ARC-D, engine-native) `[PROPOSED FALSIFIER]`

The surviving MC-T4.3 route is engine-native measurement. The trace/determinant split suggests a concrete test:

> **Measure, in the lattice engine, whether the oriented flux determinant on the 8 ⟨111⟩/BCC corners** (the stella-octangula structure, cyclically permuted by $C_3$) **dynamically realizes the three-plane product $\prod_{\text{3 planes}} G^* = G^{*3}$ — or prove it cannot.**

A confirming measurement would *force* the odd term and close MC-T4.3 positive; a null result closes it negative. Either is a result. The visual signature is exactly the user-observed split: field lines locking to ⟨110⟩ (the square/$\mathbb{Z}[i]$ trace layer) versus the ⟨111⟩ corners (the determinant/3-fold layer). This is a **theory-only proposal**; no engine run is asserted here.

---

## 7 · Scope, caveats, and what is NOT claimed

- The $2^4=4^2$ observation rests on the natural identifications "tower base = ramified-prime norm" and "level = $|\text{units}|$," both of which are themselves `[SELECTION]` per [`THEOREM_HARMONIC_INVARIANT_TOWER.md`](../../03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md) (the harmonic invariant does not force base 2). It is a structural observation, **not a forcing theorem**.
- No physical quantity is derived. The Eisenstein quadratic roots are **not** compared to any physical constant — doing so would be a coincidence search, forbidden by the project's epistemic discipline.
- This does **not** close MC-T4.3, does **not** promote $x_+ = 1/\alpha$ (FTD-0013, `[STRONGLY MOTIVATED CONJECTURE]`), and adds **no** spine theorem.
- The central new claim is `[STRUCTURAL OBSERVATION]`; the $\mathbb{Z}[\omega]\to D=3$ correction is `[CLARIFICATION]`; the MC-T4.3 reframing is `[OPEN]`.

---

## 8 · Verification

All numbers computed with `mpmath` (≥40 dp) in-session, not recalled:
- $G^* = \Gamma(1/4)/\Gamma(3/4) = 2.95867511918863889231\ldots$ (60 dp); for the FTD-0117 guard, $\varpi = G^*\sqrt{\pi}/2 = 2.62205755\ldots \neq G^*$.
- $G_\rho = 2.782655138446263$ from $|\eta(\rho)|^{12} = G_\rho^6/(216\pi^3)$, matching `PAPER_GSTAR_H1_ATLAS.tex` d=3.
- **Theorem GE-1 (§3.1):** exhaustive integer search $1 \le a < b \le 500$ gives the unique solution $(2,4)$ of $a^b = b^a$; $f(2) = \ln 2/2$ and $f(4) = \ln 4/4$ agree to 45 dp; $f'(x) = (1-\ln x)/x^2$ sign-verified (increasing on $(1,e)$, decreasing on $(e,\infty)$, max at $e=2.71828\ldots$); $2$ and $4$ straddle $e$; $\ln 2/2 = \ln b/b$ for $b>e$ returns $b = 4.000\ldots$.
- $2^4 = 4^2 = 16$; $3^6 = 729 \ne 6^2 = 36$; and $3^6 = 729 \ne 6^3 = 216$ (Eisenstein fails under **both** exponent conventions — §3.2 corollary).
- Harmonic invariant $1/y_+ + 1/y_- = 1$ verified to ~37 dp for the Gaussian and *both* Eisenstein routes → non-discriminating.
- Cuboctahedron ⟨110⟩ shell = 12 vertices; ⟨111⟩ angle = 54.7356°; $\sum\cos^2$ for ⟨111⟩ = 1, for a hypothetical 45°-to-all-axes = 1.5.
- **§5.1 ramified-prime synthesis (16 dp):** $\theta_3(0,i) = \pi^{1/4}/\Gamma(3/4) = \sqrt{G^*}/(2\pi)^{1/4} = 1.0864348112$; $\det_\zeta(D_{3/4}) = 2^{1/4}\sqrt{G^*} = 2.0455313442$; $4\sqrt{G^*} = 6.8803198986$; $\sqrt{G^*} = 1.7200799746$. Ramified prime: $2 = -i(1+i)^2$ (exact), $N(1+i) = 2$, $|1+i| = \sqrt2 = 1.4142135624$. Archimedean vs non-archimedean dressings $(2\pi)^{1/4} = 1.5832334871$ and $2^{1/4} = 1.1892071150$ are both $\ne$ any $\mathbb{Z}[i]$-unit.

## 9 · Cross-references

- Geometry: [`THEOREM_MOORE_LAYER_DECOMPOSITION.md`](../../08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md), [`DERIV_MOORE_GAUGE_STRUCTURE.md`](../../03_derivations/standard_model/DERIV_MOORE_GAUGE_STRUCTURE.md)
- $\mathbb{Z}[i]$ / Watson: [`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](../../08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md), [`DERIV_BCC_COMPLEX_STRUCTURE.md`](../general_math/DERIV_BCC_COMPLEX_STRUCTURE.md)
- Tower / coefficient: [`SPEC_ALGEBRAIC_SPINE.md`](../../01_reference/SPEC_ALGEBRAIC_SPINE.md) §4 §8, [`THEOREM_HARMONIC_INVARIANT_TOWER.md`](../../03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md), [`EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md`](EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md)
- Eisenstein / equianharmonic: [`PAPER_GSTAR_INTRODUCTION.tex`](../../../papers/PAPER_GSTAR_INTRODUCTION.tex) §16, [`PAPER_GSTAR_ETA_TOWER.tex`](../../../papers/PAPER_GSTAR_ETA_TOWER.tex), [`MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`](../../01_reference/MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md)
- MC-T4.3 odd term: [`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`](../../10_eft_program/derivations/FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md), [`PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md`](../../10_eft_program/preregistrations/PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md), [`AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md`](../../07_assessment/audits/AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md)
- §5.1 ramified-prime synthesis: [`MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`](../../01_reference/MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md) Derivation 5 (theta-null triplet), [`DERIV_SPIN_STATISTICS_BRIDGE.md`](../../03_derivations/quantum_mechanics/DERIV_SPIN_STATISTICS_BRIDGE.md) §5.4 (weight-½ spin object), [`AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md`](../../07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md) §8–§9 (Postulate-6 pricing), [`DERIV_INTEGER_4_UNIFICATION.md`](DERIV_INTEGER_4_UNIFICATION.md) ($4$ = ramified-prime-norm²), [`DERIV_GSTAR_QUARTER_CONJUGACY.md`](../../03_derivations/foundational_mechanics/DERIV_GSTAR_QUARTER_CONJUGACY.md) §5.2 (ℤ[i] prime splitting)
- Ledger: [`LEDGER.md`](../../07_assessment/core_ledgers/LEDGER.md) FTD-0237
