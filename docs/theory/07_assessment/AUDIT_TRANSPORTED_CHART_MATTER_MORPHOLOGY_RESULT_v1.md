# FTD-0764 — Transported-chart matter morphology result v1

**Status:** `[CERTIFIED MIXED NUMERICAL FACT — NO RIGID TRANSPORTED NEAR-FIELD COHERENCE; TRAILING LAG; MOTION-INDUCED RADIATION NOT ESTABLISHED; MOMENTUM OPEN]`  
**Date:** 2026-07-31  
**Protocol SHA-256:** `4F68CCD8A037363438CF94B728C56059066BFA9B2B3D8C0F82A6A5DDF3D7BDF8`

## Verdict

The WSL2 RTX 5090 qualification and all three registered `L=321` replays
execute successfully. The independent frozen-artifact certificate reports:

```text
FTD-0764 artifact certificate: 1524/1524 checks
morphology_verdict=NO_TRANSPORTED_FIELD_COHERENCE
registered_detached_outgoing_component=true
motion_induced_radiation_established=false
trailing_wake_candidate=true
momentum_candidate_closes=false
```

**FTD-0765 interpretation correction:** the preregistered trailing-moment bit
is exactly a core--residual centroid-lag observable after rest subtraction.
It does not independently establish creation of a new wake. FTD-0765 measures
only `-1.1%/15.7%/16.9%` residual-centroid entrainment on face/edge/body. The
historical registered bit is retained, but its correct reading is
`TRAILING_LAG`; wake creation remains open.

The moving two-constituent core is not accompanied by a rigidly translated
actual near-field morphology under the registered observer. The actual field
reorganizes as the core moves. A negative longitudinal residual moment grows
behind the core on every ray and is absent in the rest controls, establishing
separation from a mostly unentrained residual environment. The registered detached
outgoing-component predicate also fires, but the same far signal occurs in
the unboosted controls. It is a preparation/formation transient under this
protocol, not evidence for radiation caused by motion.

Neither pre-existing field-momentum candidate closes the matter-plus-field
ledger. No missing substrate momentum is defined by the defect.

## CUDA qualification and execution

The `L=17,33` qualification passes with zero failures. It covers both
polarities, face/edge/body geometries, CPU/CUDA parity, exact coefficient
reconstruction, zero-mode energy, integer translation, proper cubic rotation,
polarity conjugation, repeated-state identity, and scalar-only GPU transfers.

The registered replay uses the unchanged FTD-0761/0763 parent, `L=321`,
`q=0.015`, ticks `160,176,192,208,224`, and separate rest/plus arms. Every
checkpoint passes the common-action, energy, causal-speed, fractional
observer, boundary-ledger, support-ladder, and one-step inverse gates.

| ray | max Gauss residual | max mode reconstruction | max common-action residual | max energy residual | max inverse residual |
|---|---:|---:|---:|---:|---:|
| face | `7.334e-14` | `2.614e-17` | `4.550e-14` | `3.427e-15` | `6.936e-15` |
| edge | `1.063e-13` | `1.887e-17` | `5.107e-14` | `2.649e-15` | `2.994e-14` |
| body | `1.116e-13` | `1.915e-17` | `5.371e-14` | `1.729e-15` | `1.314e-14` |

The three CUDA regression targets
`cuda_state_only_support_ladder`,
`cuda_fractional_center_state_only_observer`, and
`cuda_transported_chart_morphology` pass 3/3.

## Transported morphology

The final registered values are:

| ray | near morphology distance | near energy ratio | bound-control distance | locked verdict |
|---|---:|---:|---:|---|
| face | `0.1800827041` | `1.1015000324` | `0.0317019926` | `NO_TRANSPORTED_FIELD_COHERENCE` |
| edge | `0.1805640340` | `1.2965055627` | `0.0417255879` | `NO_TRANSPORTED_FIELD_COHERENCE` |
| body | `0.2360204065` | `1.5736770744` | `0.0516821400` | `NO_TRANSPORTED_FIELD_COHERENCE` |

The locked near-distance threshold is `0.10`, the energy band is
`[0.8,1.2]`, and the bound-control threshold is `0.02`. Every ray fails the
conjunction. This result does not say that no near field exists. It says that
the registered actual near residual is not a rigidly translated copy of its
tick-160 morphology after removal of only the measured center phase.

The bound control also fails. The selected site-based fractional shape is
piecewise covariant under integer translations but does not supply exact
continuous subcell translation covariance. Therefore the result closes the
rigid-cloud interpretation for this observer/dynamics family, not all possible
relational definitions of material dressing.

## Far transient control

The preregistered outgoing predicate requires monotonically growing outer
residual energy and positive radius-48 signed radial flux at ticks
`192,208,224`. It passes. The repaired artifact serialization exposes the
corresponding rest-arm evidence:

