# PREREG — FTD Native Strong-Field Gravity Signature Campaign

**Status:** [PRE-REGISTRATION — hash-locked before execution]
**Date:** 2026-05-27
**Campaign ID:** FTD-0213
**Funder / Context:** FTD emergent gravity audit / Arc C2 spin-2 boundary theorem follow-up

---

## §1 — Objective & Process Protocol

This document pre-registers and lock-secures the design of the **FTD Native Strong-Field Gravity Signature Campaign (FTD-0213)**.

FTD's spin-2 boundary theorem (FTD-0209) proved that there is no fundamental spin-2 graviton mode on the FTD discrete substrate. Consequently, FTD's native gravity is at most a **scalar-vector** theory mediated by the quasi-static latency field $\mathcal{L}$ and the propagating flux field $\mathbf{J}$. While standard GR can be matched by *positing* a spin-2 metric perturbation $h_{\mu\nu}$ via the Deser-bootstrap (FTD-0189), the *native* gravity without this imported metric scaffold must exhibit distinct phenomenological signatures in the strong-field regime.

This campaign designs a high-precision numerical orbit analysis comparing the FTD native scalar-vector gravity against standard General Relativity (GR) across three crucial observables: the effective potential/ISCO, periapsis precession, and binary pulsar orbital decay due to gravitational radiation.

### Methodological Protocol:
1. This pre-registration document is committed and git-tagged `preregister-strong-field-gravity-v1` before any numerical code is executed or results are generated.
2. The SHA256 of this file is recorded in the manifest `REF_PREREGISTER_MANIFEST.md` and the campaign is reserved in `LEDGER.md`.
3. The simulator script `scripts/exploration/verify_strong_field_gravity.py` is written and executed.
4. The final outcomes and exact signatures are published in the result document `docs/theory/03_derivations/FOUND_STRONG_FIELD_GRAVITY_SIGNATURE.md`.

---

## §2 — The Central Question

**Q-STRONG-FIELD-GRAVITY:** What are the exact physical and mathematical differences in strong-field orbits (ISCO radius, periapsis precession) and gravitational radiation backreaction (binary orbital decay) between standard spin-2 General Relativity and FTD's native scalar-vector gravity, and is the FTD native model observationally excluded in the absence of the imported GR metric scaffold?

---

## §3 — Definitions & Formalisms

*   **Definition D1 (Standard GR Schwarzschild Orbit):** A test particle orbiting a central mass $M$ in the equatorial plane ($\theta = \pi/2$) satisfies the radial geodesic equation:
    $$\left(\frac{dr}{d\tau}\right)^2 = E^2 - V_{\text{eff}}^{\text{GR}}(r), \qquad V_{\text{eff}}^{\text{GR}}(r) = \left(1 - \frac{2M}{r}\right)\left(1 + \frac{L^2}{r^2}\right)$$
    which yields the Schwarzschild ISCO radius $r_{\text{ISCO}}^{\text{GR}} = 6M$.
*   **Definition D2 (FTD Native Scalar Gravity Orbit):** A test particle in the native scalar-only gravity couples to the latency field $\mathcal{L}$ through the Born-Infeld core, giving the radial EOM:
    $$\left(\frac{dr}{d\tau}\right)^2 = E^2 - V_{\text{eff}}^{\text{FTD}}(r)$$
    where the effective potential and resulting ISCO radius are numerically integrated.
*   **Definition D3 (Gravitational Radiation Power):**
    *   In standard GR (spin-2), the power radiated by a binary system is governed by the **quadrupole formula**:
        $$P^{\text{GR}} = \frac{32 G^4 m_1^2 m_2^2 (m_1 + m_2)}{5 c^5 r^5}$$
    *   In FTD's native scalar-vector gravity, the radiation is mediated by spin-1 vector waves ($\mathbf{J}$) and spin-0 scalar potentials. Since gravito-charge matches inertial mass by the equivalence principle, the gravito-dipole moment vanishes in the center-of-mass frame. However, the vector field $\mathbf{J}$ emits spin-1 quadrupole radiation, and the scalar field $\mathcal{L}$ (if dynamical) emits spin-0 monopole/quadrupole radiation, yielding a power $P^{\text{FTD}}$ with distinct scaling dimensions.
