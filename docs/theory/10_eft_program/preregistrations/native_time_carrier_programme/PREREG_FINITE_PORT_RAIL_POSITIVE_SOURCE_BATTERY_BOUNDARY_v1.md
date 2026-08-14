# Preregistration — Finite port rail and positive source-battery boundary v1

**Identifier:** `FTD-0883`  
**Date frozen:** 2026-08-11  
**Status:** `[LOCKED/PRE-RUN]`  
**Programme:** native time carrier / contextual actualization  
**Method guard:** exact algebra and a fixed rational `L=4` witness only; no
numerical search, fit, near-miss scan, or formula substitution is permitted.

## 1. Question

FTD-0882 conditionally prepares the matched minimum-energy Gauss record by
feeding every checkerboard gate a fresh zero environment port and exporting
the signed outgoing residual. It books the source work exactly but leaves the
source/controller as an external account.

This lock asks the next two minimal questions.

1. How many exact fresh layers can an explicit finite cyclic bank of signed
   ports supply while the complete field-plus-bank evolution remains
   reversible?
2. Can the local source work be carried by a positive one-amplitude quadratic
   battery with a unique sign-preserving reversible update?

The certificate is not allowed to infer a universal finite-dimensional memory
no-go. Exact-real encodings outside the explicit port-bank class remain out of
scope, as do Hamiltonian/symplectic completion and production embedding.

## 2. Frozen sources

