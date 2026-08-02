"""Independent FTD-0664--0666 volume/return certificate."""

from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations"
RESULTS = ROOT / "engine/results"

P1 = PREREG / "PREREG_VOLUME_SCALED_INTERNAL_MODE_TRANSFER_v1.md"
P2 = PREREG / "PREREG_VOLUME_SCALED_INTERNAL_MODE_TRANSFER_v2.md"
P3 = PREREG / "PREREG_INTERNAL_MODE_RETURN_TIME_v1.md"
J1 = RESULTS / "ftd_0664/ftd_0664_volume_scaled_internal_mode_transfer_v1.json"
A1 = RESULTS / "ftd_0664/ftd_0664_volume_scaled_internal_mode_transfer_arms_v1.csv"
J2 = RESULTS / "ftd_0665/ftd_0665_volume_scaled_internal_mode_transfer_v2.json"
A2 = RESULTS / "ftd_0665/ftd_0665_volume_scaled_internal_mode_transfer_arms_v2.csv"
T2 = RESULTS / "ftd_0665/ftd_0665_volume_scaled_internal_mode_transfer_ticks_v2.csv"
J3 = RESULTS / "ftd_0666/ftd_0666_internal_mode_return_time_v1.json"

P1_SHA = "B6C7E2632884FA6CC98499D42EE6E4CE1AE790C9B6261E034278ABABB2FFB933"
P2_SHA = "E8E627DEE418186A96A951290B61396D5C3D18B40C0AF6B18A37B26289FFE9B8"
P3_SHA = "4AFD79B3207C16A37EBDF96197EFCDA64ADFD5410DB0825D6085280791D8FDEC"
J1_SHA = "EB6228CCE248DBF83822C87E957A35D057DA82311461CF192F24CF06E150A6A8"
J2_SHA = "3D9C7F4601C4932458F351A1DE412A6E6E849E2514691C2C21093944BEE9B5B2"
J3_SHA = "E89871BA5CE26D098AFB1063BD74084E6971D4E3426CCB4907009565AA9A0749"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def relaxed_json(path: Path) -> dict[str, object]:
    # V1/v2 preserve the raw C++ spelling `inf` for an undefined descriptive
    # return-CV field. It is not used as a physics gate.
    return json.loads(path.read_text().replace(": inf", ": null"))


for path, expected in ((P1, P1_SHA), (P2, P2_SHA), (P3, P3_SHA),
                       (J1, J1_SHA), (J2, J2_SHA), (J3, J3_SHA)):
    assert sha256(path) == expected

j1 = relaxed_json(J1)
j2 = relaxed_json(J2)
j3 = relaxed_json(J3)
assert j1["protocol_sha256"] == P1_SHA
assert j1["verdict"] == "VOLUME_SCALED_INTERNAL_TRANSFER_EXECUTION_INVALID"
assert j1["execution_pass"] == 0
assert j2["protocol_sha256"] == P2_SHA
assert j2["parent_json_sha256"] == J1_SHA
assert j2["verdict"] == "VOLUME_SCALED_PRE_RETURN_TRANSFER_V2_CONSTRUCTIVE"
assert all(j2[key] == 1 for key in (
    "execution_pass", "locality_pass", "emission_pass", "outward_pass"))
assert j3["protocol_sha256"] == P3_SHA
assert j3["parent_json_sha256"] == J2_SHA
assert j3["verdict"] == "INTERNAL_MODE_RETURN_TIME_MIXED"

with A1.open(newline="") as stream:
    arms1 = list(csv.DictReader(stream))
with A2.open(newline="") as stream:
    arms2 = list(csv.DictReader(stream))
with T2.open(newline="") as stream:
    ticks2 = list(csv.DictReader(stream))

assert len(arms1) == len(arms2) == 6
assert {(int(row["volume"]), int(row["sign"])) for row in arms2} == {
    (size, sign) for size in (17, 25, 33) for sign in (-1, 1)}
assert all(row["executed"] == "0" for row in arms1)
recoveries1 = [float(row["recovery"]) for row in arms1]
assert min(recoveries1) > 1e-10 and max(recoveries1) < 1e-8
assert all(float(row["max_energy_drift"]) <= 1e-10 for row in arms1)
assert all(float(row["max_common"]) <= 1e-10 for row in arms1)
assert all(float(row["max_decomposition"]) <= 1e-10 for row in arms1)

by_arm: dict[tuple[int, int], dict[int, dict[str, str]]] = defaultdict(dict)
for row in ticks2:
    by_arm[(int(row["volume"]), int(row["sign"]))][int(row["tick"])] = row
assert all(len(history) == 4 * key[0] + 1 for key, history in by_arm.items())
assert all(math.isclose(float(history[0]["doublet_ratio"]), 1.0,
                        rel_tol=0.0, abs_tol=1e-14)
           for history in by_arm.values())

locality_sum = 0.0
locality_count = 0
for sign in (-1, 1):
    base = by_arm[(17, sign)]
    for size in (25, 33):
        history = by_arm[(size, sign)]
        for tick in range(17):
            for channel in ("doublet_ratio", "dynamic_energy_ratio"):
                difference = float(base[tick][channel]) - float(history[tick][channel])
                locality_sum += difference * difference
                locality_count += 1
locality = math.sqrt(locality_sum / locality_count)
assert math.isclose(locality, float(j2["locality_rms"]), rel_tol=1e-12)
assert locality <= 0.05

for history in by_arm.values():
    tick4 = history[4]
    tick16 = history[16]
    assert float(tick16["dynamic_energy_ratio"]) > 0.0
    assert float(tick16["dynamic_norm_ratio"]) > 0.0
    assert (float(tick16["radius_second_moment"])
            - float(tick4["radius_second_moment"])) >= 4.0

return_ticks = {(int(row["volume"]), int(row["sign"])):
                int(row["return_tick"]) for row in arms2}
assert return_ticks[(25, -1)] == return_ticks[(25, 1)] == 76
assert return_ticks[(33, -1)] == return_ticks[(33, 1)] == 76
assert int(j3["return_tick_negative"]) == int(j3["return_tick_positive"]) == 73
assert float(j3["recovery_negative"]) <= 1e-8
assert float(j3["recovery_positive"]) <= 1e-8

# The registered FTD-0666 prediction was 74..78 and therefore failed. The
# combined observed threshold times nevertheless reject direct t_return ∝ L:
# t/L varies strongly while absolute return time varies by only three ticks.
times = {17: 73.0, 25: 76.0, 33: 76.0}
scaled = [times[size] / size for size in sorted(times)]
assert max(scaled) - min(scaled) > 1.0
assert max(times.values()) - min(times.values()) == 3.0

print("FTD-0664--0666 volume/return certificate: PASS "
      "(v1 INVALID; v2 CONSTRUCTIVE; return prediction MIXED)")
