# FTD-0722 — Field-assisted derived-pair capture v1

**Status:** `[SELECTED DYNAMICS + CONDITIONAL COMMON-ACTION IDENTITIES +
MEASURED — LOCKED V1 CAPTURE CLOSED NEGATIVE]`  
**Verdict:** `FIELD_ASSISTED_CAPTURE_NOT_OBSERVED_LOCKED_V1`  
**Production status:** unchanged

## Result

The existing constituent phase space and matched face-electric/edge-magnetic
variables support one reversible atomic encounter in which motion, exact
quadratic-coat current, field update, pair impulse, field recoil, and energy
exchange are solved together. A stored bond bit is not used. All 104 locked
histories execute and pass the common-action, inverse, translation/polarity,
and symmetric-recoil gates.

That is not yet formation. Every initially unbound arm enters the compact
interaction graph, remains there for 11 ticks, and exits. None reaches the
negative pair-internal-energy sector. The locked v1 capture claim is therefore
closed negative.

## Frozen transaction

The matter state contains two opposite-polarity constituent records with
nearest-site anchors, continuous remainders, and momenta. The graph is the
instantaneous relation

\[
(1,2)\in G(X)\iff |x_1-x_2|^2<3/2,
\]

and the pair interaction is the selected FTD-0721 compact well

\[
U(d)=-16(10^{-2})(d-3/2)^2(d-3/4)
\]

inside the support and zero outside. The complete field step is the already
qualified matched face/edge action. Each constituent deposits its exact
straight quadratic-coat current; the six later momenta and both later
positions are solved simultaneously with pair and field impulses. No damping,
post-hoc correction, Poisson force, separately imposed Lorentz force, or graph
edit is present.

## Locked campaign

The campaign used `L=33`, `dt=1/4`, 24 forward and 24 reverse steps, all 13
unoriented Moore rays, both polarity orders, and two translated copies.

```text
complete histories                         104 / 104
common-action identity arms                104 / 104
state-only inverse arms                    104 / 104
symmetric recoil arms                      104 / 104
unbound capture arms                         0 / 52
already-bound controls retained             52 / 52
maximum common residual                    1.754e-11
maximum pair/field energy-balance residual 5.184e-12
maximum recoil defect                      2.845e-13
maximum 24-step inverse recovery           3.349e-10
translation/polarity scalar-history spread 2.983e-11
```

The unbound family begins with pair internal energy
`0.00945775566218`. After one encounter it ends in
`[0.00808294303224, 0.00825648519647]`. The field therefore receives
`[0.00120127046572, 0.00137481262995]`, or about `12.70%--14.54%` of the
incoming pair internal energy, with pair-plus-field balance below
`5.19e-12`.

This transfer is dynamically real but insufficient for capture. Every
unbound arm changes graph membership twice and exits with positive energy.
The dynamic field is nonzero: scattering-arm difference-field norm lies in
`[4.83e-4,5.48e-4]` and magnetic energy in
`[4.53e-4,4.96e-4]`. Its median doubled radius is three, below the locked
outgoing-field threshold four, so the run does not qualify detached radiation
either.

## Ontological consequence

FTD-0722 separates sufficiency of variables from sufficiency of a formation
mechanism:

1. the current constituent and face/edge variables can carry a reciprocal,
   energy-routing encounter without a stored edge or new reservoir primitive;
2. a dynamic magnetic/field remainder forms and receives mechanical energy;
3. the selected v1 action does not export enough energy in one `p=0.07`
   encounter to cross into the negative-energy sector.

The failure therefore does not price a new primitive. It prices a capture
threshold or a missing interaction channel. The next admissible test is a
fresh preregistered incident-energy window derived from this ledger, including
held-out momenta and an outgoing-field discriminator. Changing the locked
momentum or well after this result would be post-hoc repair and is not allowed.

## Scope

The compact well remains selected. The result does not establish a physical
particle, electromagnetism, photon emission, an infinite-volume capture cross
section, count-changing reaction, or quantum stability. It closes only the
registered `L=33`, 24-tick, `p=0.07` field-assisted capture candidate. The
already-bound controls show that failure is not simply destruction of the
negative-energy sector.
