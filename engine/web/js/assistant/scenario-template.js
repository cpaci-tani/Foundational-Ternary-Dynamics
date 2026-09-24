// @ts-check
/** Data-only preparation contracts. This module neither creates an owner nor advances physics. */
import {cloneRecipe,markFiniteOverride,setAtPath,validateRecipe} from '../seeding/recipe.js';
import {propertyError} from '../seeding/property-schema.js';
import {validateScenarioProtocol} from './scenario-limits.js';

/** @typedef {string|number|boolean} ScenarioScalar */
/** @typedef {{key:string,value:ScenarioScalar}} ScenarioSetting */
/** @typedef {{name:string,goal:string,settings:ScenarioSetting[],ticks:number,sampleEvery:number,measurements:string[]}} ScenarioDraft */
/** @typedef {{key:string,path:(string|number)[],type:string,min:number,max:number,options?:[ScenarioScalar,string][],units?:string}} ScenarioProperty */
/** @typedef {{scenarioId:string,backend:'finite-records'|'effective-lattice',size:number,recipe:Record<string,any>,properties:ScenarioProperty[],allowedMeasurements:string[],measurementDefinitions?:Record<string,any>}} ScenarioTemplate */

const FORBIDDEN=new Set(['__proto__','prototype','constructor']);
const DRAFT_KEYS=['name','goal','settings','ticks','sampleEvery','measurements'];
const FIELD=/^[A-Za-z][A-Za-z0-9_]{0,63}$/;

/** Reject executable/accessor values before reading them or cloning their containing object.
 * @param {unknown} value @param {string} name
 */
function assertData(value,name){
    const active=new Set();let nodes=0;
    /** @param {unknown} item @param {number} depth */
    const visit=(item,depth)=>{
        if(++nodes>100000 || depth>32)throw new Error(`${name} exceeds the plain-data limit.`);
        if(item===null || typeof item==='string' || typeof item==='boolean')return;
        if(typeof item==='number'){if(Number.isFinite(item))return;throw new Error(`${name} contains a non-finite number.`);}
        if(typeof item!=='object')throw new Error(`${name} must contain plain JSON data only.`);
        const array=Array.isArray(item),prototype=Object.getPrototypeOf(item);
        if(array?prototype!==Array.prototype:prototype!==Object.prototype && prototype!==null)
            throw new Error(`${name} must contain plain objects and arrays only.`);
        if(active.has(item))throw new Error(`${name} contains a circular reference.`);
        active.add(item);
        const descriptors=Object.getOwnPropertyDescriptors(item);
        if(Reflect.ownKeys(item).some(key=>typeof key!=='string'))throw new Error(`${name} cannot contain symbol keys.`);
        if(array && Object.keys(item).length!==/** @type {any[]} */(item).length)throw new Error(`${name} contains an incomplete or decorated array.`);
        for(const [key,descriptor] of Object.entries(descriptors)){
            if(array && key==='length')continue;
            if(FORBIDDEN.has(key) || array && !/^(0|[1-9]\d*)$/.test(key))throw new Error(`${name} contains a forbidden property.`);
            if(!descriptor.enumerable || !Object.hasOwn(descriptor,'value'))throw new Error(`${name} cannot contain accessors or hidden properties.`);
            visit(descriptor.value,depth+1);
        }
        active.delete(item);
    };
    visit(value,0);
}

/** @param {unknown} value @param {string} name @returns {Record<string,any>} */
function record(value,name){
    if(!value || typeof value!=='object' || Array.isArray(value))throw new Error(`${name} must be an object.`);
    return /** @type {Record<string,any>} */(value);
}
/** @param {Record<string,any>} value @param {string[]} keys @param {string} name */
function exactKeys(value,keys,name){
    if(Object.keys(value).length!==keys.length || keys.some(key=>!Object.hasOwn(value,key)))throw new Error(`${name} has unexpected or missing properties.`);
}
/** @param {unknown} value @param {number} limit @param {string} name @returns {string} */
function text(value,limit,name){
    if(typeof value!=='string' || !value.trim() || value.length>limit)throw new Error(`${name} must contain 1 to ${limit} characters.`);
    return value.trim();
}
/** @param {unknown} value @param {number} min @param {number} max @param {string} name @returns {number} */
function integer(value,min,max,name){
    if(typeof value!=='number' || !Number.isSafeInteger(value) || value<min || value>max)throw new Error(`${name} must be an exact integer from ${min} to ${max}.`);
    return value;
}
/** @param {unknown} value @returns {value is ScenarioScalar} */
function scalar(value){return typeof value==='boolean' || typeof value==='string' && value.length<=2048 || typeof value==='number' && Number.isFinite(value);}
/** @template T @param {T} value @returns {T} */
function frozen(value){
    if(value && typeof value==='object'){for(const child of Object.values(value))frozen(child);Object.freeze(value);}
    return value;
}

