// Three.js scene for the Ontology Atlas: camera, OrbitControls, bloom, and a
// world→screen projector the SVG overlay uses to anchor 2D chain arrows.
import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { EffectComposer } from 'three/addons/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/addons/postprocessing/RenderPass.js';
import { UnrealBloomPass } from 'three/addons/postprocessing/UnrealBloomPass.js';

export function createScene(canvas) {
  const renderer = new THREE.WebGLRenderer({ canvas, antialias: true, alpha: false });
  renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
  renderer.setClearColor(0x0b0c10, 1);

  const scene = new THREE.Scene();
  const camera = new THREE.PerspectiveCamera(50, 1, 0.1, 100);
  camera.position.set(3, 2, 4.4);
  camera.lookAt(0, 0, 0);

  const controls = new OrbitControls(camera, renderer.domElement);
  controls.enableDamping = true;
  controls.dampingFactor = 0.08;
  controls.minDistance = 2;
  controls.maxDistance = 16;
  controls.target.set(0, 0, 0);

  scene.add(new THREE.AmbientLight(0xffffff, 0.55));
  const key = new THREE.DirectionalLight(0xffffff, 0.85);
  key.position.set(5, 8, 6);
  scene.add(key);
  const fill = new THREE.DirectionalLight(0x88aaff, 0.25);
  fill.position.set(-6, -3, -4);
  scene.add(fill);

  let composer = null, bloom = null;
  function buildComposer(w, h) {
    composer = new EffectComposer(renderer);
    composer.addPass(new RenderPass(scene, camera));
    bloom = new UnrealBloomPass(new THREE.Vector2(w, h), 0.6, 0.5, 0.82);
    composer.addPass(bloom);
  }
  function size() {
    return { w: Math.max(1, canvas.clientWidth), h: Math.max(1, canvas.clientHeight) };
  }
  function onResize() {
    const { w, h } = size();
    renderer.setSize(w, h, false);
    camera.aspect = w / h; camera.updateProjectionMatrix();
    if (!composer) buildComposer(w, h); else composer.setSize(w, h);
    if (bloom) bloom.setSize(w, h);
  }
  onResize();

  function render() { controls.update(); composer.render(); }

  const _v = new THREE.Vector3();
  function worldToScreen(vec3) {
    const { w, h } = size();
    _v.copy(vec3).project(camera);
    return { x: (_v.x * 0.5 + 0.5) * w, y: (-_v.y * 0.5 + 0.5) * h, visible: _v.z < 1 };
  }

  return { THREE, scene, camera, controls, renderer, render, onResize, worldToScreen };
}
