"""
explore_color_excess.py — High-precision PSLQ search for the Color Excess delta_c.

Pre-registered campaign ID: FTD-0224.
This script uses mpmath to calculate the Color Excess (both tree-level and precision-corrected)
to 100 decimal digits and performs a PSLQ integer relation search over multiple transcendental
and modular period bases.
"""

import sys
import os
import json
import mpmath

# Set high-precision arithmetic to 100 decimal digits
mpmath.mp.dps = 100

def compute_constants():
    """Compute high-precision FTD constants."""
    # seeds
    pi = mpmath.pi
    e = mpmath.exp(1)
    euler = mpmath.euler
    catalan = mpmath.catalan

    # lemniscate G*
    gamma_quarter = mpmath.gamma(0.25)
    gamma_half = mpmath.gamma(0.5)
    g_star = gamma_quarter**2 / (mpmath.sqrt(2) * gamma_half**2)

    # master quadratic roots
    b = -16 * g_star**2
    c_coeff = 16 * g_star**3
    discriminant = b**2 - 4 * c_coeff
    x_plus = (-b + mpmath.sqrt(discriminant)) / 2
    x_minus = (-b - mpmath.sqrt(discriminant)) / 2

    # CFT anomaly correction
    eps = mpmath.fabs(mpmath.exp(pi) - pi - 20)
    c1 = mpmath.mpf(9) / 47
    c2 = mpmath.mpf(5) / 64
    c3 = mpmath.mpf(4) / 141
    c4 = mpmath.mpf(141) / 11

    # precision roots
    x_plus_precision = x_plus - c1*eps + c2*eps**2 - c3*eps**3 - c4*eps**4
    alpha = 1 / x_plus_precision
    x_minus_precision = 16 * g_star**3 / x_plus_precision

    # color excess
    delta_c_tree = x_minus - 3
    delta_c_precision = x_minus_precision - 3

    # Barnes G quarter-integer relation values
    # Barnes G(1/4)/G(3/4) relation:
    # log G(1/4) - log G(3/4) = -0.5 log G* - 0.125 log 2 - 0.25 log pi - G_Catalan / (2pi)

    return {
        "pi": pi,
        "e": e,
        "euler": euler,
        "catalan": catalan,
        "g_star": g_star,
        "x_plus": x_plus,
        "x_minus": x_minus,
        "x_plus_precision": x_plus_precision,
        "alpha": alpha,
        "x_minus_precision": x_minus_precision,
        "delta_c_tree": delta_c_tree,
        "delta_c_precision": delta_c_precision
    }

def run_pslq_search(target_val, target_name, basis_dict, max_coeff=10000000):
    """Run PSLQ search over a given basis for a target value."""
    basis_keys = list(basis_dict.keys())
    basis_vals = [basis_dict[k] for k in basis_keys]

    # Include the target value itself with coefficient -1 (or similar)
    # We want to find integers q_i such that sum q_i * basis_i + q_target * target_val = 0
    full_basis = basis_vals + [target_val]

    print(f"\n--- Running PSLQ for {target_name} ---")
    print(f"Basis elements: {basis_keys}")

    try:
        relation = mpmath.pslq(full_basis, tol=1e-85, maxcoeff=max_coeff)
        if relation:
            target_coeff = relation[-1]
            basis_coeffs = relation[:-1]

            # Reconstruct the relation
            terms = []
            for k, coeff in zip(basis_keys, basis_coeffs):
                if coeff != 0:
                    terms.append(f"{int(coeff)} * {k}")

            terms_str = " + ".join(terms)
            print(f"  [FOUND] Integer relation found!")
            print(f"  Relation: {terms_str} + ({int(target_coeff)}) * {target_name} = 0")

            # Compute residual
            lhs = mpmath.mpf(0)
            for val, coeff in zip(basis_vals, basis_coeffs):
                lhs += coeff * val
            lhs += target_coeff * target_val
            print(f"  Residual: {mpmath.nstr(lhs, 6)}")
            return {
                "success": True,
                "relation": [int(x) for x in relation],
                "basis": basis_keys,
                "residual": str(lhs)
            }
        else:
            print("  [NULL] No integer relation found within tolerance.")
            return {"success": False}
    except Exception as e:
        print(f"  [ERROR] PSLQ search failed: {e}")
        return {"success": False, "error": str(e)}

def main():
    consts = compute_constants()

    print(f"Tree-level delta_c:      {mpmath.nstr(consts['delta_c_tree'], 60)}")
    print(f"Precision-level delta_c: {mpmath.nstr(consts['delta_c_precision'], 60)}")

    # Build standard baskets
    baskets = {
        "basket_rational_Gstar": {
            "1": mpmath.mpf(1),
            "g_star": consts["g_star"],
            "g_star^2": consts["g_star"]**2,
            "g_star^3": consts["g_star"]**3,
        },
        "basket_transcendental_standard": {
            "1": mpmath.mpf(1),
            "pi": consts["pi"],
            "pi^2": consts["pi"]**2,
            "e": consts["e"],
            "euler": consts["euler"],
            "catalan": consts["catalan"],
        },
        "basket_mixed_ftd": {
            "1": mpmath.mpf(1),
            "alpha": consts["alpha"],
            "g_star": consts["g_star"],
            "g_star^2": consts["g_star"]**2,
            "pi": consts["pi"],
            "log(2)": mpmath.log(2),
            "log(pi)": mpmath.log(consts["pi"]),
        },
        "basket_hadronic_excess": {
            "1": mpmath.mpf(1),
            "alpha": consts["alpha"],
            "pi*alpha": consts["pi"] * consts["alpha"],
            "alpha_s": mpmath.mpf(7) / 59,
            "alpha_s/pi": (mpmath.mpf(7) / 59) / consts["pi"],
        }
    }

    results = {}

    for target in ["delta_c_tree", "delta_c_precision"]:
        target_val = consts[target]
        results[target] = {}
        for basket_name, basket_dict in baskets.items():
            res = run_pslq_search(target_val, target, basket_dict)
            results[target][basket_name] = res

    # Save the output to outputs folder
    outputs_dir = os.path.join(os.path.dirname(__file__), '..', '..', 'output')
    os.makedirs(outputs_dir, exist_ok=True)
    out_file = os.path.join(outputs_dir, 'color_excess_pslq_results.json')

    with open(out_file, 'w') as f:
        json.dump(results, f, indent=2)

    print(f"\nResults saved to {out_file}")

if __name__ == '__main__':
    main()