/** The registry/compiler supplies the template; identity and size are immutable.
 * @param {unknown} input @returns {ScenarioTemplate}
 */
function template(input){
    assertData(input,'Template');const value=record(input,'Template');
    text(value.scenarioId,120,'Template scenario');
    if(!['finite-records','effective-lattice'].includes(value.backend))throw new Error('Template backend is unsupported.');
    integer(value.size,3,256,'Template size');
    const recipe=record(value.recipe,'Template recipe');
    if(recipe.scenarioId!==value.scenarioId || recipe.size!==value.size)throw new Error('Template recipe identity or size does not match its description.');
    validateRecipe(recipe,[{id:value.scenarioId,backend:value.backend,sizes:[value.size]}]);
    if(!Array.isArray(value.properties) || value.properties.length>4096)throw new Error('Template properties are unavailable or exceed the limit.');
    const keys=new Set();
    for(const row of value.properties){
        const p=record(row,'Template property');const key=text(p.key,240,'Template property key');
        if(key!==p.key || keys.has(key) || key.split('.').some(part=>FORBIDDEN.has(part)))throw new Error('Template property keys must be unique, exact and safe.');keys.add(key);
    }
    if(!Array.isArray(value.allowedMeasurements) || !value.allowedMeasurements.length || value.allowedMeasurements.length>64
        || value.allowedMeasurements.some((/** @type {unknown} */ field)=>typeof field!=='string' || !FIELD.test(field) || FORBIDDEN.has(field))
        || new Set(value.allowedMeasurements).size!==value.allowedMeasurements.length)throw new Error('Template measurements must be an explicit unique field allowlist.');
    return /** @type {ScenarioTemplate} */(value);
}

/** A descriptor can address only existing mutable scalar fields or a native override map.
 * @param {ScenarioProperty} p @param {ScenarioTemplate} source @param {ScenarioScalar} value
 */
function validateSetting(p,source,value){
    const path=p.path;
    if(!Array.isArray(path) || !path.length || path.length>16 || path.some(part=>
        typeof part==='number' ? !Number.isSafeInteger(part) || part<0
            : typeof part!=='string' || !part || part.split('.').some(piece=>FORBIDDEN.has(piece))))throw new Error(`Unsafe property path: ${p.key}.`);
    const finite=source.backend==='finite-records';
    const rootSeed=finite && path.length===1 && path[0]==='randomSeed';
    const nativeOverride=!finite && path.length===2 && path[0]==='overrides' && typeof path[1]==='string'
        && !/^(?:recipe|ingredient)\./.test(path[1]);
    const component=path[0]==='components' && typeof path[1]==='number' && path[1]<source.recipe.components.length;
    const enabled=component && path.length===3 && path[2]==='enabled';
    const finiteProperty=finite && component && path.length>=4 && ['region','parameters'].includes(String(path[2]));
    const componentOverride=!finite && component && path.length===4 && path[2]==='overrides' && typeof path[3]==='string'
        && !/^(?:recipe|ingredient)\./.test(path[3]);
    if(!(rootSeed || nativeOverride || enabled || finiteProperty || componentOverride))throw new Error(`Property cannot change preparation identity or structure: ${p.key}.`);
    let node=source.recipe;
    for(const part of path.slice(0,-1)){
        if(!node || typeof node!=='object' || !Object.hasOwn(node,part))throw new Error(`Property parent is unavailable: ${p.key}.`);
        node=node[part];
    }
    const leaf=path[path.length-1];
    if(!node || typeof node!=='object' || !(nativeOverride || componentOverride) && (!Object.hasOwn(node,leaf) || !scalar(node[leaf])))
        throw new Error(`Property must address an existing scalar: ${p.key}.`);
    if(enabled && typeof value!=='boolean')throw new Error(`Ingredient activation must be boolean: ${p.key}.`);
    if(!['real','integer','choice'].includes(p.type) || !Number.isFinite(p.min) || !Number.isFinite(p.max) || p.min>p.max)
        throw new Error(`Property bounds or type are unavailable: ${p.key}.`);
    if(p.options!==undefined && (!Array.isArray(p.options) || p.options.some(option=>!Array.isArray(option) || option.length!==2 || !scalar(option[0]) || typeof option[1]!=='string')))
        throw new Error(`Property choices are invalid: ${p.key}.`);
    if(p.type==='choice' && !p.options?.length)throw new Error(`Property choices are unavailable: ${p.key}.`);
    const error=propertyError(p,value);if(error)throw new Error(`${p.key}: ${error}`);
    if(p.units!==undefined && (typeof p.units!=='string' || p.units.length>120))throw new Error(`Property units are invalid: ${p.key}.`);
}

