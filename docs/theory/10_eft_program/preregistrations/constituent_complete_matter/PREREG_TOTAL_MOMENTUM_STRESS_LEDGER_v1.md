# FTD-0769 — Total momentum stress ledger v1

**Status:** `[PRE-REGISTRATION — LOCKED; NOT RUN]`
**Protocol lock:** `protocol_sha256=215B03A85A76B706E91099CA24E276FAC3B57DE3852353981456F79F411D8A13`
**Date:** 2026-08-02 (rework of the 2026-08-02 first draft, after independent
adversarial mathematical and physical review; every defect either repaired by
derivation or explicitly disclosed in §11)
**Parent:** FTD-0768 `LONG_TRANSPORT_EXECUTION_INVALID` — construction protocol
**and** quantitative premise. Its numeric artifact **is** used here, as
pre-execution sizing prior and as the source of the premise numbers in §1; it
carries no FTD-0768 physics status of its own (that run failed its
forward/reverse recovery gate) and §6 G8 registers the comparability check.
Chained through FTD-0761 `M4_BOOSTED_RELATIONAL_TRANSPORT_WITNESS`-family
(connected two-polarity core, boosted transport discovery) and FTD-0764
(transported chart morphology, `NO_TRANSPORTED_FIELD_COHERENCE`).
**Derivation basis:** §2 of this document. All algebra used by this campaign is
inlined and proved below, and every stated residual was reproduced on
synthetic periodic fixtures at the tolerances quoted in §2.11. No load-bearing
step is delegated to an unregistered or ephemeral source; §12 requires
`scripts/proofs/proof_total_momentum_stress_ledger.py` to re-run the same
fixtures before the hash is taken.
**Scope:** the whole-domain change of the selected local translation
pseudomomentum `P_i` for the FTD-0761/0763/0768 moving relational core is an
exact consequence of the matter current (§2.1). What is **not** known is where
in space that change resides, and whether an independently constructed local
current transports it. This campaign measures that, under two declared
localizations, on a co-moving radius scan. No claim of a unique continuum
momentum, no Lorentz claim, no production claim, no "substrate is the
reservoir" claim.
**Production change:** forbidden

---

## 0. Frozen operators (unchanged from `matched_gauss_transport.cpp:183–243`)

With `T_r f(v) = f(v+r)` on the periodic `L=321` torus,

```text
d_a^- = I - T_{-e_a},           d_a^+ = T_{e_a} - I,
(CB)_a  = eps_{abc} d_b^- B_c,  (C^T E)_a = eps_{abc} d_b^+ E_c,
D_i     = (1/2)(T_{e_i} - T_{-e_i})   componentwise,
lambda  = C_SPEED * dt = 0.57735026918962576451 * 0.25 = 0.14433756729740643.
```

