// Synthetic snapshot/protocol tests only; no microscopic evolution is implemented.
import test from 'node:test';
import assert from 'node:assert/strict';
import { createFluidSnapshot, summarizeFluidAdvance, HYDRO_VELOCITIES } from '../js/strict/fluid-observables.js';
import { createWasmHydroAdapter, createHydroWorkerProtocol } from '../js/strict/hydro-worker-protocol.js';

const LAW = 'phi-hydro-staged-candidate-1';
const TABLE = 'abf25cf26072c03b5b7865fe84d3f31c270263d2c061e7bf3d81e27d783b5375';
const ENCODING = '3c10c134dadf3aa6f32f31ba588996e3c4755af67d4c804db567f5c4b361270c';
const SIZES = {s:1,bank:192,sc:6,fcc:12,admitted_sc:3,admitted_fcc:6,gate_sc:3,gate_fcc:6};
function checkpoint({L=4,tick='0',edit}={}) {
    const arrays = Object.fromEntries(Object.entries(SIZES).map(([name,size]) =>
        [name,Buffer.alloc(L**3*size,name === 'sc' || name === 'fcc' ? 4 : 0)]));
    arrays.bank[12] = 1;
    arrays.bank[3*24+3] = 1;
    arrays.bank[(L**3-1)*192+96+2*24+23] = 1;
    arrays.sc[0] = 7; arrays.fcc[(L**3-1)*12] = 0;
    edit?.(arrays);
    return JSON.stringify({schema:'ftd-hydro-checkpoint-2',law:LAW,table:TABLE,encoding:ENCODING,
        boundary:'periodic',L,microtick:tick,
        arrays:Object.fromEntries(Object.entries(arrays).map(([k,v]) => [k,v.toString('base64')]))});
}
function advanceResult(tick='4',absorptions=[]) {
    return {diagnostics:{law:LAW,table_hash:TABLE,L:'4',microtick:tick,phase:String(BigInt(tick)%4n),work_units:'5'},
        events:{absorptions,collisions:[],crossings:[],gate_holds:[]}};
}

test('exact block counts, momentum, and six second moments retain both polarities and all phases', () => {
    const reader = createFluidSnapshot(checkpoint());
    const result = reader.observe('1');
    assert.equal(result.observation_phase,'snapshot');
    assert.deepEqual(result.second_moment_order,['xx','yy','zz','xy','xz','yz']);
    assert.deepEqual(result.blocks[0],{field_tokens:['2','0'],momentum:[['0','1','0'],['0','0','0']],
        second_moment:[['2','1','0','1','0','0'],['0','0','0','0','0','0']],relation_tokens:'1'});
    assert.deepEqual(result.blocks[63],{field_tokens:['0','1'],momentum:[['0','0','0'],['0','-1','-1']],
        second_moment:[['0','0','0','0','0','0'],['0','1','1','0','0','1']],relation_tokens:'1'});
    assert.equal(result.ledger.work_units,'5');
    assert.deepEqual(result.ledger.field_tokens_by_polarity,['2','1']);
    assert.equal(result.source.status,'unavailable');
    const coarse = reader.observe('2');
    assert.equal(coarse.side,'2'); assert.equal(coarse.block_sites,'8');
    assert.deepEqual(coarse.blocks[0],result.blocks[0]);
    assert.deepEqual(coarse.blocks[7],result.blocks[63]);
    coarse.blocks[0].field_tokens[0]='999';
    assert.equal(reader.observe('4').blocks[0].field_tokens[0],'2');
});

test('exported canonical velocity order is deeply immutable', () => {
    assert.equal(HYDRO_VELOCITIES.length,24);
    assert.ok(Object.isFrozen(HYDRO_VELOCITIES));
    assert.ok(HYDRO_VELOCITIES.every(Object.isFrozen));
    assert.throws(() => {HYDRO_VELOCITIES[0][0]=9;});
});

