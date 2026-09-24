import test from 'node:test';
import assert from 'node:assert/strict';
import { LifetimeScope } from '../js/ui/utils/lifetime-scope.js';
import { attachInstrumentPanelCollapse } from '../js/ui/utils/instrument-panel.js';
import { computeGenesisPlotLayout } from '../js/scales/scale0/ui/overlays/genesis-burst-panel.js';

class FakeClassList {
    constructor() { this.values = new Set(); }
    toggle(name, force) {
        if (force) this.values.add(name);
        else this.values.delete(name);
    }
    remove(name) { this.values.delete(name); }
    contains(name) { return this.values.has(name); }
}

class FakeElement extends EventTarget {
    constructor(tagName = 'div') {
        super();
        this.tagName = tagName.toUpperCase();
        this.children = [];
        this.dataset = {};
        this.classList = new FakeClassList();
        this.attributes = new Map();
        this.parentNode = null;
        this.removed = false;
    }
    appendChild(child) { return this.append(child), child; }
    append(...children) {
        for (const child of children) {
            child.parentNode = this;
            this.children.push(child);
        }
    }
    prepend(child) {
        child.parentNode = this;
        this.children.unshift(child);
    }
    setAttribute(name, value) { this.attributes.set(name, String(value)); }
    getAttribute(name) { return this.attributes.get(name) ?? null; }
    remove() {
        this.removed = true;
        if (this.parentNode) {
            this.parentNode.children = this.parentNode.children.filter((child) => child !== this);
            this.parentNode = null;
        }
    }
}

test('instrument collapse binding is reusable and its click listener is lifetime-owned', () => {
    const previousDocument = globalThis.document;
    globalThis.document = { createElement: (tagName) => new FakeElement(tagName) };
    try {
        const element = new FakeElement('section');
        const content = new FakeElement('div');
        element.append(content);
        const lifetime = new LifetimeScope();
        const first = attachInstrumentPanelCollapse({ element, lifetime, label: 'Probe' });
        const second = attachInstrumentPanelCollapse({ element, lifetime, label: 'Probe' });

        assert.equal(second, first);
        assert.equal(element.children.filter((child) => child === first.chrome).length, 1);
        assert.equal(first.button.getAttribute('aria-expanded'), 'true');

        first.button.dispatchEvent(new Event('click'));
        assert.equal(first.collapsed, true);
        assert.equal(element.classList.contains('instrument-panel-collapsed'), true);
        assert.equal(first.button.getAttribute('aria-expanded'), 'false');

        first.button.dispatchEvent(new Event('click'));
        assert.equal(first.collapsed, false);
        assert.equal(element.classList.contains('instrument-panel-collapsed'), false);

        lifetime.dispose();
        assert.equal(first.disposed, true);
        assert.equal(element.children.includes(first.chrome), false);
        first.button.dispatchEvent(new Event('click'));
        assert.equal(first.collapsed, false);
    } finally {
        globalThis.document = previousDocument;
    }
});

test('Genesis plot reserves measured space between the 90 tick and A axis label', () => {
    for (const width of [326, 603, 1376]) {
        const layout = computeGenesisPlotLayout(width, 200, (text) => String(text).length * 8);
        const lastTick = layout.ticks.at(-1);
        assert.ok(lastTick.left >= 0);
        assert.ok(lastTick.right <= width);
        assert.ok(layout.axisLabel.right <= width);
        assert.ok(layout.axisLabel.left - lastTick.right >= layout.labelGap);
        assert.ok(layout.labelBaseline < 200);
    }
});
