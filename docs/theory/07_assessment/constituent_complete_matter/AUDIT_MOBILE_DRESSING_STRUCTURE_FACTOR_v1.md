# Audit — mobile dressing structure factor v1

**Ledger ID:** FTD-0655  
**Verdict:** `MOBILE_DRESSING_STRUCTURE_FACTOR_EXECUTION_INVALID`

The protocol was hash-locked before the observer was implemented, but it is
internally inconsistent. It specifies `T_phys=64` and `a=2/w`, which imply
`32w` ticks, while its locked-arm section explicitly requires `64w` forward
and reverse ticks. The runner executed `32w`. All 18 raw histories and 1,746
forward samples are present, but the literal arm count was not executed.

An independent Python certificate reconstructs both complex phase fits, both
amplitude coefficients of variation, relative-phase RMS, centre velocity,
mirror/cubic residuals, and all three refinement trends from the raw series,
then confirms the `32w != 64w` protocol nonconformance.

No co-motion statement is licensed by v1. The raw measurements are provenance
only. A corrected rerun is required before the necessary-but-not-sufficient
structure-factor gate can advance to an on-shell pole campaign.

## Reproducibility

- protocol SHA-256: `09523E64E273E7808FF21A446B26C012531931EF948F3F48F090D4F851C0F2A0`
- runner SHA-256: `6BFE9975332BB07F8531C2D2F11663146035F9A237BDBC4A54452C3B0E3D83D1`
- JSON SHA-256: `87625E629C2AC01AB9A5F3983DE044AA7483A9487A955ADDF6AE233D2D418439`
- arms CSV SHA-256: `3ECBB90E04A87651013ED09062CF46E72C0EA91681C11847E0A4D236F481029A`
- series CSV SHA-256: `F7A5490806D65B504739C75AB8BC80FF2E731267019430666A38B0C32F014931`
- protocol-nonconformance certificate SHA-256: `35A55F35D6E2BEC570F552B0DD18577AB7535DC4864F8A4D02337BF1F65B2E1E`
