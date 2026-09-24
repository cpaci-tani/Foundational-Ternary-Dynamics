import {test} from 'node:test';
import assert from 'node:assert/strict';
import {BrowserModel} from '../js/assistant/model.js';

test('explanation keeps each controlling claim status within its passage budget',async()=>{
    const model=new BrowserModel({modelBase:'./'});
    let payload;
    model.generate=async messages=>{payload=JSON.parse(messages[1].content);return 'Interpretation';};
    const companions=Array.from({length:6},(_,i)=>({claimId:`FTD-${1000+i}`,available:true,
        sourcePath:'docs/theory/07_assessment/core_ledgers/LEDGER.md',statusTags:[i===5?'RETRACTED':'OPEN'],text:'Status evidence '.repeat(80)}));
    await model.explain('Explain these claims.',{workspace:'observer',ownerId:'test',preparationVersion:0,tick:0,
        capabilities:[],facts:{},availability:{}},[{sourcePath:'historical.md',statusTags:['CONJECTURE'],text:'Historical claims',
        authoritativeCompanions:companions,authoritativeLookup:{checked:6,total:6,truncated:false}}],new AbortController().signal,()=>{});
    const source=payload.sources[0];
    assert.equal(source.authoritativeCompanions.length,6);
    assert.deepEqual(source.authoritativeCompanions[5].status,['RETRACTED']);
    assert.equal(source.authoritativeCompanions[5].claimId,'FTD-1005');
    assert.ok(source.authoritativeCompanions.every(c=>c.passageTruncated));
    assert.ok(source.authoritativeCompanions.reduce((sum,c)=>sum+c.text.length,0)<=400);
    assert.equal(source.authorityCoverage.suppliedCompanions,6);
    assert.equal(source.authorityCoverage.omittedCompanions,0);
});
