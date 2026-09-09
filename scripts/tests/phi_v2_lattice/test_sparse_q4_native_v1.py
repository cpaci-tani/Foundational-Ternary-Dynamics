"""Exact native state/event controls, never the Q4 interaction campaign.

The frozen topology generators are input fixtures only. Every expected
successor and event bundle is computed by the independent Python law.
"""
from dataclasses import asdict, replace
import hashlib
import importlib.util
from itertools import product
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import uuid

import numpy as np
import pytest

from phi_v2_lattice import sparse_credit_exchange_q4 as S
from phi_v2_lattice import credit_exchange_binding as D

ROOT = Path(__file__).resolve().parents[3]
NATIVE = ROOT/"scripts/phi_v2_lattice/native"
SOURCES = (NATIVE/"sparse_q4_kernel_v1.hpp", NATIVE/"sparse_q4_kernel_v1.cpp",
           NATIVE/"sparse_q4_kernel_probe_v1.cpp")
SPEC = ROOT/"engine/docs/SPEC_STRICT_SPARSE_Q4_NATIVE_KERNEL_V1.md"
SPEC_SHA = "28eeab5dd059d5b01aaac76955d8b53e1836dab9600a90d357e03fba8f81a167"
MAGIC = b"FTD-Q4-NATIVE-1\n"
IDENTITIES = b"".join(bytes.fromhex(x) for x in (S.RULE_HASH, S.FRAME_HASH, S.DENSE_ENCODING))
EVENT_NAMES = ("moves", "redirects", "capacity_holds", "attempt_marks", "attempt_expiries", "onsite_alignments", "credit_exchanges")
WIDTHS = (9, 5, 3, 3, 2, 5, 13)
REASONS = ("accepted", "capacity", "flux_capacity", "credit_deficit", "credit_capacity")
FIXTURE_PATH = ROOT/"scripts/tests/phi_v2_lattice/test_sparse_credit_exchange_q4.py"
FROZEN = {
    SPEC: SPEC_SHA,
    FIXTURE_PATH: "9bd09eec5b3789af833340667dfd42fd40ae8db04c4b9c7c17def1e889173477",
    Path(S.__file__): "417272af0ff258b92f31da26ef51a920a75de5dc3bdb3adf00335354b8d30b42",
    Path(D.__file__): "9a317076455949c15b743e7a2576e2dcc9a0311c47825160bee188cc39f12489",
    ROOT/"engine/docs/AUDIT_STRICT_SPARSE_Q4_NATIVE_PLAN_V1.md": "28017ad85ad90836233f8f4643446383581b0fe33d821e46439ff24df4495ac1",
}
_EVIDENCE = None
_CALL = 0


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def fixtures():
    assert sha(FIXTURE_PATH) == FROZEN[FIXTURE_PATH]
    spec = importlib.util.spec_from_file_location("frozen_sparse_q4_fixture_inputs", FIXTURE_PATH)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


F = fixtures()


def _json_new(path, value):
    with Path(path).open("xb") as stream:
        stream.write(json.dumps(value, sort_keys=True, separators=(",", ":")).encode("ascii")+b"\n")
        stream.flush()
        os.fsync(stream.fileno())


def _bytes_new(path, data):
    with Path(path).open("xb") as stream:
        stream.write(data);stream.flush();os.fsync(stream.fileno())


def oracle_pins():
    paths=set(FROZEN)|set(SOURCES)|{Path(__file__).resolve()}
    for name,module in tuple(sys.modules.items()):
        source=getattr(module,"__file__",None)
        if name.startswith("phi_v2_lattice") and source and source.endswith(".py"):
            paths.add(Path(source).resolve())
    return {str(path):sha(path) for path in sorted(paths)}


