import { test, expect } from '@playwright/test';
import { readFileSync } from 'node:fs';
import { writeFile } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

const corpusURL = new URL('./fixtures/assistant-command-corpus.json', import.meta.url);
const corpusSource = readFileSync(corpusURL, 'utf8');
const corpus = JSON.parse(corpusSource);
const requestedLimit = process.env.ASSISTANT_CORPUS_LIMIT?.trim();
if (requestedLimit && !/^[1-9]\d*$/.test(requestedLimit)) throw new Error('ASSISTANT_CORPUS_LIMIT must be a positive integer.');
const cases = requestedLimit ? corpus.slice(0, Number(requestedLimit)) : corpus;

function subset(actual, expected) {
    if (typeof expected === 'number') return typeof actual === 'number' && Number.isFinite(actual)
        && Math.abs(actual - expected) <= 1e-9 * Math.max(1, Math.abs(expected));
    if (Array.isArray(expected)) return Array.isArray(actual) && actual.length === expected.length
        && expected.every((value, index) => subset(actual[index], value));
    if (expected && typeof expected === 'object') return actual !== null && typeof actual === 'object'
        && !Array.isArray(actual) && Object.entries(expected).every(([key, value]) => Object.hasOwn(actual, key) && subset(actual[key], value));
    return actual === expected;
}

function omitStepPauses(actions) {
    return actions.filter((action, index) => {
        const next = actions[index + 1];
        const prefix = action?.type?.endsWith('.pause') ? action.type.slice(0, -'.pause'.length) : null;
        return !prefix || next?.type !== `${prefix}.step`;
    });
}

function assess(row, outcome) {
    const plan = outcome.plan;
    const rawActions = Array.isArray(plan?.actions) ? plan.actions : [];
    const mutations = rawActions.filter(action => !['lattice.observe', 'lattice.catalog'].includes(action?.type));
    const result = { correct: false, plannedActionCount: rawActions.length, plannedMutationCount: mutations.length, normalizations: [] };
    if (outcome.error || outcome.validationError) return { ...result, reason: outcome.error || outcome.validationError };
    if (row.expected.kind === 'clarify') {
        result.correct = ['clarify', 'answer'].includes(plan?.kind) && rawActions.length === 0;
        return { ...result, reason: result.correct ? null : 'Ambiguous or unsupported requests must answer/clarify with no actions.' };
    }
    if (row.expected.kind === 'answer') {
        result.correct = plan?.kind === 'answer' && rawActions.length === 0;
        return { ...result, reason: result.correct ? null : 'An explanation question must produce an answer plan with no actions.' };
    }
    if (plan?.kind !== 'actions') return { ...result, reason: 'Expected an action plan.' };
    const actual = omitStepPauses(rawActions), expected = omitStepPauses(row.expected.actions);
    if (actual.length !== rawActions.length || expected.length !== row.expected.actions.length) result.normalizations.push('A pause immediately before step is semantically redundant in these paused fixtures.');
    if (actual.length !== expected.length) return { ...result, reason: `Expected ${expected.length} actions after step/pause normalization; received ${actual.length}.` };
    for (let index = 0; index < expected.length; index++) {
        const observed = actual[index], wanted = expected[index];
        if (observed.type !== wanted.type) return { ...result, reason: `Action ${index + 1}: expected ${wanted.type}, received ${observed.type}.` };
        const args = { ...observed.args }, wantedArgs = wanted.argsSubset;
        if (row.selected && ['observer.update', 'observer.delete', 'observer.restore', 'observer.impulse'].includes(observed.type)
            && args.id !== undefined && args.id !== outcome.fixture.selectedId) return { ...result, reason: `Action ${index + 1} targets a different object from the selected cube.` };
        if (observed.type === 'observer.update') {
            if (Object.hasOwn(args, 'mass') && Object.hasOwn(args, 'massFactor')) return { ...result, reason: 'Mass and massFactor cannot be combined in one Observer update.' };
            const mass = outcome.fixture.selectedMass;
            if (row.selected && typeof mass === 'number' && mass > 0) {
                if (wantedArgs.massFactor !== undefined && args.massFactor === undefined && typeof args.mass === 'number') {
                    args.massFactor = args.mass / mass;
                    result.normalizations.push(`Absolute mass ${args.mass} is factor ${args.massFactor} for the captured mass ${mass}.`);
                }
                if (wantedArgs.mass !== undefined && args.mass === undefined && typeof args.massFactor === 'number') {
                    args.mass = args.massFactor * mass;
                    result.normalizations.push(`Mass factor ${args.massFactor} is absolute mass ${args.mass} for the captured mass ${mass}.`);
                }
            }
        }
        if (!subset(args, wantedArgs)) return { ...result, reason: `Action ${index + 1} does not match the required argument subset.` };
    }
    return { ...result, correct: true, reason: null };
}

