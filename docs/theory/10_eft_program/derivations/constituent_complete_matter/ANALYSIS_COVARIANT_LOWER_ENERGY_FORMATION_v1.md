# FTD-0726 — Covariant lower-energy formation v1

**Status:** `[SELECTED DYNAMICS + MEASURED — COVARIANT ENERGETIC
TRAPPING / DETACHED-FIELD CAPTURE CLOSED NEGATIVE]`  
**Verdict:** `COVARIANT_ENERGETIC_TRAPPING_WITHOUT_DETACHED_FIELD`  
**Production status:** unchanged

## Result

The fresh FTD-0726 full-matrix replay closes the numerical qualification that
FTD-0724 failed. With the mathematical action and physical initial states
unchanged, but with the FTD-0725 tight exact-root termination, all 312
registered complete histories pass common-action, energy, recoil, inverse,
translation, and polarity gates.

Every unbound arm at `p=0.0060`, `0.0075`, `0.0085`, and `0.0095` enters the
derived interaction graph once and remains inside it with negative pair
internal energy through tick 48. Every `p=0.0120` arm enters and leaves the
graph with positive final energy. All already-bound controls remain bound.

```text
executed histories                         312/312
unbound histories                         260
negative at tick 48                        208/260
qualified detached-field captures           0/260
bound controls retained                     52/52
maximum common-action residual             1.9915e-12
maximum symmetric-recoil residual          6.3971e-14
maximum state-only inverse residual         1.7688e-10
maximum pair-plus-field balance residual    9.7129e-13
maximum scalar-history covariance spread    8.9041e-10
```

## Energy and topology classes

The registered classes are direction- and polarity-independent:

| incident momentum | negative arms | graph transitions per arm | final graph state | doubled dynamic-field radius |
|---:|---:|---:|---|---:|
| `0.0060` | `52/52` | 1 | inside | 3 |
| `0.0075` | `52/52` | 1 | inside | 3 |
| `0.0085` | `52/52` | 1 | inside | 3 |
| `0.0095` | `52/52` | 1 | inside | 3 |
| `0.0120` | `0/52` | 2 | outside | 5 |

The low-momentum families export approximately `5.44e-4--6.84e-4` to the
field and finish with pair internal energy between `-6.03e-4` and
`-3.92e-4`. The `p=0.0120` family exports only
`2.72e-4--2.80e-4` and exits with small positive energy.

## Ontological interpretation

The result establishes a covariant, reversible, energy-balanced trapping
transaction using existing constituent phase space and face/edge field state.
It does not establish a particle or stable matter object. The locked capture
definition additionally required the receiver field to detach to doubled
radius at least four; none of the trapped histories does so.

That failure does not imply absence of a receiver. In a coupled matter-field
object, the energy-receiving field may remain localized as a bound dressing
rather than separate as radiation. The observed radius-three field is
therefore ambiguous between:

1. a persistent bound part of one complete localized object; and
2. temporary near-field storage that later returns energy and releases the
   pair.

The next admissible discriminator is temporal persistence before any periodic
boundary recurrence can return emitted field. Detached radiation is one
formation route, not a necessary condition for bound dressing.

## What is proved and what remains open

FTD-0726 proves numerical existence of the registered finite-volume complete
histories and qualifies their 48-tick energetic-trapping class. It does not
prove global root uniqueness, asymptotic stability, a particle pole, mass,
spin, statistics, or infrared Lorentz recovery. It does not promote the
selected compact pair potential to native production dynamics.

No new primitive is indicated by this gate. A state-completeness extension is
priced only if the existing complete state cannot decide or invert the next
transaction. Here it does both. The next price is a longer clean-horizon
persistence test, not explicit bond, history, or connection state.

## Verification anchors

- preregistration SHA-256:
  `8C484A05DC94F4099687757660F6D0873E614A7D55FAE40637539BECEFF4A335`;
- runner SHA-256:
  `1C13A6DDF707C46C0262B0CBED84F5C961BE89651D356A35240B2C5D5EA499FC`;
- result JSON SHA-256:
  `FE73C4FBCBB3D1FB796D0BB2A758FF8EC3A867915A1711E91713C7FC407D697D`;
- result CSV SHA-256:
  `8A428E7F8E248A64E3287278E5E0BDE75EB82ED409A22D1D54DFEDFE2F993146`;
- independent certificate:
  `410D3CFDAA021301A71E42817E006E4D87704C44AEC1D672CD3E8CDA5DFF2BB7`,
  `138/138 PASS`.