| source | SHA-256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_AND_SELF_DUAL_ENERGY_SPLIT_v1.md` | `143D897A69B5C6FED8C00402C1840EA9FAEE5BD4BC259C9BDD065DFDC616A814` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |
| `engine/include/ftd/eft/reversible_checkerboard_gauss_preparation.h` | `7C2AFBFD098268B02C9E58DABAC19ED38DD1FA173385424E111B0FEFAAD79420` |
| `engine/src/eft/reversible_checkerboard_gauss_preparation.cpp` | `CFDD471E81DBB6040C882A069468D7E22930CF8AEB48084EEBD2D56824E66511` |
| `engine/include/ftd/eft/oriented_ternary_quarter_turn.h` | `46CD15943F5EB8EDBBCE4676CDE558A7C2B08556E1AC64E7C9720D30FFEB68E1` |

## 3. Frozen finite-port model

Fix the even periodic matched `L=4` probe and the neutral dipole used by
FTD-0882. For an integer capacity `C>=1`, attach one explicit signed-port
vector `h_j` to each cursor position `j in Z_C`. Initially every `h_j=0`.

At half-layer `n`, the gate reads `e=h_k`, where `k=n mod C`, applies the
FTD-0882 checkerboard gate at parity `n mod 2`, writes the complete outgoing
signed vector back to `h_k`, and advances `k` by one. The bank coordinate and
cursor are retained. Reversal decrements the cursor, reads the stored outgoing
vector, applies the exact inverse layer, and restores the recovered incoming
vector to that bank coordinate.

The registered preparation accepts a forward layer only while the selected
incoming bank coordinate is exactly fresh. Therefore an all-zero bank supplies
at least the first `C` fresh layers. If the first stored outgoing vector is
nonzero, the returning cursor is not fresh on layer `C+1`. No clearing or
compression operation may be inserted after the lock.

The finite-bank conclusion is restricted to this explicit one-vector-per-port
cyclic representation. A growing/bilateral rail or an open signed-tail export
may supply every finite horizon, but carries correspondingly unbounded or open
history capacity.

## 4. Frozen positive source battery

For each active cell let `b_x != 0` be a battery amplitude with positive
energy

\[
 E_{b,x}=\frac12 b_x^2.
\]

The FTD-0882 local gate changes field-plus-current-port energy by

\[
 w_x=\frac{q_x}{6}(e_x-r_x).
\]

In the registered one-amplitude, quadratic-energy, sign-preserving class, the
battery update is frozen as

\[
 b'_x=\operatorname{sgn}(b_x)\sqrt{b_x^2-2w_x},
 \qquad b_x^2-2w_x>0.                                      \tag{B1}
\]

The strict reserve gate must be checked before mutating the field. Reverse the
Gauss gate first, recompute `w_x`, and recover

\[
 b_x=\operatorname{sgn}(b'_x)\sqrt{(b'_x)^2+2w_x}.           \tag{B2}
\]

The certificate may prove that (B1) is unique in the registered class and
that it closes positive energy exactly. It may not call (B1) Hamiltonian,
symplectic, naturally scaled, or production native. The amplitude/register and
its update reuse existing continuous carrier and phase-rail types, but (B1)
is an **[IMPOSED reference law]**, not a derived substrate law.

## 5. Frozen certificate gates

The certificate contains exactly **56** checks.

### C1--C8 — provenance and scope

1. five frozen source hashes match;
2. this protocol hash matches its recorded pre-run value;
3. the explicit finite-port class is stated;
4. the exact-real counterclass is excluded from the no-go;
5. the battery law is tagged imposed;
6. Hamiltonian/symplectic completion stays open;
7. production remains untouched; and
8. Born, Bell, Lorentz, biology, and completeness are absent from the result.

### C9--C24 — finite cyclic bank

9. the locked capacity is positive;
10. every initial bank coordinate is zero;
11. cursor scheduling is deterministic and context blind;
12. each of the first `C` selected inputs is fresh;
13. every accepted fresh layer is the FTD-0882 affine projection;
14. the complete signed outgoing vector is stored;
15. the cursor advances modulo `C`;
16. forward field-plus-bank evolution is injective;
17. one inverse step restores field, bank, and cursor;
18. all `C` accepted layers reverse exactly;
19. the locked dipole writes a nonzero first outgoing vector;
20. the cursor returns to its first coordinate after `C` layers;
21. layer `C+1` therefore fails the fresh-port gate;
22. a finite cyclic bank is not an indefinite fresh environment in this class;
23. capacity growth or signed-tail export supplies the declared escape; and
24. no universal finite-dimensional memory theorem is claimed.

### C25--C44 — positive quadratic battery

25. every locked battery amplitude is nonzero;
26. its energy is positive;
27. local work uses only `q_x`, `e_x`, and `r_x`;
28. the reserve radicand is tested before mutation;
29. the forward amplitude is real under the reserve gate;
30. battery sign is preserved;
31. battery energy changes by exactly `-w_x`;
32. field-plus-port-plus-battery energy is exact per gate;
33. sign preservation plus quadratic energy makes (B1) unique;
34. (B2) recovers the original amplitude;
35. one complete layer preserves total positive-booked energy;
36. stored signed ports retain the information needed by the inverse;
37. all locked forward battery steps remain above zero;
38. all locked reverse battery steps recover exactly;
39. the full field, bank, battery, and cursor state reverses exactly;
40. cumulative battery loss equals cumulative source work;
41. the finite-horizon physical balance telescopes exactly;
42. the FTD-0882 limit would consume `||J_s||^2` of battery energy;
43. reserve scale is an input rather than a prediction; and
44. fixed charge polarity is catalytic while the battery carries work.

### C45--C56 — interpretation firewall

45. the construction reuses the existing phase/history rail;
46. no sixth selected v2 type is required;
47. the square-root battery law remains imposed;
48. an autonomous finite cyclic exact recycler closes negative in the
    registered explicit-port class;
49. a finite-horizon ready bank closes positive;
50. a positive reversible source-work register closes positive;
51. a canonical Hamiltonian reservoir remains open;
52. 3D routing and finite-capacity backpressure remain open;
53. moving-source continuity remains open;
54. production migration remains open;
55. quartic-`G*` synchronization remains separate; and
56. the terminal gate executes only if C1--C55 pass.

## 6. Frozen outcomes

- **Outcome A — scoped closure:** all `56/56` pass. Book the finite-horizon
  reversible ready bank, unique positive quadratic battery law in the
  registered class, and the finite cyclic freshness boundary.
- **Outcome B — partial:** provenance passes but one or more algebraic or
  reversal gates fail. Book only the passing exact statements and keep both
  mechanisms open.
- **Outcome C — execution invalid:** any source/protocol hash or terminal-count
  gate fails. Book no theorem from this run.

## 7. Frozen terminal markers

```text
FINITE_PORT_BATTERY_STATUS=SELECTED_REFERENCE_EXISTING_TYPES
FINITE_CYCLIC_FRESH_LAYERS=CAPACITY
FINITE_CYCLIC_INDEFINITE_FRESHNESS=NO
EXACT_REAL_MEMORY_NO_GO=NOT_CLAIMED
POSITIVE_QUADRATIC_BATTERY=UNIQUE_SIGN_PRESERVING_LAW
BATTERY_LAW_STATUS=IMPOSED_REFERENCE
FULL_FINITE_STATE_REVERSIBILITY=EXACT
CANONICAL_HAMILTONIAN_RESERVOIR=OPEN
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR
BORN_BELL_STATUS=UNTOUCHED
```

