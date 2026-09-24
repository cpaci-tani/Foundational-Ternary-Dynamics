// @ts-check
/** App-owned controls: no model, no independent clock, no physics shadow. */
/** @typedef {import('./contracts.js').AssistantAction} AssistantAction */
/** @typedef {import('./contracts.js').ObservationEnvelope} ObservationEnvelope */
/** @typedef {import('./contracts.js').CommandReceipt} CommandReceipt */
/** @typedef {{signal?:AbortSignal,assertActive?:()=>void,expected?:ObservationEnvelope}} ExecutionOptions */
/** @typedef {{getCtx:()=>any,isActive?:()=>boolean,loadScenario?:((ctx:any,id:string,params?:any)=>void)|null,
 * getOwner?:(ctx:any)=>any,getState?:()=>any,getScenarios?:()=>any[],getQualification?:()=>any,getCatalogStatus?:()=>any,
 * seedServices?:()=>Promise<any>,seedRuntime?:()=>Promise<any>,
 * seedDescriptions?:()=>Promise<any>,finiteCatalog?:()=>Promise<any>}} LatticeControlOptions */
import { getActiveScale0Bridge, getScale0State, getScale0QualificationState,
    commitScale0ScientificMutation, setLatticeNeedsUpload, markFieldDirty } from '../scales/scale0/state/store.js';
import { getScale0SeedingScenarios } from '../scales/scale0/scenario-registry.js';
import { SCALE0_TOGGLES, SCALE0_ADVANCED_TOGGLES } from '../config/toggles.js';
import { LatticeObservationAdapter } from '../observer/lattice-adapter.js';
import { createRecipe } from '../seeding/recipe.js';
import { validateValue } from './contracts.js';
import { FLUX_SECTOR_LIMITS, reduceFluxSectors, normalizeNativeFluxSectors } from './field-sectors.js';

const quantities = ['tokens', 'field_tokens', 'relation_tokens', 'incidence'];
const toggles = [...new Set([...SCALE0_TOGGLES, ...SCALE0_ADVANCED_TOGGLES].map(row => row[0]))];
const MEASUREMENT_DEFINITIONS = Object.freeze({
    manifested: { units: 'count', meaning: 'Number of nonzero manifestation records/sites in the published observation.' },
    positive: { units: 'count', meaning: 'Number of positive manifestation records/sites.' },
    negative: { units: 'count', meaning: 'Number of negative manifestation records/sites.' },
    zero: { units: 'count', meaning: 'Finite-record zero-manifestation occupancy; not empty-space volume.' },
    fieldTokens: { units: 'exact token count', meaning: 'Published finite field-token count; no physical energy mapping.' },
    relationTokens: { units: 'exact token count', meaning: 'Published finite relation-token count; no physical energy mapping.' },
    incidence: { units: 'exact signed incidence count', meaning: 'Integer incidence bookkeeping, distinct from ternary manifestation and physical charge.' },
    totalFlux: { units: 'effective lattice flux units', meaning: 'Sum of |J| over sites, not a boundary flux or conserved vector.' },
    physicalTime: { units: 'engine simulation time', meaning: 'Coordinate time reported by the effective engine, not an inferred physical or proper clock.' },
    dt: { units: 'engine simulation time per tick', meaning: 'Configured effective-engine integration step.' },
    maxBandwidth: { units: 'dimensionless', meaning: 'Maximum latency-selected transport allowance fraction; not maximum speed.' },
    maxCausalBudget: { units: 'dimensionless', meaning: 'Maximum selected-law causal budget diagnostic.' },
    entropy: { units: 'nats', meaning: 'Shannon spread of normalized |J|² across sites; not thermodynamic entropy.' },
    dynamicEnergy: { units: 'effective engine energy units', meaning: 'Rest-offset-free per-tick ledger channel; not a general conservation guarantee.' },
    vacuumBaselineEnergy: { units: 'dimensionless engine diagnostic sum', meaning: 'Original sum of |born_infeld_core|; not accounted energy or a physical vacuum-energy measurement.' },
    accountedEnergy: { units: 'effective engine energy units', meaning: 'Total energy-audit channel copied only from a same-tick audit.' },
    restEnergy: { units: 'effective engine energy units', meaning: 'Selected model rest-energy contribution from a same-tick audit.' },
    fieldEnergy: { units: 'effective engine energy units', meaning: 'Flux-potential contribution, one half sum |J|² with the engine volume measure; not electric-field energy.' },
    waveEnergy: { units: 'effective engine energy units', meaning: 'Wave-velocity energy contribution from a same-tick audit.' },
    particleKE: { units: 'effective engine energy units', meaning: 'Selected manifested-motion kinetic contribution from a same-tick audit.' },
});
/** @param {Record<string,any>} [properties] @param {string[]} [required] */
const schema = (properties = {}, required = Object.keys(properties)) => ({ type: 'object', properties, required, additionalProperties: false });
/** @param {string} type @param {string} description @param {Record<string,any>} [properties] @param {string[]} [required] */
const action = (type, description, properties, required) => ({ type, description, args: schema(properties, required) });
const cancelled = () => Object.assign(new Error('Assistant lattice action cancelled'), { name: 'AbortError', status: 'superseded' });
const stale = () => Object.assign(new Error('Lattice preparation or workspace changed; request a fresh plan'), { status: 'superseded' });
/** @param {unknown} value */
const exact = value => value != null && /^(0|[1-9]\d*)$/.test(String(value))
    && (typeof value !== 'number' || Number.isSafeInteger(value)) ? String(value) : null;

