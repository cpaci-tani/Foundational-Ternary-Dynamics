# External Research Archive — 2026-09-15

**Disposition:** owner-requested removal from the current FTD working tree; preserved externally. No epistemic claim is promoted or reclassified by this move.

**Local archive:** an owner-managed archive outside this repository, identified by `2026-09-15-checkpoint`.
**Archive ID:** `2026-09-15-checkpoint`.
**Pre-cleanup commit:** `87eb1cd11c7da9ff78cd4a27852cecceccef8c77`.

The archive contains `manifest.json` with original paths, byte counts, tracking status and SHA-256 digests; `payload/` preserves the original directory layout. Every moved file was verified against its pre-move digest. `before-cleanup.bundle` preserves the complete reachable Git history and refs before cleanup. Previously tracked sources remain recoverable from that history. The ignored RH/prolate-Weil files are preserved in the payload, not newly added to Git.

The scope is the standalone RH/prolate-Weil work, six speculative application/polemic papers, and the already-retracted Navier–Stokes/Yang–Mills paper artifacts. The canonical finitude paper, per-voxel mass-gap proof, lattice/thermal investigations, and engine code remain in FTD. Historical assessments and retraction records remain as provenance; former source paths in those records refer to the pre-cleanup commit.

This is a local archive, not a portable publication URL. To restore an item, verify its manifest digest and copy it from `payload/<original_path>`. For a previously tracked source, `git show <pre-cleanup-commit>:<original_path>` also recovers the committed version. The SHA-256 table below identifies the exact working-file bytes moved.

## Preserved files

| Former repository path | Previously tracked | SHA-256 |
|---|---|---|
| `docs/papers/speculative/DERIV_CASIMIR_RATCHET.tex` | yes | `6670e0c00e94bc80622e43f20035789c8487e412dbfb708d4e9ed86a8b0b6ebf` |
| `docs/papers/speculative/DERIV_GEOMETRIC_BIOPHYSICS.tex` | yes | `3a2fe1831c67f1cbc777d4dff24006d4d803421ddf3925b357660a189cab877d` |
| `docs/papers/speculative/DERIV_GRAND_UNIFIED_MASS.tex` | yes | `a27403440c027e5776110b461a4ba5451d025db98ab35425a4379498977142b1` |
| `docs/papers/speculative/DERIV_SONOLUMINESCENCE.tex` | yes | `d518eb7f41c71a415ee24adac8e9ee1737aa15044414c78bcaf3f91012916872` |
| `docs/papers/speculative/FTD_Riemann_Hypothesis.tex` | yes | `187e8f65db1681dbc82d9c973049b28b97ed02fef7a2b1382425852ada65124f` |
| `docs/papers/speculative/LETTER_HERMITIAN_COPE.tex` | yes | `c1faee404168ce6805ef59c160aa3fc759cb23ffdbde1653c11b94a4f615b850` |
| `docs/papers/archive/retracted_under_reframe/FTD_Navier_Stokes.tex` | yes | `b78734ace575fc54a7d34fabed7da2fd9104501d78e01d817bf69968f8f64703` |
| `docs/papers/archive/retracted_under_reframe/FTD_Navier_Stokes.pdf` | no | `741fbe00488f867dcf2f36438382505d4aa3bf8468e114b4f805933c47344543` |
| `docs/papers/archive/retracted_under_reframe/FTD_Yang_Mills_Mass_Gap.tex` | yes | `9251ff1008f907b89359ae194de48bee8201d8446742a92a98eed46ff88332f9` |
| `docs/papers/archive/retracted_under_reframe/FTD_Yang_Mills_Mass_Gap.pdf` | no | `b79ed7a77ed79f86b382ad019891229dc8716361d65f08962e0a23ac69ab6492` |
| `docs/theory/09_mathematical/number_theory/EXPLR_RIEMANN_ZETA_CONNECTION.md` | yes | `c184fdf715e9e41abe791829ef521933564a559d9383caad6dbbb522e51d1de6` |
| `docs/theory/09_mathematical/number_theory/DERIV_PROLATE_WEIL_CONVERGENCE_PROOF.md` | no | `3eb7215c35f48efd6b728dd36601be9706804aeee1581e4914f7091451a20dfe` |
| `scripts/proofs/proof_prolate_weil_candidate.py` | no | `6c8c4e1a2276e5964d05f4853b5047fe212df3a2ae6ca2b6868c1b35f2627c43` |
| `scripts/proofs/proof_weil_spectral_gap.py` | no | `d33cbb66f94898bd77a71e4011ebecd3dd2b31b341c377131b876661e464f7bc` |
| `scripts/proofs/proof_active_quotient_gap.py` | no | `5bd7fc0a317217e49abec0c2568def3a9a1de390d8c1cac2bb03f25d5f4adf27` |
| `scripts/proofs/proof_complete_prolate_weil_chain.py` | no | `571e0a8ab539090951c90cf02374fed2dcb37ca8a9ed0dfec5d061cf3182d618` |
| `scripts/proofs/__pycache__/proof_active_quotient_gap.cpython-313.pyc` | no | `1d9a9d9787a143437250b9546164bc49117aee89ca58ee1ba664060c2e559c9b` |
| `scripts/proofs/__pycache__/proof_prolate_weil_candidate.cpython-313.pyc` | no | `69f2e873886bfea0307082226f9d28ce60421d4dd4a886305c30d0bced83ff79` |

## Checkpoint boundary

All existing engine and dashboard work was already committed on `main` at the pre-cleanup boundary. This cleanup adds the external-archive receipt and reconciles active navigation. Git history is retained. No remote push is authorized; the checkpoint remains local. The universal seeding plan remains the next implementation plan.
