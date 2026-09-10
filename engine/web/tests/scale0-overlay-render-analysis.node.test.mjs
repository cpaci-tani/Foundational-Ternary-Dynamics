import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import vm from 'node:vm';
import * as THREE from '../js/vendor/three/build/three.module.js';
import { DUAL_DELTA } from '../js/constants.js';
function load(path, expression, extra={}) {
 const url=new URL('../js/'+path,import.meta.url);
 const src=readFileSync(url,'utf8').replace(/^import[\s\S]*?from\s+['"][^'"]+['"];\s*/gm,'').replace(/export\s+/g,'').replace(/import\.meta\.url/g,JSON.stringify(url.href));
 return vm.runInNewContext(src+'\n'+expression,{URL,...extra});
}
const spec=load('scales/scale0/analysis/lattice-spectrum.js','({energySpectrum,resampleInto,spectralPeak,fft1d,nextPow2,denseVectorGridFromSamples})');
test('corner Fourier power retains its radial wavenumber and Parseval',()=>{
 const M=4, jx=Float64Array.from({length:M**3},(_,i)=> ((i%M+Math.floor(i/M)%M+Math.floor(i/M**2))%2 ? -1:1));
 const r=spec.energySpectrum({jx,jy:new Float64Array(M**3),jz:new Float64Array(M**3)},M,M,M);
 assert.ok(Math.abs(r.totalE-M**3)<1e-10);
 const peak=spec.spectralPeak(r.k,r.E);
 assert.ok(peak.kPeak>r.kNyq,'corner mode must not fold below axial Nyquist');
 assert.equal(spec.spectralPeak([1,2],[0,0]).kPeak,0);
});
test('periodic resampling interpolates across the seam; invalid FFT dimensions reject',()=>{
 const src=Float64Array.from({length:8},(_,i)=>i%2?2:0), dst=new Float64Array(64);
 spec.resampleInto(src,2,4,dst);
 assert.equal(dst[3],1);
 assert.throws(()=>spec.fft1d(new Float64Array(3),new Float64Array(3)),/power-of-two/);
 assert.throws(()=>spec.nextPow2(Infinity),/Invalid/);
});
const helium=load('scales/scale0/analysis/helium-spectrum-protocol.js','({timeSeriesPowerSpectrum})',spec);
test('temporal spectrum never silently compresses gaps or accepts invalid sample spacing',()=>{
 for(const [v,dt] of [[[1,NaN,2,3,4],1],[[1,2,3,4],0],[[1,2,3,4],Infinity]]) assert.throws(()=>helium.timeSeriesPowerSpectrum(v,{dt}),/finite/);
 assert.equal(helium.timeSeriesPowerSpectrum([1,-1,1,-1],{dt:2}).n,4);
});
function mesh(){const attrs=Object.fromEntries(['position','particleColor','color','size'].map(k=>[k,{array:new Float32Array(60),count:20}]));const geo={attributes:attrs,getAttribute:k=>attrs[k],drawRange:{count:5},setDrawRange(_,n){this.drawRange.count=n}};return {geometry:geo,visible:true};}
const base={_syncCenterAndRadius(){},_clipActive:()=>false,_ensureActiveIdx:n=>new Int32Array(n)};
const em=load('viewport/field-em-renderer.js','fieldEmMethods',{VOXEL_CENTER_OFFSET:0});
const quantum=load('viewport/field-quantum-renderer.js','fieldQuantumMethods',{VOXEL_CENTER_OFFSET:0});
test('invalid dual publications clear only dual geometry, including first use without chirality',()=>{
 const valid={count:1,positions:new Float32Array([1,2,3]),vectors:new Float32Array([3,0,0])};
 for(const vectors of [[Infinity,0,0],[NaN,0,0],[1e40,0,0]]) {
  const ctx={...base,_buildDualFluxVolume(){this._dualFluxVolume=mesh();}};
  assert.doesNotThrow(()=>quantum.updateDualFluxVolume.call(ctx,{...valid,vectors},valid));
  assert.equal(ctx._dualFluxVolume.geometry.drawRange.count,0);
  const chirality=mesh();ctx._chiralityField=chirality;
  quantum.updateDualFluxVolume.call(ctx,valid,{...valid,vectors});
  assert.equal(ctx._dualFluxVolume.geometry.drawRange.count,0);
  assert.equal(chirality.geometry.drawRange.count,5);
  for(const a of Object.values(ctx._dualFluxVolume.geometry.attributes))assert.ok(a.array.every(Number.isFinite));
 }
 for(const x of [NaN,Infinity,1e40]) {
  const ctx={...base,_dualFluxVolume:mesh()};
  quantum.updateDualFluxVolume.call(ctx,valid,{...valid,positions:[x,2,3]});
  assert.equal(ctx._dualFluxVolume.geometry.drawRange.count,0);
  for(const a of Object.values(ctx._dualFluxVolume.geometry.attributes))assert.ok(a.array.every(Number.isFinite));
 }
});
test('dual finite amplitudes retain their exact coordinates, colors and sizes after an invalid frame',()=>{
 const ctx={...base,_dualFluxVolume:mesh()}, left={count:1,positions:[1,2,3],vectors:[3,0,0]},right={count:1,positions:[4,5,6],vectors:[0,4,0]};
 quantum.updateDualFluxVolume.call(ctx,{...left,vectors:[NaN,0,0]},right);
 quantum.updateDualFluxVolume.call(ctx,left,right);
 const g=ctx._dualFluxVolume.geometry;
 assert.equal(g.drawRange.count,2);
 assert.deepEqual(Array.from(g.attributes.position.array.slice(0,6)),[1,2,3,4,5,6]);
 assert.deepEqual(Array.from(g.attributes.particleColor.array.slice(0,6)),[.9*.75,.4*.75,.15*.75,.3,.2,.9].map(Math.fround));
 assert.deepEqual(Array.from(g.attributes.size.array.slice(0,2)),[4,5]);
 quantum.updateDualFluxVolume.call(ctx,{...left,vectors:[0,0,0]},{...right,vectors:[0,0,0]});
 assert.equal(g.drawRange.count,0);
});
test('zero vector and scalar fields emit no NaN vertices or bogus zero arrows',()=>{
 const data={positions:new Float32Array([1,2,3]),vectors:new Float32Array(3),values:new Float32Array(1),count:1};
 for(const [methods,method,key] of [[em,'updatePoyntingVectors','_poyntingVectors'],[em,'updateDivergenceField','_divField'],[quantum,'updateChiralityField','_chiralityField']]){
 const m=mesh(),ctx={...base,[key]:m};methods[method].call(ctx,data);assert.equal(m.geometry.drawRange.count,0);for(const a of Object.values(m.geometry.attributes))assert.ok(a.array.every(Number.isFinite));
 }
 const m=mesh();em._writeArrowFieldIntoMesh.call(base,m,data,{base:[0,0,0],tip:[1,1,1]},'_m');assert.equal(m.geometry.drawRange.count,0);
});

