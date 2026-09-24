// Shared by the classic physics worker and the module proxy. One writer,
// one reader; pin a slot during the reader's copy so the writer cannot lap it.
(function install(root) {
    'use strict';
    // [version, slot0 lock, slot1 lock, ready, slot0 tick, slot1 tick].
    // Tick belongs to the slot rather than the publication header: a writer
    // may prepare the inactive slot while a reader holds the active one, so a
    // shared tick word could otherwise describe the wrong pinned payload.
    const HEADER_BYTES = 24;
    const PROTOCOL = 'pinned-flux-v2';
    const MAX_CELL_PROBES = 128;

    function create(length) {
        if (!Number.isSafeInteger(length) || length < 1) throw new RangeError('invalid flux length');
        return new SharedArrayBuffer(HEADER_BYTES + 2 * length * Float64Array.BYTES_PER_ELEMENT);
    }

    function layout(buffer, length) {
        if (!Number.isSafeInteger(length) || length < 1
            || buffer?.byteLength !== HEADER_BYTES + 2 * length * 8) {
            throw new RangeError('invalid flux publication layout');
        }
        return new Int32Array(buffer, 0, 6);
    }

    function publicationTick(value) {
        // The C++ clock is a non-negative signed 32-bit counter. Do not
        // coerce an absent or wider JS value to zero: no exact clock is an
        // unavailable clock.
        return Number.isSafeInteger(value) && value >= 0 && value <= 0x7fffffff ? value : -1;
    }

    function publish(buffer, values, sampleTick = null) {
        const length = values.length;
        const header = layout(buffer, length);
        const version = Atomics.load(header, 0);
        const slot = 1 - (version & 1);
        // -1 owns the write; +1 pins the read. Busy means skip this optional
        // visual publication, never skip or replace an engine transaction.
        if (Atomics.compareExchange(header, 1 + slot, 0, -1) !== 0) return false;
        try {
            new Float64Array(buffer, HEADER_BYTES + slot * length * 8, length).set(values);
            Atomics.store(header, 4 + slot, publicationTick(sampleTick));
            Atomics.store(header, 0, (version + 1) | 0);
            Atomics.store(header, 3, 1);
        } finally {
            Atomics.store(header, 1 + slot, 0);
        }
        return true;
    }

    function acquire(buffer, length) {
        const header = layout(buffer, length);
        if (Atomics.load(header, 3) !== 1) return null;
        for (let attempt = 0; attempt < 3; attempt++) {
            const version = Atomics.load(header, 0);
            const slot = version & 1;
            if (Atomics.compareExchange(header, 1 + slot, 0, 1) !== 0) continue;
            if (Atomics.load(header, 0) !== version) {
                Atomics.store(header, 1 + slot, 0);
                continue;
            }
            try {
                const tick = Atomics.load(header, 4 + slot);
                return { version, sampleTick: tick >= 0 ? tick : null,
                    view: new Float64Array(buffer, HEADER_BYTES + slot * length * 8, length),
                    release() { Atomics.store(header, 1 + slot, 0); } };
            } catch (error) {
                // Even constructing the shared view can fail. No release
                // handle has reached the caller yet, so unwind our pin here.
                Atomics.store(header, 1 + slot, 0);
                throw error;
            }
        }
        return null;
    }

    function snapshot(buffer, length, cached = null) {
        const header = layout(buffer, length);
        if (Atomics.load(header, 3) !== 1) return null;
        if (cached?.buffer === buffer && cached.version === Atomics.load(header, 0)) return cached;
        const read = acquire(buffer, length);
        if (!read) return cached?.buffer === buffer ? cached : null;
        try {
            return { buffer, version: read.version, sampleTick: read.sampleTick, data: new Float64Array(read.view) };
        } finally {
            read.release();
        }
    }

    function cubeLength(size) {
        const length = size ** 3;
        if (!Number.isSafeInteger(size) || size < 1 || !Number.isSafeInteger(length)) {
            throw new RangeError('invalid flux lattice size');
        }
        return length;
    }

    /** Copy only one plane, with the same a*N+b layout as get_flux_slice. */
    function snapshotSlice(buffer, size, axis, index) {
        const length = cubeLength(size);
        if (![0, 1, 2].includes(axis) || !Number.isSafeInteger(index)) {
            throw new RangeError('invalid flux slice selector');
        }
        const fixed = Math.max(0, Math.min(index, size - 1));
        const read = acquire(buffer, length);
        if (!read) return null;
        try {
            const square = size * size;
            const data = new Float64Array(square);
            for (let a = 0; a < size; a++) {
                for (let b = 0; b < size; b++) {
                    const offset = axis === 0 ? b * square + a * size + fixed
                        : axis === 1 ? b * square + fixed * size + a
                            : fixed * square + b * size + a;
                    data[a * size + b] = read.view[offset];
                }
            }
            return { version: read.version, sampleTick: read.sampleTick, data };
        } finally {
            read.release();
        }
    }

    /**
     * One publication supplies a full-volume max(|J|^2) and 1..3 requested
     * normal slabs. Capture/validate every selector before acquiring the pin.
     * No periodic padding is synthesized. Within each slab, layout is
     * data[(plane-startPlane)*N*N + a*N + b]: axis 0 uses (a,b)=(y,z),
     * axis 1 uses (x,z), and axis 2 uses (x,y).
     */
    function captureSlabSelectors(size, requests) {
        const count = requests?.length;
        if (!Array.isArray(requests) || !Number.isSafeInteger(count) || count < 1 || count > 3) {
            throw new RangeError('invalid flux slab requests');
        }
        const selectors = new Array(count);
        for (let i = 0; i < selectors.length; i++) {
            const request = requests[i];
            const axis = request?.axis, index = request?.index;
            if (![0, 1, 2].includes(axis) || !Number.isSafeInteger(index)
                || index < 0 || index >= size) {
                throw new RangeError('invalid flux slab selector');
            }
            const startPlane = Math.max(0, index - 1);
            const planeCount = Math.min(size - 1, index + 1) - startPlane + 1;
            selectors[i] = { axis, index, startPlane, planeCount };
        }
        return selectors;
    }

    // Shared by the pinned-SAB reader and the worker's atomic Gravity batch.
    // It only reads the supplied already-captured magnitude view: callers must
    // establish their own data/provenance boundary before invoking it.
    function copySlabsWithMaxRho(values, size, requests) {
        const length = cubeLength(size);
        if (!values || values.length < length) throw new RangeError('invalid flux values');
        const selectors = captureSlabSelectors(size, requests);
        return copySlabsFromSelectors(values, size, length, selectors);
    }

    function copySlabsFromSelectors(values, size, length, selectors) {
        // Match gravity-analysis.maxRhoOf exactly, including initial value,
        // Double multiplication, ascending linear order and strict compare.
        let maxRho = 1e-30;
        for (let i = 0; i < length; i++) {
            const r = values[i] * values[i];
            if (r > maxRho) maxRho = r;
        }
        const square = size * size;
        const slabs = selectors.map(({ axis, index, startPlane, planeCount }) => {
            const data = new Float64Array(planeCount * square);
            for (let plane = startPlane; plane < startPlane + planeCount; plane++) {
                for (let a = 0; a < size; a++) {
                    for (let b = 0; b < size; b++) {
                        const offset = axis === 0 ? b * square + a * size + plane
                            : axis === 1 ? b * square + plane * size + a
                                : plane * square + b * size + a;
                        data[(plane - startPlane) * square + a * size + b] = values[offset];
                    }
                }
            }
            return { data, N: size, axis, index, startPlane, planeCount };
        });
        return { N: size, maxRho, slabs };
    }

    function snapshotSlabsWithMaxRho(buffer, size, requests) {
        const length = cubeLength(size);
        // Run potentially user-defined request getters before pinning a SAB
        // slot; the published view must never be held while client code runs.
        const selectors = captureSlabSelectors(size, requests);
        const read = acquire(buffer, length);
        if (!read) return null;
        try {
            return { version: read.version, sampleTick: read.sampleTick,
                ...copySlabsFromSelectors(read.view, size, length, selectors) };
        } finally {
            read.release();
        }
    }

    function snapshotSlabWithMaxRho(buffer, size, axis, index) {
        const batch = snapshotSlabsWithMaxRho(buffer, size, [{ axis, index }]);
        return batch ? { ...batch.slabs[0], version: batch.version, sampleTick: batch.sampleTick, maxRho: batch.maxRho } : null;
    }

    /**
     * Copy 1..128 cells from one publication, in caller order. Indices use
     * z*N*N+y*N+x. Validate/capture indices before pinning; caller getters
     * cannot change the selection or run code inside the publication copy.
     */
    function snapshotCells(buffer, size, indices) {
        const length = cubeLength(size);
        const count = indices?.length;
        if ((!Array.isArray(indices) && !ArrayBuffer.isView(indices))
            || !Number.isSafeInteger(count) || count < 1 || count > MAX_CELL_PROBES) {
            throw new RangeError('invalid flux cell probes');
        }
        const cells = new Array(count);
        for (let i = 0; i < cells.length; i++) {
            const cell = indices[i];
            if (!Number.isSafeInteger(cell) || cell < 0 || cell >= length) {
                throw new RangeError('invalid flux cell index');
            }
            cells[i] = cell;
        }
        const read = acquire(buffer, length);
        if (!read) return null;
        try {
            const data = new Float64Array(cells.length);
            for (let i = 0; i < cells.length; i++) data[i] = read.view[cells[i]];
            return { version: read.version, sampleTick: read.sampleTick, data };
        } finally {
            read.release();
        }
    }

    root.FTD_FLUX_PUBLICATION = Object.freeze({ PROTOCOL, HEADER_BYTES, MAX_CELL_PROBES,
        create, publish, acquire, snapshot, snapshotSlice, snapshotCells, snapshotSlabWithMaxRho,
        snapshotSlabsWithMaxRho, copySlabsWithMaxRho });
})(typeof self !== 'undefined' ? self : globalThis);
