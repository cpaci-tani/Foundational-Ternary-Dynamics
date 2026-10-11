import { createLatticeLogo } from '../lattice-logo.js';
import { getLoadingOverlayTemplate } from './template.js';

export class LoadingOverlayComponent {
    constructor(root) {
        this.root = root;
        this._started = false;
    }

    init() {
        if (!this.root) return this;
        if (!this.root.querySelector('#load-cube')) {
            this.root.innerHTML = getLoadingOverlayTemplate();
        }
        if (!this._started) {
            this._started = true;
            this._startAnimation();
        }
        return this;
    }

    _startAnimation() {
        const cv = this.root.querySelector('#load-cube');
        if (!cv) return;
        const logo = createLatticeLogo(cv);
        const frame = () => {
            if (this.root.classList.contains('hidden')) return;
            window.requestAnimationFrame(frame);
            logo.step();
        };
        window.requestAnimationFrame(frame);
    }
}
