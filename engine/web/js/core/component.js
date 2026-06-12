/**
 * @file engine/web/js/core/component.js
 * @purpose Base component class utilizing browser-native template elements and reference binding.
 */

export class BaseComponent {
    constructor(templateHtml) {
        const template = document.createElement('template');
        template.innerHTML = templateHtml.trim();
        this.element = template.content.cloneNode(true).firstElementChild;
        this.refs = {};
        this._bindRefs(this.element);
    }

    _bindRefs(root) {
        if (root.hasAttribute('ref')) {
            this.refs[root.getAttribute('ref')] = root;
        }
        for (const el of root.querySelectorAll('[ref]')) {
            this.refs[el.getAttribute('ref')] = el;
        }
    }

    mount(parent) {
        parent.appendChild(this.element);
        this.onMount?.();
    }

    unmount() {
        this.element.remove();
        this.onUnmount?.();
    }
}
