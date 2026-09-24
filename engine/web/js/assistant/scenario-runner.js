// @ts-check
import {assertObservation,decisionObservation,validatePlan} from './contracts.js';
import {experimentSample,formatExperimentResult,observationDelay} from './experiment-state.js';
import {buildScenarioRecipe,buildEmptyScenarioRecipe} from './scenario-template.js';
import {chooseScenario,designScenario,scenarioCandidates,explicitScenarioSettings} from './scenario-designer.js';
import {SCENARIO_LIMITS} from './scenario-limits.js';
import {buildDecisionRequest} from './decision-context.js';

const RECEIPT_LIMIT=256;

/** Reject rounded counters instead of promoting them to exact evidence.
 * @param {unknown} value @returns {bigint} */
function exactCounter(value){
    if(typeof value==='number' && Number.isSafeInteger(value) && value>=0)return BigInt(value);
    if(typeof value==='string' && /^(0|[1-9]\d*)$/.test(value))return BigInt(value);
    throw new Error('The owner did not publish an exact nonnegative counter.');
}

/** @param {any} observation @param {string[]} keys */
function requireMeasurements(observation,keys){
    const counters=new Set(['manifested','positive','negative','zero','fieldTokens','relationTokens']);
    for(const key of keys){
        const value=observation.facts[key];
        if(counters.has(key)){try{exactCounter(value);continue;}catch{/* Report the named measurement below. */}}
        else if(typeof value==='number' && Number.isFinite(value))continue;
        else if(key==='incidence' && typeof value==='string' && /^-?(0|[1-9]\d*)$/.test(value))continue;
        throw new Error(`Requested measurement ${key} is unavailable or not a valid published value.`);
    }
}

/** One bounded preparation-and-measurement run, sharing Stop AI and authority
 * with ordinary assistant requests. Only acknowledged seed commits may replace
 * an owner. The LLM chooses data; existing controllers execute every write.
 * @param {import('./service.js').AssistantService} service
 * @param {{scenarioId:string,goal:string,ticks:number,sampleEvery:number,useLLM:boolean,draft?:any,expected?:any}} request
 */
