"""Precentral controls: no nonempty real central orbit is executed here."""
from dataclasses import replace
from fractions import Fraction as F
from itertools import product
import json
import os
from pathlib import Path
import subprocess
import sys
import time

import numpy as np
import pytest

from phi_v2_lattice import recovery_full_memory as M
from phi_v2_lattice.experiments import certify_full_memory as R


def test_full_geometry_and_channel_orbits():
    g = M.geometry()
    assert len(g["displacements"]) == 93
    assert sum(x["structural_zero"] for x in g["displacements"]) == 60
    assert [len(x["orbits"]) for x in g["representative_pair_orbits"]] == [14, 46, 70, 43, 46]
    for rep in g["representative_pair_orbits"]:
        members = [tuple(p) for row in rep["orbits"] for p in row["members"]]
        assert len(members) == len(set(members)) == 576
    expected = {(0,0,0):(24,18), (1,0,0):(12,8), (1,1,0):(8,6), (1,1,1):(9,6),
                (2,0,0):(6,5), (2,1,0):(3,2), (2,1,1):(2,2), (2,2,0):(1,1)}
    for row in g["displacements"]:
        assert (row["endpoint_bits"][0], row["shared_sites"]) == expected[tuple(row["type"])]


def test_complete_local_tables_match_frozen_independent_actual_count():
    path = R.ROOT/"engine/docs/evidence/strict-recovery-wave7-2026-09-08/local-tables-attempt-1/tables.json"
    assert R._sha(path) == "fd73a78da9cb9b423b03220f982ce3586caff4fcb422721ed0135a8719709197"
    actual = json.loads(path.read_bytes())
    assert M.verify_independent_tables(actual)
    own = M.analytic_local_data()
    assert len(own["joint_tables"]) == 324
    assert sum(len(t["counts"]) for t in own["joint_tables"]) == 2304
    own["eligible_masks"][0] = 0
    own["joint_tables"][0]["counts"][0] = -1
    assert M.analytic_local_data() != own


@pytest.mark.parametrize("d", M.REP_DISPLACEMENTS[1:])
def test_complete_noncentral_matrices_two_bases_and_conserved_modes(d):
    a = M._noncentral(d, "configuration")
    b = M._noncentral(d, "walsh")
    assert a == b
    M._validate_matrix(d, a)
    assert all(isinstance(x, F) for row in a for x in row)
    source, target = M.LABELS.index((1,1,0,0)), M.LABELS.index((1,-1,0,0))
    if d == (2,0,0):
        assert a[target][source] == F(139921773018153334181957509616241,
                                      44601490397061246283071436545296723011960832)


def test_same_SC_pair_is_retained_in_joint_table():
    data = M.analytic_local_data()
    for gi, group in enumerate(M.GROUPS):
        if len(group) != 2:
            continue
        row = data["joint_tables"][18*gi+gi]["counts"]
        assert sum(count*((-1)**(x.bit_count()+y.bit_count()))
                   for y,x in product(range(4),repeat=2) for count in (row[x|(y<<2)],)) == 1<<24