def build_probe(directory, extra_source=None):
    """Durable standalone build, with actual compiler and include identities."""
    directory = Path(directory).resolve()
    directory.mkdir(parents=True, exist_ok=False)
    compiler = Path(shutil.which("g++") or "").resolve()
    if not compiler.is_file():
        raise RuntimeError("g++ C++17 compiler unavailable")
    source = extra_source or SOURCES[2]
    binary = directory/("api-control.exe" if extra_source else "sparse-q4-probe.exe")
    command = [str(compiler), "-std=c++17", "-O3", "-Wall", "-Wextra", "-Wpedantic", "-H",
               "-I", str(NATIVE), str(SOURCES[1]), str(source), "-o", str(binary)]
    env = {k:v for k,v in os.environ.items() if k not in
           ("CPATH", "CPLUS_INCLUDE_PATH", "C_INCLUDE_PATH", "COMPILER_PATH", "LIBRARY_PATH", "GCC_EXEC_PREFIX")}
    before = {str(p):sha(p) for p in (*SOURCES, SPEC, Path(__file__), source)}
    record = {"schema":"strict-sparse-q4-native-build-1", "command":command, "cwd":str(ROOT),
              "compiler":str(compiler), "compiler_sha256":sha(compiler), "source_sha256":before,
              "compiler_version":subprocess.run([str(compiler), "--version"], capture_output=True, check=True).stdout.decode(),
              "binary":str(binary), "canonical_adoption":False,
              "cleared_build_overrides":["CPATH", "CPLUS_INCLUDE_PATH", "C_INCLUDE_PATH", "COMPILER_PATH", "LIBRARY_PATH", "GCC_EXEC_PREFIX"]}
    _json_new(directory/"build-lock.json", record)
    with (directory/"stdout").open("xb") as out, (directory/"stderr").open("xb") as err:
        try:
            proc = subprocess.run(command, cwd=ROOT, env=env, stdout=out, stderr=err, timeout=120)
            record["exit_code"] = proc.returncode
        except BaseException as error:
            record["failure"] = repr(error)
            record["exit_code"] = None
        out.flush(); err.flush(); os.fsync(out.fileno()); os.fsync(err.fileno())
    includes = {}
    for line in (directory/"stderr").read_text(errors="replace").splitlines():
        match = re.match(r"^\.+ (.+)$", line)
        if match:
            p = Path(match[1]).resolve()
            if not p.is_file():
                raise AssertionError("unresolved actual include: "+str(p))
            includes[str(p)] = sha(p)
    runtime = {}
    for name in ("libstdc++-6.dll", "libgcc_s_seh-1.dll", "libwinpthread-1.dll"):
        p = compiler.parent/name
        if p.is_file():
            runtime[str(p)] = sha(p)
    record.update(include_sha256=includes, compiler_runtime_sha256=runtime,
                  post_source_sha256={p:sha(p) for p in before},
                  stdout_sha256=sha(directory/"stdout"), stderr_sha256=sha(directory/"stderr"),
                  binary_sha256=sha(binary) if binary.is_file() else None,
                  status="PASS" if record["exit_code"]==0 and before=={p:sha(p) for p in before} else "FAIL")
    _json_new(directory/"build.json", record)
    if record["status"]!="PASS":
        raise AssertionError("native build failed; retained "+str(directory))
    return binary


@pytest.fixture(scope="session")
def binary():
    global _EVIDENCE
    for path, digest in FROZEN.items():
        assert sha(path)==digest
    explicit = os.environ.get("FTD_Q4_NATIVE_PROBE")
    if explicit:
        path = Path(explicit).resolve()
        build = json.loads((path.parent/"build.json").read_text())
        assert build["status"]=="PASS" and build["binary"]==str(path) and sha(path)==build["binary_sha256"]
        for source, digest in {**build["source_sha256"], **build["include_sha256"], **build["compiler_runtime_sha256"]}.items():
            assert sha(source)==digest, source
        assert sha(build["compiler"])==build["compiler_sha256"]
    else:
        path = build_probe(ROOT/"engine/docs/evidence/strict-recovery-wave8-2026-09-08"/("native-q4-build-"+uuid.uuid4().hex))
    _EVIDENCE=ROOT/"engine/docs/evidence/strict-recovery-wave8-2026-09-08"/("native-q4-parity-"+uuid.uuid4().hex)
    _EVIDENCE.mkdir(parents=True,exist_ok=False)
    pins = dict(oracle_pins(),**{str(path):sha(path)})
    _json_new(_EVIDENCE/"lock.json",{"schema":"strict-sparse-q4-native-control-lock-1","source_and_binary_sha256":pins,
                "probe_build_receipt_sha256":sha(path.parent/"build.json"),"scientific_q4_campaign":False})
    yield path
    after={p:sha(p) for p in pins}
    _json_new(_EVIDENCE/"identities-after.json",{"status":"PASS" if after==pins else "FAIL","source_and_binary_sha256":after,
                                               "raw_probe_calls":_CALL})
    assert pins==after


