import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
function load(path, expression, globals = {}) {
    const source = readFileSync(new URL('../js/' + path, import.meta.url), 'utf8')
        .replace(/^import[\s\S]*?from\s+['"][^'"]+['"];\s*/gm, '')
        .replace(/export\s+/g, '');
    return vm.runInNewContext(source + '\n' + expression, { Map, Set, ...globals });
}
function fixture(L = 3, native = true) {
    let time = 0;
    const bridge = { latticeSize: L, isNativeGPU: native, _visualEpoch: 1,
        _scenarioDataGeneration: 1, _voxelCache: new Map(), _forceAtCache: new Map(), requests: [],
        currentTick: () => 0,
        inspectVoxel(x,y,z) { const key = `${x},${y},${z}`; this.requests.push(key); return this._voxelCache.get(key) ?? null; },
        getForceAt(x,y,z) { return this._forceAtCache.get(`${x},${y},${z}`) ?? null; },
    };
    const document = { getElementById: () => null };
    const api = load('inspector/scales/lattice.js',
        '({normalizeLatticePosition,buildNeighbourOrder,refreshInspectionCache,sampleSummary,updateLatticeFields,handleLatticeClick})',
        { performance: { now: () => time }, document,
          formatPosition: (...xyz) => xyz.slice(0,3).join(','), formatVec3: () => 'vector',
          ...Object.fromEntries(['formatVelocity','formatForce','formatDensity','formatDivergence','formatField_E','formatField_B'].map(k => [k, v => ({text:String(v)})])) });
    const target = { bridge, _selectedPos: {x:0,y:0,z:0} };
    return {bridge, target, api, document, time: value => { time = value; },
        refresh: () => api.refreshInspectionCache(target,0,0,0)};
}
function fill(f, value = 1) {
    f.bridge._voxelCache.set('0,0,0', { state:value });
    for (const p of f.api.buildNeighbourOrder(0,0,0,f.bridge.latticeSize)) f.bridge._voxelCache.set(`${p.x},${p.y},${p.z}`, {state:value});
    f.bridge._forceAtCache.set('0,0,0',{coulombMag:value});
}
test('native completed replies are consumed before cadence and paused-epoch guards', () => {
    const f=fixture(); f.refresh(); fill(f); f.time(10);
    const complete=f.refresh(); assert.equal(complete.voxel.state,1); assert.equal(complete.neighbours.size,26);
    f.bridge._visualEpoch=2; f.time(800); f.refresh();
    const requestCount=f.bridge.requests.length;
    f.bridge._voxelCache.set('0,0,0',{state:-1}); f.bridge._forceAtCache.set('0,0,0',{coulombMag:7});
    f.time(810); const next=f.refresh();
    assert.equal(next.voxel.state,-1); assert.equal(next.force.coulombMag,7);
    assert.equal(f.bridge.requests.length,requestCount,'cache consumption must not request another batch');
});
test('source generation resets retained records immediately', () => {
    const f=fixture(); fill(f); const old=f.refresh();
    f.bridge._scenarioDataGeneration++; f.bridge._voxelCache.clear(); f.bridge._forceAtCache.clear();
    const next=f.refresh(); assert.notEqual(next,old); assert.equal(next.voxel,null); assert.equal(next.neighbours.size,0);
});
test('worker cache-only ingestion resolves all FIFO replies without request amplification', () => {
    const f=fixture(3,false); const metas=new Map(); f.bridge.configurationToken=4;
    f.bridge.getInspectionSample=(x,y,z)=> { const v=f.bridge._voxelCache.get(`${x},${y},${z}`); return v?{...v}:null; };
    f.bridge.getInspectionSampleMeta=(x,y,z)=>metas.get(`${x},${y},${z}`)??null;
    f.bridge.getForceSample=(x,y,z)=>f.bridge._forceAtCache.get(`${x},${y},${z}`)??null;
    f.refresh(); assert.equal(f.bridge.requests.length,27);
    fill(f); for(const key of f.bridge._voxelCache.keys()) metas.set(key,{sampleTick:8,configurationToken:4,dataVersion:2,stale:false});
    f.time(1); const cache=f.refresh(); assert.equal(cache.voxel.state,1); assert.equal(cache.neighbours.size,26);
    const revision=cache.revision; f.time(2); f.refresh(); assert.equal(cache.revision,revision,'equivalent defensive copies do not repaint');
    assert.equal(f.bridge.requests.length,27); assert.match(f.api.sampleSummary(cache),/sampled ticks 8..8/);
    assert.match(f.api.sampleSummary(cache),/force sample tick unavailable/);
});
test('actual mixed ticks and stale samples are visible, not inferred from display clock', () => {
    const f=fixture(); fill(f); const c=f.refresh();
    c.voxelMeta={sampleTick:10}; for(const key of c.neighbours.keys()) c.neighbourMeta.set(key,{sampleTick:7,stale:true});
    assert.match(f.api.sampleSummary(c),/sampled ticks 7..10/);
    assert.match(f.api.sampleSummary(c),/retained older samples/);
    assert.match(f.api.sampleSummary(c),/not one simultaneous neighborhood sample/);
});
test('L1 and L2 request only distinct noncenter sites and reach paused completeness', () => {
    for(const [L,count] of [[1,0],[2,7],[3,26]]) {
        const f=fixture(L); assert.equal(f.api.buildNeighbourOrder(0,0,0,L).length,count);
        fill(f); const c=f.refresh(); assert.equal(c.neighbours.size,count);
        const calls=f.bridge.requests.length; f.time(1000); f.refresh(); assert.equal(f.bridge.requests.length,calls);
        assert.ok(Number.isFinite(c.cursor));
    }
});
test('display owns response values and generation rollback discards them', () => {
    const f=fixture(); let tick=5; f.bridge.currentTick=()=>tick; fill(f);
    const c=f.refresh(); const response=f.bridge._voxelCache.get('0,0,0'); response.state=-1;
    assert.equal(c.voxel.state,1); f.time(1); f.refresh(); assert.equal(c.voxel.state,-1);
    tick=0; f.bridge._voxelCache.clear(); const reset=f.refresh(); assert.notEqual(reset,c); assert.equal(reset.voxel,null);
});
test('coordinate normalization rejects nonfinite and missing fields before requests', () => {
    const {api}=fixture();
    for(const p of [{x:NaN,y:1,z:2},{x:Infinity,y:1,z:2},{x:'',y:1,z:2},{x:null,y:1,z:2},{x:1,y:2},null]) assert.equal(api.normalizeLatticePosition(p,3),null);
    for(const L of [0,-1,Infinity,NaN,1.5]) assert.equal(api.normalizeLatticePosition({x:1,y:1,z:1},L),null);
    assert.equal(JSON.stringify(api.normalizeLatticePosition({x:-9,y:'1.6',z:99},3)),JSON.stringify({x:0,y:2,z:2}));
});
test('unsupported bridge remains unavailable rather than fabricating void', () => {
    const f=fixture(); delete f.bridge.inspectVoxel; delete f.bridge.getForceAt;
    f.target.fields=Object.fromEntries(['id','state','pos','spin','color','pair','locked','flux','density','divj','curl','vel','speed','accel','eMag','bMag','fCoulomb','fGravity','fMagnetic','fStrong','fExchange'].map(k=>[k,{textContent:''}]));
    f.api.updateLatticeFields(f.target); assert.equal(f.target.fields.state.textContent,'--');
});
test('public selection and bridge replacement share validation and own selected coordinates', () => {
    const f=fixture(); const Inspector=load('inspector.js','Inspector',{normalizeLatticePosition:f.api.normalizeLatticePosition});
    const target=Object.create(Inspector.prototype);
    Object.assign(target,{bridge:{latticeSize:9},_engineMode:'lattice',_showLatticeInspector(){this.shown=true},_hideLatticeInspector(){this.hidden=true}});
    assert.equal(target.selectLatticePosition({x:8,y:8,z:8}),true);
    const saved=target.getSelectedLatticePosition(); saved.x=-100; assert.equal(target._selectedPos.x,8);
    target.setBridge({latticeSize:3}); assert.equal(target._selectedPos.x,2);
    assert.equal(target.selectLatticePosition({x:NaN,y:1,z:1}),false); assert.equal(target._selectedPos.x,2);
});
test('invalid raycast indices and nonfinite point geometry never reach bridge queries', () => {
    const f=fixture(); f.target.viewport={_voidBox:{}};
    for(const index of [-1,NaN,undefined,7]) f.api.handleLatticeClick(f.target,[{index,object:{geometry:{getAttribute:()=>({array:[0.5,0.5,0.5]})}}}]);
    f.api.handleLatticeClick(f.target,[{index:0,object:{geometry:{getAttribute:()=>({array:[NaN,0.5,0.5]})}}}]);
    assert.equal(f.bridge.requests.length,0);
});

test('temporarily unavailable physics owner safely rejects selection, click and update', () => {
    const f=fixture(); const Inspector=load('inspector.js','Inspector',{normalizeLatticePosition:f.api.normalizeLatticePosition});
    const target=Object.create(Inspector.prototype);
    Object.assign(target,{bridge:f.bridge,_engineMode:'lattice',_selectedPos:{x:1,y:1,z:1},
        _showLatticeInspector(){},_hideLatticeInspector(){this.hidden=true}});
    target.setBridge(null); assert.equal(target._selectedPos,null); assert.equal(target.hidden,true);
    assert.equal(target.selectLatticePosition({x:1,y:1,z:1}),false);
    f.target.bridge=null; f.target.viewport={_voidBox:{}};
    assert.doesNotThrow(()=>f.api.updateLatticeFields(f.target));
    assert.doesNotThrow(()=>f.api.handleLatticeClick(f.target,[{index:0,object:{geometry:{getAttribute:()=>({array:[0.5,0.5,0.5]})}}}]));
    assert.equal(f.bridge.requests.length,0);
});