def test_pair_geometry_coverage_without_scientific_weights():
    ranges = M.registered_ranges()
    assert len(ranges) == 128 and ranges[0][0] == 0 and ranges[-1][1] == M.PAIR_ORBITS
    assert all(a[1] == b[0] for a,b in zip(ranges,ranges[1:]))
    assert sum(R._ordered(a,b) for a,b in ranges) == M.N_ELIGIBLE**2
    for i in range(M.N_ELIGIBLE//2):
        begin = i*(M.N_ELIGIBLE+1-i)
        end = begin+M.N_ELIGIBLE-2*i
        assert M.pair_at(begin) == (i,i)
        assert M.pair_at(end-1) == (i,M.N_ELIGIBLE-1-i)
        assert len(M.pair_members(*M.pair_at(begin))) == 2
        assert len(M.pair_members(*M.pair_at(end-1))) == 2
    for boundary,_ in ranges[1:]:
        for index in (boundary-1,boundary):
            i,j = M.pair_at(index)
            assert i <= j and i+j < M.N_ELIGIBLE
            assert len(M.pair_members(i,j)) in (2,4)


@pytest.mark.parametrize("bad", (True, -1, M.PAIR_ORBITS, 1.0, "0", None))
def test_bad_pair_indices(bad):
    with pytest.raises(ValueError):
        M.pair_at(bad)


def test_exact_conserved_normalization_and_two_streams():
    matrix = [[0]*24 for _ in range(24)]
    a,i = M.LABELS.index((1,1,0,0)), M.LABELS.index((0,1,1,0))
    matrix[a][i] = 1
    data = {(0,0,0): matrix}
    response = M.conserved_response(data)
    assert response["quadratic_matrices"]["0,1"][0][0] == "-1/24"
    result = M.four_cycle_conserved(data)
    assert set(result) == {(1,2,1)}
    assert result[(1,2,1)][0][0] == F(1,24)
    for bad in (0.1, True, np.float64(1)):
        matrix[a][i] = bad
        with pytest.raises(ValueError):
            M.conserved_response(data)
        with pytest.raises(ValueError):
            M.four_cycle_conserved(data)


def _row(start=0,stop=10,done=0,status="RUNNING"):
    return {"schema":"strict-full-memory-native-range-1","start":start,"stop":stop,
            "completed_stop":done,"checked":done-start,"ordered_pairs":R._ordered(start,done),
            "denominator_exponent":432,"numerators":["0"]*14,"status":status}


@pytest.mark.parametrize("field,value", (("checked",True),("checked",7),("completed_stop",11),
    ("ordered_pairs",3),("status","OK"),("denominator_exponent",431),("numerators",["0"]*13),
    ("start",False),("stop",10.0)))
def test_progress_rejects_corruption(field,value):
    row = _row()
    row[field] = value
    with pytest.raises(ValueError):
        R.validate_progress(row,0,10)


@pytest.mark.parametrize("bad", ("00","+1","-0","1.0","1 ","",True,str(1<<435)))
def test_progress_canonical_integer_numerators(bad):
    row = _row()
    row["numerators"][0] = bad
    with pytest.raises(ValueError):
        R.validate_progress(row,0,10)


def test_progress_requires_complete_single_terminal():
    with pytest.raises(ValueError):
        R.validate_progress(_row(done=9,status="PASS"),0,10)
    last = R.validate_progress(_row(done=10,status="PASS"),0,10)
    with pytest.raises(ValueError):
        R.validate_progress(last,0,10,last)
    with pytest.raises(ValueError):
        R.validate_progress(_row(done=2),0,10,_row(done=3))


@pytest.fixture(scope="module")
def built(tmp_path_factory):
    record = R.build(tmp_path_factory.mktemp("full_memory_native")/"build")
    assert record["status"] == "PASS"
    return Path(record["binary"])


def test_native_synthetic_wide_integer_arithmetic(built):
    cases = [(16,[1<<24]*18),(-16,[1<<24]*18),(-3,[(1<<32)-1]*12),
             (2,[(1<<32)-1]*12),(1,[(1<<32)-1]*12),(1,[0]),(-1,[1]),
             (1,[1]),(16,[(1<<24)-1]*18),(-7,[(1<<24)-1]*18)]
    text = "".join(f"{sign} {len(factors)} "+" ".join(map(str,factors))+"\n" for sign,factors in cases)
    run = subprocess.run([str(built),"--arithmetic"],input=text.encode(),capture_output=True,check=True,timeout=10)
    expected = []
    current = 0
    for sign,factors in cases:
        value = sign
        for factor in factors:
            value *= factor
        current += value
        expected.append(str(current))
    assert run.stdout.decode().splitlines() == expected


def test_native_synthetic_overflow_is_failure(built):
    run = subprocess.run([str(built),"--arithmetic"],input=("16 16 "+" ".join([str((1<<32)-1)]*16)).encode(),
                         capture_output=True,timeout=10)
    assert run.returncode != 0 and b"overflow" in run.stderr


def test_native_binary_stdin_is_byte_preserving(built):
    data = bytes(range(256))+b"\r\n\x1a\x00\r\n"
    run = subprocess.run([str(built),"--binary-check"],input=data,capture_output=True,check=True,timeout=10)
    assert run.stdout.decode().strip() == data.hex()


def test_native_empty_real_job_validates_without_evaluating_any_pair(built):
    run = subprocess.run([str(built),"--start","0","--stop","0","--seconds","10"],
                         input=M.central_job_bytes(),capture_output=True,check=True,timeout=10)
    previous = None
    for line in run.stdout.splitlines():
        previous = R.validate_progress(json.loads(line),0,0,previous)
    assert previous["status"] == "PASS" and previous["checked"] == previous["ordered_pairs"] == 0
    assert previous["numerators"] == ["0"]*14


@pytest.mark.parametrize("case", ("short","magic","eligible","padding"))
def test_native_rejects_malformed_job_before_pairs(built,case):
    job = bytearray(M.central_job_bytes())
    if case == "short": job.pop()
    elif case == "magic": job[0] ^= 1
    elif case == "eligible": job[132:136] = b"\0"*4
    else: job[132+4*M.N_ELIGIBLE+16+4*4] = 1
    run = subprocess.run([str(built),"--start","0","--stop","0","--seconds","10"],
                         input=bytes(job),capture_output=True,timeout=10)
    assert run.returncode != 0


def _sleep_tree(payload,barrier):
    barrier.wait()
    child = subprocess.Popen([sys.executable,"-c","import time; time.sleep(30)"],creationflags=0x08000000)
    Path(payload["marker"]).write_text(str(child.pid))
    time.sleep(30)


@pytest.mark.skipif(os.name != "nt",reason="registered Windows Job Object test")
def test_real_job_object_deadline_kills_coordinator_and_descendant(tmp_path):
    marker = tmp_path/"descendant.pid"
    start = time.monotonic()
    result = R.supervise(_sleep_tree,{"marker":str(marker)},3,R.MEMORY_LIMIT)
    assert result["actual_job_assignment"] and result["failure"] == "global monotonic deadline"
    assert time.monotonic()-start < 9
    assert marker.is_file()
    pid = int(marker.read_text())
    k = R.ctypes.WinDLL("kernel32",use_last_error=True)
    k.OpenProcess.argtypes = (R.W.DWORD,R.W.BOOL,R.W.DWORD)
    k.OpenProcess.restype = R.W.HANDLE
    handle = k.OpenProcess(0x1000,False,pid)
    if handle:
        k.GetExitCodeProcess.argtypes = (R.W.HANDLE,R.ctypes.POINTER(R.W.DWORD))
        k.CloseHandle.argtypes = (R.W.HANDLE,)
        code = R.W.DWORD()
        assert k.GetExitCodeProcess(handle,R.ctypes.byref(code)) and code.value != 259
        k.CloseHandle(handle)


def test_missing_receipts_retain_prefix_and_all_unstarted_ranges(tmp_path):
    pins = {"job_sha256":"a"*64,"binary_sha256":"b"*64}
    R._json(tmp_path/"ranges/00000000.progress.json",{"native":_row(stop=M.registered_ranges()[0][1],done=2)})
    R._missing_ranges(tmp_path,"synthetic stopped process",pins)
    paths = list((tmp_path/"ranges").glob("[0-9]*.json"))
    finals = [p for p in paths if ".progress." not in p.name]
    assert len(finals) == 128
    first = R._read(tmp_path/"ranges/00000000.json")
    assert first["status"] == "FAIL" and first["checked"] == 2
    R._missing_ranges(tmp_path,"another stop",pins)
    assert R._read(tmp_path/"ranges/00000000.json") == first


def test_no_acceptance_means_no_central_execution(tmp_path):
    output = tmp_path/"rejected"
    with pytest.raises((ValueError,FileNotFoundError)):
        R.run(tmp_path,tmp_path/"missing.exe","a"*64,tmp_path/"missing.json","b"*64,
              tmp_path/"missing-census.json","c"*64,output)
    row = R._read(output/"gate-rejection.json")
    assert row["central_pairs_evaluated"] == 0 and row["status"] == "FAIL"
    assert not (output/"lock.json").exists()


def _allocate_over_job_limit(payload,barrier):
    barrier.wait()
    Path(payload["marker"]).write_text("attempting oversized allocation")
    try:
        data = bytearray(1024<<20)
        Path(payload["marker"]).write_text("unexpected allocation "+str(len(data)))
    except MemoryError:
        Path(payload["marker"]).write_text("OS memory limit denied allocation")


@pytest.mark.skipif(os.name != "nt",reason="registered Windows Job Object test")
def test_real_job_object_enforces_allocation_ceiling(tmp_path):
    marker = tmp_path/"allocation.txt"
    limit = R._parent_memory()+(512<<20)
    result = R.supervise(_allocate_over_job_limit,{"marker":str(marker)},10,limit)
    assert result["actual_job_assignment"]
    assert marker.read_text() in ("attempting oversized allocation", "OS memory limit denied allocation")
    if result["exit_code"] == 0:
        assert marker.read_text() == "OS memory limit denied allocation"
    else:
        assert result["exit_code"] == 124 and result["failure"] == "aggregate coordinator/worker memory ceiling"
    # Windows can include a denied allocation in its peak commit counter. Such
    # a recorded excess must still reject the campaign, even if the child exits.
    if result["peak_aggregate_private_bytes"] > limit:
        assert result["failure"] == "aggregate coordinator/worker memory ceiling"
    assert not result["coordinator_alive_after_cleanup"]


@pytest.mark.parametrize("termination_also_fails", (False,True))
@pytest.mark.skipif(os.name != "nt",reason="registered Windows Job Object test")
def test_failed_job_assignment_cannot_leave_barrier_child(tmp_path,monkeypatch,termination_also_fails):
    def rejected(*args):
        raise OSError("synthetic assignment rejection")
    monkeypatch.setattr(R.WinJob,"assign",rejected)
    if termination_also_fails:
        monkeypatch.setattr(R.WinJob,"terminate",rejected)
    marker = tmp_path/"must-not-start.txt"
    result = R.supervise(_sleep_tree,{"marker":str(marker)},5,R.MEMORY_LIMIT)
    assert not result["actual_job_assignment"] and not result["coordinator_alive_after_cleanup"]
    assert result["exit_code"] is not None and "synthetic assignment rejection" in result["failure"]
    assert not marker.exists()


def test_control_timeout_retains_raw_output_expected_and_failure(tmp_path,built,monkeypatch):
    # Synthetic reference response: this test evaluates no eligible pair.
    monkeypatch.setattr(M,"central_reference_range",lambda job,start,stop: _row(start,stop,stop,"PASS"))
    first = _row(0,16)
    def timeout(command,**kwargs):
        kwargs["stdout"].write(M.canonical_bytes(first))
        kwargs["stderr"].write(b"synthetic diagnostic before timeout")
        raise subprocess.TimeoutExpired(command,1)
    monkeypatch.setattr(R.subprocess,"run",timeout)
    with pytest.raises(subprocess.TimeoutExpired):
        R._controls(b"synthetic",str(built),tmp_path,time.monotonic()+10)
    receipt = R._read(tmp_path/"controls/0000.json")
    assert receipt["status"] == "FAIL" and "TimeoutExpired" in receipt["error"]
    assert receipt["expected"]["checked"] == 16
    assert receipt["stdout_sha256"] == R._sha(tmp_path/"controls/0000.stdout")
    assert (tmp_path/"controls/0000.stderr").read_bytes() == b"synthetic diagnostic before timeout"
    R._missing_controls(tmp_path,"synthetic kill",{"job_sha256":"a"*64,"binary_sha256":"b"*64})
    assert len(list((tmp_path/"controls").glob("[0-9][0-9][0-9][0-9].json"))) == 255
    assert R._read(tmp_path/"controls/0000.json") == receipt


def test_missing_control_receipt_recovers_valid_prefix_before_truncated_tail(tmp_path):
    raw = M.canonical_bytes(_row(0,16,3))+b'{"unfinished":'
    R._write(tmp_path/"controls/0000.stdout",raw)
    R._missing_controls(tmp_path,"killed process",{"job_sha256":"a"*64,"binary_sha256":"b"*64})
    row = R._read(tmp_path/"controls/0000.json")
    assert row["status"] == "FAIL" and row["native"]["checked"] == 3 and row["truncated_or_invalid_tail"]
    assert (tmp_path/"controls/0000.stdout").read_bytes() == raw


@pytest.mark.parametrize("field", ("command","binary_sha256","source_sha256","compiler_sha256","status"))
def test_build_admission_rejects_stale_or_foreign_record(built,monkeypatch,field):
    original = R._read
    receipt = original(built.parent/"build.json")
    assert R._build_identity(built,R._sha(built))
    receipt[field] = [] if field == "command" else "wrong"
    monkeypatch.setattr(R,"_read",lambda path: receipt if Path(path) == built.parent/"build.json" else original(path))
    with pytest.raises(ValueError):
        R._build_identity(built,R._sha(built))


def test_every_locked_external_prerequisite_is_rechecked(tmp_path):
    labels = ("audit","contract","prepared-receipt","job","local-data","census-receipt","census-range","build","compiler")
    paths = {}
    for name in labels:
        path = tmp_path/name
        path.write_bytes(b"original")
        paths[str(path)] = R._sha(path)
    pins = {"source_sha256":{},"path_sha256":paths}
    assert R._identity_outcome(pins)["status"] == "PASS"
    for name in labels:
        (tmp_path/name).write_bytes(b"changed")
    result = R._identity_outcome(pins)
    assert result["status"] == "FAIL" and len(result["drift"]) == len(labels)


def test_outer_supervision_exception_keeps_complete_failure_inventory(tmp_path,monkeypatch):
    pins = {"source_sha256":{},"path_sha256":{},"job_sha256":"a"*64,"binary_sha256":"b"*64}
    monkeypatch.setattr(R,"accepted_inputs",lambda *args: pins)
    def fail(*args):
        raise OSError("synthetic supervisor startup failure")
    monkeypatch.setattr(R,"supervise",fail)
    output = tmp_path/"failed"
    result = R.run("unused","unused","unused","unused","unused","unused","unused",output)
    assert result["status"] == "FULL_K1_INCOMPLETE" and result["post_run_identity"]["status"] == "PASS"
    assert result["supervision"] is None and "synthetic supervisor startup failure" in result["error"]
    artifacts = result["artifact_sha256"]
    assert len([p for p in artifacts if p.startswith("controls/")]) == 255
    assert len([p for p in artifacts if p.startswith("ranges/")]) == 128
    assert result["source_sha256"] == result["path_sha256"] == {}
    assert artifacts["lock.json"] == result["lock_sha256"]


def test_admission_preserves_every_accepted_and_prerequisite_pin(tmp_path,built):
    prepared = tmp_path/"prepared"
    R.prepare(prepared)
    census = R.ROOT/"engine/docs/evidence/strict-recovery-wave7-2026-09-08/local-tables-attempt-1/report.json"
    audit = "engine/docs/AUDIT_STRICT_FULL_MEMORY_RETURN_PLAN_V1.md"
    pins = dict(R._source_pins(), **{"CLAUDE.md":R._sha(R.ROOT/"CLAUDE.md")})
    receipt = {"schema":"strict-full-memory-acceptance-1","law_id":M.LAW_ID,
               "verdict":"PASS_SCOPED_EXACT_EXECUTION","canonical_adoption":False,
               "binary_sha256":R._sha(built),"analytic_data_sha256":R._sha(prepared/"local-data.json"),
               "independent_census_sha256":R._sha(census),"gates":{key:"PASS" for key in R.REQUIRED_GATES},
               "source_sha256":pins,"audit_path":audit,"audit_sha256":R._sha(R.ROOT/audit)}
    # A synthetic receipt exercises admission only, never run()/central science.
    acceptance = tmp_path/"synthetic-acceptance.json"
    R._json(acceptance,receipt)
    accepted = R.accepted_inputs(prepared,built,R._sha(built),acceptance,R._sha(acceptance),census,R._sha(census))
    assert accepted["source_sha256"]["CLAUDE.md"] == pins["CLAUDE.md"]
    assert accepted["source_sha256"][audit] == receipt["audit_sha256"]
    assert R.CONTRACT in accepted["source_sha256"]
    required = [built,built.parent/"build.json",prepared/"prepare-receipt.json",prepared/"central-job.bin",
                prepared/"local-data.json",prepared/"geometry.json",prepared/"noncentral.json",acceptance,
                census,census.parent/"tables.json",census.parent/"lock.json"]
    assert all(str(path.resolve()) in accepted["path_sha256"] for path in required)
    assert len([path for path in accepted["path_sha256"] if "local-tables-attempt-1/ranges/" in path.replace("\\","/")]) == 64
    assert R._identity_outcome(accepted)["status"] == "PASS"
