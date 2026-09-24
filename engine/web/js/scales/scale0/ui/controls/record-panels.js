/** Extend the existing dock hosts; all preparations use their current owner. */
import { getPanelsForScale } from '../../../../ui/scale-registry/panel-registry.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { RECORD_PANEL_CONTRACTS, summarizeRecordObservation, recordSlice, appendRecordSample } from './record-panel-model.js';

const samples = new WeakMap();
const managed = new Map();
let activeOwner = null;
let activeStatus = null;
const node = (tag, text) => { const el = document.createElement(tag); if (text !== undefined) el.textContent = text; return el; };
function table(headers, rows) {
    const el = node('table'), head = node('thead'), tr = node('tr'), body = node('tbody');
    headers.forEach(label => { const th = node('th', label); th.scope = 'col'; tr.append(th); }); head.append(tr);
    rows.forEach(row => { const tr = node('tr'); row.forEach(value => tr.append(node('td', value))); body.append(tr); });
    el.append(head, body); return el;
}
export function syncRecordPanels(active) {
    if (!active) {
        for (const [host, entry] of managed) { delete host.dataset.recordObservation; entry.body.remove(); }
        managed.clear(); activeOwner = null; activeStatus = null; return;
    }
    for (const def of getPanelsForScale(0)) {
        if (RECORD_PANEL_CONTRACTS[def.id] === 'existing') continue;
        const host = document.getElementById(`panel-${def.id}`); if (!host) continue;
        if (managed.get(host)?.body.isConnected) continue;
        const body = node('section'); body.className = 'record-panel-observation';
        body.setAttribute('aria-label', `${def.label} record observations`);
        host.dataset.recordObservation = 'true'; host.append(body);
        managed.set(host, {body, def, key: null});
    }
}
export function updateRecordPanels(owner) {
    syncRecordPanels(true);
    const status = owner.failed ? 'failed' : owner.ready ? 'ready' : 'loading';
    if (activeOwner !== owner || activeStatus !== status) {
        for (const {body, def} of managed.values()) {
            // Retained and floated hosts must never show the previous owner's sample.
            body.replaceChildren(node('h3', def.label), node('p', owner.failed ? 'Runtime failed; paused. Reset to reload.' : 'Loading complete records…'));
            body.dataset.owner = owner.ownerId || ''; body.dataset.tick = '';
        }
        for (const entry of managed.values()) entry.key = null;
        activeOwner = owner;
        activeStatus = status;
    }
    let state = samples.get(owner);
    if (!state) { state = {view: null, history: [], summary: null}; samples.set(owner, state); }
    if (owner.observation && state.view !== owner.observation) {
        state.view = owner.observation; state.summary = summarizeRecordObservation(state.view);
        appendRecordSample(state.history, state.summary);
    }
    for (const [host, entry] of managed) {
        if (!isPanelLive(host, {recordObservation: true})) continue;
        const {body, def} = entry;
        const key = `${owner.ownerId}:${owner.dataVersion}:${owner.ready}:${owner.failed}:${owner.quantity}`;
        if (entry.key === key) continue; entry.key = key;
        body.replaceChildren(node('h3', def.label));
        body.dataset.owner = owner.ownerId || ''; body.dataset.tick = state.summary?.tick || '';
        if (owner.failed || !owner.ready || !state.summary) {
            body.append(node('p', owner.failed ? 'Runtime failed; paused. Reset to reload.' : 'Loading complete records…')); continue;
        }
        const s = state.summary, contract = RECORD_PANEL_CONTRACTS[def.id];
        body.append(node('p', `${owner.scenario?.title || owner.scenario?.id} · microtick ${s.tick} · ${s.phase}`));
        const counts = [['Field tokens', s.field], ['Relation tokens', s.relation], ['Signed integer Q sum', s.incidence],
            ['Stored −1 sites', s.negative], ['Stored 0 sites', s.zero], ['Stored +1 sites', s.positive]];
        if (contract === 'counts') {
            body.append(table(['Observable', 'Exact count'], counts));
            const first = state.history[0];
            body.append(node('p', `Q change from first retained observation: ${BigInt(s.incidence) - BigInt(first.incidence)}. This is sampled accounting, not a conservation proof.`));
        } else if (contract === 'clock') {
            body.append(table(['Clock', 'Value'], [['Microtick', s.tick], ['Cycle index', (BigInt(s.tick) / 4n).toString()], ['Phase', s.phase]]),
                node('p', 'One Step advances one microtick. Four scheduled phases form a cycle. Proper time, lapse and seconds are not identified.'));
        } else if (contract === 'occupancy') {
            body.append(table(['Occupancy observable', 'Exact count'], counts), node('p', 'Stored occupancy and token counts only. Temperature, thermodynamic entropy and heat transport are not identified.'));
        } else if (contract === 'geometry') {
            body.append(table(['Geometry', 'Value'], [['Edge', s.size], ['Sites', s.size ** 3], ['Observation width', '1 site'], ['Boundary', 'Periodic cube']]),
                node('p', 'Coordinates are lattice indices. No conversion to metres or higher-scale recovery is applied.'));
        } else if (contract === 'slice') {
            const label = node('label', 'Slice quantity '), select = node('select');
            for (const [value, text] of [['tokens','All tokens'],['field_tokens','Field tokens'],['relation_tokens','Relation tokens'],['incidence','Signed Q']]) {
                const option = node('option', text); option.value = value; select.append(option);
            }
            select.value = owner.quantity; select.onchange = () => owner.setRecordQuantity(select.value); label.append(select); body.append(label);
            body.append(node('p', `Exact scalar slice z=${Math.floor(s.size / 2)}. Rows are y; columns are x. Signed Q is retained here; the volume shows |Q|.`),
                table(['y / x', ...Array.from({length:s.size}, (_, i) => i)], recordSlice(state.view, owner.quantity).map((row, y) => [y, ...row])));
        } else if (contract === 'history') {
            body.append(node('p', 'Last 128 completed observations; batches can skip microticks. Changing views does not advance the state.'));
            const values = state.history.map(row => Number(row.field) + Number(row.relation)), max = Math.max(1, ...values);
            const svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg'); svg.setAttribute('viewBox','0 0 400 140'); svg.setAttribute('role','img'); svg.setAttribute('aria-label','Total stored tokens by observation index');
            const path = document.createElementNS(svg.namespaceURI, 'path'); path.setAttribute('d', values.map((v,i) => `${i ? 'L':'M'}${i * 400 / Math.max(1,values.length-1)},${135 - v / max * 130}`).join(' ')); path.setAttribute('fill','none'); path.setAttribute('stroke','currentColor'); svg.append(path); body.append(svg);
            body.append(table(['Microtick', 'Field', 'Relations', 'Signed Q'], state.history.slice(-16).map(row => [row.tick,row.field,row.relation,row.incidence])));
        } else {
            body.append(node('p', contract || 'No compatible instrument is registered for this panel.'), table(['Available observation', 'Exact count'], counts.slice(0,3)));
        }
        body.append(node('p', `Source: current compiled record owner. Physical recovery remains open.`));
    }
}
