// Passive transport validation. This module does not evolve the finite state.
const LAW_ID = 'phi-hydro-staged-candidate-1';
const HEADER_SIZE = 116;
const BYTES_PER_SITE = 229;
const HEX_SHA256 = /^[0-9a-f]{64}$/;
const MAGIC = [0x46, 0x54, 0x44, 0x48, 0x59, 0x30, 0x31, 0x00];
const ARRAY_ELEMENTS_PER_SITE = [
    ['s', 1], ['bank', 192], ['sc', 6], ['fcc', 12],
    ['admitted_sc', 3], ['admitted_fcc', 6], ['gate_sc', 3], ['gate_fcc', 6],
];
const hex = (bytes) => [...bytes].map((b) => b.toString(16).padStart(2, '0')).join('');

export async function sha256Hex(buffer) {
    return hex(new Uint8Array(await crypto.subtle.digest('SHA-256', buffer)));
}

function requireHash(hash, name) {
    if (typeof hash !== 'string' || !HEX_SHA256.test(hash)) throw new Error(`Invalid ${name} SHA-256`);
    return hash;
}

export async function verifyPinnedArtifact(buffer, expected, name) {
    requireHash(expected, name);
    const actual = await sha256Hex(buffer);
    if (actual !== expected) throw new Error(`${name} SHA-256 mismatch`);
    return actual;
}

// Optional runtime pins use the same manifest.files URL convention as preparations.
// A fetched digest with no pin is provenance only, not an execution-identity claim.
export async function verifyRuntimeArtifact(buffer, artifactUrl, manifest, manifestUrl) {
    const actual = await sha256Hex(buffer);
    let pinned = false;
    for (const [name, expected] of Object.entries(manifest.files || {})) {
        if (new URL(name, manifestUrl).href !== new URL(artifactUrl).href) continue;
        requireHash(expected, name);
        if (actual !== expected) throw new Error(`${name} SHA-256 mismatch`);
        pinned = true;
    }
    return { sha256: actual, pinned };
}

function base64FromBytes(bytes) {
    let binary = '';
    for (let i = 0; i < bytes.length; i += 0x8000) {
        binary += String.fromCharCode(...bytes.subarray(i, i + 0x8000));
    }
    return btoa(binary);
}

export async function ftdhy01ToCheckpointJson(buffer, manifest, expectedL) {
    const bytes = new Uint8Array(buffer);
    if (bytes.length < HEADER_SIZE || MAGIC.some((value, i) => bytes[i] !== value)) {
        throw new Error('preparation is not a complete FTDHY01 transport');
    }
    if (manifest.law_id !== LAW_ID) throw new Error('preparation law mismatch');
    requireHash(manifest.table_hash, 'table');
    requireHash(manifest.encoding_hash, 'encoding');
    if (manifest.table_hash16 !== manifest.table_hash.slice(0, 16)) throw new Error('table hash prefix mismatch');
    const lawHash = await sha256Hex(new TextEncoder().encode(manifest.law_id));
    if (hex(bytes.subarray(20, 52)) !== manifest.table_hash
        || hex(bytes.subarray(52, 84)) !== manifest.encoding_hash
        || hex(bytes.subarray(84, 116)) !== lawHash) {
        throw new Error('preparation header law/table/encoding mismatch');
    }
    const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
    const L = view.getUint32(8, true);
    if (!Number.isSafeInteger(expectedL) || expectedL < 3 || L !== expectedL) {
        throw new Error('preparation lattice size does not match selection');
    }
    const n = L ** 3;
    const expectedLength = HEADER_SIZE + BYTES_PER_SITE * n;
    if (!Number.isSafeInteger(expectedLength) || expectedLength !== bytes.length) {
        throw new Error('preparation transport length does not match its declared L');
    }
    let offset = HEADER_SIZE;
    const arrays = {};
    for (const [name, perSite] of ARRAY_ELEMENTS_PER_SITE) {
        const size = perSite * n;
        arrays[name] = base64FromBytes(bytes.subarray(offset, offset + size));
        offset += size;
    }
    return JSON.stringify({
        schema: 'ftd-hydro-checkpoint-2', law: manifest.law_id, table: manifest.table_hash,
        encoding: manifest.encoding_hash, boundary: 'periodic', L,
        microtick: view.getBigUint64(12, true).toString(), arrays,
    });
}

export async function validateHydroPreparation({ manifest, sidecarBuffer, binBuffer, preparation, L }) {
    const binName = `${preparation}_${L}.bin`;
    const jsonName = `${preparation}_${L}.json`;
    const entry = manifest.preparations?.find((item) => item.name === preparation && item.L === L);
    if (!entry || entry.bin !== binName || entry.json !== jsonName || !manifest.sizes?.includes(L)) {
        throw new Error('preparation selection is absent from manifest');
    }
    await verifyPinnedArtifact(sidecarBuffer, manifest.files?.[jsonName], jsonName);
    const binHash = await verifyPinnedArtifact(binBuffer, manifest.files?.[binName], binName);
    const sidecar = JSON.parse(new TextDecoder('utf-8', { fatal: true }).decode(sidecarBuffer));
    if (sidecar.preparation !== preparation || sidecar.L !== L || sidecar.law_id !== manifest.law_id
        || sidecar.bin_file !== binName || sidecar.bin_sha256 !== binHash || entry.bin_sha256 !== binHash) {
        throw new Error('preparation sidecar identity mismatch');
    }
    if (!Array.isArray(sidecar.k_integers) || sidecar.k_integers.length !== 3
        || !sidecar.k_integers.every(Number.isSafeInteger)
        || !Array.isArray(sidecar.polarization_vector) || sidecar.polarization_vector.length !== 3
        || !sidecar.polarization_vector.every(Number.isFinite)
        || !Number.isFinite(sidecar.k_squared) || sidecar.k_squared <= 0
        || sidecar.prediction?.microticks_per_stage !== 4
        || !Array.isArray(sidecar.prediction.amplitude)
        || !sidecar.prediction.amplitude.every((value) => Number.isFinite(value) && value >= 0)) {
        throw new Error('invalid preparation observation metadata');
    }
    for (const name of ['nu_T2', 'nu_E', 'c_s2', 'g']) {
        if (!Number.isFinite(sidecar.constants?.[name]?.float)) throw new Error('invalid preparation reference constants');
    }
    if (!['nu_T2', 'nu_E', 'c_s2'].includes(sidecar.constant_probed)) throw new Error('unsupported preparation reference');
    const checkpoint = await ftdhy01ToCheckpointJson(binBuffer, manifest, L);
    return { checkpoint, sidecar, binHash };
}

export function supportsShearDecayFit(sidecar) {
    return sidecar?.constant_probed === 'nu_T2' || sidecar?.constant_probed === 'nu_E';
}
