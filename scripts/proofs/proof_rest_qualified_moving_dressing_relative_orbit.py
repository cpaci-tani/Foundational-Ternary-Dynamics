"""Independent certificate for FTD-0709 rest-qualified relative orbit."""
from __future__ import annotations
import csv, hashlib, json, math
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
R=ROOT/"engine/results/ftd_0709"
SUMMARY=R/"ftd_0709_rest_qualified_moving_dressing_relative_orbit_v1.json"
METRICS=R/"ftd_0709_rest_qualified_moving_dressing_relative_orbit_metrics_v1.csv"
RUNNER=ROOT/"engine/tests/test_rest_qualified_moving_dressing_relative_orbit.cpp"
PREREG=ROOT/"docs/theory/10_eft_program/preregistrations/PREREG_REST_QUALIFIED_MOVING_DRESSING_RELATIVE_ORBIT_v1.md"
PROTOCOL="14AE617CE7D5EA4F4617FAB667F34CFE339309512B2D9E2D1BE97C946D47A74E"
HASHES={SUMMARY:"86562D8A4ACF7E46CFECC1FF420B3B669294DBECCEB7FBE6040FDAF8CC00FF7C",
METRICS:"93AF5C6B0198F1D9FC4D71B76C5BE2A94832869111ED2FCC937190E11465F3F9",
RUNNER:"03387420AEDE189C1B4062477144D41DAF675CAC954A945B38F3C0B3CB5C7717",
PREREG:PROTOCOL}
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest().upper()
for path,expected in HASHES.items():assert sha(path)==expected,path
s=json.loads(SUMMARY.read_text());assert s["protocol_sha256"]==PROTOCOL
assert s["verdict"]=="REST_QUALIFIED_CORE_TRANSLATES_WITHOUT_COMPLETE_MOVING_DRESSING"
assert s["production_changed"] is False and s["volume"]==33 and s["ticks"]==2
for gate in ("parent_pass","reconstruction_pass","execution_pass","rest_pass",
             "inverse_pass","covariance_pass"):assert s[gate]==1
assert s["position_residual"]<=0.05 and s["momentum_residual"]<=0.05
assert s["electric_residual"]>1e-6 and s["magnetic_residual"]>1e-6
assert s["complete_residual"]==s["electric_residual"]
assert s["maximum_energy_drift"]<=1e-10
assert s["maximum_common_residual"]<=1e-10
assert s["inverse_residual"]<=1e-9 and s["rest_residual"]<=1e-9
assert s["covariance_residual"]<=1e-9
with METRICS.open(newline="") as f: rows=list(csv.DictReader(f))
assert len(rows)==1 and rows[0]["verdict"]==s["verdict"]
for key in ("position_residual","momentum_residual","electric_residual",
            "magnetic_residual","complete_residual","inverse_residual",
            "rest_residual","covariance_residual"):
    assert float(rows[0][key])==s[key]
print("FTD-0709 rest-qualified relative-orbit certificate: PASS")
print(f"position={s['position_residual']:.6e} momentum={s['momentum_residual']:.6e} "
      f"electric={s['electric_residual']:.6e} magnetic={s['magnetic_residual']:.6e}")

