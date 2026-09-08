import { analyzeSpectrumObservation } from './spectrum-analysis-core.js';

// Exported so Node can exercise this exact worker handler through worker_threads.
export function handleSpectrumAnalysisRequest(message, publish) {
    const { id, context, observation } = message;
    try {
        const result = analyzeSpectrumObservation(observation);
        const transfers = result.spectrum?.mag ? [result.spectrum.mag.buffer] : [];
        publish({ id, context, result }, transfers);
    } catch (error) {
        publish({ id, context, error: String(error?.message || error) });
    }
}

if (typeof self !== 'undefined' && typeof self.addEventListener === 'function') {
    self.addEventListener('message', event => handleSpectrumAnalysisRequest(event.data,
        (message, transfers = []) => self.postMessage(message, transfers)));
}
