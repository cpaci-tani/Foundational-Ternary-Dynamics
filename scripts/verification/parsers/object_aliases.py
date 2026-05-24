"""
Canonical-name reconciliation for mathematical objects.

Identifiers extracted by E1/E2/E3 across heterogeneous scripts use varied
naming conventions for the same underlying object (G*, Gstar, G_STAR,
GSTAR all denote the lemniscatic constant Γ(1/4)/Γ(3/4)).  This table
maps those variants to a single canonical name so the node map shows one
node per concept, not many.

The kind field separates constants (numerical values), functions (mappings),
and operators (algebraic symbols / states).  The canonical_label is what
the renderers display.
"""
from __future__ import annotations

# (canonical_id, kind, label_for_display, aliases)
OBJECTS = [
    ("Gstar",        "constant", "G*",                  ["Gstar", "G_STAR", "GSTAR", "GSTAR_LEM", "G_star", "Gs"]),
    ("varpi",        "constant", "ϖ",                   ["varpi", "VARPI", "ϖ", "VARPI_LEM"]),
    ("G_G",          "constant", "G_G",                 ["G_G", "GAUSS_M", "G_Gauss", "Gauss_M", "M_AGM", "GAUSS_CONST"]),
    ("G_rho",        "constant", "G_ρ",                 ["G_rho", "GRHO", "G_RHO", "G_eq", "Gauss_rho"]),
    ("pi",           "constant", "π",                   ["pi", "PI", "PI_ONTIC", "PI_STD", "math.pi"]),
    ("e",            "constant", "e",                   ["e", "E", "math.e"]),
    ("euler_gamma",  "constant", "γ",                   ["euler", "EULER_GAMMA", "euler_gamma", "gamma_euler"]),
    ("phi",          "constant", "φ",                   ["phi", "PHI", "golden", "GOLDEN_RATIO"]),
    ("alpha",        "constant", "α",                   ["alpha", "ALPHA", "ALPHA_INV", "ALPHA_FS"]),
    ("g_c",          "constant", "g_c",                 ["g_c", "G_C", "gc"]),
    ("x_plus",       "constant", "x₊",                  ["x_plus", "x_pos", "X_PLUS", "xplus", "x_p"]),
    ("x_minus",      "constant", "x₋",                  ["x_minus", "x_neg", "X_MINUS", "xminus", "x_m"]),
    ("N_c",          "constant", "N_c",                 ["N_c", "N_C", "Nc"]),
    ("N_base",       "constant", "N_base",              ["N_base", "N_BASE", "Nbase"]),
    ("N_eff",        "constant", "N_eff",               ["N_eff", "N_EFF", "Neff"]),
    ("b_3",          "constant", "b₃",                  ["b_3", "B_3", "b3"]),
    ("D",            "constant", "D=3",                 ["D", "D_SPATIAL", "D_DIM"]),
    ("Coefficient_16","constant","16=|Aut(E)|²",        ["Coefficient_16", "COEFFICIENT", "aut_E", "|Aut(E)|^2"]),
    ("Gamma_quarter","constant", "Γ(1/4)",              ["GAMMA_QUARTER", "gamma_quarter", "Gamma1_4"]),
    ("Gamma_three_quarter","constant","Γ(3/4)",         ["GAMMA_THREE_QUARTER", "gamma_three_quarter", "Gamma3_4"]),
    ("Gamma_one_third","constant","Γ(1/3)",             ["GAMMA_THIRD", "gamma_third", "Gamma1_3"]),
    ("Gamma_one_sixth","constant","Γ(1/6)",             ["GAMMA_SIXTH", "Gamma1_6"]),
    ("Gamma_one_fifth","constant","Γ(1/5)",             ["Gamma1_5", "GAMMA_FIFTH"]),
    ("Catalan",      "constant", "G_Catalan",           ["G_Catalan", "Catalan", "CATALAN", "G_cat"]),
    ("W3",           "constant", "W^(3)_BCC",           ["W3", "W3_BCC", "W_3"]),
    ("W4",           "constant", "W^(4)_BCC",           ["W4", "W4_BCC", "W_4", "W4_raw", "W4_watson"]),
    ("W5",           "constant", "W^(5)_BCC",           ["W5", "W5_BCC", "W_5", "W5_raw"]),
    ("eta_i",        "constant", "η(i)",                ["eta_i"]),
    ("eta_2i",       "constant", "η(2i)",               ["eta_2i"]),
    ("eta_i_half",   "constant", "η(i/2)",              ["eta_i_half"]),
    ("eta_rho",      "constant", "η(ρ)",                ["eta_rho"]),
    ("theta2_i",     "constant", "θ₂(i)",               ["theta2_i", "theta2"]),
    ("theta3_i",     "constant", "θ₃(i)",               ["theta3_i", "theta3"]),
    ("theta4_i",     "constant", "θ₄(i)",               ["theta4_i", "theta4"]),
    ("E4_i",         "constant", "E₄(i)",               ["E4_i"]),
    ("E6_i",         "constant", "E₆(i)",               ["E6_i"]),
    ("E8_i",         "constant", "E₈(i)",               ["E8_i"]),
    ("E10_i",        "constant", "E₁₀(i)",              ["E10_i"]),
    ("E12_i",        "constant", "E₁₂(i)",              ["E12_i", "E12_i_basis"]),
    ("E14_i",        "constant", "E₁₄(i)",              ["E14_i"]),
    ("E16_i",        "constant", "E₁₆(i)",              ["E16_i"]),
    ("E20_i",        "constant", "E₂₀(i)",              ["E20_i"]),
    ("E24_i",        "constant", "E₂₄(i)",              ["E24_i"]),
    ("E2_i",         "constant", "E₂(i)",               ["E2_i"]),
    ("E2_rho",       "constant", "E₂(ρ)",               ["E2_rho"]),
    ("E4_rho",       "constant", "E₄(ρ)",               ["E4_rho"]),
    ("E6_rho",       "constant", "E₆(ρ)",               ["E6_rho"]),
    ("E12_rho",      "constant", "E₁₂(ρ)",              ["E12_rho"]),
    ("Delta_i",      "constant", "Δ(i)",                ["Delta_i"]),
    ("Delta_rho",    "constant", "Δ(ρ)",                ["Delta_rho"]),
    ("j_i",          "constant", "j(i)=1728",           ["j_i", "j_tau_i", "j_invariant_i"]),
    ("K_half",       "constant", "K(1/√2)",             ["K_half", "ellipK_half"]),
    ("E_half",       "constant", "E(1/√2)",             ["E_half", "ellipE_half"]),
    ("omega_E",      "constant", "ω_E",                 ["omega_E", "omega_E_int", "omega_E_gamma"]),
    ("omega_rho",    "constant", "ω_ρ",                 ["omega_rho", "omega_rho_direct", "omega_rho_reduced"]),
    ("B_quarter",    "constant", "B(1/4,1/4)",          ["B_quarter"]),
    ("M3_at",        "constant", "M_3(1,2^{-1/3})",     ["M3_at", "M3", "cubic_AGM"]),
    ("k_B",          "constant", "K_B",                 ["k_B", "K_B", "K_b", "Kb"]),
    ("ell_P",        "constant", "ℓ_P",                 ["ell_P", "l_P", "ELL_P", "L_P", "planck_length"]),
    ("m_e",          "constant", "m_e",                 ["m_e", "M_E", "Me"]),
    ("m_mu",         "constant", "m_μ",                 ["m_mu", "M_MU", "Mmu", "m_muon"]),
    ("m_tau",        "constant", "m_τ",                 ["m_tau", "M_TAU", "Mtau"]),
    ("m_p",          "constant", "m_p",                 ["m_p", "M_P", "Mp", "m_proton"]),
    ("hbar",         "constant", "ℏ",                   ["hbar", "HBAR", "hBar"]),
    ("c_speed",      "constant", "c",                   ["c", "C_SPEED", "c_speed"]),
    ("G_N",          "constant", "G_N",                 ["G_N", "GN", "newton_G"]),
    # Functions / operators
    ("gamma_fn",     "function", "Γ(·)",                ["gamma"]),
    ("agm",          "function", "AGM",                 ["agm", "AGM"]),
    ("sqrt",         "function", "√",                   ["sqrt"]),
    ("ellipk",       "function", "K(·)",                ["ellipk", "K_complete"]),
    ("ellipe",       "function", "E(·)",                ["ellipe", "E_complete"]),
    ("chi_neg4",     "function", "χ_{-4}",              ["chi_imag", "chi_neg4", "chi_minus_4"]),
    ("zeta",         "function", "ζ(·)",                ["zeta", "zeta_fn"]),
]


def build_alias_map() -> dict[str, str]:
    """Return {alias_name -> canonical_id} for fast lookup."""
    aliases: dict[str, str] = {}
    for canon, _kind, _label, alist in OBJECTS:
        aliases[canon] = canon
        for a in alist:
            aliases[a] = canon
    return aliases


def canonicalise(name: str) -> str:
    """Map a raw identifier name to its canonical id (or itself if not aliased)."""
    return _ALIASES.get(name, name)


def canonical_object_records() -> list[dict]:
    """List of canonical object records for direct insertion into layers.objects."""
    return [
        {"id": canon, "name": label, "kind": kind, "valence": 0, "sector": None}
        for canon, kind, label, _ in OBJECTS
    ]


_ALIASES = build_alias_map()
