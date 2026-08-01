# FTD-0761 — M4 boosted relational transport discovery pre-execution audit v1

**Status:** `[PRE-EXECUTION LOCK — FINAL QUALIFICATION PASSED; REGISTERED MODES NOT RUN]`  
**Date:** 2026-07-31  
**Protocol:** `PREREG_M4_BOOSTED_RELATIONAL_TRANSPORT_DISCOVERY_v1.md`

## Lock statement

At this audit point `engine/results/ftd_0761/` does not exist. No registered
face, edge, or body mode has run and no FTD-0761 evidence artifact exists.
The final frozen `L=33`, `<100>`, two-tick WSL2 CUDA qualification passed:

```text
rows                         3
initialized/executed/exact   1 / 1 / 1
final graph margin           0.614544
final energy margin          0.000909837
common-action residual       2.95692e-14
energy residual              7.069e-17
minimum singular value       0.994129
condition number             1.0766
reverse recovery             3.55271e-15
```

Two earlier source-changing development probes exposed and removed runner
bookkeeping defects: a moved-from resident field snapshot had been used for
reverse comparison, and the small-volume qualification had incorrectly
invoked the large-volume support observer. Those probe binaries are not the
frozen runner, wrote no result artifacts, and contribute no evidence.

## Frozen artifacts

| artifact | SHA-256 |
|---|---|
| protocol | `AD6368C6793374771703A1506FA60C06E1D11C0649227F315DD1A79A0F3BDA5C` |
| CUDA runner source | `E13CDB23EBBE8E1127719D8E174C4DDCD6D9CAC1139EB889A5F939172CE09459` |
| independent certificate | `FEADB09B9AAC4BAB169D8F04E1F2C098BCE2A174DFBAA478B3D250BF61083D6F` |
| inherited M3 protocol | `681FA36CCE4479D268D37651E4CD58AA6C1D5A4809F989EA4FF2AA24B7B40722` |
| inherited M3 runner | `F2CCACB00E0DF697B10838E3E85EC636E38BC94E2B2707A55A86811FFE80DCEA` |
| matter-predicate header | `B11E087E2E7E16375C173185233AD001AB8B9F049E9B9B5A3156D8618CB4F104` |
| matter-predicate source | `752CE7C3B03A9944C1E7016A62CCA584FAC868EF191D8241ACEE7E6C9C550D21` |
| CMake manifest | `F07C9A7F102E224987AB8654C78FF52CE3432C24642DB1DCDFC84543E18642DC` |
| WSL2 CUDA executable | `F682EBB62E2A0D8728EDAD48345181A902B80A30DD6787D6D0D1C5A9882A52D1` |

The independent preflight passes `33/33` checks, including exact hashes,
CUDA/inversion/transport runner tokens, and absence of the registered result
directory.

## Execution authorization

The frozen executable may now run exactly once for each registered mode, in
order:

```text
--run face
--run edge
--run body
```

The body mode may construct the aggregate only after the face and edge JSON
records exist. A failed or interrupted registered mode is evidence and may not
be tuned or repeated under FTD-0761. Production dynamics, defaults, ontology,
predicate, common action, `RenderBridge`, and scenarios remain unchanged.
