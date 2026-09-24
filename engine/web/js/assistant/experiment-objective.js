// @ts-check
import {normalizeRequest} from './request-router.js';

/** @typedef {{kind:'tick-interval',requestedTicks:string,compare:boolean,keepPaused:boolean,workspace?:'lattice'|'observer'}} TickIntervalObjective */

const numberWords = ['zero','one','two','three','four','five','six','seven','eight','nine','ten',
    'eleven','twelve','thirteen','fourteen','fifteen','sixteen','seventeen','eighteen','nineteen','twenty'];
const countPattern = `(?:[0-9]+|${numberWords.join('|')})`;
const objectivePattern = new RegExp(
    `^(?:(?:run|conduct|perform|start|carry out) (?:an? |the )?(?:(?:live|bounded) )?experiment (?:to )?)?`
    + `(?:advance|step) (?:(?:the |this )?(?:lattice|simulation|observer) )?(?:by )?(?:exactly )?(${countPattern}) (?:lattice |observer )?ticks?`
    + `(?: (?:in )?total)?(?: from (?:the )?baseline)?`
    + `(?:,? (?:then|and(?: then)?) (compare (?:the )?(?:before and after|before/after|before-after) (?:measurements|observations)))?`
    + `(?:(?:[.;] |,? (?:and|then) )(keep (?:playback|the simulation) paused))?[.!]?$`, 'i');

/** Recognise only a complete, narrowly specified interval objective. Unknown
 * extra work, multiple intervals and physical time units remain general goals.
 * @param {string} goal @returns {TickIntervalObjective|null}
 */
export function parseExperimentObjective(goal) {
    const text = normalizeRequest(goal).text.replace(/\s+/g, ' ');
    const match = text.match(objectivePattern);
    if (!match) return null;
    const workspaces = new Set([...text.toLowerCase().matchAll(/\b(lattice|observer)\b/g)].map(row => row[1]));
    if (workspaces.size > 1) return null;
    const workspace = workspaces.has('lattice') ? 'lattice' : workspaces.has('observer') ? 'observer' : null;
    const raw = match[1].toLowerCase();
    const ticks = /^\d+$/.test(raw) ? BigInt(raw) : BigInt(numberWords.indexOf(raw));
    if (ticks <= 0n) return null;
    return {kind:'tick-interval',requestedTicks:ticks.toString(),compare:!!match[2],keepPaused:!!match[3],
        ...(workspace ? {workspace} : {})};
}

/** Never round an unsafe JS number into a seemingly exact engine counter.
 * @param {unknown} value @returns {bigint|null}
 */
function exactTick(value) {
    if (typeof value === 'bigint') return value >= 0n ? value : null;
    if (typeof value === 'number') return Number.isSafeInteger(value) && value >= 0 ? BigInt(value) : null;
    if (typeof value === 'string' && /^\d+$/.test(value)) return BigInt(value);
    return null;
}

/** Exact progress is meaningful only within the original preparation. All
 * counters returned here are strings; none of these facts choose a next action.
 * @param {TickIntervalObjective|null} objective @param {any} baseline @param {any} current
 */
export function experimentObjectiveProgress(objective, baseline, current) {
    const requested = objective?.kind === 'tick-interval' ? exactTick(objective.requestedTicks) : null;
    const before = exactTick(baseline?.tick), now = exactTick(current?.tick);
    const sample = exactTick(current?.facts?.sampleTick ?? current?.tick);
    const initialSample = exactTick(baseline?.facts?.sampleTick ?? baseline?.tick);
    const result = {valid:false,reason:'',requestedTicks:requested?.toString() ?? null,
        baselineTick:before?.toString() ?? null,currentTick:now?.toString() ?? null,
        targetTick:/** @type {string|null} */(null),elapsedTicks:/** @type {string|null} */(null),
        remainingTicks:/** @type {string|null} */(null),reached:false,overshot:false,
        sampleTick:sample?.toString() ?? null,measurementsReady:false,paused:current?.facts?.running === false};
    if (requested === null || requested <= 0n) return {...result,reason:'No positive exact tick objective is available.'};
    if (objective?.workspace && (objective.workspace !== baseline?.workspace || objective.workspace !== current?.workspace)) {
        return {...result,reason:`The tick objective names ${objective.workspace}, but that workspace is not the experiment's active workspace.`};
    }
    for (const key of ['workspace','ownerId','preparationVersion']) {
        if (typeof baseline?.[key] !== 'string' || !baseline[key] || baseline[key] !== current?.[key]) {
            return {...result,reason:`The experiment ${key} changed or is unavailable.`};
        }
    }
    if (before === null || now === null) return {...result,reason:'An exact engine tick is unavailable.'};
    if (now < before) return {...result,reason:'The engine tick precedes the experiment baseline.'};
    const elapsed = now - before, remaining = requested > elapsed ? requested - elapsed : 0n;
    const measurementsReady = initialSample === before && sample === now
        && baseline?.facts?.available !== false && baseline?.facts?.stale !== true
        && current?.facts?.available !== false && current?.facts?.stale !== true;
    return {...result,valid:true,reason:'',targetTick:(before + requested).toString(),elapsedTicks:elapsed.toString(),
        remainingTicks:remaining.toString(),reached:elapsed === requested,overshot:elapsed > requested,measurementsReady};
}

/** Validate a typed proposal against the user's exact interval. This adds only
 * a constraint: executor schemas, evidence checks and JEV still govern it.
 * @param {TickIntervalObjective|null} objective @param {any} baseline @param {any} current @param {any} proposal
 */
export function validateExperimentObjectiveProposal(objective, baseline, current, proposal) {
    const progress = experimentObjectiveProgress(objective, baseline, current);
    const reject = (/** @type {string} */ reason) => ({valid:false,reason,progress});
    const accept = () => ({valid:true,reason:'',progress});
    if (!progress.valid) return reject(progress.reason);
    const phase = proposal?.experiment?.phase ?? proposal?.phase ?? (proposal?.kind === 'actions' ? 'act' : null);
    const actions = proposal?.actions ?? [];
    if (phase !== 'act' && actions.length) return reject('A non-action phase cannot contain actions.');
    if (phase === 'clarify') return accept();
    if (progress.overshot) return reject('The exact tick interval has already been exceeded.');
    if (phase === 'complete') {
        if (!progress.reached) return reject('The requested tick interval has not completed.');
        if (!progress.measurementsReady) return reject('Completed baseline and current diagnostic measurements are required.');
        if (!progress.paused) return reject('Playback must be paused before an exact interval can conclude.');
        return accept();
    }
    if (phase === 'observe') {
        return progress.paused && progress.measurementsReady
            ? reject('A paused world with current measurements cannot advance by waiting.') : accept();
    }
    if (phase !== 'act' || !Array.isArray(actions) || actions.length !== 1) return reject('One typed experimental action is required.');
    const action = actions[0];
    if (action?.type === `${current.workspace}.pause`) return accept();
    if (action?.type !== `${current.workspace}.step`) return reject('That action is outside this exact tick-interval objective.');
    if (!progress.paused) return reject('Pause playback before advancing an exact experimental interval.');
    const count = exactTick(action?.args?.count);
    if (count === null || count <= 0n) return reject('A positive exact step count is required.');
    if (count > BigInt(/** @type {string} */(progress.remainingTicks))) return reject('The proposed step exceeds the remaining requested ticks.');
    return accept();
}