/** @param {LatticeControlOptions} options @returns {import('./contracts.js').ControlAdapter} */
export function createLatticeControl({ getCtx, isActive = () => true,
    loadScenario = null,
    getOwner = getActiveScale0Bridge, getState = getScale0State,
    getScenarios = getScale0SeedingScenarios, getQualification = getScale0QualificationState,
    getCatalogStatus = () => null,
    seedServices = () => import('../seeding/service.js'),
    seedRuntime = () => import('../seeding/runtime.js'),
    seedDescriptions = () => import('../seeding/native-runtime.js'),
    finiteCatalog = () => import('../seeding/catalog.js'),
}) {
    if (typeof getCtx !== 'function') throw new TypeError('getCtx is required');
    const identities = new WeakMap(), previews = new Map();
    let identity = 0, sequence = 0, disposed = false, queue = Promise.resolve(), measuringFlux = false;
    const resolveLoader = async () => loadScenario || (await import('../scales/scale0/controller.js')).loadScenario;
    const instance = globalThis.crypto?.randomUUID?.() || `lattice-${Date.now()}-${Math.random()}`;
    const readout = new LatticeObservationAdapter(() => ({ owner: getOwner(getCtx()), mode: getCtx()?.engineMode,
        scenario: getState().currentScenarioId }));
    /** @param {any} owner */
    function source(owner) {
        if (owner?.isFiniteRecord) return { epoch: owner.checkpointSHA256 || '', native: owner.ownerId || '' };
        const meta = owner?.getScale0TelemetryGroupMeta?.('diagnostics');
        return { epoch: owner?.configurationToken ?? owner?._expectedTelemetrySourceEpoch
            ?? meta?.sourceEpoch ?? owner?._scale0TelemetrySourceEpoch ?? '', native: owner?._nativeInstanceId || '' };
    }
    /** @param {any} ctx */
    function active(ctx) { return !disposed && isActive() && ctx?.engineMode === 'lattice' && !ctx.presentationSuspended; }
    /** @param {any} owner @param {any} ctx */
    function ready(owner, ctx) {
        return active(ctx) && owner && !owner.disposed && !owner.failed && owner.ready !== false
            && !owner._backgroundSuspended && !owner._hasPendingScenarioWork?.()
            && getQualification()?.authoritativeLoad?.status !== 'pending';
    }
    /** @param {any} owner */
    function native(owner) { return !!(owner?.isNativeGPU || owner?.isNativeWebSocket); }
    /** @param {any} owner */
    function canControl(owner) { return typeof owner?.executeScale0Control === 'function' && (!native(owner) || owner.scale0ControlVersion === 1); }
    /** @param {any} owner @param {any} ctx */
    function descriptors(owner, ctx) {
        const rows = [action('lattice.observe', 'Read current completed lattice observations'),
            action('lattice.catalog', 'Find supported scenarios and their identifiers', {
                query: { type: 'string', maxLength: 120 }, limit: { type: 'integer', minimum: 1, maximum: 20 },
            }, [])];
        if (!ready(owner, ctx)) return rows;
        rows.push(action('lattice.pause', 'Pause lattice playback and drain submitted work'),
            action('lattice.resume', 'Resume the existing lattice owner'));
        if (canControl(owner) || owner.isFiniteRecord || (!owner.isWorker && !native(owner) && owner.isWasm))
            rows.push(action('lattice.step', 'Pause, then advance an exact bounded number of ticks', { count: { type: 'integer', minimum: 1, maximum: 4096 } }));
        const mayReplace = !native(owner) || owner.scale0ControlVersion === 1 && owner.seedRecipeVersion === 2;
        if (mayReplace) rows.push(action('lattice.scenario', 'Load a registered scenario, replacing the current preparation', { scenarioId: { type: 'string', maxLength: 120 } }),
            action('lattice.reset', 'Recreate the current scenario or applied seed recipe'));
        if (mayReplace && (owner.isFiniteRecord || ctx.bridge?.seedRecipeVersion === 2 || ctx.bridge?._module?.describeScenarioSeed))
            rows.push(action('lattice.seed.preview', 'Validate and stage a version-2 seed recipe without changing the current owner', { recipeJson: { type: 'string', maxLength: 65536 } }),
                action('lattice.seed.describe', 'Read constructor-owned editable settings for a scenario without loading or advancing it', {
                    scenarioId: { type: 'string', maxLength: 120 }, size: { type: 'integer', minimum: 3, maximum: 256 },
                }, ['scenarioId']));
        if (mayReplace && previews.size) rows.push(action('lattice.seed.apply', 'Apply one previously prepared seed', { previewId: { type: 'string', maxLength: 120 } }));
        if (owner.isFiniteRecord) rows.push(action('lattice.visualization', 'Select the finite-record count shown in the lattice volume', { quantity: { type: 'string', enum: quantities } }));
        else if (canControl(owner)) rows.push(action('lattice.setToggle', 'Change a supported physics term; scenario qualification is suspended', {
            name: { type: 'string', enum: toggles }, value: { type: 'boolean' },
        }));
        return rows;
    }
    /** @returns {ObservationEnvelope} */
    function observe() {
        const ctx = getCtx(), owner = getOwner(ctx), st = getState();
        if (owner && !identities.has(owner)) identities.set(owner, ++identity);
        const provenance = source(owner);
        const snapshot = /** @type {Record<string,any>} */ (readout.snapshot());
        // Native acknowledgements precede asynchronous telemetry publication.
        // Keep the completed clock distinct from each diagnostic sample's tick;
        // never relabel old diagnostic values with the new command boundary.
        const completedTick = native(owner)
            ? exact(owner?._nativeCompletedTick) ?? exact(snapshot.tick)
            : exact(snapshot.tick ?? owner?.currentTick?.());
        const ownerId = owner ? `${instance}:${identities.get(owner)}:${provenance.native}` : null;
        const preparationVersion = `${ctx?._loadGeneration || 0}:${st.mutationEpoch || 0}:${provenance.epoch}`;
        const actions = active(ctx) ? descriptors(owner, ctx) : [];
        const facts = { ...snapshot, completedTick, running: !!ctx?.running, scenarioId: st.currentScenarioId,
            qualification: getQualification()?.status ?? null, sourceEpoch: provenance.epoch,
            nativeInstanceId: provenance.native || null, controlReady: !!ready(owner, ctx),
            quantity: owner?.isFiniteRecord ? owner.quantity : undefined,
            scenarioCount: getScenarios().length,
            scenarioCatalogStatus: getCatalogStatus(),
            scenarios: getScenarios().slice(0, 12).map(({ id, name, title, backend }) => ({ id, name: name || title || id, backend: backend || 'effective-lattice' })),
            scientificBoundary: owner?.isFiniteRecord
                ? 'Finite records under a staged candidate law; count observations do not identify particles, field energy, or physical recovery.'
                : 'Effective compiled lattice diagnostics; simulation output alone is not physical certification.' };
        return { workspace: 'lattice', ownerId, preparationVersion, tick: completedTick,
            capabilities: actions.map(row => row.type), actions, facts, selected: null };
    }
    /** @param {AssistantAction} actionValue @param {ObservationEnvelope} before */
    function validate(actionValue, before) {
        if (!actionValue || typeof actionValue.type !== 'string' || !before.capabilities.includes(actionValue.type))
            throw new Error('Lattice action is unavailable on this owner');
        const args = actionValue.args ?? {}, spec = before.actions?.find(row => row.type === actionValue.type)?.args;
        validateValue(args, spec);
        return args;
    }
    /** @param {AssistantAction} command @param {ExecutionOptions} [options] @returns {Promise<CommandReceipt>} */
    async function perform(command, { signal, assertActive = () => {}, expected } = {}) {
        const before = observe(), commandId = `${instance}:${++sequence}`, ctx = getCtx(), owner = getOwner(ctx);
        const token = expected || before;
        const checkActivity = () => {
            if (signal?.aborted) throw cancelled();
            assertActive();
            if (!active(ctx) || getCtx() !== ctx) throw stale();
        };
        const check = () => {
            checkActivity();
            const now = observe();
            if (now.ownerId !== token.ownerId || now.preparationVersion !== token.preparationVersion) throw stale();
        };
        check();
        const args = validate(command, before), provenance = source(owner);
        /** @param {Record<string,any>} [extra] @returns {CommandReceipt} */
        const receipt = extra => ({ commandId, status: 'applied', action: command, before, after: observe(), ...extra });
        /** @param {any} cmd */
        const backendControl = cmd => owner.executeScale0Control(cmd, { signal,
            expectedSourceEpoch: provenance.epoch, expectedNativeInstanceId: provenance.native });
        const settle = async (idle = true) => {
            const deadline = Date.now() + 30000;
            while (owner.runningStateSettled === false || idle && (owner._simulationInFlight || owner.busy)) {
                check();
                if (Date.now() > deadline) throw Object.assign(new Error('Lattice did not acknowledge pause'), { status: 'unknown' });
                await new Promise(resolve => setTimeout(resolve, 16));
            }
            check();
            if (canControl(owner)) await backendControl({ type: 'barrier' });
            check();
        };
        const pause = async () => { check(); ctx.pauseSimulation(); await settle(); };
        if (command.type === 'lattice.observe') return receipt();
        if (command.type === 'lattice.seed.describe') {
            const spec = getScenarios().find(row => row.id === args.scenarioId);
            if (!spec || native(owner) && spec.backend === 'finite-records') throw new Error('Scenario is unavailable on the active backend');
            const size = args.size ?? owner.latticeSize;
            if (!Number.isSafeInteger(size) || size < 3 || size > 256) throw new Error('Choose an explicit supported lattice size from 3 to 256.');
            if (spec.sizes?.length && !spec.sizes.includes(size))
                throw new Error(`Scenario ${spec.id} does not support size ${size}; supported sizes: ${spec.sizes.join(', ')}. Select a size explicitly.`);
            const finite = spec.backend === 'finite-records';
            if (!finite && native(owner) && size < 4) throw new Error('The native backend requires a lattice size from 4 to 256.');
            if (!finite && !native(owner) && size % 2 === 0) throw new Error('The browser backend requires an odd lattice size.');
            let recipe = createRecipe(spec, size), description;
            if (finite) {
                const api = await finiteCatalog(); check();
                const catalog = await api.loadFiniteCatalog(); check();
                description = api.findFiniteSchema(catalog, spec.id, size);
                if (!description?.recipe) throw new Error(`No constructor-authored finite schema exists for ${spec.id} at size ${size}.`);
                recipe = description.recipe;
            } else {
                const api = await seedDescriptions(); check();
                description = await api.describeNativeRecipe(ctx, recipe); check();
            }
            if (!Array.isArray(description.properties)) throw new Error('Scenario constructor did not return editable property descriptors.');
            const properties = description.properties
                .filter((/** @type {any} */ p) => Array.isArray(p.path) && p.path.length && p.path[0] !== 'size'
                    && !/^recipe\./.test(p.key) && !/^ingredient\.\d+\.scenario$/.test(p.key))
                .map((/** @type {any} */ p) => p.path.at(-1) === 'enabled'
                    ? { ...p, default: !!p.default, value: !!p.value, type: 'choice', min: 0, max: 1, step: 1,
                        options: [[false, 'Off — disabled'], [true, 'On — enabled']] } : p);
            const available = finite ? ['manifested', 'positive', 'negative', 'zero', 'fieldTokens', 'relationTokens', 'incidence']
                : ['manifested', 'positive', 'negative', 'totalFlux', 'physicalTime', 'dt', 'maxBandwidth', 'maxCausalBudget', 'entropy',
                    'dynamicEnergy', 'vacuumBaselineEnergy', 'accountedEnergy', 'restEnergy', 'fieldEnergy', 'waveEnergy', 'particleKE']
                    .filter(key => before.facts.available === true && before.facts.stale !== true
                        && (typeof before.facts[key] === 'number' && Number.isFinite(before.facts[key]) || exact(before.facts[key]) !== null));
            // Constructor annotations may carry undefined optional fields. The
            // public template is data-only and portable through JSON unchanged.
            const template = JSON.parse(JSON.stringify({ scenarioId: spec.id, label: spec.name || spec.title || spec.id,
                backend: finite ? 'finite-records' : 'effective-lattice', size, recipe, properties,
                allowedMeasurements: available, epistemicStatus: spec.epistemicStatus,
                measurementDefinitions: Object.fromEntries(available.map(key => [key, MEASUREMENT_DEFINITIONS[/** @type {keyof typeof MEASUREMENT_DEFINITIONS} */ (key)]])),
                sizePolicy: args.size === undefined ? 'current-lattice' : 'explicit-request' }));
            check();
            return receipt({ result: { template } });
        }
        if (command.type === 'lattice.catalog') {
            const query = (args.query || '').toLowerCase();
            const matches = getScenarios().filter(row => JSON.stringify([row.id, row.name, row.title, row.category]).toLowerCase().includes(query)).slice(0, args.limit || 12);
            /** @type {any[]} */
            const result = matches.map(({ id, name, title, backend, sizes, description }) => ({ id, name: name || title || id, backend: backend || 'effective-lattice', sizes, description }));
            if (matches.length === 1 && query === matches[0].id.toLowerCase()) {
                const spec = matches[0], size = owner?.latticeSize || 33;
                let recipe = createRecipe(spec, size);
                if (spec.backend === 'finite-records') {
                    const { loadFiniteCatalog, findFiniteSchema } = await import('../seeding/catalog.js');
                    recipe = findFiniteSchema(await loadFiniteCatalog(), spec.id, recipe.size)?.recipe;
                }
                check();
                if (recipe) result[0].recipeJson = JSON.stringify(recipe);
            }
            return receipt({ result });
        }
        if (command.type === 'lattice.pause') { await pause(); return receipt(); }
        if (command.type === 'lattice.resume') {
            check(); if (!ctx.running) ctx.togglePlay();
            await settle(false); return receipt();
        }
        if (command.type === 'lattice.step') {
            await pause();
            let completedTicks = 0;
            let lastPublishedAt = performance.now();
            const publish = () => { setLatticeNeedsUpload(); markFieldDirty(); lastPublishedAt = performance.now(); };
            try {
                while (completedTicks < args.count) {
                    check(); if (ctx.running) throw stale();
                    // One tick is an interruptible commit boundary on all backends.
                    const initial = exact(owner.observation?.microtick ?? owner.currentTick?.());
                    if (owner.isFiniteRecord) await owner.advance(1);
                    else if (canControl(owner)) await backendControl({ type: 'step', count: 1 });
                    else {
                        owner.capabilities.scale0.tickScale0();
                        if (initial === null || exact(owner.currentTick?.()) !== String(BigInt(initial) + 1n))
                            throw Object.assign(new Error('Direct WASM step acknowledgement mismatch'), { status: 'unknown', completedTicks });
                        await new Promise(resolve => setTimeout(resolve, 0));
                    }
                    if (owner.failed || owner.disposed) throw Object.assign(new Error('Lattice step failed'), { status: 'unknown', completedTicks });
                    if (owner.isFiniteRecord && (initial === null || exact(owner.observation?.microtick) !== String(BigInt(initial) + 1n)))
                        throw Object.assign(new Error('Finite-record step acknowledgement mismatch'), { status: 'unknown', completedTicks });
                    completedTicks++;
                    check();
                    // A long approved run must remain visible and interruptible
                    // between sampling boundaries, including fast local owners.
                    if (performance.now() - lastPublishedAt >= 16) {
                        publish();
                        await new Promise(resolve => setTimeout(resolve, 0));
                    }
                }
            } catch (error) {
                if (!completedTicks && !(error && typeof error === 'object' && 'status' in error && error.status === 'unknown')) throw error;
                return receipt({ status: 'unknown', completedTicks,
                    error: `Stopped after ${completedTicks} confirmed ticks of ${args.count}. ${error instanceof Error ? error.message : 'Step outcome unknown'}` });
            } finally {
                // Stop may arrive after a committed tick but before its receipt.
                // Keep the last actual state visible even for a partial result.
                if (completedTicks) publish();
            }
            return receipt({ completedTicks });
        }
        if (command.type === 'lattice.visualization') {
            check(); owner.setRecordQuantity(args.quantity);
            return receipt();
        }
        if (command.type === 'lattice.setToggle') {
            check();
            const commitMutation = /** @type {(ctx:any,options:any,mutate:()=>any)=>any} */ (commitScale0ScientificMutation);
            const dispatch = commitMutation(ctx, { owner, loadGeneration: ctx._loadGeneration || 0,
                reason: 'physics-toggle', source: 'assistant', dispatchStatus: 'dispatched' }, () => backendControl({ type: 'setToggle', ...args }));
            if (!dispatch.accepted) throw stale();
            const ownVersion = observe();
            try { await dispatch.result; checkActivity(); }
            catch (error) { return receipt({ status: 'unknown', error: error instanceof Error ? error.message : 'Toggle outcome unknown' }); }
            const after = observe();
            if (after.ownerId !== ownVersion.ownerId || after.preparationVersion !== ownVersion.preparationVersion) throw stale();
            setLatticeNeedsUpload(); markFieldDirty(); return receipt();
        }
        const services = await seedServices(); check();
        if (command.type === 'lattice.seed.preview') {
            const recipe = JSON.parse(args.recipeJson);
            const staged = await services.prepareSeed(ctx, recipe, getScenarios(), signal);
            try { check(); } catch (error) { staged.dispose(); throw error; }
            for (const p of previews.values()) p.owner.dispose(); previews.clear();
            const previewId = `${instance}:preview:${sequence}`;
            previews.set(previewId, { owner: staged, recipe, token: before });
            return receipt({ result: { previewId, scenarioId: recipe.scenarioId, size: recipe.size } });
        }
        let staged = null, recipe = null;
        if (command.type === 'lattice.seed.apply') {
            staged = previews.get(args.previewId);
            if (!staged || staged.token.ownerId !== before.ownerId || staged.token.preparationVersion !== before.preparationVersion) throw stale();
            previews.delete(args.previewId); recipe = staged.recipe;
        } else if (command.type === 'lattice.reset') recipe = ctx._appliedSeedRecipe || null;
        if (native(owner) && ['lattice.scenario', 'lattice.reset'].includes(command.type)) {
            const id = command.type === 'lattice.scenario' ? args.scenarioId : getState().currentScenarioId;
            const spec = getScenarios().find(row => row.id === id);
            if (!spec || spec.backend === 'finite-records') throw new Error('Scenario is unavailable on the native backend');
            // Native replacements use the already source-fenced seed commit,
            // not the UI's fire-and-forget setup_scenario command.
            recipe ||= createRecipe(spec, owner.latticeSize);
        }
        if (recipe) {
            const loader = await resolveLoader(); check();
            const installed = await services.applySeed(ctx, { recipe, scenarios: getScenarios(), owner: staged?.owner,
                signal, assertCurrent: check, assertActive: checkActivity,
                loadScenario: (/** @type {string} */ id, /** @type {any} */ params) => loader(ctx, id, params) });
            checkActivity();
            if (getOwner(ctx) !== installed.owner || ctx._loadGeneration !== installed.installedGeneration) throw stale();
            return receipt();
        }
        const id = command.type === 'lattice.scenario' ? args.scenarioId : getState().currentScenarioId;
        const spec = getScenarios().find(row => row.id === id);
        if (!spec || spec.backend === 'finite-records' && native(owner)) throw new Error('Scenario is unavailable on the active backend');
        // Resolve the loader and waiter before the final write fence. Controller
        // imports may take a turn; no async loader can be left unobserved.
        const runtime = await seedRuntime(), loader = await resolveLoader(); check();
        ctx.pauseSimulation(); check();
        loader(ctx, id);
        const generation = ctx._loadGeneration, installedOwner = getOwner(ctx);
        await runtime.waitForSeedLoad(ctx, generation, signal);
        checkActivity();
        if (ctx._loadGeneration !== generation || getOwner(ctx) !== installedOwner) throw stale();
        return receipt();
    }
    return {
        observe,
        async measureFluxSectors({ expected, expectedTick }, signal) {
            if (typeof expectedTick !== 'string' || expectedTick.length > 20 || exact(expectedTick) !== expectedTick)
                throw new Error('expectedTick must be a bounded exact decimal tick string.');
            if (measuringFlux) throw new Error('A flux-sector measurement is already active.');
            const ctx = getCtx(), owner = getOwner(ctx), start = observe();
            const interventionEpoch = owner?._visualInterventionEpoch;
            const aggregate = owner?.fluxSectorVersion === 1 && typeof owner.getFluxSectors === 'function';
            const check = () => {
                signal.throwIfAborted();
                const now = observe();
                if (!ready(owner, ctx) || getCtx() !== ctx || getOwner(ctx) !== owner
                    || expected?.workspace !== 'lattice' || now.ownerId !== expected.ownerId
                    || now.preparationVersion !== expected.preparationVersion || now.tick !== expectedTick
                    || owner._visualInterventionEpoch !== interventionEpoch)
                    throw stale();
                if (ctx.running || owner.runningStateSettled === false || owner._simulationInFlight || owner.busy
                    || owner._queuedSimulationTicks > 0)
                    throw new Error('Pause and drain the lattice before measuring flux sectors.');
            };
            check();
            if (!native(owner) || owner.isFiniteRecord || owner._nativeBinaryVersion !== 3
                || typeof owner.getTelemetrySnapshot !== 'function'
                || typeof owner.getDynamicalStateDigest !== 'function'
                || (!aggregate && (typeof owner.getFluxVectorSampled !== 'function' || typeof owner.getSamplerSnapshotVersion !== 'function'))
                || !Number.isSafeInteger(interventionEpoch) || !Number.isSafeInteger(owner._visualEpoch))
                throw new Error('Coherent native v3 field sampling is unavailable on this owner.');
            const maxSize = aggregate ? FLUX_SECTOR_LIMITS.nativeMaxSize : FLUX_SECTOR_LIMITS.maxSize;
            if (!Number.isInteger(owner.latticeSize) || owner.latticeSize < 4 || owner.latticeSize > maxSize)
                throw new Error(`Full-resolution flux-sector measurement requires lattice size 4..${maxSize}; larger sizes need native fluxSectorVersion 1.`);
            measuringFlux = true;
            const deadline = Date.now() + (aggregate ? FLUX_SECTOR_LIMITS.nativeTimeoutMs : FLUX_SECTOR_LIMITS.timeoutMs);
            /** @param {()=>Promise<any>} request */
            const read = async request => {
                check();
                /** @type {()=>void} */ let cleanup = () => {};
                const value = await new Promise((resolve, reject) => {
                    const abort = () => reject(cancelled());
                    const timer = setTimeout(() => reject(new Error('Flux-sector read deadline exceeded.')), Math.max(1, deadline - Date.now()));
                    cleanup = () => { clearTimeout(timer); signal.removeEventListener('abort', abort); };
                    signal.addEventListener('abort', abort, { once: true });
                    Promise.resolve().then(() => { check(); return request(); }).then(resolve, reject);
                }).finally(() => cleanup());
                check();
                return value;
            };
            const digest = async () => {
                const value = await read(() => owner.getDynamicalStateDigest());
                if (value?.type !== 'dynamical_state_digest' || value.nonfiniteValueCount !== 0
                    || exact(value.tick) !== expectedTick || exact(value.sourceEpoch) !== exact(start.facts.sourceEpoch)
                    || value.latticeSize !== owner.latticeSize || value.siteCount !== owner.latticeSize ** 3
                    || exact(value.stateVersion) === null || !/^[0-9a-f]{16}$/.test(value.hashLo || '')
                    || !/^[0-9a-f]{16}$/.test(value.hashHi || ''))
                    throw new Error('A coherent finite-valued canonical digest is required for flux-sector bounds.');
                return value;
            };
            try {
                const beforeDigest = await digest();
                if (aggregate) {
                    const value = await read(() => owner.getFluxSectors()), p = value.provenance;
                    const telemetry = owner.getTelemetrySnapshot();
                    if (p?.nativeInstanceId !== start.facts.nativeInstanceId || exact(p.sourceEpoch) !== exact(start.facts.sourceEpoch)
                        || exact(p.sampleTick) !== expectedTick || exact(value.tick) !== expectedTick || exact(p.epoch) === null
                        || p.latticeSize !== owner.latticeSize || value.compute !== beforeDigest.compute
                        || exact(value.stateVersion) !== exact(beforeDigest.stateVersion)
                        || telemetry.nativeInstanceId !== p.nativeInstanceId || exact(telemetry.sourceEpoch) !== exact(p.sourceEpoch)
                        || exact(telemetry.epoch) === null || BigInt(p.epoch) < BigInt(telemetry.epoch)
                        || (exact(telemetry.tick) === expectedTick && exact(telemetry.epoch) !== exact(p.epoch)))
                        throw new Error('Native flux-sector aggregate provenance does not match the paused owner.');
                    const measurement = normalizeNativeFluxSectors(value, owner.latticeSize);
                    const afterDigest = await digest();
                    if (beforeDigest.hashLo !== afterDigest.hashLo || beforeDigest.hashHi !== afterDigest.hashHi
                        || exact(beforeDigest.stateVersion) !== exact(afterDigest.stateVersion))
                        throw new Error('Canonical lattice state changed during flux-sector measurement.');
                    check();
                    return { status: 'measured', ownerId: start.ownerId, preparationVersion: start.preparationVersion,
                        provenance: { ...p }, integrity: { before: beforeDigest, after: afterDigest,
                            unchanged: true, nonfiniteValueCount: 0 }, ...measurement };
                }
                while (!signal.aborted) {
                    check();
                    // Existing bridge owns request scheduling, binary validation,
                    // cache lifetime, and the sole native WebSocket connection.
                    const sample = owner.getFluxVectorSampled(1), p = sample?.provenance;
                    const telemetry = owner.getTelemetrySnapshot();
                    if (owner.getSamplerSnapshotVersion('fluxVector', 1) === owner._visualEpoch
                        && p?.status === 'approximate' && p.nativeInstanceId === start.facts.nativeInstanceId
                        && exact(p.sourceEpoch) === exact(start.facts.sourceEpoch)
                        && exact(p.sampleTick) === expectedTick && exact(p.epoch) !== null
                        && p.latticeSize === owner.latticeSize
                        && telemetry.nativeInstanceId === p.nativeInstanceId
                        && exact(telemetry.sourceEpoch) === exact(p.sourceEpoch)
                        && exact(telemetry.epoch) !== null && BigInt(p.epoch) >= BigInt(telemetry.epoch)
                        && (exact(telemetry.tick) !== expectedTick || exact(telemetry.epoch) === exact(p.epoch))) {
                        const measurement = reduceFluxSectors(sample, owner.latticeSize);
                        const afterDigest = await digest();
                        if (beforeDigest.hashLo !== afterDigest.hashLo || beforeDigest.hashHi !== afterDigest.hashHi
                            || exact(beforeDigest.stateVersion) !== exact(afterDigest.stateVersion))
                            throw new Error('Canonical lattice state changed during flux-sector measurement.');
                        check();
                        return { status: 'measured', ownerId: start.ownerId, preparationVersion: start.preparationVersion,
                            provenance: { ...p }, integrity: { before: beforeDigest, after: afterDigest,
                                unchanged: true, nonfiniteValueCount: 0 }, ...measurement };
                    }
                    if (Date.now() >= deadline) throw new Error('No coherent full-resolution field snapshot arrived for the requested tick.');
                    await new Promise(resolve => setTimeout(resolve, 16));
                }
                signal.throwIfAborted();
                throw cancelled();
            } finally { measuringFlux = false; }
        },
        listScenarioTemplates() {
            const ctx = getCtx(), owner = getOwner(ctx);
            if (!ready(owner, ctx)) return [];
            return getScenarios().map(({ id, name, title, category, backend, sizes, description, intent }) => ({ id,
                label: name || title || id, category: category || 'Other preparations',
                backend: backend || 'effective-lattice', sizes: sizes ? [...sizes] : [owner.latticeSize],
                ...(description || intent ? { description: description || intent } : {}),
            }));
        },
        execute(command, options) {
            const pinned = { ...options, expected: options?.expected || observe() };
            const intent = structuredClone(command);
            const task = queue.then(() => perform(intent, pinned));
            queue = task.then(() => {}, () => {});
            return task;
        },
        dispose() { disposed = true; for (const preview of previews.values()) preview.owner.dispose(); previews.clear(); },
    };
}