function summarize(results) {
    const clear = results.filter(row => row.expected.kind === 'actions');
    const rejections = results.filter(row => row.expected.kind === 'clarify');
    const answers = results.filter(row => row.expected.kind === 'answer');
    const correctClear = clear.filter(row => row.assessment.correct).length;
    return {
        attempted: results.filter(row => row.attempted).length, recorded: results.length,
        clear: { total: clear.length, correct: correctClear, accuracy: clear.length ? correctClear / clear.length : null },
        rejection: { total: rejections.length, correct: rejections.filter(row => row.assessment.correct).length,
            casesWithPlannedActions: rejections.filter(row => row.assessment.plannedActionCount > 0).length,
            casesWithPlannedMutations: rejections.filter(row => row.assessment.plannedMutationCount > 0).length,
            plannedMutationCount: rejections.reduce((sum, row) => sum + row.assessment.plannedMutationCount, 0) },
        answer: { total: answers.length, correct: answers.filter(row => row.assessment.correct).length },
        generationErrors: results.filter(row => row.error).length,
        schemaErrors: results.filter(row => row.validationError).length,
    };
}

test('actual local model meets the fixed command corpus release gate', async ({ page }, testInfo) => {
    test.skip(process.env.FTD_HARDWARE_WEBGL !== '1', 'Explicit hardware inference run; JEV is not exercised.');
    test.setTimeout(600_000);
    const started = Date.now(), workDeadline = started + 555_000;
    const evidence = {
        schemaVersion: 1, startedAt: new Date(started).toISOString(),
        corpus: { path: 'engine/web/tests/fixtures/assistant-command-corpus.json', sha256: createHash('sha256').update(corpusSource).digest('hex'),
            totalCases: corpus.length, requestedCases: cases.length, limitedDiagnostic: cases.length < corpus.length },
        gate: { minimumClearAccuracy: 0.95, maximumRejectedPlannedMutations: 0, requireRejectedCasesToHaveNoActions: true },
        planner: {pipeline:'Local clause, quantity and capability grounding plus actual SmolLM2 inference. Questions and locally rejected requests bypass plan generation; fully bound commands constrain the grammar.',rawModelOnly:false},
        jev: { exercised: false, credentialsRequired: false, note: 'This evaluates the grounded planner. JEV authorization and subsequent execution are separate gates.' },
        execution: { plannedActionsExecuted: false, fixturePreparationOnly: true, profileSnapshotsClonedForEachCase: true },
        provenance: null, results: [], fatalError: null,
    };
    let baselines;
    try {
        await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
        await page.waitForFunction(() => window.__FTD_DEV__.registry.get('assistant')?.control.observe()?.facts.controlReady === true);
        const lattice = await page.evaluate(async () => {
            const ai = window.__FTD_DEV__.registry.get('assistant'), expected = ai.control.observe();
            const receipt = await ai.control.execute({ type: 'lattice.pause', args: {} }, { expected });
            if (receipt.status !== 'applied') throw new Error(receipt.error || 'Lattice fixture could not pause.');
            return ai.control.observe();
        });
        await openObserverWorkspace(page);
        const observer = await page.evaluate(async () => {
            const registry = window.__FTD_DEV__.registry, workspace = registry.get('observerWorkspace'), ai = registry.get('assistant');
            const apply = async command => {
                const result = await workspace.command(command);
                if (!result?.ok) throw new Error(result?.error || 'Observer fixture command failed.');
            };
            await apply({ type: 'pause' });
            if (workspace.snapshot.profile !== 'sr') await apply({ type: 'profile', profile: 'sr' });
            const cube = workspace.snapshot.entities.find(entity => entity.alive && entity.shape === 'box');
            if (!cube) throw new Error('Baseline has no live cube for the selected-object fixture.');
            await apply({ type: 'update', id: cube.id, patch: { mass: 2, bodyType: 'dynamic' } });
            workspace.selectedId = cube.id; workspace.selectedHit = null; workspace.hit = null;
            const sr = structuredClone(ai.control.observe());
            await apply({ type: 'profile', profile: 'playground' });
            workspace.selectedId = cube.id; workspace.selectedHit = null; workspace.hit = null;
            const playground = structuredClone(ai.control.observe());
            for (const fixture of [sr, playground]) {
                if (fixture.facts.running || fixture.selected?.current?.mass !== 2 || fixture.selected?.current?.shape !== 'box'
                    || fixture.selected?.current?.bodyType !== 'dynamic' || !fixture.selected?.current?.alive) throw new Error('Invalid paused live-cube fixture.');
            }
            return { sr, playground };
        });
        baselines = { lattice, ...observer };
        console.log(`[AI corpus] Loading the actual browser model for ${cases.length}/${corpus.length} cases.`);
        evidence.provenance = await page.evaluate(async () => {
            const ai = window.__FTD_DEV__.registry.get('assistant'), started = performance.now();
            await ai.model.load();
            const gpu = await navigator.gpu.requestAdapter();
            const manifestResponse = await fetch('/js/assistant/model-manifest.json');
            if (!manifestResponse.ok) throw new Error('Model provenance manifest is unavailable.');
            const bytes = await manifestResponse.arrayBuffer();
            const digest = await crypto.subtle.digest('SHA-256', bytes);
            const manifest = JSON.parse(new TextDecoder().decode(bytes));
            const variant = manifest.variants.find(item => item.id === ai.model.modelId);
            return { modelId: ai.model.modelId, modelLoadMs: performance.now() - started,
                modelManifestSha256: Array.from(new Uint8Array(digest), value => value.toString(16).padStart(2, '0')).join(''),
                modelVariant: variant, browser: navigator.userAgent, origin: location.origin,
                gpu: gpu ? { vendor: gpu.info?.vendor, architecture: gpu.info?.architecture, device: gpu.info?.device, description: gpu.info?.description,
                    features: [...gpu.features] } : null };
        });
        evidence.provenance.fixtures = Object.fromEntries(Object.entries(baselines).map(([key, value]) => [key, {
            workspace: value.workspace, ownerId: value.ownerId, preparationVersion: value.preparationVersion, tick: value.tick,
            profile: value.facts.profile, physicsEngine: value.facts.physicsEngine, running: value.facts.running,
            selectedId: value.selected?.id ?? null, selectedMass: value.selected?.current?.mass ?? null,
            capabilities: value.capabilities,
        }]));
        for (let index = 0; index < cases.length; index++) {
            const row = cases[index], baseline = row.workspace === 'lattice' ? baselines.lattice : baselines[row.profile || 'sr'];
            if (!baseline) throw new Error(`No actual observation fixture for ${row.id}.`);
            const observation = structuredClone(baseline);
            if (!row.selected) observation.selected = null;
            // Unselected commands cannot acquire a target from an incidental ray hit.
            observation.facts.crosshair = null;
            const fixture = { ownerId: observation.ownerId, preparationVersion: observation.preparationVersion, tick: observation.tick,
                profile: observation.facts.profile, selectedId: observation.selected?.id ?? null, selectedMass: observation.selected?.current?.mass ?? null };
            const remaining = workDeadline - Date.now();
            let outcome;
            if (remaining < 1000 || page.isClosed()) outcome = { attempted: false, error: 'The overall evaluation deadline or browser lifetime ended before this case.', ms: 0 };
            else {
                try {
                    outcome = await page.evaluate(async ({ text, observation, timeoutMs }) => {
                        const ai = window.__FTD_DEV__.registry.get('assistant'), controller = new AbortController(), started = performance.now();
                        const timer = setTimeout(() => controller.abort(new Error('Corpus case inference deadline exceeded.')), timeoutMs);
                        let plan = null, validationError = null;
                        try {
                            plan = await ai.model.plan(text, observation, [], controller.signal);
                            try { const { validatePlan } = await import('/js/assistant/contracts.js'); validatePlan(plan, observation); }
                            catch (error) { validationError = error instanceof Error ? error.message : String(error); }
                            return { attempted: true, plan, validationError, ms: performance.now() - started };
                        } catch (error) { return { attempted: true, plan, error: error instanceof Error ? error.message : String(error), ms: performance.now() - started }; }
                        finally { clearTimeout(timer); }
                    }, { text: row.text, observation, timeoutMs: Math.min(30_000, remaining) });
                } catch (error) { outcome = { attempted: true, error: error instanceof Error ? error.message : String(error), ms: null }; }
            }
            const result = { ...row, fixture, ...outcome };
            result.assessment = assess(row, result);
            evidence.results.push(result);
            if ((index + 1) % 10 === 0 || index + 1 === cases.length) {
                const totals = summarize(evidence.results);
                console.log(`[AI corpus] ${index + 1}/${cases.length}: clear ${totals.clear.correct}/${totals.clear.total}, rejection mutations ${totals.rejection.plannedMutationCount}, generation errors ${totals.generationErrors}.`);
            }
        }
    } catch (error) {
        evidence.fatalError = error instanceof Error ? error.message : String(error);
    } finally {
        const recorded = new Set(evidence.results.map(row => row.id));
        for (const row of cases) if (!recorded.has(row.id)) {
            const outcome = { attempted: false, error: evidence.fatalError || 'Case was not reached.', plan: null, ms: 0 };
            evidence.results.push({ ...row, ...outcome, assessment: assess(row, outcome) });
        }
        evidence.finishedAt = new Date().toISOString(); evidence.elapsedMs = Date.now() - started;
        evidence.summary = summarize(evidence.results);
        evidence.fullCorpusEvaluated = cases.length === corpus.length && evidence.summary.attempted === corpus.length && !evidence.fatalError;
        evidence.releaseGatePassed = evidence.fullCorpusEvaluated && evidence.summary.clear.accuracy >= 0.95
            && evidence.summary.rejection.correct === evidence.summary.rejection.total
            && evidence.summary.rejection.plannedMutationCount === 0 && evidence.summary.answer.correct === evidence.summary.answer.total;
        const path=testInfo.outputPath('assistant-command-corpus-results.json');
        await writeFile(path,JSON.stringify(evidence,null,2));
        await testInfo.attach('assistant-command-corpus-results.json', { path, contentType: 'application/json' });
    }
    if (cases.length < corpus.length) testInfo.annotations.push({ type: 'diagnostic', description: `Only ${cases.length}/${corpus.length} cases requested; this cannot certify the full release gate.` });
    expect(evidence.fatalError, JSON.stringify(evidence.summary)).toBeNull();
    expect(evidence.summary.attempted, 'Every requested case must reach the actual local model.').toBe(cases.length);
    expect(evidence.summary.clear.accuracy ?? 1, JSON.stringify(evidence.summary)).toBeGreaterThanOrEqual(0.95);
    expect(evidence.summary.rejection.plannedMutationCount, 'Ambiguous or unsupported cases must never propose a mutation.').toBe(0);
    expect(evidence.summary.rejection.correct, 'Every rejection case must finish as answer/clarify with no actions.').toBe(evidence.summary.rejection.total);
    expect(evidence.summary.answer.correct, 'Explanation questions must produce answer plans without actions.').toBe(evidence.summary.answer.total);
});
