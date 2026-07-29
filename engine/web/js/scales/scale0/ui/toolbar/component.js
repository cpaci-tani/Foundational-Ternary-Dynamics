import { populateScale0ScenarioSelect } from '../../scenario-registry.js';
import { getScale0ScenarioToolbarTemplate, getScale0LatticeSizeToolbarTemplate } from './template.js';
import { scaleUpToParticles } from '../../../scale1/promotion.js';

function htmlToElement(markup) {
    const template = document.createElement('template');
    template.innerHTML = markup.trim();
    return template.content.firstElementChild;
}

export function createScale0ScenarioToolbarGroup() {
    const element = htmlToElement(getScale0ScenarioToolbarTemplate());
    populateScale0ScenarioSelect(element.querySelector('#scenario-select'), 'flux-pulse');
    const scaleUpBtn = element.querySelector('#btn-scale-up');
    if (scaleUpBtn) {
        scaleUpBtn.addEventListener('click', async () => {
            scaleUpBtn.disabled = true;
            try {
                await scaleUpToParticles(window.__ftdCtx, {
                    notify: (msg) => {
                        if (typeof window.showToast === 'function') window.showToast(msg, 'info');
                        else console.info('[ScaleUp]', msg);
                    },
                });
            } finally {
                scaleUpBtn.disabled = false;
            }
        });
    }
    return element;
}

export function createScale0LatticeSizeToolbarGroup() {
    return htmlToElement(getScale0LatticeSizeToolbarTemplate());
}
