// @ts-check
import {LifetimeScope} from '../ui/utils/lifetime-scope.js';
import {SCENARIO_LIMITS,SCENARIO_DEFAULTS,validateScenarioProtocol} from './scenario-limits.js';

/** A selector for registered preparations and bounded LLM-authored variations.
 * Selecting or previewing a template never changes the live lattice.
 */
export class ScenarioExperimentPanel{
    /** @param {HTMLElement} mount @param {{service:any,model:any,jev:any}} deps */
    constructor(mount,deps){
        this.deps=deps;this.scope=new LifetimeScope();this.mount=mount;this.busy=false;this.runId=null;
        this.outcome='';
        /** @type {any} */this.catalogStatus=null;
        /** @type {any[]} */this.catalog=[];
        /** @type {any} */this.observation=null;
        /** @type {any} */this.artifact=null;
        /** @type {AbortController|null} */this.preview=null;
        mount.innerHTML=`<details class="jev-scenario-panel"><summary>Scenario experiments · start from empty</summary>
<p>Start with an empty lattice, apply a canned or LLM-designed preparation, then measure its evolution. Running replaces the current preparation and finishes paused.</p>
<label for="jev-scenario-search">Find a scenario</label><input id="jev-scenario-search" type="search" placeholder="Wave, collision, gravity, finite records…">
<label for="jev-scenario-select">Experiment scenario</label><select id="jev-scenario-select"></select>
<p data-scenario="coverage" class="jev-context"></p><p data-scenario="description"></p>
<label class="jev-check"><input type="checkbox" data-scenario="custom"> Let the LLM choose settings</label>
<label for="jev-scenario-goal">Experiment goal</label><textarea id="jev-scenario-goal" rows="2" maxlength="2000" placeholder="Describe the preparation and what you want to measure…"></textarea>
<div class="jev-scenario-budget"><label>Ticks <input type="number" data-scenario="ticks" min="1" max="${SCENARIO_LIMITS.ticks}" value="${SCENARIO_DEFAULTS.ticks}"></label><label>Sample every <input type="number" data-scenario="interval" min="1" max="${SCENARIO_DEFAULTS.ticks}" value="${SCENARIO_DEFAULTS.sampleEvery}"></label></div>
<small>Up to ${SCENARIO_LIMITS.ticks.toLocaleString('en-US')} ticks, ${SCENARIO_LIMITS.intervals} measured intervals plus the baseline, and ${SCENARIO_LIMITS.durationMs/60000} minutes. JEV approves the complete fixed protocol once. Progress stays live; Stop AI cancels subsequent work. Only settings exposed by the preparation template can change.</small>
<div class="jev-buttons"><button type="button" data-scenario="preview">View settings template</button><button type="button" data-scenario="run">Empty lattice &amp; run</button></div>
<p data-scenario="status" role="status" aria-live="polite"></p>
<progress data-scenario="progress" max="100" value="0" aria-label="Experiment tick progress"></progress><p data-scenario="progress-detail" class="jev-context" aria-live="polite">No active experiment.</p>
<details><summary>Scenario &amp; settings template</summary><pre data-scenario="template">Choose a scenario to inspect its template. The approved proposal will appear here before it is applied.</pre><button type="button" data-scenario="export" disabled>Export scenario template</button></details>
<details data-scenario="results"><summary>Measured results</summary><pre data-scenario="result">No experiment has run.</pre></details></details>`;
        this.select=/** @type {HTMLSelectElement} */(mount.querySelector('#jev-scenario-select'));
        this.search=/** @type {HTMLInputElement} */(mount.querySelector('#jev-scenario-search'));
        this.goal=/** @type {HTMLTextAreaElement} */(mount.querySelector('#jev-scenario-goal'));
        this.scope.on(this.search,'input',()=>this.renderOptions());
        this.scope.on(this.select,'change',()=>{this.preview?.abort();this.describeSelection();});
        this.scope.on(this.node('custom'),'change',()=>this.updateAvailability());
        this.scope.on(this.node('ticks'),'input',()=>{
            const ticks=Number(this.input('ticks').value);
            this.input('interval').max=String(Math.max(1,ticks));
            this.updateAvailability();
        });
        this.scope.on(this.node('interval'),'input',()=>this.updateAvailability());
        this.scope.on(mount,'click',event=>{
            const target=event.target instanceof Element?event.target.closest('button'):null;
            if(target?.dataset.scenario==='run')void this.run();
            if(target?.dataset.scenario==='preview')void this.showTemplate();
            if(target?.dataset.scenario==='export')this.export();
        });
        this.scope.defer(()=>this.preview?.abort());
    }
    /** @param {string} name @returns {HTMLElement} */
    node(name){return /** @type {HTMLElement} */(this.mount.querySelector(`[data-scenario="${name}"]`));}
    /** @param {string} name @returns {HTMLInputElement} */input(name){return /** @type {HTMLInputElement} */(this.node(name));}
    /** @param {any} observation */
    refresh(observation=this.deps.service.currentObservation()){
        this.observation=observation;this.mount.hidden=observation?.workspace!=='lattice';
        if(this.mount.hidden){this.preview?.abort();return;}
        this.catalog=this.deps.service.scenarioCatalog();this.renderOptions();
    }
    /** @param {any} row */
    unavailable(row){
        if(this.observation?.facts.backend==='Native'&&row.backend==='finite-records')return 'Requires the local finite-record backend';
        if(row.sizes?.length && !row.sizes.includes(this.observation?.facts.latticeSize))return `Choose lattice size ${row.sizes.join(', ')} first`;
        return '';
    }
    renderOptions(){
        const selected=this.select.value,query=this.search.value.trim().toLowerCase();
        this.select.replaceChildren(new Option('Choose a scenario…',''),new Option('LLM-designed scenario (automatic template choice)','__design__'));
        const groups=new Map();let visible=0,available=0;
        for(const row of this.catalog){
            if(!this.unavailable(row))available++;
            if(query && !`${row.id} ${row.label} ${row.category} ${row.description||''}`.toLowerCase().includes(query))continue;
            const groupLabel=String(row.category||'Other preparations');
            if(!groups.has(groupLabel)){const group=document.createElement('optgroup');group.label=groupLabel;groups.set(groupLabel,group);this.select.append(group);}
            const reason=this.unavailable(row),option=new Option(`${row.label||row.id}${reason?' — unavailable':''}`,row.id);
            option.disabled=!!reason;option.title=reason;groups.get(groupLabel).append(option);visible++;
        }
        if([...this.select.options].some(option=>option.value===selected))this.select.value=selected;
        if(this.select.value!==selected)this.preview?.abort();
        const status=this.catalogStatus;
        const coverage=status?.status==='ready'?`Finite catalog ready (${status.registeredCount} preparations).`
            :status?.status==='unavailable'?`Finite catalog unavailable: ${status.reason||'local runtime required'}.`
            :status? 'Finite catalog is loading; more preparations may appear.':'';
        this.node('coverage').textContent=`${visible} / ${this.catalog.length} registered scenarios shown · ${available} match this backend and size. ${coverage}`;
        this.describeSelection();
    }
    describeSelection(){
        const id=this.select.value,row=this.catalog.find(item=>item.id===id);
        this.node('description').textContent=id==='__design__'?'Matching templates are found across the registered catalog. The LLM selects a supported template and chooses legal settings after observing the empty lattice.':row?`${row.description||row.label} (${row.backend})`:'';
        if(id==='__design__')this.input('custom').checked=true;
        this.input('custom').disabled=this.busy||id==='__design__';this.updateAvailability();
    }
    updateAvailability(){
        const selected=!!this.select.value;
        const ready=this.observation?.capabilities.includes('lattice.seed.describe')&&this.observation?.capabilities.includes('lattice.seed.preview');
        const needsModel=this.input('custom').checked||this.select.value==='__design__';
        let protocolError='';
        try{validateScenarioProtocol(Number(this.input('ticks').value),Number(this.input('interval').value));}
        catch(error){protocolError=error instanceof Error?error.message:'Invalid experiment protocol.';}
        const reason=!ready?'This Scale 0 owner does not expose editable scenario templates.':!selected?'Choose an experiment scenario.':this.deps.jev.connected===false?'Connect a JEV key to approve preparation and experiment operations.':needsModel&&this.deps.model.ready===false?'Load the local model to design a scenario.':protocolError;
        /** @type {HTMLButtonElement} */(this.node('run')).disabled=this.busy||!!reason;
        /** @type {HTMLButtonElement} */(this.node('preview')).disabled=this.busy||!ready||!selected||this.select.value==='__design__';
        if(!this.busy)this.node('status').textContent=reason||this.outcome||'Ready. Starting will replace the current preparation.';
    }
    async showTemplate(){
        this.preview?.abort();const controller=new AbortController();this.preview=controller;const scenarioId=this.select.value;
        this.node('status').textContent='Reading the constructor settings template…';
        try{
            const template=await this.deps.service.describeScenario(scenarioId,controller.signal);
            controller.signal.throwIfAborted();if(this.preview!==controller || this.select.value!==scenarioId)return;
            this.artifact={schemaVersion:1,kind:'scenario-settings-template',template};
            this.showArtifact();this.node('status').textContent='Template loaded. The live lattice has not changed.';
        }catch(error){if(!controller.signal.aborted)this.node('status').textContent=error instanceof Error?error.message:'Template unavailable.';}
    }
    async run(){
        this.preview?.abort();
        const row=this.catalog.find(item=>item.id===this.select.value);
        const goal=this.goal.value.trim()||(row?`Explore ${row.label||row.id} and compare the recorded measurements.`:'');
        if(!goal){this.node('status').textContent='Describe the scenario you want the LLM to create.';this.goal.focus();return;}
        let protocol;
        try{protocol=validateScenarioProtocol(Number(this.input('ticks').value),Number(this.input('interval').value));}
        catch(error){this.node('status').textContent=error instanceof Error?error.message:'Invalid experiment protocol.';this.input('interval').focus();return;}
        await this.deps.service.runScenarioExperiment({scenarioId:this.select.value,goal,ticks:protocol.ticks,sampleEvery:protocol.sampleEvery,useLLM:this.input('custom').checked||this.select.value==='__design__'});
    }
    showArtifact(){this.node('template').textContent=JSON.stringify(this.artifact,null,2);/** @type {HTMLButtonElement} */(this.node('export')).disabled=false;}
    /** @param {any} event */
    event(event){
        if(event.type==='scenario-catalog'){this.catalogStatus=event.status;this.refresh();return;}
        if(event.type==='state'){
            this.busy=!!event.busy;this.select.disabled=this.busy;this.search.disabled=this.busy;
            this.goal.disabled=this.busy;this.input('ticks').disabled=this.busy;this.input('interval').disabled=this.busy;
            this.input('custom').disabled=this.busy||this.select.value==='__design__';this.updateAvailability();return;
        }
        if(event.type==='scenario-stage'&&event.phase==='starting'){
            this.runId=event.scenarioRun;this.outcome='';this.artifact=null;
            /** @type {HTMLProgressElement} */(this.node('progress')).value=0;
            this.node('progress-detail').textContent='Preparing the fixed experiment protocol…';
            this.node('template').textContent='Preparing the requested scenario template…';
            this.node('result').textContent='The experiment is in progress.';
            /** @type {HTMLButtonElement} */(this.node('export')).disabled=true;
        }
        if(event.scenarioRun!==this.runId)return;
        if(event.type==='scenario-progress' && Number.isFinite(event.completedTicks) && Number.isFinite(event.totalTicks) && event.totalTicks>0){
            const percent=Math.max(0,Math.min(100,100*event.completedTicks/event.totalTicks));
            /** @type {HTMLProgressElement} */(this.node('progress')).value=percent;
            this.node('progress-detail').textContent=`${event.completedTicks.toLocaleString('en-US')} / ${event.totalTicks.toLocaleString('en-US')} ticks · ${percent.toFixed(1)}% · ${event.sampleCount??0} samples · ${event.approvalCount??0} JEV approval${event.approvalCount===1?'':'s'}`;
        }
        if(event.type==='scenario-stage')this.node('status').textContent=event.text;
        if(event.type==='scenario-template'){this.artifact={...event.template,allowedSettings:event.properties};this.showArtifact();}
        if(event.type==='scenario-sample')this.node('status').textContent=`${event.phase}: completed tick ${event.sample.tick} · sample ${event.sample.facts.sampleTick??event.sample.tick}`;
        if(event.type==='scenario-result'){
            const result=event.result;
            this.node('result').textContent=`${result.status.toUpperCase()}\n${result.summary}\n\n${JSON.stringify(result.samples.map((/** @type {any} */ sample)=>({tick:sample.tick,sampleTick:sample.facts.sampleTick,values:Object.fromEntries((result.template?.measurements||[]).map((/** @type {string} */ key)=>[key,sample.facts[key]??'unavailable']))})),null,2)}`;
            /** @type {HTMLDetailsElement} */(this.node('results')).open=true;
            this.outcome=`Experiment ${result.status}.`;this.node('status').textContent=this.outcome;
            if(this.artifact)this.artifact={...this.artifact,result};
        }
    }
    export(){
        if(!this.artifact)return;const url=URL.createObjectURL(new Blob([JSON.stringify(this.artifact,null,2)],{type:'application/json'}));
        const link=document.createElement('a');link.href=url;link.download='jev-scenario-experiment.json';link.click();this.scope.timeout(()=>URL.revokeObjectURL(url),1000);
    }
    dispose(){this.scope.dispose();this.mount.replaceChildren();}
}
