"""Registered exact constituent observations; never assign particle identities."""
from __future__ import annotations
from collections import Counter
from dataclasses import asdict, dataclass
from functools import lru_cache
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time
import types
import numpy as np
from . import channels as C, geometry as G, native_codec as N, staged as P, state as S
from ._proofs import encode, readout, rotate, relation_tick, phase_index

L = 9
HORIZON = 64
DIRECTIONS = ((-1,0,0),(1,0,0),(0,-1,0),(0,1,0),(0,0,-1),(0,0,1))
PLACEMENTS = ((4,4,4),(8,8,8))


@dataclass(frozen=True)
class CarrierCase:
    case_id: str
    family: str
    orientation: int
    polarity: int
    phase: int
    slot: int
    placement: int
    layer: int = 0
    second_polarity: int = 1


def cases() -> tuple[CarrierCase, ...]:
    result = []
    def add(family, orientation, polarity, phase, slot, placement, layer=0, second_polarity=1):
        result.append(CarrierCase(f"carrier_{len(result):04d}",family,orientation,polarity,
                                  phase,slot,placement,layer,second_polarity))
    for o in range(9):
        for p in (1,-1):
            for phase in range(4):
                for slot in range(2):
                    for pos in range(2): add("relation",o,p,phase,slot,pos)
    for o in range(6):
        for p in (1,-1):
            for phase in range(4):
                for pos in range(2): add("field",o,p,phase,0,pos)
    for family in ("encounter","separated"):
        for o in range(6):
            for p,p2 in ((1,1),(-1,-1),(1,-1)):
                for layer in range(3):
                    for pos in range(2): add(family,o,p,0,0,pos,layer,p2)
    return tuple(result)


def _channel(direction, phase, polarity):
    return next(c for c in range(384) if C.tangent(c)==direction and C.phase(c)==phase
                and C.polarity(c)==polarity)


def prepare(case: CarrierCase) -> P.StagedState:
    if case not in cases(): raise ValueError("unregistered carrier case")
    st = S.blank(L)
    st.ell[:] = case.layer
    owner = G.site_index(L,*PLACEMENTS[case.placement])
    if case.family == "relation":
        token = S.idx_of(encode(case.phase,case.polarity))
        if case.orientation < 3: st.sc[owner,case.orientation,case.slot] = token
        else:
            p,q = divmod(case.orientation-3,2)
            st.fcc[owner,p,q,case.slot] = token
    else:
        d = DIRECTIONS[case.orientation]
        st.bank[owner,_channel(d,case.phase,case.polarity)] = True
        if case.family in ("encounter","separated"):
            second = owner
            if case.family == "separated":
                axis = next(i for i,value in enumerate(d) if value)
                offset = tuple(4 if i==axis else 0 for i in range(3))
                second = G.shift(L,owner,offset)
            st.bank[second,_channel(d,1,case.second_polarity)] = True
    return P.initialize(st)


def preregistration() -> dict:
    return {"schema":"strict-carrier-prereg-1","law_id":P.LAW_ID,
            "collision_sha256":C.COLLISION_HASH,"encoding_sha256":N.HASHES[1].hex(),
            "L":L,"horizon_microticks":HORIZON,"boundary":"periodic",
            "case_count":len(cases()),"families":dict(Counter(c.family for c in cases())),
            "directions":DIRECTIONS,"placements":PLACEMENTS,"separation_axis_hops":4,
            "field_flag_selection":"first canonical channel at prescribed tangent/phase/polarity",
            "pass_criteria":"exact token/polarity/exchange/phase mapping; no carrier identity through pair collision",
            "M1_recovery_certified":False,"M2_recovery_certified":False,
            "cases":[asdict(c) for c in cases()]}


def _json(value): return json.dumps(value,sort_keys=True,separators=(",",":"))
def _sha(data): return hashlib.sha256(data).hexdigest()


