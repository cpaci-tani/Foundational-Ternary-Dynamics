"""
moore_shell_dm_baryon_weightings.py — enumerate natural per-site weightings
for the Moore-shell partition and check which (if any) reproduces the
Planck 2018 Ω_DM/Ω_b ≈ 5.375.

Disciplined analysis per CLAUDE.md anti-target rules:
- Each weighting must derive from a structural feature of FTD that does NOT
  reference Planck (otherwise it's a fit, not a derivation).
- A close match is a [OBSERVATION], not [DERIVED], unless the weighting itself
  is structurally privileged with independent justification.
- Honest framing: this maps which weightings give which predictions; it does
  NOT claim to derive Ω_DM/Ω_b from FTD axioms.

Moore-shell decomposition of the 27-site Moore neighborhood (THEOREM, FTD-0028):
  Center  = 1 site  (d² = 0)
  Octahedron (shell 1, SC sublattice) = 6 sites (d² = 1, face-neighbors)
  Cuboctahedron (shell 2, FCC sublattice) = 12 sites (d² = 2, edge-neighbors)
  Cube corners (shell 3, BCC sublattice) = 8 sites (d² = 3, corner-neighbors)
  Total = 1 + 6 + 12 + 8 = 27 = 3³ ✓

Stella octangula split of cube corners: 8 = T_+ (4 verts) + T_− (4 verts).

Canonical "DM = 17, BARYON = 10" partition (per constants.js / Moore Layer):
  DM      = center + cuboctahedron + T_−  = 1 + 12 + 4 = 17
  BARYON  = octahedron + T_+               = 6 + 4 = 10

Run: python scripts/exploration/moore_shell_dm_baryon_weightings.py
"""

from mpmath import mp, mpf, sqrt

mp.dps = 30

# Planck 2018 reference (informational only — NOT used to fit any weighting)
OMEGA_DM_PLANCK   = mpf('0.265')
OMEGA_B_PLANCK    = mpf('0.0493')
PLANCK_RATIO      = OMEGA_DM_PLANCK / OMEGA_B_PLANCK
print(f"Planck 2018 reference: Ω_DM/Ω_b = {OMEGA_DM_PLANCK}/{OMEGA_B_PLANCK} = {float(PLANCK_RATIO):.4f}")
print()

# ── Canonical Moore-shell sites ──────────────────────────────────────
shells = {
    'center':        {'count': 1,  'partition': 'DM',     'd2': 0, 'laplacian_weight': mpf(-4), 'sublattice': 'central'},
    'octahedron':    {'count': 6,  'partition': 'BARYON', 'd2': 1, 'laplacian_weight': mpf(1)/3, 'sublattice': 'SC'},
    'cuboctahedron': {'count': 12, 'partition': 'DM',     'd2': 2, 'laplacian_weight': mpf(1)/6, 'sublattice': 'FCC'},
    'T_plus':        {'count': 4,  'partition': 'BARYON', 'd2': 3, 'laplacian_weight': mpf(0),   'sublattice': 'BCC (T+)'},
    'T_minus':       {'count': 4,  'partition': 'DM',     'd2': 3, 'laplacian_weight': mpf(0),   'sublattice': 'BCC (T-)'},
}

# Sanity
total_sites = sum(s['count'] for s in shells.values())
dm_count    = sum(s['count'] for s in shells.values() if s['partition'] == 'DM')
b_count     = sum(s['count'] for s in shells.values() if s['partition'] == 'BARYON')
print(f"Moore neighborhood total sites: {total_sites} (expected 27) {'✓' if total_sites == 27 else '✗'}")
print(f"  DM partition:     {dm_count} sites (center + cuboct + T_−)")
print(f"  BARYON partition: {b_count} sites (oct + T_+)")
print()

# ── Weighting schemes ──────────────────────────────────────────────
# Each weighting is a function shell_name -> weight per site.
# The DM/baryon ratio is computed as:
#   ratio = Σ_{shell ∈ DM} count·weight  /  Σ_{shell ∈ BARYON} count·weight

