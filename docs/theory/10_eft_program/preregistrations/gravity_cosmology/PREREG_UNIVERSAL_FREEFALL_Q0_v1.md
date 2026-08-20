# PRE-REGISTRATION — Universal free-fall Q0 (test-body action in external \(\mathcal{L}\))

**Tag:** `[PRE-REGISTRATION]` — locks the desk proof for [`SCOPE_UNIVERSAL_FREEFALL_v1.md`](../../scopes_and_specs/SCOPE_UNIVERSAL_FREEFALL_v1.md) Q0. Contains **no result**.
**Date:** 2026-08-19
**Hash-lock target tag:** `preregister-universal-freefall-q0-v1` (pending owner commit; until the tag resolves in git this lock is recoverable only as `anchored-late` by the §12 prefix SHA256).
**LEDGER reservation:** FTD-1013 (next free after FTD-1012 at lock draft).
**Parent:** SCOPE_UNIVERSAL_FREEFALL_v1.md (2026-08-19). Sibling GW holonomy Q0 is a **different** lock and is excluded.
**Does not move:** FTD-0250, FTD-0349, FTD-0402, FTD-0208, U-8, FTD-0131, FTD-0189, FTD-0193, FTD-0213.

> LOCK-STD v1 (`SPEC_LOCK_STANDARD.md`). Sections §1–§11 are frozen before any Euler–Lagrange output is observed. Post-hoc edits to §1–§11 void v1.

---

## §1 — The question (LOCKED)

**Q-UFF-Q0-v1.** Does there exist a test-voxel action in an **external** latency field \(\mathcal{L}_{\rm ext}\) such that, inside the locked class \(\mathcal{C}\) of §3:

1. **Existence / uniqueness (A1).** The integrand is uniquely (up to a positive constant absorbed into the mass coupling \(\alpha\)) the FC-2 proper-time element of `SPEC_FTD_LAGRANGIAN.md` §4.3, once the flat reduction of §5.1 is imposed.
2. **Newton (A3).** In the weak slow limit \(\beta^2\ll 1\), \(\mathcal{L}^2\ll 1\), the Euler–Lagrange equation is \(a = C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}\), equivalently \(a=-\nabla\Phi_N\) with \(\Phi_N=-(C_{\rm SPEED}^2/2)\mathcal{L}^2\).
3. **Remainder (A4).** The live engine update \(a=F/M_{\rm INERTIAL}\) with imposed \(M_{\rm INERTIAL}=M_{\rm GRAVITATIONAL}=K_B\) is **not** identified with this action.

**Not asked:** strong EP, GNC, self-force, composition with extra forces on, geodesic principle from P1–P5 alone, Phase-G derivation of \(1/r\), identification \(\mathcal{L}^2=r_s/r\), graviton / \(h_{\mu\nu}\), engine campaign.

**Prior-favoured outcome.** FOUND. Favoured is not predetermined; §6–§8 can return CLOSED-NEGATIVE, UNDERDETERMINED, or IMPROPER.

---

## §2 — Types already set (LOCKED)

Admissible primitives:

- P1–P5 as in the constitution (discrete uncontained lattice, discrete time, ternary states, Moore locality, determinism). Used here only as the ambient of a test worldline; no new lattice dynamics is derived.
- **FC-2 clock element** (`SPEC_FTD_LAGRANGIAN.md` §4.3, constitution §5.2):
  \[
  \frac{d\tau}{dt}=\sqrt{\max(f-\beta^2,0)},\qquad
  f=1-\mathcal{L}^2,\qquad
  \beta=\frac{|u|}{C_{\rm SPEED}}.
  \]
  Domain of the proof: \(f-\beta^2>0\). The `max` is a causal cutoff, not a variational ingredient.
- Manifestation = “has a clock”: a test occupant couples to \(\mathcal{L}_{\rm ext}\) only through that element. No colour, EM, lock, or self-sourced \(\mathcal{L}\).
- Particle-core conventions of SPEC §5.1 and §5.4: flat \(L=-E_{\rm REST}\sqrt{1-\beta^2}\); weak expansion \(-E_{\rm REST}+(E_{\rm REST}/2)\beta^2+(E_{\rm REST}/2)\mathcal{L}^2\); \(E_{\rm REST}=M_{\rm INERTIAL}C_{\rm SPEED}^2\) with \(M_{\rm INERTIAL}=K_B\).
- External well: \(\mathcal{L}_{\rm ext}=\mathcal{L}(x)\) prescribed, independent of the test body, slowly varying (tides neglected). One Cartesian worldline coordinate \(x(t)\) is sufficient; the 3D statement replaces \(d\mathcal{L}/dx\) by \(\nabla\mathcal{L}\).

