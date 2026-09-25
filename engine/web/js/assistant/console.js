// @ts-check
import {LifetimeScope} from '../ui/utils/lifetime-scope.js';
import {ScenarioExperimentPanel} from './scenario-panel.js';

const MARKUP=`
<header class="jev-header"><div><small>SIMULATION ASSISTANT</small><h2 id="jev-title">JEV console</h2></div><div class="jev-buttons"><button type="button" data-jev="stop">Stop AI</button><button type="button" data-jev="close" aria-label="Close JEV console">×</button></div></header>
<p class="jev-context" data-jev="context">Lattice Sim</p>
<details data-jev="mcp-panel"><summary>Connect your own AI (MCP)</summary>
<p>Use an MCP-capable AI app with your preferred model. No browser model download is needed. The open simulator supplies tools and measurements; JEV evaluates requested changes.</p>
<div class="jev-buttons"><button type="button" data-jev="mcp-enable">Enable MCP</button><button type="button" data-jev="mcp-disconnect">Disconnect MCP</button><button type="button" data-jev="mcp-copy" disabled>Copy MCP config</button></div>
<p role="status" data-jev="mcp-status">Local dashboard server required. Pairing is off by default.</p>
<label data-jev="mcp-config-label" hidden>MCP client configuration<textarea readonly rows="8" data-jev="mcp-config" spellcheck="false"></textarea></label>
<small>Run npm ci in engine/mcp once, then paste this configuration into your AI app. The pairing token controls this tab and expires on disconnect. Stop AI revokes the connection. Keep this tab visible during experiments.</small>
</details>
<div data-jev="scenario-panel" hidden></div>
<details class="jev-setup"><summary>Connection & local model</summary>
<p>Chat runs on this computer. JEV receives your request, proposed actions and a compact observation to evaluate commands. Language interpretation is experimental; inspect the action receipts.</p>
<label>JEV API key <input type="password" autocomplete="off" spellcheck="false" data-jev="key" placeholder="Kept in memory for this tab only"></label>
<div class="jev-buttons"><button type="button" data-jev="connect">Connect key</button><button type="button" data-jev="disconnect">Disconnect</button></div>
<p role="status" data-jev="connection">JEV disconnected · local explanations remain available</p>
<div class="jev-buttons"><button type="button" data-jev="load">Download / load model</button><button type="button" data-jev="unload">Unload</button><button type="button" data-jev="cache">Remove model cache</button></div>
<p role="status" data-jev="model">SmolLM2 360M · about 213 MB download · WebGPU required</p>
<progress data-jev="progress" max="1" value="0" hidden></progress>
<label class="jev-check"><input type="checkbox" data-jev="pause"> Pause while thinking</label>
</details>
<details><summary>Received observations</summary><pre data-jev="facts">No observation yet.</pre></details>
<p class="jev-context" data-jev="knowledge" role="status">Documentation search ready</p>
<p class="jev-context" data-jev="experiment" role="status" hidden></p>
<div class="jev-messages" data-jev="messages" role="log" aria-label="Conversation" aria-live="polite"></div>
<div class="jev-suggestions"><button type="button" data-prompt="Explain the current observation using project documentation.">Explain this view</button><button type="button" data-prompt="Pause and advance 100 ticks.">Step 100 ticks</button><button type="button" data-prompt="Why is the force tether overloaded?">Explain tether</button><button type="button" data-experiment="true" data-prompt="Observe the running simulation, compare two completed measurements, and explain what changed.">Compare live observations</button></div>
<form data-jev="form"><label for="jev-input">Command or question</label><textarea id="jev-input" rows="3" maxlength="4000" placeholder="Make the selected cube twice as heavy…"></textarea>
<label class="jev-check"><input type="checkbox" data-jev="goal"> Live experiment (5 min / 50 actions)</label>
<small>You can also say “Run an experiment to …”. LLM proposes → JEV evaluates → simulator executes → measurements guide the next step. A connected JEV key is required for actions and experiments.</small>
<div class="jev-buttons"><button type="submit" data-jev="send">Send</button><button type="button" data-jev="search">Search docs</button></div></form>
<footer class="jev-buttons"><button type="button" data-jev="clear">Clear conversation</button><button type="button" data-jev="export">Export conversation</button></footer>`;

