"""Independent certificate for FTD-0708 full-coordinate L=33 rest solve."""
from __future__ import annotations
import csv, hashlib, json, math
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
R=ROOT/"engine/results/ftd_0708"
SUMMARY=R/"ftd_0708_l33_full_impulse_rest_solve_v1.json"
ITER=R/"ftd_0708_l33_full_impulse_rest_solve_iterations_v1.csv"
STATE=R/"ftd_0708_l33_full_impulse_rest_solve_state_v1.csv"
TICKS=R/"ftd_0708_l33_full_impulse_rest_solve_ticks_v1.csv"
RUNNER=ROOT/"engine/tests/test_l33_full_impulse_rest_solve.cpp"
PREREG=ROOT/"docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_L33_FULL_IMPULSE_REST_SOLVE_v1.md"
PROTOCOL="D978E8920D8121CA2FC91F3E6B4F68353B98E7B6285B4A82304511EE4177D007"
HASHES={SUMMARY:"C6CDA86233BF88EE4DA8244599F29AFE41EE0F4746597D4C01DCFB1F085B51B6",
ITER:"5F71ABC1D5FA488D95323FBE3B0FC2EA75C5BAC8122B506637AF826813C710B8",
STATE:"1D40BFBA62C81F71ACD033C030C8EB640936773AAD5B9449EB8BA02042705F44",
TICKS:"25400F4503C4350A27DFC045E1AD51473A3CE1F09A03C5BF8D91133A1B45F7B6",
RUNNER:"F18ED45C8E0859C02F754B2685C77B8305757042CE4BB7EB989279984DD7C372",
PREREG:PROTOCOL}
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest().upper()
for path,expected in HASHES.items():assert sha(path)==expected,path
s=json.loads(SUMMARY.read_text());assert s["protocol_sha256"]==PROTOCOL
assert s["verdict"]=="L33_FULL_IMPULSE_REST_FIXED_POINT_CONSTRUCTIVE"
assert s["production_changed"] is False and s["volume"]==33
for gate in ("evaluations_pass","linear_algebra_pass","root_pass","one_step_pass",
             "forward_pass","reverse_pass","covariance_pass"):assert s[gate]==1
assert s["accepted_steps"]==1 and s["evaluations"]==98
assert s["final_residual"]<=1e-9<s["starting_residual"]
assert s["maximum_displacement"]<=0.05
assert s["one_step_state"]<=1e-9 and s["one_step_momentum"]<=1e-9
assert s["total_hops"]==0 and s["maximum_state"]<=1e-8
assert s["maximum_center"]<=1e-10 and s["maximum_energy"]<=1e-10
assert s["maximum_common"]<=1e-10 and s["recovery"]<=1e-9
assert s["covariance_residual"]<=1e-9
with ITER.open(newline="") as f: rows=list(csv.DictReader(f))
assert len(rows)==1 and float(rows[0]["accepted_scale"])==1.0
assert int(rows[0]["evaluations"])==96 and float(rows[0]["minimum_pivot"])>1e-10
with STATE.open(newline="") as f: states=list(csv.DictReader(f))
with TICKS.open(newline="") as f: ticks=list(csv.DictReader(f))
assert len(states)==16 and len(ticks)==8
assert max(abs(float(row[k])) for row in states for k in ("dx","dy","dz"))==s["maximum_displacement"]
assert all(int(row["hops"])==0 for row in ticks)
print("FTD-0708 L=33 full-impulse rest certificate: PASS")
print(f"residual={s['starting_residual']:.6e}->{s['final_residual']:.6e} "
      f"dx={s['maximum_displacement']:.6e}")