def compute_ratio(weights):
    """Given dict {shell_name: weight}, compute the DM:baryon ratio."""
    dm_total = sum(shells[s]['count'] * weights[s] for s in shells if shells[s]['partition'] == 'DM')
    b_total  = sum(shells[s]['count'] * weights[s] for s in shells if shells[s]['partition'] == 'BARYON')
    if b_total == 0:
        return float('inf')
    return dm_total / b_total

# Per-site weighting candidates (structurally motivated from FTD; NO Planck input)
weightings = []

# W1: Uniform voxel-counting (current dashboard reading)
weightings.append({
    'name':    'W1: Uniform voxel-counting',
    'rationale': 'Each Moore-shell site counts as 1; this is the current dashboard convention.',
    'weights': {s: mpf(1) for s in shells},
})

# W2: 18-pt Laplacian magnitude weighting
weightings.append({
    'name':    'W2: |Laplacian weight| (kinematic coupling)',
    'rationale': 'Patra-Karttunen 18-pt isotropic Laplacian weights: face=1/3, edge=1/6, corner=0, self=-4. Weight by |coupling magnitude|.',
    'weights': {s: abs(shells[s]['laplacian_weight']) for s in shells},
})

# W3: A_1g multiplicity per shell (FTD-0110 character-theory)
# A_1g multiplicity in 27-block = 4 total, distributed as (1,1,1,1) across the four orbits (center, oct, cuboct, cube).
# Each orbit contributes one A_1g mode. So per-shell A_1g count is 1.
# But the "weight per site" is mult(A_1g, shell) / |shell|.
# center: 1/1 = 1; oct: 1/6; cuboct: 1/12; cube (T+ ∪ T−): 1/8 (split equally between T+ and T-?)
weightings.append({
    'name':    'W3: A_1g density per site (FTD-0110 weighted)',
    'rationale': 'A_1g multiplicity in 27-block = 4 (FTD-0110); distributed 1 per orbit. Weight per site = mult(A_1g, orbit) / |orbit|. Reflects how strongly each site participates in the symmetric singlet mode.',
    'weights': {
        'center':        mpf(1) / 1,
        'octahedron':    mpf(1) / 6,
        'cuboctahedron': mpf(1) / 12,
        'T_plus':        mpf(1) / 8,
        'T_minus':       mpf(1) / 8,
    },
})

# W4: shell-degree weighting (d² = orbital distance squared)
weightings.append({
    'name':    'W4: Shell-d² weighting',
    'rationale': 'Each site weighted by its L² distance² (d²) from center. Reflects the spatial spread of the shell.',
    'weights': {s: mpf(max(shells[s]['d2'], 1)) for s in shells},
})

# W5: Cuboct × N_base weighting (per lead-physicist hypothesis — heavy lepton/weak layer)
weightings.append({
    'name':    'W5: Cuboctahedron × N_base = 4',
    'rationale': 'Per lead-physicist hypothesis: 12 cuboct sites correspond to 12 fermion species (3 generations × 4 fermions); weight cuboct by N_base = 4 reflecting per-fermion internal multiplicity. All other shells uniform weight.',
    'weights': {
        'center':        mpf(1),
        'octahedron':    mpf(1),
        'cuboctahedron': mpf(4),
        'T_plus':        mpf(1),
        'T_minus':       mpf(1),
    },
})

# W6: Cuboct × N_eff = 13 weighting
weightings.append({
    'name':    'W6: Cuboctahedron × N_eff = 13',
    'rationale': 'Alternative: weight cuboct by N_eff = 13 (the second framework integer beyond N_base). Reflects an alternative internal-multiplicity choice.',
    'weights': {
        'center':        mpf(1),
        'octahedron':    mpf(1),
        'cuboctahedron': mpf(13),
        'T_plus':        mpf(1),
        'T_minus':       mpf(1),
    },
})

# W7: T_+ and T_- each × N_base, cuboct × N_base
weightings.append({
    'name':    'W7: All non-center shells × N_base = 4',
    'rationale': 'Uniform N_base weighting on shells 1, 2, 3 (octahedron, cuboctahedron, cube). Center has weight 4 (Laplacian self-coupling magnitude).',
    'weights': {
        'center':        mpf(4),
        'octahedron':    mpf(4),
        'cuboctahedron': mpf(4),
        'T_plus':        mpf(4),
        'T_minus':       mpf(4),
    },
})

