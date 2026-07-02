"""proof_gnc_rigidity.py — FTD-0354 verification.

Numerically verifies every finitely-checkable claim of
docs/theory/03_derivations/foundational_mechanics/LEMMA_GNC_RIGIDITY.md
(the GNC pointwise rigidity lemma, extension E1 of
ASSESSMENT_MATH_GRADES_AND_EXTENSIONS_2026-07-01.md, grounded in FTD-0349).

What is verified (exactly this, nothing more):

R1  Rigidity lemma, three-way equivalence at a site (3x3 matrix algebra):
      (i)  |D vhat| = K_B for all unit vhat            (GNC-s at the site)
     (ii)  D^T D = K_B^2 I
    (iii)  D = K_B Q with Q in O(3)
    including the polarization mechanism of (i)=>(ii), the singular-value
    restatement (all three singular values equal K_B), and the negation
    (non-orthogonal D admits a unit direction violating (i)).
R2  Divergence identity for affine lattice fields J = A x + c: the forward
    (and backward) lattice divergence equals tr A at every interior site;
    for A = K_B Q charge-freedom forces tr Q = 0.
R3  Classification, proper stratum: tr R(axis, theta) = 1 + 2 cos(theta),
    strictly decreasing on [0, pi]; tr = 0 iff theta = 2*pi/3 (unique);
    every trace-zero proper-orthogonal matrix is R(n, 2*pi/3) for some axis
    (reconstruct-and-match); the cyclic permutation P_cyc is exactly
    R((1,1,1)/sqrt(3), 2*pi/3); lattice texture J = K_B R x is GNC-s +
    divergence-free at every interior site (recovers FTD-0349 S7).
R4  Classification, improper stratum: every improper Q in O(3) is -R with
    R in SO(3); tr Q = 0 iff R has angle 2*pi/3; eigenvalues of trace-zero
    improper Q are {-1, e^{+i pi/3}, e^{-i pi/3}}; the concrete example
    -P_cyc gives a GNC-s + divergence-free lattice texture whose
    Born-Infeld cores resum to -N K_B sqrt(1 - V^2) exactly (the
    resummation is det-blind: orthogonality is all it uses).
R5  GNC-w (summed) is strictly weaker than GNC-s (pointwise): an explicit
    two-site configuration whose Gram matrices sum to 2 K_B^2 I while
    neither site is K_B^2 I.
R6  The affine classification is NOT exhaustive on the lattice: explicit
    fold fields.  (a) the coordinate fold J = K_B (f(x1), x2, x3) with f a
    +-1-increment walk is GNC-s everywhere but never divergence-free;
    (b) the trace-zero fold J = K_B P_cyc (|x1 - x0|, x2, x3) is GNC-s AND
    divergence-free at every interior site (both difference conventions)
    yet non-affine — its Jacobian takes exactly the two orthogonal values
    K_B P_cyc and K_B P_cyc sigma1.
R7  The key algebraic step of the continuum C^2 rigidity proof as finite
    linear algebra: the tensor space { A_{ij,k} : A_{ij,k} = A_{ji,k},
    A_{ij,k} + A_{ik,j} = 0 } is exactly {0} (27x27 rank computation).
R8  The d = 1 instance of the lemma is the clock-hypothesis shape:
    |a| = K_B  <=>  a^2 = K_B^2  <=>  a/K_B in O(1) = {+1, -1}.

Anti-target note: every check below evaluates an identity stated in the
lemma document; no scans, no near-miss searches, no fitting to targets.
No epistemic tag is moved by this script: FTD-0110/0250 unchanged, the
clock hypothesis stays [AXIOM] (FTD-0208), GNC stays un-forced, and the
engine question stays the pre-registered FTD-0349 S9 Q_ij measurement.

Run:  python scripts/proofs/proof_gnc_rigidity.py
"""

import os
import sys

import numpy as np

# ---------------------------------------------------------------------------
# canonical constants (single source of truth: scripts/constants.py)
# ---------------------------------------------------------------------------
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
try:
    from constants import K_B  # type: ignore
except Exception:  # standalone fallback (value per scripts/constants.py)
    K_B = 0.511

RNG = np.random.default_rng(20260702)

_results = []


