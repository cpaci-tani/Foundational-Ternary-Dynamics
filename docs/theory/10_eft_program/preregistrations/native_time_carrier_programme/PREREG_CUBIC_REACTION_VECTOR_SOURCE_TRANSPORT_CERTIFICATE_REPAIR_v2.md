# FTD-0890 — cubic reaction-vector/source-transport certificate repair v2

**Identifier:** `FTD-0890`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** `FTD-0889`  
**Production status:** unchanged

## 1. Failure being repaired

The first locked FTD-0889 execution reported `64/68`. All eight source hashes,
the protocol hash, the cubic scalar-to-vector no-go, minimum canonical triplet,
Jacobian/eigenvalue, inverse chart, low-energy mass relation, free transport,
quotient, face-current, field-impulse, energy-split, and scope gates passed.
Three certificate expressions failed:

1. C30 asked SymPy to reduce a positive square root of a perfect square without
   explicitly supplying the already frozen positive branch;
2. C54 asked SymPy's assumption engine to mark
   `1/(2 sqrt(z) sqrt(1-z))` positive while declaring only `z>0`, although the
   frozen compatibility interval also has `z<1`;
3. C66 searched raw line-wrapped protocol text for two scope phrases.

C68 then failed only because it depends on C1--C67. These are certificate
representation defects. No source, representation, equation, dispersion,
symplectic map, energy identity, compatibility condition, outcome, or scope
ceiling may change.

## 2. Frozen parent hashes

| artifact | SHA256 |
|---|---|
| `PREREG_CUBIC_REACTION_VECTOR_RELATIVISTIC_SOURCE_TRANSPORT_v1.md` | `A92F0BFB95993971AB80661B39296E948BA68E52ADED6D4A3DAF92804DB37F66` |
| `scripts/proofs/proof_cubic_reaction_vector_relativistic_source_transport.py` | `D8A8D80E1E6E497C08E7011ED7731E27C2B0B221EB894D3E9C8A61C89CF1EA0F` |

Both parent artifacts remain byte-frozen. Any mismatch invalidates the repair.

## 3. Exactly permitted in-memory substitutions

The wrapper must find exactly one occurrence of each old anchor and apply only
these substitutions.

### R1 — C30 positive-square normalization

```python
check("reaction norm maps exactly to relativistic kinetic energy",
      sp.simplify(kinetic_from_chart - rho**2 / 2) == 0)
```

becomes

```python
check("reaction norm maps exactly to relativistic kinetic energy",
      sp.simplify(E0**2 + c**2 * alpha**2 * rho**2
                  - (E0 + rho**2 / 2)**2) == 0
      and (E0 + rho**2 / 2).is_positive is True)
```

This verifies the frozen radicand is exactly the square of the frozen positive
branch; it changes no equation.

### R2 — C54 interval-complete monotonicity normalization

```python
check("compatibility interval maps uniquely into eta in zero to pi over two",
      sp.diff(sp.asin(sp.sqrt(sp.symbols("z", positive=True))),
              sp.symbols("z", positive=True)).is_positive is True)
```

becomes

```python
t_interval = sp.symbols("t_interval", positive=True)
z_interval = t_interval / (1 + t_interval)
eta_interval_derivative = sp.simplify(
    sp.diff(sp.asin(sp.sqrt(z_interval)), t_interval))
check("compatibility interval maps uniquely into eta in zero to pi over two",
      eta_interval_derivative == 1 / (
          2 * sp.sqrt(t_interval) * (1 + t_interval))
      and eta_interval_derivative.is_positive is True)
```

The bijection `z=t/(1+t)` supplies exactly the frozen open interval `0<z<1`.

### R3 — C66 protocol-whitespace normalization

```python
check("mass scale and common-action coupling remain open",
      "does not determine `E0`, `c`," in PROTOCOL.read_text(encoding="utf-8")
      and "full common-action coupling" in PROTOCOL.read_text(encoding="utf-8"))
```

becomes

```python
protocol_flat = " ".join(PROTOCOL.read_text(encoding="utf-8").split())
check("mass scale and common-action coupling remain open",
      "does not determine `E0`, `c`," in protocol_flat
      and "full common-action coupling" in protocol_flat)
```

The wrapper must verify each old anchor occurs exactly once, each replacement
is absent initially and present exactly once afterward, and both parent hashes
match before executing the repaired source in memory.

## 4. Inherited gates and outcome

All 68 FTD-0889 checks, their order, eight source hashes, exact representation
and symplectic algebra, selected dispersion, transport continuation,
energy/momentum compatibility law, terminal markers, and outcome map are
inherited unchanged. The only expected effect is that C30, C54, and C66
recognize the evidence already frozen in FTD-0889; C68 then passes if C1--C67
pass.

## 5. Scope firewall

```text
REPAIR_SCOPE=C30_C54_C66_REPRESENTATION_NORMALIZATION_ONLY
PARENT_PROTOCOL_UNCHANGED=TRUE
PARENT_CERTIFICATE_UNCHANGED=TRUE
SCALAR_REACTION_TO_SPATIAL_VECTOR=FORBIDDEN_BY_CUBIC_SYMMETRY
ORIENTATION_FREE_SPATIAL_REACTION=THREE_CANONICAL_PAIRS_MINIMUM
RELATIVISTIC_REACTION_TO_MOMENTUM_CHART=CONDITIONAL_ON_SELECTED_DISPERSION
INERTIAL_MASS_SCALE=NOT_DERIVED
NATIVE_VECTOR_COMMON_ACTION=OPEN
PRODUCTION_COUPLING=NONE
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

## 6. Pre-run lock

The exact SHA256 of this repair protocol and its wrapper must be recorded in
the preregistration manifest before first execution.
