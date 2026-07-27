// @ts-check
/**
 * Fine-structure card: absence of a metrics source must not render as measurement.
 *
 * `getThomsonScatteringMetrics` is defined by no live bridge. The card must
 * therefore never emit a `[M]` (measured — lattice-derived) or `[E]` (emergent
 * — lattice dynamics) badged row from a missing source; the configured-constant
 * `[T]` rows are legitimate and must survive. The sibling `thomson.js` already
 * guards this way (`if (!m || !m.active)`); this pins the same contract here.
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const SCENARIO = 's0-field-thomson-scattering';

/** Render FineStructureComponent against a stub bridge, return its innerHTML. */
async function renderCard(page, bridgeSpec) {
    return page.evaluate(async ({ spec, scenario }) => {
        const { FineStructureComponent } = await import(
            '/js/scales/scale0/ui/overlays/p1-observables/fine-structure.js'
        );
        const bridge = {
            getConstants: () => ({}),
            getToggle: () => false,
            setToggle: () => {},
        };
        if (spec.metrics) bridge.getThomsonScatteringMetrics = () => spec.metrics;
        const card = new FineStructureComponent();
        card.update(bridge, scenario);
        return card.element.innerHTML;
    }, { spec: bridgeSpec, scenario: SCENARIO });
}

const LIVE_METRICS = {
    active: true,
    toggles: {},
    fluxCentroid: { delta: { mag: 1.5e-3, y: 2e-4 } },
    poynting: { mag: 3e-3, x: 1, y: 0.5 },
    energy: { field: 1.25, wave: 2.5 },
    excessResidual: {
        l2: 4e-3, relL2: 1e-2, compX: 1, compY: 2, compZ: 3,
        localCentroid: { x: 1, y: 2, z: 3 },
    },
};

test.beforeEach(async ({ page }) => {
    page.setDefaultTimeout(60_000);
    await gotoAndReady(page);
});

test('no measured or emergent row is rendered when the metrics source is absent', async ({ page }) => {
    const html = await renderCard(page, { metrics: null });

    expect(html, 'no [M] measured badge may be emitted without a metrics source').not.toContain('[M]');
    expect(html, 'no [E] emergent badge may be emitted without a metrics source').not.toContain('[E]');
    expect(html, 'no fabricated zero value may be rendered').not.toContain('0.00e+0');
});

test('the absent metrics source is stated explicitly', async ({ page }) => {
    const html = await renderCard(page, { metrics: null });
    expect(html.toLowerCase()).toContain('waiting for field buffers');
});

test('configured-constant rows survive an absent metrics source', async ({ page }) => {
    const html = await renderCard(page, { metrics: null });

    expect(html, 'configured constants are theory rows and stay visible').toContain('[T]');
    for (const label of ['α = G_C²', '1/α', 'G_C', 'damping γ']) {
        expect(html, `${label} is a configured constant, not a measurement`).toContain(label);
    }
});

test('measured and emergent rows do render when the metrics source is live', async ({ page }) => {
    const html = await renderCard(page, { metrics: LIVE_METRICS });

    expect(html, 'a live source must still produce measured rows').toContain('[M]');
    expect(html, 'a live source must still produce emergent rows').toContain('[E]');
    expect(html).toContain('live residual');
});