| ray | outer energy rest, tick 224 | outer energy plus, tick 224 | radius-48 flux rest | radius-48 flux plus |
|---|---:|---:|---:|---:|
| face | `0.02876890336` | `0.02876953743` | `5.96275798e-18` | `5.96255332e-18` |
| edge | `0.02298778310` | `0.02298967791` | `6.02232721e-18` | `6.02216624e-18` |
| body | `0.01721703656` | `0.01722613514` | `6.03720418e-18` | `6.03704446e-18` |

The final rest/plus flux mismatch is only `2.65e-5--3.43e-5` relative. The
outgoing signal is therefore not specific to motion. The registered Boolean
is retained as a numerical fact about a detached outgoing component, while
“motion-induced radiation” remains unestablished.

The first large-volume execution omitted shell evidence from JSON even though
the in-memory classifier used it. That incomplete artifact was not certified.
Observer-only serialization was repaired, the runner rebuilt, and all three
arms rerun. The scientific metrics reproduce the first execution exactly;
the final hashes below identify only the auditable artifacts.

## Wake and momentum

The final combined longitudinal residual moments are:

| ray | rest magnitude, tick 224 | moving moment, tick 224 | local momentum defect | spline momentum defect |
|---|---:|---:|---:|---:|
| face | `<2e-13` | `-0.4085564371` | `0.0072757002` | `0.0071987043` |
| edge | `<2e-13` | `-0.3374194102` | `0.0089012532` | `0.0088270653` |
| body | `<2e-13` | `-0.3454299525` | `0.0069239361` | `0.0069661751` |

The moving moments are negative at every moved checkpoint and increase in
magnitude on all three registered intervals. This satisfies the historical
`TRAILING_WAKE_CANDIDATE` predicate. FTD-0765 proves that the quantity is the
centroid lag by construction, so the physical conclusion is only
`TRAILING_LAG`; it does not establish a newly created wake, material dressing,
lattice drag, or another specific response mechanism.

Both momentum defects exceed the locked `1e-9` closure gate by more than six
orders of magnitude. Exact local energy exchange and state-only one-step
inversion do not imply conservation of either tested translation-momentum
candidate.

## Ontological consequence

The strongest supported account at this stage is:

1. the manifested two-constituent core is a mobile relational pattern;
2. the constraint/bound field is a state-dependent relation, not a rigid
   material shell transported continuously between sites;
3. the actual dynamical field reorganizes and develops a motion-correlated
   trailing asymmetry;
4. the observed far transient belongs to preparation history at this horizon;
5. the correct total translation generator is still missing or incorrectly
   represented by the two tested field candidates.

This does not force a new ontic primitive. It does close the rigid-aura and
motion-induced-radiation readings for the registered family. The next test
must age/subtract the formation transient, sweep velocity and preparation age,
and ask whether the wake scales with motion while a face/connection momentum
ledger closes. Failure of that ledger across width and horizon would be the
proper trigger for escalation to explicit constituent phase space or a
connection-based electromagnetic ontology.

## Frozen identities

| item | SHA-256 |
|---|---|
| derivation | `F682BC9B1E32201EDE92BDBD788E303A4706B723486F59DE2749EB0872E4D72E` |
| protocol | `4F68CCD8A037363438CF94B728C56059066BFA9B2B3D8C0F82A6A5DDF3D7BDF8` |
| CUDA qualification source | `B3240BE5CC364435F67312C9A0D572B7396719EC8626965C11A8BF780951CF7A` |
| CUDA runner source | `F31C6466D89244B1F4D902029C9CB2611A4056C87F539B473D74946DE2798EE3` |
| CPU observer header | `0C47656A6F30186C459957644B01E9E75FBE993ADF9EAB38D748D7EF58B86626` |
| CPU observer source | `4AFF5394FD2AFA73972C24998A582B8383E0D46D02FF4DBD19D958CFFC314CC9` |
| CUDA observer source | `80C969D41410F2077DAADF0F864A9AE2F91761234FB0F372907340448241AC53` |
| CUDA runner executable | `7432F1D86FFDF96C9CBEF619FDA8F64BA838D582B9BCEA6445746D8BA88359C9` |
| certificate | `D54B8AC707912C8D48D355FFF78CE0E9D51D3253B185B4CDDFB38818B5FEDDB0` |
| aggregate JSON | `9769BE1330CF422FDC10CE77CF89057153E634FDC3E054522C58F2CC0145AA56` |
| face JSON | `0DDB53E3A138AF564EB3FF09F6D052D0120E9A3D74D5B132850D86990FB1017C` |
| edge JSON | `762E5C50315C2564070CDBA0009635673EA4519F5810692418996F46A38C7AA4` |
| body JSON | `43E2182CCE16BDC356A6FC2BB9A275343F9633571DA45AEC2C7CEA76E4412DF5` |

The run-of-record directory is `engine/results/ftd_0764/`.

Production dynamics, defaults, ontology primitives, toggles, scenarios, and
`RenderBridge` remain unchanged.
