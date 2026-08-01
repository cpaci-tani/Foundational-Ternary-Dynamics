# Audit — volume-scaled internal-mode transfer v1

**Ledger ID:** FTD-0664  
**Verdict:** `[EXECUTION INVALID — NO PHYSICAL VERDICT]`  
**Production status:** unchanged

Protocol SHA-256:
`B6C7E2632884FA6CC98499D42EE6E4CE1AE790C9B6261E034278ABABB2FFB933`.

The runner completed all requested histories, but every inverse recovery is
above the locked `1e-10` gate. The initial modal normalization is independently
invalid because tick zero is not unity. Favorable pre-return field morphology
is non-promotable under v1.

Artifact SHA-256 values:

- final guard-equivalent runner: `1EEA0688F32F5FB9FD567E60098E16535EF0E93B7FB74629EDB8FE2CC2BD71DE`;
- JSON: `EB6228CCE248DBF83822C87E957A35D057DA82311461CF192F24CF06E150A6A8`;
- arms: `D9DD2D447F5E476F4AE692D228CF193B882132A4930FE2D19DA160C809666815`;
- ticks: `C301B49A9DB5856F803267D64B9F32E34E6A1DB90FA1CAD6F92C250681803C8E`.

The raw JSON preserves C++'s lowercase `inf` for an undefined descriptive
return-CV field; the independent certificate parses that field as null and
does not use it as a physics gate.

Independent certificate:
`scripts/proofs/proof_volume_scaled_internal_mode_transfer.py`, SHA-256
`7352CF6E5A5AFA5C78487506048F3F0A05CE0C77080841A7C940D39C63C9807A`.
