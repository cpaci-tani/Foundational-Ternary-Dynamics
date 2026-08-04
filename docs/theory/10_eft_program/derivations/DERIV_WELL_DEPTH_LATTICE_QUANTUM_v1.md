# FTD-0788 — The Well Depth from the Lattice Quantum v1

> ## REFUTED 2026-08-03 — see FTD-0790
>
> **Verdict withdrawn.** `z*G(1) = z*G(0) - 1` **identically**, so this
> document's apparatus reduces to `K_GENESIS - 1` and carries no lattice
> information. `K_GENESIS` is not registered exact structure but
> `[SELECTION — ADOPTED]` x `[IMPOSED]` `N_c` x a declared convention; it is a
> **flux-amplitude gate**, not an energy; §3 excludes the field sector and §4
> then uses the Gauss propagator; and C2 is a one-sided threshold that ~26
> competing expressions also clear. FTD-0388's own guard — *"an adoption is
> never a derivation"* — forbids this move.
>
> **§2's claim that `particle_masses.h` mislabels its constants is WRONG:**
> the repo's registered convention is `W_SC = 0.5054620197`, consistent across
> four sources. This document switched to the literature convention and
> reported the repo's correct usage as a defect.
>
> Only `G(0)`, `G(1)` and `G(0)-G(1)=1/6` survive. Retained in full as a
> documented negative result.


**Status:** `[EXACT — LATTICE GREEN'S-FUNCTION IDENTITY AND ENGINE MATCH]` +
`[SELECTION — MOTIVATED: eps = z*G(1) = W_SC - 1]` +
`[CONDITIONAL THEOREM — C2 SATISFIED UNDER THAT SELECTION]` +
`[CLOSED NEGATIVE — FIELD-MEDIATED eps CAN NEVER SATISFY C2]` +
`[OPEN — WHICH ORIGIN THE COMPACT LAW HAS]`
**Verdict:** `EPS_IS_A_LATTICE_QUANTUM_NOT_A_FREE_SELECTION_C2_TURNS_ON_ITS_ORIGIN`
**Parents:** `FTD-0388` (K_GENESIS = N_c*K_MANIFEST), `FTD-0575` (native force,
`[OPEN]` dynamic common action), `FTD-0739`, `FTD-0783`, `FTD-0786`, `FTD-0787`
**Production impact:** none — no engine constant is changed by this document

## 1. Result in one sentence

The selected well depth `eps = 0.01` matches **no scale the framework
contains** — it sits 18x above the field-mediated scale and 51.6x below the
lattice nearest-neighbour quantum — and the two principled candidates for its
origin give **opposite answers to the carrier question**: a field-mediated
`eps` fails C2 by a factor of ~400 and forbids a carrier permanently, while
the matter-sector lattice quantum `eps = W_SC - 1 = 0.5163860592` **satisfies
C2 with a clean window**, so the entire carrier programme now turns on one
well-posed question: *where does the compact law come from?*

## 2. `eps = 0.01` is an orphan

| scale | value | ratio to `eps = 0.01` |
|---|---|---|
| field-mediated `E_F(1)` (FTD-0739 measured unit-dipole field energy) | `0.000561` = `0.0769 alpha` | `eps` is **17.8x above** |
| `K_MANIFEST = W_SC/3` (engine) | `0.505462` | `eps` is **51x below** |
| `W_SC - 1` (nearest-neighbour lattice quantum) | `0.516386` | `eps` is **51.6x below** |
| `K_GENESIS = W_SC` (engine) | `1.516386` | `eps` is **152x below** |

It is not a compromise between them; it lies in a gap where nothing in the
framework lives. The parent derivation states it plainly — "The selected well
depth is `epsilon=0.01`" — with no derivation offered.

## 3. The compact law is a matter-sector term, not a field-mediated one

Two independent reasons, both from the registered record:

1. **Compact support.** `V(q) = 0` for `q >= 3/2`, i.e. the interaction
   vanishes identically beyond `r = sqrt(3/2)`. A field-mediated Coulomb
   interaction is long-ranged and never compactly supported. The emergent
   Coulomb channel is a *separate* structure (spine §6; FTD-0575).
2. **Separate bookkeeping.** FTD-0739's certified control records
   `pair energy = -0.00956` and `field energy = +0.00056` as **distinct
   entries** whose sum is the total. The pair term is `~ -eps` (the pair sits
   near its minimum); the field term is three orders smaller and tracked
   independently. The parent derivation's trilemma likewise treats `E_F(1)`
   and `eps` as separate additive contributions, eq. (12).

So the compact well is a **contact term of the matter sector**, and its scale
should be a matter-sector scale — not `alpha`.

## 4. The lattice quantum, exactly

The engine's matter thresholds are already lattice Green's-function
quantities. With `z = 6` the simple-cubic coordination and `G` the SC lattice
Green's function:

```text
z * G(0) = W_SC = 1.516386059151978...   ==  engine K_GENESIS  (residual 1.8e-17)
K_MANIFEST = K_GENESIS / N_c = W_SC / 3  ==  0.505462019717326  (engine, FTD-0388)
```

