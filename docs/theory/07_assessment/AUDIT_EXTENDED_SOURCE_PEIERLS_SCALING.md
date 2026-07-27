# AUDIT — Extended-source Peierls scaling

**Identifier:** `FTD-0555`  
**Status:** `[THEOREM — EXACT SPECTRAL INDEX, CONDITIONAL ON FTD-0541] +
[NUMERICAL FACT — LOCKED QUALIFICATION FAILED]`  
**Locked verdict:** `SCALING_THEOREM_NUMERICAL_CONTROL_FAILED`

## 1. Registration history

The v1 lock contained an impossible raw-volume agreement gate. Before source
implementation or execution, an analytic zero-mode audit showed that the
monopole Coulomb energy has an `O(sqrt(m)/L)` correction. V1 was not run. V2
preserved every source, volume, order, fit window, constant, and all other
gates while replacing that single test with a convergence-direction test.

```text
v1 SHA256 B860E3F51A65EF712E220357C970C7BFBF0C6CEDBC65C71D51A2106CC2042FF4
v2 SHA256 C3BFA9DACE7A7831C9E88BD150310D0CFD9AE74A20BCAFF0AE9F1E5AE57B9A38
```

## 2. Locked run

The MSVC 14.44 Release observer evaluated the fixed binomial source family at
`L=257`, the replication family at `L=129`, and cyclic covariance arms at
`L=65`.

```text
main samples                 20
replication samples          16
rotation samples             12
beta                         0.021892057692994273
worst envelope identity      1.4210854715202004e-14
worst spectral identity      1.4210854715202004e-14
worst slope residual         0.13579580962701732
worst constant rel. error    0.11413876630323899
worst volume Pi difference   0.092066026775131482
worst convergence excess     0.0011571629818133561
worst rotation residual      1.0058868654288013e-13
largest Pi at m=128          2.4036737939517426e-05
minimum Pi improvement       184.85787028760569
registered gate failures     9
```

All four energy, barrier, and relative-barrier slopes fell inside the locked
`0.15` asymptotic windows. The exact identity

```text
C_i/(16U_0)=<((1-cos k_i)/(3+cos k_i))^2>_energy
```

closed at `1.42e-14`. Nevertheless, the campaign is negative because its
gates were conjunctive.

## 3. Exact failures

1. The monopole `m=128` energy and relative constants exceeded the 10% gate;
   the worst miss was `11.4139%`.
2. Seven of the predeclared “larger quotient must be closer” component tests
   failed. The largest excess was `0.1157%`. The leading finite-order and
   finite-volume corrections do not have the assumed common sign for every
   rescaled observable.
3. One cyclic transverse-dipole sum returned
   `1.0058868654288013e-13`, just above the locked `1e-13` covariance gate.

No tolerance was widened, no arm removed, and no alternate fit was used.
CTest records the expected negative verdict through a required output regular
expression; a missing verdict or crash still fails the test.

## 4. Licensed conclusion

The locked finite-volume campaign does not qualify the advertised numerical
control family. It supplies no `[EMERGENT]` carrier, particlehood, or mobility
claim.

The analytic result is narrower and survives independently: the exact
spectral-index identity is an algebraic consequence of the already selected
quadratic coat, and the binomial asymptotics follow by dominated rescaling.
The numerical data are consistent with those exponents but do not promote or
validate them under the locked protocol.

The most important new discriminator is therefore exact rather than fitted:
future native histories must reduce

```text
Pi_i=Delta U_i/U_0
```

by moving field-energy weight toward low momentum. Increasing geometric radius
while retaining lattice-scale alternating `s` content is not sufficient.

## 5. Production boundary

- No `RenderBridge` state or phase changed.
- No toggle, force, scenario, constant, or WASM interface was added.
- The binomial profiles are continuous-field observers, not fractional ternary
  states.
- The legacy soliton sweep does not provide a qualified carrier family.
- FTD-0399 remains invalid before particlehood and cannot be reused as this
  theorem's native realization.
