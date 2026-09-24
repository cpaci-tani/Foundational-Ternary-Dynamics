// @ts-check
import {decisionObservation,validatePlan} from './contracts.js';

/** Leave room under the proxy's 64 KiB request ceiling. */
export const DECISION_REQUEST_MAX_BYTES=60*1024;
const encoder=new TextEncoder();
const SECRET_KEYS=new Set(['apikey','typesafeapikey','browsertoken','clienttoken','password','secret','accesstoken','refreshtoken','credentials']);
const SCENARIO_KEYS=['id','scenarioId','label','name','title','category','backend','sizes','description','intent','qualification','epistemicStatus','admissionStatus','evidenceLevel'];

/** Clone bounded JSON data, never object internals or credential-bearing fields.
 * Protocol authorization is a data object; HTTP Authorization is a secret string.
 * @param {any} value @returns {any}
 */
function publicData(value){
    let nodes=0;const active=new Set();
    /** @param {any} item @param {number} depth @returns {any} */
    const visit=(item,depth)=>{
        if(++nodes>24000 || depth>32)throw new Error('JEV evidence exceeds the plain-data limit.');
        if(item===undefined)return undefined;
        if(item===null || typeof item==='string' || typeof item==='boolean')return item;
        if(typeof item==='number' && Number.isFinite(item))return item;
        if(!item || typeof item!=='object' || active.has(item))throw new Error('JEV evidence must contain finite plain JSON data.');
        const array=Array.isArray(item),prototype=Object.getPrototypeOf(item);
        if(array?prototype!==Array.prototype:prototype!==Object.prototype && prototype!==null)throw new Error('JEV evidence must contain plain objects and arrays.');
        active.add(item);
        /** @type {any} */const copy=array?[]:{};
        for(const [key,descriptor] of Object.entries(Object.getOwnPropertyDescriptors(item))){
            if(array && key==='length')continue;
            if(!descriptor.enumerable || !Object.hasOwn(descriptor,'value'))throw new Error('JEV evidence cannot contain accessors or hidden properties.');
            const normalized=key.replace(/[-_]/g,'').toLowerCase();
            if(['__proto__','prototype','constructor'].includes(key) || SECRET_KEYS.has(normalized)
                || normalized==='authorization' && typeof descriptor.value==='string')continue;
            const child=visit(descriptor.value,depth+1);
            if(child!==undefined)copy[key]=child;
        }
        active.delete(item);return copy;
    };
    return visit(value,0);
}

/** @param {any} observation */
const fence=observation=>observation?{workspace:observation.workspace,ownerId:observation.ownerId,preparationVersion:observation.preparationVersion,tick:observation.tick??null}:null;

/** Only confirmed receipts for this preparation are evidence for a later request.
 * @param {any[]} receipts @param {import('./contracts.js').ObservationEnvelope} observation
 */
function confirmedReceipts(receipts,observation){
    return receipts.filter(receipt=>receipt?.status==='applied' && receipt.after?.workspace===observation.workspace
        && receipt.after.ownerId===observation.ownerId && receipt.after.preparationVersion===observation.preparationVersion).slice(-4).map(receipt=>{
        const action={type:receipt.action?.type};
        const args=receipt.action?.args;
        const compact=args && typeof args==='object' && !Object.hasOwn(args,'recipeJson') && encoder.encode(JSON.stringify(args)).length<=2048;
        return{status:'applied',action:{...action,...(compact?{args}:args?{argsOmitted:true}:{})},before:fence(receipt.before),after:fence(receipt.after),
            ...(receipt.completedTicks!==undefined?{completedTicks:receipt.completedTicks}:{})};
    });
}

/** Resolve opaque apply IDs only from this preparation's latest successful
 * controller preview. A new preview replaces the previous staged recipe.
 * @param {any[]} receipts @param {any} observation @param {any[]} actions
 */
function preparedSeedEvidence(receipts,observation,actions){
    const ids=actions.filter(action=>action.type==='lattice.seed.apply').map(action=>action.args.previewId);
    if(!ids.length)return [];
    const latest=receipts.filter(receipt=>receipt?.status==='applied' && receipt.action?.type==='lattice.seed.preview'
        && receipt.after?.workspace===observation.workspace && receipt.after.ownerId===observation.ownerId
        && receipt.after.preparationVersion===observation.preparationVersion).at(-1);
    return ids.map(previewId=>{
        if(!latest || latest.result?.previewId!==previewId)return{previewId,available:false,
            reason:'No matching latest confirmed preview for this owner and preparation.'};
        let recipe;
        try{recipe=JSON.parse(latest.action.args.recipeJson);}catch{throw new Error('Confirmed seed preview lacks its complete recipe.');}
        if(!recipe || recipe.scenarioId!==latest.result.scenarioId || recipe.size!==latest.result.size)
            throw new Error('Confirmed seed preview metadata disagrees with its recipe.');
        if(JSON.stringify(publicData(recipe))!==JSON.stringify(recipe))
            throw new Error('Required seed preview evidence contains unsupported or credential-bearing fields.');
        return{previewId,available:true,source:'confirmed active-controller seed preparation',
            validation:'Recipe accepted for staging; not yet applied.',preparedAgainst:fence(latest.after),recipe};
    });
}

