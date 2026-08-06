# RUNNER LOCK — Localized-basin relaxation v2

**Identifier:** `FTD-0679`  
**Status:** `[LOCKED BEFORE EXECUTION]`  
**Date:** 2026-07-28

- protocol SHA256:
  `697FC9058FA9AD3A48F10833CAA744C9260570DB3A5AF8F2F8CE97B32C65DF95`;
- v2 runner SHA256:
  `26B45994628350BD979EE1C4CF9B8A6520B7A023D2D8FDB7696C6BDBC57E83D2`;
- embedded v1 source SHA256 after guard-only reuse change:
  `B138FCE6E91605064D5280F29CF1B1D23DECFF80D94071915B7180CA19078DF7`;
- Release executable SHA256:
  `BBC2156DA1CFA5DB3A6965589E7AEAC30FA215E8B992FFE9EF57061C1A2D1C16`;
- toolchain: pinned MSVC `14.44.35207`, Ninja Multi-Config, Release;
- compilation completed before this lock;
- the v2 executable had not been invoked when these hashes were recorded.

The change to the v1 source is only a compile-time main guard.  The executed v1
binary and its pre-guard source remain identified by the hashes in
`AUDIT_LOCALIZED_BASIN_RELAXATION_v1.md`.
