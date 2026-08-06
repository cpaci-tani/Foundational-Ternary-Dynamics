"""Independent certificate for FTD-0712 bounded internal-gait cancellation."""
from __future__ import annotations
import csv, hashlib, json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
R=ROOT/"engine/results/ftd_0712"
SUMMARY=R/"ftd_0712_resonant_internal_gait_cancellation_v1.json"
ITER=R/"ftd_0712_resonant_internal_gait_iterations_v1.csv"
STATE=R/"ftd_0712_resonant_internal_gait_state_v1.csv"
RUNNER=ROOT/"engine/tests/test_resonant_internal_gait_cancellation.cpp"
PREREG=ROOT/"docs/theory/10_eft_program/preregistrations/PREREG_RESONANT_INTERNAL_GAIT_CANCELLATION_v1.md"
PROTOCOL="47BC6C8897FFFC0C983FDA6BB73910C6FADE87206544DE74BF63E7F52E344852"
HASHES={SUMMARY:"DB9E76C4EEB0AF2C599B6C63FD520E3E58862F15272DD2EA9A4445006E8E21FB",
ITER:"8FDF21423EB9483098B3A43E3936665E306F88200B83521A6BF613344647F084",
STATE:"40CD492F9766FB1DC701CF71CA51B0B91D2B2C5E7464785F6AE2A7433FB84030",
RUNNER:"90C058BBD042E4CDE7B65C8638C49C66929D271CAE23EF4FD1778BF87E2CD6E8",
PREREG:PROTOCOL}
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest().upper()
for path,expected in HASHES.items():assert sha(path)==expected,path
s=json.loads(SUMMARY.read_text());assert s["protocol_sha256"]==PROTOCOL
assert s["verdict"]=="BOUNDED_INTERNAL_GAIT_CANNOT_CANCEL_LOCKED_RESONANCE"
for gate in ("parent_rest_pass","parent_spectral_pass","reconstruction_pass",
             "mode_algebra_pass","rigid_crosscheck_pass","evaluation_pass",
             "linear_algebra_pass","conjugacy_pass","covariance_pass"):
    assert s[gate]==1
assert s["root_pass"]==0 and s["accepted_steps"]==8
assert abs(s["rigid_null_norm"]-4.6345148020027714e-4)<=1e-12
assert s["final_null_norm"]<s["rigid_null_norm"]/10
assert s["final_residual"]>1e-10
assert 0.04999<s["maximum_displacement"]<=0.05
assert s["maximum_speed"]<1/(3**0.5) and s["edge_deformation"]<=0.10
assert s["center_residual"]<=1e-14 and s["continuity_residual"]<=1e-12
with ITER.open(newline="") as f:iterations=list(csv.DictReader(f))
assert len(iterations)==8 and all(float(r["accepted_scale"])>0 for r in iterations)
with STATE.open(newline="") as f:states=list(csv.DictReader(f))
assert len(states)==16
print("FTD-0712 bounded internal-gait cancellation certificate: PASS")
print(f"null={s['rigid_null_norm']:.6e}->{s['final_null_norm']:.6e} bound={s['maximum_displacement']:.6e}")