export class AssistantConsole {
    /** @param {{getMount:()=>HTMLElement,onVisibility:(open:boolean)=>void,activatePanel:(panel:string)=>void,dockFloat?:()=>void}} deps */
    constructor(deps) {
        this.deps=deps;this.scope=new LifetimeScope();this.open=false;
        /** @type {any} */this.service=null;/** @type {any} */this.model=null;/** @type {any} */this.jev=null;/** @type {any} */this.knowledge=null;/** @type {any} */this.mcp=null;
        /** @type {HTMLElement|null} */this.previousFocus=null;/** @type {HTMLElement|null} */this.streaming=null;
        /** @type {ScenarioExperimentPanel|null} */this.scenarios=null;
        this.element=document.createElement('aside');this.element.className='jev-console';this.element.hidden=true;
        this.element.setAttribute('role','region');this.element.setAttribute('aria-labelledby','jev-title');this.element.innerHTML=MARKUP;
        this.deps.getMount().append(this.element);
        this.input=/** @type {HTMLTextAreaElement} */(this.element.querySelector('#jev-input'));
        const style=document.createElement('link');style.rel='stylesheet';style.href=new URL('../../css/ui/components/jev-console.css',import.meta.url).href;document.head.append(style);this.scope.defer(()=>style.remove());
    }
    /** @param {string} name @returns {HTMLElement} */
    node(name){return /** @type {HTMLElement} */(this.element.querySelector(`[data-jev="${name}"]`));}
    /** @param {{service:any,model:any,jev:any,knowledge:any,mcp?:any}} deps */
    bind({service,model,jev,knowledge,mcp}) {
        this.service=service;this.model=model;this.jev=jev;this.knowledge=knowledge;this.mcp=mcp;
        this.scenarios=new ScenarioExperimentPanel(this.node('scenario-panel'),{service,model,jev});
        this.scope.defer(()=>this.scenarios?.dispose());
        this.scope.on(this.node('form'),'submit',event=>{event.preventDefault();const text=this.input.value;this.input.value='';void service.submit(text,{autonomous:this.checked('goal'),pauseWhileThinking:this.checked('pause')});});
        this.scope.on(this.element,'click',async event=>{
            const target=event.target instanceof Element?event.target.closest('button'):null;if(!target)return;
            if(target.dataset.prompt){this.input.value=target.dataset.prompt;/** @type {HTMLInputElement} */(this.node('goal')).checked=target.dataset.experiment==='true';this.input.focus();return;}
            try {
                switch(target.dataset.jev){
                case 'close':this.setOpen(false);break;
                case 'stop':service.stop();model.cancel();await mcp?.disconnect();break;
                case 'mcp-enable':await mcp?.connect();break;
                case 'mcp-disconnect':await mcp?.disconnect();break;
                case 'mcp-copy':await navigator.clipboard.writeText(/** @type {HTMLTextAreaElement} */(this.node('mcp-config')).value);break;
                case 'clear':service.clear();break;
                case 'load':await model.load();break;
                case 'unload':service.stop();await model.unload();break;
                case 'cache':service.stop();await model.clearCache();break;
                case 'connect':jev.setKey(/** @type {HTMLInputElement} */(this.node('key')).value);/** @type {HTMLInputElement} */(this.node('key')).value='';this.connection();break;
                case 'disconnect':service.stop();jev.disconnect();this.connection();break;
                case 'export':this.export();break;
                case 'search':{
                    const results=await knowledge.search(this.input.value || 'current lattice',{limit:6});
                    this.event({type:'sources',sources:results});
                    this.event({type:'notice',text:`Searched ${knowledge.coverage?.loadedShards ?? '?'} / ${knowledge.coverage?.totalShards ?? '?'} documentation shards.`});break;
                }
                }
            }catch(error){this.event({type:'error',text:error instanceof Error?error.message:'Operation failed'});}
        });
        this.scope.on(this.element,'keydown',event=>{
            if(event.key==='Escape'){event.stopPropagation();this.setOpen(false);}
            if(event.key==='Enter' && (event.ctrlKey||event.metaKey)){event.preventDefault();/** @type {HTMLFormElement} */(this.node('form')).requestSubmit();}
        });
        this.connection();
    }
    /** @param {string} name */ checked(name){return /** @type {HTMLInputElement} */(this.node(name)).checked;}
    connection(){this.node('connection').textContent=this.jev.connected?'JEV key configured · connection checked with each command':'JEV disconnected · local explanations and search available';this.scenarios?.updateAvailability();}
    /** @param {any} status */
    mcpStatus(status){
        this.node('mcp-status').textContent=status.text;
        /** @type {HTMLTextAreaElement} */(this.node('mcp-config')).value=status.config||'';
        this.node('mcp-config-label').hidden=!status.connected;
        /** @type {HTMLButtonElement} */(this.node('mcp-copy')).disabled=!status.connected;
    }
    /** @param {boolean} value @param {{fromDock?:boolean}} [options] */
    setOpen(value,{fromDock=false}={}){
        if(value && window.matchMedia('(max-width: 767px)').matches)value=false;
        if(this.deps.getMount().id==='panel-jev' && !fromDock){
            if(!value && this.element.closest('.floating-window'))this.deps.dockFloat?.();
            this.deps.activatePanel(value?'jev':'controls');
            if(!value)/** @type {HTMLElement|null} */(document.querySelector('.tab[data-panel="controls"]'))?.focus({preventScroll:true});
            return;
        }
        if(value===this.open)return;
        this.open=value;
        if(!value && this.element.matches(':popover-open'))this.element.hidePopover();
        this.element.hidden=!value;
        this.deps.onVisibility(value);
        if(!value && !fromDock && this.deps.getMount().id!=='panel-jev')this.deps.activatePanel('controls');
        if(value){this.previousFocus=document.activeElement instanceof HTMLElement?document.activeElement:null;this.relocate();}
        else if((!fromDock || this.element.classList.contains('jev-console--overlay')) && this.previousFocus?.isConnected && !this.previousFocus.closest('[inert]'))this.previousFocus.focus({preventScroll:true});
    }
    relocate(){
        const mount=this.deps.getMount(),docked=mount.id==='panel-jev',floated=!!mount.closest('.floating-window');
        if(this.element.matches(':popover-open'))this.element.hidePopover();
        if(docked)this.element.removeAttribute('popover');
        else this.element.setAttribute('popover','manual');
        this.element.classList.toggle('jev-console--overlay',!docked);
        this.element.setAttribute('role',docked?'region':'dialog');
        mount.append(this.element);
        if(!docked){
            const trigger=mount.querySelector('[data-observer-assistant]');
            if(trigger instanceof HTMLElement)this.previousFocus=trigger;
        }
        if(this.open){
            if(docked && !floated)this.deps.activatePanel('jev');
            else if(!this.element.matches(':popover-open'))this.element.showPopover();
            this.deps.onVisibility(true);
            if(docked && window.innerWidth<=767)mount.closest('#panel-area')?.scrollTo({top:0,behavior:'instant'});
            if(!docked || window.innerWidth>767)this.input.focus({preventScroll:true});
        }
    }
    /** @param {any} status */
    modelStatus(status){this.node('model').textContent=status.text || '';const progress=/** @type {HTMLProgressElement} */(this.node('progress'));progress.hidden=!status.loading;progress.value=status.progress || 0;this.scenarios?.updateAvailability();}
    /** @param {import('./search-client.js').Coverage} coverage */
    knowledgeStatus(coverage){
        const node=this.node('knowledge');node.textContent=`${coverage.complete?'Searched':coverage.loadedShards?'Searching':'Index ready:'} ${coverage.loadedShards}/${coverage.totalShards} shards · ${coverage.sourceFiles??'?'} indexed sources${coverage.extractionGaps?` · ${coverage.extractionGaps} extraction gaps. `:''}`;
        if(coverage.extractionGaps && this.knowledge?.manifestUrl){const a=document.createElement('a');a.href=this.knowledge.manifestUrl;a.textContent='Coverage manifest';a.target='_blank';a.rel='noopener noreferrer';node.append(a);}
    }
    /** @param {string} kind @param {string} text */
    message(kind,text){const item=document.createElement('div');item.className=`jev-message jev-${kind}`;item.textContent=text;this.node('messages').append(item);while(this.node('messages').childElementCount>100)this.node('messages').firstElementChild?.remove();item.scrollIntoView({block:'nearest'});return item;}
    /** @param {any} event */
    event(event){
        this.scenarios?.event(event);
        if(event.type==='observation')this.scenarios?.refresh(event.observation);
        switch(event.type){
        case 'state':this.node('send').textContent=event.busy?'Send new request':'Send';this.element.setAttribute('aria-busy',String(event.busy));if(!event.busy)this.streaming=null;break;
        case 'clear':this.node('messages').replaceChildren();this.streaming=null;break;
        case 'experiment':this.node('experiment').hidden=false;this.node('experiment').textContent=`Live experiment · ${event.phase} · ${event.actionsUsed??0}/50 actions${event.text?` · ${event.text}`:''}`;break;
        case 'experiment-sample':{
            const sample=event.sample;
            this.message('receipt',`Measured ${sample.workspace} · completed tick ${sample.tick??'unavailable'}${sample.facts.sampleTick!==undefined?` · diagnostic sample ${sample.facts.sampleTick}`:''} · ${sample.observedAt}`);break;
        }
        case 'observation':this.node('context').textContent=`${event.observation.workspace==='observer'?'Mind’s Eye':'Lattice Sim'} · tick ${event.observation.tick ?? 'unavailable'}${event.observation.selected?` · ${event.observation.selected.current?.name || event.observation.selected.id}`:''}`;this.node('facts').textContent=JSON.stringify(event.observation,null,2);break;
        case 'stream':if(!this.streaming){this.streaming=this.message('answer','');this.streaming.dataset.interpretation='true';}this.streaming.textContent+=event.text;break;
        case 'answer':if(this.streaming){this.streaming.textContent=event.text;this.streaming=null;}else this.message('answer',event.text);break;
        case 'receipt':this.message('receipt',`${event.receipt.status}: ${event.receipt.action?.type || 'command'}${event.receipt.error?` — ${event.receipt.error}`:''}${event.receipt.result!==undefined?`\n${JSON.stringify(event.receipt.result,null,2).slice(0,10000)}`:''}`);break;
        case 'decision':this.message('notice',`JEV: ${event.decision} · decision confidence ${Math.round(event.confidence*100)}% (not scientific certainty)`);break;
        case 'sources':{
            const box=this.message('sources',event.sources.length?'Sources':'No matching documentation.');
            /** @param {any} source @param {string} prefix */
            const sourceLink=(source,prefix)=>{const a=document.createElement('a');a.textContent=`${prefix} ${source.sourcePath}:${source.startLine} ${(source.statusTags || []).join(' ')}`;
                if(source.sourceUrl)try{const url=new URL(source.sourceUrl,document.baseURI);if(['https:','http:'].includes(url.protocol)){a.href=url.href;a.target='_blank';a.rel='noopener noreferrer';}}catch{} box.append(a);};
            event.sources.forEach((/** @type {any} */ source,/** @type {number} */ index)=>{
                sourceLink(source,`[${index+1}]`);
                for(const companion of source.authoritativeCompanions || []){
                    if(companion.available){sourceLink(companion,`↳ ${companion.claimId} controlling status:`);const statement=document.createElement('small');statement.textContent=(companion.statusStatements || []).join(' ').slice(0,1000);box.append(statement);}
                    else {const missing=document.createElement('small');missing.textContent=`${companion.claimId}: controlling status unavailable`;box.append(missing);}
                }
            });break;
        }
        default:if(event.text)this.message(event.type,event.text);
        }
    }
    export(){const blob=new Blob([JSON.stringify({schema:1,conversation:this.service.transcript},null,2)],{type:'application/json'});const url=URL.createObjectURL(blob);const a=document.createElement('a');a.href=url;a.download='jev-conversation.json';a.click();this.scope.timeout(()=>URL.revokeObjectURL(url),1000);}
    dispose(){this.setOpen(false);this.scope.dispose();this.element.remove();}
}