test('chirality rejects malformed and mixed nonfinite publications before GPU upload',()=>{
 const valid={count:2,positions:[1,2,3,4,5,6],values:[1,-.5]};
 const invalid=[null,{...valid,count:-1},{...valid,count:1.5},
  {...valid,values:[1]},{...valid,positions:[1,2,3]}];
 for(const bad of [NaN,Infinity,-Infinity,1e40]) {
  invalid.push({...valid,values:[1,bad]}, {...valid,positions:[1,2,3,4,bad,6]});
 }
 for(const data of invalid) {
  const m=mesh(),ctx={...base,_chiralityField:m};
  quantum.updateChiralityField.call(ctx,valid);
  assert.equal(m.geometry.drawRange.count,2);
  quantum.updateChiralityField.call(ctx,data);
  assert.equal(m.geometry.drawRange.count,0);
  for(const a of Object.values(m.geometry.attributes))assert.ok(a.array.every(Number.isFinite));
  quantum.updateChiralityField.call(ctx,valid);
  assert.equal(m.geometry.drawRange.count,2,'valid publication recovers after corruption');
 }
});

test('phase alone uses the current sampled split across direction, order and owner changes',()=>{
 const computePhaseFrame=load('scales/scale0/runtime/overlay-frames.js','computePhaseFrame');
 const phase=load('scales/scale0/runtime/field-overlays.js',
  "SCALAR_JOBS.find(([flag])=>flag==='showPhase')[1]",{computePhaseFrame,DUAL_DELTA});
 const state={fieldFlags:{showPhase:true,showDualSubstrate:false}};
 const expected=Math.atan2(1-DUAL_DELTA,1+DUAL_DELTA);
 for(const [vectors,positions,owner] of [
  [[1,0,0],[1,2,3],'old'],
  [[-1,0,0],[1,2,3],'old'],
  [[0,-2,0,0,0,3],[4,5,6,1,2,3],'new'],
 ]) {
  const fluxVector={count:vectors.length/3,vectors,positions,provenance:{owner}};
  const result=phase({fluxVector},{},state);
  assert.equal(result.positions,positions);
  assert.ok(result.dualAvailable);
  for(let i=0;i<result.count;i++)assert.ok(Math.abs(result.values[i]-expected)<1e-6);
 }
 assert.equal(phase({fluxVector:{count:0}},{},state),null);
});

