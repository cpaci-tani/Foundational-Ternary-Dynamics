# FTD-0783 — Pair-Well Anharmonicity v1

**Status:** `[MEASURED CONTROL — STATIC, SELECTED COMPACT LAW]` +
`[CLOSED NEGATIVE — PAIR-BREATHING CARRIER CHANNEL]` +
`[STRUCTURAL COROLLARY — WELLS AND WALLS BRACKET THE QUARTIC]`
**Verdict:** `PAIR_BREATHING_CHANNEL_CLOSED`
**Parents:** `FTD-0739`, `FTD-0781`, `FTD-0782`
**Production impact:** none; static computation on the registered compact law

## 1. Result in one sentence

The compact-law bound pair's breathing mode fails the carrier conditions on **three
independent grounds** — softening everywhere, fundamental inside the acoustic band, and
a flow-curve reach of `a_max = 0.62` against the `a ~ 8` the `G*` regime requires — and
the failure generalises: **finite-depth binding wells pin every candidate at the
harmonic end of the flow curve, hard-core capacity walls pin candidates at the uniform
end, and the quartic — `G*`'s only home — lies strictly between, where no identified
native mechanism produces a potential.**

## 2. The well, exactly

The registered compact law (`DERIV_MINIMAL_MANY_BODY_MATTER_NETWORK_v1.md` eq. 3),
`V(q) = -16 eps (q - 3/2)^2 (q - 3/4)` for `q = r^2 < 3/2`, zero beyond, `eps = 0.01`
selected:

| feature | value |
|---|---|
| repulsive core | `V(0) = +27 eps` — the annihilation wall is **energetically shielded** for all bound energies |
| bond minimum | `r0 = 1`, depth `-eps`, stiffness `k = 96 eps` (matches the registered `k_bond`) |
| dissociation edge | `r = sqrt(3/2)`, `V -> 0^-` quadratically (C1 merge into vacuum) |
| small-oscillation frequency | `omega_0 = sqrt(96 eps/mu_red) = 1.3856` at unit constituent masses |

## 3. The three kills

**(a) Softening, everywhere [mass-independent].** `Omega(E)` by quadrature across the
bound spectrum is monotone decreasing: `1.3855` at `E = -0.999 eps` down to `0.9367` at
`E = -0.02 eps`, heading to zero at the separatrix — the universal fate of any
finite-depth well. `sign(dOmega/dA) < 0` throughout. The §32.3 hardening requirement
fails at every amplitude.

**(b) In-band [mass-dependent].** `omega_0/omega_B = 1.3856/2 = 0.693`: the fundamental
sits deep inside the acoustic band, so the non-resonance condition fails at linear
order and the breathing pair radiates; softening drives it further in. Scope: this
kill assumes unit constituent masses; above-band placement would require constituent
mass below `~0.48` lattice units, an engine parameter not audited here. Kills (a) and
(c) do not depend on the mass.

**(c) Flow-curve reach [mass-independent].** The quartic-dominance scale
`A_* = sqrt(k/lambda_eff) = 0.365` against the well's usable width `u_max = 0.225`
gives `a_max = 0.62`. The pair **dissociates before reaching even the 46%-of-span
point at `a = 1`**; the `G*` regime needs `a >~ 8`. The pair is pinned at the harmonic
end of the `sqrt(3 pi) -> G*` flow curve, permanently.

## 4. The structural corollary — wells and walls bracket the quartic

Kill (c) generalises beyond this law. Any finite-depth well caps its accessible
amplitude at its own width, so `a_max ~ width/A_*` is order one for generic wells, and
the separatrix guarantees softening before quartic dominance is approached:
**no dissociable bound state can traverse the flow curve.** Meanwhile the native
hard-core capacity constraint (`n(v) <= 1`, ternary site capacity) supplies *hardening*
confinement — but of the box class, `E = C I^2`, `k = 2`: the `n -> infinity` endpoint
of the exponent family, with uniform occupancy, all moments rational (`1/(r+1)`), and
**no `G*` content whatsoever**.

```text
finite wells  ->  pinned at n ~ 2  (harmonic end;   G*-free)
capacity walls -> pinned at n = oo (uniform end;    G*-free)
G* lives at n = 4 - strictly between, requiring a potential with
null-flat bottom and quartic growth over ~a decade of amplitude.
```

No identified native mechanism produces such a potential. This gives A2's null-flatness
selection its final, geometric characterisation: **the selection asks for exactly the
potential class that sits in the gap between everything binding provides and everything
exclusion provides.** The engine natively offers the two ends of the exponent family
and nothing in between.

## 5. What survives, honestly

1. The pair remains interesting as a **body** — bound, shielded from annihilation,
   metastable with physical endpoints — just not as a `G*` carrier. Phase-3 body
   tracking retains its object; Gate C loses its target.
2. The carrier question is now negative at **every identified door**: linear
   functionals (§32.1), the affine sector (FTD-0781), the pair channel (this result),
   generic wells and generic walls (§4). What has *not* been excluded: an unidentified
   native mechanism producing intermediate-exponent growth, or the coupled sector's
   behaviour once the reciprocal-transaction problem is solved — but no candidate for
   either is currently registered.
3. Scope: static analysis of the selected compact law at unit masses; lattice
   discreteness, Peierls effects, and the field dressing's back-reaction are not
   included. None of these plausibly rescues (a) or (c), which are structural.

## 6. SCOPE AMENDMENT (FTD-0787) — WITHDRAWN 2026-08-03 BY FTD-0789

> **This amendment is withdrawn.** FTD-0787's counterexample was refuted: its
> "quartic" is the curvature of a rectilinear chord across an exactly flat
> bend direction. **§4's corollary stands as originally written.** FTD-0789
> supplies its proper generalisation: for central-force networks at zero
> tension, rigid networks pin at `n = 2`, hypostatic networks whose flex
> extends to a finite mechanism escape to `n = infinity`, and `n = 4` requires
> first-order flexibility with **second-order rigidity** — a decidable matrix
> condition that no registered native configuration has been shown to meet.
>
> The withdrawn text follows for the record.

### 6 (withdrawn). SCOPE AMENDMENT 2026-08-03 (FTD-0787) — the corollary was too broad

§4's structural corollary concluded that `G*` lives at `n = 4` "where no
identified native mechanism produces such a potential." **That reach is now
falsified by exhibition.** FTD-0787 exhibits one, using only the compact law
registered here and no new primitive: the **transverse** displacement of a
collinear trimer. Because `r0 = 1` is the bond *minimum* (zero tension) and a
transverse offset changes bond length only at second order, the energy is
exactly quartic in the displacement — `V(d) = -2 eps + 24 eps d^4 - 32 eps d^6`,
with the quadratic coefficient identically zero **by geometry, not tuning**.

What stands, unchanged: the three kills of the pair *breathing* channel (a),
(b), (c); the `Omega(E)` quadrature; and the statement that finite wells pin
at `n ~ 2` **in the coordinate the bond stiffness acts on**. What was wrong
was the tacit generalisation from that coordinate to all coordinates of the
same well. A well can be harmonic radially and quartic transversally at once;
this one is.

The bracket theorem therefore brackets *longitudinal* coordinates only, and
the carrier question moves from C3 (now realized) to C2, where FTD-0787's
mode falls short by a factor of 3.07 — a scale separation between matter
binding and field stiffness, not a defect of shape.
