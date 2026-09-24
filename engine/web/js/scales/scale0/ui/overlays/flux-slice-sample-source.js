/** Acquisition owner for Flux Slice sampler demands and source frames. */
import { SLOT_TO_KIND, STRIDE_ONE_SLOTS } from './flux-slice-helpers.js';

export function buildFluxSliceSampleCache(owner, bridge, visibleDrivers, mid) {

        if (owner._samplerBridge && owner._samplerBridge !== bridge) owner._releaseAllWantedSamplers();
        owner._samplerBridge = bridge;
        const sampled = {};
        const neededSlots = new Set();
        for (const drv of visibleDrivers) {
            if (drv.slot && !drv.forceType) neededSlots.add(drv.slot);
            if (drv.requiredSampledKeys) {
                for (const slot of drv.requiredSampledKeys) neededSlots.add(slot);
            }
        }
        // mid % 2 === 0  ⟺  N ≡ 1 (mod 4), since mid = (N-1)/2 for the odd
        // lattice sizes this app uses. Every size the "Size" dropdown offers
        // today (9, 17, 25, 33, 49, 65, 97, 113, 145, 181) satisfies this, so
        // coarseStride is always 2 in practice — but the fallback to 1 is a
        // real correctness case (stride-2 sampling from index 0 would MISS
        // an odd mid-plane entirely), not dead code. If a future lattice
        // size ≡ 3 (mod 4) is ever added to that dropdown, every kind here
        // (including the force fields) silently drops to full-resolution
        // sampling — an 8x cost multiplier. The get_em_force_field /
        // get_strong_force_field budget guards in ftd_wasm.cpp are the real
        // backstop against that regressing into a worker stall again; this
        // comment exists so a future edit to the size list doesn't reopen
        // the gap unknowingly.
        const coarseStride = (mid % 2 === 0) ? 2 : 1;
        // Native-GPU bridge only: fetch just the three center mid-planes we draw
        // (getFieldSlices) instead of the whole field cube — the cube is several
        // MiB per field over the WebSocket and we discard ~95% of it. The WASM
        // bridge samples in-process (a cheap heap view, no transfer), so it has
        // no getFieldSlices and keeps the full-cube getSamplerOr path unchanged.
        const useSlices = typeof bridge.getFieldSlices === 'function';
        const wantedKeys = new Set();
        for (const slot of neededSlots) {
            const kind = SLOT_TO_KIND[slot];
            if (!kind) continue;
            const stride = STRIDE_ONE_SLOTS.has(slot) ? 1 : coarseStride;
            sampled[slot] = useSlices
                ? (bridge.getFieldSlices(kind, mid, stride) ?? null)
                : (bridge.getSamplerOr?.(kind, stride) ?? null);
            wantedKeys.add(`${kind}@${stride}`);
        }
        // Force fields: getEMForceField / getGravityForceField /
        // getStrongForceField, not a sampler `kind` — fetched directly,
        // once per visible force row, at the same coarse stride as raw kinds
        // (their own defaults are stride 2 too, so this matches, not
        // regresses, established convention).
        // Force fields ('em'/'gravity'/'strong') are valid sample kinds too, so
        // on the GPU bridge they take the same slice fast path; the WASM bridge
        // keeps its dedicated getEM/Gravity/StrongForceField cube getters.
        for (const drv of visibleDrivers) {
            if (!drv.forceType || sampled[drv.slot] !== undefined) continue;
            if (useSlices) {
                sampled[drv.slot] = bridge.getFieldSlices(drv.forceType, mid, coarseStride) ?? null;
            } else if (drv.forceType === 'em') {
                sampled[drv.slot] = bridge.getEMForceField?.(coarseStride) ?? null;
            } else if (drv.forceType === 'gravity') {
                sampled[drv.slot] = bridge.getGravityForceField?.(coarseStride) ?? null;
            } else if (drv.forceType === 'strong') {
                sampled[drv.slot] = bridge.getStrongForceField?.(coarseStride) ?? null;
            }
            wantedKeys.add(`${drv.forceType}@${coarseStride}`);
        }
        // Release any kind@stride that was wanted last frame but has no
        // visible consumer this frame — WasmBridgeProxy._wantSampler's
        // registration is otherwise permanently sticky (the worker computes
        // every wanted kind on every postFrame() forever; there is no
        // un-want without this). Diffed here (not per-driver-hide) because
        // several rows share the same slot (e.g. eField feeds |E|, emEnergy,
        // ePressure, ℒ(x) — only release once NONE of them are visible).
        if (typeof bridge.replaceSamplerWants === 'function') {
            bridge.replaceSamplerWants('flux-slice', [...wantedKeys]);
            owner._prevWantedKeys = wantedKeys;
        } else if (typeof bridge.unwantSampler === 'function') {
            if (owner._prevWantedKeys) {
                for (const key of owner._prevWantedKeys) {
                    if (wantedKeys.has(key)) continue;
                    const at = key.lastIndexOf('@');
                    bridge.unwantSampler(key.slice(0, at), Number(key.slice(at + 1)));
                }
            }
            owner._prevWantedKeys = wantedKeys;
        }
        return sampled;
}
