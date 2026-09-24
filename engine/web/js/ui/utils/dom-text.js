/** Update a scalar readout without replacing its retained text node. */
export function setTextIfChanged(element, value) {
    if (!element) return false;
    const text = String(value ?? '');
    if (element.textContent === text) return false;
    const node = element.childNodes.length === 1 && element.firstChild?.nodeType === 3
        ? element.firstChild : null;
    if (node) node.nodeValue = text;
    else element.textContent = text;
    return true;
}
