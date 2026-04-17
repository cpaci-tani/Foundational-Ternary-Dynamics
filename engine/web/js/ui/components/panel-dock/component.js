import { getPanelDockShellTemplate } from './template.js';

function htmlToFragment(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content;
}

export class PanelDockComponent {
    constructor(root) {
        this.root = root;
        this.activeTitle = null;
    }

    init() {
        if (!this.root || this.root.querySelector('.panel-dock-body')) return this;

        const existingChildren = Array.from(this.root.children);
        // Extract the resize handle so it sits at the top edge of panel-area,
        // above the dock head — rather than inside the scrolling body.
        const resizer = existingChildren.find((el) => el.id === 'panel-resizer') || null;

        this.root.innerHTML = '';
        if (resizer) this.root.appendChild(resizer);
        this.root.appendChild(htmlToFragment(getPanelDockShellTemplate()));

        const body = this.root.querySelector('[data-panel-dock-body]');
        existingChildren.forEach((child) => {
            if (child === resizer) return;
            body.appendChild(child);
        });
        this.activeTitle = this.root.querySelector('#panel-dock-active-title');
        this.root.dataset.panelDock = 'true';
        return this;
    }

    setActiveTitle(label) {
        if (this.activeTitle) this.activeTitle.textContent = label || 'Controls';
    }
}
