import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';
const source = fs.readFileSync(new URL('../js/scales/scale0/runtime/scenario-loader.js', import.meta.url), 'utf8');
const helper = source.slice(source.indexOf('function syncScale0InspectorOwner('), source.indexOf('export async function fallbackToInThreadEngine('));
const fallback = source.slice(source.indexOf('export async function fallbackToInThreadEngine('), source.indexOf('export function loadScale0Scenario(')).replace('export ', '');
const loader = source.slice(source.indexOf('export function loadScale0Scenario('), source.indexOf('async function performScale0LatticeResize(')).replace('export ', '');
function fixture(workerEligible = false) {
    const seen = [], state = { currentScenarioId: 'fixture', fieldFlags: {}, tickAccumulator: {reset() {}} };
    const main = { latticeSize: 33, capabilities: {scale0:{}}, setupScenario() {} };
    const ctx = { bridge:main, engineMode:'lattice', _loadGeneration:0, resetAllVisualState() {},
        viewport:{latticeSize:33,setLatticeSize(n){this.latticeSize=n;}},
        inspector:{ setBridge(owner) { this.bridge=owner; seen.push(owner); } } };
    let callbacks;
    class Worker {
        constructor() { this.latticeSize=97; this.capabilities={scale0:{}}; this.isWorker=true; }
        beginConfiguration(options) { callbacks=options; return this.configurationToken=1; }
        setupScenario() {}
    }
    const g = { console, Number, Array, Object, Promise, WasmBridgeProxy:Worker,
        telemetryHub:{resetScale() {}}, getScale0State:()=>state,
        getScale0Scenario:()=>({id:'fixture',load:bridge=>{ if (workerEligible) assert.equal(ctx.inspector.bridge,bridge,'barriered worker must be assigned before setup'); return true; }}),
        getPhysicsHarness:bridge=>bridge, wasmWorkerEligible:()=>workerEligible,
        setCurrentScenarioId:id=>state.currentScenarioId=id, setFluxMock:(b,on)=>{state.fluxMock=b;state.useFluxMock=on;},
        captureOverlayPreferences:()=>({}), syncScale0ToggleUiFromEngine:()=>true,
        readAuthoritativeTick:()=>0, DEFAULT_TOGGLES:[],
        ...Object.fromEntries(['setPhysicsToggleCardPending','setSelectedScenarioId','beginScale0AuthoritativeLoad','completeScale0AuthoritativeLoad','failScale0AuthoritativeLoad','applyToggleDefaults','applyAuxiliaryDefaults','applyGravityAbsorbingToggles','markScenarioOverrideRows','syncComboSliders','restoreOverlayPreferences','applyScenarioVisualProfile','applyScale0OverlayApplicability','applyScenarioCameraFocus','recomputeAnyFieldActive','reportScenarioSetupFailure'].map(k=>[k,()=>{}])),
    };
    const api=vm.runInNewContext(helper+loader+'\n({loadScale0Scenario,syncScale0InspectorOwner})',g);
    return {ctx,state,main,seen,g,api,callbacks:()=>callbacks};
}
test('canonical loader points Inspector at the worker before setup and again on reset',()=>{
    const f=fixture(true); f.api.loadScale0Scenario(f.ctx,f.state,{},'fixture');
    assert.equal(f.ctx.inspector.bridge,f.state.fluxMock); assert.notEqual(f.ctx.inspector.bridge,f.main);
    const first=f.ctx.inspector.bridge; first.canReconfigure=()=>true;
    f.api.loadScale0Scenario(f.ctx,f.state,{},'fixture');
    assert.equal(f.ctx.inspector.bridge,first); assert.equal(f.seen.length,2);
});
test('canonical direct load replaces a previous worker Inspector owner',()=>{
    const f=fixture(); f.ctx.inspector.bridge={isWorker:true};
    f.api.loadScale0Scenario(f.ctx,f.state,{},'fixture'); assert.equal(f.ctx.inspector.bridge,f.main);
});
test('owner synchronization uses runtime facade and cannot overwrite another active scale',()=>{
    const f=fixture(); let received;
    f.ctx.inspectorRuntime={setBridge:b=>{received=b;}};
    f.api.syncScale0InspectorOwner(f.ctx,f.main); assert.equal(received,f.main); assert.equal(f.seen.length,0);
    f.ctx.engineMode='particles'; f.api.syncScale0InspectorOwner(f.ctx,null); assert.equal(received,f.main);
});
test('async fallback detaches unavailable owner and a superseded load cannot restore it',async()=>{
    const f=fixture(); let resolve; let loads=0;
    const api=vm.runInNewContext(helper+fallback+'\n({fallbackToInThreadEngine})', {...f.g,
        enforceDirectWasmInteractiveFallback:()=>new Promise(r=>resolve=r),
        loadScale0Scenario:()=>{loads++;},
    });
    f.ctx.inspector.bridge={isWorker:true};
    const pending=api.fallbackToInThreadEngine(f.ctx,f.state,{},'fixture',{});
    assert.equal(f.ctx.inspector.bridge,null); f.ctx._loadGeneration++;
    f.ctx.inspector.bridge={newer:true}; resolve({clamped:false}); await pending;
    assert.equal(loads,0); assert.equal(f.ctx.inspector.bridge.newer,true);
});
test('successful async fallback delegates owner installation to the canonical loader',async()=>{
    const f=fixture(); let loads=0;
    const api=vm.runInNewContext(helper+fallback+'\n({fallbackToInThreadEngine})', {...f.g,
        enforceDirectWasmInteractiveFallback:async()=>({clamped:false}),
        loadScale0Scenario:(...args)=>{loads++; return f.api.loadScale0Scenario(...args);},
    });
    f.ctx.inspector.bridge={isWorker:true}; await api.fallbackToInThreadEngine(f.ctx,f.state,{},'fixture',{});
    assert.equal(loads,1); assert.equal(f.ctx.inspector.bridge,f.main);
});

