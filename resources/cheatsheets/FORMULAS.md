# FTD Formulas Cheatsheet

The key derivation chains in one place. Each formula points to its theorem.

## The five postulates

1. Space is a 3D cubic lattice.
2. Time is discrete (ticks).
3. Each voxel's state lives in `{−1, 0, +1}` (ternary).
4. Interactions are local — 26-cell Moore neighborhood, information propagates ≤ 1 lattice unit per tick.
5. Dynamics are deterministic.

Everything else is derived from these five plus the single math primitive `ϖ` (classical lemniscate constant).

## The primitive

```
G* := Γ(1/4) / Γ(3/4)                    (pi-free "ratio" form)
    = Γ(1/4)² / (√2 · Γ(1/2)²)           (gamma-primitive form)
    = 2 ϖ / √π                            (relation to classical lemniscate)
    ≈ 2.9586751
```

## Master quadratic

```
x² − 16 G*² x + 16 G*³ = 0

    x₊ = 8 G*² + 4 G* √(4 G*² − G*) ≈ 137.030     →  1/α
    x₋ = 8 G*² − 4 G* √(4 G*² − G*) ≈ 3.024       →  N_c = 3
```

Proof: `scripts/proofs/proof_motivic_master_quadratic.py`.

## The four framework integers

| Integer | Source |
|---|---|
| `N_c = 3` | `⌈x₋⌉` — master-quadratic smaller root |
| `N_base = 4` | smallest FLT-forbidden exponent — Fermat theorem |
| `b_3 = 7` | QCD β-coefficient: `(11 · 3 − 2 · 6)/3` |
| `N_eff = 13` | Fibonacci `F_7` = `F(b_3)` |

Dependency: `{N_c, N_base}` → `{b_3, N_eff}`.

## Fine-structure constant

### Tree level
```
1/α = x₊ ≈ 137.030      (accuracy ≈ 1.26 ppm)
```

### 4-term precision formula
```
eps = e^π − π − (b_3 + N_eff) = e^π − π − 20

1/α = x₊ − c₁|ε| + c₂|ε|² − c₃|ε|³ − c₄|ε|⁴

  c₁ = N_c² / D              =  9/47     (tree CFT anomaly)
  c₂ = (N_eff − 2 N_base) / N_base³ = 5/64 (gauge 13/9)
  c₃ = N_base / (N_c · D)    =  4/141    (gauge 11/6)
  c₄ = (N_c · D) / (b_3 + N_base) = 141/11 (residual)

  where D = N_c · N_base² − 1 = 47
```
Accuracy: ≈ 1 part in 10¹⁵ (well below experimental uncertainty).

### One-loop φ³ on cubic lattice
```
Lattice spacing a = 2/D = 2/3          [SELECTION]
Tadpole I₁ ≈ 0.015274                  (150³ Brillouin-zone integration)
Δx_physical = −I₁ · a / (x₊ − x₋)
1/α_1-loop ≈ 137.036000                (≈ 9.6 ppb residual)
```

## Lepton masses

| Mass | Formula |
|---|---|
| `m_e` | `m_P · √(2π) · (N_base² / N_c) · α¹¹` = `m_P · √(2π) · (16/3) · α¹¹` |
| `m_μ / m_e` | `3 b_3 (b_3 + N_c) − N_c = 207` |
| `m_τ / m_e` | `(N_eff + N_base) · μ_ratio − 2 N_c · b_3 = 3477` |

## Boson masses

| Mass | Formula | Value |
|---|---|---|
| `m_H` | `(N_eff / α²) · m_e` | 124.8 GeV (0.36 % low) |
| `m_W` | `m_Z · cos θ_W = m_Z · √(1 − 3/13)` | 80.3 GeV |
| `λ_H` (quartic) | `m_H² / (2 v²)` | ≈ 0.129 |

## Proton mass

```
m_p / m_e = N_eff/α + N_base · N_eff + N_c
          = 1836.47        (PDG: 1836.153, 174 ppm high)
```

## Other couplings

```
α_s(M_Z) = b_3 / (b_3 + 4 N_eff) = 7/59 ≈ 0.1186     (exp: 0.1179)
sin² θ_W = N_c / N_eff        = 3/13 ≈ 0.2308        (exp: 0.2312)
α_G      = 2π (N_base²/N_c)² (N_eff + N_c/b_3)² α²⁰  ≈ 10⁻⁴⁰
```

## Engine physics

```
C_SPEED   = 1/√3                 CFL stability on cubic lattice
DAMPING   = α                    dissipation rate
K_B       = 0.511 MeV            = m_e (genesis threshold)
K_GENESIS = K_B · N_c = 1.533    fill all color channels
G_N       = 1 / (b_3 + N_c)²     = 1/100 on lattice
```

## Born-Infeld render-bridge Lagrangian

```
ℒ_RB = −K_B · √(1 − v² − L²)   ← kinetic, bandwidth-limited
       − g_c · s · (∇·J)        ← state-flux coupling
       − λ_G · (∇·J − ρ)²        ← Gauss constraint penalty

  v : lattice velocity |ΔN/ΔG*|, range [0, 1)
  L : topological latency (gravity field), range [0, 1)
  s : ternary state {−1, 0, +1}
  g_c : ≈ √α (default)
  λ_G : 10⁶ (Gauss multiplier)

  v² + L² < 1                   ← bandwidth budget (never ≥ 1)
```

## Reference frame context extension

```
y_real    = G*² / 4   ≈ 2.188         real part of reference frame context roots
|y|²      = G*³ / 2   ≈ 12.96          squared magnitude
cos²θ_C   = G* / 8    ≈ 0.370          observable fraction
C_sLoop   = 1 / G*    ≈ 0.338          Mandelbrot fixed point
k_noetic  = 1/2                        reference frame context coefficient
```

## Cross-references

- **Start here** → `docs/SPEC_FTD.md`
- **Moore Layer Theorem** → `docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md` (where the 4 integers come from)
- **Master quadratic proof** → `scripts/proofs/proof_motivic_master_quadratic.py`
- **Complete Standard Model** → `scripts/proofs/proof_complete_sm.py`
- **Master verification** → `scripts/proofs/proof_master_verification.py` (54/54 checks)