def check(name, ok, detail=""):
    _results.append((name, bool(ok)))
    status = "PASS" if ok else "FAIL"
    print(f"  [{status}] {name}" + (f"   ({detail})" if detail else ""))


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------
def unit(v):
    v = np.asarray(v, dtype=float)
    return v / np.linalg.norm(v)


def rodrigues(axis, theta):
    a = unit(axis)
    Kx = np.array([[0, -a[2], a[1]], [a[2], 0, -a[0]], [-a[1], a[0], 0]])
    return np.eye(3) + np.sin(theta) * Kx + (1 - np.cos(theta)) * (Kx @ Kx)


def random_so3():
    Q, _ = np.linalg.qr(RNG.standard_normal((3, 3)))
    if np.linalg.det(Q) < 0:
        Q[:, 0] *= -1
    return Q


def random_o3_improper():
    Q = random_so3()
    Q[:, 0] *= -1  # det -> -1
    return Q


P_CYC = np.array([[0.0, 0.0, 1.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0]])  # e1->e2->e3->e1
SIGMA1 = np.diag([-1.0, 1.0, 1.0])


def coords(L):
    """Integer coordinates, centered: shape (3, L, L, L)."""
    return np.indices((L, L, L)).astype(float) - (L - 1) / 2.0


def affine_field(A, c, L):
    """J_a(x) = A_{ab} x_b + c_a on an L^3 grid (NOT periodic — no roll)."""
    X = coords(L)
    return np.einsum("ab,bxyz->axyz", A, X) + np.asarray(c)[:, None, None, None]


def fwd_jacobian(J):
    """D[a, i] = J_a(x + e_i) - J_a(x) on the interior block [0, L-2]^3.

    Explicit slicing (no periodic wrap): valid for non-periodic fields.
    Returns shape (3, 3, L-1, L-1, L-1)."""
    L = J.shape[1]
    core = (slice(0, L - 1),) * 3
    D = np.empty((3, 3, L - 1, L - 1, L - 1))
    for a in range(3):
        for i in range(3):
            sl = [slice(0, L - 1)] * 3
            sl[i] = slice(1, L)
            D[a, i] = J[a][tuple(sl)] - J[a][core]
    return D


def gram_field(D):
    """G_ij(x) = sum_a D[a,i](x) D[a,j](x): shape (3, 3, ...)."""
    return np.einsum("aixyz,ajxyz->ijxyz", D, D)


def fwd_div(J):
    """Forward divergence sum_i [J_i(x+e_i) - J_i(x)] on the interior block."""
    D = fwd_jacobian(J)
    return D[0, 0] + D[1, 1] + D[2, 2]


def bwd_div(J):
    """Backward divergence sum_i [J_i(x) - J_i(x-e_i)] on the block [1, L-1]^3."""
    L = J.shape[1]
    core = (slice(1, L),) * 3
    out = np.zeros((L - 1, L - 1, L - 1))
    for i in range(3):
        sl = [slice(1, L)] * 3
        sl[i] = slice(0, L - 1)
        out += J[i][core] - J[i][tuple(sl)]
    return out


TOL = 1e-12


# ===========================================================================
print("=" * 76)
print("proof_gnc_rigidity.py — FTD-0354 (GNC pointwise rigidity lemma)")
print(f"K_B = {K_B}")
print("=" * 76)

# ---------------------------------------------------------------------------
# R1 — the rigidity lemma at a site (matrix algebra)
# ---------------------------------------------------------------------------
print("\nR1: GNC-s at a site  <=>  D^T D = K_B^2 I  <=>  D = K_B Q, Q in O(3)")

# (iii) => (i): scaled orthogonal matrices swing every unit direction by K_B
ok = True
for make in (random_so3, random_o3_improper):
    for _ in range(20):
        D = K_B * make()
        for _ in range(20):
            v = unit(RNG.standard_normal(3))
            ok &= abs(np.linalg.norm(D @ v) - K_B) < TOL
check("R1a: (iii)=>(i)  |K_B Q vhat| = K_B, proper AND improper Q (800 draws)", ok)

