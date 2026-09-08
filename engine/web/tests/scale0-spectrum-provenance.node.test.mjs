import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import { analyzeSpectrumObservation } from '../js/scales/scale0/analysis/spectrum-analysis-core.js';
import { captureSpectrumObservation } from '../js/scales/scale0/analysis/spectrum-analysis-client.js';
import * as topology from '../js/scales/scale0/analysis/lattice-topology.js';
import { cardStyle, titleStyle, tagBadge, formatExp as finiteExp, formatFixed as finiteFixed } from '../js/scales/scale0/ui/overlays/_card-helpers.js';

const strip = source => source.replace(/^import[\s\S]*?from\s+['"][^'"]+['"];\s*/gm, '').replace(/\bexport\s+/g, '');
const read = name => readFileSync(new URL(`../js/${name}`, import.meta.url), 'utf8');
const isCurrentScale0TelemetryMeta = vm.runInNewContext(`${strip(read('telemetry/scale0-read.js'))}\nisCurrentScale0TelemetryMeta`);
// Evaluate actual panel function bodies with real numerical and formatting dependencies.
const panel = vm.runInNewContext(`${strip(read('scales/scale0/ui/overlays/spectrum-panel.js'))}\n({readQualifiedTelemetry,METRIC_KINDS,renderSpectrum,renderTopology,renderMetrics,renderEnergy})`, {
    ...topology, cardStyle, titleStyle, tagBadge, finiteExp, finiteFixed, isCurrentScale0TelemetryMeta,
});
const emptySample = () => ({count:0, positions:new Float32Array(0), vectors:new Float32Array(0), values:new Float32Array(0)});
const caps = (available, sample = emptySample()) => ({
    getScale0FieldSamples: () => sample,
    hasScale0SamplerSnapshot: () => available,
});
const analyze = (capabilities, audit = null, mode = 'live') => analyzeSpectrumObservation(
    captureSpectrumObservation(capabilities, { L: 3, stride: 1, M: 8, mode, audit, metricKinds: panel.METRIC_KINDS }));
const rowValue = (html, label) => html.split(`${label}</span><span`)[1]?.split('>')[1]?.split('</span')[0]?.trim();

test('native and WASM Poynting schemas produce the same nonzero magnitude', () => {
    const a={}, b={};
    panel.renderEnergy(a, {poyntingX:3,poyntingY:4,poyntingZ:0}, null);
    panel.renderEnergy(b, {totalPoynting:{x:3,y:4,z:0}}, null);
    assert.equal(rowValue(a.innerHTML,'Poynting volume sum magnitude'),'5.00e+0');
    assert.equal(a.innerHTML,b.innerHTML);
    assert.match(a.innerHTML,/C_SPEED²\(E×B\)/);
    assert.match(a.innerHTML,/not net boundary flux/);
});

test('partial and nonfinite energy records remain unavailable while genuine zero survives', () => {
    const output={};
    panel.renderEnergy(output,{poyntingX:3,poyntingY:4,waveEnergy:null,fieldEnergy:NaN},undefined);
    assert.equal(rowValue(output.innerHTML,'Poynting volume sum magnitude'),'—');
    assert.equal(rowValue(output.innerHTML,'Accounted energy change'),'—');
    assert.equal(rowValue(output.innerHTML,'Flux-weight entropy'),'—');
    assert.doesNotMatch(output.innerHTML,/class="spec-energy-bar"|nan|NaN|Infinity/);
    panel.renderEnergy(output,{poyntingX:0,poyntingY:0,poyntingZ:0,energyDrift:0,waveEnergy:0,fieldEnergy:0},0,{auditTick:0,diagTick:1});
    assert.equal(rowValue(output.innerHTML,'Accounted energy change'),'0.000 %');
    assert.equal(rowValue(output.innerHTML,'Flux-weight entropy'),'0.00e+00');
    assert.match(output.innerHTML,/Audit tick 0; diagnostics tick 1/);
    assert.match(output.innerHTML,/first nonzero audit/);
});

