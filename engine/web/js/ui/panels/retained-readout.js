// Read-only instrument markup has one owner. Retain its elements so changing
// numbers/paths does not replace whole live subtrees and force their layout.
// Supported roots are sole-owner HTML div readouts, with HTML/SVG descendants.
// Template parsing is not a general contextual-fragment parser for table/SVG
// roots. This helper is not for independently edited markup or form controls.
const readouts = new WeakMap();

function reconcile(parent, desired) {
    let current = parent.firstChild;
    for (const next of desired.childNodes) {
        if (!current) {
            parent.appendChild(next.cloneNode(true));
            continue;
        }
        const following = current.nextSibling;
        if (current.nodeType !== next.nodeType
            || (current.nodeType === 1 && (current.namespaceURI !== next.namespaceURI
                || current.localName !== next.localName))) {
            parent.replaceChild(next.cloneNode(true), current);
        } else if (current.nodeType === 1) {
            for (let i = current.attributes.length - 1; i >= 0; --i) {
                const attr = current.attributes[i];
                if (!next.hasAttributeNS(attr.namespaceURI, attr.localName)) {
                    current.removeAttributeNS(attr.namespaceURI, attr.localName);
                }
            }
            for (const attr of next.attributes) {
                if (current.getAttributeNS(attr.namespaceURI, attr.localName) !== attr.value) {
                    current.setAttributeNS(attr.namespaceURI, attr.name, attr.value);
                }
            }
            reconcile(current, next);
        } else if (current.nodeValue !== next.nodeValue) {
            current.nodeValue = next.nodeValue;
        }
        current = following;
    }
    while (current) {
        const following = current.nextSibling;
        parent.removeChild(current);
        current = following;
    }
}

export function updateRetainedReadout(container, markup) {
    const html = String(markup);
    let state = readouts.get(container);
    if (state?.html === html) return false;
    if (!state) {
        state = { html: null, template: container.ownerDocument.createElement('template') };
        container.innerHTML = html;
        readouts.set(container, state);
    } else {
        state.template.innerHTML = html;
        reconcile(container, state.template.content);
    }
    state.html = html;
    return true;
}