const inspectorSource = fs.readFileSync(new URL('../js/inspector.js', import.meta.url), 'utf8');
const actualSetBridge = vm.runInNewContext('class Subject {' + inspectorSource.slice(
    inspectorSource.indexOf('    setBridge(bridge)'), inspectorSource.indexOf('    getSelectedLatticePosition()'))
    + '} Subject.prototype.setBridge');
test('direct retained selection reads only the finished zero-tick preparation',()=>{
    const f=fixture(); const samples=[]; let preparation='old';
    f.main.currentTick=()=>0; f.main.inspectVoxel=()=>({preparation,tick:0});
    Object.assign(f.ctx.inspector,{ bridge:f.main, _engineMode:'lattice', _selectedPos:{x:1,y:1,z:1},
        setBridge:actualSetBridge, selectLatticePosition() {samples.push(this.bridge.inspectVoxel());return true;} });
    f.g.getScale0Scenario=()=>({id:'fixture',load:()=>{preparation='seeded';return true;}});
    f.g.applyAuxiliaryDefaults=()=>{preparation='auxiliary';};
    f.g.applyGravityAbsorbingToggles=()=>{preparation='complete';};
    f.api.loadScale0Scenario(f.ctx,f.state,{},'fixture');
    assert.deepEqual(samples,[{preparation:'complete',tick:0}]);
    preparation='another-old-zero-tick-state'; f.api.loadScale0Scenario(f.ctx,f.state,{},'fixture');
    assert.deepEqual(samples,[{preparation:'complete',tick:0},{preparation:'complete',tick:0}]);
});
test('direct failed setup detaches the owner rather than rereading its old preparation',()=>{
    const f=fixture(); f.ctx.inspector.bridge=f.main;
    f.g.getScale0Scenario=()=>({id:'fixture',load:()=>false});
    assert.equal(f.api.loadScale0Scenario(f.ctx,f.state,{},'fixture'),false);
    assert.equal(f.ctx.inspector.bridge,null);
});