*   **Definition D4 (Binary Pulsar Decay Rate):** The orbital period decay rate $dP/dt$ for a binary system of masses $m_1, m_2$ in a circular orbit of radius $r$ is:
    $$\frac{dP}{dt} = -6\pi G^{2/3} \left(\frac{P}{2\pi}\right)^{5/3} \frac{m_1 m_2}{(m_1 + m_2)^{1/3}} \frac{P_{\text{rad}}}{m_1 m_2 / (2r)}$$
    where $P_{\text{rad}}$ is the gravitational wave radiation power.

---

## §4 — Admissible Parameters & Bounds

The numerical sweeps and evaluations are strictly confined to:
1.  Mass scales $M = 1.0$, spin parameters $a \in [0, 0.99M]$, and test particle mass $m \ll M$.
2.  Orbit radii $r \in [2M, 50M]$ to probe the strong-field region down to the event horizon.
3.  Precession calculations performed over at least 5 complete orbits.
4.  No post-hoc tuning of constants ($\alpha$, $G_N$, $K_B$) to force a match with GR.

---

## §5 — Three Pre-Blessed Outcomes

*   **Outcome A (Clear Distinguishing Signature Mapped):** FTD's native scalar-vector gravity yields a self-consistent, stable strong-field orbit profile with a specific, quantifiable ISCO/precession signature that differs from GR but remains potentially open to future astrophysical search.
*   **Outcome B (Indistinguishable in Weak-Field, Deviates in Strong-Field):** FTD's native gravity is shown to exactly recover standard GR/Newtonian limits in the weak-field regime, but diverges dramatically in the strong-field limit (e.g., $r < 10M$) where the lack of spin-2 modes prevents matching.
*   **Outcome C (Observational Exclusion):** The native FTD scalar-vector gravity (without the imported spin-2 metric perturbations) predicts an orbital decay rate or strong-field precession that is immediately and heavily excluded by high-precision astrophysical observations (e.g., the Hulse-Taylor binary pulsar decay or EHT horizon measurements), proving that FTD must rely on an imported effective metric scaffold rather than pure native emergence.

---

## §6 — Falsifiers

*   **F-a (Axiomatic violation):** Introducing ad-hoc spin-2 fields or metric degrees of freedom that are not derived from the $\{J, s, \mathcal{L}\}$ substrate fields.
*   **F-b (Weak-field Newton failure):** The native gravity does not recover the standard Newtonian $1/r^2$ force law in the far-field limit $r \gg r_s$.
*   **F-c (Binary pulsar discrepancy):** The native scalar-vector radiation formula yields an orbital period decay rate that is in direct contradiction with the extremely tight Hulse-Taylor constraints ($\leq 0.1\%$ error relative to GR quadrupole) without introducing non-standard screening mechanisms.

---

## §7 — Banned Moves

*   **B-1 (Posterior screening introduction):** Inventing ad-hoc screening mechanisms (e.g., chameleon or Vainshtein) post-hoc to hide a failing binary pulsar decay rate.
*   **B-2 (Attribution trailer):** Banned AI co-author or generator footer additions to the commit message.

---

## §8 — Verification Procedure

1.  Write the orbit simulator `verify_strong_field_gravity.py` in Python.
2.  Compute $V_{\text{eff}}^{\text{GR}}(r)$ and $V_{\text{eff}}^{\text{FTD}}(r)$ and extract their ISCO radii.
3.  Simulate a strong-field orbit at $r = 10M$ with eccentricity $e = 0.1$ and compute the periapsis precession rate $\Delta\phi$ under both theories.
4.  Compute the gravitational radiation backreaction power $P^{\text{FTD}}$ and the corresponding orbital decay rate $dP/dt$, comparing them to standard GR and binary pulsar observations.
5.  Run the full verification suite `proof_master_verification.py` and `test_all_physics.py` to ensure zero regressions.
6.  Publish the final verdict in `docs/theory/03_derivations/FOUND_STRONG_FIELD_GRAVITY_SIGNATURE.md`.
