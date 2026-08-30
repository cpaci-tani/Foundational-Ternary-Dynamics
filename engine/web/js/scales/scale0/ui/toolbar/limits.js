/**
 * Browser/WASM sizes above L=97 allocate enough engine/readback state to
 * produce multi-frame UI stalls on the target host. Keep the larger research
 * lattices visible for native-GPU sessions, but never offer them to the
 * browser worker path where the 60 FPS UI contract cannot be met.
 */
export const MAX_WASM_INTERACTIVE_LATTICE = 97;
// Forced/no-COI main-thread WASM is a recovery path, not an off-thread compute
// backend. A live L=49 tick already consumes enough of the render thread to
// violate the absolute 60 Hz UI gate; L=33 is the measured interactive ceiling.
export const MAX_DIRECT_WASM_INTERACTIVE_LATTICE = 33;

export function hasScale0WasmWorkerTransport() {
    const workerEnabled = typeof window === 'undefined'
        || window.__ftdWasmWorker === undefined
        || window.__ftdWasmWorker !== false;
    return workerEnabled
        && typeof SharedArrayBuffer !== 'undefined'
        && globalThis.crossOriginIsolated === true;
}

export function scale0InteractiveLatticeLimit(isNativeGPU = false, hasWasmWorker = hasScale0WasmWorkerTransport()) {
    if (isNativeGPU) return Infinity;
    return hasWasmWorker ? MAX_WASM_INTERACTIVE_LATTICE : MAX_DIRECT_WASM_INTERACTIVE_LATTICE;
}

export function syncScale0LatticeSizeAvailability(
    isNativeGPU = false,
    hasWasmWorker = hasScale0WasmWorkerTransport(),
) {
    const select = document.getElementById('lattice-size');
    if (!select) return;
    const native = !!isNativeGPU;
    const interactiveLimit = scale0InteractiveLatticeLimit(native, hasWasmWorker);
    for (const option of select.options) {
        const size = Number(option.value);
        const nativeOnly = Number.isFinite(size) && size > MAX_WASM_INTERACTIVE_LATTICE;
        const workerOrNative = Number.isFinite(size)
            && size > MAX_DIRECT_WASM_INTERACTIVE_LATTICE
            && size <= MAX_WASM_INTERACTIVE_LATTICE;
        option.disabled = Number.isFinite(size) && size > interactiveLimit;
        if (nativeOnly && !native) option.textContent = `${size} · Native GPU`;
        else if (workerOrNative && !native && !hasWasmWorker) {
            option.textContent = `${size} · WASM worker / Native GPU`;
        } else option.textContent = String(size);
    }

    if (Number(select.value) > interactiveLimit) {
        const activeSize = Number(window.__ftdCtx?.bridge?.latticeSize);
        select.value = String(
            Number.isFinite(activeSize) && activeSize <= interactiveLimit
                ? activeSize
                : 33,
        );
    }
}
