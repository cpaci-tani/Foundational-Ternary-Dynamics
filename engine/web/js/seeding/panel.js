import { getScale0SeedingScenarios } from '../scales/scale0/scenario-registry.js';
import { getActiveScale0Bridge, getScale0QualificationState, subscribeScale0Qualification } from '../scales/scale0/state/store.js';
import {
    COMPONENT_KINDS, RECORD_SEED_SIZES, cloneRecipe, getAtPath, setAtPath, createRecipe, createBaseRecipe,
    createNativeBaseRecipe, isCustomNativeRecipe, createComponent, validateRecipe, parseRecipeEnvelope,
    buildRecipeEnvelope, migrateRecipe, seedCoverage,
    markFiniteOverride, hasFiniteOverride,
    MAX_RECIPE_ENVELOPE_BYTES,
} from './recipe.js';
import { loadFiniteCatalog, findFiniteSchema, findFiniteBaseSchema, findComponentTemplates, collectionRowBounds } from './catalog.js';
import { prepareRecordRecipe, isEditedRecordRecipe } from './runtime.js';
import { describeNativeRecipe, prepareNativeRecipe } from './native-runtime.js';
import { applySeed } from './service.js';
import { node, button, section } from './component-editor.js';
import { propertyEditor } from './property-editor.js';
import { assertPropertyDescriptor } from './property-schema.js';
import { renderRecipeSummary, recipeText } from './recipe-summary.js';
import { enhanceScenarioPicker } from '../scales/scale0/ui/toolbar/scenario-picker.js';

const drafts = new Map();
let instance = null;
const NATIVE_REDESCRIBE_DEBOUNCE_MS = 150;

function groupProperties(properties) {
    const groups = new Map();
    for (const p of properties) { if (!groups.has(p.group)) groups.set(p.group, []); groups.get(p.group).push(p); }
    return groups;
}

function preparationReceipt(owner, recipe) {
    const diagnostics = owner.getDiagnostics?.() || {};
    const fields = owner.isFiniteRecord ? ['fieldTokens','relationTokens','manifested','tick'] : ['manifested','positive','negative','totalFlux','tick'];
    return {recipeTag: JSON.stringify(recipe), size: recipe.size,
        ...Object.fromEntries(fields.filter(k=>diagnostics[k] !== undefined).map(k=>[k,diagnostics[k]])),
        ...(owner.checkpointSHA256 ? {checkpointSHA256:owner.checkpointSHA256} : {})};
}

