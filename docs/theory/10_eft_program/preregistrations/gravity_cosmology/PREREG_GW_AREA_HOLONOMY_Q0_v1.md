# PRE-REGISTRATION — Radiative shears of transport (GW area-holonomy Q0)

**Tag:** `[PRE-REGISTRATION]` — locks the desk kinematic census for [`SCOPE_GW_AREA_HOLONOMY_v1.md`](../../scopes_and_specs/SCOPE_GW_AREA_HOLONOMY_v1.md) Q0. Contains **no result**.
**Date:** 2026-08-19
**Hash-lock target tag:** `preregister-gw-area-holonomy-q0-v1` (pending owner commit; until the tag resolves this lock is `anchored-late` via §12 prefix SHA256).
**LEDGER reservation:** FTD-1015.
**Parent:** SCOPE_GW_AREA_HOLONOMY_v1.md §4. Sibling UFF Q0 (FTD-1013) is a **different** lock (static well, not radiative shears).
**Does not move:** FTD-0189, FTD-0193, FTD-0209, FTD-0213, FTD-0026, U-8, FTD-1013, FTD-1014. No golden tick. No engine \(\Omega\). No P6C-G purchase.

> LOCK-STD v1 (`SPEC_LOCK_STANDARD.md`). Sections §1–§11 are frozen before any helicity-census output is observed. Post-hoc edits to §1–§11 void v1.

---

## §1 — The question (LOCKED)

**Q-GW-HOLONOMY-Q0-v1.** Linearize the spatial clock-transport connection \(\Omega\) of the parent SCOPE, with the kinematic constraints named in §2 and **no action**. Is the residual local field content, classified by little-group helicity at generic \(k\neq 0\), **exactly** the pair \(\{+2,-2\}\)?

- **Pass (A1).** Residual helicity set \(H=\{+2,-2\}\) with multiplicity one each, and no leftover helicity \(0\) or \(\pm 1\).
- **Fail.** Any other \(H\): missing \(\pm 2\), extra scalar/vector, or dimension \(\neq 2\).

**Not asked.** Gapless dispersion; cone-compatibility with the flux wave; TEGR / Palatini / other action; engine holonomy correlator; graviton / \(h_{\mu\nu}\); P6C-G adoption; Hulse–Taylor; identifying \(\Omega\) with \(J\) or \(A_\mu\).

**Prior-favoured outcome.** CLOSED-NEGATIVE (kinematic leftovers after gauge + fixed solder). Favoured is not predetermined.

---

## §2 — Types already set (LOCKED)

From the parent SCOPE, used as kinematics only:

1. **Carrier.** Linearized spatial connection \(\omega_{ij}(x)\in\mathbb{R}^{3\times 3}\). The first index \(i\) is the link direction; the second \(j\) is the soldered \(\mathfrak{so}(3)\) index. Solder form: the cubic grid **is** the coframe, so adjoint and spatial indices are identified. Edge lengths are not dynamical (\(\delta g_{ij}=0\) identically; no solder stretch).
2. **No \(\mathrm{SO}(1,3)\).** There is no \(\omega_{0j}\) and no \(\omega_{i0}\). Time remains P2 ticks. Radiative kinematics means the residual *spatial* \(\omega_{ij}\) that can change from tick to tick, not a spacetime connection.
3. **Gauge.** Local \(\mathrm{SO}(3)\) frame rotation, linearized: \(\omega_{ij}\mapsto\omega_{ij}+\partial_i\theta_j\). In Fourier space at wavevector \(k\), \(\delta\omega_{ij}=i k_i\theta_j\).
4. **Not in this lock.** An action, Hamiltonian constraints beyond the gauge identification, a mass term, a wave operator, or a hand TT projection.

**Native constraints (complete list).** (1)–(3) above. Nothing else.

---

## §3 — Residual and helicity (LOCKED)

**Configuration space.** Nine real fields \(\omega_{ij}\).

**Residual at generic \(k\neq 0\).** Quotient by the three-dimensional gauge orbit of (3). Dimension \(n_{\rm res}\) is recomputed (V2).

**Little group.** Take \(k=(0,0,k_z)\) with \(k_z\neq 0\) without loss (cubic continuum isotropy of the linearized identification; lattice Umklapp is out of scope). Residual components are those not in the gauge orbit. Helicity \(h\) is the weight under \(\mathrm{SO}(2)_k\): a mode \(m\) satisfies \(m\mapsto e^{ih\phi}m\) under rotation by \(\phi\) about \(k\). Both tensor indices rotate (solder).

