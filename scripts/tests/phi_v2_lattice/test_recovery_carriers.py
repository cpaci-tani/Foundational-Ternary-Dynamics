"""Local instrument regressions. Registered horizons execute on actual CUDA."""
from collections import Counter
from copy import deepcopy
from dataclasses import asdict
import pytest
from phi_v2_lattice import channels as C, staged as P, recovery_carriers as R


def _case(family, **criteria):
    return next(c for c in R.cases() if c.family==family and all(getattr(c,k)==v for k,v in criteria.items()))


def test_registered_matrix_is_exact_and_has_no_random_parameters():
    cases=R.cases()
    assert len(cases)==len({c.case_id for c in cases})==600
    assert Counter(c.family for c in cases)=={"relation":288,"field":96,"encounter":108,"separated":108}
    assert R.preregistration()["horizon_microticks"]==64
    assert R.preregistration()["L"]==9
    assert R.preregistration()["M1_recovery_certified"] is False


def test_all384_channels_have_one_declared_step_and_finite_internal_permutations():
    assert len({C.U(c) for c in range(384)})==384
    assert len({C.half_turn(c) for c in range(384)})==384
    for c in range(384):
        assert sum(abs(v) for v in C.tangent(c))==1
        assert C.phase(C.U(c))==(C.phase(c)+1)%4
        assert C.polarity(C.U(c))==C.polarity(c)


def test_unique_admission_records_expiry_and_exact_exchange():
    state=R.prepare(_case("field",phase=2))
    after,events=P.step(state)
    result=R.audit_transition(R.observe(state),R.observe(after),asdict(events))
    assert len(result["expired"])==result["absorptions"]==1
    assert result["arcs"]==[]
    assert R.observe(after)["work"]==1


def test_collision_records_ambiguous_exchange_without_pairing_parents():
    state=R.prepare(_case("encounter",polarity=1,second_polarity=1))
    state,_=P.step(state)
    after,events=P.step(state)
    result=R.audit_transition(R.observe(state),R.observe(after),asdict(events))
    assert len(result["collision_vertices"])==1
    assert result["collision_vertices"][0]["identity_assignment"]=="ambiguous"
    assert result["arcs"]==[]


def test_missing_collision_and_forged_noop_cannot_evade_complete_trigger_check():
    state=R.prepare(_case("encounter",polarity=1,second_polarity=1));state,_=P.step(state)
    after,ev=P.step(state)
    before_snapshot=R.observe(state);bad=R.observe(after)
    bad["field"]=deepcopy(before_snapshot["field"])
    for row in bad["field"]:row[2]=(row[2]-1)%3
    events=asdict(ev);events["collisions"]=[]
    with pytest.raises(ValueError,match="mandatory collision"):
        R.audit_transition(before_snapshot,bad,events)


def test_relation_payload_corruption_rejected_even_when_counts_match():
    state=R.prepare(_case("relation",phase=0))
    state,_=P.step(state)
    after,events=P.step(state)
    bad=R.observe(after);bad["relation"][0][4]=R.observe(state)["relation"][0][4]
    with pytest.raises(ValueError,match="payload"):
        R.audit_transition(R.observe(state),bad,asdict(events))


def test_duplicate_and_missing_events_rejected():
    state=R.prepare(_case("field",phase=2));after,ev=P.step(state)
    events=asdict(ev);events["absorptions"]*=2
    with pytest.raises(ValueError):R.audit_transition(R.observe(state),R.observe(after),events)
    events["absorptions"]=[]
    with pytest.raises(ValueError):R.audit_transition(R.observe(state),R.observe(after),events)


def test_segment_lifetimes_and_unwrapped_hops_in_short_instrument_fixture(monkeypatch):
    # Four local verification steps, not a substitute for the64-tick GPUcampaign.
    monkeypatch.setattr(R,"HORIZON",4)
    case=_case("field",phase=0,placement=1)
    state=R.prepare(case);states=[R.observe(state)];events=[]
    for _ in range(4):
        state,ev=P.step(state);states.append(R.observe(state));events.append(asdict(ev))
    # Roundtrip normalizes tuple event rows to the actual runner's JSON arrays.
    import json
    trace=json.loads(json.dumps({"case_id":case.case_id,"states":states,"events":events}))
    report=R.audit_trace(trace)
    assert report["field_hops"]==1
    assert report["segments"][0]["lifetime_microticks"]==4
    assert report["segments"][0]["end"]=="right_censored"
    assert report["segments"][0]["unwrapped_displacement"]==list(C.tangent(states[0]["field"][0][1]))
    bad=deepcopy(trace);bad["states"][-1]["field"][0][3]=1
    with pytest.raises(ValueError,match="context"):
        R.audit_trace(bad)


