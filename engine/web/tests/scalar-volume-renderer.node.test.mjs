import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { createServer } from 'node:http';
import { once } from 'node:events';
import vm from 'node:vm';
import * as THREE from '../js/vendor/three/build/three.module.js';

const moduleUrl = new URL('../js/viewport/scalar-volume-renderer.js', import.meta.url);
const moduleSource = readFileSync(moduleUrl, 'utf8');
const source = moduleSource.replace(/^import .*?;\s*/gm, '').replace(/export\s+/g, '');
const createScalarVolumeRenderer = vm.runInNewContext(source + '\ncreateScalarVolumeRenderer', { THREE });
const frame = (entries, { stride = 2, origin = 0 } = {}) => ({
    count: entries.length, effectiveStride: stride, origin,
    positions: new Float32Array(entries.flatMap(([x,y,z]) => [x,y,z])),
    values: new Float32Array(entries.map(row => row[3])),
});
const textureOf = renderer => renderer.group.children[0].material.uniforms.uVolume.value;
const decode = texture => Array.from(texture.image.data, n => THREE.DataUtils.fromHalfFloat(n));

test('native sparse anchors preserve x-fast grid indexing and zero-fill absent samples', () => {
    const scene = new THREE.Scene(), volume = createScalarVolumeRenderer(scene, () => 9);
    volume.show('energy', true);
    const sample = frame([[8.5, 8.5, 8.5, 2], [0.5, 0.5, 0.5, 1], [2.5, 4.5, 6.5, 0.5]]);
    assert.equal(volume.update('energy', sample), true);
    const mesh = volume.group.children[0], texture = textureOf(volume), values = decode(texture);
    assert.equal(scene.children[0], volume.group);
    assert.equal(texture.isData3DTexture, true);
    assert.equal(texture.image.width, 5);
    assert.equal(values[0], 0.5); assert.equal(values[124], 1);
    assert.equal(values[1 + 5 * 2 + 25 * 3], 0.25);
    assert.equal(values.filter(n => n !== 0).length, 3);
    assert.deepEqual(mesh.scale.toArray(), [9,9,9]);
    mesh.geometry.computeBoundingBox();
    assert.deepEqual(mesh.geometry.boundingBox.min.toArray(), [0,0,0]);
    assert.deepEqual(mesh.geometry.boundingBox.max.toArray(), [1,1,1]);
    assert.equal(mesh.material.side, THREE.BackSide);
    volume.dispose();
});

test('native centred origin and explicit shorter grids are respected', () => {
    const volume = createScalarVolumeRenderer(new THREE.Scene(), () => 17);
    volume.show('energy', true);
    const sample = frame([[2.5,2.5,2.5,1], [14.5,14.5,14.5,1]], { stride:3, origin:2 });
    assert.equal(volume.update('energy', sample), true);
    assert.equal(textureOf(volume).image.width, 5);
    delete sample.origin;
    assert.equal(volume.update('energy', sample), true); // native origin=floor(16/2)%3=2
    sample.sampleGrid = { stride:3, origin:2, count:4 };
    assert.equal(volume.update('energy', sample), false); // last anchor outside declared support
    volume.dispose();
});

test('malformed or unknown grids clear stale volume rather than inventing support', () => {
    const volume = createScalarVolumeRenderer(new THREE.Scene(), () => 9);
    volume.show('energy', true);
    const good = frame([[4.5,4.5,4.5,1]]);
    const bad = [
        { ...good, effectiveStride:undefined },
        frame([[4.75,4.5,4.5,1]]),
        frame([[10.5,4.5,4.5,1]]),
        frame([[4.5,4.5,4.5,NaN]]),
        frame([[4.5,4.5,4.5,-1]]),
        frame([[4.5,4.5,4.5,1],[4.5,4.5,4.5,1]]),
        { ...good, count:2 },
        { ...good, sampleGrid:{origin:1,stride:2} },
    ];
    for (const invalid of bad) {
        assert.equal(volume.update('energy', good), true);
        assert.equal(volume.group.visible, true);
        assert.equal(volume.update('energy', invalid), false);
        assert.equal(volume.group.visible, false);
        assert.equal(volume.group.userData.status, 'invalid');
    }
    volume.dispose();
    const large = createScalarVolumeRenderer(new THREE.Scene(), () => 65);
    large.show('energy', true);
    assert.equal(large.update('energy', frame([[32.5,32.5,32.5,1]], {stride:1})), false);
    assert.equal(textureOf(large), null);
    large.dispose();
});