# (iii) <=> (ii): D^T D = K_B^2 I exactly for D = K_B Q; and Q := D/K_B is
# orthogonal whenever D^T D = K_B^2 I (Q^T Q = I is the same equation).
ok = True
for make in (random_so3, random_o3_improper):
    for _ in range(20):
        Q = make()
        D = K_B * Q
        ok &= np.allclose(D.T @ D, K_B**2 * np.eye(3), atol=TOL)
        ok &= np.allclose((D / K_B).T @ (D / K_B), np.eye(3), atol=TOL)
check("R1b: (ii)<=>(iii)  D^T D = K_B^2 I  iff  D/K_B orthogonal (40 draws)", ok)

# (i) => (ii) mechanism: reconstruct S = D^T D from unit-sphere values of the
# quadratic form by polarization; for orthogonal D the reconstruction is
# K_B^2 I; for a generic D it recovers D^T D (so the form determines S).
def polarize(qform):
    """Reconstruct symmetric S from q(V) = V^T S V via polarization."""
    E = np.eye(3)
    S = np.zeros((3, 3))
    for i in range(3):
        S[i, i] = qform(E[i])
        for j in range(i + 1, 3):
            S[i, j] = S[j, i] = 0.25 * (qform(E[i] + E[j]) - qform(E[i] - E[j]))
    return S


ok = True
for _ in range(10):
    D = RNG.standard_normal((3, 3))
    # q evaluated ONLY through unit vectors + homogeneity, as in the proof:
    q = lambda V: np.linalg.norm(V) ** 2 * np.linalg.norm(D @ unit(V)) ** 2
    ok &= np.allclose(polarize(q), D.T @ D, atol=1e-10)
Dorth = K_B * random_so3()
q = lambda V: np.linalg.norm(V) ** 2 * np.linalg.norm(Dorth @ unit(V)) ** 2
ok &= np.allclose(polarize(q), K_B**2 * np.eye(3), atol=1e-10)
check("R1c: (i)=>(ii)  polarization reconstructs S from unit-sphere data", ok)

# negation: a non-orthogonal D violates (i) in the top-Gram-eigenvector
# direction — the equivalence is not vacuous.
ok = True
for _ in range(20):
    D = K_B * (random_so3() + 0.3 * RNG.standard_normal((3, 3)))
    w, V = np.linalg.eigh(D.T @ D)
    dev = max(abs(np.linalg.norm(D @ V[:, 0]) - K_B), abs(np.linalg.norm(D @ V[:, 2]) - K_B))
    ok &= dev > 1e-4
check("R1d: negation — non-orthogonal D admits a unit vhat with |D vhat| != K_B", ok)

# singular-value restatement: GNC-s pins the ENTIRE singular spectrum at K_B
ok = True
for make in (random_so3, random_o3_improper):
    for _ in range(20):
        s = np.linalg.svd(K_B * make(), compute_uv=False)
        ok &= np.allclose(s, K_B, atol=TOL)
check("R1e: all three singular values of K_B Q equal K_B (spectrum pinned)", ok)

# ---------------------------------------------------------------------------
# R2 — divergence of affine lattice fields
# ---------------------------------------------------------------------------
print("\nR2: affine J = A x + c: lattice divergence = tr A at every interior site")

L = 9
ok_f, ok_b = True, True
for _ in range(8):
    A = RNG.standard_normal((3, 3))
    c = RNG.standard_normal(3)
    J = affine_field(A, c, L)
    ok_f &= np.allclose(fwd_div(J), np.trace(A), atol=TOL)
    ok_b &= np.allclose(bwd_div(J), np.trace(A), atol=TOL)
check("R2a: forward divergence == tr A (8 random affine fields)", ok_f)
check("R2b: backward (Gauss-stencil) divergence == tr A likewise", ok_b)
Q = rodrigues((1, 2, 3), 2 * np.pi / 3)
J = affine_field(K_B * Q, np.zeros(3), L)
check(
    "R2c: A = K_B Q  =>  div = K_B tr Q; charge-free forces tr Q = 0",
    np.allclose(fwd_div(J), K_B * np.trace(Q), atol=TOL) and abs(np.trace(Q)) < TOL,
)

# ---------------------------------------------------------------------------
# R3 — proper stratum: theta = 2*pi/3 is the unique rotation stratum
# ---------------------------------------------------------------------------
print("\nR3: {R in SO(3) : tr R = 0} is exactly the theta = 2*pi/3 conjugacy class")

