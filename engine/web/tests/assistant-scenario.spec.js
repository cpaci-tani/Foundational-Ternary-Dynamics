import {test,expect} from '@playwright/test';
import {writeFile} from 'node:fs/promises';
import {gotoAndReady,openObserverWorkspace} from './_helpers.js';

async function ready(page){
    await gotoAndReady(page,{path:'/?engine=wasm&lattice=9'});
    await page.waitForFunction(()=>window.__FTD_DEV__?.registry.get('assistant')?.control.observe()?.capabilities.includes('lattice.seed.describe'));
    await page.evaluate(async()=>{
        const ai=window.__FTD_DEV__.registry.get('assistant');
        await ai.control.execute({type:'lattice.pause',args:{}},{expected:ai.control.observe(),signal:new AbortController().signal,assertActive(){}});
        ai.jev.setKey('fixture-not-sent');
        window.__scenarioDecisions=[];
        ai.jev.evaluate=async request=>{window.__scenarioDecisions.push(structuredClone(request));return{decision:'execute',confidence:1,model:'offline-decision-fixture'};};
    });
}

test('complete scenario dropdown and read-only template preview follow Scale 0 workspace',async({page},testInfo)=>{
    await ready(page);
    const recordsResponse=await page.request.get('/api/lattice/records/catalog');
    expect(recordsResponse.ok(),await recordsResponse.text()).toBe(true);
    const records=await recordsResponse.json();
    await expect.poll(()=>page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').service.scenarioCatalog().filter(row=>row.backend==='finite-records').length)).toBe(records.scenarios.length);
    await page.getByRole('tab',{name:'JEV',exact:true}).click();
    await page.getByText('Scenario experiments · start from empty',{exact:true}).click();
    await expect(page.locator('[data-scenario="ticks"]')).toHaveValue('10000');
    await expect(page.locator('[data-scenario="ticks"]')).toHaveAttribute('max','100000');
    await expect(page.locator('[data-scenario="interval"]')).toHaveValue('250');
    await expect(page.locator('.jev-scenario-panel')).toContainText('JEV approves the complete fixed protocol once');
    const catalog=await page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').service.scenarioCatalog());
    expect(catalog.length).toBeGreaterThan(100);
    expect(await page.locator('#jev-scenario-select option').count()).toBe(catalog.length+2);
    await page.locator('#jev-scenario-select').selectOption('flux-pulse');
    const before=await page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').control.observe());
    await page.getByRole('button',{name:'View settings template',exact:true}).click();
    await expect(page.locator('[data-scenario="status"]')).toHaveText('Template loaded. The live lattice has not changed.');
    const after=await page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').control.observe());
    expect(after.ownerId).toBe(before.ownerId);expect(after.preparationVersion).toBe(before.preparationVersion);expect(after.tick).toBe(before.tick);
    await expect(page.locator('[data-scenario="template"]')).toContainText('allowedMeasurements');
    await page.locator('.jev-console').evaluate(el=>{el.scrollTop=0;});
    await page.screenshot({path:testInfo.outputPath('jev-scenario-templates.png')});
    await page.locator('[data-scenario="ticks"]').fill('100001');
    await expect(page.getByRole('button',{name:'Empty lattice & run',exact:true})).toBeDisabled();
    await expect(page.locator('[data-scenario="status"]')).toContainText('exact integer from 1 to 100000');
    await page.locator('[data-scenario="ticks"]').fill('100000');
    await expect(page.locator('[data-scenario="status"]')).toContainText('200 measured intervals');
    await page.locator('[data-scenario="ticks"]').fill('4');
    await page.locator('[data-scenario="interval"]').fill('2');
    await page.getByRole('button',{name:'Empty lattice & run',exact:true}).click();
    await expect(page.locator('[data-scenario="status"]')).toHaveText('Experiment complete.',{timeout:30000});
    await expect(page.locator('[data-scenario="result"]')).toContainText('Completed engine tick: 0 → 4');
    await expect(page.locator('[data-scenario="progress"]')).toHaveJSProperty('value',100);
    await expect(page.locator('[data-scenario="progress-detail"]')).toContainText('4 / 4 ticks');
    await page.screenshot({path:testInfo.outputPath('jev-scenario-dropdown.png')});
    const last=catalog.at(-1);
    await page.locator('#jev-scenario-search').fill(last.id);
    await expect(page.locator('#jev-scenario-select')).toContainText(last.label);
    await page.getByRole('button',{name:'Close JEV console'}).click();
    await openObserverWorkspace(page);
    await page.locator('#observer-workspace').getByRole('button',{name:/JEV/}).click();
    await expect(page.locator('[data-jev="scenario-panel"]')).toBeHidden();
});

test('canned and LLM settings prepare the actual empty WASM lattice and measure exact evolution',async({page},testInfo)=>{
    await ready(page);
    const evidence=await page.evaluate(async()=>{
        const ai=window.__FTD_DEV__.registry.get('assistant');
        const request={scenarioId:'flux-pulse',goal:'Prepare a flux pulse and measure total flux over four ticks.',ticks:4,sampleEvery:2,useLLM:false};
        const canned=await ai.service.runScenarioExperiment(request);
        const template=await ai.service.describeScenario('flux-pulse',new AbortController().signal);
        const property=template.properties.find(p=>/amplitude/.test(p.key)&&p.type==='real');
        if(!property)throw new Error('Flux pulse must expose its actual amplitude setting');
        const value=Math.min(property.max,Math.max(property.min,property.value*0.5));
        const generated=[];
        Object.defineProperty(ai.model,'ready',{get:()=>true,configurable:true});
        ai.model.generate=async messages=>{
            generated.push(JSON.parse(messages.at(-1).content));
            return JSON.stringify({name:'Half amplitude pulse',settings:[{key:property.key,value}],measurements:['totalFlux','manifested']});
        };
        const designed=await ai.service.runScenarioExperiment({...request,useLLM:true,goal:'Use half the default pulse amplitude and measure total flux.'});
        const {getActiveScale0Bridge}=await import('/js/scales/scale0/state/store.js');
        const owner=getActiveScale0Bridge(window.__ftdCtx);
        return{canned,designed,property,value,generated,decisions:window.__scenarioDecisions,
            realWorker:owner?.isWorker&&owner?.isWasm,actualTick:owner.currentTick(),after:ai.control.observe(),remoteJev:'not-tested'};
    });
    const path=testInfo.outputPath('scenario-worker.json');await writeFile(path,JSON.stringify(evidence,null,2));
    await testInfo.attach('scenario-worker.json',{path,contentType:'application/json'});
    for(const result of [evidence.canned,evidence.designed]){
        expect(result.status,result.summary).toBe('complete');
        expect(result.emptyBaseline.facts.totalFlux).toBe(0);
        expect(result.samples.map(row=>Number(row.tick))).toEqual([0,2,4]);
        expect(new Set(result.samples.map(row=>row.ownerId)).size).toBe(1);
        expect(new Set(result.samples.map(row=>row.preparationVersion)).size).toBe(1);
    }
    expect(evidence.realWorker).toBe(true);expect(Number(evidence.actualTick)).toBe(4);expect(evidence.after.facts.running).toBe(false);
    expect(evidence.generated[0].emptyObservation.facts.totalFlux).toBe(0);
    expect(evidence.designed.template.settings).toEqual([{key:evidence.property.key,value:evidence.value,units:evidence.property.units}]);
    expect(evidence.designed.baseline.facts.totalFlux).toBeCloseTo(evidence.canned.baseline.facts.totalFlux*0.5,4);
    expect(evidence.decisions.map(row=>row.observation.facts.scenarioExperiment.authorization.scope)).toEqual(['fixed-protocol','empty-preparation','fixed-protocol']);
    const approved=evidence.decisions.find(row=>row.observation.facts.scenarioExperiment.phase==='protocol'&&row.observation.facts.scenarioExperiment.template.settings.length);
    expect(approved.observation.facts.scenarioExperiment.template).toEqual(evidence.designed.template);
    expect(approved.observation.facts.scenarioExperiment.preparation).toEqual(evidence.designed.template.recipe);
});

test('finite-record preparations execute on the active scenario lattice with exact counts',async({page},testInfo)=>{
    await ready(page);
    await expect.poll(()=>page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').service.scenarioCatalog().some(row=>row.id==='record-sparse'))).toBe(true);
    const evidence=await page.evaluate(async()=>{
        const ai=window.__FTD_DEV__.registry.get('assistant');
        const result=await ai.service.runScenarioExperiment({scenarioId:'record-sparse',goal:'Measure sparse record counts for four ticks.',ticks:4,sampleEvery:2,useLLM:false});
        const {getActiveScale0Bridge}=await import('/js/scales/scale0/state/store.js');
        return{result,actualFinite:getActiveScale0Bridge(window.__ftdCtx)?.isFiniteRecord,after:ai.control.observe(),remoteJev:'not-tested'};
    });
    const path=testInfo.outputPath('scenario-finite-worker.json');await writeFile(path,JSON.stringify(evidence,null,2));
    await testInfo.attach('scenario-finite-worker.json',{path,contentType:'application/json'});
    expect(evidence.result.status,evidence.result.summary).toBe('complete');
    expect(evidence.actualFinite).toBe(true);expect(evidence.result.emptyBaseline.facts.fieldTokens).toBe('0');
    expect(BigInt(evidence.result.baseline.facts.fieldTokens)).toBeGreaterThan(0n);
    expect(evidence.result.samples.map(row=>Number(row.tick))).toEqual([0,2,4]);
    expect(evidence.result.template.measurements).not.toContain('totalFlux');
    expect(evidence.result.template.measurements).not.toContain('dynamicEnergy');
    expect(new Set(evidence.result.samples.map(row=>row.ownerId)).size).toBe(1);
});

test('actual local model designs settings from the empty-lattice template',async({page},testInfo)=>{
    test.skip(process.env.FTD_HARDWARE_WEBGL!=='1','Requires actual WebGPU model inference.');
    test.setTimeout(180000);await ready(page);
    const evidence=await page.evaluate(async()=>{
        const ai=window.__FTD_DEV__.registry.get('assistant');await ai.model.load();
        const generated=[],generate=ai.model.generate.bind(ai.model);
        ai.model.generate=async(...args)=>{const raw=await generate(...args);generated.push(raw);return raw;};
        const result=await ai.service.runScenarioExperiment({scenarioId:'flux-pulse',goal:'Set the pulse amplitude to 0.5. Measure totalFlux and manifested.',ticks:4,sampleEvery:2,useLLM:true});
        const automatic=await ai.service.runScenarioExperiment({scenarioId:'__design__',goal:'Choose Transverse Wave Packet Propagation (flux-pulse). Set packet.amplitude to 0.5 and measure totalFlux.',ticks:4,sampleEvery:2,useLLM:true});
        return{result,automatic,generated,model:ai.model.modelId,decisions:window.__scenarioDecisions,remoteJev:'not-tested'};
    });
    const path=testInfo.outputPath('scenario-real-model.json');await writeFile(path,JSON.stringify(evidence,null,2));
    await testInfo.attach('scenario-real-model.json',{path,contentType:'application/json'});
    expect(evidence.result.status,evidence.result.summary).toBe('complete');
    expect(evidence.result.template.settings.some(row=>/amplitude/.test(row.key)&&row.value===0.5)).toBe(true);
    expect(evidence.result.samples.map(row=>Number(row.tick))).toEqual([0,2,4]);
    expect(evidence.automatic.status,evidence.automatic.summary).toBe('complete');
    expect(evidence.automatic.template.scenarioId).toBe('flux-pulse');
    expect(evidence.automatic.template.settings.some(row=>row.key==='packet.amplitude'&&row.value===0.5)).toBe(true);
});
