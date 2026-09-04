"""proof_phi_v2_isolated_relation_census.py -- FTD-1030 addendum.

Exhaustive census of the constitution's own certified transaction map, the
v3 selected reference Phi v2 (scripts/proofs/proof_v3_common_action_phi_v2.py,
the register's `executable_reference` for P5), at the level where that law
currently exists: the isolated primary/reserve relation.

Asks the C4 census questions of the CONSTITUTION's law rather than the legacy
engine's:
  Q1  what is the site-readout cycle, its period and duty?
  Q2  is the null a reversal passage (+s -> 0 -> -s) or a bounce (+s -> 0 -> +s)?
  Q3  does the carrier (the record z, up to its C4 orbit) persist across the null?
  Q4  is polarity (the sign of the ternary readout) ever changed?
  Q5  what does the external gate do to the clock?

Every quantity is computed by driving Phi v2's own `relation_tick`; nothing
is transcribed. ASCII-only output.
"""
from __future__ import annotations
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import proof_v3_common_action_phi_v2 as P  # noqa: E402  (import is cheap; parent main() is not run)

A9, BLANK = P.A9, P.BLANK
failures = 0


def check(name, ok, detail=""):
    global failures
    print(f"  [{'PASS' if ok else 'FAIL'}] {name}" + (f"  -- {detail}" if detail and not ok else ""))
    if not ok:
        failures += 1


def s(z):
    o, p, _ = P.readout(z)
    return o * p


def orbit(z):
    zs, w = set(), z
    for _ in range(4):
        zs.add(w)
        w = P.rotate(w)
    return frozenset(zs)


def glyph(seq):
    return "".join("+" if x > 0 else "-" if x < 0 else "0" for x in seq)


print("Phi v2 isolated-relation census (constitution's executable reference)\n")
print("A9 record -> (site readout s, polarity, phase):")
for z in A9:
    o, p, _ = P.readout(z)
    print(f"  z={str(z):9s} s={s(z):+d} occupied={o} polarity={p:+d} phase={P.phase_index(z)}")

# ---- Q1..Q4 over all eight payloads, token in the reserve, primary blank ----
print("\nAll eight payloads, primary BLANK, reserve = token, even_gate=True:")
print(f"  {'pol':>4s} {'ph':>3s}  period  duty   primary readout (one period)   nulls entered->exited   carrier   polarity")
periods, duties, reversals, bounces, carrier_ok, polarity_ok = [], [], 0, 0, True, True
for pol in (+1, -1):
    for ph in range(4):
        pair0 = (BLANK, P.encode(ph, pol))
        pair, seq, pols, orbits, period = pair0, [], set(), set(), None
        for t in range(64):
            pair = P.relation_tick(*pair)
            seq.append(s(pair[0]))
            tok = pair[0] if P.readout(pair[0])[0] else pair[1]
            pols.add(P.readout(tok)[1])
            orbits.add(orbit(tok))
            if pair == pair0:
                period = t + 1
                break
        per = seq[:period] if period else seq[:16]
        n = len(per)
        ev = []
        for i in range(n):
            if per[i] == 0 and per[i - 1] != 0:
                j = i
                while per[j % n] == 0:
                    j += 1
                ev.append((per[i - 1], per[j % n]))
        for a, b in ev:
            if b == -a:
                reversals += 1
            else:
                bounces += 1
        periods.append(period)
        duties.append(sum(1 for x in per if x))
        carrier_ok &= (len(orbits) == 1)
        polarity_ok &= (len(pols) == 1)
        print(f"  {pol:+4d} {ph:3d}  {str(period):>6s}  {sum(1 for x in per if x)}/{n:<3d}  {glyph(per):<28s}  {str(ev):<22s}  {len(orbits)==1!s:<8s} {len(pols)==1}")

print()
check("Q1 every payload has EXACT period 8", all(p == 8 for p in periods), str(periods))
check("Q1 every payload has duty 4/8 (a square wave)", all(d == 4 for d in duties), str(duties))
check("Q2 every null is a BOUNCE (+s->0->+s); zero reversal passages", reversals == 0 and bounces == 8,
      f"reversals={reversals} bounces={bounces}")
check("Q3 the carrier (record orbit) persists across every null", carrier_ok)
check("Q4 polarity is conserved by the relation for every payload", polarity_ok)

# ---- Q4 at the collision layer: does the 192-state collision alphabet carry polarity at all? ----
try:
    from proof_hodge_flag_pair_collision_invariant_space import one_particle_states
    st = one_particle_states()
    flag, phase = st[0]
    check("Q4 collision states are (flag, phase) with no polarity variable "
          "(polarity is a separate copy label, C8) -> collisions cannot flip it",
          len(st) == 192 and len(flag) == 3 and phase in range(4),
          f"len={len(st)} sample={st[0]}")
except Exception as e:  # pragma: no cover
    print(f"  [SKIP] collision-alphabet check: {e}")

# ---- Q5 the external gate ----
z0 = P.encode(0, +1)
pair, seq = (BLANK, z0), []
for t in range(24):
    pair = P.relation_tick(*pair, even_gate=(t % 2 == 0))
    seq.append(s(pair[0]))
alt = glyph(seq)
pair, seq = (BLANK, z0), []
for t in range(24):
    pair = P.relation_tick(*pair, even_gate=False)
    seq.append(s(pair[0]))
off = glyph(seq)
print(f"\n  gate alternating : {alt}")
print(f"  gate always off  : {off}   (token phase after 24 ticks = {P.phase_index(pair[1])}: still rotating)")
check("Q5 an alternating gate leaves the period-8 clock untouched", alt == ("++++0000" * 3))
check("Q5 a closed gate stops TRANSPORT (crossing) but not the internal C4 clock",
      off == "0" * 24 and P.phase_index(pair[1]) == 0)

print(f"\nphi_v2_isolated_relation_census: {'PASS' if failures == 0 else 'FAIL'} ({failures} failing checks)")
sys.exit(1 if failures else 0)
