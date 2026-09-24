// @ts-check
import {decisionObservation} from './contracts.js';
/** @typedef {import('./contracts.js').ObservationEnvelope} ObservationEnvelope */

/** A conclusion needs a new published sample or an acknowledged preparation edit.
 * Engine acknowledgments alone cannot relabel pending diagnostics as measured.
 * @param {ObservationEnvelope} observation @param {any} context
 */
export function hasExperimentEvidence(observation,context){
    const baseline=context.baseline;
    if(!baseline || observation.facts.available===false || observation.facts.stale===true)return false;
    const before=baseline.facts?.sampleTick??baseline.tick;
    const after=observation.facts.sampleTick??observation.tick;
    if(before!=null && after!=null && String(before)!==String(after))return true;
    if(context.completedActions?.some((/** @type {any} */ row)=>/\.(catalog|seed\.preview)$/.test(row.action?.type || '') && row.result!==undefined))return true;
    return (context.completedActions?.length || 0)>0 && baseline.preparationVersion!==observation.preparationVersion;
}

/** The small model selects actions; recorded numerical results never come from
 * its generated prose. Report source values without inferring physical mappings.
 * @param {any} baseline @param {ObservationEnvelope} current @param {number} count
 */
export function formatExperimentResult(baseline,current,count){
    const display=(/** @type {any} */ value)=>value===undefined||value===null?'unavailable':typeof value==='object'?JSON.stringify(value):String(value);
    const rows=[`Experiment concluded after ${count} approved actions/observation intervals.`,
        `Completed engine tick: ${display(baseline.tick)} → ${display(current.tick)}.`,
        `Preparation: ${display(baseline.preparationVersion)} → ${display(current.preparationVersion)}.`];
    for(const [key,label] of [['sampleTick','Published diagnostic sample tick'],['time','Coordinate time'],['manifested','Manifested count'],
        ['positive','Positive count'],['negative','Negative count'],['fieldTokens','Field tokens'],['relationTokens','Relation tokens'],['incidence','Incidence'],
        ['gravityMode','Gravity mode'],['gravityStrength','Gravity strength']]){
        if(baseline.facts?.[key]!==undefined || current.facts[key]!==undefined)rows.push(`${label}: ${display(baseline.facts?.[key])} → ${display(current.facts[key])}.`);
    }
    if(baseline.selected?.id && baseline.selected.id===current.selected?.id){
        rows.push(`Selected object: ${current.selected.id}.`);
        for(const key of ['mass','position','velocity','alive'])rows.push(`Current-source ${key}: ${display(baseline.selected.current?.[key])} → ${display(current.selected.current?.[key])}.`);
        if(current.selected.observed)rows.push(`Received-light emission time: ${display(current.selected.observed.emissionTime)} (separate from current source state).`);
    }
    if(current.facts.stale===true || current.facts.available===false)rows.push('Current diagnostics are unavailable or stale; use the recorded sample timestamps.');
    rows.push('Values above are recorded observations in the simulator’s declared units.');
    return rows.join('\n');
}

/** Capture measured records only. Keep their original timestamps and availability;
 * the observation event is not a replacement diagnostic sample timestamp.
 * @param {ObservationEnvelope} observation @param {number} now
 */
export function experimentSample(observation,now){
    const received=decisionObservation(observation);
    const facts={...received.facts};
    delete facts.experimentRun;delete facts.recentResults;delete facts.previousObservation;
    delete facts.settings;delete facts.rendering;delete facts.scenarios;delete facts.environment;
    if(facts.entities)facts.entities=facts.entities.slice(0,2);
    return{workspace:received.workspace,ownerId:received.ownerId,preparationVersion:received.preparationVersion,
        tick:received.tick??null,observedAt:new Date(now).toISOString(),facts,selected:received.selected??null};
}

/** Bounded evidence for one explicitly requested experiment; never another driver. */
export class ExperimentJournal{
    /** @param {ObservationEnvelope} observation @param {number} now */
    constructor(observation,now){
        this.baseline=experimentSample(observation,now);
        /** @type {any[]} */this.recent=[];
        /** @type {any[]} */this.completedActions=[];
        this.round=0;
    }
    /** @param {ObservationEnvelope} observation @param {number} now @param {any} [action] @param {any} [result] */
    record(observation,now,action,result){
        const sample=experimentSample(observation,now);
        this.recent.push({sample,...(action?{action:structuredClone(action)}:{})});
        while(this.recent.length>4)this.recent.shift();
        if(action){
            const received=result===undefined?undefined:JSON.stringify(result).length<=4000?structuredClone(result):{omitted:true,reason:'Result exceeds experiment context budget; inspect its full receipt.'};
            this.completedActions.push({action:structuredClone(action),...(received===undefined?{}:{result:received})});
            while(this.completedActions.length>6)this.completedActions.shift();
        }
        return sample;
    }
    /** @param {{count:number,max:number,deadline:number}} run @param {number} now */
    context(run,now){return{baseline:this.baseline,recent:this.recent,completedActions:this.completedActions,round:this.round,
        actionsCompleted:run.count,remainingActions:Math.max(0,run.max-run.count),remainingMs:Math.max(0,run.deadline-now)};}
}

/** A cancellable wall-time observation interval. No simulation ticking or pause.
 * @param {number} ms @param {AbortSignal} signal
 */
export function observationDelay(ms,signal){
    return new Promise((resolve,reject)=>{
        signal.throwIfAborted();
        const abort=()=>{clearTimeout(timer);signal.removeEventListener('abort',abort);reject(signal.reason);};
        const timer=setTimeout(()=>{signal.removeEventListener('abort',abort);resolve(undefined);},ms);
        signal.addEventListener('abort',abort,{once:true});
    });
}
