# RUNNER LOCK — Causal excitation separation v3

**Identifier:** `FTD-0687`  
**Status:** `[LOCKED AFTER COMPILATION, BEFORE EXECUTION]`  
**Date:** 2026-07-28

- v3 protocol SHA256:
  `BA1800AFB9E1B9B5715DD3A4A89908963E6BB1E6443C1C535F5D8DE1BF86D3CB`;
- v3 wrapper SHA256:
  `BD7D6E317EA677C011FB78B81496E001FDE481066C5CEF6285259F81C0A9D287`;
- parameterized embedded runner SHA256:
  `3874C2D5FB317161357C93CFE0C34ACB602046C54D558DAE73F5204DD52E4D4B`;
- Release executable SHA256:
  `862C71B2456AB1144629D8D59118F85E7AAF0F885B24272FD9D5CABAEFC7AFEA`;
- FTD-0686 header/source/test/proof SHA256:
  `464A8A1F...F89479`, `0185D1D9...DB57C`,
  `A1752FD0...E25F3`, `7EB5DB4A...D0846`;
- scalar-equivalence CTest result: worst scalar difference
  `4.04121180963557e-14`, partition closure `1.14130926931466e-13`;
- exact-rational certificate: 256 masks pass;
- toolchain: pinned MSVC `14.44.35207`, Ninja Multi-Config, Release;
- the v3 executable had not been invoked when these hashes were recorded.