verified against the Glasser–Zucker closed form
`W_SC = (sqrt6/32 pi^3) Gamma(1/24)Gamma(5/24)Gamma(7/24)Gamma(11/24)`.

The discrete Laplacian at the origin gives the **exact** companion identity

```text
z [ G(0) - G(1) ] = 1     =>     z * G(1) = W_SC - 1 = 0.5163860591519780...
```

`z*G(0)` is the **self**-energy quantum — the registered manifestation
threshold. Under the same normalisation `z*G(1)` is the **nearest-neighbour**
quantum. The compact law's minimum sits at exactly `r0 = 1`, the nearest
neighbour. That gives the motivated selection

```text
eps  =  z * G(1)  =  W_SC - 1  =  0.5163860592          [SELECTION — MOTIVATED]
```

exact, parameter-free, and 51.6x the currently selected value. **This is an
analogy from a registered exact identity, not a derivation of the compact law
— see §7.**

## 5. C2 under each candidate

The flexural mode of FTD-0787, `V(d) = -2 eps + 24 eps d^4 - 32 eps d^6`, with
`Omega ~ sqrt(eps)` and an `eps`-independent purity window
(`contamination = (4/3) d^2`). The question is whether an amplitude range
exists that is **simultaneously above band and cleanly quartic**:

| `eps` | origin | field band `1.2310` | wave band `2.0000` |
|---|---|---|---|
| `0.000561` | field-mediated | **no window** (`Omega_max = 0.095`) | **no window** |
| `0.01` | selected | **no window** (`Omega_max = 0.401`) | **no window** |
| `0.505462` | `K_MANIFEST` | window `A in [0.177, 0.704]`, **4.2%** contamination at entry | window, 12.0% |
| `0.516386` | `W_SC - 1` | window `A in [0.173, 0.704]`, **4.0%** contamination at entry | window, 11.8% |
| `1.516386` | `K_GENESIS` | window `A in [0.103, 0.704]`, **1.4%** | window `A in [0.166, 0.704]`, **3.7%** |

At `eps = W_SC - 1` the mode is above the field band over **24–100% of the
separatrix** while the quartic law is accurate to 4% at the window's entry.
**C2 is satisfied, with a usable clean-quartic window.** At `eps = K_GENESIS`
both bands clear comfortably.

## 6. The fork, stated sharply

```text
compact law is FIELD-MEDIATED   ->  eps ~ 0.077 alpha ~ 5.6e-4
                                ->  Omega_max = 0.095 vs band 1.231
                                ->  short by ~400x; NO CARRIER, EVER.

compact law is a MATTER-SECTOR  ->  eps = W_SC - 1 = 0.5164 (or W_SC)
contact term at the lattice     ->  above-band window with 4% contamination
quantum                         ->  C2 SATISFIED; the carrier is viable.
```

The two candidate origins differ by a factor of **~2700**, and they answer the
carrier question in opposite directions. Everything the programme has been
pursuing therefore reduces to one question it has already registered as open:
**derive the compact law from the native force** — FTD-0575's
`[OPEN — EXACT DYNAMIC ENERGY/MOBILE CARRIER]`, and the same "Mechanism B"
gap flagged in `gauge_couplings.h` for `g_c` itself. That is now the single
most decisive open problem in the programme, and it is a *derivation*
problem, not a campaign.

## 7. What is NOT claimed — scrupulously

- **This is not a derivation of `eps`.** It is an exact lattice identity
  (`z*G(1) = W_SC - 1`) plus a motivated identification of that quantum with
  the nearest-neighbour bond depth, resting on the analogy with the
  *registered, exact* `K_GENESIS = z*G(0)`. It is `[SELECTION]`-grade. The
  house rule against promoting reformulations applies: nothing here proves
  the compact law's coefficient.
- **The compact law's shape is itself selected** — its parent document is
  tagged `[SELECTED NO-NEW-PRIMITIVE PATH; NOT IMPLEMENTED]`. This sets the
  scale of an already-selected form.
- **No engine constant is changed.** `eps = 0.01` remains what the code and
  every registered campaign used.
- **Every FTD-0600–0739 quantitative result was obtained at `eps = 0.01`.**
  Their internal consistency is unaffected, but every *ratio* between matter
  and field quantities in them would change by up to 51.6x under this
  selection. Nothing here retro-fits or invalidates them; it does mean the
  matter campaign was run at a scale the framework does not justify.
- **Raising `eps` worsens the collapse tension.** The parent derivation's
  trilemma (eq. 12) has coefficient `E_F(1) - eps`, already negative at
  `eps = 0.01` and 51.6x more negative here. That channel is blocked by the
  ternary site-capacity constraint rather than by energetics, and the parent
  document already says the unrestricted all-pairs promotion "is not a
  material thermodynamic limit" — but the saturation mechanism must now carry
  proportionally more weight. `[OPEN]`.
- **C2 is necessary, not sufficient.** Clearing the band does not deliver a
  carrier: C5 (drain-free window against `K_GENESIS`) is unevaluated and
  becomes *tighter* as `eps` approaches `K_GENESIS`, the trimer's in-band
  longitudinal modes (FTD-0787 §9) still exist, and no campaign has run.