def state_wire(state):
    S.validate(state)
    assert 4<=state.L<=64 and not state.L%2
    out = bytearray(MAGIC+IDENTITIES)
    out += struct.pack("<HBBB3x", state.L, state.origin_code, state.charge_frame, len(state.edges))
    for c in state.carriers:
        out += struct.pack("<IBBBB", c.site,c.slot,c.direction,c.credit,c.attempted)
    for e in state.edges:
        out += struct.pack("<IBb2x", e.owner,e.axis,e.q)
    out += bytes(8*(4-len(state.edges)))
    tick = format(state.microtick, "x").encode("ascii")
    out += struct.pack("<Q", len(tick))+tick
    assert len(out)==192+len(tick)
    return bytes(out)


def decode_state(data):
    assert data[:112]==MAGIC+IDENTITIES
    L,origin,eta,n = struct.unpack_from("<HBBB",data,112)
    assert data[117:120]==bytes(3)
    carriers = tuple(S.Carrier(*struct.unpack_from("<IBBBB",data,120+8*i)) for i in range(4))
    edges=[]
    for i in range(4):
        row=data[152+8*i:160+8*i]
        assert row[6:]==b"\0\0"
        if i<n: edges.append(S.Edge(*struct.unpack("<IBb2x",row)))
        else: assert row==bytes(8)
    h=struct.unpack_from("<Q",data,184)[0]
    assert len(data)==192+h
    tick=data[192:].decode("ascii")
    state=S.SparseQ4State(L,int(tick,16),carriers,tuple(edges),origin,eta)
    assert state_wire(state)==data
    return state


def decode_events(data):
    assert len(data)>=8 and data[7]==0 and len(data)<=112
    out={};offset=8
    for name,width,count in zip(EVENT_NAMES,WIDTHS,data[:7]):
        rows=[]
        for _ in range(count):
            row=list(struct.unpack_from("<"+"i"*width,data,offset));offset+=4*width
            if name in ("redirects","attempt_marks"): row[-1]=REASONS[row[-1]]
            rows.append(tuple(row))
        out[name]=rows
    assert offset==len(data)
    return out


def requests(binary, raws, allow_stream_failure=False):
    global _CALL
    assert len(raws)<=4096
    data=struct.pack("<I",len(raws))+b"".join(struct.pack("<Q",len(x))+x for x in raws)
    proc=subprocess.run([str(binary),"--step-stream"],input=data,capture_output=True,timeout=30)
    if _EVIDENCE is not None:
        stem=f"{_CALL:06d}";_CALL+=1
        _bytes_new(_EVIDENCE/(stem+".input"),data)
        _bytes_new(_EVIDENCE/(stem+".stdout"),proc.stdout)
        _bytes_new(_EVIDENCE/(stem+".stderr"),proc.stderr)
        _json_new(_EVIDENCE/(stem+".json"),{"request_count":len(raws),"exit_code":proc.returncode,
                  "input_sha256":sha(_EVIDENCE/(stem+".input")),"stdout_sha256":sha(_EVIDENCE/(stem+".stdout")),
                  "stderr_sha256":sha(_EVIDENCE/(stem+".stderr")),"lock_sha256":sha(_EVIDENCE/"lock.json")})
    if not allow_stream_failure: assert proc.returncode==0,proc.stderr.decode(errors="replace")
    output=[];offset=0
    for _ in raws:
        status=proc.stdout[offset];offset+=1
        n=struct.unpack_from("<Q",proc.stdout,offset)[0];offset+=8
        first=proc.stdout[offset:offset+n];offset+=n
        if status:
            output.append((None,first.decode("ascii")));continue
        n=struct.unpack_from("<Q",proc.stdout,offset)[0];offset+=8
        second=proc.stdout[offset:offset+n];offset+=n
        output.append((decode_state(first),decode_events(second)))
    assert offset==len(proc.stdout)
    return output


