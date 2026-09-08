import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';

// Independent review: execute the actual shared renderer methods, using
// counted resources instead of a WebGL context. This is no FPS/browser claim.
function load(path, symbol, globals = {}) {
    const source = readFileSync(new URL('../js/' + path, import.meta.url), 'utf8')
        .replace(/^import[\s\S]*?from\s+['"][^'"]+['"];\s*/gm, '')
        .replace(/export\s+/g, '');
    return vm.runInNewContext(source + '\n' + symbol, globals);
}
const Core = load('viewport/scene-core.js', 'ViewportSceneCore', { THREE: { Color: class {} } });
const Viewport = load('viewport.js', 'Viewport');
function resource() { return { calls: 0, dispose() { this.calls++; } }; }
function group(children) { return { traverse(fn) { fn(this); for (const child of children) fn(child); } }; }

test('boundary teardown releases shared materials, maps and geometry once', () => {
    const geometry = resource(), texture = resource(), material = { ...resource(), map: texture };
    const boundary = group([{ geometry, material }, { geometry, material: [material] }, { material }]);
    const core = Object.create(Core.prototype);
    core._scene = { remove() {} };
    core.wireframe = boundary;
    core._disposeBoundary(); core._disposeBoundary();
    assert.equal(geometry.calls, 1); assert.equal(material.calls, 1); assert.equal(texture.calls, 1);
    assert.equal(core.wireframe, null);
});

test('scene decoration does not release borrowed global ArrowHelper geometry', () => {
    const geometry = resource(), material = resource();
    const core = Object.create(Core.prototype); core._scene = { remove() {} };
    core._disposeDecorationGroup(group([
        { parent: { type: 'ArrowHelper' }, geometry, material },
        { parent: { type: 'ArrowHelper' }, geometry, material },
    ]));
    assert.equal(geometry.calls, 0); assert.equal(material.calls, 1);
});

test('scene teardown releases bloom including high-pass and composer exactly once', () => {
    const core = Object.create(Core.prototype); core._scene = { remove() {} };
    const highpass = resource(), bloom = { ...resource(), materialHighPassFilter: highpass };
    const composer = resource(), hover = resource();
    Object.assign(core, { _bloomPass: bloom, _composer: composer, _globalClockHover: hover });
    core.dispose(); core.dispose();
    for (const r of [highpass, bloom, composer, hover]) assert.equal(r.calls, 1);
    assert.equal(core._bloomPass, null); assert.equal(core._composer, null);
});

test('viewport teardown releases controls, renderer, canvas and every owned delegate once', () => {
    const viewport = Object.create(Viewport.prototype), delegates = [];
    for (const key of ['_fluxRenderer', '_particleRenderer', '_molRenderer', '_topoRenderer', 'spinArrowManager', '_fieldRenderer', '_sceneCore']) {
        const r = resource(); delegates.push(r); viewport[key] = r;
    }
    const controls = resource(); let detached = 0, disconnected = 0;
    const renderer = { ...resource(), domElement: { remove() { detached++; } } };
    Object.assign(viewport, { controls, renderer, scene: { remove() {} },
        _resizeObserver: { disconnect() { disconnected++; } } });
    viewport.dispose(); viewport.dispose();
    for (const r of [...delegates, controls, renderer]) assert.equal(r.calls, 1);
    assert.equal(detached, 1); assert.equal(disconnected, 1);
});
