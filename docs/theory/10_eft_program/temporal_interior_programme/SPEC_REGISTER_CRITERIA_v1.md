# SPEC — Minimum Constraints on a Co-Transported Register v1 (front T2)

**Status:** `[SYNTHESIS — CONSOLIDATED CRITERIA SPEC]`; declares the checklist
before any candidate is scored; introduces no claim, scores no candidate
**Date:** 2026-08-07 · **Successor parent charter:** `SCOPE_TEMPORAL_INTERIOR_PROGRAM_v2.md` (v1 provenance archived)
**Parents:** FTD-0777 (clock–memory boundary: one transitive tower holds no
payload; a stable relative payload requires a separately selected,
co-transported reference lift), FTD-0394 (readout irreversibility),
FTD-0676 (in-band decay of the internal doublet), FTD-0768/0761 (transport
status), `SPEC_CARRIER_CONSTRAINTS_v1.md` (C5/C7/C10/C11/C12 analogues)

A *register* is the physical structure FTD-0777 proved a clock alone cannot
be: state that rides with a recurrent carrier and survives its cycles.
Retention converts passage into a "was" — the second of the temporal
interior's three purchased structures. Any proposed register must be scored
against all nine criteria before preregistration.

## Tier 1 — existence

**R1. Distinct carrier.** The register is a separately selected structure,
not a function of the clock's own phase (FTD-0777's negative: a transitive
tower observes only `m + n mod 2^K`; no cycle-invariant payload). Declaring
the register's identity *is* the FTD-0777 "reference lift" made physical,
and its selection is priced as such.

**R2. Discrete state space.** At least two distinguishable configurations
separated by a declared barrier, with a declared readout observable that is
not a closing linear functional of the field (C7 analogue) and has fixed,
state-independent support (C10 analogue).

**R3. Retention.** Hold time of at least `K_ret` clock cycles of the
companion carrier under native evolution, with spontaneous switching
probability below a declared floor per cycle. `K_ret ≥ 8` inherits the
FTD-0772 gate lineage; occupancy-grade applications want far more.

## Tier 2 — operability

**R4. Write admissibility.** A declared interaction (native or selected —
stated which) that switches the register in bounded time without destroying
the companion clock; the write channel is priced separately from the
register's existence.

**R5. Read without erasure.** A readout leaving the register state intact
within declared tolerance. FTD-0394's measured manifestation-readout
irreversibility (magnitude loss) binds here: any readout routed through
manifestation must budget that loss explicitly.

**R6. Drain-freedom.** The register's field dressing stays below the
genesis threshold over the full hold time (C5 analogue): `|J| < K_GENESIS`
on its support, or every crossing bleeds the state.

## Tier 3 — composition and evidence

**R7. Co-transport.** The clock+register composite survives translation
(the FTD-0761 moving-core witness class), or the candidate is explicitly
scoped static-first with transport deferred and priced as open.

**R8. Native licensing.** Scored in a profile where imposed phenomenology
is not doing the work (C11 analogue, including its FTD-0786 correction:
selection-scoped results are admissible with the selection declared).

**R9. Preregistration.** Locked channels, held-out seeds, calibrated
instruments, and — after the hexagon-wheel lesson — an explicit
*coupled-escape* check: the register's barrier must be verified against
combined flexes/paths, not per-mode (the G4-vs-G5 distinction of
`native_chain_network_verify.py`).

## Candidate pool (named, unscored)

1. **Internal modes of the dressed composites** (FTD-0600–0739 sector).
   Known hazard: the first internal doublet decays in-band at
   `Γ_E = 0.0065`/tick (FTD-0676) — R3 likely fails fast for oscillatory
   states; *configurational* (barrier-separated) internal states are the
   live sub-class.
2. **Polarity/geometry configurations of small bonded networks** — e.g.
   distinct stable arrangements of an MVC-class chain (its z-mirror parity
   pair) or of capacity-saturated blocks' surface states (the diffuse
   surface observer of the many-body derivation).
3. **The FTD-0494/0495 history fiber** — closes bookkeeping but not
   ordinary common action; admissible only as a comparison baseline, per
   its own registered scope.
4. **Relational kernel internal state** (FTD-0669 boundary) — the matter
   ontology's supported identity candidate; scoring requires the cleared-
   region energy ledger discipline already registered there.

The first screen over this pool is a separate, preregistered act (R9).
This spec moves no tag and licenses no candidate.
