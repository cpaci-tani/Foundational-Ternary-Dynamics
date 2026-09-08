import { parseFtv2Frame } from '../lib/ftv2.js';
import { exactCounter, safeCounterNumber } from '../lib/exact-counter.js';

export const NATIVE_FRAME_V3_MAGIC = 0x334e5446;
export const NATIVE_FRAME_V3_HEADER_BYTES = 88;

const FLUX_VOLUME_MAGIC = 0x31565446; // FTV1
const FLUX_VOLUME_COMPACT_MAGIC = 0x32565446; // FTV2
const PARTICLE_FRAME_MAGIC = 0x32505446; // FTP2
const FIELD_SAMPLE_MAGIC = 0x31535446; // FTS1
const FIELD_SAMPLE_V2_MAGIC = 0x32535446; // FTS2

export const FLUX_VOLUME_AXIS_SAMPLES = 53;
export const FIELD_SAMPLE_KINDS = Object.freeze([
    'e', 'b', 'poynting', 'divJ', 'fluxVector', 'vorticity', 'helicity',
    'kretschmann', 'latency', 'fisher', 'coherence', 'curlJ', 'state',
    'gaussResidual', 'em', 'gravity', 'strong', 'poissonLatency',
]);
export const FIELD_SAMPLE_KIND_CODES = new Map(
    FIELD_SAMPLE_KINDS.map((kind, code) => [kind, code]),
);
export const WS_VECTOR_FIELD_KINDS = new Set([
    'e', 'b', 'poynting', 'fluxVector', 'curlJ', 'em', 'gravity', 'strong',
]);

export function decodeNativeBinaryFrame(buffer, { latticeSize = 0 } = {}) {
    if (!(buffer instanceof ArrayBuffer) || buffer.byteLength < 4) {
        return { type: 'invalid-envelope', error: 'Missing binary frame' };
    }
    const header = new DataView(buffer);
    if (header.getUint32(0, true) !== NATIVE_FRAME_V3_MAGIC) {
        const frame = decodeLegacyFrame(header, { latticeSize });
        frame.provenance = Object.freeze({ status: 'unavailable', reason: 'legacy-wire' });
        return frame;
    }
    const invalid = error => ({ type: 'invalid-envelope', error });
    if (buffer.byteLength < NATIVE_FRAME_V3_HEADER_BYTES
        || header.getUint32(4, true) !== NATIVE_FRAME_V3_HEADER_BYTES) return invalid('Invalid FTN3 header length');
    const requestId = safeCounterNumber(header.getBigUint64(8, true));
    const payloadBytes = safeCounterNumber(header.getBigUint64(40, true));
    const size = header.getUint32(64, true);
    if (!requestId || payloadBytes === null || payloadBytes < 4
        || payloadBytes !== buffer.byteLength - NATIVE_FRAME_V3_HEADER_BYTES
        || !size || header.getUint32(68, true) !== 1) return invalid('Invalid FTN3 identity, extent or flags');
    const nested = new DataView(buffer, NATIVE_FRAME_V3_HEADER_BYTES, payloadBytes);
    if (![PARTICLE_FRAME_MAGIC, FLUX_VOLUME_COMPACT_MAGIC, FIELD_SAMPLE_V2_MAGIC]
        .includes(nested.getUint32(0, true))) return invalid('Unsupported FTN3 payload');
    // V3 requires the explicit-origin FTV2 layout; an inferred legacy origin
    // is not an observation stamped by this versioned producer.
    if (nested.getUint32(0, true) === FLUX_VOLUME_COMPACT_MAGIC
        && (payloadBytes < 20 || payloadBytes !== 20 + nested.getUint32(16, true) ** 3 * 4)) {
        return invalid('FTN3 requires explicit volume origin');
    }
    const nativeInstanceId = header.getBigUint64(80, true).toString(16).padStart(16, '0')
        + header.getBigUint64(72, true).toString(16).padStart(16, '0');
    const physicalTime = header.getFloat64(48, true), dt = header.getFloat64(56, true);
    const provenance = Object.freeze({
        status: 'approximate', model: 'production-reference', units: 'reference-lattice',
        nativeInstanceId, source: nativeInstanceId,
        sampleTick: exactCounter(header.getBigUint64(16, true)),
        sourceEpoch: exactCounter(header.getBigUint64(24, true)),
        epoch: exactCounter(header.getBigUint64(32, true)),
        physicalTime: Number.isFinite(physicalTime) ? physicalTime : null,
        dt: Number.isFinite(dt) ? dt : null, latticeSize: size,
    });
    const frame = decodeLegacyFrame(nested, { latticeSize: size });
    if (!['particles', 'volume', 'field'].includes(frame.type)) return invalid(frame.error || 'Invalid FTN3 payload');
    if (frame.type === 'volume' && frame.data.latticeSize !== size) return invalid('FTN3 lattice size mismatch');
    return { ...frame, requestId, wireVersion: 3, provenance };
}

