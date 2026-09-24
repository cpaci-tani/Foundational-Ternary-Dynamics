// @ts-check
/** @typedef {{type:string,args:Record<string,any>}} AssistantAction */
/** @typedef {{workspace:string,ownerId:string|null,preparationVersion:string,tick?:number|string|null,capabilities:string[],facts:Record<string,any>,selected?:any,actions?:any[],catalogs?:any}} ObservationEnvelope */
/** @typedef {{kind:'actions'|'answer'|'clarify',message:string,actions:AssistantAction[],continuation?:string,experiment?:{phase:'act'|'observe'|'complete'|'clarify',waitMs:number,reason:string}}} ActionPlan */
/** @typedef {{commandId?:string,status:'applied'|'rejected'|'superseded'|'unknown',action?:AssistantAction,before?:any,after?:any,result?:any,error?:string}} CommandReceipt */
/** @typedef {{observe:()=>ObservationEnvelope|null,execute:(action:AssistantAction,options:any)=>Promise<CommandReceipt>,listScenarioTemplates?:()=>any[],measureFluxSectors?:(args:any,signal:AbortSignal)=>Promise<any>,dispose?:()=>void}} ControlAdapter */

export const LIMITS = Object.freeze({ actions:8, goalActions:50, goalMs:300000, inputChars:4000, transcriptBytes:262144, transcriptTurns:100 });

/** Validate the supported JSON-schema subset, including closed property sets.
 * @param {unknown} value @param {any} schema @param {string} [path]
 */
export function validateValue(value, schema, path = 'args') {
    if (!schema || typeof schema !== 'object') throw new Error(`Missing schema for ${path}`);
    if (schema.enum && !schema.enum.some((/** @type {unknown} */ item) => JSON.stringify(item) === JSON.stringify(value))) throw new Error(`Unsupported ${path}`);
    if (schema.anyOf) {
        if (!schema.anyOf.some((/** @type {any} */ candidate) => { try { validateValue(value, candidate, path); return true; } catch { return false; } })) throw new Error(`Invalid ${path}`);
        return;
    }
    if (schema.type === 'object') {
        if (!value || typeof value !== 'object' || Array.isArray(value)) throw new Error(`${path} must be an object`);
        const record = /** @type {Record<string,unknown>} */ (value);
        for (const key of schema.required || []) if (!Object.hasOwn(record, key)) throw new Error(`Missing ${path}.${key}`);
        for (const [key, item] of Object.entries(record)) {
            if (['__proto__','constructor','prototype'].includes(key)) throw new Error(`Invalid property ${key}`);
            if (schema.properties?.[key]) validateValue(item, schema.properties[key], `${path}.${key}`);
            else if (schema.additionalProperties !== true) throw new Error(`Unsupported ${path}.${key}`);
        }
    } else if (schema.type === 'array') {
        if (!Array.isArray(value) || value.length < (schema.minItems ?? 0) || value.length > (schema.maxItems ?? 128)) throw new Error(`Invalid ${path} array`);
        for (const item of value) validateValue(item, schema.items, path);
    } else if (schema.type === 'number' || schema.type === 'integer') {
        if (typeof value !== 'number' || !Number.isFinite(value) || (schema.type === 'integer' && !Number.isInteger(value)) || value < (schema.minimum ?? -Infinity) || value > (schema.maximum ?? Infinity)) throw new Error(`Invalid ${path} number`);
    } else if (schema.type === 'string') {
        if (typeof value !== 'string' || value.length > (schema.maxLength ?? 2048) || value.length < (schema.minLength ?? 0)) throw new Error(`Invalid ${path} text`);
    } else if (schema.type === 'boolean' && typeof value !== 'boolean') throw new Error(`${path} must be boolean`);
    else if (schema.type === 'null' && value !== null) throw new Error(`${path} must be null`);
}

/** @param {unknown} input @param {ObservationEnvelope} observation @returns {ActionPlan} */
export function validatePlan(input, observation) {
    if (!input || typeof input !== 'object') throw new Error('The model did not produce a plan');
    const p = /** @type {ActionPlan} */ (input);
    if (!['actions','answer','clarify'].includes(p.kind) || typeof p.message !== 'string' || p.message.length > 4000 || !Array.isArray(p.actions) || p.actions.length > LIMITS.actions) throw new Error('Invalid or over-budget model plan');
    if (p.kind !== 'actions' && p.actions.length) throw new Error('Only command plans may contain actions');
    if(p.continuation!==undefined && (p.kind!=='actions' || typeof p.continuation!=='string' || p.continuation.length>LIMITS.inputChars))throw new Error('Invalid command continuation');
    if (p.kind === 'actions' && !p.actions.length) throw new Error('An empty command plan cannot execute');
    if(p.experiment){
        validateValue(p.experiment,{type:'object',properties:{phase:{type:'string',enum:['act','observe','complete','clarify']},waitMs:{type:'integer',minimum:0,maximum:5000},reason:{type:'string',maxLength:600}},required:['phase','waitMs','reason'],additionalProperties:false},'experiment');
        const phase=p.experiment.phase;
        if(p.continuation || (phase==='act'?(p.kind!=='actions'||p.actions.length!==1):(p.actions.length!==0||p.kind!==(phase==='clarify'?'clarify':'answer'))))throw new Error('Invalid experiment phase plan');
        if(phase==='observe'?p.experiment.waitMs<250:p.experiment.waitMs!==0)throw new Error('Invalid experiment observation interval');
    }
    for (const action of p.actions) {
        if (!action || typeof action.type !== 'string' || Object.keys(action).some(k => !['type','args'].includes(k))) throw new Error('Invalid action');
        const descriptor = observation.actions?.find(item => item.type === action.type);
        if (!descriptor || !observation.capabilities.includes(action.type)) throw new Error(`Unavailable action: ${action.type}`);
        validateValue(action.args, descriptor.args);
    }
    return structuredClone(p);
}

/** @param {ObservationEnvelope|null} now @param {ObservationEnvelope} expected */
export function assertObservation(now, expected) {
    if (!now || now.workspace !== expected.workspace || now.ownerId !== expected.ownerId || now.preparationVersion !== expected.preparationVersion) throw new Error('The world changed while interpreting this request. Please send it again.');
}

/** Request state is deliberately bounded; never include credentials or a complete lattice dump.
 * @param {ObservationEnvelope} observation
 * @param {{actionTypes?:string[]}} [options]
 */
export function decisionObservation(observation,{actionTypes=[]}={}) {
    const facts=structuredClone(observation.facts);
    if(Array.isArray(facts.entities)) {facts.entityCount=facts.entities.length;facts.entities=facts.entities.slice(0,8);}
    // Render settings and GPU internals are not needed to decide physical actions.
    if(facts.settings) facts.settings={fov:facts.settings.fov,moveSpeed:facts.settings.moveSpeed,overlays:facts.settings.overlays};
    if(facts.scenarios){facts.scenarios=facts.scenarios.slice(0,8);facts.scenariosTruncated=(facts.scenarioCount??observation.facts.scenarios.length)>facts.scenarios.length;}
    const requested=new Set(actionTypes);
    const actions=(observation.actions||[]).filter(row=>requested.has(row.type) && observation.capabilities.includes(row.type))
        .map(row=>({type:row.type,description:row.description,args:structuredClone(row.args)}));
    return {workspace:observation.workspace,ownerId:observation.ownerId,preparationVersion:observation.preparationVersion,tick:observation.tick,
        selected:observation.selected,facts,capabilities:observation.capabilities,...(actions.length?{actions}:{})};
}