test('spin arrow geometry stays aligned with every signed axis during illustrative rotation',()=>{
 const SpinArrowManager=load('viewport/spin-arrow-manager.js','SpinArrowManager',
  {THREE,performance:{now:()=>0}});
 const scene=new THREE.Scene(),manager=new SpinArrowManager(scene);
 for(const axis of [[1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]]) {
  manager.track(1,{getPosition:()=>({x:2,y:3,z:4}),
   getSpin:()=>({sx:axis[0],sy:axis[1],sz:axis[2],omega_z:1})});
  for(let i=0;i<80;i++)manager.update(16);
  const tracked=manager._tracked.get(1),shaft=tracked.inner.children[0];
  scene.updateMatrixWorld(true);
  shaft.geometry.computeBoundingBox();
  const direction=shaft.geometry.boundingBox.getCenter(new THREE.Vector3());
  shaft.localToWorld(direction);
  direction.sub(tracked.group.position).normalize();
  assert.ok(direction.distanceTo(new THREE.Vector3(...axis))<1e-6,axis.join(','));
  manager.untrack(1);
 }
 manager.dispose();
 assert.equal(scene.children.length,0);
});
test('empty quantum, phase and horizon publications clear older geometry',()=>{
 const q=mesh();quantum._populateQuantumField.call({...base,_quantumField:q},null,'psi2');assert.equal(q.geometry.drawRange.count,0);
 const h=mesh();quantum.updateHorizonField.call({...base,_horizonField:{geo:h.geometry}}, {count:0});assert.equal(h.geometry.drawRange.count,0);
 const p=mesh();em.updatePhaseField.call({...base,_phaseVisible:true,_phaseNeedles:p},{count:0});assert.equal(p.geometry.drawRange.count,0);
});

test('material instances own independent shader uniform records',()=>{
 const uniforms={shape:{value:1}};
 const make=load('viewport/field-renderer-shared.js','_makeParticleFragMaterial',{
  THREE:{ShaderMaterial:class{constructor(o){Object.assign(this,o)}},NormalBlending:1},
  PARTICLE_SHADER_UNIFORMS:uniforms,PARTICLE_VERT:'',PARTICLE_FRAG:''});
 const a=make(),b=make();a.uniforms.shape.value=3;
 assert.equal(b.uniforms.shape.value,1);assert.equal(uniforms.shape.value,1);
});

