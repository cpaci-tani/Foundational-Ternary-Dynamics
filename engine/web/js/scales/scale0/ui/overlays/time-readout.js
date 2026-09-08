import { updateRetainedReadout } from '../../../../ui/panels/retained-readout.js';

// Private Time A/B/C/E/F view. Only this module writes these readout roots;
// Card D's editable controls stay outside it. A stable template is parsed and
// traversed once, then its numeric/text and SVG-attribute slots are retained.
// Values are plain text/attribute values, never HTML fragments or entities.
// Slot markers belong only to authored templates, never to incoming values.
const views = new WeakMap();
const marker = /__FTD_TIME_SLOT_(\d+)__/g;

function partsOf(value) {
    marker.lastIndex = 0;
    const parts = [];
    let match, start = 0;
    while ((match = marker.exec(value))) {
        parts.push(value.slice(start, match.index), Number(match[1]));
        start = marker.lastIndex;
    }
    if (!parts.length) return null;
    parts.push(value.slice(start));
    return parts;
}

function bindSlots(container, template) {
    const bindings = [];
    // SHOW_ELEMENT | SHOW_TEXT, including SVG descendants. No global DOM is
    // required: a panel's owner document supplies the traversal and parser.
    const walker = container.ownerDocument.createTreeWalker(container, 5);
    const patternWalker = container.ownerDocument.createTreeWalker(template.content, 5);
    for (let pattern = patternWalker.nextNode(); pattern; pattern = patternWalker.nextNode()) {
        const node = walker.nextNode();
        if (pattern.nodeType === 3) {
            const parts = partsOf(pattern.nodeValue);
            if (parts) bindings.push({ node, parts, attribute: null });
        } else {
            for (const attr of pattern.attributes) {
                const parts = partsOf(attr.value);
                if (parts) bindings.push({ node, parts, attribute: {
                    namespace: attr.namespaceURI, name: attr.name, localName: attr.localName,
                } });
            }
        }
    }
    return bindings;
}

function applySlots(bindings, values) {
    for (const { node, parts, attribute } of bindings) {
        let value = parts[0];
        for (let i = 1; i < parts.length; i += 2) value += values[parts[i]] + parts[i + 1];
        if (!attribute) {
            if (node.nodeValue !== value) node.nodeValue = value;
        } else {
            // The tooltip manager moves title into data-ui-tooltip. Preserve
            // that ownership on stable-template refreshes, including changed
            // probe latencies and sampler counts; do not recreate native tips.
            const migrated = attribute.namespace === null && attribute.name === 'title'
                && !node.hasAttribute('title') && node.hasAttribute('data-ui-tooltip');
            const name = migrated ? 'data-ui-tooltip' : attribute.name;
            const localName = migrated ? name : attribute.localName;
            if (node.getAttributeNS(attribute.namespace, localName) !== value) {
                node.setAttributeNS(attribute.namespace, name, value);
            }
        }
    }
}

export function beginTimeReadout(container) {
    const values = [];
    return {
        slot(value) {
            const index = values.length;
            values.push(String(value));
            return `__FTD_TIME_SLOT_${index}__`;
        },
        commit(markup) {
            const template = String(markup);
            let view = views.get(container);
            if (view?.template !== template) {
                // Marker-bearing SVG attributes stay in an inert template;
                // only real numeric/path values enter the live document.
                const pattern = container.ownerDocument.createElement('template');
                pattern.innerHTML = template;
                const html = template.replace(marker, (_, index) => values[Number(index)]
                    .replaceAll('&', '&amp;').replaceAll('<', '&lt;')
                    .replaceAll('>', '&gt;').replaceAll('"', '&quot;'));
                updateRetainedReadout(container, html);
                view = { template, bindings: bindSlots(container, pattern) };
                views.set(container, view);
            }
            applySlots(view.bindings, values);
        },
    };
}

// Unavailable/static states must pass through the same owner so a later
// recovery cannot reuse bindings into a superseded subtree.
export function updateTimeReadout(container, markup) {
    beginTimeReadout(container).commit(markup);
}
