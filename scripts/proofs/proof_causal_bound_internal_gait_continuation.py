"""Independent certificate for FTD-0713 causal internal-gait continuation."""
from __future__ import annotations
import csv, hashlib, json, math
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
R=ROOT/"engine/results/ftd_0713"
SUMMARY=R/"ftd_0713_causal_bound_internal_gait_continuation_v1.json"
ITER=R/"ftd_0713_causal_bound_internal_gait_iterations_v1.csv"
STATE=R/"ftd_0713_causal_bound_internal_gait_state_v1.csv"
RUNNER=ROOT/"engine/tests/test_causal_bound_internal_gait_continuation.cpp"
PREREG=ROOT/"docs/theory/10_eft_program/preregistrations/PREREG_CAUSAL_BOUND_INTERNAL_GAIT_CONTINUATION_v1.md"
PROTOCOL="901F2F2FDACEB47D62ED57EE0E4E114B1C4C29C6DF7F8188EA39E86F3DC724BF"
HASHES={SUMMARY:"E32B537808A128B4B080FE2EC6B42C4DF5E494F87E9D1EF09CB86CBB88DFE051",
ITER:"0F57ECD24388A9AE4B7CCD43402E911501E3046027EC627C6C7CF266DA001D34",
STATE:"6C9B7684DBEB2976823B2A0B908407ED201253E4A6ED22D1F83B053712C4ACDF",
RUNNER:"B4A0CB9824A10EAC2FCCF005A43430F5949D16C2093A742B903C2A703BFF08D1",
PREREG:PROTOCOL}
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest().upper()
for path,expected in HASHES.items():assert sha(path)==expected,path
s=json.loads(SUMMARY.read_text());assert s["protocol_sha256"]==PROTOCOL
assert s["verdict"]=="CAUSAL_INTERNAL_GAIT_CANCELLATION_CONSTRUCTIVE"
for gate in ("parent_pass","reconstruction_pass","mode_algebra_pass",
             "state_load_pass","parent_crosscheck_pass","evaluation_pass",
             "linear_algebra_pass","root_pass","conjugacy_pass","covariance_pass"):
    assert s[gate]==1
assert s["accepted_steps"]==2 and s["final_residual"]<=1e-10
assert s["final_null_norm"]<=1e-10<s["starting_null_norm"]
assert s["maximum_displacement"]>0.05
assert s["maximum_speed"]<=1/math.sqrt(3)+1e-12
assert s["edge_deformation"]<=0.10 and s["center_residual"]<=1e-14
assert s["continuity_residual"]<=1e-12 and s["covariance_residual"]<=1e-10
with ITER.open(newline="") as f:iterations=list(csv.DictReader(f))
assert len(iterations)==2 and all(float(r["accepted_scale"])==1 for r in iterations)
with STATE.open(newline="") as f:states=list(csv.DictReader(f))
assert len(states)==16
deltas=[tuple(float(r[k]) for k in ("dx","dy","dz")) for r in states]
assert max(abs(sum(d[a] for d in deltas)) for a in range(3))<=1e-14
print("FTD-0713 causal internal-gait continuation certificate: PASS")
print(f"null={s['starting_null_norm']:.6e}->{s['final_null_norm']:.6e} vmax={s['maximum_speed']:.6f}")
