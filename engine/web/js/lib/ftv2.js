/**
 * Compact native flux-volume frames (FTV2).
 *
 * Current wire: 20-byte header
 *   [u32 "FTV2"][u32 latticeSize][u32 stride][u32 origin][u32 axisCount]
 *   [float32 density[axisCount^3]]  x-fastest
 *
 * Legacy 16-byte header (no origin) is still accepted: origin is inferred
 * from the centre-anchored visual sample grid so grow/stride frames do not
 * shift toward index 0.
 */

import { visualSampleGrid } from './visual-sample-grid.js';

export const FTV2_MAGIC = 0x32565446; // LE bytes: F T V 2

export function parseFtv2Frame(buf) {
    if (!buf || buf.byteLength < 16) return null;
    const header = buf instanceof DataView ? buf : new DataView(buf);
    const bytes = header.buffer;
    if (header.getUint32(0, true) !== FTV2_MAGIC) return null;
    const latticeSize = header.getUint32(4, true);
    const stride = header.getUint32(8, true);
    const plausible = (axisCount, headerBytes) => {
        const count = axisCount * axisCount * axisCount;
        return latticeSize >= 1 && stride >= 1
            && axisCount >= 1 && axisCount <= 64
            && Number.isSafeInteger(count)
            && bytes.byteLength === headerBytes + count * 4;
    };

    if (bytes.byteLength >= 20) {
        const origin = header.getUint32(12, true);
        const axisCount = header.getUint32(16, true);
        if (plausible(axisCount, 20)) {
            return {
                latticeSize,
                stride,
                origin,
                axisCount,
                data: new Float32Array(bytes, 20, axisCount * axisCount * axisCount),
            };
        }
    }

    const axisCount = header.getUint32(12, true);
    if (!plausible(axisCount, 16)) return null;
    const grid = visualSampleGrid(latticeSize, stride, false);
    return {
        latticeSize,
        stride,
        origin: grid.origin,
        axisCount,
        data: new Float32Array(bytes, 16, axisCount * axisCount * axisCount),
    };
}