test('queued flux snapshots survive producer reuse; hiding cancels active and pending jobs',()=>{
 const F=load('viewport/flux-renderer.js','ViewportFluxRenderer',{
  createFluxActivationStepper:()=>({}),cancelAnimationFrame:()=>{},
 });
 const ctx=Object.create(F.prototype);
 Object.assign(ctx,{_fluxActivation:new Float64Array(0),_fluxAsyncJob:{},_fluxAsyncRaf:0,
  _mapManifestedState(p){this.mapped=p},_scheduleFluxAsyncSlice(){},_fluxVolume:mesh()});
 ctx._ensureFluxActivationCapacity(8);
 const density=Float64Array.from({length:8},(_,i)=>i+1),positions=new Float32Array([1,2,3]);
 ctx._queueLargeFluxUpdate({density,sourceN:2}, {positions,count:1});
 density.fill(99);positions.fill(99);ctx._startPendingFluxUpdate();
 assert.deepEqual(Array.from(ctx._fluxAsyncJob.density),[1,2,3,4,5,6,7,8]);
 assert.deepEqual(Array.from(ctx.mapped.positions),[1,2,3]);
 ctx._queueLargeFluxUpdate({density,sourceN:2},null);
 assert.equal(ctx._fluxAsyncJob.density[0],1,'pending frame cannot overwrite active snapshot');
 ctx.toggleFluxVolume(false);
 assert.equal(ctx._fluxAsyncJob,null);assert.equal(ctx._fluxPendingFrame,null);
 assert.equal(ctx._fluxVolume.geometry.drawRange.count,0);
});

test('streamlines preserve perpendicular B seed orientation and reject zero/nonfinite flow',()=>{
 const lines=load('fieldlines.js','({generateBFieldSeeds,computeStreamlines})');
 const seeds=lines.generateBFieldSeeds([{x:5,y:5,z:5,fx:1,fy:0,fz:0}],2,8);
 for(const p of seeds)assert.ok(Math.abs(p[0]-5)<1e-12,'ring must be perpendicular to x');
 for(const vector of [[0,0,0],[NaN,0,0],[Infinity,0,0]]) {
  const r=lines.computeStreamlines({fieldFn:()=>vector},[[5,5,5]],{N:10,maxSteps:4,minMag:0});
  assert.equal(r.count,0);
 }
});

test('sheet resize retires data support and shared gravity geometry is disposed once',()=>{
 const noop=()=>{};
 const T=load('viewport/topology-sheet-renderer.js','TopologySheetRenderer',{
  rampGravWell:noop,rampEmEnergy:noop,rampCharge:noop,rampVorticity:noop,rampEPressure:noop,rampBPressure:noop});
 const ctx=Object.create(T.prototype),solid=mesh(),wire=mesh();
 Object.assign(ctx,{_topoSheets:{emEnergy:{solid,wire,lastData:{count:1}}},
  _gravPotData:{count:1},_scatterBufs:{_bd:{}},_rebuildSheetIfResized:noop});
 ctx.onLatticeSizeChanged(9,4.5);
 assert.equal(ctx._topoSheets.emEnergy.lastData,null);assert.equal(solid.geometry.drawRange.count,0);
 assert.equal(ctx._gravPotData,null);assert.equal(ctx._scatterBufs._bd,null);
 let disposed=0;const shared={dispose(){disposed++}};
 Object.assign(ctx,{scene:{remove:noop},_topoSheets:{},_gravSurface:{geometry:shared,material:{dispose:noop}},_gravSurfaceWire:{geometry:shared,material:{dispose:noop}}});
 ctx.dispose();assert.equal(disposed,1);
});