/** Validate only data selected from the compiler's current descriptor catalog.
 * Coupled constructor constraints remain the preparation compiler's responsibility.
 * @param {unknown} draft @param {unknown} description @returns {ScenarioDraft}
 */
export function validateScenarioDraft(draft,description){
    const source=template(description);assertData(draft,'Draft');const input=record(draft,'Draft');exactKeys(input,DRAFT_KEYS,'Draft');
    const name=text(input.name,120,'Name'),goal=text(input.goal,2000,'Goal');
    const {ticks,sampleEvery}=validateScenarioProtocol(input.ticks,input.sampleEvery);
    if(!Array.isArray(input.settings) || input.settings.length>12)throw new Error('Choose at most 12 changed settings.');
    const seen=new Set(),paths=new Set();
    /** @type {ScenarioSetting[]} */const settings=[];
    for(const row of input.settings){
        const setting=record(row,'Setting');exactKeys(setting,['key','value'],'Setting');
        if(typeof setting.key!=='string' || seen.has(setting.key))throw new Error('Setting keys must be unique strings.');
        const property=source.properties.find(p=>p.key===setting.key);
        if(!property)throw new Error(`Unknown setting: ${setting.key}.`);
        if(!scalar(setting.value))throw new Error(`Setting ${setting.key} must be a finite scalar.`);
        validateSetting(property,source,setting.value);
        const identity=JSON.stringify(property.path);if(paths.has(identity))throw new Error('Settings cannot address the same property twice.');
        paths.add(identity);seen.add(setting.key);settings.push({key:setting.key,value:setting.value});
    }
    if(!Array.isArray(input.measurements) || !input.measurements.length || input.measurements.length>12
        || input.measurements.some((/** @type {unknown} */ field)=>typeof field!=='string' || !source.allowedMeasurements.includes(field))
        || new Set(input.measurements).size!==input.measurements.length)throw new Error('Choose 1 to 12 unique measurements from the template allowlist.');
    return frozen({name,goal,settings,ticks,sampleEvery,measurements:[...input.measurements]});
}

/** A single immutable document shared with the local model, JEV, and the runner.
 * @param {unknown} description @param {unknown} draft
 */
export function buildScenarioRecipe(description,draft){
    const source=template(description),valid=validateScenarioDraft(draft,source),recipe=cloneRecipe(source.recipe);
    const settings=valid.settings.map(setting=>{
        const property=/** @type {ScenarioProperty} */(source.properties.find(p=>p.key===setting.key));
        setAtPath(recipe,property.path,setting.value);
        if(source.backend==='finite-records')markFiniteOverride(recipe,property.path);
        return {...setting,units:property.units || ''};
    });
    validateRecipe(recipe,[{id:source.scenarioId,backend:source.backend,sizes:[source.size]}]);
    const definitions=source.measurementDefinitions;
    const settingsSchema=valid.settings.map(setting=>{
        const p=/** @type {ScenarioProperty} */(source.properties.find(row=>row.key===setting.key));
        return{key:p.key,path:[...p.path],type:p.type,min:p.min,max:p.max,units:p.units||'',...(p.options?.length?{options:structuredClone(p.options)}:{})};
    });
    return frozen({schemaVersion:1,name:valid.name,goal:valid.goal,start:'empty',scenarioId:source.scenarioId,
        backend:source.backend,size:source.size,recipe,settings,settingsSchema,protocol:{ticks:valid.ticks,sampleEvery:valid.sampleEvery},measurements:[...valid.measurements],
        ...(definitions?{measurementDefinitions:Object.fromEntries(valid.measurements.filter(key=>Object.hasOwn(definitions,key)).map(key=>[key,structuredClone(definitions[key])]))}:{})});
}

/** Blank start for the same constructor and size; all authored ingredients stay inactive.
 * @param {unknown} description @returns {Record<string,any>}
 */
export function buildEmptyScenarioRecipe(description){
    const source=template(description),recipe=cloneRecipe(source.recipe);recipe.blank=true;
    delete recipe.explicitOverrides;
    for(const component of recipe.components){component.enabled=false;if(source.backend==='effective-lattice')component.overrides={};}
    if(source.backend==='effective-lattice'){
        recipe.overrides={};
        if(!recipe.components.some((/** @type {any} */ component)=>component.scenarioId===source.scenarioId)){
            let id='assistant-empty-anchor';while(recipe.components.some((/** @type {any} */ component)=>component.id===id))id+='-0';
            recipe.components.push({id,kind:'native',enabled:false,scenarioId:source.scenarioId,overrides:{}});
        }
    }
    validateRecipe(recipe,[{id:source.scenarioId,backend:source.backend,sizes:[source.size]}]);
    return frozen(recipe);
}