ok = True
for _ in range(10):
    theta = RNG.uniform(0, np.pi)
    ok &= abs(np.trace(rodrigues(RNG.standard_normal(3), theta)) - (1 + 2 * np.cos(theta))) < TOL
check("R3a: tr R(n, theta) = 1 + 2 cos(theta) (10 random axes/angles)", ok)

th = np.linspace(0, np.pi, 100001)
tr = 1 + 2 * np.cos(th)
check(
    "R3b: 1 + 2 cos strictly decreasing on [0, pi]; unique zero at 2*pi/3",
    np.all(np.diff(tr) < 0) and abs(1 + 2 * np.cos(2 * np.pi / 3)) < 1e-15,
)

# every trace-zero proper-orthogonal matrix IS R(n, 2*pi/3): reconstruct-and-match
ok = True
R0 = rodrigues((0, 0, 1), 2 * np.pi / 3)
for _ in range(20):
    U = random_so3()
    Q = U @ R0 @ U.T  # generic trace-zero proper orthogonal
    ok &= abs(np.trace(Q)) < 1e-12 and abs(np.linalg.det(Q) - 1) < 1e-12
    ev, evec = np.linalg.eig(Q)
    k = np.argmin(np.abs(ev - 1.0))
    axis = np.real(evec[:, k])
    match = min(
        np.max(np.abs(rodrigues(axis, 2 * np.pi / 3) - Q)),
        np.max(np.abs(rodrigues(-axis, 2 * np.pi / 3) - Q)),
    )
    ok &= match < 1e-9
check("R3c: every trace-zero proper Q reconstructs as R(n, 2*pi/3) (20 draws)", ok)

check(
    "R3d: P_cyc == R((1,1,1)/sqrt3, 2*pi/3) exactly; tr 0; det +1",
    np.allclose(P_CYC, rodrigues((1, 1, 1), 2 * np.pi / 3), atol=TOL)
    and abs(np.trace(P_CYC)) < TOL
    and abs(np.linalg.det(P_CYC) - 1) < TOL,
)

# lattice texture J = K_B R x (FTD-0349 S7 recovery): GNC-s + div-free sitewise
ok = True
for _ in range(5):
    R = rodrigues(RNG.standard_normal(3), 2 * np.pi / 3)
    J = affine_field(K_B * R, RNG.standard_normal(3), L)
    G = gram_field(fwd_jacobian(J))
    tgt = K_B**2 * np.eye(3)[:, :, None, None, None]
    ok &= np.max(np.abs(G - tgt)) < 1e-10
    ok &= np.max(np.abs(fwd_div(J))) < 1e-10 and np.max(np.abs(bwd_div(J))) < 1e-10
check("R3e: J = K_B R x textures: sitewise Gram = K_B^2 I AND div = 0 (both stencils)", ok)

# ---------------------------------------------------------------------------
# R4 — improper stratum: Q = -R(n, 2*pi/3), the parity partner
# ---------------------------------------------------------------------------
print("\nR4: {Q in O(3)\\SO(3) : tr Q = 0} is exactly {-R(n, 2*pi/3)}")

ok = True
for _ in range(20):
    Q = random_o3_improper()
    ok &= abs(np.linalg.det(-Q) - 1) < 1e-12  # -Q is proper (odd dimension)
check("R4a: det(-Q) = +1 for improper Q — every improper Q is -R, R in SO(3)", ok)

ok = True
for _ in range(20):
    U = random_so3()
    Q = -(U @ R0 @ U.T)  # generic trace-zero improper orthogonal
    ok &= abs(np.trace(Q)) < 1e-12 and abs(np.linalg.det(Q) + 1) < 1e-12
    ev = np.sort_complex(np.linalg.eigvals(Q))
    tgt = np.sort_complex(
        np.array([-1.0, np.exp(1j * np.pi / 3), np.exp(-1j * np.pi / 3)])
    )
    ok &= np.max(np.abs(ev - tgt)) < 1e-9
check("R4b: trace-zero improper eigenvalues = {-1, e^{+-i pi/3}} (20 draws)", ok)