def parity(binary, states, dense=True):
    raws=[state_wire(s) for s in states]
    actual=requests(binary,raws)
    for s,before,(a,ae) in zip(states,raws,actual):
        expected,ee=S.step(s)
        assert a==expected and ae==asdict(ee)
        assert state_wire(s)==before
        assert S.checkpoint(a)==S.checkpoint(expected)
        if dense:
            dense_expected,de=D.step(S.to_dense(s))
            assert a==S.from_dense(dense_expected) and ae==asdict(de)
    return actual


def test_identity_and_zero_request_binary_transport(binary):
    p=subprocess.run([str(binary),"--identity"],capture_output=True,check=True)
    identity=json.loads(p.stdout)
    assert identity["law_id"]==S.LAW_ID and identity["rule_hash"]==S.RULE_HASH
    assert identity["frame_hash"]==S.FRAME_HASH and identity["dense_encoding"]==S.DENSE_ENCODING
    assert identity["supported_even_L"]==[4,64] and not identity["canonical_adoption"]
    assert identity["payload_size"]<=80 and identity["events_size"]<=384
    assert requests(binary,[])==[]


@pytest.mark.parametrize("fixture",range(13))
@pytest.mark.parametrize("phase",range(19))
def test_all_frozen_topologies_all_backgrounds_phases(binary,fixture,phase):
    s=F.topology_fixtures()[fixture][1] if fixture<12 else F.winding_fixture()
    parity(binary,[replace(s,microtick=phase,origin_code=origin,charge_frame=eta)
                   for origin,eta in product(range(48),range(2))])


@pytest.mark.parametrize("attempts",tuple(product(range(2),repeat=4)))
@pytest.mark.parametrize("heading",range(1,7))
def test_all_attempt_patterns_and_headings(binary,attempts,heading):
    cs=tuple(S.Carrier(x,slot,heading,1,attempts[j]) for j,(x,slot) in
             enumerate(((0,0),(0,1),(36,0),(36,1))))
    base=S.initialize(6,cs,(),0,0)
    parity(binary,[replace(base,microtick=p,charge_frame=eta) for p,eta in product(range(19),range(2))])


@pytest.mark.parametrize("g",tuple(F.cubic_actions()))
def test_all768_labelled_spatial_and_same_clock_C_actions(binary,g):
    states=[];expected=[]
    for color,eta,p in product(range(8),range(2),range(19)):
        s=replace(F.topology_fixtures()[7][1],origin_code=color,charge_frame=eta,microtick=p)
        states.append(S.transform(s,g,(1,-1,1),True))
        expected.append(S.transform(S.step(s)[0],g,(1,-1,1),True))
    actual=parity(binary,states)
    assert [a for a,e in actual]==expected


@pytest.mark.parametrize("v",((1,0,0),(-1,0,0),(0,1,0),(0,-1,0),(0,0,1),(0,0,-1)))
def test_signed_unit_translations_every_background_and_phase(binary,v):
    identity=((1,0,0),(0,1,0),(0,0,1))
    states=[];expected=[]
    for origin,eta,p in product(range(48),range(2),range(19)):
        s=replace(F.topology_fixtures()[7][1],origin_code=origin,charge_frame=eta,microtick=p)
        states.append(S.transform(s,identity,v))
        expected.append(S.transform(S.step(s)[0],identity,v))
    assert [a for a,e in parity(binary,states)]==expected


@pytest.mark.parametrize("L",range(4,65,2))
def test_every_supported_even_size_seams(binary,L):
    base=F.winding_fixture() if L==4 else F.topology_fixtures(L)[7][1]
    s=S.transform(base,((1,0,0),(0,1,0),(0,0,1)),(L-1,L-1,L-1))
    parity(binary,[replace(s,microtick=p) for p in range(19)],dense=L in (4,6,8,64))


