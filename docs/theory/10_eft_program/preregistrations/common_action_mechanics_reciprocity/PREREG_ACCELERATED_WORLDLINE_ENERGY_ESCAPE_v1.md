# PRE-REGISTRATION — Accelerated-worldline energy escape

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0547`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0490`, `FTD-0543`, `FTD-0545`, `FTD-0546`  
**Scope:** observer-only exact one-dimensional/collinear relativistic motion in
a uniform electric force. No production kinetic law, current, force, toggle,
default, scenario, or tolerance is changed.

## 1. Locked exact trajectory

Use

```text
H(p)=sqrt(M^2+c^2 p^2),
p0=p-a,
p1=p+a,
F=2a/h.                                           (1)
```

The frozen straight/free-midpoint rule uses

```text
v_mid=H'(p)=c^2 p/H(p),
d_mid=h v_mid.                                    (2)
```

The exact constant-force solution is

```text
p(tau)=p-a+2a tau,
x(tau)-x0=h[H(p(tau))-H(p-a)]/(2a),
v_sec=[H(p+a)-H(p-a)]/(2a),
d_exact=h v_sec.                                  (3)
```

Use the continuous `a->0` limit `v_sec=H'(p)`. Require direct differentiation
of (3), endpoint reconstruction, `|v_sec|<c`, and polarity/time-reversal
relations below `1e-14`.

## 2. Locked work identities

Define

```text
D_mid=H(p+a)-H(p-a)-2a v_mid,
D_exact=H(p+a)-H(p-a)-2a v_sec.                  (4)
```

Require

```text
D_exact=0,
D_mid=2a(v_sec-v_mid)                             (5)
```

below `1e-14`. The result must reproduce the analytic FTD-0545 defect and its
small-`a` leading term

```text
D_mid=-(c^4 M^2 p/H(p)^5)a^3+O(a^5).             (6)
```

The exact trajectory must differ from uniform interpolation between its exact
endpoints for at least one nonzero massive arm. Record the maximum midpoint
schedule deviation.

## 3. Locked arms and verdicts

Use `M=0.511`, `h=c=C_SPEED`, momenta `0.1,0.2,0.3`, field amplitudes
`0.04,0.08,0.12`, both charge signs, `beta=1` and the FTD-0478 native beta,
and directions `<100>,<010>,<001>,<111>`. Here `a=beta q E/2`. This gives
`144` registered arms. Add zero-force and invalid-input controls.

- every identity closes and a nonuniform schedule is present:
  `UNIFORM_ACCELERATED_WORLDLINE_REPAIRS_WORK_EXACTLY`;
- exact work fails:
  `ACCELERATED_WORLDLINE_ENERGY_ESCAPE_INVALID`;
- only numerical limiting controls fail:
  `ACCELERATED_WORLDLINE_ENERGY_ESCAPE_UNRESOLVED`.

A constructive result identifies the FTD-0545/0546 defect as a straight/free
within-tick kinematic error in this integrable sector. It does not yet provide
a general field-dependent worldline action. The next gate must rederive
`K0`, `K1`, and `T` on the nonuniform temporal schedule; reusing the linear
schedule deposits is forbidden.