Qi = -P_CYC
Ji = affine_field(K_B * Qi, np.zeros(3), L)
G = gram_field(fwd_jacobian(Ji))
tgt = K_B**2 * np.eye(3)[:, :, None, None, None]
check(
    "R4c: -P_cyc texture: orthogonal, tr 0, det -1; sitewise GNC-s + div-free",
    np.allclose(Qi.T @ Qi, np.eye(3), atol=TOL)
    and abs(np.trace(Qi)) < TOL
    and abs(np.linalg.det(Qi) + 1) < TOL
    and np.max(np.abs(G - tgt)) < 1e-10
    and np.max(np.abs(fwd_div(Ji))) < 1e-10,
)

# Born-Infeld resummation is det-blind: per-site speed = |V| for BOTH strata
ok = True
for Qtex in (P_CYC, -P_CYC, rodrigues((2, -1, 5), 2 * np.pi / 3)):
    V = 0.3 * unit(RNG.standard_normal(3))
    D = K_B * Qtex
    v_site = np.linalg.norm(D @ V) / K_B  # flux swing per tick / K_B under transport
    N = 27
    lhs = N * (-K_B * np.sqrt(1 - v_site**2))
    rhs = -N * K_B * np.sqrt(1 - np.dot(V, V))
    ok &= abs(v_site - np.linalg.norm(V)) < TOL and abs(lhs - rhs) < TOL
check("R4d: BI cores resum to -N K_B sqrt(1-V^2) for proper AND improper textures", ok)

# ---------------------------------------------------------------------------
# R5 — GNC-w (summed) is strictly weaker than GNC-s (pointwise)
# ---------------------------------------------------------------------------
print("\nR5: summed GNC-w does not imply pointwise GNC-s")

G1 = K_B**2 * np.diag([2.0, 1.0, 0.0])
G2 = K_B**2 * np.diag([0.0, 1.0, 2.0])
D1 = np.sqrt(np.maximum(G1, 0.0))  # diagonal PSD square roots: D^T D = G
D2 = np.sqrt(np.maximum(G2, 0.0))
summed = D1.T @ D1 + D2.T @ D2
check(
    "R5a: two-site example — Gram sum = 2 K_B^2 I, neither site Gram = K_B^2 I",
    np.allclose(summed, 2 * K_B**2 * np.eye(3), atol=TOL)
    and not np.allclose(D1.T @ D1, K_B**2 * np.eye(3), atol=1e-3)
    and not np.allclose(D2.T @ D2, K_B**2 * np.eye(3), atol=1e-3),
)

# ---------------------------------------------------------------------------
# R6 — lattice folds: the affine classification is NOT exhaustive on Z^3
# ---------------------------------------------------------------------------
print("\nR6: non-affine GNC-s lattice fields exist (folds); one is divergence-free")

# (a) coordinate fold: f = +-1-increment walk. GNC-s everywhere, div never 0.
Lf = 12
steps = RNG.choice([-1.0, 1.0], size=Lf - 1)
steps[0], steps[1] = 1.0, -1.0  # force at least one turn (non-affine)
f = np.concatenate([[0.0], np.cumsum(steps)])
X = coords(Lf) + (Lf - 1) / 2.0  # integer indices 0..Lf-1
Jw = np.stack([K_B * f[X[0].astype(int)], K_B * X[1], K_B * X[2]])
Gw = gram_field(fwd_jacobian(Jw))
tgt = K_B**2 * np.eye(3)[:, :, None, None, None]
dw = fwd_div(Jw)
check(
    "R6a: walk-fold J = K_B(f(x1), x2, x3): sitewise Gram = K_B^2 I everywhere",
    np.max(np.abs(Gw - tgt)) < 1e-10,
)
check(
    "R6b: walk-fold divergence in {K_B, 3K_B} — never 0 (div-free is a real constraint)",
    np.min(np.abs(dw)) > 0.9 * K_B,
)