def test_every_event_family_and_all_denial_reasons(binary):
    # The addressed-hop sweep below clears attempts. Retain the independently
    # registered mixed-attempt reset fixtures as explicit expiry controls.
    states=[replace(base,microtick=0) for name,base in F.topology_fixtures()]
    for name,base in F.topology_fixtures():
        for heading,p,eta in product(range(1,7),range(19),range(2)):
            cs=tuple(replace(c,direction=heading,attempted=0) for c in base.carriers)
            states.append(replace(base,carriers=cs,microtick=p,charge_frame=eta))
    observed=set();reasons=set()
    for start in range(0,len(states),4096):
        for s,events in parity(binary,states[start:start+4096]):
            observed.update(name for name,rows in events.items() if rows)
            reasons.update(row[-1] for row in events["attempt_marks"])
    assert observed==set(EVENT_NAMES) and reasons==set(REASONS)


def test_exact_huge_ordinals_and38_step_owned_replay(binary):
    ticks=list(range(39))+[19*(1<<200)+p for p in range(19)]+[(1<<256)-1,1<<256,int("f"*4096,16)]
    base=F.topology_fixtures()[11][1]
    states=[replace(base,microtick=t) for t in ticks]
    limit=sys.get_int_max_str_digits()
    actual=parity(binary,states,dense=False)
    for s,(a,e) in zip(states,actual): assert a.microtick==s.microtick+1
    selected=[states[-1],states[-2],states[-3],states[39]]
    for _ in range(38): selected=[a for a,e in parity(binary,selected,dense=False)]
    assert sys.get_int_max_str_digits()==limit


@pytest.mark.parametrize("kind",("magic","rule","frame","encoding","header_reserved","edge_reserved",
    "Lsmall","Lodd","Lunsupported","origin","eta","edge_count","site","slot","direction","credit","attempt",
    "duplicate","order","zero_edge","q","gauss","account","population","inactive","tick_empty","tick_upper",
    "tick_zero","tick_sign","tick_NUL","length","truncated","trailing"))
def test_invalid_wire_or_state_is_rejected_before_successor(binary,kind):
    state=F.topology_fixtures()[1][1]
    raw=bytearray(state_wire(state))
    offsets={"magic":0,"rule":16,"frame":48,"encoding":80,"header_reserved":117,"edge_reserved":158}
    if kind in offsets: raw[offsets[kind]]^=1
    elif kind.startswith("L"):
        raw[112:114]=struct.pack("<H",{"Lsmall":2,"Lodd":5,"Lunsupported":66}[kind])
    elif kind in ("origin","eta","edge_count"):
        raw[{"origin":114,"eta":115,"edge_count":116}[kind]]={"origin":48,"eta":2,"edge_count":5}[kind]
    elif kind=="site":raw[120:124]=struct.pack("<I",6**3)
    elif kind in ("slot","direction","credit","attempt"):
        raw[{"slot":124,"direction":125,"credit":126,"attempt":127}[kind]]={"slot":2,"direction":0,"credit":2,"attempt":2}[kind]
    elif kind=="duplicate":raw[128:136]=raw[120:128]
    elif kind=="order":raw[120:128],raw[128:136]=raw[128:136],raw[120:128]
    elif kind in ("zero_edge","q","gauss"):raw[157]={"zero_edge":0,"q":2,"gauss":255}[kind]
    elif kind=="account":raw[126]^=1
    elif kind=="population":raw[148]^=1
    elif kind=="inactive":raw[160]=1
    elif kind.startswith("tick_"):
        tick={"tick_empty":b"","tick_upper":b"A","tick_zero":b"00","tick_sign":b"-1","tick_NUL":b"\0"}[kind]
        raw[184:]=struct.pack("<Q",len(tick))+tick
    elif kind=="length":raw[184:192]=struct.pack("<Q",(1<<64)-1)
    elif kind=="truncated":raw=raw[:-1]
    else:raw+=b"\0"
    bad,good=requests(binary,[bytes(raw),state_wire(state)])
    assert bad[0] is None and ("UNSUPPORTED_DOMAIN" if kind=="Lunsupported" else "INVALID_") in bad[1]
    assert good[0]==S.step(state)[0]


