// @ts-check
import {validateValue} from './contracts.js';

/** Split only at an explicit next command verb, never inside a vector or name.
 * @param {string} text
 */
export function commandClauses(text) {
    const verb='(?:pause|advance|step|resume|set|apply|show|hide|load|create|delete|rename|disable|enable|explain|describe|switch|observe|watch|monitor|measure|record|test|investigate|explore|find|track|keep|return(?=\\s+to\\s+(?:lattice|mind)))';
    const boundary=new RegExp(`(?:,\\s*(?:and\\s+|then\\s+)?|\\s+(?:and|then)\\s+)(?=${verb}\\b)`,'ig');
    const clauses=[];let start=0;
    for(const match of text.matchAll(boundary)) {
        const before=text.slice(0,match.index);
        if((before.match(/"/g)||[]).length%2 || before.lastIndexOf('[')>before.lastIndexOf(']'))continue;
        clauses.push(text.slice(start,match.index).trim());start=match.index+match[0].length;
    }
    clauses.push(text.slice(start).trim());
    // A leading condition describes observed state; it is not a requested write.
    return clauses.filter(clause=>!/^while\b/i.test(clause)||/\b(advance|step|set|apply|resume|pause)\b/i.test(clause));
}

/** Limit the small model to properties actually mentioned in this command.
 * This is language grounding, not an additional source of simulation authority.
 * @param {string} text @param {any} observation @param {any[]} candidates
 */
export function proposalDescriptors(text,observation,candidates) {
    const t=text.toLowerCase();
    const mappings={
        'observer.world':{profile:/(?:change|switch|set).{0,20}profile/,gravityMode:/gravity.*(?:toward|central plane|uniform)/,gravityStrength:/gravity.*(?:strength|strong|weaker|zero)/,gravity:/gravity.*\[/,objectCollisions:/collision/,planeCollision:/collision.*plane|plane.*collision/},
        'observer.forceGun':{enabled:/enable|disable|turn/,sensitivity:/sensitivity|delicate|normal|strong/,multiplier:/multiplier/,release:/release/},
        'observer.environment':{preset:/environment|shell/,radius:/radius/,density:/density/,spacing:/spacing/,opacity:/opacity/,seed:/seed/,orientation:/orientation/,animationRate:/animation|rate/,color:/color|colour/,anchor:/anchor/},
        'observer.overlay':{layer:/layer|axes|grid|bounds|trajectory/,field:/field/,filter:/filter|all|none/,id:/selected/,enabled:/show|hide|enable|disable|on|off/},
        'observer.camera':{mode:/.*/,id:/look at/,position:/position/,yaw:/yaw/,pitch:/pitch/,roll:/roll/,fov:/fov|field of view/,speed:/speed/,acceleration:/acceleration/,grounded:/ground|flight|fly/,worldUp:/world.?up/,optical:/optical/,doppler:/doppler/,beaming:/beaming/,artisticShading:/shading/,renderScale:/resolution|render scale/,autoQuality:/quality/},
    };
    return candidates.map(candidate=>{
        const action=structuredClone(candidate);
        const map=/** @type {Record<string,RegExp>|undefined} */(/** @type {any} */(mappings)[action.type]);
        if(map){
            action.args.properties=Object.fromEntries(Object.entries(action.args.properties).filter(([key])=>map[key]?.test(t)));
            if(action.type==='observer.world' && /collision.*plane|plane.*collision/.test(t))delete action.args.properties.objectCollisions;
        }
        /** @param {string} key @param {any} value */
        const bind=(key,value)=>{
            const schema=action.args.properties[key];if(!schema)return;
            validateValue(value,schema,key);action.args.properties[key]={...schema,enum:[value]};
            action.args.required=[...new Set([...(action.args.required||[]),key])];
        };
        /** @param {string} key @param {any[]} [catalog] */
        const bindChoice=(key,catalog=[])=>{
            const choices=action.args.properties[key]?.enum;if(!choices)return;
            const match=choices.filter((/** @type {string} */ value)=>new RegExp(`\\b${value.replace(/[.*+?^${}()|[\]\\]/g,'\\$&')}\\b`,'i').test(t)
                || catalog.some(item=>item.id===value && t.includes(item.label.toLowerCase())));
            if(match.length===1)bind(key,match[0]);
        };
        for(const [key,schema] of Object.entries(action.args.properties)) {
            if(/** @type {any} */(schema).type==='boolean' && /\b(disable|off|hide)\b/.test(t))bind(key,false);
            else if(/** @type {any} */(schema).type==='boolean' && /\b(enable|on|show)\b/.test(t))bind(key,true);
        }
        if(action.type==='observer.world')bindChoice('gravityMode');
        if(action.type==='observer.forceGun')bindChoice('sensitivity');
        if(action.type==='observer.select' && /clear|deselect/.test(t))bind('id',null);
        if(action.type==='observer.create') {
            if(/\bcube\b/.test(t))bind('shape','box');else bindChoice('shape',observation.catalogs?.shapes);
            if(!action.args.properties.shape?.enum || action.args.properties.shape.enum.length!==1)throw new Error('Choose a supported geometric shape from the object library.');
        }
        if(action.type==='observer.environment')bindChoice('preset',observation.catalogs?.environments);
        if(action.type==='observer.preset'){
            bindChoice('preset',observation.catalogs?.experiments);
            if(/approaching.*receding.*clock/.test(t))bind('preset','clocks');
        }
        if(action.type==='observer.overlay'){bindChoice('layer',observation.catalogs?.layers);bindChoice('filter');}
        if(action.type==='observer.camera')bind('mode',/look at/.test(t)?'lookAt':/position|yaw|pitch/.test(t)?'pose':'view');
        if(action.type==='lattice.setToggle')bindChoice('name');
        const name=text.match(/(?:rename.*?\bto|named?)\s+["“]([^"”]+)["”]/i);if(name)bind('name',name[1]);
        const scenario=text.match(/\bid\s+([\w-]+)/i);if(scenario&&action.type==='lattice.scenario')bind('scenarioId',scenario[1]);
        if(action.type==='lattice.catalog'){
            const query=text.match(/(?:for|matching)\s+["“]([^"”]+)["”]/i);if(query)bind('query',query[1]);
            const limit=t.match(/(?:at most|limit)\s+(\d+)/);if(limit)bind('limit',Number(limit[1]));
        }
        if(action.type==='lattice.seed.preview'){
            const recipe=text.match(/\{[\s\S]*\}/)?.[0];
            if(!recipe)throw new Error('Supply a seed recipe to preview; no current recipe was provided.');
            JSON.parse(recipe);bind('recipeJson',recipe);
        }
        if(action.type==='lattice.seed.apply'){
            const id=text.match(/preview(?:Id| id)?\s*[:=]?\s*["']?([\w:-]{8,})/i)?.[1];
            if(!id)throw new Error('Preview a seed in this session first, then provide its preview ID.');
            bind('previewId',id);
        }
        for(const [key,pattern] of [['fov',/(?:fov|field of view)\s+(?:to\s+)?([\d.]+)/],['multiplier',/multiplier\s+(?:to\s+)?([\d.]+)/]]){
            const value=t.match(/** @type {RegExp} */(pattern));if(value)bind(/** @type {string} */(key),Number(value[1]));
        }
        return action;
    });
}