`C_SPEED` is quoted here to the engine's own literal (`constants.h`), not to a
freshly-rounded `1/sqrt(3)`: the two agree to 15 digits but differ in the
16th (`...643` vs a naive re-rounding's `...646`), and `interaction_scale`
below is byte-exact only against the engine literal (build-instrumentation
finding, 2026-08-02) — this document freezes to the engine, not to an
independent re-derivation of the same constant.

The staggered step (`matched_face_energy_transaction.h:150–172`) is

```text
B' = B - lambda C^T E,      E' = E + lambda C B',      E'' = E' - K.
```

Write `u := C B'` (a face field), `w := C^T E` (an edge field), `M := C C^T`
(face to face), `M' := C^T C` (edge to edge). Note `M != M'`. The tested
candidate is the unchanged header-level definition
(`matched_face_momentum_transaction.h:14`, comment verbatim: *"a selected
minimal local pseudomomentum, not a unique continuum momentum or a production
particle-recoil law"*):

```text
P_i(E,B) = <E, D_i C B>.
```

`matched_local_translation_momentum` (`matched_face_momentum_transaction.h:76`)
sums this over the **entire domain**. There is no coded density and no coded
region. Both are introduced fresh by this pre-registration and are
`[SELECTION]`, not forced — see §2.4, §2.5 and §6 G3.

`P_i` is one member of the family `{<E, A C B> : A^T = -A, [A, C C^T] = 0}`,
symbol `i*sin(k_i)`. Every verdict in §7 closes or supports `D_i` specifically,
never "lattice momentum" as a class.

---

## 1. Locked question

The premise, recomputed from `engine/results/ftd_0768/
ftd_0768_long_transport_dynamic_response_v1.json` and quoted exactly:

| `tau` | `Delta p_matter,z` | `Delta P_local,z` | `D_z` (their sum) | `|D_z|/|p_matter,z|` |
|---|---|---|---|---|
| 64 | −7.1668e-4 | −1.0156e-4 | −8.1823e-4 | 1.38% |
| 192 | −2.8660e-3 | −4.4104e-4 | −3.3070e-3 | 5.79% |
| 256 | −3.3701e-3 | −6.9742e-4 | −4.0676e-3 | 7.18% |
| 512 | −3.8028e-6 | −1.3139e-3 | −1.3177e-3 | 2.20% |
| 768 | −2.5887e-3 | −2.0044e-3 | −4.5931e-3 | 8.00% |

(The number `2.6e-3` that appeared in the first draft's framing sentence was
misattributed: `2.5887e-3` is `|Delta p_matter,z|` alone and `2.6185e-3` is the
*spline* candidate's defect, a candidate this campaign does not test. The local
candidate's own non-closure at `tau=768` is `4.5931e-3`, and that is the number
this campaign is about.)

The wider record, characterized correctly (the first draft's "`~4.5%-8%` of
`p_matter` in the FTD-0761/0764/0768 record" was wrong on two counts — it
quoted only the uncertified run, and even for that run the range is
`1.4%-8.0%`, not `4.5%-8%`):

| source | status | boost / horizon | cumulative defect | vs `p_matter` |
|---|---|---|---|---|
| FTD-0761 | certified | `q=0.015`, 224–416 ticks | 6.97e-3 – 1.66e-2 | 23% – 55% |
| FTD-0764 | certified | `q=0.015` | 6.92e-3 – 8.90e-3 (local+spline, all channels) | 23% – 30% |
| FTD-0768 | **execution-invalid** | `q=0.030`, 768 ticks | 8.2e-4 – 4.59e-3 | 1.4% – 8.0% |

The §6.6 G5 thresholds are **not** sized from this table. The first draft sized
them from the mischaracterized range; §2.9 replaces that with landmarks derived
from the identity itself, so the only surviving dependence on the table is the
reported `rho_i` diagnostic and the §6.9 G8 comparability flag.

By §2.1 the field-side book is exact: `Delta P_i = -Q_i` where `Q_i` is the
accumulated matter-current source term. The defect `D_i = Delta p_matter,i +
Delta P_i = Delta p_matter,i - Q_i` is therefore a mismatch between two
bookkeepings that are **both** constructed at `supp(K)`. Asking "is the defect
localized at the core" is, in that reading, close to tautological, and the
first draft's framing did not say so. The non-tautological question — and the
one locked here — is about the spatial residence of the field-side change:

> For the registered moving relational core (FTD-0761/0763 connected
> two-polarity parent, boosted `q=+0.030` along `(0,0,1)`, formation 160 ticks
> + preparation age 128 ticks + discovery 768 ticks, as constructed for
> FTD-0768):
>
> **(Q1)** Does the whole-domain change `Delta P_i(tau)` of the selected local
> translation pseudomomentum remain inside a co-moving Chebyshev control
> volume `Omega_R` around the core, cross its boundary at an `R`-independent
> rate, or accumulate in the near zone between the tested radii? The
> discriminating observable is the retention fraction `eta_i(R,tau)` of §2.9,
> whose landmark values `1` and `0` are derived, not chosen.
>
> **(Q2)** Is the magnitude of that boundary transfer commensurate with the
> non-closure `D_i(tau)`, i.e. is there a core-associated transport structure
> at the scale of the missing momentum? (Reported diagnostic `rho_i`, §2.9;
> §2.9 also derives a ceiling on `rho_i` that holds whenever `|eta-1| <= 1` —
> not unconditionally, since OVER_DEPLETING fixtures with `|eta-1| > 1` sit
> outside it — which is one more reason `rho_i` is not a verdict gate.)
>
> **(Q3)** Do the matter-side change `Delta p_matter,i` and the field-side
> source term `Q_i` even carry the same sign? (Exchange-sign flag, §2.10.)

Whether the regional ledger "closes" is not a legitimate locked question: the
regional identity (§2.7) is a `[THEOREM]`, so measuring it can fail only on
**implementation** (wrong sign, wrong stencil, mis-paired snapshots — the
FTD-0768 failure mode), never on physics. The identity is used as an
instrument, gated at G0/G1, and never as a discriminator.

---

## 2. The mathematics, inlined and proved

Everything in this section is elementary and self-contained. It is reproduced
here rather than cited so that a future reader can verify the algebra from the
committed corpus alone.

### 2.1 Global identity `[THEOREM]`

`D_i` is skew (`D_i^T = -D_i`, requires periodicity) and commutes with every
translation-invariant operator, in particular with `C` and `C^T`; `M = C C^T`
is symmetric, so `D_i M` is skew: `(D_i M)^T = M^T D_i^T = -M D_i = -D_i M`.
Then

```text
P_i(E',B') = <E + lambda u, D_i u>          [E' = E + lambda C B' = E + lambda u]
           = <E, D_i u> + lambda <u, D_i u>
           = <E, D_i u>                      [<u, D_i u> = 0, skewness]
           = <E, D_i C B> - lambda <E, D_i M E>   [C B = u + lambda M E]
           = P_i(E,B)                        [<E, D_i M E> = 0, D_i M skew]
```

and with the conservative current update `E'' = E' - K`,

```text
Delta P_i = -<K, D_i C B'>.                                            (G)
```

Verified to `<= 3.5e-14` on all three axes, source-free and with a compactly
supported current (§2.11 rows B).

### 2.2 Restriction lemma `[THEOREM]`

Let `N` be finite-range, translation-invariant and skew-adjoint on the
periodic torus, `N = sum_r N_r T_r` with `N_r` a 3x3 component matrix, so that
`(Nf)_a(v) = sum_{r,b} N_r[a][b] f_b(v+r)`.

*Transpose.* `<g, Nf> = sum_{v,a,b,r} g_a(v) N_r[a][b] f_b(v+r)`; substituting
`v' = v+r` gives `(N^T g)_b(v) = sum_r (N_r^T)[b][a] g_a(v-r)`, i.e.
`N^T = sum_r N_r^T T_{-r}`. Skewness `N^T = -N` therefore forces

```text
N_{-r} = -N_r^T,                                                       (S)
```

and in particular `N_0` is antisymmetric, so its diagonal vanishes.

**`R+` representative rule (frozen; the first two rework passes left this
unstated, a gap surfaced only by the build).** Pair `(r,a,b)` with
`(-r,b,a)`; keep the member whose key `(r_x,r_y,r_z,a,b)` is lexicographically
greater. This is well-defined because `N_0`'s vanishing diagonal means no
`r=0` term is ever ambiguous between the two. Any other consistent tie-break
gives the same `Phi` (the chord form (R) is representative-independent by
construction — only the internal `T^(i)`/`S^(i)` split of Section 2.3
depends on the choice, per Banned move B5), so this is a hash-lock
formality, not a physics choice — but B5 still forbids changing it once the
exactness pre-check of Section 6.4 is frozen.

*Chord form.* For any per-component mask `chi_a(v) in {0,1}`, pair the term
`(r,a,b)` with `(-r,b,a)`. Shifting the second by `r` and using (S),

```text
<chi f, N f> = sum_{(r,a,b) in R+} sum_v N_r[a][b] f_a(v) f_b(v+r)
                 * [ chi_a(v) - chi_b(v+r) ]                           (R)
```

with `R+` one representative per pair. Every summand carries the factor
`chi_a(v) - chi_b(v+r)`, which vanishes unless the chord `(a,v) -> (b,v+r)`
straddles the region boundary in the augmented `(component, site)` graph. The
expression is exactly odd under `chi -> 1-chi`, so
`Phi(dOmega) + Phi(dOmega^c) = 0` termwise.

Verified: (S) holds exactly (`max |N_{-r} + N_r^T| = 0`) for `N = D_i`,
`D_i C C^T` and `D_i C^T C`; (R) reproduces the direct masked bilinear to
`<= 3.6e-14` for all four operators used below, under both a site mask and a
genuinely per-component mask (§2.11 rows F).

### 2.3 Unit-bond form and the site-mask collapse `[THEOREM]`

Split the chord factor by walking `v -> v+r` in unit steps along a fixed
lexicographic path `p_0 = 0, ..., p_{|r|_1} = r` (`x` steps, then `y`, then
`z`; this path convention is frozen and hash-locked, Banned move B5):

```text
chi_a(v) - chi_b(v+r) = sum_k [ chi_a(v+p_k) - chi_a(v+p_{k+1}) ]
                        + [ chi_a(v+r) - chi_b(v+r) ].
```

Collecting the first group on canonical unit bonds `(v, v+e_d)` and the second
on sites gives

```text
Phi_i[chi] = sum_{a,d,v} T^(i)_{a,d}(v) [ chi_a(v) - chi_a(v+e_d) ]
           + sum_{a,b,v} S^(i)_{a,b}(v) [ chi_a(v) - chi_b(v)     ]    (U)
```

with, writing `W_{r,a,b}(v) := N_r[a][b] f_a(v) f_b(v+r)`,

```text
T^(i)_{a,d}(v) = sum over (r,a,b) in R+ and path steps p_k -> p_k +- e_d
                 of  (+-1) * W_{r,a,b}(v - base),   base = p_k (step +e_d)
                                                    base = p_{k+1} (step -e_d)
S^(i)_{a,b}(v) = sum over (r,a,b) in R+  of  W_{r,a,b}(v - r).
```

`T^(i)` is the discrete momentum-current array — this document's "stress
ledger", the honest discrete analogue of `T_{id}` contracted with an outward
face normal. `S^(i)` is an on-site component-crossing current with no energy
analogue (a discrete Belinfante/spin part).

**Site-mask collapse.** If the mask is component-independent,
`chi_a(v) = chi(v)`, then `chi_a(v) - chi_b(v) = 0` for every `(a,b)` and the
entire `S^(i)` term drops out:

```text
chi component-independent  =>  Phi_i[chi] = sum_{a,d,v} T^(i)_{a,d}(v)
                                            [ chi(v) - chi(v+e_d) ],
```

a pure sum over unit lattice bonds crossing `dOmega`. The campaign's regions
are Chebyshev cubes of sites, so this is the form the production instrument
actually evaluates. Verified numerically: the `S`-channel contribution is
exactly `0.000e+00` under every site mask tested, and non-zero
(`-3.1e+1`, `-2.1e+1`, `-5.3e+1` for `i = x,y,z`) under the per-component mask
(§2.11 rows F). Because the production path never exercises `S^(i)`, §6 G3
**requires** the `L=11` pre-check to run at least one genuinely per-component
mask, so the array is not untested dead code inside a hash-locked protocol.

### 2.4 Localization L1 ("E-carries") and its regional identity M1 `[THEOREM]`

Select the density `pi_i^(1)(a,v) := E_a(v) (D_i C B)_a(v)`, masked at `E`'s
site. Then, using `C B = u + lambda M E` and `E'' = E + lambda u - K`,

```text
sum_{a,v} chi_a(v) [ pi_i^(1),after - pi_i^(1),before ]
   = <chi E'', D_i u> - <chi E, D_i C B>
   = <chi E, D_i u> + lambda <chi u, D_i u> - <chi K, D_i u>
     - <chi E, D_i u> - lambda <chi E, D_i M E>
   = lambda <chi u, D_i u> - lambda <chi E, D_i M E> - <chi K, D_i C B'>. (M1)
```

The first two terms are masked bilinears of a skew operator against a single
field, so by §2.2 each is a pure chord sum straddling `dOmega`:

```text
Phi_i^(u)[chi] := <chi u, D_i u>            N = D_i,        field u = C B'
Phi_i^(E)[chi] := <chi E, D_i M E>          N = D_i C C^T,  field E (before)
```

The source term is masked directly on `K`, so its regional support is exactly
`supp(K) ∩ Omega`. Verified to `<= 1.6e-14` on all three axes.

### 2.5 Localization L2 ("B-carries") and its regional identity M2 `[THEOREM]`

The scalar transpose `<E, D_i C B> = <(D_i C)^T E, B> = -<C^T D_i E, B>` gives
the alternate density

```text
pi_i^(2)(a,v) := -B_a(v) (C^T D_i E)_a(v),
```

masked at `B`'s site. `L1` and `L2` sum to the identical whole-domain total by
construction (verified to `<= 2.9e-14`), but differ as **regional** splits
because `chi` is applied at different sites for the two factorizations.

The first draft asserted that `L2` could be measured with `L1`'s flux pair.
It cannot. Direct check on a random fixture: the `L1` right-hand side misses
the `L2` masked change by `7.5e-1`, `8.3e0`, `1.1e1` for `i = x,y,z`, against
`|L2 change|` of `3.4e0`, `9.7e0`, `2.4e0` — an `O(1)` failure, not a
tolerance question. `L2` needs its own decomposition, derived here.

Write `B = B' + lambda w` with `w = C^T E`, and `E'' = E' - K` with
`E' = E + lambda u`. Then

```text
sum_{a,v} chi_a(v) [ pi_i^(2),after - pi_i^(2),before ]
   = -<chi B', C^T D_i E''> + <chi B, C^T D_i E>
   = -<chi B', C^T D_i E'> + <chi B', C^T D_i K>
     + <chi B', C^T D_i E> + lambda <chi w, C^T D_i E>
   = -lambda <chi B', C^T D_i u> + <chi B', C^T D_i K>
     + lambda <chi w, C^T D_i E>.
```

Now use `[D_i, C^T] = 0` (verified to `<= 1.8e-15`): `C^T D_i E = D_i C^T E =
D_i w`, and `C^T D_i u = C^T D_i C B' = D_i C^T C B' = D_i M' B'`. Hence

```text
sum_{a,v} chi_a(v) [ pi_i^(2),after - pi_i^(2),before ]
    = lambda * Phi_i^(w)[chi] - lambda * Phi_i^(B')[chi]
      + <chi B', D_i C^T K>                                            (M2)
```

with

```text
Phi_i^(w)[chi]  := <chi w,  D_i w>            N = D_i,        field w = C^T E (before)
Phi_i^(B')[chi] := <chi B', D_i M' B'>        N = D_i C^T C,  field B' (after)
```

`D_i M'` is skew because `M'^T = (C^T C)^T = C^T C = M'`, so §2.2 applies to
both terms. Verified to `<= 9.8e-15` on all three axes.

Note the exact mirror of M1: M1 places the plain-`D_i` flux on the *after*-side
object `u` and the `D_i M` flux on the *before*-side object `E`; M2 places the
plain-`D_i` flux on the *before*-side object `w` and the `D_i M'` flux on the
*after*-side object `B'`. This is the staggering of the step map, read from the
other end.

The two source terms agree globally — `<B', D_i C^T K> + <K, D_i C B'> = 0` to
`<= 1.8e-15` — but not regionally: L1 masks `K` itself, while L2 masks `B'`
against `D_i C^T K`, whose support is `supp(K)` dilated by the `l_inf` reach of
`D_i C^T`, which is **2** (§2.6). This is a real difference in regional source
attribution and is one of the two declared `[SELECTION]`s; §6 G4 turns it into
a clearance requirement rather than leaving it implicit.

### 2.6 Chord census and true reach `[THEOREM]`, computed not asserted

Impulse-response extraction of `N_r` on an `L=11` torus gives:

| operator | nonzero displacements `r` | nonzero `(r,a,b)` entries | `R+` classes | max `|r|_1` | max `|r|_inf` |
|---|---|---|---|---|---|
| `D_i` | 2 | 6 | 3 | 1 | 1 |
| `D_i C C^T` (L1 `E` flux) | 25 | 74 | 37 | 3 | **2** |
| `D_i C^T C` (L2 `B'` flux) | 25 | 74 | 37 | 3 | **2** |
| `D_i C^T` (L2 source dilation) | 8 | 24 | — | 2 | 2 |
| `C C^T` (one-step field cone) | 13 | 39 | — | 2 | 1 |

For `D_i C C^T` (and identically for `D_i C^T C`) the **displacements** split by
`|r|_1` as `1, 6, 10, 8` (sum 25) and the **`R+` chord classes** split as
`2, 11, 20, 4` (sum 37, i.e. 74 entries paired). The first draft printed the
second breakdown as if it were a breakdown of the first; both numbers were
individually right and the sentence was wrong. Table-sizing for the
implementation follows the 37/74 column, not the 25 column.

The `l_inf` reach is **2**, not 3: `|r|_1 <= 3` does not imply `l_inf` reach 3,
and the actual maximum per-axis displacement is 2 (along the `D_i` axis) and 1
on the other two. Consequently the flux of the binding operator lives on the
**two outermost layers** `R-1, R` of `Omega_R`, and the flux of the plain-`D_i`
operator on the single layer `R`. Every §6 statement about shells uses reach
`t = 2`.

### 2.7 The cumulative moving-mask ledger — the registered object `[THEOREM]`

The mask moves with the core, so the object that telescopes is the regional
content evaluated with the contemporaneous mask,

```text
Pi_i(R,tau) := sum_{a,v} chi_{Omega_R(tau)}(a,v) * pi_i^tau(a,v).
```

Split each tick with the FTD-0768 pairing convention (material term uses the
**new** mask with both snapshots; sweep term uses the **old** field with the
mask difference):

```text
Pi_i(R,t+1) - Pi_i(R,t)
   = sum[ chi_{t+1} (pi^{t+1} - pi^t) ]  +  sum[ (chi_{t+1} - chi_t) pi^t ].
```

The first bracket is exactly M1 (or M2) evaluated at `chi = chi_{t+1}`.
Summing over `t = 0 .. tau-1` telescopes to the registered identity:

```text
Pi_i(R,tau) - Pi_i(R,0)  =  F_i(R,tau) + W_i(R,tau) - Q_i(R,tau)       (L)
```

with the three accumulators, all summed **tick by tick from the boost**:

```text
F_i(R,tau) := sum_{t<tau} lambda * ( Phi_i^(u)[chi_{t+1}] - Phi_i^(E)[chi_{t+1}] )
                                              [L1; L2 uses Phi^(w), Phi^(B')]
W_i(R,tau) := sum_{t<tau} sum[ (chi_{t+1} - chi_t) pi_i^t ]
Q_i(R,tau) := sum_{t<tau} <chi_{t+1} K_t, D_i C B'_t>   [L1]
                          ( -<chi_{t+1} B'_t, D_i C^T K_t>  for L2 )
```

`F_i` is the accumulated net **inflow** of regional pseudomomentum through
`dOmega_R` by field transport; `W_i` is the accumulated **convective** gain from
the control volume sweeping over new sites; `Q_i` is the accumulated removal by
the matter current inside the region. The orientation "into `Omega`" is not a
convention imposed here: it is read off (L), in which `F + W` is precisely what
the boundary contributes to the increase of the regional content.

**This choice — cumulative from the boost, accumulated every tick — is the
resolution of the first draft's central defect.** The first draft compared a
per-step flux against a cumulative defect, a rate against a total; the ratio
was then meaningless and, at the observed scales, would have forced one verdict
bucket regardless of physics. The alternative repair (make the denominator
per-step) was rejected because the already-measured reference columns
(`Delta p_matter`, `Delta P_local`) are cumulative-from-formation and the
campaign must remain commensurate with them. Accumulating instead at
checkpoints only is not available: checkpoints sample a small subset of ticks
and (L) telescopes only if every tick is included. The instrumentation
requirement in §4 therefore mandates per-tick accumulation explicitly, and this
also removes the first draft's hop-alignment problem entirely (see §5).

Verified on a 24-tick moving-mask fixture with a moving compact current:
residual of (L) `<= 8.9e-16` on all three axes. Whole-domain limit `chi ≡ 1`:
`F_i = 0` to `<= 1.7e-15`, `W_i = 0` identically, and `Delta P_i + Q_i = 0` to
`<= 4.5e-16` (§2.11 rows G).

### 2.8 Shell corollary `[THEOREM]` — the H4 instrument

(L) is linear in `chi`, and `chi_{shell} = chi_{R2} - chi_{R1}` for `R1 < R2`.
If `supp(K)` (dilated as in §2.5 for L2) lies inside `Omega_{R1}` at every
tick, then `Q_i(R2,tau) = Q_i(R1,tau)` exactly, and subtracting the two
instances of (L) gives

```text
[F_i(R2) - F_i(R1)] + [W_i(R2) - W_i(R1)]
        = [Pi_i(R2,tau) - Pi_i(R2,0)] - [Pi_i(R1,tau) - Pi_i(R1,0)]    (H)
```

— the difference of the two boundary transfers equals the shell's own content
change, measured separately. This is a genuine, independently checkable
identity and not the banned residual-as-flux move: both `F` terms are
constructed from `T^(i)`/`S^(i)` on straddling bonds, and (H) checks their
difference against a quantity computed from the density directly.

Verified: `Q_i(6) - Q_i(4) = 0.000e+00` exactly (source enclosed) and the
residual of (H) `<= 6.7e-16` on all three axes (§2.11 rows H).

### 2.9 Retention normalization and the no-third-option corollary `[THEOREM]`

Let `R` be a radius that encloses the (dilated) source at every tick, so
`Q_i(R,tau) = Q_i(infinity,tau) = -Delta P_i^whole(tau)` by §2.1. Define

```text
eta_i(R,tau)  := [ Pi_i(R,tau) - Pi_i(R,0) ] / Delta P_i^whole(tau)   (retention)
tau_i(R,tau)  := [ F_i(R,tau) + W_i(R,tau) ] / Delta P_i^whole(tau)   (transfer)
```

Substituting into (L),

```text
eta_i(R,tau) = 1 + tau_i(R,tau)          i.e.   eta_i - tau_i ≡ 1.     (N)
```

Verified to `<= 1.5e-13` at two radii on the fixture, and the shell form
`eta(R2) - eta(R1) = tau(R2) - tau(R1)` to `<= 1.2e-13` (§2.11 rows J).

(N) has four consequences, and they replace the first draft's asserted
discriminators wholesale.

**(i) The landmarks are derived, not chosen.** `eta = 1` means the entire
whole-domain field-pseudomomentum change sits inside `Omega_R` and nothing
crosses (`tau = 0`): a **retained/bound** carrier. `eta = 0` means the regional
content is unchanged while the source drains `Q` from it, so the boundary must
resupply exactly `Q` (`tau = -1`): a **through-flowing** carrier, the region
acting as a replenished sink. These are the only two values fixed by the
identity; the bands around them in §6 G5 are widths, not locations.

**(ii) `R`-flat flux is the signature of transport, not of delocalization.** By
(H), `tau(R2) = tau(R1)` if and only if the shell between them holds no net
content change. A compactly supported source feeding a steadily outflowing
current gives exactly that: what leaves through `R1` crosses `R2`, so the flux
is `R`-independent — the discrete Gauss's-law statement. The first draft
assigned `R`-flat flux to `DELOCALIZED_ARTEFACT` and decaying flux to
`LOCAL_CARRIER`. This is backwards. Decay of `|tau|` with `R` means, by (H),
that the shell is **absorbing** the transfer: content is piling up between the
radii — a growing near-zone dressing, which is the least core-localized of the
three regimes, not the most.

**(iii) There is no "nothing anywhere" bucket.** By (N), `eta` and `tau` differ
by exactly `1`, so `|eta| + |tau| >= 1` always. A configuration with both
`eta ≈ 0` (region holds none of the change) and `tau ≈ 0` (nothing crossed the
boundary) is algebraically impossible once the source is enclosed. The first
draft's `DELOCALIZED_ARTEFACT` verdict — "the defect is measured and quantified
but does not concentrate anywhere" — names a configuration the identity
forbids. What is *not* forbidden, and is a real possible outcome, is
`eta ≈ 0` with `tau ≈ -1`: everything crossed out. That is a **located**
carrier, just not a bound one, and it must not be reported as an artefact. The
verdict names in §7 are rewritten accordingly. (On the fixture, `|eta| + |tau|`
came out at 113.5, 166.6 and 74.6 — far from the bound, as expected for a
random background where the regional content change is dominated by field noise
rather than by the source. That is itself the reason for the rest-arm control
in §6 G7: `eta` is interpretable only when the regional content change is
source-dominated.)

**(iv) `rho` cannot be a verdict gate.** The first draft's coverage ratio,
repaired to be cumulative-over-cumulative, is

```text
rho_i(R,tau) := | F_i(R,tau) + W_i(R,tau) | / max( |D_i(tau)|, 1e-9 )
              = |eta_i - 1| * |Delta P_i^whole(tau)| / |D_i(tau)|.
```

The factor `|Delta P^whole| / |D|` is already on record and is **at most
1.166** over the whole window, dropping to `0.171` at `tau = 256` and `0.113`
at `tau = 128`. Requiring `rho >= 0.5` at every `tau >= 256`, as the first
draft did, therefore demanded `|eta - 1| >= 2.92` at `tau = 256` and
`>= 4.43` at `tau = 128` — the region would have to lose three to four times the
whole domain's change. Combined with the draft's simultaneous "decay with `R`"
clause, the `LOCAL_CARRIER` bucket as drafted selected a pathological
over-depletion, not localization. `rho_i` is retained in §6 as a **reported
diagnostic** answering (Q2), with its derived ceiling reported alongside, and
gates nothing.

### 2.10 Exchange-sign corollary `[THEOREM]`

By (N), `sign(F_i + W_i)` relative to `sign(Delta P_i^whole)` equals
`sign(eta_i - 1)` exactly. The sign of the boundary transfer therefore carries
**no information not already in `eta_i`**, and cannot be an independent verdict.
The first draft's `SIGN_ANTICORRELATED` test — `sign(Phi_i) != sign(Delta
p_matter,i)` — is retired as a bucket for that reason, and for a second: it
compared the transfer against `Delta p_matter,i`, which enters (L) only through
the unexplained defect, so the test mixed the transport question with the
non-closure question.

What *is* independent, and is a genuine test, is the sign relation between the
two books themselves:

```text
Delta p_matter,i(tau)   vs   Q_i(infinity,tau) = -Delta P_i^whole(tau).
```

If the ledger closed, these would be equal. If they merely mis-size, a missing
same-signed reservoir could repair the account. If they are persistently
**opposite in sign** with both magnitudes resolved, no rescaling and no missing
same-signed term can repair it: the two selected books run against each other.
That is a `[CLOSED NEGATIVE]` — but for the **pair** `(P_i, p_matter)` as a
closed two-book momentum account for this state, not for `D_i` as "a momentum
carrier" in general. A sign inversion is equally consistent with a wrong matter
momentum definition or with a third reservoir (binding/constraint sector), and
the campaign cannot distinguish those. §7 and §9 license exactly this and
nothing more.

**Magnitude floor.** A sign is read only where the quantity is resolved. The
floor is scale-free and not fitted to a target: a checkpoint qualifies for the
sign test only if both `|Delta p_matter,i(tau)|` and `|Q_i(infinity,tau)|` are
at least one fifth of their own running maxima up to `tau`. On the FTD-0768
record this removes exactly the near-zero crossings the first draft would have
been decided by (`tau = 512` at `3.8e-6`, `tau = 576` at `2.27e-4`, `tau = 640`
at `5.27e-4`, against a running max of `3.37e-3`) and retains
`tau in {64,...,448, 704, 768}` at stride 64.

### 2.11 Numerical verification record

All checks below were run on random periodic fixtures with the operators of §0
transcribed directly from the engine source, in double precision. `79/79`
passed. Worst residual per group:

| group | content | worst residual | tolerance |
|---|---|---|---|
| A | `C^T` is the adjoint of `C`; `D_i` skew; `[D_i,C] = [D_i,C^T] = 0` | 5.7e-14 | 1e-11 |
| B | global identity (G), source-free and sourced, `L=9` | 3.6e-14 | 1e-11 |
| C | M1 regional identity, static mask, `L=9` | 1.6e-14 | 1e-12 |
| D | M2 regional identity + global density/source agreement | 2.9e-14 | 1e-12 |
| D' | control: L1 flux pair against L2 masked change | **O(1) failure** (7.5e-1 / 8.3e0 / 1.1e1) | — |
| E | chord census and reach; skewness `N_{-r} = -N_r^T` | 0.0 | 1e-13 |
| F | unit-bond `(T,S)` construction vs direct masked bilinear, `L=11`, four operators, site and per-component masks; complementarity | 3.6e-14 | 1e-11 |
| G | cumulative moving-mask ledger (L), 24 ticks, `L=15`; whole-domain limit | 1.7e-15 | 1e-12 |
| H | shell corollary (H); source-enclosure `Q(R2)-Q(R1) = 0` | 6.7e-16 (and exactly 0.0) | 1e-12 |
| J | `eta - tau = 1` and its shell form | 1.5e-13 | 1e-9 |

§12 requires these fixtures to be reproduced by
`scripts/proofs/proof_total_momentum_stress_ledger.py`, registered in the
repository, before the protocol hash is taken. No number in this document
depends on a script that will not be committed.

---

## 3. Units: the `interaction_scale` convention and its cross-check

The artifact's `moving_local_momentum` is `interaction_scale * <E,D_iCB>`
(applied at `cuda_matched_field_pipeline.cu:1204–1206`, verified there),
`interaction_scale = 0.021892057692994273` for this construction. The
constant itself is not a literal in that file — it is a runtime parameter
computed from `energy_scale * field_scale * current_scale`
(`face_flux_normalization.h`, fed by `gauge_couplings.h`); the file cited
above is where the weighting is *applied*, not where the number originates
(build-instrumentation finding, 2026-08-02). Matter momentum is in raw
`q`-units. The first draft never mentioned this, and none of its gates could
have caught the discrepancy: every identity in §2 is exactly scale-invariant,
so a raw-field instrument would pass G0 and G3 identically while inflating
every ratio by `1/0.0219 ≈ 45.7`.

**Convention (frozen).** `pi_i`, `Pi_i`, `F_i`, `W_i`, `Q_i`, `Delta P_i^whole`
and every derived ratio carry the artifact's `interaction_scale` weighting.
The instrument multiplies each reduction by `interaction_scale` at the same
point the existing `local_momentum` lambda does, so that all momentum-sector
quantities are commensurate with `Delta p_matter,i` and with the existing
`*_local_momentum` columns.

**Cross-check gate (G_U, part of §6 G0).** The convention is not merely
declared; it is measured, using the whole-domain mask as the reference:

```text
chi ≡ 1 (all sites), every checkpoint, every component, both localizations:
  | Pi_i(infinity,tau) - moving_local_momentum_i(tau) |
        <= 1e-12 * max(1, |moving_local_momentum_i(tau)|)
  | [Pi_i(infinity,tau) - Pi_i(infinity,0)] - Delta P_i^local(tau) |
        <= 1e-12 * max(1, |Delta P_i^local(tau)|)
  | F_i(infinity,tau) | <= 1e-11        (exactly zero in exact arithmetic)
  | W_i(infinity,tau) | = 0             (identically; mask differences vanish)
  | Q_i(infinity,tau) + Delta P_i^local(tau) | <= 1e-11
```

The second line is the requested reconciliation against the already-measured
whole-domain reference column, computed by the fresh run for itself. A unit
error of any size fails it immediately.

---

## 4. New instrumentation required (no existing artifact suffices)

**This is a new engine campaign.** Confirmed by inspection of the three
candidate sources, all frozen at commits `8d0f500a` / `d1b06370` on
`scale1-revision` (2026-08-01):

- `engine/results/ftd_0761/ftd_0761_m4_boosted_transport_v1_body.csv` (771
  rows/direction) — whole-domain 3-vectors and scalars only, no masks, no
  fields.
- `engine/results/ftd_0764/ftd_0764_transported_chart_morphology_v1_body.json`
  — whole-domain momentum 3-vectors plus 6 radial shells carrying **scalar
  energy** diagnostics only. No per-shell momentum.
- `engine/results/ftd_0768/ftd_0768_long_transport_dynamic_response_v1.json`
  — regional data exists (`laboratory`/`moving_near` blocks) but is
  **energy-only**; momentum appears only as whole-domain vectors and scalar
  defects. The CUDA kernel (`cuda_matched_field_pipeline.cu:289`
  `local_translation_momentum_kernel`) is an unmasked grid-stride reduction to
  a single 3-vector; §5 forbids full-field download, so no snapshot exists to
  post-process offline.

The energy-sector precedent is also a warning, not a template: the existing
`boundary_transport_into` is computed as `energy_pre_current - energy_before`
(`paired_field_response.cpp:360`), i.e. as a residual difference of regional
totals. That construction satisfies its own ledger by rearrangement and is
physics-empty. The momentum instrument specified here must not copy it
(Banned move B1).

**Required build (new files, none of which may touch production,
`RenderBridge`, or any existing test):**

1. `engine/include/ftd/eft/momentum_transport_current.h` (new) +
   `engine/src/eft/momentum_transport_current.cpp` (new) — implements
   `T^(i)_{a,d}(v)` and `S^(i)_{a,b}(v)` per §2.3, for **all four** operators:
   `N = D_i` on `u` and `N = D_i C C^T` on `E` (localization L1, §2.4), and
   `N = D_i` on `w = C^T E` and `N = D_i C^T C` on `B'` (localization L2,
   §2.5). Masked reductions `Phi_i[chi]` are computed **from `T^(i)`, `S^(i)`
   directly summed on straddling bonds** — never as `region_after -
   region_before`, and never as any other residual difference of regional
   totals (§6 G0, Banned move B1). Chord tables are sized from the 37-class /
   74-entry column of §2.6, not the 25-displacement column.
