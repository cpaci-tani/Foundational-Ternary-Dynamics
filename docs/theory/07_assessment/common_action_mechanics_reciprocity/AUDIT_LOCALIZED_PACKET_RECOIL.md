# AUDIT — Localized-packet recoil gate

**Date:** 2026-07-24  
**Identifier:** `FTD-0457`  
**Status:** `[CONSTRUCTIVE EXISTENCE — FINITE-ENERGY LOCAL PACKET]` + `[MEASURED — BOX-STABLE R=1 THRESHOLD]` + `[OPEN — PRODUCTION SELECTION]`  
**Verdict:** `LOCALIZED_PACKET_R1_THRESHOLD_VOLUME_STABLE`  
**Pre-registration:** [`PREREG_LOCALIZED_PACKET_RECOIL_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_LOCALIZED_PACKET_RECOIL_v1.md)  
**Run of record:** `engine/results/ftd_0457/windows_msvc_cpu.csv`

## 1. The extensive-background defect does not survive localization

FTD-0456 found a 36-site zero-energy recoil response in a box-wide `n=1` mode,
but the local threshold energy grew approximately with volume. FTD-0457 replaces
that mode with the canonical finite discrete-curl packet used by the qualified
scenario layer and evolves it under the exact source-free production wave map.

All 18 registered `(L,direction,sample tick)` arms bracket a zero-energy `R=1`
transaction. Selecting the lowest threshold energy within each preregistered
three-tick family gives:

| `L` | packet `-x`: tick, energy | packet `+x`: tick, energy |
|---:|---:|---:|
| 33 | `8`, `5.039397e-4` | `16`, `5.582372e-4` |
| 49 | `8`, `5.039397e-4` | `16`, `5.582374e-4` |
| 65 | `8`, `5.039397e-4` | `16`, `5.582374e-4` |

The `L=49` and `L=65` means agree exactly at the printed precision. Unit packet
energy has coefficient of variation `2.62e-15`; selected threshold energy CV is
`5.11%`, entirely from propagation direction rather than volume; the registered
high-volume relative difference is zero.

The correct conclusion is therefore stronger than FTD-0456 but still
conditional: a finite-energy localized native flux packet has enough local
phase-space structure to carry an exact energy-and-momentum-conserving face-hop
transaction without consulting the rest of the lattice.

## 2. The packet is a valid source-free native field excitation

The packet is a centered discrete curl, so its measured maximum divergence is
`2.78e-17`. Its exact modified tick energy drifts by at most `1.42e-12` over 16
forward ticks, and 16 analytic inverse ticks recover `J/W` to `5.57e-16`.

At the event threshold:

- worst complete particle + interaction + wave energy residual: `1.49e-15`;
- worst field-momentum residual: `6.94e-17`;
- impulse outside the 36-site support: exactly zero by construction;
- after eight source-free ticks, `74.99–82.74%` of the event-control difference
  norm lies outside the original support;
- reversing those ticks and undoing the impulse recovers the pre-event field to
  `7.40e-17`.

The event perturbation is therefore neither erased nor permanently trapped in
the hop neighborhood. It becomes an outgoing reversible field disturbance.

## 3. Ontological extrapolation

The most economical story now consistent with the measured construction is:

1. A finite divergence-free `J/W` packet is the propagating dispositional
   object.
2. A ternary manifestation is a localized event or marker coupled to that
   packet, not the whole packet itself.
3. As the packet reaches an oriented link, its local gradient and conjugate
   phase change the minimum energy cost of transferring the manifestation.
4. When a zero-cost transaction is available, the manifestation can hop while
   the same 36-site event conserves total energy and field momentum.
5. The recoil then leaves as a reversible wave disturbance, giving the field a
   physical memory of the manifestation event.

This is a mathematically coherent pilot-field candidate. It is not yet native
dynamics because the campaign asks an optimizer whether the transaction exists;
the production tick does not ask or execute that question.

## 4. The selection problem is narrower than it first appeared

Let the supported impulse be `S`, the three momentum constraints be `A S=p`,
and the complete event energy be

```text
E(S) = 1/2 ||S||^2 + c.S.
```

The constrained minimum `S_min` is unique. If `E_min<0`, the zero-energy
solutions form a sphere in `ker(A)`:

```text
S = S_min + z,    z in ker(A),    ||z||^2 = -2 E_min.
```

For 36 sites there are 108 components; after three independent momentum
constraints and one energy condition the generic zero-energy family has
dimension 104. But one additional local variational statement removes it:
choose the zero-energy solution with minimum impulse norm. Its unique null-space
direction is opposite the null projection of `S_min`, equivalently along the
projected coefficient `P_ker(A)c`. That is exactly the covariant-null direction
already used by the constructive solver.

So the missing ingredient is no longer an arbitrary 104-parameter choice. It is
one explicit candidate rule:

```text
Among local impulses that conserve the required momentum and total energy,
execute the one with minimum ||S||.
```

This rule remains `[SELECTION]`; it is not contained in the five postulates or
the production tick.

## 5. Intuitive questions exposed by the result

1. Is “a particle moves” actually the statement that a packet makes one local
   zero-cost manifestation transaction available after another?
2. Is mass the amount of local packet structure required before such a
   transaction becomes possible?
3. Does inertia mean the next minimum-norm transaction usually lies along the
   incoming packet momentum?
4. Does interference steer manifestation because superposed packets reshape the
   local transaction-cost surface before any hop occurs?
5. Is measurement the irreversible part introduced only when genesis,
   evaporation, or other many-to-one state rules act, while the underlying
   packet-plus-hop sector can remain reversible?
6. Does electric polarity merely change the sign of the interaction term `c.S`,
   causing opposite manifestations to choose opposite zero-cost routes?
7. Can a stationary massive state be a circulating packet whose available
   minimum-norm transactions close into a loop rather than translate?
8. Does the outgoing recoil disturbance become the next piece of the pilot
   field, making motion self-consistent rather than externally guided?

## 6. Next decisive gate

Formalize the minimum-norm zero-energy transaction and test it without amplitude
bisection at the event layer. The next campaign must determine whether a
pre-event local cost functional can:

- select a unique impulse at arbitrary negative `E_min`;
- commute with all cubic rotations/reflections;
- reverse independently with the packet and hop;
- produce consecutive hops guided by the evolved outgoing field;
- handle face, edge, and corner channels without the hidden x/y/z routing defect
  identified by FTD-0445/0446.

Only after those gates pass should the rule be proposed as a default-off engine
toggle. The frozen production tick remains unchanged here.

## 7. Scope and reproducibility

The result covers one packet width, one carrier, one transverse lobe offset, one
face-hop orientation, two propagation directions, three sample ticks, and three
volumes. It establishes neither universality across packets nor spontaneous
manifestation. No external constant was targeted.

- campaign SHA256: `4D97BFB60875CE476B9DA1C3C59C851FFA7596362ABE9B9690D1EA0BD46AD026`
- packet helper SHA256: `8AA0E4DBE189D2EADD277F43A5E7652D8459663F463B0B7654CA623CF02F64BA`
- record SHA256: `BCC8D64935B4D44FAB760049E2F0E12C3F75DA6D7F7B3819024BCA2B340655A3`
- compiler: pinned MSVC `14.44.35207`, Release

