# PRE-REGISTRATION — Fixed-step variational energy scope

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0543`  
**Status:** `[PRE-REGISTRATION — LOCKED/NOT YET RUN]`  
**Parents:** `FTD-0479`, `FTD-0536`, `FTD-0539`, `FTD-0542`  
**Scope:** theorem-level audit of what an autonomous fixed-duration discrete
action does and does not imply about exact energy. This record changes no FTD
dynamics, action coefficient, field representation, energy ledger, primitive,
toggle, default, or scenario.

## 1. Locked general derivation

For an autonomous discrete Lagrangian `L_d(q_n,q_(n+1);h_n)`, derive the
configuration equation

```text
D_2 L_d(q_(n-1),q_n;h_(n-1))
+D_1 L_d(q_n,q_(n+1);h_n)=0.                       (1)
```

With fixed time nodes, (1) is the complete variational equation; it contains
no independent duration equation. If the interior time node is varied so that
`h_(n-1)=t_n-t_(n-1)` and `h_n=t_(n+1)-t_n`, the additional equation is

```text
D_h L_d(q_(n-1),q_n;h_(n-1))
-D_h L_d(q_n,q_(n+1);h_n)=0.                       (2)
```

Thus `E_d=-D_h L_d` is conserved only when (2) is imposed, or when a separate
special identity supplies an invariant. The locked statement is not that
fixed-step schemes can never preserve energy; it is that fixed-step action
stationarity alone does not prove it.

## 2. Locked exact counterexample

Use the one-dimensional quartic oscillator and no fitted parameter:

```text
H(q,p)=p^2/2+q^4/4,
L_d(q0,q1;h)=(q1-q0)^2/(2h)-h[(q0+q1)/2]^4/4.     (3)
```

The discrete Legendre maps are

```text
p0=(q1-q0)/h+h m^3/2,
p1=(q1-q0)/h-h m^3/2,
m=(q0+q1)/2.                                       (4)
```

Prove exactly

```text
H(q1,p1)-H(q0,p0)=(q0+q1)(q1-q0)^3/8.             (5)
```

At `q0=0`, `q1=1`, `h=1`, require the rational witness

```text
p0=17/16,
p1=15/16,
Delta H=1/8,
-D_h L_d=33/64.                                    (6)
```

Equation (6) disproves any inference from fixed-step Legendre stationarity to
conservation of the endpoint Hamiltonian.

## 3. Locked discrete-gradient comparison

Replace the midpoint force by the exact divided difference

```text
g(q0,q1)=[V(q1)-V(q0)]/(q1-q0),
q1-q0=h(p1+p0)/2,
p1-p0=-h g(q0,q1).                                 (7)
```

Prove that (7) conserves `H` exactly. For the quartic potential, compute the
one-step phase-area determinant

```text
det D Phi = [1+h^2 g_0/2]/[1+h^2 g_1/2],           (8)
```

where `g_0` and `g_1` are the derivatives of `g` with respect to its first and
second arguments. Exhibit a nonzero symplectic/area defect away from
`q1^2=q0^2`. This is a concrete price statement, not a universal no-go theorem
for all energy-preserving variational constructions.

## 4. Locked gates and verdicts

Require all algebraic identities and the rational witness below `1e-14` in
double evaluation. Require the discrete-gradient energy defect below `1e-14`
and its registered generic area defect above `1e-6`.

- all gates close:
  `FIXED_STEP_ACTION_ENERGY_NOT_AUTOMATIC`;
- the midpoint witness conserves endpoint energy:
  `FIXED_STEP_COUNTEREXAMPLE_INVALID`;
- the discrete-gradient comparison fails exact energy:
  `DISCRETE_GRADIENT_COMPARISON_INVALID`;
- only floating evaluation fails:
  `FIXED_STEP_ENERGY_SCOPE_UNRESOLVED`.

A constructive verdict requires the next coat-based mobile candidate to choose
and preregister one of three logically distinct routes: vary a temporal/lapse
degree of freedom, derive a special exact invariant of the fixed-step action,
or use a separately justified energy-preserving discrete-gradient map and pay
its non-variational/symplectic price. It does not choose among those routes.
