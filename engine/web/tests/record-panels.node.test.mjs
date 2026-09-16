import {test} from 'node:test';
import assert from 'node:assert/strict';
import {getPanelsForScale} from '../js/ui/scale-registry/panel-registry.js';
import {RECORD_PANEL_CONTRACTS, summarizeRecordObservation, recordSlice, appendRecordSample} from '../js/scales/scale0/ui/controls/record-panel-model.js';
const view = () => ({lattice_size:'3',microtick:'7',phase:'3',field_tokens:Array.from({length:27},(_,i)=>String(i)),relation_tokens:Array(27).fill('2'),incidence:Array.from({length:27},(_,i)=>String(i-13)),manifestation_counts:Array.from({length:27},(_,i)=>i===5?['1','0','0']:['0','1','0'])});
test('every Scale-0 sidepanel has an explicit record contract',()=>{
    assert.deepEqual(Object.keys(RECORD_PANEL_CONTRACTS).sort(),getPanelsForScale(0).map(p=>p.id).sort());
});
test('exact signed counts and lagged manifestation remain independent without mutation',()=>{
    const input=view(),before=JSON.stringify(input),s=summarizeRecordObservation(input);
    assert.equal(s.field,'351');assert.equal(s.relation,'54');assert.equal(s.incidence,'0');assert.equal(s.negative,'1');assert.equal(s.zero,'26');assert.equal(s.phase,'Manifestation');
    input.incidence[0]='9007199254740993';assert.equal(summarizeRecordObservation(input).incidence,'9007199254741006');
    input.incidence[0]='-13';assert.equal(JSON.stringify(input),before);
});
test('slice preserves x-major site identity and signed Q at each coordinate',()=>{
    const input=view();
    for(let z=0;z<3;z++)for(let y=0;y<3;y++)for(let x=0;x<3;x++){
        assert.equal(recordSlice(input,'incidence',z)[y][x],String((x*3+y)*3+z-13));
        assert.equal(recordSlice(input,'tokens',z)[y][x],String((x*3+y)*3+z+2));
    }
    assert.throws(()=>recordSlice(input,'energy'));assert.throws(()=>recordSlice(input,'tokens',3));
});
test('history deduplicates read-only observations and bounds retained samples',()=>{
    const history=[];
    for(let t=0;t<150;t++) { const s={tick:String(t)};assert.equal(appendRecordSample(history,s),true);assert.equal(appendRecordSample(history,s),false); }
    assert.equal(history.length,128);assert.equal(history[0].tick,'22');
});
import {isPanelLive} from '../js/ui/panels/panel-visibility.js';
test('record hosts suppress effective instruments while respecting floating collapse',()=>{
    const old=globalThis.document;
    globalThis.document={documentElement:{classList:{contains:()=>false}}};
    let collapsed=false;
    const host={isConnected:true,hidden:false,dataset:{recordObservation:'true'},classList:{contains:()=>true},closest:selector=>selector==='.floating-window'?{hidden:false,classList:{contains:()=>collapsed}}:null};
    try {assert.equal(isPanelLive(host),false);assert.equal(isPanelLive(host,{recordObservation:true}),true);collapsed=true;assert.equal(isPanelLive(host,{recordObservation:true}),false);delete host.dataset.recordObservation;collapsed=false;assert.equal(isPanelLive(host),true);}finally{globalThis.document=old;}
});