2. A **fused per-tick masked reduction kernel**, adapted from
   `local_translation_momentum_kernel` (`cuda_matched_field_pipeline.cu:1192–
   1263`), reusing the region-predicate machinery already present in
   `cuda_paired_field_response.cu:115–125`. One pass over the lattice per tick
   emits, for every component `i in {x,y,z}`, every radius `R`, and both
   localizations, the per-tick contributions to `Phi^(·)`, the sweep, the
   source term, and the regional content `Pi_i(R,t)`. Output per tick is a
   small vector of doubles (block-reduced), downloaded once per tick and
   accumulated host-side in `long double`. Reduction remains scalar-only — no
   full-field host download — per FTD-0768 §6's CUDA-telemetry discipline,
   inherited unchanged.
3. **Per-tick accumulation is mandatory, not optional.** (L) telescopes only
   if every tick contributes. `F_i`, `W_i`, `Q_i` are running host-side
   accumulators; checkpoints are readout points of those accumulators, not
   sampling points of the underlying rates. A checkpoint-only accumulation
   returns `MOMENTUM_LEDGER_INFRASTRUCTURE_UNRESOLVED` at G0.
4. Region construction reuses the `ChebyshevCube` **geometric type and its
   radius parametrization only** — **not** the existing `contains()`
   evaluation calls of `paired_field_response.cpp`, which test membership at
   staggered per-component positions (`component_position(0/1, axis, x, y,
   z)`, built for a scalar energy density living separately on E-faces and
   B-edges). Section 2.3's site-mask collapse and Section 5's `Omega_R(tau) =
   {x : ||x-c(tau)||_inf <= R}` both require a single, component-independent
   membership test evaluated once per integer lattice site `v` (`chi_a(v) =
   chi(v)`), which the staggered predicate does not provide. This
   instrumentation therefore implements a fresh integer-site membership test
   sharing only the Chebyshev-cube shape and radius sweep `{8, 16, 24, 32,
   48}` with `make_ftd0768_response_regions` (`paired_field_response.h:18`),
   not FTD-0768's single fixed `near_radius=8` predicate call sites. (Gap
   surfaced by the 2026-08-02 build pass, not by either review round — the
   letter of this item as first drafted would have produced a
   component-staggered mask inconsistent with the campaign's own derived
   math.)