test('L32 maximum population remains an exact bounded integer sum', () => {
    const reader = createFluidSnapshot(checkpoint({L:32,edit(a) {
        a.bank.fill(0);
        for(let site=0;site<32**3;++site) for(let p=0;p<2;++p) a.bank.fill(1,site*192+p*96,site*192+p*96+24);
    }}));
    const result = reader.observe('32');
    assert.deepEqual(result.blocks[0].field_tokens,['786432','786432']);
    assert.deepEqual(result.blocks[0].momentum,[['0','0','0'],['0','0','0']]);
    assert.deepEqual(result.blocks[0].second_moment,[['393216','393216','393216','0','0','0'],['393216','393216','393216','0','0','0']]);
    assert.equal(result.ledger.work_units,'1572866');
});

test('snapshot rejects wrong identities, framing, bank alphabet, exclusion, and relation symbols', () => {
    for(const key of ['schema','law','table','encoding','boundary']) {
        const value=JSON.parse(checkpoint()); value[key]='foreign';
        assert.throws(() => createFluidSnapshot(JSON.stringify(value)),/foreign/);
    }
    for(const L of [2,33,4.5,true]) {
        const value=JSON.parse(checkpoint()); value.L=L;
        assert.throws(() => createFluidSnapshot(JSON.stringify(value)),/lattice/);
    }
    for(const tick of ['-0','1.5','18446744073709551616',4]) {
        const value=JSON.parse(checkpoint()); value.microtick=tick;
        assert.throws(() => createFluidSnapshot(JSON.stringify(value)),/microtick/);
    }
    const extra=JSON.parse(checkpoint()); extra.arrays.unknown='';
    assert.throws(() => createFluidSnapshot(JSON.stringify(extra)),/fields/);
    const bad=JSON.parse(checkpoint()); bad.arrays.bank=bad.arrays.bank.slice(4);
    assert.throws(() => createFluidSnapshot(JSON.stringify(bad)),/base64/);
    const padded=JSON.parse(checkpoint()); padded.arrays.s=padded.arrays.s.slice(0,-3)+'B==';
    assert.throws(() => createFluidSnapshot(JSON.stringify(padded)),/pad bits/);
    assert.throws(() => createFluidSnapshot(checkpoint({edit:a=>{a.bank[0]=2;}})),/Boolean/);
    assert.throws(() => createFluidSnapshot(checkpoint({edit:a=>{a.bank[24+12]=1;}})),/exclusion/);
    assert.throws(() => createFluidSnapshot(checkpoint({edit:a=>{a.sc[0]=9;}})),/relation/);
    const reader=createFluidSnapshot(checkpoint());
    for(const width of ['0','3','8','01',1]) assert.throws(()=>reader.observe(width));
});

test('actual absorption records give signed source momentum and positive relation transfer', () => {
    const source=summarizeFluidAdvance(advanceResult('12',[[0,48,0,0,0,0],[63,167,1,62,2,1]]),'4');
    assert.deepEqual(source,{status:'available',from_microtick:'8',to_microtick:'12',microticks:'4',
        transferred_tokens:'2',tokens_by_polarity:['-1','-1'],momentum_by_polarity:[['-1','0','0'],['0','1','1']]});
    assert.deepEqual(createFluidSnapshot(checkpoint({tick:'12'})).observe('2',source).source,source);
    assert.throws(()=>createFluidSnapshot(checkpoint()).observe('2',source),/stale/);
    for(const row of [[0,0,0,0,0,0],[64,48,0,0,0,0],[0,48,0,0,0,1],[0,48,2,0,0,0],[0,48.5,0,0,0,0],[false,48,0,0,0,0]]) {
        assert.throws(()=>summarizeFluidAdvance(advanceResult('12',[row]),'4'));
    }
    assert.throws(()=>summarizeFluidAdvance(advanceResult('2'),'4'),/clock/);
    assert.throws(()=>summarizeFluidAdvance(advanceResult('4'),'0'),/interval/);
});

