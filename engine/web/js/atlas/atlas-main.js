// FTD Ontology Atlas — bootstrap. Builds the Three.js scene + Moore lattice
// stage and starts the render loop. Field renderers, chain stepper, SVG overlay,
// and UI are wired in later phases.
import { createScene } from './atlas-scene.js';
import { createLattice } from './atlas-lattice.js';
import { createLayers } from './atlas-fields.js';
import * as data from './atlas-data.js';

const canvas = document.getElementById('atlas-canvas');
const scene = createScene(canvas);
const lattice = createLattice(scene.THREE);
scene.scene.add(lattice);

// Field-layer renderers. Each root starts hidden; add them all once.
const layers = createLayers(scene.THREE, scene, data);
for (const L of layers.values()) scene.scene.add(L.root);

// Default-visible layers (lattice octa/cubo are already visible).
for (const id of ['J', 's', 'divJ']) layers.get(id)?.setVisible(true);

const reduceMotion = matchMedia('(prefers-reduced-motion: reduce)').matches;

const api = {
  stageCount: 14,
  ready: false,
  _scene: scene,
  _lattice: lattice,
  layers,
  layerIds() { return [...layers.keys()]; },
  toggleLayer(id) {
    const L = layers.get(id);
    if (!L) return false;
    const next = !L.root.visible;
    L.setVisible(next);
    return next;
  },
  setLayerVisible(id, vis) { layers.get(id)?.setVisible(!!vis); },
  layerBoundsCenterX(id) {
    const L = layers.get(id);
    if (!L) return NaN;
    return L.bounds().getCenter(new scene.THREE.Vector3()).x;
  },
  dispose() { api._running = false; },
  _running: true,
};
window.__ftdAtlas = api;

const home = scene.camera.position.clone();
document.getElementById('reset-view')?.addEventListener('click', () => {
  scene.camera.position.copy(home);
  scene.controls.target.set(0, 0, 0);
  scene.controls.update();
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
  tickLayers();
  scene.render();
  requestAnimationFrame(frame);
}
// First frame, then flip ready (so tests wait for an actual render).
requestAnimationFrame(() => {
  tickLayers();
  scene.render();
  api.ready = true;
  requestAnimationFrame(frame);
});

console.log('[atlas] boot');
