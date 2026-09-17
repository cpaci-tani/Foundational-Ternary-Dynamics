import { COMPONENT_KINDS } from './recipe.js';

export function node(tag, text, className) {
    const el = document.createElement(tag);
    if (text !== undefined) el.textContent = text;
    if (className) el.className = className;
    return el;
}
export function button(text, action) {
    const el = node('button', text, 'ctrl-btn-secondary'); el.type = 'button';
    el.addEventListener('click', action); return el;
}
export function section(title) {
    const el = node('details', undefined, 'seed-section'); el.open = true;
    el.append(node('summary', title)); return el;
}
export function inputField(parent, label, value, change, {options, min, max, step = 1, type = 'number'} = {}) {
    const wrap = node('label', undefined, 'seed-field'); wrap.append(node('span', label));
    const input = node(options ? 'select' : 'input', undefined, 'ctrl-input');
    if (options) for (const [key, title] of options) {
        const option = node('option', title); option.value = String(key); input.append(option);
    }
    else {
        input.type = type;
        if (min !== undefined) input.min = min;
        if (max !== undefined) input.max = max;
        input.step = step;
    }
    if (type === 'checkbox' && !options) input.checked = value;
    else input.value = String(value);
    input.setAttribute('aria-label', label);
    input.addEventListener('change', () => change(type === 'checkbox' && !options ? input.checked
        : options ? input.value : type === 'number' ? input.valueAsNumber : input.value));
    wrap.append(input); parent.append(wrap); return input;
}

export function componentEditor(c, size, channels, {changed, remove, duplicate, move}) {
    const root = section(COMPONENT_KINDS[c.kind]); root.dataset.componentId = c.id;
    const content = node('div', undefined, 'seed-fields'); root.append(content);
    inputField(content, 'Include component', c.enabled, value => { c.enabled = value; changed(); }, {type: 'checkbox'});
    const region = section('Placement and support'); content.append(region);
    const fields = node('div', undefined, 'seed-fields'); region.append(fields);
    inputField(fields, 'Shape', c.region.shape, value => {c.region.shape = value; showRegion(); changed();},
        {options: [['point', 'Single site'], ['box', 'Box'], ['sphere', 'Sphere'], ['all', 'Entire lattice']]});
    const position = node('div', undefined, 'seed-vector'); fields.append(position);
    for (const axis of ['x', 'y', 'z']) inputField(position, axis.toUpperCase(), c.region[axis], value => {c.region[axis] = value; changed();}, {min: 0, max: size - 1});
    const extent = node('div', undefined, 'seed-vector'); fields.append(extent);
    for (const axis of ['x', 'y', 'z']) inputField(extent, `${axis.toUpperCase()} extent`, c.region[`d${axis}`], value => {c.region[`d${axis}`] = value; changed();}, {min: 1, max: size});
    const radius = inputField(fields, 'Radius (cells)', c.region.radius, value => {c.region.radius = value; changed();}, {min: 0, max: size, step: 0.25});
    function showRegion() {
        position.hidden = c.region.shape === 'all'; extent.hidden = c.region.shape !== 'box';
        radius.parentElement.hidden = c.region.shape !== 'sphere';
    }
    showRegion();
    region.append(node('p', 'Coordinates are site indices. Spheres clip at domain edges; boxes must fit entirely inside. Masks do not wrap.', 'seed-hint'));
    const p = c.parameters;
    if (c.kind === 'field') {
        inputField(content, 'Field channel', p.channel, value => {p.channel = Number(value); changed();}, {
            options: channels.map(ch => [ch.id, `${ch.id} · phase ${ch.phase} · ${ch.polarity > 0 ? '+' : '−'} · flag ${JSON.stringify(ch.flag)}`]),
        });
        inputField(content, 'Occupied', p.occupied, value => {p.occupied = value; changed();}, {type: 'checkbox'});
        inputField(content, 'Selected fraction', p.density, value => {p.density = value; changed();}, {min: 0, max: 1, step: 0.01});
        content.append(node('p', 'Sets or clears this exact Boolean channel. Fractional selection uses the recipe seed and this component’s stable ID.', 'seed-hint'));
    } else if (c.kind === 'relation') {
        const options = ['SC · x', 'SC · y', 'SC · z', 'FCC · yz +', 'FCC · yz −', 'FCC · xz +', 'FCC · xz −', 'FCC · xy +', 'FCC · xy −'];
        inputField(content, 'Relation orientation', p.orientation, value => {p.orientation = Number(value); changed();}, {options: options.map((v, i) => [i, v])});
        inputField(content, 'Slot', p.slot, value => {p.slot = Number(value); changed();}, {options: [[0, 'Primary'], [1, 'Reserve']]});
        inputField(content, 'Phase', p.phase, value => {p.phase = Number(value); changed();}, {options: [0, 1, 2, 3].map(v => [v, String(v)])});
        inputField(content, 'Polarity', p.polarity, value => {p.polarity = Number(value); changed();}, {options: [[1, '+1'], [-1, '−1']]});
        inputField(content, 'Occupied', p.occupied, value => {p.occupied = value; changed();}, {type: 'checkbox'});
    } else {
        const values = c.kind === 'manifestation' ? [-1, 0, 1] : [0, 1, 2];
        inputField(content, c.kind === 'manifestation' ? 'Stored ternary state' : 'Collision layer', p.value,
            value => {p.value = Number(value); changed();}, {options: values.map(v => [v, String(v)])});
        if (c.kind === 'manifestation') content.append(node('p', 'The stored readout may intentionally lag the relation incidence.', 'seed-hint'));
    }
    const actions = node('div', undefined, 'seed-actions');
    actions.append(button('Duplicate', duplicate), button('Move up', () => move(-1)), button('Move down', () => move(1)), button('Remove', remove));
    content.append(actions);
    return root;
}
