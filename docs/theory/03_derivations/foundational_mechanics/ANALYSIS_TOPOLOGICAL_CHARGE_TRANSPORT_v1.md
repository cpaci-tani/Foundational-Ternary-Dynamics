# ANALYSIS — Terminal topological-charge transport test (FTD-0398)

**Status:** `[MEASURED — UNDERDETERMINED]` under `preregister-topological-charge-transport-v1`.  
**Scope:** the unchanged FTD-0392 Berg–Lüscher convention on scaled octahedral shells `R=1..6`, A/C/E seeds, `L=17`, post-injection ticks `0..8`.  
**Mass consequence:** no mass evidence; the registered topology route is terminal and no alternative shell geometry is licensed.

## 1. Provenance and execution

| Item | Record |
|---|---|
| Lock commit / tag | `993d78c57395ce5c7d5a3f1b0df926c74b695c01` / `preregister-topological-charge-transport-v1` |
| Tag instant / deadline | `2026-07-20T20:40:43-05:00` / `2026-07-23T20:40:43-05:00` |
| Instrument SHA256 | `44f8965d167231bad7019b2bbf79fc8f23356dc26a87f6f29d4db3bb11cae12c` |
| Verifier SHA256 | `a73e9036f1946039e9eb0496bb7f0719669d020cec264f552e9bb2e2550906b2` |
| Binary SHA256 | `4e8b0b3b4e24d277c4fb47052d0e75ede2aebb9cdcd44ddbe24a04a31ec5b649` |
| Canonical CSV SHA256 | `5338d373b80b9eae37bd3c0b23a563d3ea7523ed7920d75693e5cf31eb1ac4fc` |
| Platform | WSL2, Ubuntu 22.04.5 LTS, kernel `6.6.87.2-microsoft-standard-WSL2`, GCC 11.4.0 |
| Build | `cmake --build engine/build_wsl --target campaign_topological_charge_transport -j 32` |
| Run | `unset FTD_FORCE_GPU && ./engine/build_wsl/campaign_topological_charge_transport` (twice) |
| Verifier | `python scripts/proofs/verify_topological_charge_transport.py .../run1.csv --expect UNDERDETERMINED` |
| Raw record | `engine/results/topological_charge_transport_2026-07-20/{run1,run2}.{csv,stderr}` |

Both CSV and stderr records are byte-identical across the two executions. The binary was CUDA-capable and its constructor reported that a GPU backend was available; each `RenderBridge` then immediately called the public `force_cpu()` method before configuration, injection, or any tick. By the backend contract, `force_cpu()` replaces the active backend with `CpuBackend`; `FTD_FORCE_GPU` was unset. Thus the measurements are CPU-executed despite the availability message emitted during construction.

## 2. Correctness gates

All gates passed:

- synthetic radial/anti-radial fields gave `Q=+1/-1` at every `R=1..6`;
- rigid rotation and independent positive rescaling preserved charge;
- all six radii were inside the `L=17` boundary;
- all seeds first manifested at tick 2 at `(8,8,8)`, with one manifested site;
- freeze `R=1` charge reproduced FTD-0392 (`|Q|<=5e-9`);
- freeze `e_half` reproduced A/C/E as `1.3686763085027709`, `5.8282464628352226`, and `0.54072027778782894`;
- every CSV validity bit exactly matched `min_j>1e-12`;
- the grid contained exactly `3 x 9 x 6 = 162` finite, unique rows;
- targeted CTest regression passed `manifestation_seed_diversity`, `hedgehog_charge_robustness`, and `topological_charge_transport` (3/3).

The synthetic sign gate excludes a zero-returning comparator; the FTD-0392 gate excludes an instrument drift; undefined shells were never interpreted as zero.

## 3. Measured charge histories

Each cell lists the charge on `R=1..6`; `U` means undefined. Values printed as `0` below are at floating roundoff around zero, while every displayed `+1/-1` is within floating roundoff of that integer.

| Seed | t | Q(R=1..6) |
|---|---:|---|
| A | 0 | `+1,+1,U,U,U,U` |
| A | 1 | `0,0,0,0,0,+1` |
| A | 2 | `0,+1,-1,0,0,0` |
| A | 3 | `-1,0,0,+1,+1,0` |
| A | 4 | `0,0,+1,-1,0,0` |
| A | 5 | `0,-1,0,0,0,-1` |
| A | 6 | `0,0,0,+1,0,0` |
| A | 7 | `0,0,-1,0,0,0` |
| A | 8 | `0,0,0,0,0,0` |
| C | 0 | `+1,+1,U,U,U,U` |
| C | 1 | `0,0,0,0,0,+1` |
| C | 2 | `0,0,-1,+1,0,0` |
| C | 3 | `0,0,0,0,0,+1` |
| C | 4 | `0,0,+1,0,0,0` |
| C | 5 | `0,-1,0,0,0,-1` |
| C | 6 | `0,0,0,-1,0,0` |
| C | 7 | `0,0,-1,0,0,0` |
| C | 8 | `0,0,0,0,0,0` |
| E | 0 | `+1,+1,U,U,U,U` |
| E | 1 | `0,0,0,0,0,+1` |
| E | 2 | `0,0,-1,0,0,0` |
| E | 3 | `-1,0,0,0,0,0` |
| E | 4 | `0,0,+1,-1,0,0` |
| E | 5 | `0,-1,0,0,0,-1` |
| E | 6 | `0,0,0,+1,0,-1` |
| E | 7 | `0,0,-1,0,0,0` |
| E | 8 | `-1,0,0,0,0,0` |

The recomputed smallest charged radii were:

- A: `[1,6,2,1,3,2,4,3,None]`
- C: `[1,6,3,6,3,2,4,3,None]`
- E: `[1,6,3,1,3,2,4,3,1]`

## 4. Frozen verdict

**UNDERDETERMINED.** The precedence predicates evaluate as follows:

1. **COLOCALIZED — false.** No fixed `R<=2` carries `|Q|>=0.95` from freeze through four later ticks for any seed.
2. **TRANSPORTED — false.** All histories show outward appearances, but charged inner shells later return; the registered non-return condition fails.
3. **ZERO-CROSSING/DESTROYED — false.** The initially outer shells are undefined only before the pulse reaches them. Once defined, the measured shells do not furnish the registered charged-to-undefined crossing followed by exclusively trivial enclosing shells.

The scaled octahedral degree is integer-valued but moves non-monotonically among radii and signs. These records do not distinguish genuine outward transport, repeated local charge creation/annihilation between sampled shells, or coarse-shell aliasing. That ambiguity is exactly the frozen UNDERDETERMINED class, not evidence for any one story.

## 5. Licensed conclusion

FTD-0392's `R=1` zero cannot be promoted into a transport or destruction claim. Conversely, the transient unit values on larger shells do not supply a persistent local invariant or mass floor. This campaign supplies **no mass evidence**, opens no energy-bound proof, and exhausts the registered shell-geometry route. No other shell geometry is to be designed after this result.

FTD-0096 remains the governing no-go on deriving a dimensional rest-mass scale from the current substrate. No framework commitment, selected type, calibration, or epistemic tag is changed.
