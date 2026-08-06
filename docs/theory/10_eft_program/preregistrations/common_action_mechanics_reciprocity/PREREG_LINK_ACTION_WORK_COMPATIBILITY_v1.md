# PRE-REGISTRATION — Link-action work compatibility v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0470`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0443`, `FTD-0447`, `FTD-0468`, `FTD-0469`  
**Engine artifact:** `engine/tests/campaign_link_action_work_compatibility.cpp`  
**Campaign SHA256:** `CDCD31B8EC37B6D997C8655945A7AC2F69E63CC6C2396A902E7590AE3FFC76A5`  
**Helper SHA256:** `47D10DDFD14AF34BCDD5EF02C11DCE96713818717A0430E49712787058643069`

## 1. Question

FTD-0443 proved that moving charge `q` from lattice site `a` to a face
neighbour `b` changes the written interaction by the exact endpoint work

```text
W_hop = G_C q [div(J)(b) - div(J)(a)].
```

FTD-0468 proved momentum reciprocity for the site force
`G_C q grad(div J)`. FTD-0469 showed that applying that force to frozen sites
creates unfunded kinetic energy. This campaign asks whether trapezoidally
centering the site force at the two hop endpoints actually integrates to the
exact finite-hop work, or whether an oriented link difference is required.

No production hop is executed. This is an observer-only compatibility test of
the frozen interaction and operators.

## 2. Pre-derived identities

For a face displacement `d=b-a`, `|d|^2=1`, define

```text
I_site = (G_C q/2)[grad divJ(a) + grad divJ(b)],
W_site = I_site . d,
I_link = W_hop d.
```

- **L1 (exact link work):** `I_link.d=W_hop` identically.
- **L2 (polynomial controls):** the centered site work is exact when
  `div(J)` is affine or quadratic along the link. For the registered cubic
  potential at the centered source, `W_site=(5/2)W_hop`, so the defect is
  `(3/2)W_hop`.
- **L3 (Fourier symbol):** for a periodic longitudinal mode
  `div(J)=A sin(kx+phi)`,

  ```text
  W_site / W_hop = cos^2(k/2),
  (W_hop-W_site)/W_hop = sin^2(k/2).
  ```

  Thus the site force is an infrared approximation with `O((ka)^2)` work
  error, not an exact finite-link derivative.

These are discrete finite-lattice statements; no continuum limit is used.

## 3. Frozen fixtures

1. **Polynomial controls:** `L=17`; three axes, both link directions, both
   charge polarities; exact primitives whose central divergence is
   `a r`, `a r^2`, or `a r^3`, with `a=1e-3`. Total: 36 rows.
2. **Fourier symbol:** `L=32`; modes `n={1,2,4,8,12,15}`, three axes, both
   directions, both polarities, amplitude `0.01`, phase `0.37`. Total: 72
   rows.
3. **Evolving native histories:** `L=33`; locked opposite-polarity pair at
   separation 8 plus the FTD-0468 longitudinal travelling mode `n=2`, phase
   `0.37`; wave and coupling toggles only. At each of 64 ticks, measure both
   directions from both sources for each of six axis/orientation arms. Total:
   1536 records.

All bridges force CPU after construction. The production tick, source, force,
and movement rules are unchanged.

## 4. Gates

- finite values and CPU backend in every arm;
- every static exact work magnitude above `1e-10`;
- affine/quadratic centered-work residual `<=1e-12`;
- cubic `(3/2)W_hop` defect-formula residual `<=1e-12` and nonzero defect;
- Fourier `cos^2(k/2)` formula residual `<=1e-12` and a nonzero defect for
  every mode except the lowest control, which is recorded without a defect
  floor;
- exact-link residual `<=1e-12` in every static and dynamic record;
- evolving-history exact-work RMS above `1e-10` and centered-defect RMS above
  `1e-10` in every arm.

The dynamic defect floor is a mechanism discriminator, not a claim about its
experimental size.

## 5. Outcome map

- all exact identities pass and every dynamic arm has nonzero mismatch:
  `SITE_GRADIENT_IS_IR_APPROXIMATION_LINK_DIFFERENCE_IS_EXACT`;
- exact identities pass but native histories have no registered mismatch:
  `NATIVE_HISTORIES_ACCIDENTALLY_CLOSE_SITE_GRADIENT`;
- an exact operator/formula gate fails with valid protocol:
  `LINK_ACTION_WORK_IDENTITY_FAILS`;
- invalid/nontriviality gate fails: `PROTOCOL_INVALID`.

No outcome derives a production hop rule. A positive exact-link result only
selects the scalar longitudinal event impulse already isolated by FTD-0447;
field recoil, source transport, edge/corner routing, kinetic update, and
reversal remain separate gates.

## 6. Run of record

Pinned MSVC `14.44.35207`, Release, CPU observer, focused target
`campaign_link_action_work_compatibility`, output
`engine/results/ftd_0470/windows_msvc_cpu.csv`.

**Recorded outcome:**
`SITE_GRADIENT_IS_IR_APPROXIMATION_LINK_DIFFERENCE_IS_EXACT`. The worst
Fourier formula residual was `1.84e-18`, all exact-link residuals were zero,
and all six evolving-history arms had centered-work defect RMS above
`5.88e-3`. See `AUDIT_LINK_ACTION_WORK_COMPATIBILITY.md`.
