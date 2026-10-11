// Zoom-out beyond the Scale 0 lattice: the reference-length table and the ring
// layer that draws it. Every length is an external reference computed from
// the measured masses in constants.js; nothing between the lattice and these
// lengths is simulated, and the layer has to say so.
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import {
    FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M, HBAR_C_MEV_M,
    M_B_PHYS, M_C_PHYS, M_E_PHYS, M_HIGGS_PHYS, M_MU_PHYS, M_T_PHYS, M_TAU_PHYS, M_W_PHYS, M_Z_PHYS,
} from '../js/constants.js';
import {
    LANDMARK_FRAMED_VIEWS, LHC_COLLISION_ENERGY_MEV, NUCLEAR_SCALE_M, OUTERMOST_LANDMARK_M, RING_MIN_PX,
    SCALE_LANDMARKS, framedLandmarkVoxels, getScaleLandmark, metresToVoxels, reducedComptonWavelength,
    visibleLandmarks,
} from '../js/ui/components/live-rulers/scale-landmarks.js';

const VOXEL = FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M;

test('every landmark is hbar c over a measured mass, or a stated reference', () => {
    const masses = {
        top: M_T_PHYS, higgs: M_HIGGS_PHYS, z: M_Z_PHYS, w: M_W_PHYS, bottom: M_B_PHYS,
        tau: M_TAU_PHYS, charm: M_C_PHYS, muon: M_MU_PHYS, electron: M_E_PHYS,
    };
    for (const [id, mass] of Object.entries(masses)) {
        const landmark = getScaleLandmark(id);
        assert.equal(landmark.kind, 'particle');
        assert.equal(landmark.metres, HBAR_C_MEV_M / mass, id);
        assert.equal(landmark.metres, reducedComptonWavelength(mass));
        assert.match(landmark.basis, /reduced Compton wavelength/);
    }
    assert.equal(getScaleLandmark('lhc').metres, HBAR_C_MEV_M / LHC_COLLISION_ENERGY_MEV);
    assert.equal(LHC_COLLISION_ENERGY_MEV, 13.6e6);
    assert.equal(getScaleLandmark('nuclear').metres, NUCLEAR_SCALE_M);
    assert.equal(SCALE_LANDMARKS.length, Object.keys(masses).length + 2);
    assert.equal(getScaleLandmark('up'), null, 'light quarks are left out');

    // Spot values a reader can check by hand (hbar c = 197.327 MeV fm).
    const close = (actual, expected) => assert.ok(Math.abs(actual / expected - 1) < 2e-3, `${actual} vs ${expected}`);
    close(getScaleLandmark('electron').metres, 3.8616e-13);
    close(getScaleLandmark('muon').metres, 1.8676e-15);
    close(getScaleLandmark('w').metres, 2.455e-18);
    close(getScaleLandmark('lhc').metres, 1.451e-20);
    close(metresToVoxels(getScaleLandmark('electron').metres), 2.394e22);
});

test('the table runs outward from the lattice to the electron, in order', () => {
    const lengths = SCALE_LANDMARKS.map((landmark) => landmark.metres);
    assert.deepEqual(lengths, [...lengths].sort((a, b) => a - b));
    assert.equal(new Set(SCALE_LANDMARKS.map((landmark) => landmark.id)).size, SCALE_LANDMARKS.length);
    assert.deepEqual(SCALE_LANDMARKS.map((landmark) => landmark.id),
        ['lhc', 'top', 'higgs', 'z', 'w', 'bottom', 'tau', 'charm', 'nuclear', 'muon', 'electron']);
    assert.equal(OUTERMOST_LANDMARK_M, getScaleLandmark('electron').metres);
    // The largest browser lattice is thirteen powers of ten below the first landmark.
    assert.ok(lengths[0] / (97 * VOXEL) > 1e12);
    assert.ok(lengths.at(-1) < 1e-12);
    assert.ok(Object.isFrozen(SCALE_LANDMARKS) && SCALE_LANDMARKS.every(Object.isFrozen));
});

