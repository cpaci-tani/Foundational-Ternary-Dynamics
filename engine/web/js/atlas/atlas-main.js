// FTD Ontology Atlas — bootstrap. Builds the Three.js scene + Moore lattice,
// the field-layer renderers, the SVG chain overlay, and the UI (layer panel,
// detail panel, 14-stage stepper, mode switch), then runs the render loop and
// drives the guided chain.
import { createScene } from './atlas-scene.js';
import { createLattice } from './atlas-lattice.js';
import { createLayers } from './atlas-fields.js';
import { createOverlay } from './atlas-overlay.js';
import { createUI } from './atlas-ui.js';
import { LAYERS, GROUPS } from './atlas-content.js';
import { STAGES, STAGE_COUNT } from './atlas-chain.js';
import * as data from './atlas-data.js';

const canvas = document.getElementById('atlas-canvas');
const scene = createScene(canvas);
const lattice = createLattice(scene.THREE);
scene.scene.add(lattice);

// Field-layer renderers. Each root starts hidden; add them all once.
const layers = createLayers(scene.THREE, scene, data);
for (const L of layers.values()) scene.scene.add(L.root);

// SVG chain overlay over the canvas; ticked each frame after render.
const overlay = createOverlay(document.getElementById('atlas-overlay'), scene);

const reduceMotion = matchMedia('(prefers-reduced-motion: reduce)').matches;

// The lattice shells are always-on context (stage 0's substrate).
const THREE = scene.THREE;

// ── camera tween (spherical around the target) ────────────────────────────
// STAGES[n].camera = {az, el, zoom}; interpret as spherical around (0,0,0):
//   radius = 5.2 / zoom, azimuth az°, elevation el°.
const TARGET = new THREE.Vector3(0, 0, 0);
function sphericalToCart(az, el, zoom) {
  const r = 5.2 / (zoom || 1);
  const a = (az || 0) * Math.PI / 180, e = (el || 0) * Math.PI / 180;
  return new THREE.Vector3(
    r * Math.cos(e) * Math.sin(a),
    r * Math.sin(e),
    r * Math.cos(e) * Math.cos(a),
  );
}
let tween = null;   // { from, to, t0, dur } | null
function tweenCameraTo(cam) {
  const to = sphericalToCart(cam.az, cam.el, cam.zoom);
  if (reduceMotion) {
    scene.camera.position.copy(to);
    scene.controls.target.copy(TARGET);
    scene.controls.update();
    tween = null;
    return;
  }
  tween = { from: scene.camera.position.clone(), to, t0: performance.now(), dur: 600 };
}
function stepTween(now) {
  if (!tween) return;
  const k = Math.min(1, (now - tween.t0) / tween.dur);
  const e = k < 0.5 ? 2 * k * k : 1 - Math.pow(-2 * k + 2, 2) / 2;   // easeInOutQuad
  scene.camera.position.lerpVectors(tween.from, tween.to, e);
  scene.controls.target.copy(TARGET);
  if (k >= 1) tween = null;
}

// Resolve a layer's arrow endpoint: its anchor nudged by a small stable offset
// (derived from the id) so layers sharing one world point don't draw on top of
// each other. Returns a fresh Vector3 (callers may mutate it).
function anchorOf(id) {
  const base = layers.get(id)?.anchor || TARGET;
  let h = 0;
  for (let k = 0; k < id.length; k++) h = (h * 31 + id.charCodeAt(k)) & 0xffff;
  const ang = (h / 0xffff) * Math.PI * 2;
  const rad = 0.14;
  return base.clone().add(new THREE.Vector3(Math.cos(ang) * rad, Math.sin(ang) * rad, 0));
}
// Ensure two endpoints are at least MIN apart (in world space) so the arrow
// has a visible length + direction; pushes them symmetrically along x if not.
function separate(from, to) {
  const MIN = 0.34;
  const d = from.distanceTo(to);
  if (d >= MIN) return;
  const push = (MIN - d) / 2 + 0.02;
  from.x -= push; to.x += push;
}

// ── api (the test handle) ─────────────────────────────────────────────────
let ui = null;   // assigned after createUI; setStage uses it lazily.

