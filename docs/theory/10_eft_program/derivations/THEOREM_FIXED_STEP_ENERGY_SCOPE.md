# THEOREM — Fixed-step action does not automatically conserve endpoint energy

**Identifier:** `FTD-0543`  
**Status:** `[THEOREM — VARIATIONAL SCOPE]` +
`[CONSTRUCTIVE — EXACT COUNTEREXAMPLE AND DISCRETE-GRADIENT PRICE]`

## 1. Configuration and time variations are different equations

For an autonomous discrete Lagrangian

```text
L_d(q_n,q_(n+1);h_n),
h_n=t_(n+1)-t_n,
```

variation of `q_n` gives

```text
D_2 L_d(q_(n-1),q_n;h_(n-1))
+D_1 L_d(q_n,q_(n+1);h_n)=0.                       (1)
```

If the time nodes are frozen, (1) exhausts stationarity. If `t_n` is varied,
then `delta h_(n-1)=delta t_n` and `delta h_n=-delta t_n`, producing the
independent equation

```text
D_h L_d(q_(n-1),q_n;h_(n-1))
-D_h L_d(q_n,q_(n+1);h_n)=0.                       (2)
```

Thus the segment quantity `E_d=-D_h L_d` is conserved by the extended
variational principle. Equation (2) is absent from a fixed-tick principle.
This proves only non-implication: a special fixed-step map may still possess
an independently proven energy invariant.

## 2. Exact quartic counterexample

Take

```text
H(q,p)=p^2/2+q^4/4,
L_d=(q1-q0)^2/(2h)-h[(q0+q1)/2]^4/4.
```

Writing `Delta=q1-q0` and `m=(q0+q1)/2`, the discrete Legendre transforms are

```text
p0=Delta/h+h m^3/2,
p1=Delta/h-h m^3/2.                                (3)
```

Their endpoint-energy difference is

```text
Delta H
=(p1-p0)(p1+p0)/2+(q1^4-q0^4)/4
=(-h m^3)(Delta/h)+Delta(2m)(q1^2+q0^2)/4
=m Delta[(q1^2+q0^2)/2-m^2]
=m Delta^3/4
=(q0+q1)(q1-q0)^3/8.                              (4)
```

For `q0=0`, `q1=1`, `h=1`, equation (3) gives

```text
p0=17/16,
p1=15/16,
H1-H0=1/8.                                         (5)
```

Meanwhile the duration derivative is

```text
-D_h L_d=Delta^2/(2h^2)+m^4/4=33/64.               (6)
```

The path satisfies the fixed-step Legendre map exactly while the endpoint
Hamiltonian changes by `1/8`. Therefore spatial/configuration stationarity is
not an energy proof.

## 3. Exact energy has a structural price

The divided-difference update

```text
q1-q0=h(p1+p0)/2,
p1-p0=-h [V(q1)-V(q0)]/(q1-q0)                    (7)
```

conserves `H` exactly because the kinetic change is
`(p1+p0)(p1-p0)/2` and the potential change is the divided difference times
`q1-q0`; (7) cancels them identically.

For `V=q^4/4`, let `g` be the divided difference and `g_0,g_1` its two
partial derivatives. Implicit differentiation of (7) gives

```text
det D Phi=(1+h^2 g_0/2)/(1+h^2 g_1/2).             (8)
```

At the same registered endpoint pair this is `9/11`, not one. The simple
energy-preserving discrete gradient therefore loses phase-area preservation
generically. This is a price witness, not a universal theorem excluding every
more elaborate construction.

## 4. Consequence for reciprocal mobile matter

The smooth coat has solved the spatial representation and common-source
problems. The exact-energy gate now requires an explicit choice:

1. vary a temporal/lapse variable and impose (2);
2. derive a special fixed-step invariant of the full coat-field action; or
3. select an energy-preserving discrete-gradient map and independently audit
   reversibility, phase-volume/spectral positivity, and common interaction
   origin.

Silently treating the fixed production tick as if it supplied equation (2)
is mathematically invalid.
