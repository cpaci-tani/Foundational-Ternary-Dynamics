"""
Proof - Lattice Bianchi identities (subsidiary of Phase G; FTD-0114)
======================================================================

CLAIM [DERIVED]: On the vertex-centered cubic lattice with centered
differences (d_i f)(x) = [f(x + e_i) - f(x - e_i)] / 2, the discrete
exterior derivative satisfies d^2 = 0 identically. Equivalently:

    div(curl(A)) == 0    exactly at every lattice site
    curl(grad(phi)) == 0 exactly at every lattice site

independent of the choice of Laplacian stencil (G6/G18/G26).

This script verifies both identities numerically at L = 8 with random
vector and scalar fields, asserting the residual is at machine precision
(< 1e-12).

Provenance: docs/theory/03_derivations/DERIV_LATTICE_HODGE_DUALITY.md
LEDGER: FTD-0114 (subsidiary of FTD-0004 Phase G, parallel to FTD-0113).

Usage:
    python scripts/proofs/proof_lattice_hodge_duality.py
"""

import sys
import random


L = 8                # lattice side
SEED = 42            # for reproducibility
TOL = 1.0e-12        # machine-precision threshold


# ---------------------------------------------------------------------------
# Lattice utilities (periodic boundary conditions)
# ---------------------------------------------------------------------------

def wrap(i):
    return i % L


def random_scalar_field(rng):
    """Returns a list of L*L*L floats indexed by linearized (x,y,z)."""
    return [rng.uniform(-1.0, 1.0) for _ in range(L * L * L)]


def random_vector_field(rng):
    """Returns three scalar fields A_x, A_y, A_z."""
    return [random_scalar_field(rng) for _ in range(3)]


def idx(x, y, z):
    return wrap(x) * L * L + wrap(y) * L + wrap(z)


# ---------------------------------------------------------------------------
# Centered differences
# ---------------------------------------------------------------------------

def diff_x(f, x, y, z):
    return 0.5 * (f[idx(x + 1, y, z)] - f[idx(x - 1, y, z)])


def diff_y(f, x, y, z):
    return 0.5 * (f[idx(x, y + 1, z)] - f[idx(x, y - 1, z)])


def diff_z(f, x, y, z):
    return 0.5 * (f[idx(x, y, z + 1)] - f[idx(x, y, z - 1)])


def grad(phi):
    """Returns (G_x, G_y, G_z) where G_i = d_i phi at every site."""
    Gx = [0.0] * (L ** 3)
    Gy = [0.0] * (L ** 3)
    Gz = [0.0] * (L ** 3)
    for x in range(L):
        for y in range(L):
            for z in range(L):
                Gx[idx(x, y, z)] = diff_x(phi, x, y, z)
                Gy[idx(x, y, z)] = diff_y(phi, x, y, z)
                Gz[idx(x, y, z)] = diff_z(phi, x, y, z)
    return Gx, Gy, Gz


def curl(A):
    """Returns (B_x, B_y, B_z) where B = curl(A) at every site,
    using centered differences on each component.
    """
    Ax, Ay, Az = A
    Bx = [0.0] * (L ** 3)
    By = [0.0] * (L ** 3)
    Bz = [0.0] * (L ** 3)
    for x in range(L):
        for y in range(L):
            for z in range(L):
                # B_x = d_y A_z - d_z A_y
                Bx[idx(x, y, z)] = diff_y(Az, x, y, z) - diff_z(Ay, x, y, z)
                # B_y = d_z A_x - d_x A_z
                By[idx(x, y, z)] = diff_z(Ax, x, y, z) - diff_x(Az, x, y, z)
                # B_z = d_x A_y - d_y A_x
                Bz[idx(x, y, z)] = diff_x(Ay, x, y, z) - diff_y(Ax, x, y, z)
    return Bx, By, Bz


def div(B):
    """Returns rho_check = div(B) at every site."""
    Bx, By, Bz = B
    rho = [0.0] * (L ** 3)
    for x in range(L):
        for y in range(L):
            for z in range(L):
                rho[idx(x, y, z)] = (
                    diff_x(Bx, x, y, z)
                    + diff_y(By, x, y, z)
                    + diff_z(Bz, x, y, z)
                )
    return rho


def linf(field):
    return max(abs(v) for v in field)


def linf_vec(V):
    return max(linf(c) for c in V)


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

def test_div_curl_zero(rng):
    """Bianchi I: div(curl(A)) = 0 for any A."""
    A = random_vector_field(rng)
    B = curl(A)
    rho = div(B)
    return linf(rho), linf_vec(A), linf_vec(B)


def test_curl_grad_zero(rng):
    """Bianchi II: curl(grad(phi)) = 0 for any phi."""
    phi = random_scalar_field(rng)
    G = grad(phi)
    C = curl(G)
    return linf_vec(C), linf(phi), linf_vec(G)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def run():
    print("=" * 72)
    print("PROOF: Lattice Bianchi identities (FTD-0114)")
    print("Subsidiary of FTD-0004 (Phase G); parallel to FTD-0113")
    print("=" * 72)
    print(f"L = {L},  centered differences,  random fields (seed={SEED})")
    print(f"Tolerance for machine-precision pass: < {TOL}")
    print()

    rng = random.Random(SEED)

    print("(1) Bianchi I:  div(curl(A)) == 0")
    div_curl_inf, A_inf, B_inf = test_div_curl_zero(rng)
    pass1 = div_curl_inf < TOL
    print(f"      ||A||_inf            = {A_inf:.6e}")
    print(f"      ||curl(A)||_inf      = {B_inf:.6e}")
    print(f"      ||div(curl(A))||_inf = {div_curl_inf:.6e}    [{'PASS' if pass1 else 'FAIL'}]")
    print()

    print("(2) Bianchi II: curl(grad(phi)) == 0")
    curl_grad_inf, phi_inf, G_inf = test_curl_grad_zero(rng)
    pass2 = curl_grad_inf < TOL
    print(f"      ||phi||_inf             = {phi_inf:.6e}")
    print(f"      ||grad(phi)||_inf       = {G_inf:.6e}")
    print(f"      ||curl(grad(phi))||_inf = {curl_grad_inf:.6e}    [{'PASS' if pass2 else 'FAIL'}]")
    print()

    print("=" * 72)
    print(f"PASS 1 (Bianchi I,  div curl = 0): {'PASS' if pass1 else 'FAIL'}")
    print(f"PASS 2 (Bianchi II, curl grad = 0): {'PASS' if pass2 else 'FAIL'}")
    print("=" * 72)

    if not (pass1 and pass2):
        sys.exit(1)


if __name__ == "__main__":
    run()