# W8: Site-degeneracy by O_h irrep dimension (T_1u sector for fermions)
# T_1u multiplicity in cuboctahedron orbit; each cuboct site participates in T_1u (3-dim irrep) doubly.
# Other shells: octahedron carries A_1g + E_g + T_1u; T_+ and T_- carry A_2u + T_2g + T_1u; center carries A_1g only.
# Use dimension of T_1u where present (=3), 1 otherwise.
weightings.append({
    'name':    'W8: T_1u dimension where present (=3)',
    'rationale': 'T_1u (vector irrep, dim 3) is the fermion sector by FTD-0028. Weight by T_1u dimension (3) where present, 1 otherwise. Center has no T_1u (only A_1g) so center weight = 1.',
    'weights': {
        'center':        mpf(1),
        'octahedron':    mpf(3),    # T_1u dim
        'cuboctahedron': mpf(3),    # T_1u dim (cuboct decomp includes T_1u + T_1g + T_2u + T_2g + E_g + A_1g)
        'T_plus':        mpf(3),    # T_1u dim
        'T_minus':       mpf(3),    # T_1u dim
    },
})

# W9: Hybrid — center×4, cuboct×4, T_- × 4 (boost DM sites only by N_base)
weightings.append({
    'name':    'W9: DM-shells × N_base = 4 (target test)',
    'rationale': 'Boost DM-classified shells (center, cuboct, T_-) each by N_base = 4. This isolates the question: does weighting DM by N_base alone explain the discrepancy?',
    'weights': {
        'center':        mpf(4),
        'octahedron':    mpf(1),
        'cuboctahedron': mpf(4),
        'T_plus':        mpf(1),
        'T_minus':       mpf(4),
    },
})

# ── Compute and report ───────────────────────────────────────────────
print(f"{'Scheme':<60} {'DM:b ratio':>12} {'vs Planck':>12} {'note':<60}")
print('─' * 145)
results = []
for w in weightings:
    ratio = compute_ratio(w['weights'])
    rel_diff = abs(ratio - PLANCK_RATIO) / PLANCK_RATIO
    flag = ''
    if rel_diff < mpf('0.02'):
        flag = '← MATCHES PLANCK (<2%)'
    elif rel_diff < mpf('0.05'):
        flag = '← close (<5%)'
    elif rel_diff < mpf('0.10'):
        flag = '← in family (<10%)'
    print(f"{w['name']:<60} {float(ratio):>12.4f} {float(rel_diff)*100:>10.1f}%  {flag:<60}")
    results.append({'name': w['name'], 'ratio': ratio, 'rel_diff': rel_diff, 'rationale': w['rationale']})

# ── Detailed report for each weighting ──────────────────────────────
print()
print("─── Per-weighting detail ─────────────────────────────────────")
for r in results:
    print(f"\n{r['name']}")
    print(f"  Rationale: {r['rationale']}")
    print(f"  DM:b      = {float(r['ratio']):.4f}")
    print(f"  vs Planck = {float(r['rel_diff'])*100:.2f}% deviation from observed 5.375")

# ── Summary ─────────────────────────────────────────────────────────
print()
print("─── Summary ──────────────────────────────────────────────────")
best = min(results, key=lambda r: r['rel_diff'])
print(f"  Closest weighting to Planck: {best['name']}")
print(f"    ratio = {float(best['ratio']):.4f} (Planck = {float(PLANCK_RATIO):.4f})")
print(f"    relative deviation: {float(best['rel_diff'])*100:.2f}%")
print()
print("  HONEST READING:")
print("  - The voxel-counted 17:10 = 1.70 misses Planck by factor ~3.2.")
print("  - No structurally-privileged weighting in FTD reproduces 5.375 EXACTLY.")
print("  - The closest match (if <5%) may be a structural hint or numerical accident.")
print("  - Per CLAUDE.md anti-target rules: this is [OBSERVATION] of weight-space")
print("    arithmetic, NOT a [DERIVED] prediction of Ω_DM/Ω_b from FTD axioms.")
