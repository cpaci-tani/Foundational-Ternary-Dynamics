// @ts-check
import {LifetimeScope} from '../ui/utils/lifetime-scope.js';
import {AssistantConsole} from './console.js';
import {AssistantService} from './service.js';
import {BrowserModel} from './model.js';
import {JevClient} from './jev-client.js';
import {KnowledgeSearch} from './search-client.js';
import {createLatticeControl} from './lattice-control.js';
import {createObserverControl} from './observer-control.js';
import {assertObservation,validateValue} from './contracts.js';
import {McpBridge} from './mcp-bridge.js';
import {getRecordCatalogStatus} from '../scales/scale0/ui/controls/record-observation.js';
import {floatingWindowManager} from '../ui/components/floating-window/component.js';

/** @param {{getCtx:()=>any,isLattice:()=>boolean,loadLatticeScenario?:any,host:any,registry:any,app:HTMLElement,activatePanel:(panel:string)=>void}} deps */
export async function createAssistant(deps){
    const scope=new LifetimeScope();
    const response=await fetch(new URL('./config.json',import.meta.url));
    const config=await response.json();
    let local={jevConfigured:false};
    if(['localhost','127.0.0.1','[::1]'].includes(location.hostname)){
        try{const status=await fetch('/api/ai/status');if(status.ok){local=await status.json();Object.assign(config,local,{apiBase:''});}}catch{}
    }
    const lattice=createLatticeControl({getCtx:deps.getCtx,loadScenario:deps.loadLatticeScenario,getCatalogStatus:getRecordCatalogStatus,isActive:()=>!deps.host.active && !deps.host.suspended && deps.isLattice()});
    const observer=createObserverControl({getWorkspace:()=>deps.registry.get('observerWorkspace'),isActive:()=>deps.host.active});
    const active=()=>deps.host.active?observer:deps.isLattice()?lattice:null;
    let transitioning=false;
    const switchDescriptor={type:'workspace.switch',description:'Explicitly switch between Lattice Sim and Mind’s Eye',args:{type:'object',properties:{workspace:{type:'string',enum:['lattice','observer']}},required:['workspace'],additionalProperties:false}};
    const router={
        /** @param {any} args @param {AbortSignal} signal */
        async measureFluxSectors(args,signal){
            if(active()!==lattice)throw new Error('Flux-sector measurement requires the active lattice workspace.');
            return lattice.measureFluxSectors?.(args,signal);
        },
        listScenarioTemplates(){return lattice.listScenarioTemplates?.()||[];},
        observe(){const o=active()?.observe();if(!o)return null;return {...o,actions:[...(o.actions || []),switchDescriptor],capabilities:[...o.capabilities,'workspace.switch']};},
        /** @param {import('./contracts.js').AssistantAction} action @param {any} options */
        async execute(action,options){
            options.assertActive?.();assertObservation(router.observe(),options.expected);
            if(action.type!=='workspace.switch'){
                const receipt=await /** @type {any} */(active()).execute(action,options);
                if(receipt.after){assertObservation(router.observe(),receipt.after);receipt.after={...receipt.after,actions:[...(receipt.after.actions || []),switchDescriptor],capabilities:[...receipt.after.capabilities,'workspace.switch']};}
                return receipt;
            }
            validateValue(action.args,switchDescriptor.args);options.signal?.throwIfAborted();const before=router.observe();transitioning=true;
            const switching=before?.workspace!==action.args.workspace;
            const abort=()=>{if(switching && action.args.workspace==='observer')void deps.host.exit();};
            options.signal?.addEventListener('abort',abort,{once:true});
            try{if(action.args.workspace==='observer')await deps.host.enter();else await deps.host.exit();options.assertActive?.();options.signal?.throwIfAborted();
                const after=router.observe();
                if(!after?.ownerId || after.workspace!==action.args.workspace)return {status:'superseded',action,before,after,error:'Workspace transition was superseded before the destination became ready.'};
                return {status:'applied',action,before,after};
            }catch(error){
                if(options.signal?.aborted && switching){
                    if(before?.workspace==='observer')await deps.host.enter();
                    else if(before?.workspace==='lattice')await deps.host.exit();
                }
                throw error;
            }finally{options.signal?.removeEventListener('abort',abort);transitioning=false;}
        },
    };
    const view=new AssistantConsole({getMount:()=>deps.host.active?deps.registry.get('observerWorkspace').getAssistantMount():/** @type {HTMLElement} */(deps.app.querySelector('#panel-jev')),
        activatePanel:deps.activatePanel,
        dockFloat:()=>floatingWindowManager.getWindow('jev')?.dock(),
        onVisibility:open=>{deps.registry.get('observerWorkspace')?.releaseAssistantInput(open && deps.host.active);if(open){const observation=router.observe();if(observation)view.event({type:'observation',observation});}}});
    const model=new BrowserModel({modelBase:config.modelBase,onStatus:status=>view.modelStatus(status),
        onActivity:busy=>deps.registry.get('observerWorkspace')?.renderer?.setExternalGpuLoad(busy)});
    const jev=new JevClient({apiBase:config.apiBase,localConfigured:local.jevConfigured});
    const knowledge=new KnowledgeSearch({manifestUrl:new URL(config.knowledgeManifest,document.baseURI).href,onCoverage:coverage=>view.knowledgeStatus(coverage)});
    const service=new AssistantService({getControl:()=>/** @type {any} */(router),model,jev,knowledge,onEvent:event=>view.event(event),onUserStop:()=>{void mcp.disconnect();}});
    const mcp=new McpBridge({service,onStatus:status=>view.mcpStatus(status)});
    view.bind({service,model,jev,knowledge,mcp});
    const catalogStatus=()=>view.event({type:'scenario-catalog',status:getRecordCatalogStatus()});
    scope.on(document,'ftd:record-catalog-status',catalogStatus);catalogStatus();
    const mobile=window.matchMedia('(max-width: 767px)');
    scope.on(document,'ftd:assistant-toggle',()=>{if(!mobile.matches)view.setOpen(!view.open);});
    scope.on(window,'ftd:panel-visibility-change',event=>{
        const detail=/** @type {CustomEvent} */(event).detail;
        if(!deps.host.active && detail?.reason==='floating-mount' && detail.panelId==='jev' && !mobile.matches){
            view.setOpen(true,{fromDock:true});
            return;
        }
        if(!deps.host.active && detail?.activePanel){
            if(deps.app.querySelector('.tab[data-panel="jev"].is-floated') && !mobile.matches)return;
            view.setOpen(detail.activePanel==='jev' && !detail.collapsed,{fromDock:true});
        }
    });
    if(!deps.host.active && deps.app.querySelector('#panel-jev.active') && !deps.app.classList.contains('panels-collapsed')){
        view.setOpen(true,{fromDock:true});
    }
    scope.on(document,'ftd:workspace-change',()=>{
        if(!transitioning)service.stop('Workspace changed');
        view.relocate();
        if(!deps.host.active){
            const floated=!!deps.app.querySelector('.tab[data-panel="jev"].is-floated');
            const selected=!!deps.app.querySelector('.tab[data-panel="jev"].active');
            view.setOpen(!mobile.matches && (floated || selected && !deps.app.classList.contains('panels-collapsed')),{fromDock:true});
        }
        const o=router.observe();if(o)view.event({type:'observation',observation:o});
    });
    scope.on(mobile,'change',()=>{
        if(!mobile.matches)return;
        service.stop('JEV unavailable on mobile');
        void mcp.disconnect();
        floatingWindowManager.getWindow('jev')?.dock();
        view.setOpen(false,{fromDock:true});
        deps.activatePanel('controls');
    });
    scope.on(document,'visibilitychange',()=>{if(document.hidden){service.stop('Tab hidden');void mcp.disconnect();}});
    scope.on(window,'pagehide',()=>{void mcp.disconnect();});
    const api={service,view,model,jev,knowledge,mcp,control:router,dispose(){scope.dispose();mcp.dispose();service.dispose();view.dispose();lattice.dispose?.();observer.dispose?.();deps.registry.unregister('assistant');}};
    deps.registry.register('assistant',api);
    return api;
}
