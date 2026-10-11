// The Scale 0 visualization layer table: one declaration per layer, grouped by
// the quantity it draws, with an equation-first tooltip and no status text.
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import {
    GROUP_TO_TOGGLES, LAYER_GROUPS, LAYERS, SCALAR_RENDER_TOGGLES, VECTOR_STYLE_TOGGLES, layerTooltip,
} from '../js/scales/scale0/ui/overlays/layer-catalog.js';
import { COL_TO_TOGGLES } from '../js/scales/scale0/ui/overlays/presets.js';
import { FIELD_TOGGLE_BINDINGS } from '../js/scales/scale0/ui/dom.js';

const read = (path) => readFileSync(new URL(`../js/${path}`, import.meta.url), 'utf8');
const byId = new Map(LAYERS.map((layer) => [layer.id, layer]));
const groupOf = (id) => LAYER_GROUPS.find((group) => group.layers.some((layer) => layer.id === id))?.id;

test('thirty-seven layers in eight groups, each declared once', () => {
    assert.deepEqual(LAYER_GROUPS.map((group) => [group.id, group.label, group.layers.length]), [
        ['flux', 'Flux J', 7],
        ['sources', 'Sources and matter', 6],
        ['curl', 'Curl, E and B', 4],
        ['energy', 'Energy', 6],
        ['forces', 'Forces', 4],
        ['clocks', 'Gravity and clocks', 6],
        ['dual', 'Dual substrate', 3],
        ['reference', 'Reference', 1],
    ]);
    assert.equal(LAYERS.length, 37);
    assert.equal(byId.size, 37, 'no id appears twice');
    assert.equal(COL_TO_TOGGLES, GROUP_TO_TOGGLES, 'the panel and the table share one map');
    assert.deepEqual(Object.values(GROUP_TO_TOGGLES).flat(), LAYERS.map((layer) => layer.id));
    assert.ok(Object.isFrozen(LAYERS) && LAYERS.every(Object.isFrozen));
});

