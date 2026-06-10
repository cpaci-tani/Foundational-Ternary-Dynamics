// FTD Ontology Atlas — bootstrap. Builds the Three.js scene + Moore lattice
// stage and starts the render loop. Field renderers, chain stepper, SVG overlay,
// and UI are wired in later phases.
import { createScene } from './atlas-scene.js';
import { createLattice } from './atlas-lattice.js';

const canvas = document.getElementById('atlas-canvas');
const scene = createScene(canvas);
const lattice = createLattice(scene.THREE);
scene.scene.add(lattice);

const api = {
  stageCount: 14,
  ready: false,
  _scene: scene,
  _lattice: lattice,
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

function frame() {
  if (!api._running) return;
  scene.render();
  requestAnimationFrame(frame);
}
// First frame, then flip ready (so tests wait for an actual render).
requestAnimationFrame(() => {
  scene.render();
  api.ready = true;
  requestAnimationFrame(frame);
});

console.log('[atlas] boot');
