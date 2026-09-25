// @ts-check
import {LIMITS,assertObservation,validatePlan,validateValue} from './contracts.js';
import {buildDecisionRequest} from './decision-context.js';
import {SCENARIO_LIMITS,validateScenarioProtocol} from './scenario-limits.js';

const EXPECTED={type:'object',properties:{workspace:{type:'string',enum:['lattice','observer']},ownerId:{type:'string',minLength:1,maxLength:512},preparationVersion:{type:'string',maxLength:512}},required:['workspace','ownerId','preparationVersion'],additionalProperties:false};
const string=(maxLength=2000)=>({type:'string',minLength:1,maxLength});
/** @param {Record<string,any>} properties @param {string[]} [required] */
const object=(properties,required=Object.keys(properties))=>({type:'object',properties,required,additionalProperties:false});
const SCHEMAS={
    observe:object({}),
    measure_flux_sectors:object({expected:EXPECTED,expectedTick:string(20)}),
    list_scenarios:object({query:{type:'string',maxLength:200},offset:{type:'integer',minimum:0,maximum:100000},limit:{type:'integer',minimum:1,maximum:50}},[]),
    describe_scenario:object({scenarioId:string(160)}),
    execute_plan:object({intent:string(4000),expected:EXPECTED,actions:{type:'array',minItems:1,maxItems:8,items:object({type:string(160),args:{type:'object',additionalProperties:true}})}}),
    run_scenario:object({goal:string(),scenarioId:string(160),ticks:{type:'integer',minimum:1,maximum:SCENARIO_LIMITS.ticks},sampleEvery:{type:'integer',minimum:1,maximum:SCENARIO_LIMITS.ticks},expected:EXPECTED,draft:{type:'object',additionalProperties:true}},['goal','scenarioId','ticks','sampleEvery','expected']),
    run_status:object({runId:string(160)}),
    search_docs:object({query:string(),limit:{type:'integer',minimum:1,maximum:6}},['query']),
    stop:object({}),
};

/** Typed external proposals enter the existing service and simulation adapters.
 * This layer never loads a model, evaluates code, or advances a second owner.
 */
