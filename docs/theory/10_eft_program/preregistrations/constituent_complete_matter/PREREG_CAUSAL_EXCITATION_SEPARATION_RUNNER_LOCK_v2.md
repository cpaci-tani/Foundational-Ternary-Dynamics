# RUNNER LOCK — Causal excitation separation v2

**Identifier:** `FTD-0685`  
**Status:** `[LOCKED AFTER COMPILATION, BEFORE EXECUTION]`  
**Date:** 2026-07-28

- v2 protocol SHA256:
  `FEDD4A5B09DBA6443A34159D9563456E652BA2B4060643A2693511621EED95DF`;
- v2 wrapper SHA256:
  `4A4D7F3D0FE0F0DE2CD272113B0F885E224C624C30CD07ED3A3360CA0FD9EFF1`;
- parameterized embedded runner SHA256:
  `958CFF531DAA319E8CE3C27B377B45F41C88883373C31A0E7D34FA4B34D4F265`;
- v2 Release executable SHA256:
  `45E7242E40FD1060A25989CDF856DC9100C89919D159B81915984915D62B9E6F`;
- sole v2 change: fixed-origin center preflight uses norm tolerance `1e-12`;
- all FTD-0684 physical parameters, observers, gates, classifiers, and output
  contracts are inherited unchanged;
- toolchain: pinned MSVC `14.44.35207`, Ninja Multi-Config, Release;
- the v2 executable had not been invoked when these hashes were recorded.