def observe(state: P.StagedState) -> dict:
    P.validate(state)
    st = state.lattice
    field = [[int(x),int(c),int(st.ell[x]),int(st.s[x])] for x,c in zip(*np.nonzero(st.bank))]
    relation = []
    for kind,arr,gate in (("sc",st.sc,state.gate_sc),("fcc",st.fcc,state.gate_fcc)):
        for index in zip(*np.nonzero(arr != S.BLANK_IDX)):
            owner = int(index[0]); slot = int(index[-1])
            ori = int(index[1]) if kind=="sc" else int(index[1])*2+int(index[2])
            relation.append([kind,owner,ori,slot,int(arr[index]),int(gate[index[:-1]]),
                             int(state.admitted_sc[owner,ori]) if kind=="sc" else 0])
    return {"microtick":str(state.microtick),"sha256":_sha(N.encode(state)),"L":int(st.L),
            "field":field,"relation":relation,"work":len(field)+len(relation)}


def _field(snapshot):
    return {(int(x),int(c)):(int(ell),int(s)) for x,c,ell,s in snapshot["field"]}


def _relations(snapshot):
    return {(str(k),int(o),int(i),int(slot)):(int(code),int(g),int(a))
            for k,o,i,slot,code,g,a in snapshot["relation"]}


def _polarities(snapshot):
    answer = Counter(C.polarity(int(c)) for _,c,_,_ in snapshot["field"])
    answer.update(int(readout(S.z_of(int(row[4])))[1]) for row in snapshot["relation"])
    return answer


@lru_cache(maxsize=1)
def _tables(): return C.load_collision_tables()