def test_binary_control_bytes_and_stream_failure_retain_prefix(binary):
    # Literal ctrl-Z and CR/LF in site words must traverse Windows binary stdin.
    cs=tuple(S.Carrier(x,slot,1,1,0) for x in (0x1a,0x0a0d) for slot in range(2))
    state=S.initialize(64,cs,(),0,0)
    raw=state_wire(state)
    assert b"\x1a" in raw and b"\r\n" in raw
    parity(binary,[state],dense=False)
    normal=struct.pack("<I",1)+struct.pack("<Q",len(raw))+raw
    for damaged in (normal+b"x",struct.pack("<I",4097),struct.pack("<IQ",1,(1<<20)+1),normal[:-1]):
        p=subprocess.run([str(binary),"--step-stream"],input=damaged,capture_output=True,timeout=30)
        assert p.returncode!=0 and b"FAILED_STREAM" in p.stderr
    p=subprocess.run([str(binary),"--step-stream"],input=normal+b"x",capture_output=True)
    assert p.stdout and p.stdout[0]==0  # retained valid prefix is not a successful whole stream


@pytest.mark.parametrize("phase",range(19))
def test_legal_distant_intervention_one_tick_Moore_support(binary,phase):
    L=8;site=S.site_index(L,(4,4,4))
    cs=tuple(S.Carrier(x,slot,1,1,0) for x in (0,site) for slot in range(2))
    base=replace(S.initialize(L,cs,(),47,1),microtick=phase)
    changed=replace(base,carriers=tuple(replace(c,direction=6,attempted=1) if c.site==site and c.slot==0 else c for c in base.carriers))
    (a,_),(b,_)=parity(binary,[base,changed])
    da,db=S.to_dense(a),S.to_dense(b)
    for x in range(L**3):
        coords=S.coordinates(L,x)
        if max(min((coords[j]-4)%L,(4-coords[j])%L) for j in range(3))>1:
            for name in D.NAMES: assert np.array_equal(getattr(da,name)[x],getattr(db,name)[x])