export async function runScenarioExperiment(service,request){
    if(service.disposed)return;
    service.stop('Superseded by a scenario experiment');
    // Deterministic, explicitly approved protocols have their own bounded
    // execution budget. The general LLM autonomy limits remain unchanged.
    const run={controller:new AbortController(),generation:service.generation,deadline:service.now()+SCENARIO_LIMITS.durationMs,count:0,
        max:6+Math.ceil(SCENARIO_LIMITS.ticks/SCENARIO_LIMITS.chunkTicks)+SCENARIO_LIMITS.intervals,autonomous:false,
        progress:{completedTicks:0,totalTicks:request.ticks,sampleCount:0,approvalCount:0}};
    service.run=run;
    const timer=setTimeout(()=>{if(service.run===run)service.stop('Scenario experiment deadline reached');},SCENARIO_LIMITS.durationMs);
    const signal=run.controller.signal;
    /** @type {any[]} */const samples=[];
    /** @type {any[]} */const receipts=[];
    /** @type {any[]} */const approvalReceipts=[];
    /** @type {any} */let shared=null;
    /** @type {any} */let empty=null;
    /** @type {any} */let baseline=null;
    /** @type {any} */let expected=null;
    /** @type {any} */let initial=null;
    /** @type {any} */let emptyRecipe=null;
    let emptyApproved=false,protocolApproved=false,receiptCount=0,outcomeUnknown=false;
    const useLLM=request.useLLM || request.scenarioId==='__design__';
    const emit=(/** @type {any} */ event)=>service.emit({...event,scenarioRun:run.generation});
    const progress=()=>emit({type:'scenario-progress',...run.progress});
    const provenance=(/** @type {any} */ o)=>o?{workspace:o.workspace,ownerId:o.ownerId,preparationVersion:o.preparationVersion,tick:o.tick}:null;
    const resultData=()=>({template:shared,emptyBaseline:empty,baseline,samples,receipts,receiptCount,approvalReceipts,
        omittedReceipts:Math.max(0,receiptCount-receipts.length),progress:{...run.progress},outcomeUnknown});
    emit({type:'state',busy:true});
    emit({type:'scenario-stage',phase:'starting',text:'Preparing an empty-lattice experiment…'});
    const intent=`Start from an empty Scale 0 lattice, then ${useLLM?'design and run':'run the canned'} scenario experiment. ${request.goal} Advance ${request.ticks} ticks, sample every ${request.sampleEvery} ticks, and finish paused. Preparation replacement is explicitly requested; no changes to the microscopic law or arbitrary code.`;
    emit({type:'user',text:intent});
    try{
        if(typeof request.goal!=='string' || !request.goal.trim() || request.goal.length>2000)throw new Error('Describe the experiment in 1–2,000 characters.');
        if(!Number.isInteger(request.ticks)||request.ticks<1||request.ticks>SCENARIO_LIMITS.ticks || !Number.isInteger(request.sampleEvery)||request.sampleEvery<1||request.sampleEvery>request.ticks || Math.ceil(request.ticks/request.sampleEvery)>SCENARIO_LIMITS.intervals)
            throw new Error(`Use 1–${SCENARIO_LIMITS.ticks.toLocaleString('en-US')} ticks and at most ${SCENARIO_LIMITS.intervals} measured intervals.`);
        if(service.deps.jev.connected===false)throw new Error('Connect a JEV key before starting a scenario experiment.');
        if(useLLM && service.deps.model.ready===false)throw new Error('Load the local model to design a scenario.');
        const control=service.deps.getControl();expected=control?.observe();
        if(!control || expected?.workspace!=='lattice' || !expected.ownerId)throw new Error('Open Scale 0 to prepare a lattice experiment.');
        if(request.expected)assertObservation(expected,request.expected);
        initial=decisionObservation(expected);
        if(!expected.capabilities.includes('lattice.seed.describe')||!expected.capabilities.includes('lattice.seed.preview'))throw new Error('This owner does not support editable scenario preparations.');
        const live=()=>{service.assertLive(run);assertObservation(control.observe(),expected);};
        const approve=async(/** @type {'empty'|'protocol'} */ phase)=>{
            live();const fresh=control.observe();if(!fresh)throw new Error('The scenario owner is unavailable.');
            const includesEmptyPreparation=phase==='empty'||!empty;
            const preparation=phase==='empty'?emptyRecipe:shared.recipe;
            /** @type {import('./contracts.js').ActionPlan} */
            const plan={kind:'actions',message:phase==='empty'?'Authorize only the empty baseline for subsequent model design.':
                `Authorize the complete preparation and fixed ${shared.protocol.ticks}-tick sampling protocol, finishing paused.`,
                actions:[{type:'lattice.seed.preview',args:{recipeJson:JSON.stringify(includesEmptyPreparation?emptyRecipe:preparation)}}]};
            validatePlan(plan,fresh);
            /** @type {any[]} */const operations=[];
            const installation=(/** @type {string} */ recipeRef)=>operations.push(
                {type:'lattice.seed.preview',recipeRef},
                {type:'lattice.seed.apply',previewRef:`ID returned by the ${recipeRef} preview; no other preparation`},
                {type:'lattice.pause',when:'The acknowledged prepared owner is running'});
            if(includesEmptyPreparation)installation('emptyPreparation');
            if(phase==='protocol'){
                installation('preparation');
                operations.push({type:'lattice.step',totalTicks:shared.protocol.ticks,maxChunkTicks:SCENARIO_LIMITS.chunkTicks,sampleEvery:shared.protocol.sampleEvery});
                // Preflight available concrete schemas without inventing the
                // future preview ID or seed.apply capability.
                validatePlan({kind:'actions',message:'Fixed protocol capability admission',actions:[
                    {type:'lattice.pause',args:{}},{type:'lattice.step',args:{count:Math.min(SCENARIO_LIMITS.chunkTicks,shared.protocol.sampleEvery,shared.protocol.ticks)}}]},fresh);
            }
            const context={schemaVersion:1,phase,template:shared,preparation,emptyPreparation:emptyRecipe,
                initialPreparation:initial,currentPreparation:decisionObservation(fresh),emptyBaseline:empty,
                protocol:phase==='protocol'?{...shared.protocol,chunkTicks:SCENARIO_LIMITS.chunkTicks,
                    intervals:Math.ceil(shared.protocol.ticks/shared.protocol.sampleEvery),maxDurationMs:SCENARIO_LIMITS.durationMs}:null,
                authorization:{scope:phase==='empty'?'empty-preparation':'fixed-protocol',includesEmptyPreparation,finishPaused:true},operations};
            const decision=await service.deps.jev.evaluate(buildDecisionRequest(intent,fresh,plan,{
                scenarioCatalog:control.listScenarioTemplates?.()||[],receipts,experimentContext:context}),signal);
            live();
            approvalReceipts.push({phase,scope:context.authorization.scope,decision:decision.decision,confidence:decision.confidence,
                model:decision.model,at:new Date(service.now()).toISOString(),
                ...(phase==='protocol'?{ticks:shared.protocol.ticks,sampleEvery:shared.protocol.sampleEvery}:{})});
            emit({type:'decision',...decision});
            if(decision.decision!=='execute')throw Object.assign(new Error(`JEV ${decision.decision==='clarify'?'requested clarification':'rejected this preparation or operation'}. No subsequent operation was executed.`),{status:decision.decision});
            ++run.progress.approvalCount;progress();
            if(includesEmptyPreparation)emptyApproved=true;
            if(phase==='protocol')protocolApproved=true;
        };
        const execute=async(/** @type {any} */ action,/** @type {string} */ phase)=>{
            if(!['lattice.pause','lattice.step','lattice.seed.preview','lattice.seed.apply'].includes(action.type))throw new Error('Unsupported scenario-run operation.');
            if(!(phase==='empty'?emptyApproved:protocolApproved))throw new Error('This preparation or protocol has not been approved.');
            const plan={kind:'actions',message:phase,actions:[action]};
            live();service.assertRun(run);const fresh=control.observe();
            if(!fresh)throw new Error('The scenario owner is unavailable.');validatePlan(plan,fresh);
            if(action.type==='lattice.step' && (fresh.facts.running!==false || exactCounter(fresh.tick)!==exactCounter(baseline.tick)+BigInt(run.progress.completedTicks)))
                throw new Error('Playback or completed ticks changed outside the approved fixed protocol.');
            /** @type {any} */let receipt;
            try{receipt=await control.execute(action,{expected,signal,assertActive:()=>service.assertRun(run)});}
            catch(error){outcomeUnknown=true;throw error;}
            ++receiptCount;
            receipts.push({status:receipt.status,action:{type:action.type,...(action.type==='lattice.step'?{args:action.args}:{})},before:provenance(receipt.before),after:provenance(receipt.after),
                ...(receipt.completedTicks!==undefined?{completedTicks:receipt.completedTicks}:{}),...(receipt.error?{error:receipt.error}:{})});
            // Keep preparation provenance and the latest chunk receipts. The
            // explicit omitted count prevents a bounded journal claiming completeness.
            if(receipts.length>RECEIPT_LIMIT)receipts.splice(6,1);
            let receiptProblem=null;
            try{
                assertObservation(receipt.before,expected);
                if(action.type!=='lattice.seed.apply')assertObservation(receipt.after,expected);
                if(action.type==='lattice.step'){
                    if(exactCounter(receipt.before?.tick)!==exactCounter(baseline.tick)+BigInt(run.progress.completedTicks))
                        throw new Error('Completed tick interval started outside the approved protocol.');
                    const actual=exactCounter(receipt.after?.tick)-exactCounter(receipt.before?.tick);
                    const count=receipt.status==='applied'?action.args.count:receipt.completedTicks;
                    if(Number.isSafeInteger(count)&&count>=0&&count<=action.args.count&&actual===BigInt(count))run.progress.completedTicks+=count;
                    else throw new Error('Completed tick interval differs from the approved protocol.');
                }
            }catch(error){receiptProblem=error;outcomeUnknown=true;}
            if(receipt.status==='unknown')outcomeUnknown=true;
            if(receipt.status==='applied')++run.count;
            // A committed operation remains auditable if Stop arrived in flight.
            emit({type:'receipt',receipt});progress();service.assertLive(run);
            if(receiptProblem)throw receiptProblem;
            if(receipt.status!=='applied'||!receipt.after)throw new Error(receipt.error||'The operation was not acknowledged.');
            expected=receipt.after;live();
            return receipt;
        };
        const published=async()=>{
            const deadline=service.now()+15000;
            while(!signal.aborted){
                live();const now=control.observe();
                if(now?.facts.available===true && now.facts.stale!==true && now.facts.running===false
                    && now.tick!=null && String(now.facts.sampleTick??now.tick)===String(now.tick)){
                    exactCounter(now.tick);return now;
                }
                if(service.now()>=deadline)throw new Error('The owner did not publish a coherent paused measurement in time.');
                await observationDelay(50,signal);
            }
            signal.throwIfAborted();throw new Error('The measurement was cancelled.');
        };
        const install=async(/** @type {any} */ recipe,/** @type {string} */ phase)=>{
            emit({type:'scenario-stage',phase,text:phase==='empty'?'Preparing the empty lattice…':'Applying the approved scenario and settings…'});
            const preview=await execute({type:'lattice.seed.preview',args:{recipeJson:JSON.stringify(recipe)}},phase);
            if(typeof preview.result?.previewId!=='string')throw new Error('Seed preview did not return a stable preparation ID.');
            await execute({type:'lattice.seed.apply',args:{previewId:preview.result.previewId}},phase);
            if(control.observe()?.facts.running!==false)await execute({type:'lattice.pause',args:{}},phase);
            return published();
        };
        let scenarioId=request.scenarioId;
        if(scenarioId==='__design__'){
            const coverage=expected.facts.scenarioCatalogStatus;
            if(['initial','loading'].includes(coverage?.status))throw new Error('The scenario catalog is still loading. Wait for its coverage status before automatic template selection.');
            const catalog=control.listScenarioTemplates?.()||[];
            const candidates=scenarioCandidates(catalog,request.goal,expected);
            emit({type:'notice',text:`Searched all ${catalog.length} registered scenarios; ${candidates.length} supported candidates supplied to the model.${coverage?.status==='unavailable'?' Finite-record preparations are unavailable on this server.':''}`});
            scenarioId=await chooseScenario(service.deps.model,candidates,request.goal,signal);live();
        }
        const described=await control.execute({type:'lattice.seed.describe',args:{scenarioId}},{expected,signal,assertActive:()=>service.assertRun(run)});
        live();if(described.status!=='applied'||!described.result?.template)throw new Error(described.error||'Scenario settings template is unavailable.');
        const template=described.result.template;
        // Resolve obvious quantity/target ambiguities before clearing the old
        // preparation. Generation repeats this check against the same template.
        if(useLLM)explicitScenarioSettings(template.properties,request.goal);
        const measurements=template.allowedMeasurements.slice(0,8);
        const canned={name:template.label||template.scenarioId,goal:request.goal,settings:[],ticks:request.ticks,sampleEvery:request.sampleEvery,measurements};
        if(request.draft && (useLLM || request.draft.goal!==request.goal || request.draft.ticks!==request.ticks || request.draft.sampleEvery!==request.sampleEvery))
            throw new Error('The supplied experiment draft must match the requested goal and protocol.');
        // External MCP planners use the same data-only compiler and JEV path.
        // Validate before clearing the current lattice, including setting bounds.
        shared=buildScenarioRecipe(template,request.draft ?? canned);
        emit({type:'scenario-template',template:shared,properties:template.properties});
        emptyRecipe=buildEmptyScenarioRecipe(template);
        await approve(useLLM?'empty':'protocol');
        const emptyObservation=await install(emptyRecipe,'empty');
        requireMeasurements(emptyObservation,['manifested','positive','negative',...(template.backend==='finite-records'?['fieldTokens','relationTokens','incidence']:['totalFlux'])]);
        for(const key of ['manifested','positive','negative','fieldTokens','relationTokens','incidence','totalFlux']){
            const value=emptyObservation.facts[key];
            if(value!==undefined && value!==null && Number(value)!==0)throw new Error(`The empty preparation has nonzero ${key}; configuration stopped.`);
        }
        empty=experimentSample(emptyObservation,service.now());
        emit({type:'scenario-sample',phase:'empty',sample:empty});
        if(useLLM){
            emit({type:'scenario-stage',phase:'designing',text:'The LLM is choosing settings from the scenario template and empty-lattice observations…'});
            const draft=await designScenario(service.deps.model,template,request,decisionObservation(emptyObservation),signal);live();
            shared=buildScenarioRecipe(template,draft);emit({type:'scenario-template',template:shared,properties:template.properties});
            await approve('protocol');
        }
        const prepared=await install(shared.recipe,'preparation');
        requireMeasurements(prepared,shared.measurements);
        baseline=experimentSample(prepared,service.now());samples.push(baseline);
        run.progress.sampleCount=samples.length;progress();
        emit({type:'scenario-sample',phase:'baseline',sample:baseline});
        for(let measured=0;measured<shared.protocol.ticks;){
            const nextSample=Math.min(measured+shared.protocol.sampleEvery,shared.protocol.ticks);
            while(run.progress.completedTicks<nextSample){
                live();const before=control.observe();
                if(before?.facts.running!==false || exactCounter(before.tick)!==exactCounter(baseline.tick)+BigInt(run.progress.completedTicks))
                    throw new Error('Playback or completed ticks changed outside the approved fixed protocol.');
                const count=Math.min(SCENARIO_LIMITS.chunkTicks,nextSample-run.progress.completedTicks);
                emit({type:'scenario-stage',phase:'measuring',text:`Measuring ${run.progress.completedTicks} / ${shared.protocol.ticks} ticks…`});
                await execute({type:'lattice.step',args:{count}},'measurement');
                // The driver remains the sole owner. Yield between bounded
                // commands so rendering, Stop, and manual interventions run.
                await observationDelay(0,signal);
            }
            const after=await published();
            if(exactCounter(after.tick)-exactCounter(baseline.tick)!==BigInt(nextSample))throw new Error('Completed tick interval differs from the approved protocol.');
            requireMeasurements(after,shared.measurements);
            const sample=experimentSample(after,service.now());samples.push(sample);measured=nextSample;
            run.progress.sampleCount=samples.length;progress();
            emit({type:'scenario-sample',phase:'measurement',sample});
        }
        const current=await published();
        if(exactCounter(current.tick)-exactCounter(baseline.tick)!==BigInt(shared.protocol.ticks))throw new Error('Completed tick interval differs from the approved protocol.');
        const measured=shared.measurements.map((/** @type {string} */ key)=>`${key}: ${baseline.facts[key]} → ${current.facts[key]}${shared.measurementDefinitions?.[key]?.units?` (${shared.measurementDefinitions[key].units})`:''}.`).join('\n');
        const evidence=formatExperimentResult(baseline,current,run.count).split('\n').slice(1).join('\n');
        const result={status:'complete',...resultData(),
            summary:`Fixed protocol completed ${run.progress.completedTicks} ticks and ${samples.length-1} measured intervals after ${run.progress.approvalCount} JEV approval(s), using ${run.count} acknowledged preparation/chunk operations.\n${evidence}\n\nSelected measurements:\n${measured}`};
        emit({type:'scenario-result',result});emit({type:'answer',text:result.summary});
        return result;
    }catch(error){
        const status=signal.aborted?'stopped':/** @type {any} */(error).status||'failed';
        const message=error instanceof Error?error.message:'Scenario experiment failed.';
        const result={status,...resultData(),summary:`${message}\nRecorded ${run.progress.completedTicks} / ${request.ticks} confirmed ticks since the prepared baseline, with ${Math.max(0,samples.length-1)} measured intervals.${outcomeUnknown?' An operation outcome is uncertain; inspect receipts before retrying.':''}`};
        emit({type:'scenario-result',result});
        if(run.generation===service.generation)emit({type:'error',text:message});
        return result;
    }finally{
        clearTimeout(timer);
        if(service.run===run){service.run=null;emit({type:'state',busy:false});}
    }
}
