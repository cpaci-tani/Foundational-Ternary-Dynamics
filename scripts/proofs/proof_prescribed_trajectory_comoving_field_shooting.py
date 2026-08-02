"""Independent certificate for FTD-0710 prescribed co-moving field shooting."""
from __future__ import annotations
import csv, hashlib, json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
R=ROOT/"engine/results/ftd_0710"
SUMMARY=R/"ftd_0710_prescribed_trajectory_comoving_field_shooting_v1.json"
GMRES=R/"ftd_0710_prescribed_trajectory_comoving_field_gmres_v1.csv"
FIELD=R/"ftd_0710_prescribed_trajectory_comoving_field_rhs_v1.csv"
RUNNER=ROOT/"engine/tests/test_prescribed_trajectory_comoving_field_shooting.cpp"
PREREG=ROOT/"docs/theory/10_eft_program/preregistrations/PREREG_PRESCRIBED_TRAJECTORY_COMOVING_FIELD_SHOOTING_v1.md"
PROTOCOL="82E52438F5483C5C3A427B31D9B068314778B804C2320EEBFFCA1EA6EE593A4B"
HASHES={SUMMARY:"194AA2AA9AB989CDF2AFED59E71E6565555EB7B639EC8D814160C630E528122A",
GMRES:"102BE1B856771F55A95804FB4E2E624AAC91985319A04B935EAC182999C5A449",
FIELD:"76618236A4F6DB01B27666247245E689D68FBD2CA86A56E051D90DAE38C38A0D",
RUNNER:"88A971564428691FBED81AA5AD0A67CD035CBC827316957891C324BA6E368F8C",
PREREG:PROTOCOL}
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest().upper()
for path,expected in HASHES.items():assert sha(path)==expected,path
s=json.loads(SUMMARY.read_text())
assert s["protocol_sha256"]==PROTOCOL
assert s["verdict"]=="PRESCRIBED_TRAJECTORY_FIELD_SHOOTING_NOT_RESOLVED"
assert s["production_changed"] is False and s["volume"]==33
assert s["field_dof"]==6*33**3 and s["gmres_iterations"]==480
assert s["current_pass"]==1 and s["algebra_pass"]==1
assert s["gmres_converged"]==0 and s["field_pass"]==0
assert s["final_field_l2_residual"]<s["initial_field_l2_residual"]
assert s["complete_field_residual"]>1e-9
assert s["gauss_before_residual"]<=1e-10
assert s["gauss_after_residual"]<=1e-10
assert s["harmonic_mean_residual"]<=1e-12
assert s["field_covariance_residual"]<=1e-9
assert s["reciprocal_attempted"]==0
for key in ("reciprocal_inverse_residual","complete_relative_orbit_residual"):
    assert s[key] is None
with GMRES.open(newline="") as f:rows=list(csv.DictReader(f))
values=[float(row["residual_l2"]) for row in rows]
assert values[0]==s["initial_field_l2_residual"]
assert values[-1]==s["final_field_l2_residual"]
assert len(values)>=481 and values[-1]<values[0]/80
with FIELD.open(newline="") as f:
    reader=csv.DictReader(f);count=sum(1 for _ in reader)
assert count==33**3
print("FTD-0710 prescribed co-moving field shooting certificate: PASS")
print(f"GMRES={values[0]:.6e}->{values[-1]:.6e} field_max={s['complete_field_residual']:.6e}")