test('Flux Slice releases sampler demands from their original owner',()=>{
 const P=load('scales/scale0/ui/overlays/flux-slice-panel.js','FluxSlicePanel');
 const calls=[];const old={replaceSamplerWants:(...v)=>calls.push(['old',...v])};
 const next={replaceSamplerWants:(...v)=>calls.push(['new',...v])};
 const ctx=Object.create(P.prototype);
 Object.assign(ctx,{_samplerBridge:old,getBridge:()=>next,_prevWantedKeys:new Set()});
 ctx._releaseAllWantedSamplers();assert.equal(calls.length,1);assert.equal(calls[0][0],'old');
 assert.equal(ctx._samplerBridge,null);
});


test('spectrum reconstruction follows effective stride and center origin, not requested stride',()=>{
 const grid=spec.denseVectorGridFromSamples({effectiveStride:2,origin:0,count:2,
  positions:[.5,.5,.5,96.5,96.5,96.5],vectors:[1,2,3,4,5,6]},97,1);
 assert.equal(grid.srcN,49);assert.equal(grid.jx.length,49**3);
 assert.equal(grid.jx[0],1);assert.equal(grid.jx.at(-1),4);
 assert.equal(grid.effectiveStride,2);assert.equal(grid.uniformPeriodicSpacing,false);
 const centered=spec.denseVectorGridFromSamples({effectiveStride:4,origin:3,count:1,
  positions:[3.5,3.5,3.5],vectors:[2,0,0]},7,1);
 assert.equal(centered.srcN,1);assert.equal(centered.jx[0],2);
});

function nestedFunction(file,name,nextMarker,context={}) {
 const source=readFileSync(new URL('../js/scales/scale0/ui/overlays/'+file,import.meta.url),'utf8');
 const start=source.indexOf('    function '+name+'('),end=source.indexOf(nextMarker,start);
 assert.ok(start>=0&&end>start);
 return vm.runInNewContext(source.slice(start,end)+'\n'+name,context);
}
test('canceling clock phases have no mean direction and expose sampled support',()=>{
 const sample=nestedFunction('time-panel.js','sampleProperTimeMetrics','    // Card F —');
 const result=sample({getScale0FieldSamples:({kind})=>kind==='dbPhase'
  ? {values:new Float32Array([0,Math.PI]),count:2,effectiveStride:2}
  : {values:[],count:0,effectiveStride:2}});
 assert.ok(Number.isNaN(result.dbPhaseMean));assert.ok(result.dbPhaseCircVar>.999999);
 assert.equal(result.phaseCount,2);assert.equal(result.phaseStride,2);
});
test('scenario intent clears retained spectrum plot and numeric cards while pending',()=>{
 const bodies=Array.from({length:4},()=>({innerHTML:'previous measurement 123'}));
 const context={scenarioSyncToken:0,scenarioSyncRaf:0,EMPTY_SCENARIO_ID:'empty',
  stopCoordinator(){},releaseSamplerWants(){},analysis:{cancel(){context.cancelCalls++;}},cancelCalls:0,
  deepTimer:0,deepPending:true,deepRequestToken:1,latestAnalysisResult:{},mode:'deep',lastSpec:{},
  modeBadge:{},liveBtn:{},deepBtn:{},panel:{dataset:{}},
  specBody:bodies[0],topoBody:bodies[1],metBody:bodies[2],enBody:bodies[3],
  SCENARIO_SYNC_MAX_FRAMES:1,disposed:false,inapplicable:false,getScale0State:()=>({currentScenarioId:'new'}),
  isScale0AuthoritativeGenerationReady:()=>false};
 context.cancelDeepMeasurement=nestedFunction('spectrum-panel.js','cancelDeepMeasurement','    function stopCoordinator',context);
 nestedFunction('spectrum-panel.js','handleScenarioIntent','    function onScenarioChange',context)('new');
 for(const body of bodies){assert.match(body.innerHTML,/measurement unavailable/);assert.doesNotMatch(body.innerHTML,/123/);}
 assert.equal(context.lastSpec,null);assert.equal(context.panel.dataset.applicability,'pending-scenario');
 assert.equal(context.deepPending,false);assert.equal(context.latestAnalysisResult,null);
 assert.equal(context.cancelCalls,1);
});