def test_relation_residency_is_recorded_without_slot_particle_identity(monkeypatch):
    monkeypatch.setattr(R,"HORIZON",4)
    case=_case("field",phase=2)
    state=R.prepare(case);states=[R.observe(state)];events=[]
    for _ in range(4):
        state,ev=P.step(state);states.append(R.observe(state));events.append(asdict(ev))
    import json
    report=R.audit_trace(json.loads(json.dumps({"case_id":case.case_id,"states":states,"events":events})))
    assert len(report["relation_residencies"])==1
    resident=report["relation_residencies"][0]
    assert resident["start_tick"]==1 and resident["residency_microticks"]==3
    assert resident["anchor_displacement"]==[0,0,0]
    assert "no fabricated" in resident["identity"]


@pytest.fixture
def miniature_lock(tmp_path,monkeypatch):
    """A one-case preflight unit fixture; no GPU campaign is executed here."""
    from pathlib import Path
    root=Path(R.__file__).resolve().parents[2]
    if not (root/"engine/build_strict_recovery/ftd_strict_carrier_campaign").is_file():
        pytest.skip("optional carrier runner required for preflight binary-hash fixture")
    one=R.cases()[:1]
    monkeypatch.setattr(R,"cases",lambda:one)
    R.prepare_campaign(tmp_path)
    return tmp_path


def test_lock_covers_transitive_python_and_native_sources(miniature_lock):
    lock=R.validate_lock(miniature_lock)
    paths=set(lock["instrument_sha256"])
    assert {"scripts/phi_v2_lattice/geometry.py","scripts/phi_v2_lattice/state.py",
            "scripts/phi_v2_lattice/channels.py","scripts/phi_v2_lattice/_proofs.py",
            "scripts/phi_v2_lattice/__init__.py",
            "scripts/proofs/proof_c18_equivariant_single_record_collision_no_go.py",
            "engine/strict/staged_runtime.h","engine/strict/staged_cuda.h"}<=paths
    assert any(p.startswith("scripts/proofs/") for p in paths)


@pytest.mark.parametrize("corruption",["source","missing_source","input","manifest","runner"])
def test_preflight_rejects_tampering_before_any_gpu_launch(miniature_lock,monkeypatch,corruption):
    import json
    directory=miniature_lock
    lock=json.loads((directory/"lock.json").read_text())
    if corruption=="source":lock["instrument_sha256"]["scripts/phi_v2_lattice/geometry.py"]="0"*64
    elif corruption=="missing_source":del lock["instrument_sha256"]["scripts/phi_v2_lattice/geometry.py"]
    elif corruption=="runner":lock["runner_sha256"]="0"*64
    elif corruption=="manifest":
        (directory/"manifest.tsv").write_text("unexpected\t/path\n")
    else:
        path=directory/(lock["manifest"][0]["case"]["case_id"]+".bin")
        data=bytearray(path.read_bytes());data[-1]^=1;path.write_bytes(data)
    (directory/"lock.json").write_text(json.dumps(lock))
    def forbidden(*args,**kwargs):raise AssertionError("GPU launched before rejecting bad lock")
    monkeypatch.setattr(R.subprocess,"run",forbidden)
    with pytest.raises(ValueError):R.run_campaign(directory)


@pytest.mark.parametrize("stale",["preflight_digest","lock_digest"])
def test_stale_execution_receipt_cannot_authorize_summary(miniature_lock,stale):
    import json,hashlib
    directory=miniature_lock
    preflight={"lock_sha256":hashlib.sha256((directory/"lock.json").read_bytes()).hexdigest()}
    if stale=="lock_digest":preflight["lock_sha256"]="0"*64
    blob=json.dumps(preflight).encode();(directory/"preflight.json").write_bytes(blob)
    execution={"preflight_sha256":hashlib.sha256(blob).hexdigest()}
    if stale=="preflight_digest":execution["preflight_sha256"]="0"*64
    (directory/"execution.json").write_text(json.dumps(execution))
    with pytest.raises(ValueError,match="receipt"):
        R.summarize_campaign(directory)
