import { getViewportFrameTemplate } from './template.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export class ViewportFrameComponent {
    constructor(root) {
        this.root = root;
    }

    init() {
        if (!this.root) return this;
        if (!this.root.querySelector('#viewport-frame-chrome')) {
            this.root.appendChild(htmlToElement(getViewportFrameTemplate()));
        }
        this.root.dataset.viewportFrame = 'true';
        return this;
    }
}
