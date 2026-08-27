import { parseFtv2Frame } from '../lib/ftv2.js';

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
        return { type: 'unknown', magic: 0, byteLength: buffer?.byteLength ?? 0 };
    }

    const header = new DataView(buffer);
    const magic = header.getUint32(0, true);
    if (magic === PARTICLE_FRAME_MAGIC) {
        if (buffer.byteLength < 8) {
            return { type: 'invalid-particles', error: `short FTP2 header (${buffer.byteLength} bytes)` };
        }
        const count = header.getUint32(4, true);
        const posBytes = count * 3 * 4;
        const colBytes = count * 3 * 4;
        const scalarBytes = count * 4;
        const expectedBytes = 8 + posBytes + colBytes + scalarBytes * 3;
        if (buffer.byteLength !== expectedBytes) {
            return {
                type: 'invalid-particles',
                error: `got ${buffer.byteLength}, expected ${expectedBytes}`,
            };
        }
        let offset = 8;
        const positions = new Float32Array(buffer, offset, count * 3); offset += posBytes;
        const colors = new Float32Array(buffer, offset, count * 3); offset += colBytes;
        const sizes = new Float32Array(buffer, offset, count); offset += scalarBytes;
        const spin = new Float32Array(buffer, offset, count); offset += scalarBytes;
        const colorCharge = new Float32Array(buffer, offset, count);
        return { type: 'particles', data: { positions, colors, sizes, spin, colorCharge, count } };
    }

    if (magic === FLUX_VOLUME_COMPACT_MAGIC) {
        const parsed = parseFtv2Frame(buffer);
        if (!parsed) {
            return { type: 'invalid-volume', error: `invalid FTV2 frame (${buffer.byteLength} bytes)` };
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
        if (buffer.byteLength < 8) {
            return { type: 'invalid-volume', error: `short FTV1 header (${buffer.byteLength} bytes)` };
        }
        const count = header.getUint32(4, true);
        const expectedBytes = 8 + count * 4;
        if (buffer.byteLength !== expectedBytes) {
            return {
                type: 'invalid-volume',
                error: `got ${buffer.byteLength}, expected ${expectedBytes}`,
            };
        }
        return { type: 'volume', data: new Float32Array(buffer, 8, count) };
    }

    if (magic === FIELD_SAMPLE_MAGIC || magic === FIELD_SAMPLE_V2_MAGIC) {
        const isV2 = magic === FIELD_SAMPLE_V2_MAGIC;
        const headerBytes = isV2 ? 28 : 20;
        if (buffer.byteLength < headerBytes) {
            return {
                type: 'invalid-field', token: 0,
                error: `short FTS${isV2 ? 2 : 1} header (${buffer.byteLength} bytes)`,
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
        if (!kind || (components !== 1 && components !== 3)
            || (isV2 && (effectiveStride < 1 || origin > latticeSize))
            || buffer.byteLength !== expectedBytes) {
            return {
                type: 'invalid-field', token,
                error: `got kind=${kindCode}, components=${components}, bytes=${buffer.byteLength}; expected ${expectedBytes}`,
            };
        }
        const positions = new Float32Array(buffer, headerBytes, count * 3);
        const payload = new Float32Array(
            buffer, headerBytes + count * 3 * 4, count * components,
        );
        return {
            type: 'field', token, kind, components, count, positions, payload,
            effectiveStride, origin,
        };
    }

    return { type: 'unknown', magic, byteLength: buffer.byteLength };
}
