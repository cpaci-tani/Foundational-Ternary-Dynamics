/**
 * Scale 0 Visualization panel: markup.
 *
 * The panel is a layer inspector built from one table, layer-catalog.js:
 * a header, a filter, the strip of layers that are on, the Rendering rows,
 * then one collapsible group per quantity. Every layer is always listed. A
 * layer the current scenario cannot draw is dimmed and cannot be switched on
 * (overlays/applicability.js).
 *
 * Behaviour lives elsewhere and keys on the ids and classes written here:
 *   overlays/panel-shell.js  group collapse, the active strip, the filter,
 *                            and which Rendering rows are shown
 *   ui/bindings.js           what each switch does
 *
 * A layer's own controls (sub-switches, slice height, the transport
 * threshold) sit directly under it and are shown only while it is on.
 */

import { LAYER_GROUPS, layerTooltip } from './layer-catalog.js';

function el(tag, attrs = {}, children = []) {
    const node = document.createElement(tag);
    for (const [name, value] of Object.entries(attrs)) {
        if (value === false || value == null) continue;
        if (name === 'class') node.className = value;
        else if (name === 'text') node.textContent = value;
        else if (name === 'hidden') node.hidden = !!value;
        else node.setAttribute(name, value === true ? '' : String(value));
    }
    for (const child of [].concat(children)) {
        if (child) node.append(child);
    }
    return node;
}

function header() {
    return el('header', { class: 's0-overlay-header' }, [
        el('div', { class: 's0-overlay-identity' }, [
            el('span', { class: 's0-overlay-mark', 'aria-hidden': 'true' }, [el('span'), el('span'), el('span')]),
            el('span', { class: 's0-overlay-title', text: 'Visualization' }),
        ]),
        el('div', { class: 's0-overlay-header-tools' }, [
            el('span', { class: 's0-overlay-summary', id: 's0-overlay-summary', 'aria-live': 'polite' }, [
                el('span', { class: 's0-overlay-summary-count', text: '0' }),
                el('span', { class: 's0-overlay-summary-unit', text: ' active' }),
            ]),
            el('button', {
                class: 's0-overlay-collapse u-no-baseline', type: 'button',
                'aria-label': 'Collapse visualization overlay', 'aria-expanded': 'true', title: 'Collapse overlay',
            }, el('span', { class: 's0-overlay-collapse-icon', 'aria-hidden': 'true' })),
        ]),
    ]);
}

function search() {
    return el('div', { class: 's0-overlay-command' }, el('label', { class: 's0-overlay-search', for: 's0-overlay-search' }, [
        el('span', { class: 's0-overlay-search-icon', 'aria-hidden': 'true' }),
        el('input', {
            type: 'search', id: 's0-overlay-search', class: 's0-overlay-search-input',
            placeholder: 'Find a layer', autocomplete: 'off', spellcheck: 'false',
            'aria-label': 'Filter visualization layers',
        }),
        el('button', {
            class: 's0-overlay-search-clear u-no-baseline', id: 's0-overlay-search-clear', type: 'button',
            'aria-label': 'Clear layer filter', title: 'Clear filter', hidden: true,
        }),
    ]));
}

function styleButton(label, dataset, active, title) {
    return el('button', {
        class: active ? 'style-btn active' : 'style-btn', type: 'button',
        'aria-pressed': active ? 'true' : 'false', title, ...dataset,
    }, document.createTextNode(label));
}

/** Rendering rows. Each is shown only while a layer it applies to is on. */
function renderDeck() {
    return el('section', { class: 's0-overlay-render-deck', 'aria-label': 'Rendering', hidden: true }, [
        el('div', { class: 's0-overlay-render-row', 'data-render-row': 'scalar', hidden: true }, [
            el('span', { class: 's0-overlay-render-label', text: 'Scalar' }),
            el('div', {
                class: 's0-overlay-render-choices', id: 'scalar-render-row', role: 'group',
                'aria-label': 'Scalar layer rendering',
            }, [
                styleButton('Surface', { 'data-scalar-mode': 'default' }, true, 'Each scalar layer as its own sheet or point cloud.'),
                styleButton('Heat map', { 'data-scalar-mode': 'heatmap' }, false, 'Each scalar layer as glowing points through the volume.'),
                styleButton('Volume', { 'data-scalar-mode': 'volume' }, false, 'Each scalar layer as a smooth volume. The interpolation is for display only.'),
            ]),
        ]),
        el('label', { class: 's0-overlay-render-opacity', id: 'scalar-volume-controls', for: 'scalar-volume-opacity', hidden: true }, [
            document.createTextNode('Opacity'),
            el('input', { id: 'scalar-volume-opacity', type: 'range', min: '0', max: '1', step: '0.05', value: '0.75' }),
            el('output', { id: 'scalar-volume-opacity-value', for: 'scalar-volume-opacity', text: '75%' }),
        ]),
        el('div', { class: 's0-overlay-render-row', 'data-render-row': 'vector', hidden: true }, [
            el('span', { class: 's0-overlay-render-label', text: 'Vector' }),
            el('div', {
                class: 's0-overlay-render-choices', id: 'force-style-row', role: 'group',
                'aria-label': 'Vector layer rendering',
            }, [
                styleButton('Arrows', { 'data-style': 'arrows' }, true, 'Each vector layer as arrows.'),
                styleButton('Heat', { 'data-style': 'heatmap' }, false, 'Each vector layer as a heat map of its magnitude.'),
                styleButton('Glyphs', { 'data-style': 'glyphs' }, false, 'Each vector layer as oriented glyphs.'),
            ]),
        ]),
    ]);
}