5. `engine/tests/campaign_total_momentum_stress_ledger.cpp` (new) — the
   campaign harness; writes `engine/results/ftd_0769/
   ftd_0769_total_momentum_stress_ledger_v1*.{json,csv}` only after the
   qualification firewall (§8) passes.

**Per-checkpoint payload**, per component, per radius, per localization,
per arm (scalar-only): `Pi_i(R,tau)`, `Pi_i(R,0)`, `F_i(R,tau)`,
`W_i(R,tau)`, `Q_i(R,tau)`, the ledger residual of (L), `F_i` and `W_i` for
the complement mask and their sums with the region values, `eta_i`, `tau_i`,
`rho_i`, `kappa_i := Q_i(R,tau)/Q_i(R_out,tau)`, the maximum per-tick residual
since the previous checkpoint, plus the whole-domain `matter_momentum`,
`local_field_momentum` and `momentum_cumulative_defect` columns carried
through unchanged for §3's cross-check and §1's premise comparison. Also
recorded once per tick, cheaply: the integer mask centre `c(t)` and a flag for
whether it changed (the sweep-event log).

---

## 5. Frozen parent, construction, and evolution

Reconstruct the unchanged FTD-0761/0763 connected opposite-polarity parent
through tick 160, using the final FTD-0760/0761 protocol and common-action
options, exactly as FTD-0768 §2 did. Age 128 further ticks. Branch into
matched rest control `q=0` and moving discovery arm `q=+0.030 (0,0,1)`, as
FTD-0768. `L=321`, periodic, `dt=0.25`, `lambda = 0.14433756729740646`. All
common-action options, compact selected interaction, face/link field,
normalization, implicit solve, and state-only observer are frozen unchanged
from FTD-0768.