const api = {
  stageCount: STAGE_COUNT,
  ready: false,
  _scene: scene,
  _lattice: lattice,
  _stage: 0,
  layers,
  layerIds() { return [...layers.keys()]; },
  toggleLayer(id) {
    const L = layers.get(id);
    if (!L) return false;
    const next = !L.root.visible;
    L.setVisible(next);
    ui?.syncToggles();
    return next;
  },
  setLayerVisible(id, vis) { layers.get(id)?.setVisible(!!vis); ui?.syncToggles(); },
  layerBoundsCenterX(id) {
    const L = layers.get(id);
    if (!L) return NaN;
    return L.bounds().getCenter(new THREE.Vector3()).x;
  },

  // Drive the chain to stage n: visibility (guided), arrows, camera, detail.
  setStage(n) {
    const i = Math.max(0, Math.min(STAGE_COUNT - 1, n | 0));
    api._stage = i;
    const stage = STAGES[i];

    // Guided mode: visibility is dictated by the stage. Free mode: leave the
    // user's toggles alone (stepping only moves camera + overlay + detail).
    if (ui?.getMode?.() !== 'free') {
      const on = new Set(stage.layersOn);
      for (const [id, L] of layers) L.setVisible(on.has(id));
    }

    // Flow-arrows for this stage, coloured by the destination layer's group.
    // Several layers anchor to the same world point (the origin or a charge), so
    // we resolve each endpoint to a small stable per-layer offset off its anchor
    // and, if a pair is still coincident, push the two apart — otherwise the
    // arrow would collapse to an invisible dot.
    overlay.setArrows(stage.arrows.map(([a, b]) => {
      const from = anchorOf(a);
      const to = anchorOf(b);
      separate(from, to);
      const grpId = LAYERS[b]?.group;
      const color = GROUPS.find((g) => g.id === grpId)?.color || '#9aa0ad';
      return [from, to, color];
    }));

    tweenCameraTo(stage.camera);
    ui?.showDetail(stage.contentKey);
    ui?.syncToggles();
    ui?.markActive(i);
  },

  nextStage() { api.setStage((api._stage + 1) % STAGE_COUNT); },
  selected() { return ui?.selected?.() ?? STAGES[api._stage].contentKey; },
  mode() { return ui?.getMode?.() ?? 'guided'; },
  setMode(s) { ui?.setMode?.(s); },

  dispose() { api._running = false; },
  _running: true,
};
window.__ftdAtlas = api;

// ── UI (needs api; setStage needs ui — resolved by lazy `ui` reference) ────
ui = createUI(
  {
    layerPanel: document.getElementById('layer-panel'),
    detailPanel: document.getElementById('detail-panel'),
    stepper: document.getElementById('chain-stepper'),
    modeButtons: [...document.querySelectorAll('#mode-switch button[data-mode]')],
  },
  {
    layers, LAYERS, GROUPS, STAGES, scene, overlay, api,
    // Any direct user interaction (toggle, node click, mode switch) flips to
    // free-form: pause the autoplay so the chain doesn't fight the user.
    onUserInteract: () => ui?.pause?.(),
  },
);

document.getElementById('reset-view')?.addEventListener('click', () => {
  ui?.pause?.();
  tweenCameraTo(STAGES[api._stage].camera);
});
window.addEventListener('resize', () => scene.onResize());

function tickLayers() {
  if (reduceMotion) return;
  const t = performance.now() / 1000;
  for (const L of layers.values()) {
    if (L.root.visible) L.update(t);
  }
}

function frame() {
  if (!api._running) return;
  stepTween(performance.now());
  tickLayers();
  scene.render();
  overlay.tick();
  requestAnimationFrame(frame);
}

// First frame, set the opening stage, then flip ready (so tests wait for an
// actual render with the guided chain already on stage 0).
requestAnimationFrame(() => {
  api.setStage(0);
  stepTween(performance.now());
  tickLayers();
  scene.render();
  overlay.tick();
  api.ready = true;
  requestAnimationFrame(frame);
});

console.info('[atlas] boot');
