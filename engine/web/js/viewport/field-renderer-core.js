export const fieldCoreMethods = {
    _ensureActiveIdx(len) {
        if (!this._activeIdx || this._activeIdx.length < len) {
            this._activeIdx = new Int32Array(len);
        }
        return this._activeIdx;
    },


    // PERF (F-13): true only when the active boundary shape actually clips.
    // For 'cube' / 'none' / undefined, insideBoundary() returns true for every
    // point, so the per-voxel insideBoundary() call in the hot arrow/streamline
    // loops is pure overhead. Hoisting `const _needsClip = this._clipActive();`
    // once per update and gating the call on it is output-EXACT (identical
    // control flow — the call would have returned true and never `continue`d)
    // while skipping ~100k function calls + 3 divisions per upload at L=64.
    // Mirrors the hoist already present in flux-renderer.js:233-234.
    _clipActive() {
        const bs = this._boundaryShape;
        return !(bs === 'cube' || bs === 'none' || bs === undefined);
    },
    _checkInsideBoundary(x, y, z) {
        const isOrigin = this._getBoundaryMode && this._getBoundaryMode() === 'origin';
        if (isOrigin) {
            const boundaryRadius = 35.0; // PE boundary radius
            return this._insideBoundary(x / boundaryRadius, y / boundaryRadius, z / boundaryRadius);
        } else {
            return this._insideBoundary((x - this._center) / this._radius, (y - this._center) / this._radius, (z - this._center) / this._radius);
        }
    },
    _syncCenterAndRadius() {
        const isOrigin = this._getBoundaryMode && this._getBoundaryMode() === 'origin';
        if (isOrigin) {
            this._center = 0.0;
            this._radius = 35.0;
            setVoxelCenterOffset(0.0);
        } else {
            this._center = this._latticeSize / 2;
            this._radius = this._latticeSize / 2;
            setVoxelCenterOffset(0.0);
        }
    }
};