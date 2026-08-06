# FTD-0786 — The Movement Sector Was Opened; The Wall Is C2 v1

**Status:** `[CORRECTION — FTD-0782 GOVERNANCE CLAIM]` +
`[SYNTHESIS — REGISTERED-RECORD RECONCILIATION]` +
`[EXACT — C2 FAILURE OF THE FIRST INTERNAL DOUBLET]` +
`[STRUCTURAL COROLLARY — THE TWO OPEN DOORS ARE ONE DOOR]`
**Verdict:** `MOVEMENT_SECTOR_CARRIER_FAILS_AT_C2_NOT_AT_THE_TRANSACTION`
**Parents:** `FTD-0545/0546/0549` (the frozen-form negatives), `FTD-0551`,
`FTD-0600`–`FTD-0656`, `FTD-0658`, `FTD-0663`, `FTD-0676`, `FTD-0699`,
`FTD-0700`–`FTD-0703`, `FTD-0739`, `FTD-0772`, `FTD-0780`, `FTD-0782`,
`FTD-0783`, `SPEC_CARRIER_CONSTRAINTS_v1`
**Production impact:** none; ledger reconciliation and static arithmetic

## 1. The correction

FTD-0782 registered this governance claim: *"any carrier result in this
sector is `[ENGINE FACT]`-grade until the reciprocal-transaction problem
(FTD-0545/0546/0549, closed negative in frozen forms) is solved."* Read as
"the movement sector is closed behind an unsolved problem," **this is wrong,
and the registered record says so.** The correction matters because it was
propagated into `SPEC_CARRIER_CONSTRAINTS_v1` (C11), the sidebranch §32.6–32.7
"unopened doors," and the handoff package's priority statement.

What FTD-0545/0546/0549 actually closed is narrow and exactly as titled: the
**frozen fixed-step common action** is not an exact-energy reciprocal law
(FTD-0545's `D = H(p+a) − H(p−a) − 2a c²p/H(p) = −(c⁴M²p/H⁵)a³ + O(a⁵)`, the
central-difference-of-a-nonlinear-dispersion defect), the neutral
self-consistent pair inherits it (FTD-0546, `max|D_pair| = 9.68e-9`, four
orders above the field-algebra residual), and endpoint/midpoint data
underdetermine the spacetime current (FTD-0549). FTD-0549's own row states
the escape: *"the next candidate must solve internal stages atomically; this
does not require a new ontological primitive."*

**That escape was taken, and it worked.** FTD-0551 replaces the fixed-step
map with the **discrete-gradient transaction** `vbar = c²(p0+p1)/(H0+H1)`,
which satisfies `H1 − H0 = vbar · (p1 − p0)` identically — the standard cure
for precisely FTD-0545's obstruction — giving exact total-energy exchange and
projection-free Gauss propagation, conditional on a converged root. The
program then built, on that foundation:

- constituent-complete common actions with exact conditional identities and
  repeated state-only reversible bound motion (FTD-0600/0601/0622/0623);
- a static dressed fixed point with a **positive analytic 48-coordinate
  Hessian** and a genuine stationary state (FTD-0637/0638), plus reversible
  dynamical rest across 512 forward/reverse steps (FTD-0639, worst
  common-action residual `4.07e-14` against a `1e-10` gate);
- internal matter modes (FTD-0640), independent transverse field modes
  (FTD-0641), coupled hybrid response (FTD-0642);
- fixed-mass cross-resolution reciprocal action with coherent co-moving
  dressed transport and **out-of-sample** normalized mobility
  (FTD-0649/0652/0654/0656);
- field-assisted capture, covariant energetic trapping, and a durable
  finite-support relational core with an outgoing tail (FTD-0726–0739).

**Correct governance statement:** the reciprocal transaction is *solved as
`[SELECTED DYNAMICS]`* — a valid reciprocal integrator, not a derived unique
microscopic law (FTD-0551 §3 says so explicitly). Results there are
**selection-scoped**, not blocked. C11 of the carrier spec should read
"native licensing = imposed phenomenology off **and** the selection of the
transaction declared," not "wait for an unsolved problem."

## 2. The actual wall, exactly: C2

With the sector open, the carrier question was asked there — and it failed,
for the reason the carrier spec names as C2 (spectrum avoidance). Three
registered results, in escalating strength:

**(a) Exact `[THEOREM]` — band embedding (FTD-0663).** The matched field
dispersion covers `[0, pi]`; the first internal matter doublet's phase is
`Omega = 1.0911648733663635` per tick, and the one-axis field branch maximum
is `2 arcsin(1/sqrt 3) = 1.2309594173407747` (re-derived here, residual
`1.8e-17`). Since `Omega <` that maximum, the mode sits **strictly inside**
the propagating band, at matching wavenumber `k* = 0.7111037549763191 pi`.
Frequency-gap protection is excluded *exactly*. In carrier-spec terms:

```text
Omega / omega_band = 0.8864     C2 FAILS
```

**(b) Measured — the decay (FTD-0676).** Canonical pre-contact mode energy
decays exponentially at `Gamma_E = 0.00653712` per tick, `R^2 = 0.999332`,
both polarities, over ticks 8–64 — *before* causal return, so it is not a
periodic-boundary artifact. Derived quantities, computed here:

| quantity | value |
|---|---|
| ticks per cycle `2 pi / Omega` | 5.7582 |
| quality factor `Omega / Gamma_E` | 166.92 |
| cycles per energy e-fold | 26.57 |
| amplitude remaining at cycle 8 | 0.860 of launch |

Note that 5.7582 ticks/cycle **is** the corrected FTD-0659 doublet rate
(FTD-0780's amended figure, 5.756) — this is the same object my own
FTD-0780 excluded independently, as harmonic. Two independent kills of one
candidate.

**(c) Confirmed — resonant transfer (FTD-0699), and no screening escape
(FTD-0700–0703).** Both polarities peak within one native phase bin on
`<100>`, `<110>`, `<111>` with current-normalized contrasts 87.4–182.6. The
axial lattice Cherenkov channel exists as a theorem (FTD-0700), the ideal
connected bipole's form factor screens it only **partially** — global
cancellation `[CLOSED NEGATIVE]` (FTD-0701) — and the actual deposited
current suppresses the zone-edge channel to `2.71e-9` but leaves interior
channels open (FTD-0703).

## 3. Why C2 cannot be fixed here: it is C3 wearing a different mask

The obvious repair — raise the amplitude until the frequency climbs above the
band — is unavailable, and the reason is structural rather than
computational. The doublet is a **linear normal mode of a positive-definite
Hessian** (FTD-0637/0638): exponent `n = 2`, harmonic, so its frequency is
**amplitude-independent**. No excitation lifts it. It would need to rise by a
factor of only `1.1281` — and cannot rise at all.

That is exactly the bracket theorem (FTD-0783) speaking in the movement
sector: *wells pin candidates at `n ~ 2`*. The compact-law pair failed the
same way at `omega_0/omega_B = 0.693`; this doublet fails at `0.8864`. Two
structurally unrelated objects — a two-body chemical-type bond and a
16-constituent block's internal normal mode — land on the same side of the
same band for the same reason.

```text
C2 (spectrum avoidance) is unreachable while C3 (intermediate-exponent
confinement) is unfilled. They are not two constraints; C3 is the
mechanism by which C2 could be satisfied, and nothing native supplies it.
```

## 4. Consequence: the two open doors are one door

The sidebranch (§32.7) and the carrier spec both listed two unopened doors:
*(i)* an unidentified intermediate-exponent native mechanism, and *(ii)* the
coupled sector beyond the reciprocal-transaction problem. Door (ii) does not
exist as stated — **the coupled sector was opened, extensively, and the
carrier fails inside it at C2/C3.** The honest inventory is now:

- **One open door:** a native mechanism producing null-flat-bottomed,
  quartic-growth confinement sustained over `a >~ 8` (C3). Every other
  identified route — linear functionals (exact), the affine sector
  (FTD-0781), the pair channel (FTD-0783), generic wells and walls (the
  bracket), and now the fully-built movement sector (this result) — is
  closed.
- The carrier question is therefore **negative at every door that has been
  opened, with exactly one door never opened**, and that door is a single
  well-posed mathematical request rather than a research direction.

This is a stronger and cleaner negative than the program had before, and it
was obtained entirely from the existing record.

## 5. Scope and what is *not* claimed

Nothing here shows C3 is unsatisfiable — no such theorem exists, and the
bracket theorem bounds only *identified* mechanisms (finite wells, capacity
walls, linear normal modes). Nothing here promotes any FTD-0600–0739 result:
they remain `[SELECTED DYNAMICS]`, since the transaction is a selection, and
FTD-0658 already registered that no current candidate supplies an *intrinsic*
rest phase (only an excited one). FTD-0663's own caveat stands: band
embedding plus nonzero transfer does not prove an infinite-volume resonance
or exclude a symmetry-protected embedded mode — but a symmetry-protected
embedded mode is not excluded *and not exhibited*, and FTD-0676's measured
decay is evidence against one for this object. The corrections in §1 are to
FTD-0782's governance sentence only; FTD-0782's source map and FTD-0781's
affine-sector theorem are untouched.