**Both arms are instrumented identically.** The rest arm is not merely a
displacement control here; §6 G7 makes it a precondition of every physics
verdict, per the FTD-0764 lesson (that campaign's detached-outgoing predicate
fired identically in the rest control, forcing the demotion of "motion-induced
radiation" to "preparation transient").

**Radius scan.** Evolve both arms for 768 ticks under five region
instrumentations, `R in {8, 16, 24, 32, 48}`, sharing one forward trajectory
(the region mask is a read-only diagnostic; it does not feed back into the
dynamics).

```text
Omega_R(tau) := { x : ||x - c(tau)||_inf <= R },
c(tau)       := componentwise nearest integer to the recorded continuous
                moving-core centre, read from THIS run's own record (not
                any prior artifact's `moving_center` — see G8), ties
                resolved toward +infinity.
```

`c(tau)` is the **rounded lattice site**, not the continuous centroid. The
first draft left this undefined and then required hop-aligned checkpoints so
that mask changes would coincide with recorded ticks. With per-tick
accumulation (§4 item 3) that requirement disappears: the mask is evaluated and
the sweep term accumulated at **every** tick, whether or not the centre moved,
so no sweep event can fall between records. The recorded core displacement over
768 ticks in the FTD-0768 run is `10.63` sites, so `c` changes roughly eleven
times per axis-crossing; the sweep-event log records exactly when.

The role of each radius is derived in §6 G4 and summarized here:

| `R` | chord-active volume fraction (reach 2) | role |
|---|---|---|
| 8 | 55.3% | clearance-marginal; recorded, excluded from the flatness test |
| 16 | 32.1% | physics radius |
| 24 | 22.5% | physics radius |
| 32 | 17.3% | physics radius |
| 48 | 11.9% | escape gauge / far-field reference `R_out` |

The first draft's percentages (`32/29/23/17/12%`) were single-layer (reach 1)
arithmetic printed under a reach-3 caption, and the reach was itself wrong
(§2.6). The corrected fractions are above. They are **reported context, not a
gate**: volume fraction is the wrong sharpness criterion, and §6 G4 replaces it
with the criterion that actually binds.

**Checkpoints.** `T := {0, 64, 128, ..., 768}`, retained unchanged from
FTD-0768 for direct comparability with the premise columns of §1. Because §2.7
accumulates every tick, the checkpoint set no longer carries any correctness
burden; it is a readout schedule. The `tau < 256` exclusion of the first draft
is **removed**: its stated justification ("both candidates are numerically zero
there") is false against the cited artifact — the defect is `-8.2e-4` at
`tau=64` and `-3.3e-3` at `tau=192`, the latter being 72% of the final value —
and with cumulative accumulation there is no transient to exclude. Early
checkpoints are instead filtered by the significance precondition of §6 G6,
which is a measured criterion rather than an asserted one.

Run the unchanged FTD-0768 state-only reverse check at the cadence FTD-0768
used; this diagnostic is inherited, not re-derived (§6 G2).

---

## 6. Frozen gates

### 6.1 G0 — Identity gate (implementation only; never a discriminator)

```text
per tick, per (i, R, localization, arm):
  |one-tick residual of (M1) or (M2)|
        <= 1e-11 * max(1, |LHS|, |lambda*Phi^(1)|, |lambda*Phi^(2)|, |source|)

cumulative at each checkpoint tau (ticks elapsed N = tau):
  |Pi_i(R,tau) - Pi_i(R,0) - F_i - W_i + Q_i|
        <= 1e-11 * N * max(1, |Pi_i(R,tau)|, |F_i|, |W_i|, |Q_i|)

complementarity (every checkpoint, every R):
  |F_i[chi] + F_i[1-chi]| <= 1e-11 * max(1, |F_i[chi]|)

units cross-check: the G_U block of §3, in full.

independence (structural, checked once per build, not per tick):
  every Phi is computed ONLY by summing T^(i), S^(i) on bonds straddling
  dOmega; it is never computed as region_after - region_before, and never
  as any other residual difference of regional totals.
```

The cumulative bound is written as the per-tick bound accumulated linearly
(`1e-11 * N`, i.e. `7.7e-9` at `N = 768` and scale 1) rather than as an
independent absolute constant. The first draft paired a `1e-11` per-checkpoint
gate with a `1e-9` cumulative gate; with the checkpoint count the campaign
actually generates, a run passing every per-checkpoint gate could fail the
cumulative one arithmetically. That failure direction was safe (spurious
`INFRASTRUCTURE_UNRESOLVED`, not a false pass) but it was an internally
inconsistent pair inside a frozen gate. Writing the cumulative gate as the
linear accumulation of the per-tick gate removes the inconsistency by
construction.

Tolerance justification: the §2.11 fixture verification closes the same
identities at `1e-15` to `1e-13`; `1e-11` per tick adds roughly two to four
orders of margin for WSL2/CUDA floating-point accumulation. Momentum-sector
quantities here are of order `1e-3` to `1e-2` after the `interaction_scale`
weighting of §3, well below the `max(1, ...)` floor, so the gate is effectively
absolute at `1e-11` per tick — stated plainly rather than dressed as a
scale-relative bound, which the first draft's narrative did.

### 6.2 G1 — Reynolds/mask-sweep transfer gate

```text
per tick, per (i, R, localization, arm), identity-level:
  sum_x[ chi_{t+1} pi^{t+1} - chi_t pi^t ]
     = sum_x[ chi_{t+1} (pi^{t+1} - pi^t) ] + sum_x[ (chi_{t+1}-chi_t) pi^t ]
  residual <= 1e-12 * max(1, |terms|)
```

Material term uses the *new* mask with both snapshots; sweep term uses the
*old* field with the mask difference — the FTD-0768 pairing convention
(`paired_field_response.cpp:391–421`), transferred without modification. The
scalar site mask serves all three `i` simultaneously (§2.3 site-mask collapse).

### 6.3 G2 — Inherited execution-validity gates (frozen unchanged from
FTD-0768 §6–§7; not loosened, not re-derived, not dropped)

```text
valid state-only matter membership and derived interaction graph
common-action, continuity, Gauss, work, complete-energy residuals <= 1e-12
causal-speed excess                                              <= 1e-12
minimum root singular value >= 1e-3, condition number <= 1e4 at checkpoints
one-step state-only inverse residual                              <= 1e-12
no periodic-boundary contact (boundary_margin > 0)
rest-core displacement                                            <= 1e-12
forward/reverse full-recovery                                     <= 1e-10
```

FTD-0768's own run failed the last of these (`reverse_recovery =
3.8786822642578045e-9 > 1e-10`, `reverse_valid: false`). This gate is inherited
**unchanged**, not loosened to manufacture a pass for Arc 2 (Banned move B4).
If the same near-miss recurs, Arc 2 returns `MOMENTUM_LEDGER_BASELINE_INVALID`.

The boundary-margin gate carries a caveat that the campaign must record rather
than assume. FTD-0768 computes `boundary_margin = 0.5*L - 4 - causal_reach`
with `causal_reach = (160 + 128 + 768) * lambda = 152.42`, giving `4.08` at
`L=321`, i.e. amplitude-level recurrence is excluded. The **strict stencil**
cone is 1 site per tick in `l_inf` (`C C^T` has `l_inf` reach 1, §2.6), which
does wrap within the horizon. Exponentially small stencil-tail contamination of
the exterior is therefore not excluded by this gate, only amplitude-level
recurrence. The `R_out = 48` escape gauge of §5 measures it directly:
`eta_i(48,tau) ≈ 1` with `tau_i(48,tau) ≈ 0` is empirical confirmation that
nothing of consequence has left the instrumented neighbourhood; a departure is
recorded as `FAR_FIELD_ACTIVE` and reported alongside the verdict.

### 6.4 G3 — Localization pre-check (the two `[SELECTION]`s)

Two independent sources of non-uniqueness are declared explicitly:

- **Localization L1 ("E-carries", canonical)**: `pi_i^(1)(a,v) := E_a(v)
  (D_i C B)_a(v)`, masked at `E`'s site; regional identity (M1), fluxes
  `Phi^(u)` on `N = D_i` and `Phi^(E)` on `N = D_i C C^T`; source term
  `<chi K, D_i C B'>`, supported exactly on `supp(K) ∩ Omega`.
- **Localization L2 ("B-carries", alternate)**: `pi_i^(2)(a,v) := -B_a(v)
  (C^T D_i E)_a(v)`, masked at `B`'s site; regional identity (M2), fluxes
  `Phi^(w)` on `N = D_i` (field `w = C^T E`, before) and `Phi^(B')` on
  `N = D_i C^T C` (field `B'`, after); source term `-<chi B', D_i C^T K>`,
  supported on `supp(K)` dilated by `l_inf` 2.
- **Path convention for `T^(i)`**: fixed lexicographic unit-step order
  (`x`, then `y`, then `z`, from `v` toward `v+r`), frozen and hash-locked
  (Banned move B5).

All of the above must pass an exactness pre-check on a fixed deterministic
`L=11` challenge lattice **before any registered artifact may be written**:

```text
global check:  |sum_v pi_i^(1)(v) - sum_v pi_i^(2)(v)| <= 1e-12
flux check L1: |Phi^(u), Phi^(E) via (T,S) - direct masked bilinear| <= 1e-12
flux check L2: |Phi^(w), Phi^(B') via (T,S) - direct masked bilinear| <= 1e-12
identity check: residual of (M1) <= 1e-12 and residual of (M2) <= 1e-12
source agreement: |<B',D_i C^T K> + <K, D_i C B'>| <= 1e-12
complementarity: |Phi[chi] + Phi[1-chi]| <= 1e-12
```

The flux checks must be run under **two** masks: a site mask (the production
form) **and** at least one genuinely per-component mask `chi_a(v)` that differs
between components. Under a site mask the `S^(i)` array contributes exactly
zero (§2.3), so without the per-component case the `S` instrumentation would
pass through the entire gate suite unexercised inside a hash-locked protocol.
The per-component case must produce a non-zero `S` contribution, and the flux
check must still close.

Failure of any check returns `MOMENTUM_LEDGER_INFRASTRUCTURE_UNRESOLVED`
before any production-scale run is attempted.

**The physics verdict (§7) is computed independently under `L1` and `L2`.** If
the two land in different buckets, the campaign reports
`MOMENTUM_LEDGER_LOCALIZATION_AMBIGUOUS` rather than picking one:
localization-dependence is itself a reportable result, not resolvable by
choosing a preferred convention post hoc.

### 6.5 G4 — Radius adequacy (measured, not asserted)

The first draft gated radius adequacy on the chord-active volume fraction and
disqualified `R=8` because "the boundary layer is a third of the region". That
criterion does not survive the corrected arithmetic: with the true reach of 2,
`R=16` is 32.1% active and `R=24` is 22.5%, so the stated rule would disqualify
the registered radii too. The criterion is replaced by the three conditions
that the identities of §2 actually require.

```text
(a) SOURCE ENCLOSURE (exact, measured every tick):
      kappa_i(R,tau) := Q_i(R,tau) / Q_i(R_out,tau) == 1 to <= 1e-13
    for every physics radius, both localizations.  L1 requires
    supp(K) subset Omega_R; L2 requires supp(K) dilated by l_inf 2 subset
    Omega_R.  With the frozen `support_half_width = 4` this is satisfied for
    R >= 7; it is verified, not assumed.

(b) SHELL CLEARANCE: the flux shell occupies layers R-1 .. R (reach 2).  The
    (dilated) source support must not touch it:
      R - 1 > r_K + 2,   r_K := recorded max l_inf radius of supp(K) about c(tau).
    At r_K = 4 this gives R >= 8 with one layer of clearance (marginal) and
    R >= 16 with nine (ample).  R=8 is therefore recorded but declared
    CLEARANCE_MARGINAL and excluded from the flatness test of §6.6.

(c) SHELL DISJOINTNESS: consecutive physics radii must differ by more than
    twice the reach, R_{k+1} - R_k > 4, so that the shell corollary (H) refers
    to a genuine bulk region.  {16, 24, 32} (gaps of 8) and the R_out = 48
    gauge all satisfy this.
```

A valid physics verdict requires `G0`-passing data with `kappa = 1` at **all
three** of `R = 16, 24, 32`. Otherwise the campaign returns
`MOMENTUM_LEDGER_INSTRUMENT_LIMITED`.

### 6.6 G5 — Physics-content thresholds

The landmarks `eta = 1` (retained) and `eta = 0` (through-flowing) are derived
in §2.9 and are not authored here. What is authored here is the width of the
bands around them and the flatness tolerance. Both are set from the geometry of
the landmarks, not from the observed magnitudes:

```text
band half-width  h := 0.25
    the two landmarks are separated by exactly 1, so any h < 0.5 keeps the
    buckets disjoint; h = 0.25 leaves a 0.5-wide ambiguous middle that falls
    through to MIXED rather than being forced into either bucket.

flatness tolerance  g := 0.10
    a classification must not be able to straddle two buckets, so the allowed
    R-variation must be smaller than the band half-width; g = 0.10 gives 2.5x
    headroom against h = 0.25.

accumulation threshold  G := 0.25
    equal to h: content amounting to a full bucket-width has moved into the
    shells between the physics radii.
```

Evaluated at every checkpoint that passes the significance precondition G6, for
`R in {16, 24, 32}`:

```text
CORE_RETAINED test:
    |eta_i(R,tau) - 1| <= h   at all three radii
    AND max_R eta_i - min_R eta_i <= g

THROUGH_FLOWING test:
    |eta_i(R,tau)|     <= h   at all three radii
    AND max_R eta_i - min_R eta_i <= g

NEAR_ZONE_ACCUMULATING test:
    eta_i(32,tau) - eta_i(16,tau) >= G   (monotone gain across the shells)

OVER_DEPLETING test:
    eta_i(R,tau) >= 1 + h   at all three radii, or eta_i(R,tau) <= -h at all
    three (the region's content moves further than, or against, the whole
    domain's)
```

Reported alongside, gating nothing (§2.9(iv)): `rho_i(R,tau)` together with its
derived ceiling `|Delta P_i^whole(tau)| / |D_i(tau)|` at the same `tau`, so
that any reader can see what fraction of the ceiling was attained.

### 6.7 G6 — Per-component significance precondition

`Delta p_matter,x/y` and `Delta P_local,x/y` sit at roundoff in the reference
run (`3.8e-11` and `2.3e-12` for the moving arm's local momentum at `tau=768`,
against `2.0e-3` for `z`). Normalizing by a roundoff-scale denominator produces
a label with no content. A component `i` qualifies for a physics verdict at
`tau` only if

```text
|Delta P_i^whole(tau)| >= 1e-9                                (absolute floor)
AND |Delta P_i^whole(tau)| >= 1e3 * |Delta P_i^whole,rest(tau)|  (arm separation)
AND |D_i(tau)|          >= 1e-9
```

Components failing this at every checkpoint are reported as
`MOMENTUM_LEDGER_NULL_NO_DEFECT` for that axis. On the reference run this
nulls `x` and `y` and retains `z`, as it should.

### 6.8 G7 — Rest-arm control (precondition of every physics verdict)

FTD-0764 demonstrated the failure this gate exists to prevent: its
detached-outgoing predicate fired identically in the rest control
(rest/plus flux mismatch `2.65e-5`–`3.43e-5` relative), and the motion reading
had to be demoted to a preparation transient. §2.9(iii) supplies the further,
derived reason: `eta_i` is interpretable only when the regional content change
is source-dominated; a background-dominated regional change produces `|eta|`
far from both landmarks (on the random fixture of §2.9(iii), `|eta|+|tau|`
reached 166.6, so `|eta|` alone reached well over 100) and a verdict that
describes noise.

```text
at every qualifying checkpoint, every physics radius, both localizations:
  |F_i^rest + W_i^rest|       <= 0.1 * |F_i^moving + W_i^moving|
  |Pi_i^rest(R,tau) - Pi_i^rest(R,0)| <= 0.1 * |Delta P_i^whole,moving(tau)|
```

Failure returns `MOMENTUM_LEDGER_REST_ARM_CONTAMINATED`. The factor `0.1` is
the minimum separation at which the moving-arm reading is not a
formation transient; FTD-0764's failure had a ratio of essentially 1, and the
FTD-0768 reference run's whole-domain rest/moving ratio is `1e-13`, so the gate
is loose in practice and binding in principle.

### 6.9 G8 — Prior-comparability flag (reported, non-blocking)

The band constants of G5 are derived from landmarks and do not depend on the
FTD-0768 magnitudes; the `rho_i` diagnostic and the §1 premise comparison do.
The fresh run therefore records

```text
|Delta P_z^whole(768)| within [1/3, 3] x 2.0044e-3   ?
|D_z(768)|             within [1/3, 3] x 4.5931e-3   ?
```

If either falls outside, the run is flagged `PRIOR_MISMATCH`, the `rho_i`
diagnostic is reported as out-of-family, and the §1 premise table is marked as
not describing this run. The physics buckets are unaffected, because they do
not use those numbers. This closes the first draft's false header disclaimer
("its own numeric artifact is not reused as data") by acknowledging the reuse
and gating it, rather than by denying it.

---

## 7. Verdict map

Apply the first matching verdict, **per component `i`**, then report the triple
`(x,y,z)` together with any per-axis disagreement noted explicitly (no silent
majority-vote collapse across axes).

1. Any hash, parent, build, CUDA, record, **G0, G1**, or G3 pre-check failure:
   **`MOMENTUM_LEDGER_INFRASTRUCTURE_UNRESOLVED`**.
2. Any G2 inherited execution-validity gate fails (including the
   forward/reverse recovery gate, unchanged from FTD-0768):
   **`MOMENTUM_LEDGER_BASELINE_INVALID`**.
3. G4 fails (no `kappa = 1`, G0-passing data at all of `R = 16, 24, 32`):
   **`MOMENTUM_LEDGER_INSTRUMENT_LIMITED`**.
4. G6 fails at every checkpoint for this component:
   **`MOMENTUM_LEDGER_NULL_NO_DEFECT`** (expected for `x` and `y`).
5. G7 fails: **`MOMENTUM_LEDGER_REST_ARM_CONTAMINATED`**.
6. `L1` and `L2` land in different buckets of items 7–11:
   **`MOMENTUM_LEDGER_LOCALIZATION_AMBIGUOUS`**.
7. `L1` and `L2` agree, CORE_RETAINED test holds at every qualifying
   checkpoint: **`MOMENTUM_LEDGER_CORE_RETAINED`** — the field-side
   pseudomomentum change generated by the moving core stays inside the
   co-moving control volume; no net boundary transfer accumulates (equal-
   and-opposite circulation is not excluded by this test alone).
8. `L1` and `L2` agree, THROUGH_FLOWING test holds at every qualifying
   checkpoint: **`MOMENTUM_LEDGER_THROUGH_FLOWING`** — the change is supplied
   across the boundary at an `R`-independent rate over the tested radii, the
   discrete Gauss's-law signature of a transported current. This is a
   **located** carrier, not an artefact, and must not be reported as one.
9. `L1` and `L2` agree, NEAR_ZONE_ACCUMULATING test holds at every qualifying
   checkpoint: **`MOMENTUM_LEDGER_NEAR_ZONE_ACCUMULATING`** — the transfer is
   absorbed into the shells between the physics radii; a growing near-zone
   dressing, neither bound at the core nor cleanly transported.
10. `L1` and `L2` agree, OVER_DEPLETING test holds at every qualifying
    checkpoint: **`MOMENTUM_LEDGER_OVER_DEPLETING`** — the region's content
    moves further than, or against, the whole domain's.
11. `L1` and `L2` agree, none of 7–10 apply: **`MOMENTUM_LEDGER_MIXED`**.

**Orthogonal registered flag (not a bucket).** Independently of items 7–11, and
only if items 1–5 all pass, the campaign records

```text
EXCHANGE_SIGN_INVERTED := sign(Delta p_matter,i(tau)) != sign(Q_i(R_out,tau))
                          at every checkpoint passing the 1/5-running-max
                          magnitude floor of Sec 2.10
```

`Q_i(R_out,tau)` is read under **`L1` only** (frozen; Section 2.10 and this
item were silent on which localization supplies it, a gap surfaced by the
build). `L1` is used because it is this document's canonical localization
(Section 2.4) and because the two source terms already agree globally to
`<= 1.8e-15` (Section 2.5); `L2`'s value is available as a cross-check but is
not part of the frozen flag.

If set, this licenses a `[CLOSED NEGATIVE]` for **the pair `(P_i, p_matter)` as
a closed two-book momentum account for this state** — see §9 for the exact
scope, which is narrower than the first draft's.

Also recorded and reported with every verdict, gating nothing:
`FAR_FIELD_ACTIVE` (§6.3), `CLEARANCE_MARGINAL` at `R=8` (§6.5), and
`PRIOR_MISMATCH` (§6.9).

No branch of this map licenses a physical-momentum claim, a "the substrate is
the reservoir" claim, a Lorentz claim, or a production change.

---

## 8. Firewall

One non-evidential `L=17`, two-tick qualification run may verify the runner
schema, CUDA calls, masked-kernel interface, per-tick accumulator wiring, and
absence of result writes. It may not change `q`, the horizon, the radius set,
the checkpoint set, any gate in §6, the two localizations `L1`/`L2`, the
`T^(i)` path convention, or the verdict map. The `L=11` exactness pre-check of
§6.4 is separate from this firewall and must also pass, on its own frozen
fixture, before the qualification run. **The firewall's scope is wiring, not
physics validity — a distinction the first build pass got wrong and this
paragraph now makes explicit.** A deliberately minimal, zero-formation-tick
`L=17` probe state is not expected to satisfy the state-only matter
membership check or the full common-action gate suite of §6.3 G2 (those
gates exist to validate the registered 768-tick campaign, not a two-tick
wiring smoke-test); a first implementation shared one stepper class between
the firewall and the registered campaign with no way to say so, so a
membership miss on tick 1 unconditionally poisoned the stepper and caused
tick 2 to short-circuit before any wiring was exercised at all — including
the labelled masked-kernel interface probe, which as a result silently never
ran. Fixed by giving the firewall's stepper (and only the firewall's) a mode
flag that stops a membership/common-action miss from poisoning subsequent
ticks, while leaving `result.ledger_valid` (the actual wiring/CUDA-kernel
check this section cares about) and the registered campaign's own stepper
fully unconditional. Verified post-fix: both firewall ticks complete, the
labelled probe exercises real straddling bonds, and host/device parity holds
to `~1e-21` against a `1e-10` gate.