# (b) trace-zero fold: J = K_B P_cyc (|x1 - x0|, x2, x3).
x0 = (Lf - 1) // 2
m = np.abs(X[0] - x0)
F = np.stack([m, X[1], X[2]])
Jf = K_B * np.einsum("ab,bxyz->axyz", P_CYC, F)
Df = fwd_jacobian(Jf)
Gf = gram_field(Df)
check(
    "R6c: trace-zero fold: sitewise Gram = K_B^2 I at EVERY interior site",
    np.max(np.abs(Gf - tgt)) < 1e-10,
)
check(
    "R6d: trace-zero fold: div = 0 at every interior site (fwd AND bwd stencils)",
    np.max(np.abs(fwd_div(Jf))) < 1e-10 and np.max(np.abs(bwd_div(Jf))) < 1e-10,
)
# Jacobian takes exactly the two orthogonal values K_B P_cyc, K_B P_cyc sigma1
Dflat = Df.reshape(3, 3, -1)
vals = {tuple(np.round(Dflat[:, :, n].ravel() / K_B, 9)) for n in range(Dflat.shape[2])}
expected = {
    tuple(np.round(P_CYC.ravel(), 9)),
    tuple(np.round((P_CYC @ SIGMA1).ravel(), 9)),
}
check(
    "R6e: fold Jacobian takes exactly {K_B P_cyc, K_B P_cyc sigma1} — non-affine",
    vals == expected
    and abs(np.trace(P_CYC @ SIGMA1)) < TOL
    and np.allclose((P_CYC @ SIGMA1).T @ (P_CYC @ SIGMA1), np.eye(3), atol=TOL),
)
# rank-one connection: the two branch matrices differ by a rank-1 matrix
diff = P_CYC - P_CYC @ SIGMA1
check(
    "R6f: branch matrices rank-one connected (fold compatibility across the plane)",
    np.linalg.matrix_rank(diff, tol=1e-10) == 1,
)
# magic-angle condition: fold needs (Q)_11 = 0 <=> axis n1^2 = 1/3
ok = True
for _ in range(10):
    n1 = RNG.uniform(-1, 1)
    rest = unit(RNG.standard_normal(2)) * np.sqrt(max(1 - n1**2, 0))
    Rq = rodrigues((n1, rest[0], rest[1]), 2 * np.pi / 3)
    ok &= abs(Rq[0, 0] - (-0.5 + 1.5 * n1**2)) < 1e-10
check("R6g: R(n,2pi/3)_11 = -1/2 + (3/2) n1^2; fold-normal magic angle n1^2 = 1/3", ok)

# ---------------------------------------------------------------------------
# R7 — continuum C^2 rigidity, key step as finite linear algebra
# ---------------------------------------------------------------------------
print("\nR7: {A_ijk : A_ijk = A_jik, A_ijk + A_ikj = 0} = {0}  (27-dim rank check)")

rows = []
def idx(i, j, k):
    return 9 * i + 3 * j + k

for i in range(3):
    for j in range(3):
        for k in range(3):
            r = np.zeros(27)
            r[idx(i, j, k)] += 1
            r[idx(j, i, k)] -= 1
            rows.append(r.copy())
            r = np.zeros(27)
            r[idx(i, j, k)] += 1
            r[idx(i, k, j)] += 1
            rows.append(r)
M = np.array(rows)
check(
    "R7a: constraint matrix has rank 27 — null space is exactly {0}",
    np.linalg.matrix_rank(M, tol=1e-10) == 27,
)

# ---------------------------------------------------------------------------
# R8 — the d = 1 instance is the clock-hypothesis shape
# ---------------------------------------------------------------------------
print("\nR8: d = 1 rigidity: |a| = K_B <=> a^2 = K_B^2 <=> a/K_B in O(1) = {+1,-1}")

ok = True
for a in (K_B, -K_B):
    ok &= abs(abs(a) - K_B) < TOL and abs(a**2 - K_B**2) < TOL and a / K_B in (1.0, -1.0)
for a in (0.9 * K_B, -1.1 * K_B):
    ok &= abs(a**2 - K_B**2) > 1e-4
check("R8a: the 1x1 case of R1 — the clock hypothesis' pinned-swing shape", ok)

# ---------------------------------------------------------------------------
print("\n" + "=" * 76)
npass = sum(1 for _, ok in _results if ok)
print(f"RESULT: {npass}/{len(_results)} PASS")
if npass != len(_results):
    for name, ok in _results:
        if not ok:
            print(f"  FAILED: {name}")
    sys.exit(1)
print("All finitely-checkable claims of LEMMA_GNC_RIGIDITY.md verified.")
print("No tag moved: GNC un-forced; clock hypothesis [AXIOM]; Q_ij measurement [OPEN].")
