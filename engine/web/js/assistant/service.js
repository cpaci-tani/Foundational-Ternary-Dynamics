// @ts-check
import {LIMITS,assertObservation,decisionObservation,validatePlan} from './contracts.js';
import {ExperimentJournal,observationDelay,hasExperimentEvidence,formatExperimentResult} from './experiment-state.js';
import {normalizeRequest} from './request-router.js';
import {assistantHelp} from './capability-help.js';
import {parseExperimentObjective,validateExperimentObjectiveProposal} from './experiment-objective.js';
import {runScenarioExperiment} from './scenario-runner.js';
import {buildDecisionRequest} from './decision-context.js';
/** @typedef {import('./contracts.js').ObservationEnvelope} ObservationEnvelope */
/** @typedef {import('./contracts.js').ControlAdapter} ControlAdapter */

/** One service owns plans; simulation workers remain the only physics owners.
 * @related docs/adr/0018-assistant-control-and-local-inference.md
 */
export class AssistantService {
    /** @param {{getControl:()=>ControlAdapter|null,model:any,jev:any,knowledge:any,onEvent?:(event:any)=>void,onUserStop?:()=>void,now?:()=>number}} deps */
    constructor(deps) {
        this.deps=deps; this.now=deps.now || Date.now; this.generation=0;
        /** @type {{controller:AbortController,generation:number,deadline:number,count:number,max:number,autonomous:boolean,progress?:{completedTicks:number,totalTicks:number,sampleCount:number,approvalCount:number}}|null} */
        this.run=null;
        /** @type {any[]} */ this.transcript=[];
        this.disposed=false;
    }
    /** @param {any} event */
    emit(event) {
        // Only explicitly assembled events enter the transcript, never network requests or keys.
        const item={...event,at:new Date(this.now()).toISOString()};
        if (['user','answer','receipt','sources','error','decision','notice','experiment','experiment-sample','scenario-template','scenario-sample','scenario-result'].includes(event.type)) {
            this.transcript.push(item);
            while(this.transcript.length>LIMITS.transcriptTurns || JSON.stringify(this.transcript).length>LIMITS.transcriptBytes) this.transcript.shift();
        }
        this.deps.onEvent?.(item);
    }
    /** @param {string} [reason] */
    stop(reason='Stopped by user') {
        if(this.run?.autonomous)this.emit({type:'experiment',status:'stopped',phase:'stopped',actionsUsed:this.run.count,text:reason});
        ++this.generation;
        this.run?.controller.abort(new Error(reason)); this.run=null;
        this.deps.model.cancel?.();
        this.emit({type:'state',busy:false});
    }
    clear() { this.stop(); this.transcript=[]; this.emit({type:'clear'}); }
    /** @param {{scenarioId:string,goal:string,ticks:number,sampleEvery:number,useLLM:boolean}} request */
    runScenarioExperiment(request){return runScenarioExperiment(this,request);}
    currentObservation(){return this.deps.getControl()?.observe()??null;}
    scenarioCatalog(){return this.deps.getControl()?.listScenarioTemplates?.()??[];}
    /** Read-only constructor template preview. @param {string} scenarioId @param {AbortSignal} signal */
    async describeScenario(scenarioId,signal){
        const control=this.deps.getControl(),expected=control?.observe();
        if(!control||expected?.workspace!=='lattice')throw new Error('Select Scale 0 to inspect scenario templates.');
        const receipt=await control.execute({type:'lattice.seed.describe',args:{scenarioId}},{expected,signal,assertActive:()=>signal.throwIfAborted()});
        assertObservation(control.observe(),expected);
        if(receipt.status!=='applied'||!receipt.result?.template)throw new Error(receipt.error||'The template is unavailable.');
        return receipt.result.template;
    }
    /** @param {NonNullable<AssistantService['run']>} run */
    assertLive(run) {
        run.controller.signal.throwIfAborted();
        if (this.disposed || run.generation!==this.generation) throw new Error('This AI request was superseded');
        if(this.now()>=run.deadline) throw new Error('The AI run reached its time or action limit');
    }
    /** @param {NonNullable<AssistantService['run']>} run */
    assertRun(run) {
        this.assertLive(run);
        if(run.count>=run.max) throw new Error('The AI run reached its time or action limit');
    }
    /** @param {string} text @param {{autonomous?:boolean,pauseWhileThinking?:boolean}} [options] */
    async submit(text,{autonomous=false,pauseWhileThinking=false}={}) {
        if (this.disposed) return;
        text=text.trim();
        if (!text || text.length>LIMITS.inputChars) { this.emit({type:'error',text:'Enter a request of at most 4,000 characters.'}); return; }
        const request=normalizeRequest(text);
        if (/^(stop (?:the )?ai|cancel(?: (?:the )?ai)?|stop)[.!]?$/i.test(request.text)) { this.stop(); this.deps.onUserStop?.(); this.emit({type:'notice',text:'AI stopped. Simulation playback is unchanged.'}); return; }
        this.stop('Superseded by a new request');
        if(request.kind==='help'){
            this.emit({type:'user',text});
            const observation=this.deps.getControl()?.observe();
            if(observation)this.emit({type:'observation',observation:decisionObservation(observation)});
            this.emit({type:'answer',text:assistantHelp(observation,{ready:this.deps.model.ready,connected:this.deps.jev.connected})});
            return;
        }
        // Only an explicit user request (or the checked mode) enables autonomy.
        // Model-generated rewrites cannot turn a question into an experiment.
        const negative=/^(?:do\s+not|never|avoid)\b/i.test(request.text);
        autonomous=(autonomous && request.kind!=='question' && !negative) || request.kind==='experiment';
        const run={controller:new AbortController(),generation:this.generation,deadline:this.now()+LIMITS.goalMs,count:0,max:autonomous?LIMITS.goalActions:LIMITS.actions,autonomous};
        this.run=run;
        const timer=setTimeout(()=>{if(this.run===run)this.stop('AI run deadline reached');},LIMITS.goalMs);
        this.emit({type:'user',text}); this.emit({type:'state',busy:true});
        if(request.kind==='experiment')this.emit({type:'notice',text:'Starting the requested live experiment: up to 5 minutes / 50 actions, with JEV evaluation before each step.'});
        /** @type {{control:ControlAdapter,expected:ObservationEnvelope}|null} */ let restore=null;
        let experimentStatus='stopped';
        try {
            const control=this.deps.getControl();
            const initial=control?.observe();
            if (!control || !initial) throw new Error('Open Lattice Sim or Mind’s Eye to use the console.');
            if(autonomous && typeof this.deps.model.experimentStep!=='function')throw new Error('This language model does not support live experiment planning.');
            if(autonomous && this.deps.jev.connected===false)throw new Error('Connect a JEV key to conduct a live experiment.');
            const journal=autonomous?new ExperimentJournal(initial,this.now()):null;
            const objective=autonomous?parseExperimentObjective(text):null;
            /** @param {any} plan */
            const assertObjective=plan=>{
                if(!objective || !journal)return;
                const result=validateExperimentObjectiveProposal(objective,journal.baseline,control.observe(),plan);
                if(!result.valid)throw new Error(result.reason);
            };
            /** @type {ObservationEnvelope} */ let expected=initial;
            /** @type {any[]} */ const recentResults=[];
            this.emit({type:'observation',observation:decisionObservation(expected)});
            if (pauseWhileThinking && expected.facts.running===true) {
                const pause={type:`${expected.workspace}.pause`,args:{}};
                const paused=await control.execute(pause,{expected,signal:run.controller.signal,assertActive:()=>this.assertRun(run)});
                this.emit({type:'receipt',receipt:paused});
                this.assertLive(run);
                if(paused.status!=='applied' || !paused.after)throw new Error(paused.error || 'Pause could not be confirmed');
                ++run.count;
                expected=paused.after;assertObservation(control.observe(),expected);
                restore={control,expected};
            }
            /** @type {any[]} */ let sources=[];
            let instruction=text;
            let continuation=false;
            do {
                this.assertRun(run); assertObservation(control.observe(),expected);
                // Tick progression does not invalidate authority, but every round
                // must consume new measurements from that same preparation.
                expected=/** @type {ObservationEnvelope} */(control.observe());
                expected={...expected,facts:{...expected.facts,recentResults}};
                if(journal){
                    ++journal.round;
                    const sample=journal.record(expected,this.now());
                    this.emit({type:'experiment-sample',sample});
                    this.emit({type:'observation',observation:decisionObservation(expected)});
                    expected={...expected,facts:{...expected.facts,experimentRun:journal.context(run,this.now())}};
                    this.emit({type:'experiment',status:'running',phase:'planning',round:journal.round,actionsUsed:run.count,remainingActions:run.max-run.count});
                }
                const proposal=journal
                    ?await this.deps.model.experimentStep(text,expected,journal.context(run,this.now()),run.controller.signal)
                    :await this.deps.model.plan(instruction,expected,sources,run.controller.signal);
                this.assertRun(run); assertObservation(control.observe(),expected);
                const plan=validatePlan(proposal,expected);
                assertObjective(plan);
                if(journal && !plan.experiment)throw new Error('The model did not provide a typed experiment step.');
                if(journal && plan.experiment?.phase==='complete' && !hasExperimentEvidence(expected,journal.context(run,this.now())))throw new Error('The experiment has no new completed measurement or acknowledged preparation edit to conclude from.');
                if (plan.kind==='clarify') { experimentStatus='clarify';this.emit({type:'answer',text:plan.message || plan.experiment?.reason || 'Please specify a supported action, its target, and the quantity in displayed simulation units.'}); break; }
                if (plan.kind==='answer' && !journal) {
                    try { sources=await this.deps.knowledge.search(text,{limit:3,signal:run.controller.signal}); }
                    catch { this.assertRun(run); this.emit({type:'notice',text:'Documentation search is unavailable; answers will be limited to supplied observations.'}); }
                    this.assertRun(run);
                    if(sources.length)this.emit({type:'sources',sources});
                    assertObservation(control.observe(),expected);
                    const answer=await this.deps.model.explain(text,expected,sources,run.controller.signal,(/** @type {string} */ delta)=>{if(run.generation===this.generation&&!run.controller.signal.aborted)this.emit({type:'stream',text:delta});});
                    this.assertRun(run);assertObservation(control.observe(),expected);this.emit({type:'answer',text:answer});break;
                }
                // JEV evaluates the proposal against fresh received facts, not
                // the older snapshot from before language inference.
                expected=/** @type {ObservationEnvelope} */(control.observe());
                expected={...expected,facts:{...expected.facts,recentResults,...(journal?{experimentRun:journal.context(run,this.now())}:{})}};
                if(journal)this.emit({type:'experiment',status:'running',phase:'evaluating',round:journal.round,actionsUsed:run.count});
                const decisionRequest=buildDecisionRequest(text,expected,plan,{scenarioCatalog:control.listScenarioTemplates?.(),
                    receipts:this.transcript.filter(event=>event.type==='receipt').map(event=>event.receipt)});
                const decision=await this.deps.jev.evaluate(decisionRequest,run.controller.signal);
                this.assertRun(run); assertObservation(control.observe(),expected);
                this.emit({type:'decision',decision:decision.decision,confidence:decision.confidence,model:decision.model});
                if (decision.decision!=='execute') { experimentStatus=decision.decision;this.emit({type:'answer',text:decision.decision==='clarify'?'Please specify the target, action, and units more precisely.':'JEV rejected this interpretation. No further actions were executed.'}); break; }
                assertObjective(plan);
                if(journal && plan.experiment?.phase==='observe'){
                    ++run.count;
                    this.emit({type:'experiment',status:'running',phase:'observing',round:journal.round,actionsUsed:run.count,text:plan.experiment.reason});
                    await observationDelay(plan.experiment.waitMs,run.controller.signal);
                    this.assertLive(run);assertObservation(control.observe(),expected);
                    continue;
                }
                if(journal && plan.experiment?.phase==='complete'){
                    expected=/** @type {ObservationEnvelope} */(control.observe());
                    const sample=journal.record(expected,this.now());this.emit({type:'experiment-sample',sample});
                    this.emit({type:'observation',observation:decisionObservation(expected)});
                    this.emit({type:'answer',text:formatExperimentResult(journal.baseline,expected,run.count)});
                    experimentStatus='complete';
                    break;
                }
                for (const action of plan.actions) {
                    this.assertRun(run); assertObservation(control.observe(),expected);
                    assertObjective(plan);
                    // Restore the source before host entry captures its playback state.
                    if(restore && action.type==='workspace.switch') {
                        const resumed=await control.execute({type:`${expected.workspace}.resume`,args:{}},{expected,signal:run.controller.signal,assertActive:()=>this.assertRun(run)});
                        this.emit({type:'receipt',receipt:resumed});
                        this.assertLive(run);
                        if(resumed.status!=='applied' || !resumed.after)throw new Error('Source playback restoration failed');
                        ++run.count;
                        expected=resumed.after;restore=null;assertObservation(control.observe(),expected);
                    }
                    this.assertRun(run);
                    let receipt;
                    try{receipt=await control.execute(action,{expected,signal:run.controller.signal,assertActive:()=>this.assertRun(run)});}
                    catch(error){restore=null;throw error;}
                    this.emit({type:'receipt',receipt});
                    // A committed write remains auditable after Stop, but may not
                    // publish stale answers or continue the cancelled request.
                    this.assertLive(run);
                    if(receipt.status!=='applied') {restore=null;throw new Error(receipt.error || `Action outcome: ${receipt.status}`);}
                    if(receipt.result!==undefined){recentResults.push({action:action.type,result:receipt.result});if(recentResults.length>3)recentResults.shift();}
                    ++run.count;
                    if (action.type.endsWith('.pause') || action.type.endsWith('.resume')) restore=null;
                    // Only our acknowledged actions may advance the preparation fence.
                    expected=receipt.after;
                    if(!expected)throw new Error('Command receipt did not include the resulting preparation');
                    assertObservation(control.observe(),expected);
                    expected={...expected,facts:{...expected.facts,recentResults}};
                    if(journal){const sample=journal.record(expected,this.now(),action,receipt.result);this.emit({type:'experiment-sample',sample});}
                    if(restore && (restore.expected.ownerId!==expected.ownerId || /\.(reset|scenario|preset|undo|seed\.apply)$/.test(action.type) || action.args.profile))restore=null;
                    if (restore) restore.expected=expected;
                }
                this.emit({type:'observation',observation:decisionObservation(expected)});
                // Reacquire capabilities after an explicitly requested owner or
                // profile transition before interpreting its remaining clauses.
                continuation=!!plan.continuation;
                if(continuation){instruction=/** @type {string} */(plan.continuation);continue;}
                if(!autonomous) {
                    this.emit({type:'answer',text:`Completed ${plan.actions.length} acknowledged action${plan.actions.length===1?'':'s'}.`});
                    if(/\b(explain|describe|compare|what changed)\b/i.test(text)){
                        try{sources=await this.deps.knowledge.search(text,{limit:3,signal:run.controller.signal});}catch{sources=[];}
                        this.assertLive(run);assertObservation(control.observe(),expected);
                        if(sources.length)this.emit({type:'sources',sources});
                        const captured={...expected,facts:{...expected.facts,previousObservation:decisionObservation(initial)}};
                        const answer=await this.deps.model.explain(text,captured,sources,run.controller.signal,(/** @type {string} */ delta)=>{if(run.generation===this.generation&&!run.controller.signal.aborted)this.emit({type:'stream',text:delta});});
                        this.assertLive(run);assertObservation(control.observe(),expected);this.emit({type:'answer',text:answer});
                    }
                    break;
                }
                // Dedicated experiment planning receives the original goal and
                // acknowledged evidence, never a control prompt parsed as a command.
                await observationDelay(1000,run.controller.signal);
            } while(autonomous || continuation);
        } catch(error) {
            experimentStatus='failed';
            if(run.generation===this.generation) this.emit({type:'error',text:error instanceof Error?error.message:'Assistant request failed'});
        } finally {
            clearTimeout(timer);
            if(autonomous && run.generation===this.generation)this.emit({type:'experiment',status:experimentStatus,phase:experimentStatus,actionsUsed:run.count});
            // Stop/cancel deliberately suppresses queued resume continuations.
            if(restore && run.generation===this.generation && !run.controller.signal.aborted) {
                try { this.assertRun(run);assertObservation(restore.control.observe(),restore.expected);const resumed=await restore.control.execute({type:`${restore.expected.workspace}.resume`,args:{}},{expected:restore.expected,signal:run.controller.signal,assertActive:()=>this.assertRun(run)});this.emit({type:'receipt',receipt:resumed});if(resumed.status==='applied')++run.count;else this.emit({type:'notice',text:'Playback restoration was not acknowledged.'}); }
                catch { this.emit({type:'notice',text:'Playback was not restored because the world or request changed.'}); }
            }
            if(this.run===run){this.run=null;this.emit({type:'state',busy:false});}
        }
    }
    dispose() { this.disposed=true;this.stop('Console disposed');this.deps.jev.dispose?.();this.deps.model.dispose?.();this.deps.knowledge.dispose?.(); }
}
