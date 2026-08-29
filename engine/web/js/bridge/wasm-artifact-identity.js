const MANIFEST_SCHEMA_VERSION = 1;
const MANIFEST_URL = new URL('../../wasm/build_info.json', import.meta.url);
const VARIANT_CONTRACTS = Object.freeze({
    wasm32: Object.freeze({ factory: 'createFTDModule', pointerBits: 32, memory64: false, threads: false, sharedMemory: false }),
    wasm64: Object.freeze({ factory: 'createFTDModule64', pointerBits: 64, memory64: true, threads: false, sharedMemory: false }),
    'wasm32-threads': Object.freeze({ factory: 'createFTDModuleMT', pointerBits: 32, memory64: false, threads: true, sharedMemory: true }),
});

function cloneIdentity(value) {
    return value == null ? null : JSON.parse(JSON.stringify(value));
}

export function selectWasmArtifactIdentity(manifest, variantId) {
    if (!manifest || manifest.schemaVersion !== MANIFEST_SCHEMA_VERSION) {
        throw new Error(`Unsupported WASM build manifest schema: ${manifest?.schemaVersion ?? 'missing'}`);
    }
    if (!/^[0-9a-f]{40}$/.test(String(manifest.source?.commit || ''))) {
        throw new Error('WASM build manifest has no exact source commit');
    }
    if (typeof manifest.source?.dirty !== 'boolean') {
        throw new Error('WASM build manifest has no source cleanliness flag');
    }
    if (!/^[0-9a-f]{64}$/.test(String(manifest.bundleSha256 || ''))) {
        throw new Error('WASM build manifest has no canonical bundle hash');
    }
    const variant = manifest.variants?.find((candidate) => candidate?.id === variantId);
    if (!variant) throw new Error(`WASM build manifest has no ${variantId} variant`);
    const contract = VARIANT_CONTRACTS[variantId];
    if (!contract || variant.factory !== contract.factory
        || variant.abi?.pointerBits !== contract.pointerBits
        || variant.abi?.memory64 !== contract.memory64
        || variant.abi?.threads !== contract.threads
        || variant.abi?.sharedMemory !== contract.sharedMemory) {
        throw new Error(`WASM build manifest ${variantId} ABI/factory contract is invalid`);
    }
    if (!Array.isArray(variant.artifacts) || variant.artifacts.length !== 2) {
        throw new Error(`WASM build manifest ${variantId} artifact set is incomplete`);
    }
    for (const artifact of variant.artifacts) {
        if (!/^[0-9a-f]{64}$/.test(String(artifact?.sha256 || ''))
            || !Number.isSafeInteger(artifact?.sizeBytes)
            || artifact.sizeBytes <= 0) {
            throw new Error(`WASM build manifest ${variantId} has invalid artifact identity`);
        }
    }
    return Object.freeze({
        schemaVersion: manifest.schemaVersion,
        bundleSha256: manifest.bundleSha256,
        source: Object.freeze(cloneIdentity(manifest.source)),
        toolchain: Object.freeze(cloneIdentity(manifest.toolchain)),
        variant: Object.freeze(cloneIdentity(variant)),
        manifestUrl: MANIFEST_URL.href,
    });
}

async function sha256Hex(bytes) {
    if (!globalThis.crypto?.subtle) {
        throw new Error('WebCrypto SHA-256 is unavailable; WASM identity cannot be verified');
    }
    const digest = await globalThis.crypto.subtle.digest('SHA-256', bytes);
    return [...new Uint8Array(digest)]
        .map((value) => value.toString(16).padStart(2, '0')).join('');
}

function canonicalBundleBytes(manifest) {
    let value = 'ftd-wasm-bundle-v1\n';
    for (const variant of manifest.variants || []) {
        for (const artifact of variant.artifacts || []) {
            value += `${artifact.file}\0${artifact.sizeBytes}\0${artifact.sha256}\n`;
        }
    }
    return new TextEncoder().encode(value);
}

async function fetchBytes(fetchImpl, url) {
    const response = await fetchImpl(url, { cache: 'no-store' });
    if (!response.ok) {
        throw new Error(`WASM artifact fetch failed: HTTP ${response.status} (${url})`);
    }
    return new Uint8Array(await response.arrayBuffer());
}

/**
 * Fetch and verify the exact loader/module bytes before either can execute.
 * `fetchImpl` is injectable so corruption and mixed-generation behavior are
 * testable without weakening the production path.
 */
export async function loadVerifiedWasmVariant(variantId, fetchImpl = fetch) {
    const manifestResponse = await fetchImpl(MANIFEST_URL, { cache: 'no-store' });
    if (!manifestResponse.ok) {
        throw new Error(`WASM build manifest fetch failed: HTTP ${manifestResponse.status}`);
    }
    const manifest = await manifestResponse.json();
    const identity = selectWasmArtifactIdentity(manifest, variantId);
    const bundleHash = await sha256Hex(canonicalBundleBytes(manifest));
    if (bundleHash !== identity.bundleSha256) {
        throw new Error('WASM build manifest canonical bundle hash mismatch');
    }

    const byRole = {};
    for (const artifact of identity.variant.artifacts) {
        const url = new URL(`../../wasm/${artifact.file}`, import.meta.url);
        url.searchParams.set('bundle', identity.bundleSha256);
        const bytes = await fetchBytes(fetchImpl, url);
        if (bytes.byteLength !== artifact.sizeBytes) {
            throw new Error(`WASM artifact size mismatch: ${artifact.file}`);
        }
        if (await sha256Hex(bytes) !== artifact.sha256) {
            throw new Error(`WASM artifact hash mismatch: ${artifact.file}`);
        }
        byRole[artifact.role] = bytes;
    }
    if (!byRole.loader || !byRole.module) {
        throw new Error(`WASM ${variantId} loader/module roles are incomplete`);
    }
    return {
        identity,
        loaderText: new TextDecoder().decode(byRole.loader),
        moduleBytes: byRole.module,
    };
}
