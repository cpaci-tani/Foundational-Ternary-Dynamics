# AUDIT — Link-action work compatibility

**Identifier:** `FTD-0470`  
**Date run:** 2026-07-25  
**Status:** `[THEOREM — EXACT FINITE-LINK INTERACTION WORK]` +
`[THEOREM — SITE-GRADIENT IR SYMBOL]` +
`[MEASURED — EVOLVING NATIVE HISTORIES]` +
`[CLOSED NEGATIVE — CENTERED SITE FORCE AS EXACT HOP LAW]` +
`[OPEN — LINK RECOIL/SOURCE TRANSPORT]`  
**Pre-registration:** [`PREREG_LINK_ACTION_WORK_COMPATIBILITY_v1.md`](../10_eft_program/preregistrations/PREREG_LINK_ACTION_WORK_COMPATIBILITY_v1.md)  
**Run of record:** `engine/results/ftd_0470/windows_msvc_cpu.csv`

## Verdict

`SITE_GRADIENT_IS_IR_APPROXIMATION_LINK_DIFFERENCE_IS_EXACT`

The written interaction fixes finite-hop work by an endpoint difference. A
centered site-gradient force reproduces that work only for restricted field
profiles and in the long-wavelength approximation. It is not the exact
matter-side derivative for an integer event.

## Exact finite-link law

For

```text
H_int(s,J) = -G_C sum_x s(x) div(J)(x),
```

moving charge `q` from site `a` to a face neighbour `b` changes the
interaction by

```text
Delta H_int = -G_C q[div(J)(b)-div(J)(a)].
```

The corresponding particle work is therefore

```text
W_hop = G_C q[div(J)(b)-div(J)(a)].
```

For face displacement `d=b-a`, the longitudinal link impulse
`I_link=W_hop d` satisfies `I_link.d=W_hop` identically. The run recorded
zero link-work residual in all 1644 static and dynamic rows.

This identity fixes a scalar longitudinal event response. It does not yet
prove a field recoil, decide when the hop occurs, or solve edge/corner path
ambiguity.

## Why the site force differs

The source-centered continuous candidate uses

```text
I_site = (G_C q/2)[grad divJ(a)+grad divJ(b)].
```

Its work is `W_site=I_site.d`. The polynomial fixtures give:

| `div(J)` along link | `W_site/W_hop` | Result |
|---|---:|---|
| affine | `1` | exact |
| quadratic | `1` | exact |
| cubic at registered source | `5/2` | defect `(3/2)W_hop` |

All affine/quadratic residuals were zero. The cubic formula residual was at
most `2.71e-20`.

For the periodic Fourier mode `div(J)=A sin(kx+phi)`, direct discrete
calculation gives

```text
W_site/W_hop = cos^2(k/2),
(W_hop-W_site)/W_hop = sin^2(k/2).
```

The six registered modes reproduced that symbol with worst absolute formula
residual `1.84e-18`:

| mode on `L=32` | relative work defect |
|---:|---:|
| 1 | `0.00960736` |
| 2 | `0.0380602` |
| 4 | `0.146447` |
| 8 | `0.5` |
| 12 | `0.853553` |
| 15 | `0.990393` |

Thus the centered site force converges to the link law with an
`O((ka)^2)` correction in the infrared. Exact finite-lattice energy and
emergent low-energy force are different requirements.

## Evolving native histories

Six `L=33` wave-plus-coupling arms supplied 1536 counterfactual face-link
measurements from locked opposite-polarity pairs. Every arm was nontrivial.

- exact hop-work RMS: `0.00854929` to `0.00855296`;
- centered-force defect RMS: `0.00588656` to `0.00588668`;
- maximum sampled relative defect: `0.751452`;
- accidental centered closures at `1e-12`: `0/1536`;
- exact-link residual: `0`.

The large mismatch is not a fitted comparison. It follows from applying the
two preregistered discrete derivatives to the same evolving `J` histories.
The stationary polarity source carries finite-lattice/high-momentum content,
so the infrared approximation is not accurate on these local event probes.

## Consequence for the ontology

FTD can keep the central site gradient as a low-energy smooth-force observer.
It cannot use the same expression as the exact work law for an integer hop.
Event-native mechanics needs an oriented link derivative of the interaction.
That is compatible with FTD-0447's cubic-stabilizer result for a face event:
the unique isolated longitudinal response is `W_hop d`.

This does not authorize replacing the production force. The production tick
contains continuous remainder acceleration, whereas this audit concerns an
actual change of the site-valued source. The two may coexist only if their
domains and energy accounting are separated explicitly.

## Next gate

For a source-centered pre/post kick, the field recoil corresponds to the
centered site impulse, not generally to `I_link`. The next test must determine
whether conservative source transport supplies the missing momentum

```text
Delta I = I_link - I_site
```

without adding energy, losing injectivity, or choosing a nonlocal correction.
If it does not, the written interaction plus present `J/W` update still lacks
one composite hop transaction even though both its smooth-force and exact-work
limits are now known.

## Reproducibility

- campaign SHA-256:
  `CDCD31B8EC37B6D997C8655945A7AC2F69E63CC6C2396A902E7590AE3FFC76A5`
- helper SHA-256:
  `47D10DDFD14AF34BCDD5EF02C11DCE96713818717A0430E49712787058643069`
- toolchain: MSVC `14.44.35207`, Release, CPU forced after bridge construction
- production dynamics: unchanged
