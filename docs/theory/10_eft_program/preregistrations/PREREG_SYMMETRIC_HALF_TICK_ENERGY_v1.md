# PRE-REGISTRATION — Symmetric half-tick transaction energy gate v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0469`  
**Status:** `[PRE-REGISTRATION — RELOCKED/RUN; ABSOLUTE GATE FAILED]`  
**Parents:** `FTD-0293`, `FTD-0443`, `FTD-0452`, `FTD-0467`, `FTD-0468`  
**Engine artifact:** `engine/tests/campaign_symmetric_half_tick_energy.cpp`  
**Campaign SHA256:** `08D79B1D345D33AC0CDC638947365193D4AFADC753B06576849F290330ECACD8`  
**Helper SHA256:** `E72D74499736DF6D922EDD00728E81DA343DAECA566CB91D8CB998D0F7676B3E`  
**Independent replica:** `scratch/ftd_0469_replica/` (`replica.py`
`3C6C002CFC3030D0224DAE60E76B9D6998C66025BC8E4BD3C6CADB3229B6DCFE`,
`verify.py`
`1AEE53247CE3DD0DB8A1C7B8BC06C9DB1AF6CE47DFA5FFEA2FD84540150BFC6C`)

**Pre-run relock note:** the first unrun draft used “time-symmetric” for a
source-centered composition and evaluated multiparticle kinetic energy from
the global net impulse. Before any C++ campaign execution, both defects were
corrected: H6 now states the non-self-adjoint result, and H5 now uses one
momentum ledger per manifested site. The hashes above are the only campaign
and helper hashes authorized for the run of record.

## 1. Question

FTD-0468 established the exact common-action momentum pair: source kick
`Delta W = -G_C grad(s)` and matter impulse `I = +G_C s grad(div J)` close
total momentum on every finite periodic lattice. The remaining obstruction
named by FTD-0468 is energy and time-centering. This campaign asks:

1. Does one source-centered kick-drift-kick transaction (half source kick and half
   matter impulse, exact source-free symplectic-Euler drift, second half
   kick with the post-drift field) conserve an exact energy without a fitted
   counterterm?
2. Is the total momentum `p_matter + P_field` exact through whole
   transactions, including through the drift?
3. Does the implemented algebraic inverse restore the field and per-site
   matter-momentum ledger exactly?
4. Where does the particle kinetic energy `E(p) - E_REST` sit in the exact
   ledger while the site state `s` is frozen (no hop)?

## 2. Pre-derived identities under test

With `A = -C_WAVE^2 L` (L the 18-point laplacian), `g = -G_C grad(s)`,
`E_tick` the FTD-0293/0452 invariant, `H_int = -G_C sum_x s div(J) = -g^T J`:

- **H1 (shadow energy):** the source-centered transaction exactly conserves
  `E_shadow = E_tick + H_int + CT_sym`, `CT_sym = +(1/4) g^T A J`.
  Derivation: with `A J* = g`, the transaction is the exact source-free
  drift in variables `(J - J*, W - g/2)`; the coefficient 1/4 is forced.
- **H2 (naive total difference):** the uncorrected ledger satisfies
  `[E_tick+H_int](t) - [E_tick+H_int](0) = CT_sym(0) - CT_sym(t)` exactly:
  bounded, no secular term.
- **H3 (drift momentum):** the source-free drift conserves the central
  field momentum exactly (D_i and L commute, L D_i is skew-adjoint).
- **H4 (production-ordering control):** the fused full-kick ordering
  conserves `E_tick + H_int + (1/2) g^T W` exactly.
- **H5 (unfunded matter energy):** the field-plus-interaction ledger closes
  at machine precision while `sum_i [E(p_i)-E_REST]` grows; each manifested
  site has its own impulse ledger. At frozen sites the particle kinetic
  energy is the total-ledger surplus and is not funded by the
  field-interaction sector. Funding is reserved for hop events
  (FTD-0443/0452 exact hop work).
- **H6 (not self-adjoint):** source-centering does not make the whole method
  time symmetric. The middle symplectic-Euler drift is not self-adjoint:
  for homogeneous one-step matrix
  `M_h=[[I-h^2 A,hI],[-hA,I]]`, generically `M_-h M_h != I`.
  The campaign tests exact inversion by the separately derived inverse, not
  the stronger and false identity `Phi_-h=Phi_h^-1`.

## 3. Frozen fixtures

Static (L=17, 256 transactions, 12 arms): 3 axes x 2 orientations x
{single locked polarity + quadratic axial `J`, opposite locked pair
(separation 6) + cubic axial `J`}, deterministic nonzero `W` background
(same seeds as FTD-0468). Dynamic (L=33, 64 transactions, 6 arms): locked
pair separation 8 plus exact longitudinal travelling mode `n=2`, phase
`0.37`, per axis and orientation. Production-ordering control (L=17, 256
ticks, 3 arms): pair + cubic fixture.

## 4. Gates and outcome mapping

- residual gate `1e-12` on: shadow-energy drift (H1), naive total-difference
  identity (H2), total momentum closure, reversal restore residual,
  production-ordering invariant (H4);
- protocol validity: CPU backend, finite ledgers, accumulated impulse above
  `1e-14` in every arm (no trivial-zero closure).

Outcomes:

- all gates pass: `SYMMETRIC_HALF_TICK_SHADOW_ENERGY_EXACT`;
- any energy/momentum/reversal gate fails with valid protocol:
  `SYMMETRIC_HALF_TICK_ENERGY_FAILS` (H1-H4 falsified as written);
- protocol invalid: `PROTOCOL_INVALID`, no verdict on H1-H5.

The summed per-site `E(p_i)` surplus (H5) is recorded per arm as
`particle_energy_delta` against
`field_ledger_delta`; it is a measurement, not a pass/fail gate, and any
claim that the surplus is funded by `J/W` at frozen site is pre-registered
as false unless `field_ledger_delta` tracks `-particle_energy_delta`.

## 5. Run of record

Pinned MSVC `14.44.35207`, Release, forced CPU, focused CTest
`campaign_symmetric_half_tick_energy`, record to
`engine/results/ftd_0469/windows_msvc_cpu.csv`. The Python replica is
supporting evidence only and is not the run of record.

**Recorded outcome:** valid protocol; verdict
`SYMMETRIC_HALF_TICK_ENERGY_FAILS`. Worst shadow residual
`2.1103119252074976e-12` exceeded the locked absolute `1e-12` gate.
Momentum and explicit-inverse residuals remained below gate. See
`AUDIT_SYMMETRIC_HALF_TICK_ENERGY.md` for the exact scope and the corrected
per-site matter-energy result.