test('every wired switch is in the table, and nothing else is', () => {
    const bound = FIELD_TOGGLE_BINDINGS.map(([id]) => id);
    for (const id of bound) assert.ok(byId.has(id), `${id} is bound but missing from the table`);
    const own = ['toggle-flux-volume', 'toggle-flux-slice', 'toggle-sm-reference'];
    assert.deepEqual([...byId.keys()].filter((id) => !bound.includes(id)).sort(), [...own].sort(),
        'only the three layers with their own handlers are outside the shared binding list');
    // The panel markup is generated from the table: no layer is written by hand.
    assert.doesNotMatch(read('scales/scale0/ui/overlays/template.js'), /id="toggle-|'toggle-[a-z]/);
});

test('layers that show the same quantity sit together', () => {
    assert.equal(groupOf('toggle-div-field'), groupOf('toggle-charge-density'), 'divergence as points and as a sheet');
    assert.equal(groupOf('toggle-force-weak'), groupOf('toggle-vorticity'), 'the curl as arrows and as a magnitude');
    assert.equal(groupOf('toggle-b-field'), 'curl');
    assert.equal(groupOf('toggle-latency'), groupOf('toggle-horizon'), 'latency and its peak marker');
    assert.equal(groupOf('toggle-dual-substrate'), groupOf('toggle-phase'));
    assert.deepEqual(GROUP_TO_TOGGLES.forces, ['toggle-force-em', 'toggle-force-gravity', 'toggle-force-strong', 'toggle-confinement']);
});

test('the Rendering rows apply to the layers the renderer treats that way', () => {
    const fieldKey = new Map(FIELD_TOGGLE_BINDINGS);
    const forceKeys = [...read('scales/scale0/state/store.js')
        .match(/export const FORCE_FIELD_KEYS = new Set\(\[([\s\S]*?)\]\)/)[1].matchAll(/'(show\w+)'/g)].map((m) => m[1]);
    assert.deepEqual(VECTOR_STYLE_TOGGLES.map((id) => fieldKey.get(id)).sort(), forceKeys.sort());
    const scalarKeys = [...read('scales/scale0/viewport-scalar-adapter.js').matchAll(/^\s+(show\w+):\s+\{ key:/gm)].map((m) => m[1]);
    assert.equal(scalarKeys.length, 11);
    assert.deepEqual(SCALAR_RENDER_TOGGLES.map((id) => fieldKey.get(id)).sort(), scalarKeys.sort());
    for (const layer of LAYERS) assert.ok([undefined, 'scalar', 'vector'].includes(layer.render), layer.id);
});

test('tooltips lead with the equation, then say what is drawn, and carry no status text', () => {
    for (const layer of LAYERS) {
        const tip = layerTooltip(layer);
        if (layer.id === 'toggle-sm-reference') {
            assert.equal(layer.equation, '', 'a reference card has no equation');
            assert.equal(tip, layer.detail);
        } else {
            assert.ok(layer.equation.length > 0, `${layer.id} has an equation`);
            assert.ok(tip.startsWith(`\\[${layer.equation}\\] `), `${layer.id} leads with its equation`);
            // TeX is HTML-escaped before KaTeX sees it.
            assert.doesNotMatch(layer.equation, /[<>&]/, `${layer.id}: use \\lt and \\gt, and no alignment`);
        }
        assert.match(layer.detail, /^[A-Z].*\.$/s, `${layer.id}: detail is a sentence`);
        assert.ok(tip.length <= 420, `${layer.id}: ${tip.length} characters`);
        const text = `${layer.label} ${layer.detail}`;
        assert.doesNotMatch(text, /\[[A-Z][A-Z —-]*\]/, `${layer.id}: no status tag`);
        assert.doesNotMatch(text, /LEDGER|FTD-\d{4}|\bNOT\b|CLOSED NEGATIVE|PROXY|PARAMETRIC|\baudit\b/, `${layer.id}: no claim text`);
    }
});

test('labels name the quantity, and the earlier names still find them', () => {
    const renamed = {
        'toggle-psi-squared': ['|J|²', 'psi'],
        'toggle-dark-halo': ['Sub-threshold flux', 'halo'],
        'toggle-horizon': ['Latency peak (L ≥ 0.95)', 'horizon'],
        'toggle-confinement': ['Pair links', 'confinement'],
        'toggle-force-strong': ['Flux-tube force', 'strong'],
        'toggle-entropy-density': ['Disorder 4p(1−p)', 'entropy'],
        'toggle-lagrangian-density': ['½|E|² − ½(∇·J)²', 'lagrangian'],
        'toggle-color-charge': ['Axis label', 'color'],
        'toggle-phase': ['arg(J_L + i J_R)', 'phase'],
        'toggle-dual-substrate': ['Amplitude split', 'dual'],
    };
    for (const [id, [label, earlier]] of Object.entries(renamed)) {
        assert.equal(byId.get(id).label, label);
        assert.ok(byId.get(id).search.includes(earlier), `${id} is still found by "${earlier}"`);
    }
    for (const [id, label] of Object.entries({
        'toggle-e-field': 'Radiative E', 'toggle-b-field': 'B field', 'toggle-force-em': 'EM',
        'toggle-force-gravity': 'Gravity', 'toggle-genesis-iso': 'Genesis', 'toggle-latency': 'Latency L',
    })) assert.equal(byId.get(id).label, label);
    assert.equal(new Set(LAYERS.map((layer) => layer.label)).size, 37, 'no two layers share a label');
});

test('layer-owned controls are declared with their layer', () => {
    const sheets = LAYERS.filter((layer) => layer.sheet);
    assert.deepEqual(sheets.map((layer) => layer.sheet.key).sort(),
        ['bPressure', 'chargeDensity', 'ePressure', 'emEnergy', 'gravPotential', 'vorticity']);
    for (const layer of sheets) assert.equal(layer.render, 'scalar', `${layer.id}: only sheet layers have a slice height`);
    assert.deepEqual(byId.get('toggle-flux-volume').sub.items.map((item) => item.id), ['toggle-flux-organic', 'toggle-flux-glow']);
    assert.deepEqual(byId.get('toggle-flux-slice').sub.items.map((item) => item.id),
        ['flux-slice-axis-xy', 'flux-slice-axis-xz', 'flux-slice-axis-yz']);
    assert.deepEqual(LAYERS.filter((layer) => layer.active).map((layer) => layer.id), ['toggle-flux-volume']);
});
