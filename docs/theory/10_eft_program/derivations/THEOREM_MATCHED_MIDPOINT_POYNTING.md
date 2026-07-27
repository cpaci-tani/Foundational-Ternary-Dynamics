# THEOREM — Exact matched midpoint Poynting identity

**Identifier:** `FTD-0544`  
**Status:** `[THEOREM — EXACT AUXILIARY FIELD ENERGY AND GAUSS TRANSPORT]`  
**Inputs:** the matched face/edge complex and any exact conservative face
current, including FTD-0541.

## Derivation

Let

```text
Ebar=(E0+E1)/2,
Bbar=(B0+B1)/2,
E1-E0=h C Bbar-K,
B1-B0=-h C^T Ebar.                                 (1)
```

For normalized field energy

```text
U(E,B)=(||E||^2+||B||^2)/2,
```

the polarization identity gives

```text
U1-U0
=<Ebar,E1-E0>+<Bbar,B1-B0>
=h<Ebar,C Bbar>-<Ebar,K>-h<Bbar,C^T Ebar>
=-<Ebar,K>.                                        (2)
```

The curl terms cancel because `C^T` is the exact periodic transpose of `C`.
This is the discrete Poynting/work theorem. Magnetic exchange redistributes
field energy but contributes zero to the total scalar ledger.

Taking divergence of the electric update and using `div C=0` gives

```text
div E1-div E0=-div K.                              (3)
```

Therefore, if `rho_n=div E_n`, equation (3) is exactly

```text
rho1-rho0+div K=0.                                 (4)
```

Gauss propagation and field-energy exchange are consequently compatible
without projection.

## Boundary

Equation (2) identifies the exact amount the matter sector must gain:
`Delta H_matter=<Ebar,K>`. It does not prove that spatial endpoint derivatives
of the fixed-step particle action generate a momentum update satisfying that
production-dispersion identity. FTD-0543 shows why that is an independent
gate. No particle force or mobile dynamics follows from (2) alone.
