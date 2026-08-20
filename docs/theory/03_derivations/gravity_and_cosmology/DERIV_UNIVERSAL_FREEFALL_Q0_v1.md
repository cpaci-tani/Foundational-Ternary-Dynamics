# DERIV — Universal free fall Q0 (test-body action in external \(\mathcal{L}\))

**Tag:** `[THEOREM given FC-2]` (test-body UFF + weak Newton in \(\Phi_N(\mathcal{L})\)) + `[SELECTION]` (class \(\mathcal{C}\) / geodesic principle) + `[IMPOSED]` remainder (live engine \(m_i=m_g\)).
**Date:** 2026-08-19
**LEDGER:** FTD-1013
**Lock:** [`PREREG_UNIVERSAL_FREEFALL_Q0_v1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_UNIVERSAL_FREEFALL_Q0_v1.md) — prefix SHA256 `1F8A8005C05444EA04DC430FFD5CD620304D3C5D1AE3CAE8E0F35829BA215B23`. Git tag `preregister-universal-freefall-q0-v1` pending owner commit; this result is **`anchored-late`** until that tag resolves.
**Verifier:** `scripts/proofs/proof_universal_freefall_q0.py` SHA256 `66FBF6DE5CCDD2649399E25EFE44C21B9D1143A96CA0196E93C91A03319E56D5` — **15/15**, verdict **FOUND**.
**Parent:** [`SCOPE_UNIVERSAL_FREEFALL_v1.md`](../../10_eft_program/scopes_and_specs/SCOPE_UNIVERSAL_FREEFALL_v1.md).
**Does not move:** FTD-0250, FTD-0349, FTD-0402, FTD-0208, U-8, FTD-0131, FTD-0189.

---

## 0 · Verdict

Inside the locked class \(\mathcal{C}\) (Lagrangians \(L=-\alpha G(s)\) with \(s\) the FC-2 scalar of `SPEC_FTD_LAGRANGIAN.md` §4.3), the SR filter of SPEC §5.1 forces \(G(s)=s\). The action is therefore

\[
S=-\alpha\int d\tau,\qquad
d\tau=\sqrt{f-\beta^2}\,dt,\qquad
\alpha=N E_{\rm REST}.
\]

Its Euler–Lagrange equation does not contain \(\alpha\) or \(N\). In the weak slow limit it is Newton with

\[
\Phi_N=-\frac{C_{\rm SPEED}^2}{2}\mathcal{L}^2,\qquad
\mathbf a=-\nabla\Phi_N=C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}.
\]

That is universal free fall for a test occupant that only carries a clock, **given FC-2**, in an **external** well, extra forces off.

It is **not** a derivation of FC-2, **not** a proof that P1–P5 force the geodesic principle, **not** a derivation of \(\mathcal{L}^2=r_s/r\) or of \(GM/r\), and **not** an identification of the live \(F/M\) integrator with this action.

---

## 1 · Class \(\mathcal{C}\) and uniqueness

FC-2 supplies the scalar \(s=\sqrt{f-\beta^2}\) along a worldline. Class \(\mathcal{C}\) is the Lagrangians that are an overall mass coupling times a \(C^1\) function of that scalar alone (prereg §3). Restricting to \(\mathcal{C}\) is a **[SELECTION]** relative to P1–P5: the clock element is given; the variational law “extremize \(\int d\tau\)” is the class, not a Scale-0 theorem.

SPEC §5.1 requires \(G\bigl(\sqrt{1-\beta^2}\bigr)=\sqrt{1-\beta^2}\) for all \(\beta\in[0,1)\). The map \(\beta\mapsto\sqrt{1-\beta^2}\) is locally invertible on \((0,1)\) (verifier V1a–V1b). Hence \(G(s)=s\) uniquely in \(\mathcal{C}\). The witness \(G(s)=s^2\) fails the filter (V6).

Uniqueness is **inside \(\mathcal{C}\)**. Among all Lagrangians of \((x,u,\mathcal{L})\) it is not unique: the Newtonian split \(\tfrac12 m_i u^2-m_g\Phi\) is a different class and produces mass-dependent fall when \(m_i\neq m_g\) (V7).

---

## 2 · Weak Newton

SPEC §5.4, recomputed (V3):

\[
-E_{\rm REST}\sqrt{1-\beta^2-\mathcal{L}^2}
=-E_{\rm REST}+\frac{E_{\rm REST}}{2}\beta^2+\frac{E_{\rm REST}}{2}\mathcal{L}^2+\cdots.
\]

With \(E_{\rm REST}=M_{\rm INERTIAL}C_{\rm SPEED}^2\) the quadratic terms are \(\tfrac12 M|\mathbf u|^2 - M\Phi_N\) for \(\Phi_N=-(C_{\rm SPEED}^2/2)\mathcal{L}^2\). The weak Euler–Lagrange equation is \(\mathbf a=C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}=-\nabla\Phi_N\) (V4, V5), independent of \(E_{\rm REST}\) and of clock count \(N\) (V8).

Connecting \(\Phi_N\) to \(GM/r\) cites Phase G / FTD-0131 and the SPEC §5.2 identification \(\mathcal{L}^2=r_s/r\). Those are **not** this Q0.

Overall-factor cancellation (V2) is **vacuous inside \(\mathcal{C}\)** and carries zero evidential weight. The geometric content is V1+V4+V5.

---

## 3 · Engine remainder

SPEC §4.1–§4.2: the live update accumulates forces and divides by an imposed \(M_{\rm INERTIAL}\), with \(M_{\rm GRAVITATIONAL}=M_{\rm INERTIAL}=K_B\) **[IMPOSED]** (FTD-0402). That is the V7 split, not a member of \(\mathcal{C}\) (V9). Cluster \(m\propto N\) (FTD-0250) and GNC (FTD-0349) are untouched.

The later engine campaign (FTD-1014) ran the registered \(a(N)/g_{\rm ext}\) comparison extra-forces-off on a prescribed well: **CLOSED-NEGATIVE**. The production tick is not this action. Live \(m_i=m_g\) remains `[IMPOSED]`.

---

## 4 · What this does not license

- Strong EP, self-force, tides, composition with colour/EM on.
- A graviton, \(h_{\mu\nu}\), or occupancy telegraph.
- Promotion of U-8, \(\alpha_G\), or \(g_{rr}\).
- UFF from P1–P5 without FC-2.

---

*Verifier 15/15 FOUND. Zero promotions outside the tags of §0.*
