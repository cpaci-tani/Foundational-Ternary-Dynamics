import test from 'node:test';
import assert from 'node:assert/strict';
import { prepareFluid, PREPARATIONS } from '../../strict/web/hydro/fluid-preparation.js';
import { analyzeFluid, fitMode } from '../../strict/web/hydro/fluid-analysis.js';
import { createFluidSnapshot } from '../js/strict/fluid-observables.js';

const observed = (options = {}, width = '2') => createFluidSnapshot(prepareFluid({L:8,...options}).checkpoint).observe(width);
test('seeded preparations are reproducible, finite, distinct and keep single-phase exclusion', () => {
    const original = prepareFluid({L:8});
    assert.equal(original.checkpoint,prepareFluid({L:8}).checkpoint);
    assert.notEqual(original.checkpoint,prepareFluid({L:8,seed:1730}).checkpoint);
    for (const preparation of Object.keys(PREPARATIONS)) {
        const p = prepareFluid({L:8,preparation});
        const obs = createFluidSnapshot(p.checkpoint).observe('1');
        assert.equal(obs.ledger.field_tokens_by_polarity[1],'0');
        assert.equal(obs.ledger.relation_tokens_sc,String(6*8**3));
        assert.equal(obs.ledger.relation_tokens_fcc,String(12*8**3));
    }
    assert.throws(()=>{PREPARATIONS['hydro-shear-wave-t2'].k[0]=2;});
    assert.throws(()=>prepareFluid({seed:0}));
    assert.throws(()=>prepareFluid({density:NaN}));
});
test('resolution changes preserve exact raw moments while central stress obeys covariance nesting', () => {
    const fine=analyzeFluid(observed({},'1')), coarse=analyzeFluid(observed({},'8'));
    assert.equal(fine.total,coarse.total);
    assert.deepEqual(fine.momentum,coarse.momentum);
    const b=coarse.blocks[0], V=8**3;
    const pairs=[[0,0],[1,1],[2,2],[0,1],[0,2],[1,2]];
    for(let k=0;k<6;k++) {
        const [i,j]=pairs[k];
        const nested=fine.blocks.reduce((s,f)=>s+f.stress[k]+(f.N?f.N*(f.velocity[i]-b.velocity[i])*(f.velocity[j]-b.velocity[j]):0),0)/V;
        assert.ok(Math.abs(nested-b.stress[k])<1e-12);
    }
});
test('vacuum has zero raw/central stress but undefined velocity and all derivatives', () => {
    const a=analyzeFluid(observed({density:0}));
    assert.equal(a.total,0); assert.equal(a.rmsSpeed,null); assert.equal(a.derivativeBlocks,0);
    assert.ok(a.blocks.every(b=>b.velocity===null&&b.divergence===null&&b.curl===null&&b.pressure===0&&b.stress.every(v=>v===0)));
    const emptyPol=analyzeFluid(observed(),'1');
    assert.equal(emptyPol.derivativeBlocks,0);
});
test('periodic central stencil has correct curl, spacing and advective term on a shear', () => {
    const obs=observed({},'2'), side=4;
    obs.blocks=Array.from({length:side**3},(_,q)=>{
        const x=Math.floor(q/(side*side)); const py=[0,2,0,-2][x];
        return {field_tokens:['4','0'],momentum:[['0',String(py),'0'],['0','0','0']],second_moment:[['0','4','0','0','0','0'],Array(6).fill('0')]};
    });
    const b=analyzeFluid(obs).blocks[0];
    assert.equal(b.gradient[1][0],0.25); assert.deepEqual(b.curl,[0,0,0.25]);
    assert.ok(Math.abs(b.strainNorm-Math.sqrt(0.03125))<1e-14);
    assert.equal(b.divergence,0); assert.deepEqual(b.advection,[0,0,0]);
    assert.equal(analyzeFluid(observed({},'4')).derivativeBlocks,0);
});
test('analysis rejects inconsistent support, imprecise integers and nonzero vacuum moments', () => {
    const malformed=observed(); malformed.width='16'; assert.throws(()=>analyzeFluid(malformed));
    const bad=observed({density:0}); bad.blocks[0].momentum[0][0]='1'; assert.throws(()=>analyzeFluid(bad));
    const overflow=observed(); overflow.blocks[0].field_tokens[0]='9007199254740993'; assert.throws(()=>analyzeFluid(overflow));
});
test('mode diagnostics use real cycle intervals, preserve growth and reject singular fits', () => {
    const decay=Array.from({length:8},(_,i)=>({cycle:100+2*i,amplitude:Math.exp(-0.4*i)}));
    const fit=fitMode(decay,0.25,true);
    assert.ok(Math.abs(fit.gamma-0.2)<1e-12); assert.ok(Math.abs(fit.diffusivity-0.8)<1e-12);
    assert.equal(fit.r2,1); assert.equal(fit.from,100); assert.equal(fit.to,114);
    assert.equal(fitMode(decay,0.25,false).available,false);
    assert.equal(fitMode(decay.slice(1),0.25,true).available,false);
    assert.equal(fitMode(decay,Number.MIN_VALUE,true).available,false);
    const growth=decay.map(p=>({...p,amplitude:1/p.amplitude}));
    assert.ok(fitMode(growth,0.25,true).diffusivity<0);
    const zero=decay.map(p=>({...p,amplitude:0})); assert.equal(fitMode(zero,.25,true).available,false);
    const huge=decay.map((p,i)=>({...p,cycle:i*1e307})); assert.equal(fitMode(huge,.25,true).available,false);
});