class SeedingPanel {
    constructor(host, ctx, actions) {
        this.host = host; this.ctx = ctx; this.actions = actions; this.revision = 0; this.sequence = 0;
        this.abort = null; this.preview = null; this.busy = false; this.disposed = false;
        this.catalog = null; this.nativeProperties = null; this.nativeTimer = null; this.nativeDescriptionTag = null;
        this.finiteTimer = null; this.finiteDescription = null; this.descriptionTag = null;
        // Stop simulation shortcuts (space/arrow keys etc.) from firing while typing
        // in the panel, but let Escape keep bubbling to document — the global
        // tooltip component and per-property inline help both dismiss on it.
        this.onKey = event => {if (event.key !== 'Escape' && event.target.closest('input, select, button, summary')) event.stopPropagation();};
        host.addEventListener('keydown', this.onKey);
        this.onCatalog = () => this.refreshCatalog().catch(error => this.reportFailure(error));
        this.reseed = recipe => this.apply(cloneRecipe(recipe)).catch(error => {this.reportFailure(error); throw error;});
        ctx.reseedScale0Recipe = this.reseed;
        const header = node('header', undefined, 'seed-header'); header.append(node('h2', 'Seeding'));
        this.coverage = node('p', undefined, 'seed-hint'); header.append(this.coverage);
        this.select = node('select', undefined, 'ctrl-input'); this.select.id = 'seed-scenario';
        this.select.setAttribute('aria-label', 'Preparation');
        this.select.addEventListener('change', () => this.choose(this.select.value));
        const selectWrap = node('label', undefined, 'seed-field'); selectWrap.append(node('span', 'Preparation'), this.select);
        header.append(selectWrap);
        enhanceScenarioPicker(this.select, {id: 'seed-picker', label: 'Choose preparation', afterLabel: true});
        const searchWrap = node('label', undefined, 'seed-field'); searchWrap.append(node('span', 'Find properties'));
        this.search = node('input', undefined, 'ctrl-input'); this.search.type = 'search';
        this.search.setAttribute('aria-label', 'Find properties'); searchWrap.append(this.search); header.append(searchWrap);
        this.search.addEventListener('input', () => this.filter());
        const expand = node('div', undefined, 'seed-actions');
        expand.append(button('Expand all', () => this.body.querySelectorAll('details').forEach(el => {el.open = true;})),
            button('Collapse all', () => this.body.querySelectorAll('details').forEach(el => {el.open = false;})));
        header.append(expand);
        this.body = node('fieldset', undefined, 'seed-body');
        this.status = node('output', '', 'seed-status'); this.status.id = 'seed-status'; this.status.setAttribute('aria-live', 'polite');
        const footer = node('footer', undefined, 'seed-footer');
        this.previewButton = button('Preview seed', () => this.previewSeed()); this.previewButton.id = 'seed-preview';
        this.applyButton = button('Seed lattice', () => this.apply().catch(e => this.reportFailure(e))); this.applyButton.id = 'seed-apply';
        this.runButton = button('Seed & run', () => this.apply(undefined, true).catch(e => this.reportFailure(e))); this.runButton.id = 'seed-run';
        const primary = node('div', undefined, 'seed-actions'); primary.append(this.previewButton, this.applyButton, this.runButton);
        const secondary = node('div', undefined, 'seed-actions');
        secondary.append(button('Restore preset', () => this.restore()),
            button('Export recipe', () => void this.exportRecipe()), button('Import recipe', () => this.file.click()),
            button('View summary', () => void this.showSummary().then(() => {
                this.summarySection.open = true;
                this.summarySection.scrollIntoView({block:'start'});
            })), button('Copy summary', () => void this.copySummary()));
        this.cancel = button('Cancel preparation', () => {this.invalidate(); this.report('Preparation cancelled.');});
        this.file = node('input'); this.file.type = 'file'; this.file.accept = '.json,application/json'; this.file.hidden = true;
        this.file.id = 'seed-import'; this.file.addEventListener('change', () => this.importRecipe());
        this.summaryBox = node('div', undefined, 'seed-summary-box');
        this.summarySection = section('Resolved recipe'); this.summarySection.append(this.summaryBox);
        footer.append(this.status, primary, secondary, this.cancel, this.file);
        host.replaceChildren(header, this.body, this.summarySection, footer);
        document.getElementById('scenario-select')?.addEventListener('scenario-options-changed', this.onCatalog);
        void this.refreshCatalog().catch(error => this.reportFailure(error));
        this.off = subscribeScale0Qualification(q => {
            if (this.disposed) return;
            const externalChanged = this.externalScenario !== undefined
                && (this.externalScenario !== q.scenarioId || this.externalGeneration !== ctx._loadGeneration);
            this.externalScenario = q.scenarioId;
            this.externalGeneration = ctx._loadGeneration;
            if (externalChanged && !this.installing) void this.choose(q.scenarioId);
        });
    }
    async ensureCatalog() {
        if (this.catalog) return;
        this.catalog = await loadFiniteCatalog();
    }
    async refreshCatalog() {
        const scenarios = getScale0SeedingScenarios();
        // The main picker also emits on selection changes. Only a changed
        // catalog may rebuild this editor or invalidate a prepared draft.
        if (this.scenarios?.length === scenarios.length && this.scenarios.every((s, i) => s === scenarios[i])) return;
        this.scenarios = scenarios;
        const coverage = seedCoverage(this.scenarios);
        const finite = coverage.filter(row => row.capability === 'record-components').length;
        this.coverage.textContent = `${coverage.length - finite} native presets · ${finite} editable record preparations`;
        const groups = new Map();
        for (const s of this.scenarios) {
            if (!groups.has(s.category)) {const g = node('optgroup'); g.label = s.category; groups.set(s.category, g);}
            const option = node('option', s.title); option.value = s.id; groups.get(s.category).append(option);
        }
        this.select.replaceChildren(...groups.values());
        await this.choose(this.recipe?.scenarioId || getScale0QualificationState().scenarioId || 'flux-pulse');
    }
    invalidate() {
        this.revision++; this.abort?.abort(); this.abort = null;
        this.preview?.owner.dispose(); this.preview = null;
        clearTimeout(this.nativeTimer); this.nativeTimer = null;
        clearTimeout(this.finiteTimer); this.finiteTimer = null;
        this.setBusy(false);
    }
    currentScenario() { return this.scenarios.find(s => s.id === this.recipe.scenarioId); }
    async choose(id) {
        const scenario = this.scenarios.find(s => s.id === id); if (!scenario) return;
        this.invalidate(); const revision = this.revision;
        const size = scenario.backend === 'finite-records'
            ? getActiveScale0Bridge(this.ctx)?.latticeSize : this.ctx.bridge?.latticeSize;
        let recipe = drafts.get(id);
        if (!recipe) {
            if (scenario.backend === 'finite-records') {
                await this.ensureCatalog(); if (this.disposed || revision !== this.revision) return;
                const schema = findFiniteSchema(this.catalog, id, scenario.sizes?.includes(size) ? size : scenario.sizes?.at(-1));
                recipe = schema ? cloneRecipe(schema.recipe) : createRecipe(scenario, size || 33);
            } else recipe = createRecipe(scenario, size || 33);
        }
        if (scenario.backend !== 'finite-records') recipe.size = size || recipe.size;
        this.recipe = recipe; drafts.set(id, recipe); this.select.value = id;
        this.select.dispatchEvent(new Event('scenario-options-changed'));
        this.nativeProperties = null; this.nativeDescriptionTag = null; this.finiteDescription = null; this.descriptionTag = null;
        this.receipt = null;
        this.report('Draft only — the lattice has not changed.');
        try { await this.render(); } catch (error) { this.reportFailure(error); }
    }
    recipeChanged() {
        this.invalidate(); drafts.set(this.recipe.scenarioId, this.recipe);
        this.receipt = null;
        this.summaryBox.replaceChildren(node('p', 'Resolving the edited preparation…', 'seed-hint'));
        try {validateRecipe(this.recipe, this.scenarios); this.report('Modified draft — seed the lattice to apply.');}
        catch (error) {this.report(error.message, true);}
    }
    async structuralChange() { this.recipeChanged(); try { await this.render(); } catch(error) {this.reportFailure(error);} }
    renderSafely() { return this.render().catch(error => this.reportFailure(error)); }
    restore() {
        if (this.busy) return;
        drafts.delete(this.recipe.scenarioId); void this.choose(this.recipe.scenarioId);
    }
    setBusy(value) {
        this.busy = value; this.body.disabled = value; this.select.disabled = value;
        const picker = this.host.querySelector('#seed-picker');
        picker?.querySelector('summary').setAttribute('aria-disabled', String(value));
        if (picker && value) picker.open = false;
        for (const b of [this.previewButton, this.applyButton, this.runButton]) if (b) b.disabled = value;
        if (this.cancel) this.cancel.hidden = !value;
    }
    report(text, error = false) {this.status.textContent = text; this.status.dataset.error = String(error);}
    reportFailure(error) {if (!this.disposed && !error.seedSuperseded) this.report(error.message, true);}
    newId() {
        let id; do {id = `component-${++this.sequence}`;} while (this.recipe.components.some(c => c.id === id)); return id;
    }
    /** Finite: the full inactive union of all 9 record kinds. Native: the full
     * inactive union of every constructor registered in this scenario's own
     * category, as an ordered ingredient list (recipe.blank composite). */
    async openBase() {
        const scenario = this.currentScenario();
        if (scenario.backend === 'finite-records') {
            await this.ensureCatalog();
            const baseSchema = findFiniteBaseSchema(this.catalog, this.recipe.size);
            this.recipe = createBaseRecipe(scenario, baseSchema);
        } else {
            this.recipe = createNativeBaseRecipe(scenario, this.scenarios);
            this.recipe.size = this.ctx.bridge?.latticeSize || this.recipe.size;
        }
        await this.structuralChange();
    }
    async addComponent(kind) {
        if (this.recipe.components.length >= 64) return this.report('Maximum 64 components.', true);
        await this.ensureCatalog();
        const size = this.recipe.size;
        const template = findComponentTemplates(this.catalog, size, findFiniteBaseSchema(this.catalog, size)).get(kind);
        this.recipe.components.push(createComponent(template, this.newId(), size));
        await this.structuralChange();
    }
    async removeComponent(index) { this.recipe.components.splice(index, 1); await this.structuralChange(); }
    async duplicateComponent(index) {
        if (this.recipe.components.length >= 64) return this.report('Maximum 64 components.', true);
        const copy = cloneRecipe(this.recipe.components[index]); copy.id = this.newId();
        this.recipe.components.splice(index + 1, 0, copy); await this.structuralChange();
    }
    async moveComponent(index, offset) {
        const next = index + offset; if (next < 0 || next >= this.recipe.components.length) return;
        const c = this.recipe.components;
        [c[index], c[next]] = [c[next], c[index]]; await this.structuralChange();
    }
    async describeFinite() {
        const recipe = cloneRecipe(this.recipe), tag = JSON.stringify(recipe), revision = this.revision;
        if (this.descriptionTag === tag && this.finiteDescription) return this.finiteDescription;
        const registered = findFiniteSchema(this.catalog, recipe.scenarioId, recipe.size);
        let description;
        if (registered && JSON.stringify(registered.recipe) === tag) description = cloneRecipe(registered);
        else {
            const response = await fetch('/api/lattice/records/seed-description', {
                method: 'POST', headers: {'Content-Type': 'application/json'}, body: tag,
            });
            if (!response.ok) {
                const failure = await response.json().catch(() => ({}));
                throw new Error(failure.error || 'The local preparation service could not describe this draft.');
            }
            description = await response.json();
        }
        for (const p of description.properties) assertPropertyDescriptor(p);
        if (this.disposed || revision !== this.revision) return null;
        this.finiteDescription = description; this.descriptionTag = tag;
        return description;
    }
    scheduleFiniteRedescribe() {
        clearTimeout(this.finiteTimer);
        this.finiteTimer = setTimeout(() => {
            const revision = this.revision;
            void this.describeFinite().then(description => {
                if (description) { this.updateVisibleProperties(description.properties); void this.showSummary(); }
            }).catch(error => { if (revision === this.revision) this.reportFailure(error); });
        }, NATIVE_REDESCRIBE_DEBOUNCE_MS);
    }
    resizeDraft(size) {
        const recipe = this.recipe, oldSize = recipe.size;
        const oldSchema = recipe.blank ? findFiniteBaseSchema(this.catalog, oldSize)
            : findFiniteSchema(this.catalog, recipe.scenarioId, oldSize);
        const nextSchema = recipe.blank ? findFiniteBaseSchema(this.catalog, size)
            : findFiniteSchema(this.catalog, recipe.scenarioId, size);
        const oldTemplates = findComponentTemplates(this.catalog, oldSize, findFiniteBaseSchema(this.catalog, oldSize));
        const nextTemplates = findComponentTemplates(this.catalog, size, findFiniteBaseSchema(this.catalog, size));
        const follow = (current, before, after, path) => {
            for (const key of Object.keys(current)) {
                if (before?.[key] === undefined || after?.[key] === undefined) continue;
                if (hasFiniteOverride(recipe, [...path,key])) continue;
                if (JSON.stringify(current[key]) === JSON.stringify(before[key])) current[key] = cloneRecipe(after[key]);
                else if (current[key] && typeof current[key] === 'object' && !Array.isArray(current[key]))
                    follow(current[key], before[key], after[key], [...path,key]);
            }
        };
        for (const [i,component] of recipe.components.entries()) {
            const before = oldSchema?.recipe.components.find(c => c.id === component.id) || oldTemplates.get(component.kind);
            const after = nextSchema?.recipe.components.find(c => c.id === component.id) || nextTemplates.get(component.kind);
            follow(component.region, before?.region, after?.region, ['components',i,'region']);
            follow(component.parameters, before?.parameters, after?.parameters, ['components',i,'parameters']);
        }
        recipe.size = size;
    }
    updateVisibleProperties(properties) {
        if (!properties || this.disposed) return;
        const editors = [...this.body.querySelectorAll('[data-property-key]')];
        const byKey = new Map(properties.map(p => [p.key, p]));
        const keys = new Set(editors.map(el => el.dataset.propertyKey));
        if (properties.some(p => !keys.has(p.key)) || editors.some(el => !byKey.has(el.dataset.propertyKey))) {
            const focusedKey = document.activeElement?.closest('[data-property-key]')?.dataset.propertyKey;
            const open = [...this.body.querySelectorAll('details')].map(el => el.open);
            void this.render().then(() => {
                this.body.querySelectorAll('details').forEach((el, i) => {if (i < open.length) el.open = open[i];});
                [...this.body.querySelectorAll('[data-property-key]')].find(el => el.dataset.propertyKey === focusedKey)?.querySelector('input[type=number]')?.focus();
            }).catch(error => this.reportFailure(error));
            return;
        }
        for (const el of editors) {
            const p = byKey.get(el.dataset.propertyKey), explicit = getAtPath(this.recipe, p.path);
            const value = explicit === undefined ? p.value : explicit;
            el.updateProperty?.(p, value, p.path.includes('overrides') ? explicit !== undefined : JSON.stringify(value) !== JSON.stringify(p.default));
        }
        for (const el of this.body.querySelectorAll('[data-component-id]'))
            el.dataset.enabled = String(this.recipe.components.find(c => c.id === el.dataset.componentId)?.enabled);
    }
    /** A collection's own add/remove-row controls: the new row is always the
     * collection's registered defaultRow (never invented), and add/remove are
     * disabled past minRows/maxRows exactly as the compiler would reject them. */
    renderCollectionControls(collection) {
        const wrap = node('div', undefined, 'seed-actions seed-collection-actions');
        const bounds = collectionRowBounds(collection, collection.rows);
        const add = button(`+ ${collection.label}`, () => {
            getAtPath(this.recipe, collection.path).push(bounds.defaultRow);
            this.recipeChanged(); void this.renderSafely();
        });
        add.disabled = !bounds.canAdd; add.setAttribute('aria-label', `Add ${collection.label} row`);
        const remove = button(`− Remove last ${collection.label}`, () => {
            getAtPath(this.recipe, collection.path).pop();
            this.recipeChanged(); void this.renderSafely();
        });
        remove.disabled = !bounds.canRemove; remove.setAttribute('aria-label', `Remove last ${collection.label} row`);
        wrap.append(add, remove);
        return wrap;
    }
    /** Renders one property per descriptor, grouped into collapsible sections.
     * `afterEdit` runs in addition to the ordinary draft revalidation — used by
     * native overrides to schedule a debounced dependent-defaults redescribe.
     * `collections` (finite only) adds add/remove-row controls to their own group. */
    renderPropertyGroups(container, properties, {afterEdit, collections = []} = {}) {
        for (const [group, items] of groupProperties(properties)) {
            const box = section(group); container.append(box);
            for (const property of items) {
                assertPropertyDescriptor(property);
                const explicit = getAtPath(this.recipe, property.path);
                const value = explicit === undefined ? property.value : explicit;
                const nativeOverride = property.path.includes('overrides');
                const commit = next => {
                    if (this.currentScenario().backend === 'finite-records') markFiniteOverride(this.recipe, property.path);
                    setAtPath(this.recipe, property.path, next); this.recipeChanged();
                    if (afterEdit) afterEdit(); else this.scheduleFiniteRedescribe();
                };
                box.append(propertyEditor(property, value, commit, {
                    reset: () => {
                        if (nativeOverride) delete getAtPath(this.recipe, property.path.slice(0,-1))[property.path.at(-1)];
                        else {
                            setAtPath(this.recipe, property.path, property.default);
                            if (this.currentScenario().backend === 'finite-records') markFiniteOverride(this.recipe, property.path, false);
                        }
                        this.recipeChanged(); void this.renderSafely();
                    },
                    modified: nativeOverride ? explicit !== undefined : hasFiniteOverride(this.recipe,property.path) || JSON.stringify(value) !== JSON.stringify(property.default),
                }));
            }
            for (const collection of collections.filter(c => c.group === group)) box.append(this.renderCollectionControls(collection));
        }
    }
    renderComponent(component, index) {
        const wrap = node('div', undefined, 'seed-component'); wrap.dataset.componentId = component.id;
        const {properties, collections} = this.finiteDescription;
        wrap.dataset.enabled = String(component.enabled);
        this.renderPropertyGroups(wrap, properties.filter(p => p.path[0] === 'components' && p.path[1] === index),
            {collections: collections.filter(c => c.path[1] === index)});
        const actions = node('div', undefined, 'seed-actions');
        actions.append(button('Duplicate', () => void this.duplicateComponent(index)),
            button('Move up', () => void this.moveComponent(index, -1)),
            button('Move down', () => void this.moveComponent(index, 1)),
            button('Remove', () => void this.removeComponent(index)));
        wrap.append(actions);
        return wrap;
    }
    async render() {
        const revision = this.revision;
        const recipe = this.recipe, scenario = this.currentScenario();
        this.host.dataset.seedReady = 'false';
        const finite = scenario.backend === 'finite-records';
        if (finite) { await this.ensureCatalog(); await this.describeFinite(); }
        if (this.disposed || revision !== this.revision) return;
        this.body.replaceChildren();
        this.summaryBox.replaceChildren();
        const domain = section('Domain and starting state'); this.body.append(domain);
        domain.append(node('p', scenario.category, 'seed-hint'));
        if (finite) {
            const top = this.finiteDescription.properties.filter(p => p.path.length === 1);
            const sizes = recipe.blank ? RECORD_SEED_SIZES : scenario.sizes;
            const sizeProperty = {...top.find(p => p.path[0] === 'size'), options: sizes.map(n => [n, `${n}`])};
            for (const property of [sizeProperty, ...top.filter(p => p.path[0] !== 'size')]) {
                domain.append(propertyEditor(property, getAtPath(recipe, property.path),
                    next => {
                        if (property.path[0] === 'size') this.resizeDraft(next);
                        else setAtPath(recipe, property.path, next);
                        this.recipeChanged(); void this.renderSafely();
                    }, {reset: () => {
                        if (property.path[0] === 'size') this.resizeDraft(property.default);
                        else setAtPath(recipe,property.path,property.default);
                        this.recipeChanged(); void this.renderSafely();
                    }}));
            }
            domain.append(button('Open full category base', () => void this.openBase()));
            domain.append(node('p', 'Periodic finite domain. Components below replace selected records in list order. The tick law is unchanged.', 'seed-hint'));
            const add = node('div', undefined, 'seed-actions'); domain.append(add);
            for (const [kind, label] of Object.entries(COMPONENT_KINDS)) add.append(button(`+ ${label}`, () => void this.addComponent(kind)));
            for (const [index, c] of recipe.components.entries()) this.body.append(this.renderComponent(c, index));
            if (!recipe.components.length) this.body.append(node('p', 'The registered preparation is preserved exactly. Add a component to edit records or choose the empty category base.', 'seed-hint'));
        } else {
            domain.append(node('p', `Uses the native lattice size (${this.ctx.bridge?.latticeSize || recipe.size}). Change size with the main toolbar.`, 'seed-hint'));
            domain.append(node('p', recipe.blank
                ? 'Category base: every constructor registered in this category, inactive by default. Enable the ones you want; a later-listed ingredient owns its own source/profile fields at a shared site.'
                : 'Overrides below replace the registered preset\'s own constructor arguments. Reset restores the preset default and deletes the override.', 'seed-hint'));
            domain.append(button('Open full category base', () => void this.openBase()));
            this.domainSection = domain;
            await this.redescribeNative(true);
            if (this.disposed || revision !== this.revision) return;
        }
        const evidence = section('Preparation provenance'); this.body.append(evidence);
        evidence.append(node('p', scenario.epistemicStatus, 'seed-hint'), node('p', finite
            ? 'Edited preparations are custom initial states. Registered evidence remains attached to the original preset; transport recovery is open.'
            : scenario.intent || 'Native scenario defaults are applied by the engine.', 'seed-hint'));
        this.previewButton.textContent = 'Preview seed';
        this.filter();
        await this.showSummary();
        if (!this.disposed && revision === this.revision) {
            this.host.dataset.preparation = recipe.scenarioId;
            this.host.dataset.seedBase = String(recipe.blank);
            this.host.dataset.seedReady = 'true';
        }
    }
    /** Native property list comes from the live detached constructor (dependent
     * defaults), never cached across recipes. Internal recipe-composition control
     * keys (`recipe.*`, `ingredient.N.scenario`) are the compiler's own dispatch
     * plumbing, not user-facing properties, and are filtered out here. */
    async redescribeNative(rebuild) {
        const revision = this.revision, tag = JSON.stringify(this.recipe);
        try {
            const described = await describeNativeRecipe(this.ctx, this.recipe);
            if (this.disposed || revision !== this.revision) return;
            this.nativeProperties = described.properties
                .filter(p => !/^recipe\./.test(p.key) && !/^ingredient\.\d+\.scenario$/.test(p.key))
                .map(p => p.path.at(-1) === 'enabled'
                    ? {...p, default: !!p.default, value: !!p.value, type: 'choice', min: 0, max: 1, step: 1, options: [[false, 'Off — disabled'], [true, 'On — enabled']]}
                    : p);
            this.nativeDescriptionTag = tag;
        } catch (error) {
            if (this.disposed || revision !== this.revision) return;
            // Keep the last legal editor metadata so a coupled constraint error
            // can be corrected. It must never be exported as this draft's result.
            this.nativeDescriptionTag = null;
            this.report(error.message, true);
            throw error;
        }
        if (!rebuild) return;
        this.body.querySelector('.seed-native-properties')?.remove();
        const box = node('div', undefined, 'seed-native-properties');
        if (this.domainSection?.parentElement === this.body) this.domainSection.after(box);
        else this.body.append(box);
        const options = {afterEdit: () => this.scheduleNativeRedescribe()};
        if (this.recipe.blank) {
            this.renderPropertyGroups(box, (this.nativeProperties || []).filter(p => p.path[0] !== 'components'), options);
            this.recipe.components.forEach((component, i) => {
                const card = node('div', undefined, 'seed-component');
                card.dataset.componentId = component.id; card.dataset.enabled = String(component.enabled);
                card.append(node('h3', this.scenarios.find(s => s.id === component.scenarioId)?.title || component.scenarioId));
                this.renderPropertyGroups(card, (this.nativeProperties || []).filter(p => p.path[0] === 'components' && p.path[1] === i), options);
                const actions = node('div', undefined, 'seed-actions');
                actions.append(button('Move up', () => void this.moveComponent(i, -1)), button('Move down', () => void this.moveComponent(i, 1)));
                card.append(actions); box.append(card);
            });
        } else this.renderPropertyGroups(box, this.nativeProperties || [], options);
        this.filter();
    }
    scheduleNativeRedescribe() {
        clearTimeout(this.nativeTimer);
        this.nativeTimer = setTimeout(() => {
            const revision = this.revision;
            void this.redescribeNative(false).then(() => {
                if (revision === this.revision) { this.updateVisibleProperties(this.nativeProperties); void this.showSummary(); }
            }).catch(error => { if (revision === this.revision) this.reportFailure(error); });
        }, NATIVE_REDESCRIBE_DEBOUNCE_MS);
    }
    filter() {
        const query = this.search.value.trim().toLowerCase();
        for (const el of this.body.children) {
            el.hidden = !!query && !el.textContent.toLowerCase().includes(query);
            if (query && !el.hidden) { el.open = true; el.querySelectorAll?.('details').forEach(d => {d.open = true;}); }
        }
    }
    async previewSeed() {
        if (this.busy) return;
        let owner, revision;
        try {
            const recipe = cloneRecipe(this.recipe), scenario = validateRecipe(recipe, this.scenarios);
            this.invalidate(); revision = this.revision; const generation = this.ctx._loadGeneration;
            this.abort = new AbortController(); this.setBusy(true); this.report('Preparing an isolated seed preview…');
            owner = scenario.backend === 'finite-records'
                ? await prepareRecordRecipe(recipe, this.scenarios, this.abort.signal)
                : await prepareNativeRecipe(this.ctx, recipe, this.abort.signal);
            if (this.disposed || revision !== this.revision || generation !== this.ctx._loadGeneration) {owner.dispose(); return;}
            this.preview = {owner, recipe: JSON.stringify(recipe)};
            const diagnostics = owner.getDiagnostics?.();
            this.receipt = preparationReceipt(owner,recipe);
            const digest = owner.checkpointSHA256 ? ` SHA-256 ${owner.checkpointSHA256}` : '';
            this.report(owner.isFiniteRecord && diagnostics
                ? `Preview: ${recipe.size}³ sites · ${diagnostics.fieldTokens} field tokens · ${diagnostics.relationTokens} relation tokens · ${diagnostics.manifested} stored markers. The active lattice is unchanged.${digest}`
                : `Preview: ${recipe.size}³ sites staged. The active lattice is unchanged.${digest}`);
            await this.showSummary();
        } catch (error) {owner?.dispose(); if (!this.disposed && (revision === undefined || revision === this.revision)) this.report(error.message, true);}
        finally {if (!this.disposed && revision === this.revision) this.setBusy(false);}
    }
    async apply(input, run = false) {
        if (this.busy) throw new Error('Wait for or cancel the current preparation');
        const recipe = cloneRecipe(input || this.recipe), scenario = validateRecipe(recipe, this.scenarios);
        if (this.ctx.engineMode !== 'lattice') throw new Error('Switch to the lattice before seeding');
        if (scenario.backend === 'finite-records' && this.ctx.bridge?.isNativeGPU) throw new Error('Record preparation requires the local WASM connection');
        let owner = null, adopted = false;
        const cached = this.preview?.recipe === JSON.stringify(recipe) ? this.preview.owner : null;
        if (cached) this.preview = null;
        this.invalidate(); const revision = this.revision, generation = this.ctx._loadGeneration;
        this.abort = new AbortController(); const signal = this.abort.signal;
        this.setBusy(true); this.report('Preparing lattice seed…');
        try {
            const installed = await applySeed(this.ctx, {
                recipe, scenarios: this.scenarios, owner: cached, signal,
                loadScenario: (id, params) => this.actions.load(id, params),
                assertCurrent: () => {
                    if (this.disposed || revision !== this.revision || generation !== this.ctx._loadGeneration)
                        throw new Error('Seed superseded by another scenario or draft');
                },
                onInstalling: value => { this.installing = value; if (value) this.cancel.hidden = true; },
            });
            ({ owner } = installed); adopted = true;
            const { custom, installedGeneration } = installed;
            this.receipt = preparationReceipt(owner,recipe);
            this.report(`Applied ${custom ? 'custom preparation' : 'registered preset'} · tick ${getActiveScale0Bridge(this.ctx)?.currentTick?.() ?? 0}.`);
            await this.showSummary();
            if (run && !this.ctx.running && !this.disposed && revision === this.revision && installedGeneration === this.ctx._loadGeneration) this.ctx.togglePlay?.();
        } catch (error) {
            error.seedSuperseded = this.disposed || revision !== this.revision;
            throw error;
        } finally {
            this.installing = false; if (!adopted) owner?.dispose();
            if (!this.disposed && revision === this.revision) this.setBusy(false);
        }
    }
    /** Resolve this exact draft before exporting or displaying dependent values. */
    async currentDescription() {
        const scenario = this.currentScenario();
        if (scenario.backend === 'finite-records') { await this.ensureCatalog(); return this.describeFinite(); }
        if (!this.nativeProperties || this.nativeDescriptionTag !== JSON.stringify(this.recipe)) await this.redescribeNative(false);
        return {schemaIdentity: 'native-seed-2', properties: this.nativeProperties || [],
            ingredients: this.recipe.components.map(c => ({id:c.id, label:this.scenarios.find(s=>s.id===c.scenarioId)?.title || c.scenarioId}))};
    }
    async exportRecipe() {
        const revision = this.revision, recipe = cloneRecipe(this.recipe);
        try {
            const scenario = validateRecipe(recipe, this.scenarios);
            const custom = scenario.backend === 'finite-records' ? await isEditedRecordRecipe(recipe) : isCustomNativeRecipe(recipe);
            const description = await this.currentDescription();
            if (this.disposed || revision !== this.revision || !description) return;
            const envelope = buildRecipeEnvelope(recipe, scenario, description, {custom, receipt:this.currentReceipt()});
            const url = URL.createObjectURL(new Blob([JSON.stringify(envelope, null, 2)], {type: 'application/json'}));
            const a = node('a'); a.href = url; a.download = `${recipe.scenarioId}-seed.json`; a.click();
            setTimeout(() => URL.revokeObjectURL(url), 1000);
        } catch (error) {this.report(error.message, true);}
    }
    async importRecipe() {
        const file = this.file.files[0]; this.file.value = ''; if (!file) return;
        const revision = this.revision;
        try {
            if (file.size > MAX_RECIPE_ENVELOPE_BYTES) throw new Error('Recipe envelope exceeds 2 MiB');
            let recipe = parseRecipeEnvelope(await file.text(), this.scenarios);
            if (recipe.version !== 2) {
                const scenario = this.scenarios.find(s => s.id === recipe.scenarioId);
                if (!scenario) throw new Error('Unknown scenario; its preparation service may be unavailable');
                let defaultRecipe = null;
                if (scenario.backend === 'finite-records') {
                    await this.ensureCatalog();
                    const schema = findFiniteSchema(this.catalog, recipe.scenarioId, recipe.size);
                    if (!schema) throw new Error('No registered default is available to expand this legacy recipe');
                    defaultRecipe = schema.recipe;
                }
                recipe = migrateRecipe(recipe, scenario, defaultRecipe);
                validateRecipe(recipe, this.scenarios);
            }
            if (this.disposed || revision !== this.revision || this.busy) return;
            drafts.set(recipe.scenarioId, recipe); await this.choose(recipe.scenarioId); this.report('Imported draft. Active lattice unchanged.');
        } catch (error) {this.report(error.message, true);}
    }
    async copySummary() {
        try {
            const scenario = this.currentScenario();
            const description = await this.currentDescription();
            const applied = this.ctx._appliedSeedRecipe;
            const custom = scenario.backend === 'finite-records' ? await isEditedRecordRecipe(this.recipe) : isCustomNativeRecipe(this.recipe);
            const text = recipeText(this.recipe, scenario, description, {applied, custom, receipt: this.currentReceipt()});
            await navigator.clipboard.writeText(text);
            this.report('Recipe summary copied to the clipboard.');
        } catch (error) {this.report(error.message, true);}
    }
    async showSummary() {
        const revision = this.revision;
        try {
            const scenario = this.currentScenario();
            const description = await this.currentDescription();
            const applied = this.ctx._appliedSeedRecipe;
            const custom = scenario.backend === 'finite-records' ? await isEditedRecordRecipe(this.recipe) : isCustomNativeRecipe(this.recipe);
            if (this.disposed || revision !== this.revision || !description) return;
            const open = [...this.summaryBox.querySelectorAll('details')].map(el => el.open);
            this.summaryBox.replaceChildren(renderRecipeSummary(this.recipe, scenario, description, {applied, custom, receipt: this.currentReceipt()}));
            this.summaryBox.querySelectorAll('details').forEach((el, i) => {if(i < open.length) el.open = open[i];});
            this.summaryBox.hidden = false;
        } catch (error) {this.report(error.message, true);}
    }
    dispose() {
        this.disposed = true; this.invalidate(); this.off?.();
        this.host.removeEventListener('keydown', this.onKey);
        document.getElementById('scenario-select')?.removeEventListener('scenario-options-changed', this.onCatalog);
        if (this.ctx.reseedScale0Recipe === this.reseed) delete this.ctx.reseedScale0Recipe;
        this.host.replaceChildren();
    }
    currentReceipt() {
        const tag=JSON.stringify(this.recipe);
        if (this.receipt?.recipeTag !== tag) return null;
        return {...this.receipt, ...(this.descriptionTag === tag && this.finiteDescription?.geometrySupport
            ? {geometrySupport:this.finiteDescription.geometrySupport} : {})};
    }
}
export function initSeedingPanel(ctx, actions) {
    const host = document.getElementById('panel-seeding');
    if (host && !instance) instance = new SeedingPanel(host, ctx, actions);
    return instance;
}
export function disposeSeedingPanel() {instance?.dispose(); instance = null;}