test('framed views name real landmarks and frame a few times their length', () => {
    assert.deepEqual(Object.keys(LANDMARK_FRAMED_VIEWS), ['lhc', 'electroweak', 'nuclear', 'electron']);
    for (const [id, view] of Object.entries(LANDMARK_FRAMED_VIEWS)) {
        const landmark = getScaleLandmark(view.landmark);
        assert.ok(landmark, id);
        assert.ok(view.across > 1 && view.across <= 5);
        assert.equal(framedLandmarkVoxels(id), (landmark.metres / VOXEL) * view.across);
    }
    assert.equal(framedLandmarkVoxels('lattice'), null);
    // The electroweak view holds all four heavy particles.
    const span = framedLandmarkVoxels('electroweak');
    for (const id of ['top', 'higgs', 'z', 'w']) assert.ok(metresToVoxels(getScaleLandmark(id).metres) < span);
});

test('a ring is drawn only while it is a visible size', () => {
    const width = 1440, height = 900;
    // View about as wide as the W boson's length: the four heavy particles are on screen.
    const perVoxel = width / (metresToVoxels(getScaleLandmark('w').metres) * 2.2);
    const shown = visibleLandmarks(perVoxel, width, height);
    assert.deepEqual(shown.map((entry) => entry.id), ['top', 'higgs', 'z', 'w']);
    for (const entry of shown) {
        assert.equal(entry.diameterPx, metresToVoxels(entry.metres) * perVoxel);
        assert.ok(entry.diameterPx >= RING_MIN_PX);
    }
    // At the lattice, and anywhere in the thirteen empty decades, nothing is drawn.
    assert.deepEqual(visibleLandmarks(10, width, height), []);
    assert.deepEqual(visibleLandmarks(1e-8, width, height), []);
    assert.deepEqual(visibleLandmarks(0, width, height), []);
    assert.deepEqual(visibleLandmarks(NaN, width, height), []);
});

// ── Ring layer ───────────────────────────────────────────────────────────
function fakeDocument(log) {
    const element = (tag) => {
        const style = new Proxy({}, {
            set(target, key, value) { log.push(`style.${String(key)}`); target[key] = value; return true; },
        });
        const attributes = new Map();
        let text = '';
        let hidden = false;
        return {
            tag, style, children: [], className: '', dataset: {}, title: '',
            append(...nodes) { this.children.push(...nodes); },
            setAttribute(name, value) { log.push(`attr.${name}`); attributes.set(name, String(value)); },
            getAttribute(name) { return attributes.get(name) ?? null; },
            get textContent() { return text; },
            set textContent(value) { log.push('text'); text = String(value); },
            get hidden() { return hidden; },
            set hidden(value) { log.push('hidden'); hidden = !!value; },
        };
    };
    return { createElement: element, createElementNS: (_ns, tag) => element(tag) };
}

