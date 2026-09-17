import { syncRecordPanels, updateRecordPanels } from './record-panels.js';
/** Passive record controls in the existing Scale-0 controls dock. */
import { loadRecordCatalog } from '../../../../bridge/finite-record-bridge.js';
import { populateScale0ScenarioSelect, registerScale0RecordScenarios } from '../../scenario-registry.js';
import { getActiveScale0Bridge } from '../../state/store.js';
import { FIELD_TOGGLE_BINDINGS } from '../dom.js';

let catalog = null, initialization = null;
export function getWebRecordCatalog() { return catalog; }
export function initializeRecordScenarios(ctx) {
    if (initialization) return initialization;
    initialization = loadRecordCatalog().then(value => {
        catalog = value; registerScale0RecordScenarios(value);
        const select = document.getElementById('scenario-select');
        populateScale0ScenarioSelect(select, select.value);
        const card = document.createElement('section'); card.className = 'card'; card.id = 'record-observation-card'; card.hidden = true;
        card.innerHTML = `<h3 class="card-title">Finite records</h3>
            <label class="ctrl-label" for="record-quantity">Lattice volume</label>
            <select class="ctrl-input" id="record-quantity"><option value="tokens">All stored tokens</option>
            <option value="field_tokens">Field tokens</option><option value="relation_tokens">Relation tokens</option>
            <option value="incidence">Integer incidence magnitude |Q|</option></select>
            <p>Exact site counts in the normal lattice volume. Manifestation markers show the stored ternary state.</p><p id="record-view-note" hidden></p>
            <output id="record-readout" style="display:block;white-space:pre-wrap"></output>
            <details><summary>State provenance</summary><pre id="record-provenance" style="white-space:pre-wrap;overflow-wrap:anywhere"></pre></details>
            <details><summary>Tests and preparation coverage</summary><p id="record-coverage"></p>
            <label for="record-test-filter">Find test</label><input class="ctrl-input" id="record-test-filter" type="search">
            <ul id="record-tests"></ul></details>`;
        document.getElementById('panel-controls-grid').prepend(card);
        document.getElementById('record-quantity').onchange = event => {
            const owner = getActiveScale0Bridge(ctx);
            if (owner?.isFiniteRecord) owner.setRecordQuantity(event.target.value);
        };
        const coverage = document.getElementById('record-coverage');
        coverage.textContent = `${value.scenarios.length} preparations; ${value.tests.length} Python modules. Loading does not run tests. Physical recovery remains open.`;
        for (const row of value.excluded_from_this_runtime) {
            const p = document.createElement('p'); p.textContent = `${row.family}: ${row.reason}`; coverage.appendChild(p);
        }
        const showTests = () => {
            const filter = document.getElementById('record-test-filter').value.toLowerCase();
            document.getElementById('record-tests').replaceChildren(...[...value.tests.map(row => ({path: row.module, command: row.command})), ...value.backend_checks]
                .filter(row => JSON.stringify(row).toLowerCase().includes(filter)).map(row => {
                    const li = document.createElement('li'); li.textContent = `${row.path}: ${row.command} (not run by catalog)`; return li;
                }));
        };
        document.getElementById('record-test-filter').oninput = showTests; showTests();
        return value;
    }).catch(() => null); // Static/hosted dashboard has no local compiled-record endpoint.
    return initialization;
}

