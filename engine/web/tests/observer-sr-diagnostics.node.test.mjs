import test from 'node:test';
import assert from 'node:assert/strict';
import {ObserverSession} from '../js/observer/session.js';
import {traceObserverRay} from '../js/observer/optics.js';
import {observationDiagnostics} from '../js/observer/diagnostics.js';

test('event export retains negative prehistory clocks, revisions, image/source distinction and worldline provenance',()=>{
    const s=new ObserverSession({playing:false});s.state.entities=[];s.state.segments=[];
    s.addEntity({position:[0,3,-2],shape:'sphere',size:[1,1,1]},true);
    s.state.observer.position=[0,3,6];
    const snapshot=s.snapshot(),before=structuredClone(snapshot),settings={mirrorWorld:false};
    const hit=traceObserverRay(snapshot,settings),diagnostic=observationDiagnostics(snapshot,settings,hit);
    assert.ok(diagnostic.hit);assert.ok(diagnostic.hit.sourceProperTime<0);
    assert.equal(diagnostic.hit.sourceRevision,hit.revision);assert.equal(diagnostic.sessionId,snapshot.sessionId);
    assert.equal(diagnostic.observerWorldlineId,snapshot.observer.worldline);assert.deepEqual(snapshot,before);
    assert.deepEqual(s.snapshot(),before);
    snapshot.observer.pitch=-Math.atan2(6,8);
    const reflected=traceObserverRay(snapshot,{mirrorWorld:true});
    assert.ok(reflected?.mirrored);
    const d=observationDiagnostics(snapshot,{mirrorWorld:true},reflected);
    assert.equal(d.hit.apparentPosition[1],-d.hit.emissionPosition[1]);s.dispose();
});

test('known cutoff rejection differs from an ordinary empty ray and never substitutes a present pose',()=>{
    const s=new ObserverSession({playing:false});s.state.entities=[];s.state.segments=[];
    s.addEntity({position:[0,3,-2],shape:'sphere'},true);s.state.observer.position=[0,3,6];
    s.state.historyStart=-1;
    const snapshot=s.snapshot(),hit=traceObserverRay(snapshot,{mirrorWorld:false});assert.equal(hit,null);
    const cutoff=observationDiagnostics(snapshot,{mirrorWorld:false},hit);assert.equal(cutoff.historyStatus,'before-history-start');assert.equal(cutoff.hit,null);
    snapshot.observer.yaw=Math.PI;
    const empty=observationDiagnostics(snapshot,{mirrorWorld:false},null);assert.equal(empty.historyStatus,'no-hit');assert.equal(empty.hit,null);
    assert.deepEqual(empty.availableInterval,[-1,0]);s.dispose();
});
