/**
 * RenderChipComponent — floating progress indicator for an active render.
 *
 * Usage:
 *   const chip = new RenderChipComponent(viewportEl, {
 *       onCancel: () => renderController.cancel(),
 *   }).mount();
 *   chip.bindController(renderController);
 */

export class RenderChipComponent {
    constructor(viewportEl, { onCancel }) {
        this.viewportEl = viewportEl;
        this.onCancel = onCancel;
        this.el = document.createElement('div');
        this.el.id = 'render-chip';
        this.el.className = 'render-chip';
        this.el.hidden = true;
        this.el.innerHTML = `
            <span class="render-chip-label">Rendering…</span>
            <div class="render-chip-bar"><div class="render-chip-fill"></div></div>
            <span class="render-chip-pct">0%</span>
            <button class="render-chip-cancel" type="button" aria-label="Cancel render">&times;</button>
        `;
        this.labelEl = this.el.querySelector('.render-chip-label');
        this.fillEl  = this.el.querySelector('.render-chip-fill');
        this.pctEl   = this.el.querySelector('.render-chip-pct');
        this.el.querySelector('.render-chip-cancel').addEventListener('click', () => this.onCancel?.());
    }

    mount() {
        if (!this.viewportEl || this.el.parentElement) return this;
        this.viewportEl.appendChild(this.el);
        return this;
    }

    setVisible(v) { this.el.hidden = !v; }

    setLabel(text) { this.labelEl.textContent = text; }

    setProgress(frac) {
        const pct = Math.round(frac * 100);
        this.fillEl.style.width = `${pct}%`;
        this.pctEl.textContent  = `${pct}%`;
    }

    bindController(controller) {
        controller.addEventListener('start', (e) => {
            const secs = (e.detail.totalTicks / 60).toFixed(0);
            this.setLabel(`Rendering ${secs}s…`);
            this.setProgress(0);
            this.setVisible(true);
        });
        controller.addEventListener('progress', (e) => this.setProgress(e.detail.progress));
        controller.addEventListener('done', () => {
            this.setLabel('Render complete');
            this.setProgress(1);
            setTimeout(() => this.setVisible(false), 1200);
        });
        controller.addEventListener('cancel', () => this.setVisible(false));
        controller.addEventListener('error', () => {
            this.setLabel('Render failed');
            setTimeout(() => this.setVisible(false), 2000);
        });
    }
}