export function syncRecordControls(ctx, spec = null) {
    const active = !!spec;
    syncRecordPanels(active);
    const note = document.getElementById('record-view-note');
    if (note) {
        note.hidden = !spec?.category?.includes('mixed');
        note.textContent = 'Mixed controls have occupied relations at every site. The Field tokens view isolates the probe. Phase and slot differences are not resolved by these count-only views.';
    }
    const source = document.getElementById('flux-volume-source');
    if (source) source.textContent = active
        ? 'Selected record counts (or |Q|), without energy conversion or neighbour smoothing. These controls change presentation only.'
        : 'Effective field activation proxy; these controls change presentation only.';
    const label = document.getElementById('flux-threshold-label');
    if (label) {
        label.dataset.effectiveTooltip ??= label.dataset.uiTooltip || label.title;
        const explanation = active ? 'Relative cutoff on the selected scalar count (or |Q|), using the held visual peak. No energy conversion, neighbour smoothing or state-energy contribution.' : label.dataset.effectiveTooltip;
        label.title = explanation; label.dataset.uiTooltip = explanation;
    }
    const threshold = document.getElementById('flux-threshold');
    if (threshold) threshold.setAttribute('aria-label', active ? 'Relative record-count threshold' : 'Relative local activation-energy threshold');
    const card = document.getElementById('record-observation-card'); if (card) card.hidden = !active;
    // These actions belong to the effective law. Keep the normal dock and its
    // supported volume, particle and selection controls; mark only unsupported
    // actions unavailable instead of redirecting them to a second owner.
    const elements = [
        ...document.querySelectorAll('[data-scale0-control-card="physics"], [data-scale0-control-card="substrate"], [data-scale0-control-card="flow-lines"]'),
        ...FIELD_TOGGLE_BINDINGS.filter(([, flag]) => flag !== 'showStateField').map(([id]) => document.getElementById(id)),
        document.getElementById('flux-boundary-mode'), document.getElementById('flux-periodic-axis'),
        document.getElementById('toggle-flux-slice'), document.getElementById('btn-prime-tick'),
    ].filter(Boolean);
    for (const el of elements) {
        if (active && !el.dataset.recordControlHeld) {
            el.dataset.recordControlHeld = '1'; el.dataset.recordPriorInert = String(el.inert);
            el.dataset.recordPriorTitle = el.title; el.inert = true;
            el.title = 'Unavailable for this finite-record law'; el.setAttribute('aria-disabled', 'true');
            el.style.opacity = '.45';
        } else if (!active && el.dataset.recordControlHeld) {
            el.inert = el.dataset.recordPriorInert === 'true'; el.title = el.dataset.recordPriorTitle;
            delete el.dataset.recordControlHeld; delete el.dataset.recordPriorInert; delete el.dataset.recordPriorTitle;
            el.removeAttribute('aria-disabled'); el.style.opacity = '';
        }
    }
    for (const checkbox of document.querySelectorAll('[data-scale0-control-card="physics"] input[type="checkbox"]')) {
        if (active) {
            if (checkbox.dataset.recordPriorChecked === undefined) checkbox.dataset.recordPriorChecked = String(checkbox.checked);
            checkbox.checked = false;
        } else if (checkbox.dataset.recordPriorChecked !== undefined) {
            checkbox.checked = checkbox.dataset.recordPriorChecked === 'true'; delete checkbox.dataset.recordPriorChecked;
        }
    }
    const sizes = document.getElementById('lattice-size');
    if (!sizes) return;
    for (const option of [...sizes.options]) {
        if (!active && option.dataset.recordExtra) { option.remove(); continue; }
        if (active) {
            if (option.dataset.recordPriorDisabled === undefined) option.dataset.recordPriorDisabled = String(option.disabled);
            option.disabled = !spec.sizes.includes(Number(option.value));
        } else if (option.dataset.recordPriorDisabled !== undefined) {
            option.disabled = option.dataset.recordPriorDisabled === 'true'; delete option.dataset.recordPriorDisabled;
        }
    }
    if (active) for (const n of spec.sizes) if (![...sizes.options].some(option => option.value === String(n))) {
        const option = new Option(String(n), String(n)); option.dataset.recordExtra = '1'; sizes.add(option);
    }
    if (active) {
        document.getElementById('flux-boundary-mode').value = '0';
        document.getElementById('flux-periodic-axis').value = '3';
    }
}

export function updateRecordReadout(owner) {
    updateRecordPanels(owner);
    const el = document.getElementById('record-readout'); if (!el) return;
    const diag = owner.getDiagnostics();
    el.textContent = owner.failed ? 'Runtime failed; paused. Reset to reload.' : !owner.ready ? 'Loading complete records…' :
        `${owner.scenario?.seedRecipe ? 'Custom preparation\n' : ''}Microtick ${diag.tick} · ${['Admission', 'Collision / relations', 'Streaming', 'Manifestation'][diag.phase]}\nField tokens ${diag.fieldTokens} · relation tokens ${diag.relationTokens}\nInteger Q sum ${diag.incidence}`;
    document.getElementById('record-provenance').textContent = JSON.stringify(owner.getProvenance(), null, 2);
    document.getElementById('record-quantity').value = owner.quantity;
}