test('native and WASM flux/wave asymmetries agree; absent or zero support has no ratio', () => {
    for (const audit of [null,{}, {ELTotal:0,ERTotal:0,wvLTotal:0,wvRTotal:0}, {ELTotal:NaN,ERTotal:1}, {ELTotal:-1,ERTotal:3}]) {
        const r=topology.chiralityFromAudit(audit);
        assert.equal(r.eAsym,null); assert.equal(r.wvAsym,null);
    }
    assert.equal(topology.chiralityFromAudit({waveLTotal:3,waveRTotal:1}).wvAsym,.5);
    assert.equal(topology.chiralityFromAudit({wvLTotal:3,wvRTotal:1}).wvAsym,.5);
    assert.equal(topology.chiralityFromAudit({ELTotal:1,ERTotal:1}).eAsym,0);
});

test('unavailable Gauss never receives a passing color; completed exact zero does', () => {
    const output={};
    const t=analyze(caps(false)).topology;
    panel.renderTopology(output,t);
    assert.equal(rowValue(output.innerHTML,'Vacuum constraint residual Σr²'),'—');
    assert.doesNotMatch(output.innerHTML,/positive-text/);
    assert.equal(t.defects,null); assert.equal(t.tubes,null);
    const valid=analyze(caps(true),{gaussViolation:0,maxGaussError:0}).topology;
    panel.renderTopology(output,valid);
    assert.match(output.innerHTML,/positive-text/);
    assert.equal(valid.defects.sources,0); assert.equal(valid.tubes.count,0);
});

test('pending placeholders are distinguished from completed empty samples', () => {
    assert.equal(analyze(caps(false)).spectrum,null);
    const complete=analyze(caps(true)).spectrum;
    assert.equal(complete.sampleCount,0);
    assert.equal(complete.spec.totalE,0);
    const output={};
    panel.renderSpectrum(output,null,false);
    assert.match(output.innerHTML,/unavailable or pending/);
    panel.renderSpectrum(output,complete,false);
    assert.match(output.innerHTML,/sampled grid/);
    assert.doesNotMatch(output.innerHTML,/No field energy/);
});

test('corrupt sample shapes and nonfinite values are rejected before reconstruction', () => {
    for (const sample of [
        {...emptySample(),count:-1}, {...emptySample(),count:1},
        {count:1,positions:[.5,.5,.5],vectors:[1,NaN,0],values:[NaN]},
        {count:1,positions:[Infinity,.5,.5],vectors:[1,0,0],values:[1]},
    ]) assert.equal(analyze(caps(true,sample)).spectrum,null);
});

test('empty or pending metric samples have no invented RMS or histogram', () => {
    for (const available of [false,true]) {
        const values=analyze(caps(available)).metrics;
        assert.ok(values.every(v=>v.stats===null && v.hist===null));
        const output={}; panel.renderMetrics(output,values);
        assert.match(output.innerHTML,/No retained sample statistics/);
        assert.doesNotMatch(output.innerHTML,/nan|0\.00e\+00/);
    }
    const sample={count:1,positions:[.5,.5,.5],values:[0]};
    assert.ok(analyze(caps(true,sample)).metrics.every(v=>v.stats.rms===0));
});

test('only current qualified groups are used, with independent sample ticks', () => {
    const diag={entropy:0}, audit={energyDrift:0};
    const meta={diagnostics:{tick:8},audit:{tick:7}};
    const hub={s0:{diag,audit},getScale0TelemetryMeta:group=>meta[group]};
    const r=panel.readQualifiedTelemetry(hub);
    assert.equal(r.diag,diag); assert.equal(r.audit,audit);
    assert.equal(r.diagTick,8); assert.equal(r.auditTick,7);
    for (const invalid of [null,{tick:7,stale:true},{tick:7,status:'unavailable'},{tick:-1},{tick:1.5},{tick:Number.MAX_SAFE_INTEGER+1}]) {
        meta.audit=invalid;
        assert.equal(panel.readQualifiedTelemetry(hub).audit,null);
    }
});

test('frozen-hero topology computes current magnitude connectivity without another FFT', () => {
    const c=caps(true,{count:1,positions:[1.5,1.5,1.5],vectors:[3,4,0]});
    const spatial=analyze(c,null,'topology');
    const transformed=analyze(c);
    assert.deepEqual(spatial.topology.tubes,transformed.topology.tubes);
    assert.equal(transformed.spectrum.mag[13],5);
    assert.equal(spatial.spectrum,null);
});