/** Build the complete decision request from controller-owned evidence. The
 * caller supplies registry entries, never model-authored descriptions. Required
 * plan/recipe/protocol evidence is retained exactly or rejected before dispatch.
 * @param {string} intent
 * @param {import('./contracts.js').ObservationEnvelope} observation
 * @param {import('./contracts.js').ActionPlan} plan
 * @param {{scenarioCatalog?:any[],receipts?:any[],experimentContext?:any}} [options]
 */
export function buildDecisionRequest(intent,observation,plan,{scenarioCatalog,receipts=[],experimentContext}={}){
    const validated=validatePlan(plan,observation);
    const protocol=experimentContext??observation.facts.scenarioExperiment;
    const types=new Set(validated.actions.map(action=>action.type));
    for(const operation of protocol?.operations||[])if(typeof operation?.type==='string')types.add(operation.type);
    if(types.size>24)throw new Error('JEV protocol references too many operation types.');
    const received=decisionObservation(observation,{actionTypes:[...types]});
    const prepared=preparedSeedEvidence(receipts,observation,validated.actions);
    if(prepared.length)received.facts.preparedSeeds=prepared;
    // A short catalog sample cannot establish whether a requested target exists.
    // Replace it with explicit evidence for the requested/current scenarios only.
    delete received.facts.scenarios;delete received.facts.scenariosTruncated;
    const ids=new Set();
    const add=(/** @type {any} */ value)=>{if(typeof value==='string' && value.length && value.length<=160)ids.add(value);};
    if(observation.workspace==='lattice')add(observation.facts.scenarioId);
    for(const action of validated.actions){
        if(action.type.startsWith('lattice.'))add(action.args.scenarioId);
        if(action.type==='lattice.seed.preview' && typeof action.args.recipeJson==='string'){
            try{add(JSON.parse(action.args.recipeJson).scenarioId);}catch{throw new Error('JEV seed evidence requires a valid recipe.');}
        }
    }
    add(protocol?.template?.scenarioId);add(protocol?.preparation?.scenarioId);add(protocol?.emptyPreparation?.scenarioId);
    for(const evidence of prepared)add(evidence.recipe?.scenarioId);
    if(ids.size>16)throw new Error('JEV request references too many scenario targets.');
    if(ids.size){
        received.facts.scenarioEvidence={source:'active-controller registry',catalogAvailable:Array.isArray(scenarioCatalog),
            targets:[...ids].map(id=>{
                const row=scenarioCatalog?.find(item=>(item.id??item.scenarioId)===id);
                return{scenarioId:id,registered:row?true:Array.isArray(scenarioCatalog)?false:null,
                    ...(row?{descriptor:Object.fromEntries(SCENARIO_KEYS.filter(key=>Object.hasOwn(row,key)).map(key=>[key,row[key]]))}:{})};
            })};
    }
    received.facts.decisionScope={kind:protocol?'fixed-scenario-protocol':validated.experiment?'live-experiment-step':'explicit-plan',
        proposedActions:validated.actions.length,actionSchemasSource:'current controller capabilities'};
    const confirmed=confirmedReceipts(receipts,observation);
    if(confirmed.length)received.facts.confirmedReceipts=confirmed;
    if(protocol!==undefined)received.facts.scenarioExperiment=protocol;
    for(const required of [validated,protocol])if(required!==undefined && JSON.stringify(publicData(required))!==JSON.stringify(required))
        throw new Error('Required JEV plan or protocol contains credential-bearing or unsupported fields.');
    const request=publicData({intent,observation:received,plan:validated});
    if(encoder.encode(JSON.stringify(request)).length>DECISION_REQUEST_MAX_BYTES)
        throw new Error('Required JEV plan and evidence exceed the 60 KiB decision budget. Reduce the preparation or protocol before retrying.');
    return /** @type {{intent:string,observation:any,plan:import('./contracts.js').ActionPlan}} */(request);
}