function fakeCompiled() {
    const counters={checkpoints:0,observations:0,deleted:0};
    class HydroState {
        constructor(text) {this.text=text; this.tick=JSON.parse(text).microtick;}
        checkpoint() {++counters.checkpoints; return this.text;}
        observe(width,kind) {++counters.observations; return JSON.stringify({width,kind,microtick:this.tick});}
        diagnostics() {return JSON.stringify(advanceResult(this.tick).diagnostics);}
        advance(ticks) {
            if(ticks==='42') throw new Error('atomic fixture failure');
            this.tick=String(BigInt(this.tick)+BigInt(ticks));
            const parsed=JSON.parse(this.text); parsed.microtick=this.tick; this.text=JSON.stringify(parsed);
            return JSON.stringify(advanceResult(this.tick));
        }
        restore(text) {if(text==='bad') throw new Error('atomic restore failure'); this.text=text;this.tick=JSON.parse(text).microtick;}
        delete() {++counters.deleted;}
    }
    return {module:{HydroState},counters};
}

test('adapter cache reads once per state and never snapshots legacy observations', () => {
    const f=fakeCompiled(), adapter=createWasmHydroAdapter(f.module,checkpoint());
    adapter.observe('1','fields'); adapter.observe('1','counts');
    assert.equal(f.counters.checkpoints,0);
    adapter.observe('1','fluid'); adapter.observe('2','fluid');
    assert.equal(f.counters.checkpoints,1); assert.equal(f.counters.observations,2);
    adapter.advance('0'); adapter.observe('1','fluid');
    assert.equal(f.counters.checkpoints,1);
    assert.throws(()=>adapter.advance('42'));
    assert.throws(()=>adapter.restore('bad'));
    adapter.observe('2','fluid'); assert.equal(f.counters.checkpoints,1);
    adapter.advance('4');
    const next=adapter.observe('1','fluid');
    assert.equal(f.counters.checkpoints,2); assert.equal(next.source.status,'available');
    assert.equal(next.source.to_microtick,'4');
    adapter.restore(checkpoint({tick:'8'}));
    assert.equal(adapter.observe('2','fluid').source.status,'unavailable');
    assert.equal(f.counters.checkpoints,3);
    adapter.dispose();adapter.dispose();assert.equal(f.counters.deleted,1);
    assert.throws(()=>adapter.observe('1','fluid'),/disposed/);
});

test('fluid publication retains protocol owner/generation and rejects stale work', async () => {
    const f=fakeCompiled(), adapter=createWasmHydroAdapter(f.module,checkpoint());
    const protocol=createHydroWorkerProtocol(adapter,{ownerId:'fluid-owner'});
    const req=(op,generation,fields={})=>({requestId:`${op}-${generation}`,op,generation,ownerId:'fluid-owner',...fields});
    const first=await protocol.dispatch(req('observe','0',{width:'2',observable:'fluid'}));
    assert.equal(first.ownerId,'fluid-owner');assert.equal(first.generation,'0');
    await protocol.dispatch(req('advance','0',{microticks:'4'}));
    await assert.rejects(protocol.dispatch(req('observe','0',{width:'2',observable:'fluid'})),/stale/);
    const next=await protocol.dispatch(req('observe','1',{width:'2',observable:'fluid'}));
    assert.equal(next.payload.microtick,'4');assert.equal(next.generation,'1');
    assert.equal(f.counters.checkpoints,2);
});

