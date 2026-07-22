# ANALYSIS — Full-state irreversibility of the current engine map (FTD-0395)

**Verdict:** **FULL-NONINJECTIVE**.  
**Tag earned:** `[THEOREM — current engine update map on the public-API-admissible domain]`.  
**Lock:** `PREREG_FULL_STATE_IRREVERSIBILITY_v1.md`, tag `preregister-full-state-irreversibility-v1`, lock commit `30bf2216`, prereg SHA256 `cad088896cdc6854ffa6e9fdc70d28b5c5a4bd7424006d5ab4f8d03a64ec82e4`.  
**No tag change:** FC-2 remains `[AXIOM]`; FTD-0253 and FTD-0394 keep their existing grades.

## 1. What was proved

Let `F` be the complete public `RenderBridge::tick()` update on the current engine. Two admissible states were constructed using only `inject_particle`. They were identical except for the center particle's `(spin,color)` labels: `(+1,1)` versus `(-1,3)`. Both used seed `20260422`, `L=8`, identical particle IDs/counters, and only the evaporation phase enabled.

The pre-tick states were exactly different under the enumerating comparator. On the first full tick both particles evaporated. The comparator then found every persistent voxel field and every locked public global/audit field bit-identical. Sixteen additional full ticks remained bit-identical. Therefore there exist `X != Y` with `F(X)=F(Y)`, so the current engine update map is non-injective on this public-API-admissible domain.

This is an existence theorem about the implemented map, not a theorem that every manifestation loses information and not a theorem about all future engine rules.

## 2. Gate record

| Gate | Result |
|---|---|
| Census GREEN before lock and after tag | PASS |
| WSL2 canonical build; `force_cpu()` effective; `FTD_FORCE_GPU` unset | PASS |
| Prestate differs exactly in spin/color | PASS |
| Both arms evaporate on full tick 1; one event each | PASS |
| Complete post-tick state bit-identical | PASS |
| Sixteen-tick tail bit-identical | PASS |
| Evaporation-off control retains the label difference | PASS |
| FTD-0394 readout target: same readout, distinct `J` magnitudes | PASS |
| Duplicate executions bit-identical in target output | PASS |

The build logs print `GPU backend active` during each constructor because this CUDA build initially selects the GPU backend. The test immediately calls `force_cpu()` before injection or dynamics; executable gate G2 checks the effective backend and passed in every run.

## 3. The three-map split

1. `R: J -> (state,color,spin)` is non-injective by finite-readout cardinality and by FTD-0394's engine measurement. FTD-0394's three `J` magnitudes remain distinct after genesis; it is a readout collision, not a full-state collision.
2. Genesis does not erase `J` in engine memory. The earlier phrase “manifestation destroys all information” was therefore too broad.
3. Evaporation supplies a different witness: it clears state, particle identity, spin, and color while retaining otherwise identical field data. When the only input distinction is in the cleared labels, the complete public future collides exactly.

Observer histories are sequences of readouts. They can fail to distinguish histories without proving `F` non-injective; the present result avoids that inference by comparing complete persistent state.

## 4. Provenance and reproduction

- Git SHA at execution: `30bf22163c4fbd60e51605044e9e0e6ebf10f14c`.
- Platform: Ubuntu 22.04 under WSL2, Linux `6.6.87.2-microsoft-standard-WSL2`, x86_64.
- Compiler: GCC `11.4.0`; CUDA toolkit `13.0.88` present, but dynamics CPU-forced.
- Instrument SHA256: `45ec650a3a7ef62b1ba4fa9fd6833b6102ddc81f364d10e0c8f23cbdb9445674`.
- Instrument binary SHA256: `bf83a1d7c12333b54b03ffda811bd46b54b16cd937c9b578d4988e7a4dc31301`.
- FTD-0394 gate binary SHA256: `19125c501d8cf5c66311b477d42b89c4b788d72aad7a094dc79d3afe4b9abe0b`.
- Effective toggles: all disabled except `evaporation=true`; default periodic flux boundary; seed `20260422`; `dt=1`; SOR default (unused).
- Commands:

```sh
unset FTD_FORCE_GPU
engine/build_wsl/test_full_state_irreversibility
engine/build_wsl/campaign_manifestation_readout_collision
```

Each command was executed twice through an in-memory `cmp` gate; both reported `DUPLICATE_IDENTICAL=1`. Raw records are in `engine/results/full_state_irreversibility_2026-07-20/`.

## 5. Consequence

Canonical arrow prose should use the exact statement: **genesis readout is lossy while retaining continuous flux; full-map non-injectivity is separately witnessed by evaporation.** This evidence supports the internal consistency of FC-2 but does not derive or promote that commitment.