**Helicity set \(H\).** The multiset of generator weights of a complex eigenbasis of the residual  representation. Integer helicities only; a 2-plane rotating as \(\mathrm{SO}(2)\) on \(\mathbb{R}^2\) contributes \(\{+h,-h\}\) when the complex eigenvalues are \(\pm ih\).

**TT shears.** Helicity \(\pm 2\) residual modes.

---

## §4 — Executable protocol (LOCKED)

**Operator.** Symbolic linear algebra on the \(9\)-component Fourier mode; no engine binary, no toggle set, no CUDA, no golden tick, no numerical search, no CODATA.

**Correctness gate.** `scripts/proofs/proof_gw_area_holonomy_q0.py` **recomputes** dimensions and helicity weights with SymPy; it does not bookkeep author-supplied residuals (LOCK-STD 8).

**Checks (named, frozen):**

| ID | Claim recomputed | Pass if |
|---|---|---|
| V1 | Nine real components | `ω` is a \(3\times 3\) independent-entry matrix (9 symbols) |
| V2 | Gauge orbit dimension 3 at \(k=\hat z\neq 0\) | rank of the map \(\theta_j\mapsto i k_i\theta_j\) equals 3; \(n_{\rm res}=6\) |
| V3 | No temporal connection in the type | the locked field list does not contain \(\omega_{0j}\) (static assertion in-code: only spatial `ω_ij`) |
| V4 | Residual generator is skew-Hermitian / pure-imaginary spectrum | eigenvalues of \(d/d\phi\) on the complexified residual are in \(i\mathbb{Z}\) |
| V5 | Helicity multiset \(H\) | \(H\) equals the frozen list printed by the verifier from those eigenvalues (recomputed, not typed in) |
| V6 | \(\pm 2\) subspace dimension | \(\dim\ker(G-2iI)+\dim\ker(G+2iI)\) recomputed |
| V7 | Leftover dimension | \(n_{\rm res}-(\text{V6 dim})\) recomputed |
| V8 | Vacuity witness: hand TT projection of a *symmetric* \(3\times 3\) at fixed \(k\) has dimension 2 | that count is 2, and is **not** this Q0’s residual |
| V9 | Catalog remainder | this census is of \(\omega_{ij}\), not of \(J\) or of \(J\otimes J\) (FTD-0209 catalog untouched) |

**Physics gate (classifies; does not fail the process if the identities reduce):**

| ID | Claim | FOUND if |
|---|---|---|
| A1 | Exactly two TT shears, nothing else | \(H=\{+2,-2\}\) as a set of distinct weights **and** \(n_{\rm res}=2\) **and** V6 dim \(=2\) **and** V7 leftover \(=0\) |

Gapless / cone-compatible are **not** A1. They require an action, which §2 forbids.

---

## §5 — Outcome map (LOCKED)

**IMPROPER** (precedes): (I1) selecting TEGR/Palatini/any action to kill leftovers, then claiming FOUND; (I2) hand TT-projecting the residual (V8’s 2) and calling that A1; (I3) identifying \(\Omega\) with \(J\), \(A_\mu\), occupancy, or a Fock graviton; (I4) claiming gapless/cone from this lock; (I5) moving FTD-0189/0193/0209/0213/0026/U-8; (I6) banned move of §8.

**FOUND.** Not IMPROPER. V1–V9 pass. A1 passes. Tag: kinematic residual of spatial \(\Omega\) **is** exactly two TT shears `[THEOREM — kinematic, no action]`. Gapless/cone remain `[OPEN]` (need a later action lock). P6C-G still unadopted.

**CLOSED-NEGATIVE.** Not IMPROPER. V1–V9 pass. A1 fails. Transport geometry as typed does **not** isolate a LIGO-like shear pair. Gravity remains the static well. Do not posit \(h_{\mu\nu}\). Do not retarget \(J\).

**UNDERDETERMINED.** Not IMPROPER. Any of V1–V9 fails to reduce (SymPy rank/spectrum ill-posed).

Partition: IMPROPER first; then protocol fail → UNDERDETERMINED; else A1 true → FOUND else CLOSED-NEGATIVE. One column only.

| Admissible result | IMPROPER | FOUND | CLOSED-NEG | UNDERDET |
|---|:---:|:---:|:---:|:---:|
| I1–I6 | ✓ | | | |
| V1–V9 pass and A1 | | ✓ | | |
| V1–V9 pass and not A1 | | | ✓ | |
| some V* neither pass nor a definite fail | | | | ✓ |

---

## §6 — Tie-breaks (LOCKED)