test('corrupt source observations fail closed without relabeling a committed advance as rolled back', async () => {
    const f=fakeCompiled(), Base=f.module.HydroState;
    f.module.HydroState=class extends Base {
        advance(ticks) {
            const returned=JSON.parse(super.advance(ticks));
            returned.events.absorptions=[[0,0,0,0,0,0]]; // invalid passive phase
            return JSON.stringify(returned);
        }
    };
    const protocol=createHydroWorkerProtocol(createWasmHydroAdapter(f.module,checkpoint()),{ownerId:'owner'});
    const req=(op,generation,fields={})=>({requestId:op,op,generation,ownerId:'owner',...fields});
    const advanced=await protocol.dispatch(req('advance','0',{microticks:'4'}));
    assert.equal(advanced.generation,'1');
    await assert.rejects(protocol.dispatch(req('observe','1',{width:'2',observable:'fluid'})),/phase two/);
    assert.equal(f.counters.checkpoints,0);
    const diagnostics=await protocol.dispatch(req('diagnostics','1'));
    assert.equal(diagnostics.payload.microtick,'4');
});

test('explicit diagnostics reply preserves default events and the adapter source observation', async () => {
    const responses = [];
    for (const publication of [undefined, 'diagnostics']) {
        const f = fakeCompiled(), Base = f.module.HydroState;
        f.module.HydroState = class extends Base {
            advance(ticks) {
                const result = JSON.parse(super.advance(ticks));
                result.events.absorptions = [[0,48,0,0,0,0]];
                return JSON.stringify(result);
            }
        };
        const protocol = createHydroWorkerProtocol(createWasmHydroAdapter(f.module, checkpoint()), {ownerId:'owner'});
        const req = (op,generation,fields={}) => ({requestId:op,op,generation,ownerId:'owner',...fields});
        const advanced = await protocol.dispatch(req('advance','0',{microticks:'4',publication}));
        assert.equal(advanced.ownerId,'owner'); assert.equal(advanced.generation,'1');
        const fluid = await protocol.dispatch(req('observe','1',{width:'2',observable:'fluid'}));
        assert.equal(fluid.payload.microtick,'4');
        assert.equal(fluid.payload.source.status,'available');
        assert.equal(fluid.payload.source.transferred_tokens,'1');
        assert.deepEqual(fluid.payload.source.tokens_by_polarity,['-1','0']);
        assert.deepEqual(fluid.payload.source.momentum_by_polarity,[['-1','0','0'],['0','0','0']]);
        responses.push({advanced,fluid});
        await assert.rejects(protocol.dispatch(req('advance','0',{microticks:'4',publication})),/stale generation/);
        await assert.rejects(protocol.dispatch(req('advance','1',{ownerId:'foreign',microticks:'4',publication})),/foreign owner/);
    }
    assert.deepEqual(responses[0].advanced.payload.events.absorptions,[['0','48','0','0','0','0']]);
    assert.deepEqual(Object.keys(responses[0].advanced.payload),['diagnostics','events']);
    assert.deepEqual(responses[1].advanced.payload,{diagnostics:responses[0].advanced.payload.diagnostics});
    assert.deepEqual(responses[0].fluid,responses[1].fluid);
});

test('invalid advance publication selectors reject before any adapter mutation', async () => {
    const f = fakeCompiled(); let calls = 0;
    const adapter = createWasmHydroAdapter(f.module,checkpoint()), advance = adapter.advance;
    adapter.advance = ticks => { calls++; return advance(ticks); };
    const protocol = createHydroWorkerProtocol(adapter,{ownerId:'owner'});
    const req = (op,generation,fields={}) => ({requestId:op,op,generation,ownerId:'owner',...fields});
    for (const publication of [null,false,0,'','full','Diagnostics',[],{}]) {
        await assert.rejects(protocol.dispatch(req('advance','0',{microticks:'4',publication})),/publication/);
    }
    assert.equal(calls,0);
    assert.equal((await protocol.dispatch(req('diagnostics','0'))).payload.microtick,'0');
    const zero = await protocol.dispatch(req('advance','0',{microticks:'0',publication:'diagnostics'}));
    assert.equal(zero.generation,'0'); assert.equal(zero.payload.diagnostics.microtick,'0');
    assert.equal(calls,1);
});