test('mutable source changes upload; unchanged values and display options reuse texture', () => {
    const volume = createScalarVolumeRenderer(new THREE.Scene(), () => 9);
    volume.show('energy', true);
    const sample = frame([[0.5,0.5,0.5,2], [8.5,8.5,8.5,1]]);
    volume.update('energy', sample);
    const texture = textureOf(volume), uploads = volume.uploadCount;
    volume.update('energy', { ...sample }, {opacity:0.2,normalizer:4});
    assert.equal(volume.uploadCount, uploads);
    assert.equal(textureOf(volume), texture);
    assert.equal(volume.group.children[0].material.uniforms.uOpacity.value, 0.2);
    sample.values[1] = 0.25;
    volume.update('energy', sample);
    assert.equal(volume.uploadCount, uploads + 1);
    assert.equal(textureOf(volume), texture);
    assert.equal(decode(texture).at(-1), 0.125);
    assert.equal(volume.group.children[0].material.uniforms.uOpacity.value, 0.2);
    const version = texture.version;
    assert.equal(volume.setOpacity(0), true);
    assert.equal(volume.group.visible, false);
    assert.equal(volume.setOpacity(0.6), true);
    assert.equal(volume.group.visible, true);
    assert.equal(volume.setOpacity(NaN), false);
    assert.equal(volume.setOpacity(2), false);
    assert.equal(texture.version, version);
    assert.equal(volume.uploadCount, uploads + 1);
    assert.equal(volume.group.userData.transfer.opacity, 0.6);
    assert.equal(volume.group.userData.transfer.gamma, 0.5);
    assert.equal(volume.group.userData.transfer.visualOnly, true);
    volume.dispose();
});

test('signed transfer, kind selection, resize, empty data and teardown fail closed', () => {
    let L = 9;
    const scene = new THREE.Scene(), unrelated = new THREE.Group(); scene.add(unrelated);
    const volume = createScalarVolumeRenderer(scene, () => L);
    const sample = frame([[0.5,0.5,0.5,-1], [8.5,8.5,8.5,1]]);
    volume.show('hamiltonian', true);
    assert.equal(volume.update('hamiltonian', sample, {signed:true}), true);
    assert.equal(decode(textureOf(volume))[0], -1);
    volume.show('energy', true);
    assert.equal(volume.group.visible, false);
    assert.equal(volume.update('hamiltonian', sample, {signed:true}), false);
    volume.show('hamiltonian', true);
    assert.equal(volume.group.visible, true);
    L = 17; volume.show('hamiltonian', true);
    assert.equal(volume.group.visible, false);
    L = 9; volume.clear(); volume.show('hamiltonian', true);
    assert.equal(volume.group.visible, false);
    assert.equal(volume.update('hamiltonian', frame([])), true);
    assert.equal(volume.group.visible, false);
    const mesh = volume.group.children[0];
    const resources = [textureOf(volume), mesh.geometry, mesh.material, mesh.material.uniforms.uRamp.value];
    const counts = resources.map(resource => { const c = { count:0 }; resource.addEventListener('dispose', () => c.count++); return c; });
    volume.dispose(); volume.dispose();
    assert.ok(counts.every(c => c.count === 1));
    assert.deepEqual(scene.children, [unrelated]);
    volume.show('energy', true); assert.equal(volume.group.visible, false);
});

