# Retirement Record — Runtime Scale Handoff

**Status:** `[RETIRED — REMOVED]`
**Date:** 2026-09-01
**Replacement:** Scale Context sidepanel for pedagogical scale comparison

## Removed surface

- the Scale-0 toolbar action **Project to Scale 1**
- the browser capture/promotion pipeline and worker coarse-graining message
- the native `Scale1Projector`, projection-loss ledger, and WASM bindings
- the promotion, source-voxel, and mass-comparison overlays
- the projection and Scale-1/Scale-2 handoff scenarios
- the generic Scale-0/Scale-1, Scale-1/Scale-2, and Scale-2/Scale-5
  coarsen/refine utilities
- all dedicated bridge tests and cross-scale campaigns

These paths used `[IMPOSED]`, lossy mappings. They did not preserve a
state-complete provenance chain and were not evidence that a higher-scale
entity emerged from the Scale-0 substrate.

## Retained boundaries

- `NativeMatterObserver` remains a read-only scientific observer. It can
  inspect a registered/coherent source record but cannot create a
  `ParticleEngine` body.
- `ParticleEngine`, `AtomEngine`, and `CosmicEngine` remain independent
  effective/reference engines seeded by their own explicit scenarios.
- `ScaleEngine`, `ScaleLevel`, and the compact `OnticEntity` presentation
  summary remain shared interfaces; none performs state transfer.
- The Scale Context sidepanel is the sole pedagogical scale-comparison tool.
  It changes presentation only and creates no cross-scale simulation state.

No replacement runtime scale handoff is planned.