function toggle(layer, extraClass = 'field-toggle') {
    const content = el('span', { class: 's0-toggle-content' }, [
        layer.swatch ? el('span', { class: `field-swatch field-swatch-${layer.swatch}` }) : null,
        document.createTextNode(layer.label),
    ]);
    return el('button', {
        class: `view-toggle ${extraClass}${layer.active ? ' active' : ''}`,
        id: layer.id, type: 'button',
        'aria-pressed': layer.active ? 'true' : 'false',
        title: layer.equation === undefined ? layer.detail : layerTooltip(layer),
        'data-search': layer.search || null,
    }, content);
}

function subRow(sub) {
    return el('div', { class: 's0-sub-row', role: 'group', 'aria-label': sub.aria },
        sub.items.map((item) => toggle(item, 's0-sub-toggle')));
}

function sheetRow(layer) {
    const { key, sliderId, value, aria } = layer.sheet;
    const shown = value.toFixed(2);
    return el('div', { class: 's0-sheet-height-row', 'data-sheet-height': key }, [
        el('span', { class: 's0-sheet-height-cap', title: 'Slice height: where along y the sheet samples the field.', text: 'y' }),
        el('input', {
            type: 'range', class: 'pe-slider s0-sheet-height-slider', id: sliderId,
            min: '0', max: '0.999', step: '0.01', value: shown, 'aria-label': aria,
        }),
        el('span', { class: 'pe-ctrl-value s0-sheet-height-val', id: `${sliderId}-val`, text: shown }),
    ]);
}

function nativeTransportPanel() {
    return el('div', { id: 'native-transport-panel', class: 'native-transport-legend', hidden: true }, [
        el('div', { id: 'native-transport-legend' }),
        el('label', { class: 'native-transport-threshold' }, [
            document.createTextNode('Draw links above '),
            el('input', {
                type: 'range', id: 'native-transport-threshold', min: '1', max: '50', step: '1', value: '5',
                title: 'Links whose current is below this share of the frame maximum are not drawn. At most the 20,000 strongest are drawn.',
            }),
            document.createTextNode(' % of the frame maximum'),
        ]),
    ]);
}

function standardModelCard() {
    const field = (name, text = '—') => el('b', { 'data-sm-field': name, text });
    const pair = (label, name) => el('span', {}, [document.createTextNode(`${label} `), field(name)]);
    return el('div', { class: 's0-sm-context-card', id: 's0-sm-context-card' }, [
        el('div', { class: 's0-sm-context-head' }, el('span', {}, [
            el('strong', { 'data-sm-field': 'symbol', text: '—' }),
            document.createTextNode(' '),
            el('span', { 'data-sm-field': 'name', text: 'No particle in this scenario' }),
        ])),
        el('div', { class: 's0-sm-context-values' }, [
            pair('spin', 'spin'), pair('Q', 'charge'), pair('chirality', 'chirality'),
            pair('gen', 'generation'), pair('color', 'color'),
        ]),
    ]);
}

/** One layer with whatever sits under it. A layer with its own controls is a group. */
function layerNodes(layer) {
    const button = toggle(layer);
    const under = [
        layer.sub ? subRow(layer.sub) : null,
        layer.sheet ? sheetRow(layer) : null,
    ].filter(Boolean);
    const nodes = under.length ? [el('div', { class: 's0-overlay-group' }, [button, ...under])] : [button];
    if (layer.nativeTransport) nodes.push(nativeTransportPanel());
    if (layer.standardModel) nodes.unshift(standardModelCard());
    return nodes;
}

function group(spec) {
    return el('div', { class: 's0-overlay-col', 'data-col': spec.id }, [
        el('div', { class: 's0-overlay-col-head' }, [
            el('span', { class: 's0-overlay-col-label', text: spec.label }),
            el('span', { class: 's0-overlay-col-count', 'data-count-for': spec.id, 'aria-hidden': 'true', text: '0' }),
            el('button', {
                class: 's0-overlay-col-clear u-no-baseline', 'data-clear-col': spec.id, type: 'button',
                title: `Turn off every layer in ${spec.label}`, 'aria-label': `Turn off every layer in ${spec.label}`,
                text: '✕',
            }),
        ]),
        ...spec.layers.flatMap(layerNodes),
    ]);
}

export function getScale0OverlayTemplate() {
    return el('div', { id: 'viewport-overlay', class: 'scale0-only s0-overlay-panel' }, [
        header(),
        search(),
        el('div', { class: 's0-overlay-active', id: 's0-overlay-active', 'aria-label': 'Layers that are on', hidden: true }),
        renderDeck(),
        el('div', { class: 's0-overlay-body' }, LAYER_GROUPS.map(group)),
    ]);
}