test('WebGL2 compiles the raymarch shader and renders from outside and inside', {timeout:45000}, async () => {
    const { chromium } = await import('@playwright/test');
    const threeSource = readFileSync(new URL('../js/vendor/three/build/three.module.js', import.meta.url), 'utf8');
    const server = createServer((request, response) => {
        response.setHeader('Content-Type', request.url === '/' ? 'text/html' : 'text/javascript');
        if (request.url === '/three.js') response.end(threeSource);
        else if (request.url === '/volume.js') response.end(moduleSource.replace("from 'three'", "from '/three.js'"));
        else response.end('<!doctype html><html><body></body></html>');
    });
    server.listen(0, '127.0.0.1'); await once(server, 'listening');
    let browser;
    try {
        browser = await chromium.launch({headless:true});
        const page = await browser.newPage();
        const errors = [];
        page.on('pageerror', error => errors.push(error.message));
        page.on('console', message => { if (message.type() === 'error') errors.push(message.text()); });
        await page.goto(`http://127.0.0.1:${server.address().port}/`);
        const rendered = await page.evaluate(async () => {
            const THREE = await import('/three.js');
            const {createScalarVolumeRenderer} = await import('/volume.js');
            const renderer = new THREE.WebGLRenderer({preserveDrawingBuffer:true, antialias:false});
            renderer.setSize(64,64); renderer.setClearColor(0x000000,1);
            document.body.appendChild(renderer.domElement);
            const scene = new THREE.Scene(), volume = createScalarVolumeRenderer(scene, () => 9);
            const p = [], v = [];
            for (let z = 0; z < 9; ++z) for (let y = 0; y < 9; ++y) for (let x = 0; x < 9; ++x) {
                p.push(x+0.5,y+0.5,z+0.5); v.push(1);
            }
            volume.show('energy', true);
            volume.update('energy', {positions:new Float32Array(p),values:new Float32Array(v),count:v.length,effectiveStride:1,origin:0});
            const camera = new THREE.PerspectiveCamera(45,1,0.01,100);
            const pixel = new Uint8Array(4), gl = renderer.getContext();
            const at = (pos,target) => {
                camera.position.set(...pos); camera.lookAt(...target); camera.updateMatrixWorld();
                renderer.render(scene,camera); gl.readPixels(32,32,1,1,gl.RGBA,gl.UNSIGNED_BYTE,pixel);
                return Array.from(pixel);
            };
            const outside = at([4.5,4.5,23],[4.5,4.5,4.5]);
            const inside = at([4.5,4.5,4.5],[4.5,4.5,9]);
            // A uniform interior grid has no observations in the outer shell.
            // Rays through that shell must stay black instead of extending edge values.
            const interiorPositions = [], interiorValues = [];
            for (let z = 2; z <= 6; ++z) for (let y = 2; y <= 6; ++y) for (let x = 2; x <= 6; ++x) {
                interiorPositions.push(x+0.5,y+0.5,z+0.5); interiorValues.push(1);
            }
            volume.update('energy', {positions:new Float32Array(interiorPositions),values:new Float32Array(interiorValues),
                count:interiorValues.length,effectiveStride:1,origin:2,sampleGrid:{stride:1,origin:2,count:5}});
            const interiorObserved = at([4.5,4.5,23],[4.5,4.5,4.5]);
            const unobservedShell = at([0.5,4.5,23],[0.5,4.5,4.5]);
            const faintFrame = {positions:new Float32Array(p),values:new Float32Array(v.length).fill(0.001),
                count:v.length,effectiveStride:1,origin:0,normalizer:1};
            const whiteRamp = (_t,out) => { out[0]=out[1]=out[2]=1; };
            volume.update('energy', faintFrame, {gamma:1,ramp:whiteRamp});
            const rawCutoff = at([4.5,4.5,23],[4.5,4.5,4.5]);
            const uploads = volume.uploadCount;
            volume.update('energy', faintFrame, {gamma:0.5,ramp:whiteRamp});
            const gammaVisible = at([4.5,4.5,23],[4.5,4.5,4.5]);
            const gammaNoUpload = uploads === volume.uploadCount;
            volume.setOpacity(0);
            const transparent = at([4.5,4.5,23],[4.5,4.5,4.5]);
            volume.clear(); const cleared = at([4.5,4.5,23],[4.5,4.5,4.5]);
            const error = gl.getError(); volume.dispose(); renderer.dispose();
            return {outside,inside,cleared,error,interiorObserved,unobservedShell,rawCutoff,gammaVisible,gammaNoUpload,transparent};
        });
        assert.deepEqual(errors, []);
        assert.equal(rendered.error, 0);
        assert.ok(rendered.outside.slice(0,3).some(n => n > 30));
        assert.ok(rendered.inside.slice(0,3).some(n => n > 30));
        assert.ok(rendered.interiorObserved.slice(0,3).some(n => n > 30));
        assert.deepEqual(rendered.unobservedShell.slice(0,3), [0,0,0]);
        assert.deepEqual(rendered.rawCutoff.slice(0,3), [0,0,0]);
        assert.ok(rendered.gammaVisible.slice(0,3).some(n => n > 20));
        assert.equal(rendered.gammaNoUpload, true);
        assert.deepEqual(rendered.transparent.slice(0,3), [0,0,0]);
        assert.deepEqual(rendered.cleared.slice(0,3), [0,0,0]);
    } finally {
        await browser?.close();
        await new Promise(resolve => server.close(resolve));
    }
});