export class McpControl {
    /** @param {import('./service.js').AssistantService} service */
    constructor(service){
        this.service=service;this.stopped=false;this.sequence=0;
        /** @type {Map<string,any>} */this.jobs=new Map();
        /** @type {NonNullable<import('./service.js').AssistantService['run']>|null} */this.ownedRun=null;
    }
    rearm(){this.stopped=false;this.jobs.clear();}
    stop(){this.stopped=true;if(this.ownedRun && this.service.run===this.ownedRun)this.service.stop('MCP control stopped');this.ownedRun=null;}
    /** @param {string} method @param {any} args @param {AbortSignal} signal */
    async handle(method,args,signal){
        signal.throwIfAborted();
        const schema=SCHEMAS[/** @type {keyof typeof SCHEMAS} */(method)];
        if(!schema)throw new Error('Unsupported MCP operation.');
        validateValue(args,schema,'request');
        if(method==='stop'){this.service.stop('Stopped by MCP client');this.stop();return{status:'stopped',message:'No further MCP writes until this tab reconnects. Playback is unchanged.'};}
        if(method==='observe')return{observation:this.service.currentObservation(),jevConnected:this.service.deps.jev.connected===true,writesStopped:this.stopped,limits:LIMITS,scenarioLimits:SCENARIO_LIMITS};
        if(method==='measure_flux_sectors'){
            const control=this.service.deps.getControl();
            assertObservation(control?.observe()??null,args.expected);
            if(this.service.run)throw new Error('Wait for the active assistant operation to finish before measuring flux sectors.');
            if(!control?.measureFluxSectors)throw new Error('Flux-sector measurement is unavailable on this owner.');
            return control.measureFluxSectors(args,signal);
        }
        if(method==='list_scenarios'){
            const catalog=this.service.scenarioCatalog(),query=(args.query||'').toLowerCase();
            const matches=catalog.filter(row=>JSON.stringify(row).toLowerCase().includes(query));
            const offset=args.offset||0,limit=args.limit||25;
            return{total:catalog.length,matched:matches.length,offset,items:matches.slice(offset,offset+limit),nextOffset:offset+limit<matches.length?offset+limit:null,coverage:this.service.currentObservation()?.facts.scenarioCatalogStatus};
        }
        if(method==='describe_scenario')return this.service.describeScenario(args.scenarioId,signal);
        if(method==='search_docs')return{sources:await this.service.deps.knowledge.search(args.query,{limit:args.limit||4,signal}),coverage:this.service.deps.knowledge.coverage};
        if(method==='run_status'){
            const job=this.jobs.get(args.runId);if(!job)throw new Error('Unknown experiment run in this connection.');
            return{...structuredClone(job),...(job.status==='running'?{progress:{actionsUsed:this.ownedRun?.count??0,...this.ownedRun?.progress,observation:this.service.currentObservation()}}:{})};
        }
        if(this.stopped)throw new Error('MCP writes are stopped. Reconnect MCP in the simulator to enable them.');
        if(this.service.run)throw new Error('Another assistant operation is active. Wait for its receipt or stop it first.');
        const observation=this.service.currentObservation();
        assertObservation(observation,args.expected);
        if(this.service.deps.jev.connected===false)throw new Error('Connect JEV in the simulator before requesting mutations.');
        if(method==='execute_plan')return this.executePlan(args,signal);
        if(method==='run_scenario'){
            if(this.jobs.size>=16)throw new Error('This connection has reached its 16 experiment history limit. Reconnect to start another.');
            if(args.scenarioId==='__design__')throw new Error('Choose an explicit scenario template using list_scenarios and describe_scenario.');
            validateScenarioProtocol(args.ticks,args.sampleEvery);
            const runId=`mcp-${++this.sequence}`;
            /** @type {any} */const job={runId,status:'running',startedAt:new Date().toISOString(),result:null};this.jobs.set(runId,job);
            const promise=this.service.runScenarioExperiment({...args,useLLM:false});
            this.ownedRun=this.service.run;const owned=this.ownedRun;
            // Accepted experiments outlive the short start request. Stop AI,
            // disconnect, visibility and workspace changes still cancel them.
            void promise.then(result=>{job.status=result?.status||'failed';job.result=result;},error=>{job.status='failed';job.result={summary:error instanceof Error?error.message:'Experiment failed'};}).finally(()=>{if(this.ownedRun===owned)this.ownedRun=null;});
            return{runId,status:job.status};
        }
        throw new Error('Unsupported MCP operation.');
    }
    /** @param {any} args @param {AbortSignal} signal */
    async executePlan(args,signal){
        const service=this.service,control=service.deps.getControl();
        const initial=control?.observe();if(!control||!initial)throw new Error('The active simulation owner is unavailable.');
        /** @type {import('./contracts.js').ObservationEnvelope} */let expected=initial;
        const plan=validatePlan({kind:'actions',message:args.intent,actions:args.actions},expected);
        if(plan.actions.some(action=>action.type==='workspace.switch') && plan.actions.length!==1)throw new Error('Switch workspace in a separate call, then acquire its new capabilities.');
        if(service.run)throw new Error('Another assistant operation is active. Wait for its receipt or stop it first.');
        const run={controller:new AbortController(),generation:service.generation,deadline:service.now()+55000,count:0,max:LIMITS.actions,autonomous:false};
        service.run=run;this.ownedRun=run;
        const abort=()=>run.controller.abort(signal.reason);signal.addEventListener('abort',abort,{once:true});
        const timer=setTimeout(()=>run.controller.abort(new Error('MCP plan deadline reached')),55000);
        /** @type {any[]} */const receipts=[];let awaitingReceipt=false;
        service.emit({type:'user',text:`External MCP: ${args.intent}`});service.emit({type:'state',busy:true});
        try{
            signal.throwIfAborted();service.assertRun(run);assertObservation(control.observe(),args.expected);
            const request=buildDecisionRequest(args.intent,expected,plan,{scenarioCatalog:control.listScenarioTemplates?.(),
                receipts:service.transcript.filter(event=>event.type==='receipt').map(event=>event.receipt)});
            const decision=await service.deps.jev.evaluate(request,run.controller.signal);
            service.assertRun(run);assertObservation(control.observe(),expected);service.emit({type:'decision',...decision});
            if(decision.decision!=='execute')return{status:decision.decision,receipts,decision};
            for(const action of plan.actions){
                service.assertRun(run);assertObservation(control.observe(),expected);
                awaitingReceipt=true;
                const receipt=await control.execute(action,{expected,signal:run.controller.signal,assertActive:()=>service.assertRun(run)});
                awaitingReceipt=false;
                receipts.push(receipt);service.emit({type:'receipt',receipt});
                if(receipt.status!=='applied')return{status:receipt.status,receipts};
                ++run.count;service.assertLive(run);
                if(!receipt.after)throw new Error('The command did not return its resulting preparation.');
                expected=receipt.after;assertObservation(control.observe(),expected);
            }
            return{status:'applied',receipts,observation:control.observe()};
        }catch(error){
            return{status:awaitingReceipt?'unknown':run.controller.signal.aborted?'stopped':'failed',receipts,error:error instanceof Error?error.message:'MCP plan failed',observation:control.observe()};
        }finally{
            clearTimeout(timer);signal.removeEventListener('abort',abort);
            if(service.run===run){service.run=null;service.emit({type:'state',busy:false});}
            if(this.ownedRun===run)this.ownedRun=null;
        }
    }
}