- Generic \(k\): \(k=(0,0,1)\) is the representative. A different axis is the same residual by relabelling.
- Helicity weights: if an eigenvalue \(\lambda\) of \(G=d/d\phi\) satisfies \(\lambda/i \in\mathbb{Z}\), the weight is that integer. Non-integer \(\lambda/i\) → V4 fail → UNDERDETERMINED, not a physics fail.
- Multiplicity: A1 requires \(n_{\rm res}=2\), not “at least two TT inside a larger residual.” Extra \(\pm 2\) copies also fail A1.
- Equality of sets: \(\{+2,-2\}=\{-2,+2\}\). A lone \(+2\) without \(-2\) fails A1.
- V8 passing is required for FOUND *and* for CLOSED-NEGATIVE (firewall live). If V8 fails, UNDERDETERMINED.

---

## §7 — Vacuity firewall (LOCKED)

| Criterion | Can fail? | Witness |
|---|---|---|
| A1 | Yes | Residual dim 6 with leftover helicity \(\pm 1\) and \(0\) (if that is what V5 returns) |
| V2 | Yes | Forgetting the gauge map (would report 9) |
| V8 | Yes | TT dim of a symmetric tensor is 2; if the code reported 0 or 9 the projector is wrong |
| Gapless/cone | **Not a criterion** | Declared unaskable without an action; citing them as FOUND is I4 |

Hand TT projection (V8) **always** yields 2 on a symmetric tensor. Using it as A1 is I2 (IMPROPER), evidential weight zero for this Q0.

---

## §8 — Banned moves (LOCKED)

- Select an action to reduce 6→2 and call that this Q0.
- Posit \(h_{\mu\nu}\) or a graviton as consolation if A1 fails.
- Retarget occupancy or \(J\); reuse IMP-S4 links.
- Promote FTD-0209 / 0193 / 0189; claim FTD “has GWs.”
- Quietly amend `SCOPE_CONSUMPTION_PROGRAM.md`’s “one massless spin-2 field.”
- Coincidence search; golden-tick change; engine \(\Omega\) representation.
- Edit this prereg after observing \(H\).

---

## §9 — Quantifier coverage (LOCKED)

A1 is \(\forall\) linearized Fourier modes of **this** spatial \(\omega_{ij}\) at generic \(k\neq 0\), after **this** gauge quotient, with solder identification. It is not \(\forall\) actions, not lattice-dispersed Umklapp, not GPU, not adopted P6C-G.

---

## §10 — Window (LOCKED)

2026-08-19 America/Chicago through 23:59, this session, desk verifier. Past window with no verdict books F10. Census is not a gate on this desk Q0. Git tag pending; result cites §12 SHA as `anchored-late`.

---

## §11 — Reconciliation (LOCKED)

FTD-1015 is a new row. FTD-0209 remains the no-helicity-±2 *particle* pole in the \(\{J,s,\mathcal{L}\}\) catalog. This row does not add \(\Omega\) to that catalog. U-8 unmoved. P6C-G unadopted either way.

---

<!-- END HASHED PREFIX -->

## §12 — Content hash (LOCK-STD 9; excluded from hashed prefix)

SHA256 of the UTF-8 bytes from the start of this file through the line `<!-- END HASHED PREFIX -->` inclusive, with newline bytes as stored.

**Content SHA256 of hashed prefix:** `6381C791B58F3E4259138CCED06A5777DF5175FE015F5BDD670804786DA035C9`

---

## §13 — Execution record (not part of the hashed prefix)

Executed 2026-08-19, desk verifier, no engine binary. `scripts/proofs/proof_gw_area_holonomy_q0.py` SHA256 `C5FF94BDBD81B1CDAD9F7EA7D117C2F1DF427F7F295A144610D23880EC130A5D`, protocol **11/11**, A1 failed. Frozen classifier **CLOSED-NEGATIVE**. Residual helicity multiset \(H=\{-2,-1,0,0,+1,+2\}\), \(n_{\rm res}=6\), \(n_{\rm TT}=2\), leftover \(=4\). Plus/cross exist inside a larger residual; they are not isolated. Result: [`ANALYSIS_GW_AREA_HOLONOMY_Q0_v1.md`](../../../03_derivations/gravity_and_cosmology/ANALYSIS_GW_AREA_HOLONOMY_Q0_v1.md). Anchor: **`anchored-late`** until `git rev-parse preregister-gw-area-holonomy-q0-v1` succeeds. FTD-0189 / 0193 / 0209 / 0213 / 0026 / U-8 / 1013 / 1014 unmoved. No graviton consolation. P6C-G not narrowed.