function decodeLegacyFrame(header, { latticeSize = 0 } = {}) {
    const buffer = header.buffer;
    const base = header.byteOffset;
    const length = header.byteLength;
    const magic = header.getUint32(0, true);
    if (magic === PARTICLE_FRAME_MAGIC) {
        if (length < 8) {
            return { type: 'invalid-particles', error: `short FTP2 header (${length} bytes)` };
        }
        const count = header.getUint32(4, true);
        const posBytes = count * 3 * 4;
        const colBytes = count * 3 * 4;
        const scalarBytes = count * 4;
        const expectedBytes = 8 + posBytes + colBytes + scalarBytes * 3;
        if (length !== expectedBytes) {
            return {
                type: 'invalid-particles',
                error: `got ${length}, expected ${expectedBytes}`,
            };
        }
        let offset = base + 8;
        const positions = new Float32Array(buffer, offset, count * 3); offset += posBytes;
        const colors = new Float32Array(buffer, offset, count * 3); offset += colBytes;
        const sizes = new Float32Array(buffer, offset, count); offset += scalarBytes;
        const spin = new Float32Array(buffer, offset, count); offset += scalarBytes;
        const colorCharge = new Float32Array(buffer, offset, count);
        return { type: 'particles', data: { positions, colors, sizes, spin, colorCharge, count } };
    }

    if (magic === FLUX_VOLUME_COMPACT_MAGIC) {
        const parsed = parseFtv2Frame(header);
        if (!parsed) {
            return { type: 'invalid-volume', error: `invalid FTV2 frame (${length} bytes)` };
        }
        return {
            type: 'volume',
            data: {
                data: parsed.data,
                latticeSize: parsed.latticeSize,
                stride: parsed.stride,
                origin: parsed.origin,
                axisCount: parsed.axisCount,
            },
        };
    }

    if (magic === FLUX_VOLUME_MAGIC) {
        if (length < 8) {
            return { type: 'invalid-volume', error: `short FTV1 header (${length} bytes)` };
        }
        const count = header.getUint32(4, true);
        const expectedBytes = 8 + count * 4;
        if (length !== expectedBytes) {
            return {
                type: 'invalid-volume',
                error: `got ${length}, expected ${expectedBytes}`,
            };
        }
        return { type: 'volume', data: new Float32Array(buffer, base + 8, count) };
    }

    if (magic === FIELD_SAMPLE_MAGIC || magic === FIELD_SAMPLE_V2_MAGIC) {
        const isV2 = magic === FIELD_SAMPLE_V2_MAGIC;
        const headerBytes = isV2 ? 28 : 20;
        if (length < headerBytes) {
            return {
                type: 'invalid-field', token: 0,
                error: `short FTS${isV2 ? 2 : 1} header (${length} bytes)`,
            };
        }
        const token = header.getUint32(4, true);
        const kindCode = header.getUint32(8, true);
        const components = header.getUint32(12, true);
        const count = header.getUint32(16, true);
        const effectiveStride = isV2 ? header.getUint32(20, true) : null;
        const origin = isV2 ? header.getUint32(24, true) : null;
        const kind = FIELD_SAMPLE_KINDS[kindCode];
        const expectedBytes = headerBytes + count * (3 + components) * 4;
        if (!kind || components !== (WS_VECTOR_FIELD_KINDS.has(kind) ? 3 : 1)
            || (isV2 && (effectiveStride < 1 || origin >= latticeSize))
            || length !== expectedBytes) {
            return {
                type: 'invalid-field', token,
                error: `got kind=${kindCode}, components=${components}, bytes=${length}; expected ${expectedBytes}`,
            };
        }
        const positions = new Float32Array(buffer, base + headerBytes, count * 3);
        const payload = new Float32Array(
            buffer, base + headerBytes + count * 3 * 4, count * components,
        );
        return {
            type: 'field', token, kind, components, count, positions, payload,
            effectiveStride, origin,
        };
    }

    return { type: 'unknown', magic, byteLength: length };
}
