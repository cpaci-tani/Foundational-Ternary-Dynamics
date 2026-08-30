/**
 * app-wire/bridge-boot.js — boot-time bridge probe (native → WASM).
 * Extracted from app.js init() (behavior-preserving).
 */

import { createBridge } from '../bridge-init.js?v=2';
import { tryNativeBridge } from '../ws-bridge.js?v=3';
import { parseNativeWsPort } from '../lib/origin-policy.js';
import { debugLog } from '../core/log.js';

/**
 * Probe native WebSocket bridge, else create in-thread WasmBridge.
 * @param {number} latticeSize
 * @param {{
 *   showToast: (msg: string, severity?: string) => void,
 *   loadProgress: (pct: number, msg: string) => void,
 * }} ui
 * @returns {Promise<object>} live bridge
 */
export async function bootBridge(latticeSize, ui) {
    const { showToast, loadProgress } = ui;
    const engineEl = document.getElementById('status-engine');
    const computeEl = document.getElementById('status-compute');

    const urlParams = new URLSearchParams(window.location.search);
    const forceNative = urlParams.get('engine') === 'native';
    const forceWasm = urlParams.get('engine') === 'wasm';
    const isLiveServerPort = /^55\d{2}$/.test(window.location.port);

    let bridge = null;
    if (!forceWasm && (forceNative || !isLiveServerPort)) {
        const requestedWsPort = parseNativeWsPort(urlParams, window.location.href, 9100);
        debugLog(`[init] Trying native GPU engine on ws://127.0.0.1:${requestedWsPort}...`);
        try {
            bridge = await tryNativeBridge(latticeSize);
        } catch (e) {
            console.warn('[init] Native GPU bridge error:', e);
            bridge = null;
        }
    } else if (forceWasm) {
        debugLog('[init] Native GPU bypassed by explicit ?engine=wasm execution contract');
        bridge = null;
    } else {
        debugLog('[init] Skipping native GPU: static dev server (use ?engine=native for ws_server)');
        bridge = null;
    }

    debugLog('[init] Native bridge result:', bridge ? 'connected' : 'unavailable');
    if (bridge && bridge.ready) {
        loadProgress(30, 'GPU engine connected');
        if (engineEl) {
            engineEl.textContent = 'Native Engine';
            engineEl.style.color = 'var(--accent-text)';
        }
        if (computeEl) {
            computeEl.textContent = bridge.isNativeGPU ? 'GPU' : 'CPU';
            computeEl.style.color = bridge.isNativeGPU ? 'var(--positive-text)' : 'var(--axis-z-text)';
            computeEl.title = bridge.isNativeGPU
                ? 'Connected to native GPU engine (CUDA)'
                : 'Connected to native CPU engine';
        }
        showToast('Native engine connected — CUDA backend active.', 'success');
        return bridge;
    }

    loadProgress(20, 'Compiling WASM engine...');
    try {
        bridge = await createBridge(latticeSize);
    } catch (err) {
        loadProgress(25, 'WASM engine FAILED to load');
        throw new Error(
            'WASM engine failed to load (' + err.message + '). The dashboard ' +
            'cannot run scenarios without it — rebuild via engine\\build_wasm.bat ' +
            'and check the browser console / network tab for the failing module.');
    }
    loadProgress(30, 'WASM engine ready');
    if (engineEl) {
        engineEl.textContent = 'WASM Engine';
        engineEl.style.color = 'var(--positive-text)';
    }
    if (computeEl) {
        computeEl.textContent = 'CPU';
        computeEl.style.color = 'var(--axis-z-text)';
        computeEl.title = 'Browser WASM runs on CPU. Start ws_server.exe for GPU.';
    }
    return bridge;
}