def _ends(size,key):
    kind,owner,ori=key
    return G.sc_endpoints(size,owner,ori) if kind=="sc" else G.fcc_endpoints(size,owner,ori//2,ori%2)


def _check_snapshot(snapshot):
    if type(snapshot["L"]) is not int or snapshot["L"]<3:raise ValueError("invalid snapshot dimensions")
    n=snapshot["L"]**3
    if type(snapshot["work"]) is not int or snapshot["work"]<0:raise ValueError("invalid snapshot count")
    for row in snapshot["field"]:
        if len(row)!=4 or any(type(v) is not int for v in row):raise ValueError("invalid field record")
        x,c,ell,s=row
        if not (0<=x<n and 0<=c<384 and 0<=ell<3 and -1<=s<=1):raise ValueError("field record outside alphabet")
    for row in snapshot["relation"]:
        if len(row)!=7 or row[0] not in ("sc","fcc") or any(type(v) is not int for v in row[1:]):
            raise ValueError("invalid relation record")
        kind,x,ori,slot,code,gate,flag=row
        if not (0<=x<n and 0<=ori<(3 if kind=="sc" else 6) and slot in (0,1)
                and 0<=code<9 and code!=S.BLANK_IDX and gate in (0,1) and flag in (0,1)):
            raise ValueError("relation record outside alphabet")
        if kind=="fcc" and flag:raise ValueError("FCC has no admission control")


def audit_transition(before: dict, after: dict, events: dict) -> dict:
    """Check exact constituent exchanges and return unambiguous field arcs.

    Collision vertices explicitly terminate incoming identities. The caller may
    start new analysis segments at outputs, without assigning individual parents.
    """
    _check_snapshot(before);_check_snapshot(after)
    events=json.loads(json.dumps(events))  # accept dataclass tuple rows or JSON lists
    tick = int(before["microtick"])
    if int(after["microtick"]) != tick+1 or before["L"] != after["L"]:
        raise ValueError("noncontiguous carrier trace")
    old,new = _field(before),_field(after)
    ro,rn = _relations(before),_relations(after)
    if len(old)!=len(before["field"]) or len(new)!=len(after["field"]) or len(ro)!=len(before["relation"]) or len(rn)!=len(after["relation"]):
        raise ValueError("duplicate constituent record")
    if before["work"]!=len(old)+len(ro) or after["work"]!=len(new)+len(rn) or before["work"]!=after["work"]:
        raise ValueError("token count mismatch")
    if _polarities(before)!=_polarities(after): raise ValueError("polarity exchange mismatch")
    counts_old=Counter(key[:3] for key in ro);counts_new=Counter(key[:3] for key in rn)
    additions={key:counts_new[key]-counts_old[key] for key in counts_new.keys()|counts_old.keys()}
    admissions=events["absorptions"]
    expected_additions=Counter(("sc",int(owner),int(axis)) for _,_,owner,axis in admissions)
    if any(value != expected_additions[key] for key,value in additions.items()) or any(key not in additions for key in expected_additions):
        raise ValueError("relation-anchor migration or unlogged exchange")
    if len(new)-len(old)!=-len(admissions): raise ValueError("field/relation budget mismatch")
    phase=tick%4
    if admissions and phase!=0: raise ValueError("admission outside registered stage")
    if events["collisions"] and phase!=1: raise ValueError("collision outside registered stage")
    if (events["crossings"] or events["gate_holds"]) and phase!=1:
        raise ValueError("relation events outside registered stage")
    actual_crossings=[];actual_holds=[]
    for key in sorted(counts_old.keys()|counts_new.keys(),key=lambda k:(k[1],k[0]=="fcc",k[2])):
        oldpair=tuple(ro.get(key+(slot,),(S.BLANK_IDX,0,0))[0] for slot in range(2))
        newpair=tuple(rn.get(key+(slot,),(S.BLANK_IDX,0,0))[0] for slot in range(2))
        expected_pair=oldpair
        if phase==0 and expected_additions[key]:
            matches=[row for row in admissions if ("sc",int(row[2]),int(row[3]))==key]
            if len(matches)!=1:raise ValueError("multiple admissions to one relation")
            expected_pair=(S.BLANK_IDX,S.idx_of(rotate(encode(2,C.polarity(int(matches[0][1]))))))
        elif phase==1:
            controls=[value[1:] for slot in range(2) if (value:=ro.get(key+(slot,))) is not None]
            if any(c!=controls[0] for c in controls):raise ValueError("inconsistent relation controls")
            gate,admitted=controls[0]
            if not admitted:
                expected_pair=tuple(S.idx_of(z) for z in relation_tick(*(S.z_of(v) for v in oldpair),even_gate=bool(gate)))
                lo=oldpair[0]!=S.BLANK_IDX;rocc=oldpair[1]!=S.BLANK_IDX
                index=[key[2]] if key[0]=="sc" else [key[2]//2,key[2]%2]
                direction=1 if lo and expected_pair[0]==S.BLANK_IDX else (-1 if not lo and expected_pair[0]!=S.BLANK_IDX else 0)
                if direction:actual_crossings.append([key[0],key[1],index,direction])
                if lo!=rocc and phase_index(S.z_of(oldpair[0] if lo else oldpair[1]))==0 and not gate:
                    actual_holds.append([key[0],key[1],index])
        if newpair!=expected_pair:raise ValueError("relation payload transition mismatch")
    if phase==1 and (actual_crossings!=events["crossings"] or actual_holds!=events["gate_holds"]):
        raise ValueError("relation event log mismatch")
    expected=set(old); arcs=[]; vertices=[]; expired=[]
    if phase==0:
        proposals={}
        for x,c in sorted(old):
            if C.phase(c)==2:proposals.setdefault(G.sc_edge_of(int(before["L"]),x,C.tangent(c)),[]).append((x,c))
        expected_admissions=[[x,c,owner,axis] for (owner,axis),items in proposals.items()
                             if len(items)==1 and counts_old[("sc",owner,axis)]==0 for x,c in items]
        if admissions!=expected_admissions:raise ValueError("admission arbitration log mismatch")
        for x,c,owner,axis in admissions:
            key=(int(x),int(c))
            if key not in expected: raise ValueError("invalid absorption source")
            expected.remove(key);expired.append(key)
            d=C.tangent(int(c)); edge=G.sc_edge_of(int(before["L"]),int(x),d)
            if C.phase(int(c))!=2 or edge!=(int(owner),int(axis)) or counts_old[("sc",int(owner),int(axis))]:
                raise ValueError("invalid absorption carrier")
        arcs=[(key,key,(0,0,0)) for key in sorted(expected)]
    elif phase==1:
        consumed=set();produced=set()
        tables=_tables()
        groups={}
        for x,c in old:groups.setdefault((x,C.polarity(c)),[]).append(c%C.N_STATES)
        required={key:tuple(sorted(values)) for key,values in groups.items() if len(values)==2}
        logged=set()
        for x,pol,pair,outpair in events["collisions"]:
            offset=192 if pol<0 else 0
            inputs=tuple((int(x),int(c)+offset) for c in pair)
            outputs=tuple((int(x),int(c)+offset) for c in outpair)
            if len(set(inputs))!=2 or any(k not in old or k in consumed for k in inputs):
                raise ValueError("invalid collision constituents")
            if {k for k in old if k[0]==int(x) and C.polarity(k[1])==pol}!=set(inputs):
                raise ValueError("collision is not the exactly-two sector")
            if required.get((int(x),int(pol)))!=tuple(pair) or (int(x),int(pol)) in logged:
                raise ValueError("collision trigger mismatch")
            logged.add((int(x),int(pol)))
            layer=old[inputs[0]][0]
            if tuple(outpair)!=tables[layer][tuple(pair)]: raise ValueError("collision table mismatch")
            consumed.update(inputs);produced.update(outputs)
            vertices.append({"incoming":inputs,"outgoing":outputs,"identity_assignment":"ambiguous"})
        if logged!=set(required):raise ValueError("missing mandatory collision trigger")
        expected=(set(old)-consumed)|produced
        arcs=[(key,key,(0,0,0)) for key in sorted(set(old)-consumed)]
    elif phase==2:
        expected=set()
        for key,(ell,s) in old.items():
            x,c=key;d=C.tangent(c);target=C.U(c)
            if s:target=C.half_turn(target)
            dest=(G.shift(int(before["L"]),x,d),target)
            if dest in expected: raise ValueError("streaming write collision")
            expected.add(dest);arcs.append((key,dest,d))
    else: arcs=[(key,key,(0,0,0)) for key in sorted(old)]
    if expected!=set(new): raise ValueError("field transition does not match event/stage map")
    return {"arcs":arcs,"collision_vertices":vertices,"expired":expired,
            "absorptions":len(admissions),"relation_anchor_migrations":0}


def audit_trace(trace: dict) -> dict:
    states,logs=trace["states"],trace["events"]
    if len(states)!=HORIZON+1 or len(logs)!=HORIZON or int(states[0]["microtick"])!=0:
        raise ValueError("trace does not match registered horizon")
    case=next((case for case in cases() if case.case_id==trace["case_id"]),None)
    if case is None:raise ValueError("unknown trace case")
    for t,snapshot in enumerate(states):
        if int(snapshot["microtick"])!=t or snapshot["L"]!=L:raise ValueError("unregistered trace time/domain")
        digest=snapshot["sha256"]
        if len(digest)!=64 or any(c not in "0123456789abcdef" for c in digest):raise ValueError("invalid complete record digest")
        committed=states[t-t%4];incidence=Counter()
        if t>=4:
            for kind,owner,ori,slot,code,*_ in committed["relation"]:
                if slot==0:
                    tail,head=_ends(L,(kind,owner,ori));pol=readout(S.z_of(code))[1]
                    incidence[tail]+=pol;incidence[head]-=pol
        for x,c,ell,s in snapshot["field"]:
            if ell!=(case.layer-(t+2)//4)%3 or s!=((incidence[x]+1)%3-1 if t>=4 else 0):
                raise ValueError("field context differs from registered layer/readout history")
        populations=Counter(row[0] for row in committed["field"])
        admitted=set(tuple(row[2:]) for row in logs[t-t%4]["absorptions"]) if t%4 else set()
        for kind,owner,ori,slot,code,gate,flag in snapshot["relation"]:
            tail,head=_ends(L,(kind,owner,ori))
            expected_gate=int((populations[tail]+populations[head])%2==0) if t%4 else 0
            expected_flag=int(kind=="sc" and (owner,ori) in admitted)
            if (gate,flag)!=(expected_gate,expected_flag):raise ValueError("saved gate/admission context mismatch")
    segments=[];active={};serial=0;vertices=0;absorptions=0;hops=0
    residencies={}
    def resident(snapshot,tick):
        groups={}
        for kind,owner,ori,slot,code,*_ in snapshot["relation"]:
            groups.setdefault((kind,owner,ori),[0,0])[int(readout(S.z_of(code))[1]<0)]+=1
        for anchor,polarity in groups.items():
            if anchor not in residencies:
                residencies[anchor]={"anchor":anchor,"endpoint_sites":_ends(L,anchor),
                    "start_tick":tick,"polarity_inventory":polarity,"anchor_displacement":[0,0,0]}
            elif residencies[anchor]["polarity_inventory"]!=polarity:
                raise ValueError("resident anchor polarity changed")
    resident(states[0],0)
    def start(key,tick,reason):
        nonlocal serial
        active[key]={"analysis_segment":serial,"start_tick":tick,"origin":key,
                     "birth":reason,"hops":0,"unwrapped_displacement":[0,0,0]}
        serial+=1
    def stop(key,tick,reason):
        segment=active.pop(key)
        segment.update(stop_tick=tick,lifetime_microticks=tick-segment["start_tick"],end=reason)
        segments.append(segment)
    for key in sorted(_field(states[0])):start(key,0,"preparation")
    for before,after,events in zip(states,states[1:],logs):
        result=audit_transition(before,after,events);now=int(after["microtick"])
        resident(after,now)
        for key in result["expired"]:stop(key,now,"record_expiry")
        for vertex in result["collision_vertices"]:
            for key in vertex["incoming"]:stop(key,now,"ambiguous_pair_collision")
        updates={}
        for source,target,d in result["arcs"]:
            segment=active.pop(source)
            if any(d):
                segment["hops"]+=1;hops+=1
                segment["unwrapped_displacement"]=[a+b for a,b in zip(segment["unwrapped_displacement"],d)]
            updates[target]=segment
        if active:raise ValueError("unaccounted active field segments")
        active=updates
        for vertex in result["collision_vertices"]:
            for key in vertex["outgoing"]:start(key,now,"collision_output_no_individual_parent")
        vertices+=len(result["collision_vertices"]);absorptions+=result["absorptions"]
    for key in list(active):stop(key,HORIZON,"right_censored")
    for row in residencies.values():
        row.update(stop_tick=HORIZON,residency_microticks=HORIZON-row["start_tick"],
                   end="right_censored",identity="anchor inventory; no fabricated persistent slot identity")
    return {"case_id":trace["case_id"],"work":states[0]["work"],"physical_microticks":HORIZON,
            "field_hops":hops,"collision_vertices":vertices,"absorptions":absorptions,
            "initial_field_tokens":len(states[0]["field"]),"final_field_tokens":len(states[-1]["field"]),
            "final_relation_tokens":len(states[-1]["relation"]),"relation_anchor_migrations":0,
            "segments":segments,"relation_residencies":list(residencies.values()),
            "final_complete_sha256":states[-1]["sha256"],
            "M1_recovery_certified":False,"M2_recovery_certified":False}


def _linux(path):
    value=str(Path(path).resolve()).replace("\\","/")
    return "/mnt/"+value[0].lower()+value[2:] if len(value)>1 and value[1]==":" else value


def instrument_paths() -> set[str]:
    """Actual project-source closure, independent of unrelated caller imports."""
    root=Path(__file__).resolve().parents[2]
    paths={"engine/strict/recovery/recovery_carrier_main.cpp","engine/strict/recovery/CMakeLists.txt",
           "engine/strict/CMakeLists.txt","engine/strict/cuda.cmake",
           "engine/docs/PREREG_STRICT_CARRIER_RECOVERY.md","engine/strict/staged_runtime.cpp",
           "engine/strict/staged_cuda.cu","engine/strict/frozen_tables.h",
           "engine/strict/staged_runtime.h","engine/strict/staged_cuda.h"}
    # Explicit constant-only imports supplement object references: their Python
    # values do not retain the module that originally supplied those constants.
    seeds=(__name__,"phi_v2_lattice","phi_v2_lattice.channels","phi_v2_lattice.state","phi_v2_lattice._proofs",
           "phi_v2_lattice.geometry","phi_v2_lattice.staged","phi_v2_lattice.tick",
           "phi_v2_lattice.native_codec","phi_v2_lattice.checkpoint",
           "proof_c18_equivariant_single_record_collision_no_go")
    pending=[sys.modules[name] for name in seeds];seen=set()
    while pending:
        module=pending.pop()
        if module.__name__ in seen:continue
        seen.add(module.__name__)
        raw=getattr(module,"__file__",None)
        if not raw:continue
        path=Path(raw).resolve()
        if path.suffix!=".py" or not path.is_relative_to(root):continue
        paths.add(path.relative_to(root).as_posix())
        for value in tuple(vars(module).values()):
            if isinstance(value,types.ModuleType):pending.append(value)
            elif isinstance(value,(types.FunctionType,type)):
                parent=sys.modules.get(getattr(value,"__module__",""))
                if parent is not None:pending.append(parent)
    return paths


def prepare_campaign(directory) -> dict:
    directory=Path(directory);directory.mkdir(parents=True,exist_ok=True)
    if (directory/"lock.json").exists(): raise ValueError("campaign already locked; do not overwrite registration")
    root=Path(__file__).resolve().parents[2]
    runner=root/"engine/build_strict_recovery/ftd_strict_carrier_campaign"
    if not runner.is_file():raise ValueError("build the real CUDA carrier runner before locking")
    paths=instrument_paths()
    manifest=[];lines=[]
    for case in cases():
        path=directory/(case.case_id+".bin");blob=N.encode(prepare(case));path.write_bytes(blob)
        manifest.append({"case":asdict(case),"initial_sha256":_sha(blob)})
        lines.append(case.case_id+"\t"+_linux(path))
    registration=preregistration()
    lock={"registration":registration,"registration_sha256":_sha(_json(registration).encode()),
          "manifest":manifest,"manifest_sha256":_sha(_json(manifest).encode()),
          "instrument_sha256":{p:_sha((root/p).read_bytes()) for p in sorted(paths)},
          "runner_sha256":_sha(runner.read_bytes())}
    (directory/"lock.json").write_text(_json(lock)+"\n",encoding="utf-8")
    (directory/"manifest.tsv").write_text("\n".join(lines)+"\n",encoding="utf-8")
    return {"case_count":len(manifest),"manifest_sha256":lock["manifest_sha256"],
            "registration_sha256":lock["registration_sha256"],"microticks":len(manifest)*HORIZON}


def validate_lock(directory) -> dict:
    """Mandatory preflight/postflight; read-only and independent of trace data."""
    directory=Path(directory);lock=json.loads((directory/"lock.json").read_text())
    root=Path(__file__).resolve().parents[2]
    if set(lock["instrument_sha256"])!=instrument_paths():raise ValueError("incomplete instrument source closure")
    if any(_sha((root/p).read_bytes())!=h for p,h in lock["instrument_sha256"].items()):
        raise ValueError("instrument source changed after registration lock")
    if _sha((root/"engine/build_strict_recovery/ftd_strict_carrier_campaign").read_bytes())!=lock["runner_sha256"]:
        raise ValueError("CUDA runner changed after registration lock")
    if _sha(_json(lock["registration"]).encode())!=lock["registration_sha256"] or _json(lock["registration"])!=_json(preregistration()):
        raise ValueError("registration lock changed")
    if _sha(_json(lock["manifest"]).encode())!=lock["manifest_sha256"]: raise ValueError("manifest lock changed")
    if [row["case"] for row in lock["manifest"]]!=[asdict(case) for case in cases()]:
        raise ValueError("manifest case inventory differs from registration")
    expected_lines=[]
    for row in lock["manifest"]:
        path=directory/(row["case"]["case_id"]+".bin")
        if _sha(path.read_bytes())!=row["initial_sha256"]:raise ValueError("initial preparation hash changed")
        expected_lines.append(row["case"]["case_id"]+"\t"+_linux(path))
    if (directory/"manifest.tsv").read_text(encoding="utf-8")!="\n".join(expected_lines)+"\n":
        raise ValueError("executable manifest paths/order differ from lock")
    return lock


def run_campaign(directory) -> dict:
    """Run the fixed GPU instrument only after complete frozen-input preflight."""
    directory=Path(directory).resolve();lock=validate_lock(directory)
    root=Path(__file__).resolve().parents[2]
    runner=root/"engine/build_strict_recovery/ftd_strict_carrier_campaign"
    preflight={"lock_sha256":_sha((directory/"lock.json").read_bytes()),
               "registration_sha256":lock["registration_sha256"],"manifest_sha256":lock["manifest_sha256"],
               "runner_sha256":lock["runner_sha256"],"instrument_sha256":lock["instrument_sha256"],
               "case_count":len(lock["manifest"]),"validation":"complete preflight passed"}
    (directory/"preflight.json").write_text(_json(preflight)+"\n",encoding="utf-8")
    args=[runner,directory/"manifest.tsv",directory/"trace.jsonl"]
    command=(["wsl","-d","Ubuntu-22.04","--"]+[_linux(p) for p in args]
             if os.name=="nt" else [str(p) for p in args])
    started=time.perf_counter()
    result=subprocess.run(command+[str(HORIZON)],capture_output=True,text=True,timeout=1800)
    elapsed=time.perf_counter()-started
    (directory/"execution.stdout.txt").write_text(result.stdout,encoding="utf-8")
    (directory/"execution.stderr.txt").write_text(result.stderr,encoding="utf-8")
    if result.returncode:raise RuntimeError("GPU carrier runner failed; partial evidence retained: "+result.stderr)
    if validate_lock(directory)!=lock or _sha((directory/"lock.json").read_bytes())!=preflight["lock_sha256"]:
        raise ValueError("campaign inputs changed during GPU execution")
    devices=[json.loads(line) for line in result.stderr.splitlines() if line.startswith('{')]
    if len(devices)!=1 or devices[0].get("backend")!="cuda_device_kernels":raise ValueError("missing real GPU provenance")
    execution={"schema":"strict-carrier-execution-2","preflight_sha256":_sha((directory/"preflight.json").read_bytes()),
               "postflight":"complete frozen-source/input validation passed","device":devices[0],
               "elapsed_seconds":elapsed,"physical_microticks":len(cases())*HORIZON,
               "trace_sha256":_sha((directory/"trace.jsonl").read_bytes())}
    (directory/"execution.json").write_text(_json(execution)+"\n",encoding="utf-8")
    return execution


def summarize_campaign(directory) -> dict:
    directory=Path(directory);lock=validate_lock(directory)
    execution=json.loads((directory/"execution.json").read_text())
    preflight_bytes=(directory/"preflight.json").read_bytes();preflight=json.loads(preflight_bytes)
    if (execution["preflight_sha256"]!=_sha(preflight_bytes)
        or preflight["lock_sha256"]!=_sha((directory/"lock.json").read_bytes())):
        raise ValueError("execution/preflight receipt is not tied to current lock")
    for name in ("registration_sha256","manifest_sha256","runner_sha256","instrument_sha256"):
        if preflight[name]!=lock[name]:raise ValueError("preflight identity differs from accepted lock")
    if preflight["case_count"]!=len(lock["manifest"]) or execution["device"].get("backend")!="cuda_device_kernels":
        raise ValueError("execution capability or case inventory mismatch")
    if execution["postflight"]!="complete frozen-source/input validation passed" or execution["trace_sha256"]!=_sha((directory/"trace.jsonl").read_bytes()):
        raise ValueError("missing or mismatched executed campaign provenance")
    results=[]
    with (directory/"trace.jsonl").open(encoding="utf-8") as stream:
        for expected,line in zip(lock["manifest"],stream,strict=True):
            trace=json.loads(line);case=expected["case"];initial=(directory/(case["case_id"]+".bin")).read_bytes()
            if trace["case_id"]!=case["case_id"] or _sha(initial)!=expected["initial_sha256"]:
                raise ValueError("case identity or initial state mismatch")
            if trace["states"][0]!=observe(N.decode(initial)):raise ValueError("initial GPU observer mismatch")
            final=(directory/(case["case_id"]+".final.bin")).read_bytes()
            if trace["states"][-1]!=observe(N.decode(final)):raise ValueError("final GPU observer/hash mismatch")
            results.append(audit_trace(trace))
    output={"schema":"strict-carrier-report-2","instrument_revision":2,"law_id":P.LAW_ID,"boundary":"periodic",
            "backend":"cuda_device_kernels","registration_sha256":lock["registration_sha256"],
            "manifest_sha256":lock["manifest_sha256"],"trace_sha256":_sha((directory/"trace.jsonl").read_bytes()),
            "runner_sha256":lock["runner_sha256"],"instrument_sha256":lock["instrument_sha256"],
            "execution":execution,
            "cases":len(results),"physical_microticks":len(results)*HORIZON,
            "absorptions":sum(r["absorptions"] for r in results),
            "field_hops":sum(r["field_hops"] for r in results),
            "collision_vertices":sum(r["collision_vertices"] for r in results),
            "relation_anchor_migrations":sum(r["relation_anchor_migrations"] for r in results),
            "M1_recovery_certified":False,"M2_recovery_certified":False,"results":results}
    (directory/"report.json").write_text(_json(output)+"\n",encoding="utf-8")
    return output