After the instrumentation (§4), the exactness pre-check (§6.4), and this
qualification firewall all pass, the implementation is frozen and hashed
(`protocol_sha256`, computed per `REF_PREREGISTER_MANIFEST.md`'s byte-prefix
convention over this file, with an independent
`scripts/proofs/proof_total_momentum_stress_ledger.py` certificate matching a
compile-time constant in the new engine artifact). Every registered `(R,
localization, arm)` combination then runs once. Interrupted or failed modes
are not tuned or rerun under this pre-registration; a fresh `v2` would be
required.

---

## 9. Interpretation boundary

A green G0/G1/G2/G3 suite establishes only that the implementation correctly
measures a `[THEOREM]`-level identity; **it is not itself a physics result and
licenses no new tag** (Banned move B2). Only items 7–11 of §7 are physics
verdicts, and only after G0–G7 all pass.

`MOMENTUM_LEDGER_CORE_RETAINED` and `MOMENTUM_LEDGER_THROUGH_FLOWING` each
license a future pre-registered attempt to price `T^(i)`/`S^(i)` as a candidate
discrete stress-energy-momentum object in the import-ledger sense
(`SPEC_IMPORT_LEDGER.md`). Neither itself adds or removes a priced-import line,
promotes `P_i` above `[SELECTION]`, or touches `x_+ = 1/alpha`, MC-T4.3, or any
production default. They are different verdicts describing different regimes
(bound versus transported) and must be reported as such.

