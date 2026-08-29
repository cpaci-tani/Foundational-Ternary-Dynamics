/**
 * Browser/WASM sizes above L=97 allocate enough engine/readback state to
 * produce multi-frame UI stalls on the target host. Keep the larger research
 * lattices visible for native-GPU sessions, but never offer them to the
 * browser worker path where the 60 FPS UI contract cannot be met.
 */
export const MAX_WASM_INTERACTIVE_LATTICE = 97;

export function syncScale0LatticeSizeAvailability(isNativeGPU = false) {
    const select = document.getElementById('lattice-size');
    if (!select) return;
    const native = !!isNativeGPU;
    for (const option of select.options) {
        const size = Number(option.value);
        const nativeOnly = Number.isFinite(size) && size > MAX_WASM_INTERACTIVE_LATTICE;
        option.disabled = nativeOnly && !native;
        option.textContent = nativeOnly && !native
            ? `${size} · Native GPU`
            : String(size);
    }

    if (!native && Number(select.value) > MAX_WASM_INTERACTIVE_LATTICE) {
        const activeSize = Number(window.__ftdCtx?.bridge?.latticeSize);
        select.value = String(
            Number.isFinite(activeSize) && activeSize <= MAX_WASM_INTERACTIVE_LATTICE
                ? activeSize
                : 33,
        );
    }
}