API_CONTROL = r'''
#include "sparse_q4_kernel_v1.hpp"
#include <iostream>
#include <stdexcept>
#include <utility>
namespace Q=ftd::q4native_v1;
void check(bool x) {if(!x) throw std::runtime_error("API correctness assertion");}
template<class F> void rejects(F f) {
    bool rejected=false;try {f();} catch(const Q::Error&) {rejected=true;}
    check(rejected);
}
Q::Payload base(unsigned L) {
    Q::Payload p;p.L=static_cast<std::uint16_t>(L);p.origin_code=47;p.eta=1;
    const unsigned n=L*L*L;
    p.carriers={{{0,0,1,1,0},{0,1,2,1,1},{n-1,0,5,1,1},{n-1,1,6,1,0}}};
    return p;
}
int main() {
    unsigned checks=0;
    for(unsigned L=4;L<=64;L+=2) {
        auto p=base(L);const auto s=Q::admit(p,"10000000000000000");
        const auto saved=Q::encode_state(s);
        p.carriers[0].direction=6;check(Q::payload(s).carriers[0].direction==1);++checks;
        auto copy=s;copy=Q::admit(p,"2");check(Q::encode_state(s)==saved);++checks;
        for(unsigned x:{0U,L*L*L-1U,L*L*L/2U}) for(unsigned a=0;a<3;++a) {
            check(Q::shift_site(s,Q::shift_site(s,x,a,1),a,-1)==x);++checks;
            const unsigned code=Q::background_code(s,x);
            // origin47 has the reversed xyz column permutation (2,1,0).
            const unsigned xyz[3]={x/(L*L),(x/L)%L,x%L};
            check(code==(47U^((xyz[2]&1)|((xyz[1]&1)<<1)|((xyz[0]&1)<<2))));++checks;
            for(unsigned b=0;b<2;++b) {
                auto e=Q::matching_edge(s,x,a,b);
                check(e.axis==2-a && Q::shift_site(s,e.owner,e.axis,1)==e.head);++checks;
                auto reverse=Q::matching_edge(s,x==e.owner?e.head:e.owner,a,b);
                check(e.owner==reverse.owner && e.head==reverse.head);++checks;
            }
        }
        rejects([&]{Q::shift_site(s,0,3,1);});rejects([&]{Q::shift_site(s,0,0,0);});
        rejects([&]{Q::shift_site(s,L*L*L,0,1);});rejects([&]{Q::background_code(s,L*L*L);});
        rejects([&]{Q::matching_edge(s,0,3,0);});rejects([&]{Q::matching_edge(s,0,0,2);});checks+=6;
    }
    const auto initial=Q::admit(base(6),"0");const auto saved=Q::encode_state(initial);
    auto bad=base(6);bad.carriers[0].credit=2;rejects([&]{Q::admit(bad,"0");});
    check(Q::encode_state(initial)==saved);checks+=2;
    for(const char*text:{"","00","A","+1","-1","0x1"," 1","1 "}) {
        rejects([&]{Q::admit(base(6),text);});++checks;
    }
    auto carried=Q::step(Q::admit(base(6),std::string(4096,'f'))).state;
    check(Q::microtick_hex(carried)=="1"+std::string(4096,'0'));++checks;
    rejects([&]{Q::decode_state(nullptr,1);});rejects([&]{Q::decode_events(nullptr,1);});checks+=2;
    Q::Events e;auto zero=Q::encode_events(e);check(zero==std::vector<std::uint8_t>(8));++checks;
    auto corrupt=zero;corrupt[7]=1;rejects([&]{Q::decode_events(corrupt.data(),corrupt.size());});++checks;
    corrupt=zero;corrupt.push_back(0);rejects([&]{Q::decode_events(corrupt.data(),corrupt.size());});++checks;
    for(unsigned i=0;i<7;++i) {
        corrupt=zero;corrupt[i]=255;rejects([&]{Q::decode_events(corrupt.data(),corrupt.size());});++checks;
    }
    e.attempt_marks.count=1;e.attempt_marks.rows[0]={1,0,Q::accepted};
    rejects([&]{Q::encode_events(e);});++checks;
    e={};e.redirects.count=1;e.redirects.rows[0]={1,0,1,2,Q::credit_deficit};
    e.attempt_marks.count=1;e.attempt_marks.rows[0]={1,0,Q::credit_deficit};
    auto wire=Q::encode_events(e);check(Q::encode_events(Q::decode_events(wire.data(),wire.size()))==wire);++checks;
    e.attempt_marks.rows[0][2]=Q::accepted;rejects([&]{Q::encode_events(e);});++checks;
    e.attempt_marks.rows[0][2]=Q::credit_deficit;e.attempt_expiries.count=1;e.attempt_expiries.rows[0]={1,0};
    rejects([&]{Q::encode_events(e);});++checks;
    std::cout<<"{\"status\":\"PASS\",\"checks\":"<<checks<<"}\n";
}
'''


def test_direct_cpp_api_ownership_helpers_event_rejection_and_huge_carry(binary):
    # Test source is retained next to its distinct build; no engine target.
    directory=ROOT/"engine/docs/evidence/strict-recovery-wave8-2026-09-08"/("native-q4-api-"+uuid.uuid4().hex)
    directory.mkdir(parents=True,exist_ok=False)
    source=directory/"api-control.cpp"
    with source.open("x",encoding="utf-8",newline="\n") as stream:stream.write(API_CONTROL)
    executable=build_probe(directory/"build",extra_source=source)
    p=subprocess.run([str(executable)],capture_output=True,timeout=30)
    (directory/"stdout").write_bytes(p.stdout);(directory/"stderr").write_bytes(p.stderr)
    _json_new(directory/"result.json",{"status":"PASS" if p.returncode==0 else "FAIL","exit_code":p.returncode,
              "source_sha256":sha(source),"binary_sha256":sha(executable),
              "stdout_sha256":sha(directory/"stdout"),"stderr_sha256":sha(directory/"stderr")})
    assert p.returncode==0,p.stderr.decode(errors="replace")
    assert json.loads(p.stdout)["checks"]>=1800