`MOMENTUM_LEDGER_NEAR_ZONE_ACCUMULATING` and `MOMENTUM_LEDGER_OVER_DEPLETING`
report where the change went within the tested radii and license nothing
beyond that. In particular, no verdict may be stated as "does not concentrate
anywhere": the instrument is five nested co-moving cubes with `R <= 48`, so
every statement is bounded by "within the tested radii, at this boost and
horizon". Concentration in a trailing wake, beyond `R = 48`, or in a structure
the Chebyshev cube does not resolve is outside what this campaign can see.

`EXCHANGE_SIGN_INVERTED`, if set with all gates green, licenses a
`[CLOSED NEGATIVE]` for the **two-book account** `(P_i, p_matter)` for this
state: no rescaling and no additional same-signed reservoir can reconcile two
books that move in opposite senses. It does **not** close `D_i` as a momentum
carrier in general — a sign inversion is equally consistent with a
mis-specified matter momentum or with a third (binding/constraint) reservoir,
and this campaign cannot distinguish those. It does not close the family
`{<E,ACB> : A^T=-A, [A,CC^T]=0}`, and it does not license a claim that no
discrete lattice momentum can ever be defined.

---

## 10. Banned moves

- **B1.** No computing any `Phi` as a residual difference of regional totals
  (`region_after - region_before` or any equivalent), on pain of reproducing
  the tautology trap the existing energy instrument contains
  (`paired_field_response.cpp:360`): a residual-defined flux satisfies its
  ledger by rearrangement and is physics-empty. Every `Phi` must be summed
  directly from `T^(i)`, `S^(i)` on straddling bonds (§4, §6.1 G0).