**Explicitly not set by this lock:** equality \(M_{\rm INERTIAL}=M_{\rm GRAVITATIONAL}\) as a derived identity; cluster \(m\propto N\) (FTD-0250); GNC (FTD-0349); the live \(F/M\) integrator as a variation of the Born–Infeld term (SPEC §4.1–§4.2).

---

## §3 — Admissible class \(\mathcal{C}\) (LOCKED)

A Lagrangian \(L(x,u)\) is in \(\mathcal{C}\) iff there exist a worldline-constant \(\alpha>0\) and a \(C^1\) function \(G:(0,1]\to\mathbb{R}\) such that

\[
L(x,u)=-\alpha\,G(s),\qquad
s=\sqrt{f(x)-\beta^2},\qquad
f(x)=1-\mathcal{L}(x)^2,\qquad
\beta=\frac{|u|}{C_{\rm SPEED}}.
\]

\(\alpha\) is the **degree-1 mass coupling**: \(\alpha=N E_{\rm REST}\) for \(N\in\mathbb{N}\) co-located test clocks sharing one worldline in \(\mathcal{L}_{\rm ext}\). Degree 1 is the extensivity identification, not an extra dynamical assumption.

**SR filter (admission, not a bonus).** \(L\) matches SPEC §5.1 identically: \(\mathcal{L}\equiv 0\) implies \(G\bigl(\sqrt{1-\beta^2}\bigr)=\sqrt{1-\beta^2}\) for all \(\beta\in[0,1)\). Equivalently \(G(s)=s\) on \((0,1]\).

**Out of \(\mathcal{C}\).** Any Lagrangian that is not an overall factor times a function of the single FC-2 scalar \(s\). In particular the Newtonian split \(L=\tfrac12 m_i u^2-m_g\Phi\) with independent \(m_i,m_g\), and any coupling of extra charges into \(L\).

**Geodesic-principle remainder.** Restricting to \(\mathcal{C}\) is a **[SELECTION]** relative to P1–P5. FC-2 supplies \(d\tau\) along a worldline; it does not by itself force the variational law that the worldline extremizes \(\int d\tau\). Q0 uniqueness is uniqueness **inside \(\mathcal{C}\)**, not a proof that P1–P5 force \(\mathcal{C}\).

---

## §4 — Executable protocol (LOCKED)

**Operator.** Symbolic Euler–Lagrange on a 1D worldline, plus Taylor expansion of the SPEC §5.4 particle core. No engine binary, no toggle set, no CUDA, no golden tick, no numerical search.

**Correctness gate (must pass before a FOUND/CLOSED-NEGATIVE credit).** `scripts/proofs/proof_universal_freefall_q0.py` **recomputes** identities with SymPy / mpmath; it does not bookkeep author-supplied residuals (LOCK-STD 8).

**Checks (named, frozen):**

