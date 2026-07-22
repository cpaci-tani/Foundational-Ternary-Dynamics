# ANALYSIS — Target-blind particlehood before mass (FTD-0399)

**Status:** `[INVALID — G2 manifestation portability gate failed]` under `preregister-target-blind-particlehood-v1`.  
**Licensed conclusion:** no particlehood result and no permission to attempt another mass observable on the current engine.  
**Scope:** exact A/C/E histories, dissipative and undamped protocols, `L=33/65`; no particle target, mass constant, clock, force, or imported mass diagnostic.

## 1. Provenance

| Item | Record |
|---|---|
| Lock commit / tag | `c4f7af98cac1a15c5d0478c8c4953068dedc2436` / `preregister-target-blind-particlehood-v1` |
| Tag instant / deadline | `2026-07-20T21:02:36-05:00` / `2026-07-23T21:02:36-05:00` |
| Instrument SHA256 | `14833be2d81d31b682af73b51618126fa9a6c1991b2d965d0538a907c367b501` |
| Frozen full-profile verifier SHA256 | `a35a7c1b2a4b50818678cc59fcf4343d6d818bde61171813fd5bfa370d1e75d0` |
| Binary SHA256 | `738898b931001bb8895ae425c8c477bdb3a5a61d28401999d407d0c7ffd56cb7` |
| Canonical stderr SHA256 | `a8edc12dbfd73a77f91bb39e4873fa56039aab53ce6e6e722bb22fc3c4d39972` |
| Platform | WSL2, Ubuntu 22.04.5 LTS, kernel `6.6.87.2-microsoft-standard-WSL2`, GCC 11.4.0 |
| Build | `cmake --build engine/build_wsl --target campaign_target_blind_particlehood -j 32` |
| Run | `unset FTD_FORCE_GPU && ./engine/build_wsl/campaign_target_blind_particlehood details.csv > summary.csv 2> run.stderr` (twice) |
| Raw record | `engine/results/target_blind_particlehood_2026-07-20/` |

The binary is CUDA-capable and logs GPU availability during each constructor. Every bridge then calls `force_cpu()` before toggle configuration, injection, or ticks; the active backend check in each captured admissible history is CPU. `FTD_FORCE_GPU` was unset.

## 2. Decisive correctness result

G2 required every one of the 12 protocol/size/seed arms to manifest and reproduce freeze tick 2. The exact results were:

| Protocol | L | A_baseline | C_hot | E_cold |
|---|---:|---|---|---|
| dissipative | 33 | no manifestation by tick 200 | tick 2 | no manifestation by tick 200 |
| dissipative | 65 | no manifestation by tick 200 | tick 2 | no manifestation by tick 200 |
| undamped | 33 | no manifestation by tick 200 | tick 2 | no manifestation by tick 200 |
| undamped | 65 | no manifestation by tick 200 | tick 2 | no manifestation by tick 200 |

All 12 histories were internally executed twice. Every duplicate was bit-identical, including the eight no-manifestation records. The complete campaign was also executed twice externally: both 1,866-byte stderr records are byte-identical and both terminate `VERDICT,INVALID`.

This is not a stochastic miss and not a damping effect. The exact A and E histories that manifested at tick 2 in the prior `L=17` seed-diversity campaign are not manifestation-portable to either registered larger size under either protocol. Only the high-amplitude C history is portable in this test.

## 3. Why the summary and detail files are empty

LOCK-STD gives correctness gates absolute precedence. Once G2 failed, the instrument completed the remaining gate census but did not enter the profile-comparison phase. It therefore emitted no summary rows and no detail file. The empty summary SHA256 is the standard empty-file digest `e3b0c442...b855`.

The frozen Python verifier is a recomputer for a complete 166,212-row profile record. Such a record is inadmissible here because eight histories never acquire `t_post=0`; running distance, CV, cross-size, or outcome calculations on the four surviving C arms would violate the target-blind three-history design. The verifier is therefore **NOT APPLICABLE after G2**, not silently passed. The exact gate evidence is the internally and externally duplicated engine record.

## 4. Frozen verdict and scope

**INVALID.** No later outcome row is evaluated:

- not SPECIES-INVARIANT;
- not DISSIPATIVE-ATTRACTOR;
- not HISTORY-FAMILY;
- not NO-STABLE-EXCITATION (that row begins only after correctness gates pass).

The result does not prove that native particlehood is impossible. It proves that the specifically authorized A/C/E comparator cannot test it at `L=33/65`, because the supposed common birth ensemble does not exist there. Repairing the births would change the registered production histories and would be a new protocol, not a reinterpretation of this one.

Under the roadmap's stopping rule, current-engine first-principles mass generation remains stopped: native particlehood was not established, and no mass observable, energy–momentum pair, or dispersion relation is opened. FTD-0096 remains the governing rest-mass-scale no-go. No framework type, commitment, calibration, or epistemic tag changes.
