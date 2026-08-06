# FTD-0727 — Bound-dressing persistence v1

**Status:** `[EXECUTION UNRESOLVED — RAW 96-TICK TRAPPING AND FIELD
EXTENSION NON-PROMOTABLE]`  
**Verdict:** `BOUND_DRESSING_PERSISTENCE_TRANSACTION_UNRESOLVED`  
**Production status:** unchanged

## Result

All 208 registered 96-forward/96-reverse histories execute and pass their
rowwise common-action, recoil, energy, and state-only inverse gates. The
campaign nevertheless fails its locked global covariance gate:

```text
maximum scalar-history covariance spread   1.1065308669344631e-9
locked gate                                1.0e-9
```

The physical histories also invalidate the clean escape control. Every
`p=0.0120` arm has three graph transitions—entry, exit, then re-entry—rather
than remaining outside after tick 48. Twelve of 52 finish in the negative
sector. The locked escape-control pass count is therefore zero.

The two parent trapped families remain raw-persistent:

| momentum | tail-persistent | graph transitions | radius at tick 48 | radius at tick 96 | final pair energy |
|---:|---:|---:|---:|---:|---:|
| `0.0060` | `52/52` | 1 per arm | 3 | 5--6 | `-1.380e-3` to `-1.076e-3` |
| `0.0095` | `52/52` | 1 per arm | 3 | 5 | `-1.406e-3` to `-1.088e-3` |
| `0.0120` | `0/52` | 3 per arm | 5 | 3--12 | mixed |

All 52 pre-bound controls remain persistent and radius two.

## Interpretation

The radius-three field observed at tick 48 does not remain compact under the
locked classifier. By tick 96 it has expanded to radius five or six in every
raw trapped parent arm while the pair remains negative and connected. The
data are therefore compatible with a bound core plus an outgoing field tail,
not with the stronger compact-dressing verdict.

That compatibility is non-promotable because two gates fail. First, the
translation/polarity scalar spread is above tolerance. Second, the nominal
escaping family re-enters in every direction, so the finite-volume history
does not provide a clean scattering control.

The result does not show that the extended field causes the re-entry or that
the re-entry is a periodic-boundary artifact. The initial dress is periodic,
and the exact stencil support can grow by one site per tick; `L=33`, 96 ticks
is not a pre-wrap experiment.

## Ontological consequence

The complete constituent-plus-field state continues to determine and invert
the transaction. No history, bond, or connection primitive is priced. The
new candidate structure is:

> a negative-energy relational core accompanied by a dynamical field that can
> develop an extended or outgoing component while the core remains trapped.

This resembles localized matter plus radiation more than a rigid compact
aura, but it is not yet stable matter. A fresh numerical-conditioning
diagnostic must first close or confirm the small covariance miss. A separate
volume campaign must then decide whether `p=0.0120` re-entry and low-energy
persistence are local dynamics or finite-volume recurrence.

## Verification anchors

- preregistration SHA-256:
  `49941B346EFF02394C381E6661D47E1A519FD333A087A88A25825D817457312F`;
- runner SHA-256:
  `5640D53F956D8E3B9610A9B2269F16A4DAAFF79CDE436252734BA5EE3F085B68`;
- result JSON SHA-256:
  `52C9537C4E40AD33683070081EA1D4160BFF3C1101AF9237FC36B4EBA95E3F95`;
- result CSV SHA-256:
  `007E3FC1E0E3E7FA3E034E0308B028E14BF76E6503BFAC0B547D430542B76668`;
- independent certificate:
  `7D25D32205B644B071A27C66772E10B1FD2249086104FA805F5C00CFBFC01243`,
  `93/93 PASS`.

