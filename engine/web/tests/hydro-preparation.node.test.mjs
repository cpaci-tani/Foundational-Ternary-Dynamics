// Small synthetic byte/transport fixtures only; no physics execution or WASM build.
import assert from 'node:assert/strict';
import test from 'node:test';
import {
    sha256Hex, ftdhy01ToCheckpointJson, validateHydroPreparation,
    verifyRuntimeArtifact, supportsShearDecayFit,
} from '../../strict/web/hydro/hydro-preparation.js';

const utf8 = (value) => new TextEncoder().encode(typeof value === 'string' ? value : JSON.stringify(value));
const preparation = 'hydro-shear-wave-t2';
const L = 3;
const hashBytes = (hex) => Uint8Array.from(hex.match(/../g), (value) => parseInt(value, 16));

async function fixture(ticks = 0n) {
    const law = 'phi-hydro-staged-candidate-1';
    const manifest = { law_id: law, table_hash: '12'.repeat(32), encoding_hash: '34'.repeat(32),
        table_hash16: '12'.repeat(8), sizes: [L], files: {}, preparations: [] };
    const bin = new Uint8Array(116 + 229 * L ** 3);
    bin.set([0x46, 0x54, 0x44, 0x48, 0x59, 0x30, 0x31, 0]);
    const view = new DataView(bin.buffer);
    view.setUint32(8, L, true);
    view.setBigUint64(12, ticks, true);
    bin.set(hashBytes(manifest.table_hash), 20);
    bin.set(hashBytes(manifest.encoding_hash), 52);
    bin.set(hashBytes(await sha256Hex(utf8(law))), 84);
    // Nonzero sentinel verifies transport conversion preserves the data bytes.
    bin[116 + 229 * L ** 3 - 1] = 1;
    const binName = `${preparation}_${L}.bin`, jsonName = `${preparation}_${L}.json`;
    const binHash = await sha256Hex(bin);
    const sidecar = { preparation, L, law_id: law, bin_file: binName, bin_sha256: binHash,
        k_integers: [1, 0, 0], k_squared: 0.5, polarization_vector: [0, 1, 0],
        prediction: { microticks_per_stage: 4, amplitude: [1, 0.8] }, constant_probed: 'nu_T2',
        constants: Object.fromEntries(['nu_T2', 'nu_E', 'c_s2', 'g'].map((key) => [key, { float: 1 }])) };
    const sidecarBuffer = utf8(sidecar).buffer;
    manifest.files[binName] = binHash;
    manifest.files[jsonName] = await sha256Hex(sidecarBuffer);
    manifest.preparations.push({ name: preparation, L, bin: binName, json: jsonName, bin_sha256: binHash });
    return { manifest, sidecarBuffer, binBuffer: bin.buffer, preparation, L, sidecar, binName, jsonName };
}

test('binary conversion preserves the complete uint64 microtick and payload bytes', async () => {
    for (const tick of [0n, 9007199254740991n, 9007199254740993n, 18446744073709551615n]) {
        const input = await fixture(tick);
        const result = await validateHydroPreparation(input);
        const checkpoint = JSON.parse(result.checkpoint);
        assert.equal(checkpoint.schema, 'ftd-hydro-checkpoint-2');
        assert.equal(checkpoint.microtick, tick.toString());
        assert.equal(BigInt(checkpoint.microtick) % 4n, tick % 4n);
        assert.equal(Buffer.from(checkpoint.arrays.gate_fcc, 'base64').at(-1), 1);
        assert.equal(result.binHash, input.manifest.files[input.binName]);
    }
});

test('each native header identity is checked independently of manifest substitution', async () => {
    for (const offset of [20, 52, 84]) {
        const { binBuffer, manifest } = await fixture();
        new Uint8Array(binBuffer)[offset] ^= 1;
        await assert.rejects(ftdhy01ToCheckpointJson(binBuffer, manifest, L), /header law\/table\/encoding mismatch/);
    }
});

test('short, wrong-size and wrong-law preparations fail before conversion', async () => {
    const { binBuffer, manifest } = await fixture();
    await assert.rejects(ftdhy01ToCheckpointJson(binBuffer.slice(0, 20), manifest, L), /complete FTDHY01/);
    await assert.rejects(ftdhy01ToCheckpointJson(binBuffer.slice(0, -1), manifest, L), /transport length/);
    await assert.rejects(ftdhy01ToCheckpointJson(binBuffer, manifest, L + 1), /lattice size/);
    await assert.rejects(ftdhy01ToCheckpointJson(binBuffer, { ...manifest, law_id: 'another-law' }, L), /law mismatch/);
});

test('downloaded preparation and sidecar must match the manifest byte hashes', async () => {
    const wrongBin = await fixture();
    new Uint8Array(wrongBin.binBuffer)[116] ^= 1;
    await assert.rejects(validateHydroPreparation(wrongBin), /\.bin SHA-256 mismatch/);
    const wrongSidecar = await fixture();
    wrongSidecar.sidecarBuffer = utf8({ ...wrongSidecar.sidecar, L: 4 }).buffer;
    await assert.rejects(validateHydroPreparation(wrongSidecar), /\.json SHA-256 mismatch/);
});

test('self-consistent hashes cannot hide wrong selection metadata or missing pins', async () => {
    for (const change of [{ L: 4 }, { preparation: 'hydro-sound-wave' }, { law_id: 'wrong-law' }, { bin_sha256: '00'.repeat(32) }]) {
        const input = await fixture();
        input.sidecarBuffer = utf8({ ...input.sidecar, ...change }).buffer;
        input.manifest.files[input.jsonName] = await sha256Hex(input.sidecarBuffer);
        await assert.rejects(validateHydroPreparation(input), /sidecar identity mismatch/);
    }
    const noPin = await fixture();
    delete noPin.manifest.files[noPin.jsonName];
    await assert.rejects(validateHydroPreparation(noPin), /Invalid .* SHA-256/);
});

test('runtime files verify available URL-resolved pins and mark unpinned digests', async () => {
    const bytes = utf8('synthetic WASM artifact');
    const url = new URL('https://example.test/engine/build_strict_hydro_wasm/ftd_hydro_wasm.mjs');
    const manifestUrl = new URL('https://example.test/engine/build_strict_hydro/lab/manifest.json');
    const key = '../../build_strict_hydro_wasm/ftd_hydro_wasm.mjs';
    const hash = await sha256Hex(bytes);
    assert.deepEqual(await verifyRuntimeArtifact(bytes, url, { files: {} }, manifestUrl), { sha256: hash, pinned: false });
    assert.deepEqual(await verifyRuntimeArtifact(bytes, url, { files: { [key]: hash } }, manifestUrl), { sha256: hash, pinned: true });
    await assert.rejects(verifyRuntimeArtifact(bytes, url, { files: { [key]: '00'.repeat(32) } }, manifestUrl), /SHA-256 mismatch/);
});

test('sound reference never enables the shear decay-to-diffusivity readout', () => {
    assert.equal(supportsShearDecayFit({ constant_probed: 'c_s2' }), false);
    assert.equal(supportsShearDecayFit({ constant_probed: 'nu_T2' }), true);
    assert.equal(supportsShearDecayFit({ constant_probed: 'nu_E' }), true);
});
