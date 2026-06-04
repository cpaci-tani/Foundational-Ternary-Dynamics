// SharedArrayBuffer layout for the Scale-0 flux field — the single source of
// truth shared by the physics Web Worker (writer) and the main-thread
// MockBridgeProxy (reader). One SAB per buffer keeps attach/detach simple; an
// Int32 control SAB carries the frame counter (Atomics) + a few live integers.
// See engine/web/docs/PLAN_SCALE0_PHYSICS_WORKER.md (Phase 2).
//
// Requires cross-origin isolation (COOP/COEP via serve.py) so SharedArrayBuffer
// exists. Callers must gate on `crossOriginIsolated` before using this module.

export const FIELD_BYTES = {
    fluxJ:   (N) => N * N * N * 3 * 8,   // Float64 ×3 (Jx,Jy,Jz)
    fluxWV:  (N) => N * N * N * 3 * 8,   // Float64 ×3 (wave velocity)
    fluxMag: (N) => N * N * N * 8,       // Float64 (|J|, cached)
    state:   (N) => N * N * N,           // Int8 (ternary state grid)
};

// Control SAB (Int32Array) index map. Float64 scalars (energies, etc.) ride in
// the small frame-ready postMessage rather than shared memory.
export const CTRL = { FRAME: 0, N: 1, TICK: 2, RUNNING: 3, PCOUNT: 4, LEN: 8 };

// Allocate a fresh shared-field set at lattice size N. Throws if SAB is
// unavailable — callers must have checked `crossOriginIsolated` first.
export function allocSharedField(N) {
    return {
        N,
        ctrl:    new SharedArrayBuffer(CTRL.LEN * 4),
        fluxJ:   new SharedArrayBuffer(FIELD_BYTES.fluxJ(N)),
        fluxWV:  new SharedArrayBuffer(FIELD_BYTES.fluxWV(N)),
        fluxMag: new SharedArrayBuffer(FIELD_BYTES.fluxMag(N)),
        state:   new SharedArrayBuffer(FIELD_BYTES.state(N)),
    };
}

// Build typed-array views over an existing SAB set (used identically on the
// worker side and the main-thread shadow side — both see the same memory).
export function viewSharedField(sab) {
    return {
        N:       sab.N,
        ctrl:    new Int32Array(sab.ctrl),
        fluxJ:   new Float64Array(sab.fluxJ),
        fluxWV:  new Float64Array(sab.fluxWV),
        fluxMag: new Float64Array(sab.fluxMag),
        state:   new Int8Array(sab.state),
    };
}