- **B2.** No treating a fresh measurement's specific numeric defect as itself
  justifying a "the substrate is the reservoir," "momentum is conserved after
  all," or any other physical-carrier claim **unless** the flux-attribution
  machinery of §2–§6 actually places it under items 7–8 of §7 with all gates
  green. A large or small number alone proves nothing.
- **B3.** No silently reusing FTD-0768's energy-Reynolds gate NUMBERS
  (`1e-10` regional, `1e-12` per-transaction) for momentum. §6 re-derives
  momentum-appropriate tolerances from the §2.11 fixture closures; any future
  revision must do the same.
- **B4.** No adjusting any threshold in §6 (identity tolerances, band
  half-width `h = 0.25`, flatness `g = 0.10`, accumulation `G = 0.25`, the
  `1/5`-running-max sign floor, the significance floors, the rest-arm factor
  `0.1`, or the inherited G2 suite) after seeing results. A `v2`
  pre-registration is required for any threshold change, with its own
  justification independent of the run it would have altered.
- **B5.** No changing the `T^(i)` unit-step path convention, the `R+`
  representative choice, or which localization is `L1` vs `L2` after the
  exactness pre-check (§6.4) is frozen. A different convention is a different
  candidate and requires its own pre-registration.
- **B6.** No dropping the radius scan, and no reporting a result from fewer
  than the three physics radii `{16, 24, 32}` as anything other than
  `MOMENTUM_LEDGER_INSTRUMENT_LIMITED`. `R=8` is `CLEARANCE_MARGINAL` by §6.5
  and may not be used in the flatness test.
- **B7.** No collapsing the buckets of §7 items 7–11 into a single "defect
  measured" headline, and specifically no reporting
  `MOMENTUM_LEDGER_THROUGH_FLOWING` as a null or artefact result: §2.9(iii)
  derives that a through-flowing signature is a located carrier, and the
  identity forbids the "nothing anywhere" configuration the first draft named.
- **B8.** No reporting `EXCHANGE_SIGN_INVERTED` as a closure of `D_i` as a
  momentum carrier, of the operator family, or of lattice momentum as a class.
  Its licensed scope is §9, verbatim.
- **B9.** No production, `RenderBridge`, scenario, common-action, binding-law,
  predicate, constant, or ontology-primitive change in response to any verdict
  in §7.
- **B10.** No accumulating `F`, `W` or `Q` at checkpoints rather than every
  tick, and no substituting a per-step flux for the accumulated one anywhere in
  §6 or §7. (L) telescopes only over the full tick sequence; the first draft's
  rate-versus-total ratio is the defect this ban exists to prevent recurring.

---

## 11. Disclosures

These are judgment calls and known hazards that a reviewer should attack first.

1. **The exchange-sign pattern is already visible in the parent artifact.** On
   the FTD-0768 record, `Delta p_matter,z < 0` at every checkpoint passing the
   §2.10 floor, while `Q_z = -Delta P_local,z > 0` throughout. If
   `EXCHANGE_SIGN_INVERTED` fires on the fresh run it is a **replication** of an
   already-observed pattern on an execution-invalid run, not a discovery. The
   pre-registration's value here is that it fixes the floor, the checkpoint
   set, and the licensed interpretation before the valid run, not that the
   outcome is unknown.
2. **The band widths `h = 0.25`, `g = 0.10`, `G = 0.25` are the primary
   remaining judgment call.** Their *locations* are derived (§2.9(i)); their
   widths are argued from bucket disjointness and nothing else. A reviewer who
   prefers `h = 0.15` or `h = 0.35` should say so before the lock.
3. **`eta` is only interpretable when the regional content change is
   source-dominated.** §6.8 G7 enforces this through the rest arm, with a
   factor `0.1`. On a background-dominated configuration the derived landmarks
   still hold algebraically but describe the background rather than the core.
4. **`R_out = 48` is inside the amplitude front.** Over the 768-tick discovery
   window the front travels `768 * lambda = 110.9` sites, so material emitted
   during the window can pass `R = 48`; `eta(48) < 1` therefore reads as
   genuine escape from the instrumented neighbourhood. It also means `R=48` is
   not a quiescent far-field reference, and `FAR_FIELD_ACTIVE` is expected to
   carry real information rather than being a formality.
5. **The strict stencil cone wraps.** §6.3 records this; the campaign measures
   the consequence rather than excluding it by assumption.
6. **Localization-dependence is a live possibility, not a formality.** L1 and
   L2 differ not only in where the density is masked but in where the *source
   term* is attributed (exactly `supp(K)` for L1, `supp(K)` dilated by 2 for
   L2). A `LOCALIZATION_AMBIGUOUS` outcome is a real and reportable result.
7. **This document has had a provisional, AI-simulated adversarial review, not
   external human review.** The rework above repairs every defect those reviews
   raised; it does not substitute for the external validation this project has
   explicitly not yet received.

---

## 12. Lock record

This file was a draft through 2026-08-02. Locking required, in order, and was
completed as follows:

1. **Built** §4's instrumentation, including the per-tick accumulators and the
   full L2 operator pair (`D_i` on `w`, `D_i C^T C` on `B'`) with its own
   source term — the first draft froze only the L1 pair, which could not
   compute the L2 branch (§2.5). New files:
   `engine/include/ftd/eft/momentum_transport_current.h`,
   `engine/src/eft/momentum_transport_current.cpp`,
   `engine/include/ftd/eft/cuda_momentum_transport_current.h`,
   `engine/cuda/cuda_momentum_transport_current.cu`,
   `engine/tests/campaign_total_momentum_stress_ledger.cpp`. One build-time
   defect was found and fixed under this project's systematic-debugging
   discipline (root-caused, single minimal change, verified before and after)
   — see the §8 firewall note above; the §4 item 4 region-mask correction and
   the R+ representative / `Q_i(R_out,tau)` localization / `C_SPEED` /
   `interaction_scale`-provenance disclosures elsewhere in this document are
   the other build-surfaced gaps, all closed before lock.
2. **Registered** `scripts/proofs/proof_total_momentum_stress_ledger.py`,
   reproducing every fixture closure of §2.11 independently in numpy —
   **320/320 checks pass** (worst residuals at or below the quoted §2.11
   values; Group D' negative control confirmed to fail as designed). No
   load-bearing number in this document rests on an uncommitted script.
3. **Passed** the `L=11` exactness pre-check (§6.4): `pass=true`, chord census
   exact against §2.6 (25 displacements / 74 entries / 37 `R+` classes,
   reach 2), site-mask `S` channel exactly `0`, per-component-mask `S`
   channel non-zero (confirming it is exercised).
4. **Passed** the §8 firewall (post-fix): `pass=true ticks=2`,
   `probe_bonds=true`, host/device parity `~1e-21` against the `1e-10` gate.
5. **Ran** `scripts/audit/check_registry.py` on `scale1-revision` (the only
   branch unmerged into `main`; the script scans the checked-out working
   tree, not `main` by name) — `762` ids referenced, max `FTD-0768`, next
   free **`FTD-0769`**. Every `FTD-XXXX`/`ftd_XXXX` placeholder in this file
   and in the engine artifact's compile-time constants was replaced with
   `FTD-0769` / `ftd_0769`.
6. **Computed** `protocol_sha256` over the byte-prefix of this file preceding
   the `protocol_sha256=` field (header, line 4), per
   `REF_PREREGISTER_MANIFEST.md`'s convention, and wrote the matching value
   into `kMomentumProtocolSha256` in
   `engine/tests/campaign_total_momentum_stress_ledger.cpp`, independently
   re-verified by `scripts/proofs/proof_total_momentum_stress_ledger.py`.
7. Row added to `REF_PREREGISTER_MANIFEST.md` and a LEDGER stub row added
   **before** execution, per standing practice. `engine/results/ftd_0769/`
   does not yet exist; the campaign has not been run.
