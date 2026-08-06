"""Independent certificate for FTD-0707 reduced L=33 rest refinement."""
from __future__ import annotations
import csv, hashlib, json, math
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
R=ROOT/"engine/results/ftd_0707"
SUMMARY=R/"ftd_0707_l33_symmetry_rest_refinement_v1.json"
ITER=R/"ftd_0707_l33_symmetry_rest_refinement_iterations_v1.csv"
STATE=R/"ftd_0707_l33_symmetry_rest_refinement_state_v1.csv"
TICKS=R/"ftd_0707_l33_symmetry_rest_refinement_ticks_v1.csv"
RUNNER=ROOT/"engine/tests/test_l33_symmetry_rest_refinement.cpp"
PREREG=ROOT/"docs/theory/10_eft_program/preregistrations/PREREG_L33_SYMMETRY_REST_REFINEMENT_v1.md"
PROTOCOL="0E1C61DDE059B8693DB68438CA17E056B39146278804BB35349DEA6FB5827FB0"
HASHES={SUMMARY:"E6AFF7C65AB2A4086AEE17F3608911EED2F044CF89B7E9585ECB5F8AF67CE367",
ITER:"D0CBC4FC895A2C99EEBF653A4178FD3EE367A2B4A41B50D76B3158A09A7E36F0",
STATE:"9FFB9E247E5BEA4A3455947B3948C6EFFCB90EA6D0D8FD311E777B837B81C5A9",
TICKS:"D1EB457F96CCA9077453D09EFE5A8C5CDF3D37B26BB46BD1751E8A363EDF322B",
RUNNER:"2FAB51A8B6DF6A758507CF2985F8F6C70F567241939D5DFFDDC6364AB9930589",
PREREG:PROTOCOL}
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest().upper()
for path,expected in HASHES.items():assert sha(path)==expected,path
s=json.loads(SUMMARY.read_text());assert s["protocol_sha256"]==PROTOCOL
assert s["verdict"]=="L33_REST_REQUIRES_FULL_COORDINATE_REFINEMENT"
assert s["production_changed"] is False and s["volume"]==33
assert s["evaluations_pass"]==1 and s["optimization_pass"]==0
assert s["one_step_pass"]==0 and s["forward_pass"]==1
assert s["reverse_pass"]==1 and s["covariance_pass"]==1
assert s["accepted_steps"]==0 and s["final_gradient"]<=1e-9
assert s["minimum_eigenvalue"]>1e-6
assert s["maximum_impulse"]>1e-9 and s["one_step_state"]>1e-9
assert s["one_step_momentum"]<=1e-9
assert s["maximum_common"]<=1e-10 and s["maximum_energy"]<=1e-10
assert s["recovery"]<=1e-9 and s["covariance_residual"]<=1e-9
with ITER.open(newline="") as f: rows=list(csv.DictReader(f))
assert len(rows)==1 and float(rows[0]["gradient"])==0.0
with STATE.open(newline="") as f: states=list(csv.DictReader(f))
with TICKS.open(newline="") as f: ticks=list(csv.DictReader(f))
assert len(states)==16 and len(ticks)==8
print("FTD-0707 L=33 symmetry-rest certificate: PASS")
print(f"verdict={s['verdict']} impulse={s['maximum_impulse']:.6e}")

