import {test,expect} from '@playwright/test';
import {gotoAndReady} from './_helpers.js';
import {randomUUID} from 'node:crypto';
import {fileURLToPath} from 'node:url';
import {Client} from '../../mcp/node_modules/@modelcontextprotocol/sdk/dist/esm/client/index.js';
import {StdioClientTransport} from '../../mcp/node_modules/@modelcontextprotocol/sdk/dist/esm/client/stdio.js';

test('external MCP relay controls the actual open lattice without loading an SLM and revokes on Stop AI',async({page,request})=>{
    await page.route('**/api/ai/jev',route=>route.fulfill({json:{decision:'execute',confidence:1,model:'browser-fixture'}}));
    await gotoAndReady(page,{path:'/?engine=wasm&lattice=9'});
    await page.waitForFunction(()=>window.__FTD_DEV__.registry.get('assistant'));
    await page.getByRole('button',{name:'Open the JEV console',exact:true}).click();
    await page.locator('[data-jev="mcp-panel"] > summary').click();
    await page.getByRole('button',{name:'Enable MCP',exact:true}).click();
    await expect(page.locator('[data-jev="mcp-status"]')).toContainText('enabled');
    const config=JSON.parse(await page.locator('[data-jev="mcp-config"]').inputValue()).mcpServers.ftd;
    const client=new Client({name:'ftd-browser-regression',version:'1.0.0'});
    await client.connect(new StdioClientTransport({command:process.execPath,args:[fileURLToPath(new URL('../../mcp/server.mjs',import.meta.url))],env:{...process.env,...config.env}}));
    try{
    expect((await client.listTools()).tools).toHaveLength(8);
    const headers={Authorization:`Bearer ${config.env.FTD_MCP_TOKEN}`};
    const call=async(method,args={},id=randomUUID())=>{
        const response=await request.post('/api/mcp/call',{headers,data:{sessionId:config.env.FTD_MCP_SESSION,id,method,args}});
        expect(response.ok()).toBe(true);return response.json();
    };
    const initial=(await client.callTool({name:'ftd_observe',arguments:{}})).structuredContent.result.observation;
    expect(initial.workspace).toBe('lattice');
    expect((await call('list_scenarios',{limit:3})).result.items).toHaveLength(3);
    await page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').jev.setKey('synthetic-mcp-key'));
    const expected={workspace:initial.workspace,ownerId:initial.ownerId,preparationVersion:initial.preparationVersion};
    const args={intent:'Pause and advance exactly two ticks',expected,actions:[{type:'lattice.pause',args:{}},{type:'lattice.step',args:{count:2}}]},id=randomUUID();
    const applied=(await call('execute_plan',args,id)).result;
    expect(applied.status).toBe('applied');expect(applied.receipts).toHaveLength(2);
    expect(Number(applied.receipts[1].after.tick)-Number(applied.receipts[0].after.tick)).toBe(2);
    expect(applied.observation.ownerId).toBe(initial.ownerId);
    expect((await call('execute_plan',args,id)).result).toEqual(applied);
    expect((await call('observe')).result.observation.tick).toBe(applied.observation.tick);
    const sdkResult=(await client.callTool({name:'ftd_execute_plan',arguments:{intent:'Advance one more tick',expected:{...expected,preparationVersion:applied.observation.preparationVersion},actions:[{type:'lattice.step',args:{count:1}}]}})).structuredContent.result;
    expect(sdkResult.status).toBe('applied');expect(Number(sdkResult.observation.tick)).toBe(Number(applied.observation.tick)+1);
    expect(await page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').model.ready)).toBe(false);
    await page.getByRole('button',{name:'Stop AI',exact:true}).click();
    await expect(page.locator('[data-jev="mcp-config"]')).toHaveValue('');
    const revoked=await request.post('/api/mcp/call',{headers,data:{sessionId:config.env.FTD_MCP_SESSION,id:randomUUID(),method:'observe',args:{}}});
    expect(revoked.ok()).toBe(false);
    expect(await page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').control.observe().tick)).toBe(sdkResult.observation.tick);
    }finally{await client.close();}
});

test('fixed MCP protocol reaches 20000 real WASM ticks with one JEV approval and stops a longer run',async({page})=>{
    test.setTimeout(240000);
    const decisions=[];
    await page.route('**/api/ai/jev',route=>{decisions.push(route.request().postDataJSON());return route.fulfill({json:{decision:'execute',confidence:1,model:'browser-fixture'}});});
    await gotoAndReady(page,{path:'/?engine=wasm&lattice=9'});
    await page.waitForFunction(()=>window.__FTD_DEV__.registry.get('assistant'));
    await page.getByRole('button',{name:'Open the JEV console',exact:true}).click();
    await page.getByText('Connection & local model',{exact:true}).click();
    await page.getByLabel('JEV API key',{exact:true}).fill('synthetic-long-run-key');
    await page.getByRole('button',{name:'Connect key',exact:true}).click();
    await page.locator('[data-jev="mcp-panel"] > summary').click();
    await page.getByRole('button',{name:'Enable MCP',exact:true}).click();
    await expect(page.locator('[data-jev="mcp-status"]')).toContainText('enabled');
    const config=JSON.parse(await page.locator('[data-jev="mcp-config"]').inputValue()).mcpServers.ftd;
    const client=new Client({name:'ftd-long-run-regression',version:'1.0.0'});
    await client.connect(new StdioClientTransport({command:process.execPath,args:config.args,env:{...process.env,...config.env}}));
    const tool=async(name,args={})=>{
        const reply=await client.callTool({name,arguments:args});
        expect(reply.isError,JSON.stringify(reply.structuredContent)).not.toBe(true);
        return reply.structuredContent.result;
    };
    const start=async(ticks,sampleEvery)=>{
        const {observation,scenarioLimits}=await tool('ftd_observe');
        expect(scenarioLimits.ticks).toBe(100000);
        return tool('ftd_run_scenario',{scenarioId:'flux-pulse',goal:`Measure the registered packet for exactly ${ticks} ticks using its default preparation.`,ticks,sampleEvery,
            expected:{workspace:observation.workspace,ownerId:observation.ownerId,preparationVersion:observation.preparationVersion}});
    };
    try{
        const run=await start(20000,2000);let status;
        await expect.poll(async()=>{status=await tool('ftd_run_status',{runId:run.runId});return status.status;},{timeout:180000,intervals:[1000,2000]}).not.toBe('running');
        expect(status.status,status.result.summary).toBe('complete');
        expect(decisions).toHaveLength(1);
        expect(status.result.emptyBaseline.facts.totalFlux).toBe(0);
        expect(status.result.samples.map(row=>Number(row.tick))).toEqual(Array.from({length:11},(_,i)=>i*2000));
        expect(new Set(status.result.samples.map(row=>row.ownerId)).size).toBe(1);
        const steps=status.result.receipts.filter(row=>row.action.type==='lattice.step');
        expect(steps.reduce((sum,row)=>sum+row.action.args.count,0)).toBe(20000);
        expect(steps.every(row=>row.action.args.count<=128)).toBe(true);
        expect((await tool('ftd_observe')).observation.facts.running).toBe(false);

        const longer=await start(100000,10000);
        await expect.poll(async()=>{const active=await tool('ftd_run_status',{runId:longer.runId});return active.progress?.completedTicks??0;},{timeout:20000,intervals:[250,500]}).toBeGreaterThanOrEqual(128);
        await tool('ftd_stop');
        await expect.poll(async()=>{status=await tool('ftd_run_status',{runId:longer.runId});return status.status;},{timeout:10000}).not.toBe('running');
        expect(status.status).toBe('stopped');
        const stopped=await tool('ftd_observe');
        expect(Number(stopped.observation.tick)).toBeLessThan(100000);
        expect(stopped.observation.facts.running).toBe(false);
        await new Promise(resolve=>setTimeout(resolve,100));
        expect((await tool('ftd_observe')).observation.tick).toBe(stopped.observation.tick);
        expect(decisions).toHaveLength(2);
    }finally{await client.close();}
});

test('MCP supplied settings run an empty-lattice experiment and text Stop AI revokes pairing',async({page})=>{
    const decisions=[];
    await page.route('**/api/ai/jev',route=>{decisions.push(route.request().postDataJSON());return route.fulfill({json:{decision:'execute',confidence:1,model:'browser-fixture'}});});
    await gotoAndReady(page,{path:'/?engine=wasm&lattice=9'});
    await page.waitForFunction(()=>window.__FTD_DEV__.registry.get('assistant'));
    await page.getByRole('button',{name:'Open the JEV console',exact:true}).click();
    await page.locator('[data-jev="mcp-panel"] > summary').click();
    await page.getByRole('button',{name:'Enable MCP',exact:true}).click();
    await expect(page.locator('[data-jev="mcp-status"]')).toContainText('enabled');
    const config=JSON.parse(await page.locator('[data-jev="mcp-config"]').inputValue()).mcpServers.ftd;
    const client=new Client({name:'ftd-experiment-regression',version:'1.0.0'});
    await client.connect(new StdioClientTransport({command:process.execPath,args:config.args,env:{...process.env,...config.env}}));
    try{
        const tool=async(name,args={})=>{
            const response=await client.callTool({name,arguments:args});expect(response.isError,JSON.stringify(response.structuredContent)).not.toBe(true);return response.structuredContent.result;
        };
        await page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').jev.setKey('synthetic-mcp-key'));
        const template=await tool('ftd_describe_scenario',{scenarioId:'flux-pulse'});
        const property=template.properties.find(row=>/amplitude/.test(row.key)&&row.type==='real');expect(property).toBeTruthy();
        const observation=(await tool('ftd_observe')).observation;
        const goal='Prepare a half-amplitude pulse and measure four completed ticks.';
        const draft={name:'External pulse preparation',goal,settings:[{key:property.key,value:property.value*.5}],ticks:4,sampleEvery:2,measurements:['totalFlux','manifested']};
        const started=await tool('ftd_run_scenario',{goal,scenarioId:'flux-pulse',ticks:4,sampleEvery:2,draft,expected:{workspace:observation.workspace,ownerId:observation.ownerId,preparationVersion:observation.preparationVersion}});
        let status;
        await expect.poll(async()=>{status=await tool('ftd_run_status',{runId:started.runId});return status.status;},{timeout:30000,intervals:[100,250,500]}).not.toBe('running');
        expect(status.status,status.result.summary).toBe('complete');
        expect(status.result.emptyBaseline.facts.totalFlux).toBe(0);
        expect(status.result.samples.map(row=>Number(row.tick))).toEqual([0,2,4]);
        expect(new Set(status.result.samples.map(row=>row.ownerId)).size).toBe(1);
        expect(status.result.template.settings[0].value).toBe(property.value*.5);
        expect(status.result.receipts.filter(row=>row.action.type==='lattice.step')).toHaveLength(2);
        expect(decisions.some(row=>row.observation.facts.scenarioExperiment?.template.settings[0]?.value===property.value*.5)).toBe(true);
        expect(await page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').model.ready)).toBe(false);
        await page.locator('#jev-input').fill('Stop AI');await page.getByRole('button',{name:'Send',exact:true}).click();
        await expect(page.locator('[data-jev="mcp-config"]')).toHaveValue('');
        expect((await client.callTool({name:'ftd_observe',arguments:{}})).isError).toBe(true);
    }finally{await client.close();}
});
