import test from 'node:test';
import assert from 'node:assert/strict';
import {ObserverSession} from '../js/observer/session.js';
import {ObserverStorage} from '../js/observer/storage.js';
import {DEFAULT_SETTINGS} from '../js/observer/catalog.js';
import {reflectAuthorPatch} from '../js/observer/mirror-frame.js';
import {INTEGRATOR_VERSION,LEGACY_INTEGRATOR_VERSION} from '../js/observer/conventions.js';

test('legacy and canonical commands normalize once, reject conflicts and preserve stale guards',async()=>{
    const s=new ObserverSession({playing:false});await s.initialize();
    const e=s.state.entities.find(e=>e.shape==='clock');
    const result=await s.command({type:'update',id:e.id,patch:{properAcceleration:[.1,.2,.3]}});
    assert.equal(result.ok,true);assert.deepEqual(result.snapshot.entities.find(x=>x.id===e.id).coordinateForcePerMass,[.1,.2,.3]);
    assert.equal('properAcceleration' in s.state.entities.find(x=>x.id===e.id),false);
    const before=s.snapshot();
    for(const patch of [{properAcceleration:[1,0,0],coordinateForcePerMass:[2,0,0]}, {coordinateForcePerMass:[1,2]}, {coordinateForcePerMass:[NaN,0,0]}, {properAcceleration:null}]) {
        assert.equal((await s.command({type:'update',id:e.id,patch})).ok,false);assert.deepEqual(s.snapshot(),before);
    }
    assert.equal((await s.command({type:'update',id:e.id,expectedEpoch:0,patch:{coordinateForcePerMass:[1,0,0]}})).ok,false);
    const both=await s.command({type:'update',id:e.id,patch:{properAcceleration:[.1,0,0],coordinateForcePerMass:[.1,0,0]}});assert.equal(both.ok,true);
    const extended=s.state.entities.find(e=>e.shape==='sphere');
    assert.equal((await s.command({type:'update',id:extended.id,patch:{coordinateForcePerMass:[1,0,0]}})).ok,false);
    const mirrored=reflectAuthorPatch({coordinateForcePerMass:[1,2,3]});assert.deepEqual(mirrored.coordinateForcePerMass,[1,-2,3]);
    assert.equal((await s.command({type:'update',id:e.id,patch:mirrored})).ok,true);
    await s.command({type:'undo'});assert.deepEqual(s.state.entities.find(x=>x.id===e.id).coordinateForcePerMass,[.1,0,0]);
    s.dispose();
});

test('portable storage migrates field names without inventing new historical trajectories',async()=>{
    const s=new ObserverSession({playing:false}),storage=new ObserverStorage();await s.initialize();
    const legacy=s.snapshot();delete legacy.integratorVersion;
    for(const e of legacy.entities){e.properAcceleration=e.coordinateForcePerMass;delete e.coordinateForcePerMass;}
    for(const segment of legacy.segments)delete segment.integratorVersion;
    const original=structuredClone(legacy);
    const text=storage.exportWorld(legacy,DEFAULT_SETTINGS),migrated=storage.importWorld(text).snapshot;
    assert.deepEqual(legacy,original);assert.equal(migrated.integratorVersion,LEGACY_INTEGRATOR_VERSION);
    for(let i=0;i<migrated.segments.length;i++) {
        assert.equal(migrated.segments[i].integratorVersion,LEGACY_INTEGRATOR_VERSION);
        const segment={...migrated.segments[i]};delete segment.integratorVersion;assert.deepEqual(segment,original.segments[i]);
    }
    assert.equal((await s.command({type:'load',snapshot:migrated})).ok,true);
    assert.equal((await s.command({type:'step'})).ok,true);assert.equal(s.state.integratorVersion,INTEGRATOR_VERSION);
    assert.ok(s.state.segments.some(x=>x.integratorVersion===LEGACY_INTEGRATOR_VERSION));
    assert.ok(s.state.segments.some(x=>x.integratorVersion===INTEGRATOR_VERSION));
    const conflicting=structuredClone(migrated);conflicting.entities[0].properAcceleration=[1,0,0];
    assert.throws(()=>storage.exportWorld(conflicting,DEFAULT_SETTINGS),/Conflicting/);
    s.dispose();storage.dispose();
});

test('acceleration, direction changes, cap and relocation expose their clock provenance',async()=>{
    const s=new ObserverSession({playing:false});await s.initialize();
    const e=s.addEntity({shape:'beacon',position:[0,0,0],coordinateForcePerMass:[.5,0,0]});
    for(let i=0;i<20;i++)s.step(1/120,{move:[1,0,0],speed:.99,acceleration:.5});
    const tau=s.state.observer.properTime,worldline=s.state.observer.worldline;
    await s.command({type:'update',id:e.id,patch:{coordinateForcePerMass:[0,.5,0]}});
    for(let i=0;i<20;i++)s.step(1/120,{move:[1,0,0],speed:.99,acceleration:.5});
    assert.ok(s.state.observer.properTime>tau);assert.equal(s.state.observer.worldline,worldline);
    await s.command({type:'update',id:e.id,patch:{coordinateForcePerMass:[10000,0,0]}});s.step(1/120,{});
    assert.equal(s.state.entities.find(x=>x.id===e.id).capApplied,true);
    assert.ok(s.state.segments.filter(x=>x.entityId===e.id).every(x=>Math.hypot(...x.velocity)<=.99+1e-12));
    await s.command({type:'dolly',displacement:[1,2,3]});
    assert.equal(s.state.observer.properTime,0);assert.equal(s.state.observer.worldline,worldline+1);
    assert.match(s.state.observer.worldlineReason,/relocation/);
    await s.command({type:'play'});assert.equal(s.state.observer.worldline,worldline+2);assert.match(s.state.observer.worldlineReason,/resume/);
    s.dispose();
});
