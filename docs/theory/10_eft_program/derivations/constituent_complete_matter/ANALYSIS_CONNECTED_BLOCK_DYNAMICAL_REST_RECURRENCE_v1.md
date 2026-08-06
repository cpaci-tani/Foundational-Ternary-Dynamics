# FTD-0627 — The connected block has bounded multimode centre rest

**Status:** `[SELECTED DYNAMICS]` +
`[MEASURED — 256-TICK BOUNDED REVERSIBLE CENTRE REST]` +
`[CLOSED NEGATIVE — LOCKED COMPLETE-STATE RECURRENCE]` +
`[CLOSED NEGATIVE — LOCKED EIGHT-BIN SPECTRAL CONCENTRATION]` +
`[OPEN — STATIC REFINEMENT / LONGER RECURRENCE / MODE CLASSIFICATION]`  
**Protocol SHA-256:**
`72B38166003A90DF92FFEFEF90F2F363A00A96CFEA4EEDDB8BBC57EE3CAF0A4A`  
**Verdict:** `CONNECTED_BLOCK_BOUNDED_IRREGULAR_REST_OPEN`  
**Production status:** unchanged

## 1. Result

The fibre-enabled exact-half connected block remains a bounded centre-rest
configuration for 256 forward ticks and reconstructs its complete initial
state after 256 state-only inverse ticks. Both cyclic arms pass every action,
energy, fibre, boundedness, metadata, and inverse gate.

The locked word `IRREGULAR` means only that neither the registered double-
return recurrence nor the registered eight-bin concentration conjunction
passed. It does not mean chaotic, stochastic, dissipative, or unstable.

## 2. Long-horizon bounds

| observer | base `x` | cyclic `y` |
|---|---:|---:|
| maximum centre displacement | `3.9221e-12` | `1.2888e-13` |
| maximum centre momentum | `1.1604e-12` | `3.7797e-14` |
| maximum complete-state distance | `1.512253e-3` | `1.512253e-3` |
| maximum labelled shape displacement | `1.261495e-3` | `1.261495e-3` |
| maximum squared-edge strain | `4.085621e-3` | `4.085621e-3` |
| minimum shared-pair separation | `0.9979551` | `0.9979551` |
| energy drift | `1.7009e-13` | `1.6964e-13` |
| state-only recovery | `9.3117e-12` | `9.3907e-12` |

Maximum multiplicity remains two. No constituent, charge, or bond is created,
deleted, or reordered. The cyclic history residual is `2.97e-12`.

The internal envelope does not grow materially relative to FTD-0626's 16-tick
record. Shape maximum is unchanged to reported precision; strain grows only
from `3.9147e-3` to `4.0856e-3`. This is finite-horizon boundedness, not an
all-time stability theorem.

## 3. Recurrence

No integer `P` in `16...128` returns the complete state within `1e-6` at both
`P` and `2P`. The locked periodic classifier therefore fails. A return of a
single reduced shape coordinate would not have sufficed; matter and the field
were compared together.

## 4. Fixed spectral record

The unwindowed 256-tick spectra show at least two resolved bands. The dominant
bins are near `k=69` and `k=109`, corresponding descriptively to periods near
`3.71` and `2.35` ticks. These are finite-record lattice frequencies, not
physical masses or a Compton clock.

| observable | `C_8` base | dominant bins |
|---|---:|---|
| axial-bond coordinate `Q_1` | `0.86704` | `69,68,109,107` |
| face-diagonal coordinate `Q_2` | `0.86364` | `69,109,68,107` |
| body-diagonal coordinate `Q_3` | `0.87874` | `109,107,69,110` |
| polarity-interface coordinate | `0.94041` | `69,68,70,67` |

Both cyclic arms select identical top-eight bin sets. The interface coordinate
passes the `0.90` concentration gate, but every bond-shell sequence misses it.
The preregistered conjunction therefore closes negative without moving the
threshold. Neighboring dominant bins may reflect finite-window leakage, while
the separated `69/109` bands support genuine multimode content. Distinguishing
those explanations requires a new lock.

## 5. Ontological consequence

The strongest current connected-matter statement is now:

> A finite constituent-complete ternary pattern can remain macroscopically at
> rest while its internal configuration and coupled field evolve through a
> bounded, deterministic, energy-preserving, state-only-reversible multimode
> history.

This realizes the dynamical-pattern idea more directly than a frozen voxel or
rigid lump. It does not yet establish self-generation, asymptotic stability,
a physical particle, or an intrinsic clock. The initial rigid block may simply
be a nonstationary point near a static fibre-enabled equilibrium.

## 6. Next discriminator

Before spending another factor of four on a longer recurrence record, solve
the simpler structural question:

1. construct a symmetry-reduced, fibre-enabled static refinement at exact
   half phase using the same total energy and minimum-energy Gauss field;
2. require a genuine zero-momentum one-tick fixed point and a positive
   constrained Hessian, not a time average declared to be rest;
3. if a fixed point exists, extract its small-amplitude normal modes and boost
   it;
4. if no fixed point exists, use a preregistered shooting method for a periodic
   orbit or extend the spectral record with a leakage-aware estimator.

No reaction transaction is motivated by this branch.

## 7. Reproducibility

- runner SHA-256:
  `77FF3D7A9068897A67A23EF281F95D61CC8F8CA4FBE7C5C53B128FA05DDC6051`;
- JSON SHA-256:
  `E3451D9230A87610B68F8DF27D67C5D536C5582B24818ACF6CB93FDB7E62AE93`;
- arm CSV SHA-256:
  `ED6D1C95900F0B1A0D2B8B9C1AE60DF13F7354985FBCE551C7C806980FFCC9FB`;
- tick CSV SHA-256:
  `C541C7E82B65DD31E00AAF51AEA8209DB881024E9401F1F947F67AFB1DCDEA78`;
- spectrum CSV SHA-256:
  `B622F2127A88B594CA0AC4F569D02FD5D675A9AF6C7106FB968BC636F8FF8E5F`;
- independent certificate SHA-256:
  `C6415FAD5A2482608990467759722700D5B3D3DE9FCD8703B23B8F9AD4D792DD`;
- independent certificate: `61/61` checks pass.