test('ring layer: honest caption, next reference, rings, labels, and no idle writes', async () => {
    const log = [];
    const previous = globalThis.document;
    globalThis.document = fakeDocument(log);
    try {
        const { createLandmarkRings } = await import('../js/ui/components/live-rulers/landmark-rings.js');
        const layer = createLandmarkRings();
        const [svg, centre, caption] = layer.el.children;
        const labels = layer.el.children.slice(3);
        const rings = svg.children;
        assert.equal(labels.length, SCALE_LANDMARKS.length);
        assert.equal(rings.length, SCALE_LANDMARKS.length);
        assert.equal(layer.el.hidden, true, 'hidden until the view is beyond the lattice');
        assert.match(caption.children[0].textContent, /Nothing is simulated beyond the lattice/);
        assert.match(caption.children[0].textContent, /not FTD results/);
        for (const [index, label] of labels.entries()) {
            const landmark = SCALE_LANDMARKS[index];
            assert.ok(label.textContent.startsWith(`${landmark.label} · `));
            assert.match(label.textContent, /×10⁻\S+ m$/);
            assert.equal(label.title, landmark.basis, 'the tooltip says where the number comes from');
        }

        const width = 1440, height = 900;
        const frameAt = (perVoxel) => ({ x: 720, y: 450, pixelsPerVoxel: perVoxel, viewWidth: width, viewHeight: height, latticeVoxels: 33 });
        const shownRings = () => rings.filter((ring) => ring.getAttribute('visibility') === 'visible').length;
        const shownLabels = () => labels.filter((label) => !label.hidden).map((label) => label.dataset.landmark);

        // In the empty range: no ring, the lattice as a dot, and the next reference named.
        layer.render(frameAt(1e-9));
        assert.equal(layer.el.hidden, false);
        assert.equal(shownRings(), 0);
        assert.equal(centre.hidden, false);
        assert.match(centre.children[1].textContent, /^lattice · 5\.3\d\d×10⁻³⁴ m$/);
        assert.match(caption.children[1].textContent, /^Next reference out: LHC resolution at 1\.451×10⁻²⁰ m\.$/);

        // At the heavy particles: four rings, four labels, none overlapping.
        const perVoxel = width / (metresToVoxels(getScaleLandmark('w').metres) * 2.2);
        layer.render(frameAt(perVoxel));
        assert.equal(shownRings(), 4);
        assert.deepEqual(shownLabels(), ['top', 'higgs', 'z', 'w']);
        assert.match(caption.children[1].textContent, /^Next reference out: bottom quark/);
        const radius = (id) => Number(rings[SCALE_LANDMARKS.findIndex((l) => l.id === id)].getAttribute('r'));
        assert.ok(Math.abs(radius('w') - (metresToVoxels(getScaleLandmark('w').metres) * perVoxel) / 2) < 0.01, 'diameter is the length');
        // Labels read outward in ring order: each larger ring's label is
        // higher on screen than the one before, and clear of it.
        const ys = labels.filter((label) => !label.hidden)
            .map((label) => Number(/translate3d\([^,]+, ([-\d.]+)px/.exec(label.style.transform)[1]));
        for (let i = 1; i < ys.length; i++) assert.ok(ys[i - 1] - ys[i] >= 24, `labels ${ys[i - 1]} and ${ys[i]} are apart, in order`);

        // The same frame again writes nothing.
        log.length = 0;
        layer.render(frameAt(perVoxel));
        layer.render({ ...frameAt(perVoxel) });
        assert.deepEqual(log, []);

        // At the electron there is nothing further out to name.
        layer.render(frameAt(width / (metresToVoxels(getScaleLandmark('electron').metres) * 2.4)));
        assert.deepEqual(shownLabels(), ['electron']);
        assert.equal(caption.children[1].textContent, '');

        // Back at the lattice the layer hides.
        layer.render(null);
        assert.equal(layer.el.hidden, true);
    } finally {
        if (previous === undefined) delete globalThis.document;
        else globalThis.document = previous;
    }
});

test('source contract: the view, the zoom menu and the context ruler share one table', () => {
    const read = (path) => readFileSync(new URL('../js/' + path, import.meta.url), 'utf8');
    const viewport = read('viewport.js');
    assert.match(viewport, /import \{ OUTERMOST_LANDMARK_M, framedLandmarkVoxels, metresToVoxels \} from '\.\/ui\/components\/live-rulers\/scale-landmarks\.js'/);
    assert.match(viewport, /this\.controls\.maxDistance = this\._zoomOutLimit\(\);/);
    const template = read('ui/components/play-bar/template.js');
    for (const id of Object.keys(LANDMARK_FRAMED_VIEWS)) {
        assert.match(template, new RegExp(`data-framed-view="${id}" data-travel`), `zoom menu offers ${id}`);
    }
    // The stops at and inside the lattice travel too, so the way back in is seen.
    for (const id of ['moore', 'neighborhood', 'detail', 'lattice', 'lattice-out', 'quasi']) {
        assert.match(template, new RegExp(`data-framed-view="${id}" data-travel`), `${id} is travelled to`);
    }
    assert.equal(template.match(/data-framed-view="/g).length, template.match(/data-travel/g).length, 'every stop travels');
    assert.match(template, /nothing simulated/);
    const context = read('scales/scale0/ui/overlays/scale-context-panel.js');
    assert.match(context, /SCALE_LANDMARKS\.filter/);
    const mount = read('ui/components/live-rulers/mount.js');
    assert.match(mount, /landmarks\.render\(beyond \?/);
});
