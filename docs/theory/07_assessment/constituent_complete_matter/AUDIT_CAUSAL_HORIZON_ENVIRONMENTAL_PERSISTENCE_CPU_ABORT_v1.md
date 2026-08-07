# FTD-0746 CPU execution abort audit v1

**Identifier:** FTD-0746  
**Status:** `[ABORTED BEFORE SERIALIZATION — NO PHYSICS RESULT]`  
**Date:** 2026-07-29  
**Protocol:**
[`PREREG_CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_v1.md`](../../10_eft_program/preregistrations/constituent_complete_matter/PREREG_CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_v1.md)

## Disposition

The clean CPU `face` arm was terminated at the user's direction after
approximately 2 hours 55 minutes so the large-volume instrument could be
ported to CUDA. The process had not reached its serialization point. Direct
post-termination checks found no `engine/results/ftd_0746/` directory and no
CSV or JSON artifact.

Therefore:

- FTD-0746 has no completed arm and no physics verdict;
- elapsed runtime, process memory, and any in-memory partial history are not
  admissible physics evidence;
- `edge` and `body` were never started;
- the locked CPU protocol is closed administratively as aborted, not passed,
  failed, negative, or unresolved;
- a CUDA campaign must use a new identifier and protocol version after its
  numerical parity and determinism gates pass.

The initial shell cancellation did not terminate the child process inside
WSL2. The child was then sent `SIGTERM` directly and its absence was verified
with `ps`. No recursive deletion or artifact cleanup was necessary because no
result artifact existed.

## Preserved provenance

The pre-execution audit and frozen hashes remain the record of what would have
run. They do not license reuse of a partial trajectory. The CUDA successor may
retain the same physics state, gates, and discovery record, but its kernels,
parity tolerances, determinism contract, source hash, executable hash, and run
records must be frozen separately before execution.