| ID | Claim recomputed | Pass if |
|---|---|---|
| V1 | SR filter \(\Rightarrow G(s)=s\) uniquely in \(\mathcal{C}\) | `G(sqrt(1-b**2)) - sqrt(1-b**2)` is identically 0 forces `G(s)-s` identically 0 |
| V2 | EL of \(L=-\alpha s\) independent of \(\alpha\) | residual after cancelling \(\alpha\) is \(\alpha\)-free; **zero evidential weight** (see §7) |
| V3 | SPEC §5.4 series | series of \(-E\sqrt{1-\beta^2-\mathcal{L}^2}\) through quadratic order equals \(-E+(E/2)\beta^2+(E/2)\mathcal{L}^2\) |
| V4 | weak EL | leading-order EL \(\Rightarrow a = C^2 \mathcal{L}\,\mathcal{L}'\) |
| V5 | Newton potential | \(\Phi_N=-(C^2/2)\mathcal{L}^2\) satisfies \(a=-\Phi_N'\) given V4 |
| V6 | SR witness | \(G(s)=s^2\) fails V1 |
| V7 | UFF witness outside \(\mathcal{C}\) | \(L=\tfrac12 m_i u^2-m_g\Phi\) with \(m_i\neq m_g\) has \(a=-(m_g/m_i)\Phi'\) |
| V8 | \(N\)-extensivity | \(L=-N E s\) has the same weak \(a\) as \(N=1\) |
| V9 | engine remainder | live \(F/M\) with independent mass roles is V7-class, not \(\mathcal{C}\) |

No CODATA/PDG target is read. No near-miss scan. \(C_{\rm SPEED}\) remains symbolic.

---

## §5 — Outcome map (LOCKED; mutually exclusive, jointly exhaustive)

**IMPROPER** (precedes every physics verdict). Any of: (I1) a FOUND is claimed from V2 alone; (I2) the engine \(F/M\) integrator is identified with the \(\mathcal{C}\) action; (I3) a criterion that cannot fail on admissible data is treated as evidence; (I4) banned move of §8.

**FOUND.** Not IMPROPER, and V1 ∧ V3 ∧ V4 ∧ V5 ∧ V6 ∧ V7 ∧ V8 ∧ V9 all pass. V2 may pass or fail as a sanity print; it does not decide FOUND. Tag consequences (and only these):

- Test-body UFF for \(S=-\alpha\int d\tau\) with FC-2 \(d\tau\): **`[THEOREM given FC-2]`** inside \(\mathcal{C}\).
- Weak Newton \(a=-\nabla\Phi_N\), \(\Phi_N=-(C_{\rm SPEED}^2/2)\mathcal{L}^2\): **`[THEOREM given FC-2]`**. Connecting \(\Phi_N\) to \(GM/r\) cites FTD-0131 / Phase G / SPEC §5.2 identification; this lock does **not** re-prove \(1/r\).
- Class \(\mathcal{C}\) itself: remains **`[SELECTION]`**.
- Live engine \(m_i=m_g\): remains **`[IMPOSED]`**. FTD-0250 / 0349 / 0402 / 0208 / U-8 **unmoved**.

**CLOSED-NEGATIVE.** Not IMPROPER, and at least one of V1, V3, V4, V5 fails (a definite no inside \(\mathcal{C}\)+SR+weak Newton). Then UFF stays the imposed mass-role equality; §12-EP stays without a geometric desk anchor.

**UNDERDETERMINED.** Not IMPROPER, not FOUND, not CLOSED-NEGATIVE: the identities cannot be completed (SymPy fails to reduce, or V4’s leading-order statement is ill-posed) without minting a definition absent from this lock.

**Partition proof.** Exactly one row fires:

| Admissible result of §4 | IMPROPER | FOUND | CLOSED-NEG | UNDERDET |
|---|:---:|:---:|:---:|:---:|
| I1–I4 | ✓ | | | |
| V1∧V3∧V4∧V5 and V6–V9 pass, not I* | | ✓ | | |
| V1 or V3 or V4 or V5 fails, not I* | | | ✓ | |
| V1,V3,V4,V5 neither all-pass nor a definite fail | | | | ✓ |

No dataset fires two columns: IMPROPER is checked first; FOUND requires every load-bearing V; CLOSED-NEGATIVE requires a load-bearing fail; UNDERDETERMINED is the complement.

---

## §6 — Frozen tie-breaks (LOCKED)

- V1: uniqueness is **projective** in \(\alpha\). \(G\mapsto\lambda G\) with \(\lambda>0\) absorbed into \(\alpha\) is the same action. Two non-proportional \(G\) both matching §5.1 \(\Rightarrow\) V1 fail.
- V4: “weak slow” means retain terms through first order in \(\{\beta^2,\mathcal{L}^2,\mathcal{L}\mathcal{L}'\}\) after dividing out the overall \(\alpha/s\) factor, then set \(\beta\to 0\), \(\mathcal{L}\to 0\) in every remaining coefficient of \(a-\cdots\). Higher-order remainder does not fail V4.
- V5 vs FTD-0131: V5 is the \(\Phi_N(\mathcal{L})\) identity only. Failure to mention \(1/r\) is not a V5 fail.
- If V2 fails for \(L=-\alpha s\) while V1,V4 pass: CLOSED-NEGATIVE (the advertised action would not even cancel \(\alpha\)).
- If V6 or V7 or V8 or V9 fails: the vacuity firewall is broken \(\Rightarrow\) IMPROPER, not FOUND.

---

## §7 — Vacuity firewall (LOCKED)

| Criterion | Can it fail on admissible data? | Witness |
|---|---|---|
| V2 (\(\alpha\) drops in \(\mathcal{C}\)) | **No, inside \(\mathcal{C}\)** | Overall factor always drops from EL. **Declared vacuous in \(\mathcal{C}\); weight zero.** |
| V1 | Yes | V6: \(G(s)=s^2\) |
| V4 / V5 | Yes | A different clock element, e.g. \(d\tau/dt=f-\beta^2\) not sqrt, changes the quadratic potential identification. Locked class uses sqrt; the witness is the Newtonian split V7, which is outside \(\mathcal{C}\) and **does** produce mass-dependent \(a\). |
| V7 | Yes | \(m_i=m_g\) makes V7’s \(a\) mass-independent — that is the engine’s imposed EP, not a \(\mathcal{C}\) theorem. |

FOUND may not cite V2 as evidence. The geometric content is V1+V4+V5: FC-2’s scalar is the unique \(G\) in \(\mathcal{C}\) matching SR, and that action’s weak limit is universal Newton in \(\Phi_N(\mathcal{L})\).

---

## §8 — Banned moves (LOCKED)

- Promote or “derive” FTD-0250, FTD-0349, FTD-0402, FTD-0208, U-8, or engine \(m_i=m_g\).
- Identify the live \(F/M\) integrator with variation of \(S=-\alpha\int d\tau\).
- Smuggle strong EP, GNC, self-force, or tides into weak EP.
- Treat extra forces on as a UFF falsifier.
- Posit \(h_{\mu\nu}\), a graviton, or occupancy telegraph as this Q0.
- Coincidence / near-miss numerical search; CODATA insertion as derivation.
- Claim UFF from P1–P5 without FC-2, or claim P1–P5 force class \(\mathcal{C}\).
- Re-open \(G_N=1/100\); identify \(x_-\) with \(N_c\); golden-tick change.
- Edit this prereg after observing EL output.

---

## §9 — Quantifier coverage (LOCKED)

- Uniqueness is \(\forall G\in C^1((0,1])\) such that \(L=-\alpha G(s)\) matches SPEC §5.1. It is **not** uniqueness among all Lagrangians of \((x,u,\mathcal{L})\).
- UFF is \(\forall\alpha>0\) and \(\forall N\in\mathbb{N}\) in the test-body limit (external \(\mathcal{L}\), extra forces off). It is **not** \(\forall\) composition channels.
- Newton is the weak slow limit of this 1D EL. It is **not** a 3D curvature theorem and **not** a derivation of \(\mathcal{L}^2=r_s/r\).

---

## §10 — Execution window and debt (LOCKED)

- **Window:** 2026-08-19, America/Chicago, this session through 23:59. A lock past window with no run, or a run past window with no verdict, auto-books an F10 row.
- **Executor:** coding agent, desk proof only.
- **Census:** this is not an arc rollover; census RED is not a gate on a gravity desk Q0. No engine campaign.
- **Anchor:** git tag before execution is the standard. Owner has not been asked to commit. Result docs must cite the §12 prefix SHA256 and mark `anchored-late` until `git rev-parse preregister-universal-freefall-q0-v1` succeeds.

---

## §11 — Reconciliation (LOCKED)

Any LEDGER booking of FTD-1013 uses banner-plus-preserved-kernel: previous EP/mass-role rows stay; this row adds the test-body desk statement at the tags of §5. No deletion of FTD-0402 PARTIAL or FTD-0250 IMPOSED.

---

<!-- END HASHED PREFIX -->

## §12 — Content hash (LOCK-STD 9; excluded from hashed prefix)

SHA256 of the UTF-8 bytes from the start of this file through the line `<!-- END HASHED PREFIX -->` inclusive, with newline bytes as stored.

**Content SHA256 of hashed prefix:** `1F8A8005C05444EA04DC430FFD5CD620304D3C5D1AE3CAE8E0F35829BA215B23`

---

## §13 — Execution record (not part of the hashed prefix)

Executed 2026-08-19, desk proof, no engine binary. Verifier `scripts/proofs/proof_universal_freefall_q0.py` SHA256 `66FBF6DE5CCDD2649399E25EFE44C21B9D1143A96CA0196E93C91A03319E56D5`, **15/15**, frozen classifier **FOUND**. Result: [`DERIV_UNIVERSAL_FREEFALL_Q0_v1.md`](../../../03_derivations/gravity_and_cosmology/DERIV_UNIVERSAL_FREEFALL_Q0_v1.md). Anchor: **`anchored-late`** until `git rev-parse preregister-universal-freefall-q0-v1` succeeds. FTD-0250 / 0349 / 0402 / 0208 / U-8 unmoved.
