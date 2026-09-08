import { test, expect } from '@playwright/test';

test('Spectrum default browser timers and real module worker survive cancel and completion', async ({ page }) => {
    const errors = [];
    page.on('pageerror', error => errors.push(String(error)));
    await page.goto('/js/scales/scale0/analysis/spectrum-analysis-client.js');
    const result = await page.evaluate(async () => {
        const { SpectrumAnalysisClient, captureSpectrumObservation } = await import('/js/scales/scale0/analysis/spectrum-analysis-client.js');
        const sample = { count: 1, positions: new Float32Array([1.5, 1.5, 1.5]),
            vectors: new Float32Array([1, 2, 3]), values: new Float32Array([.25]), sampleTick: 7 };
        const capture = () => captureSpectrumObservation({ getScale0FieldSamples: () => sample,
            hasScale0SamplerSnapshot: () => true }, { L: 3, stride: 1, M: 8, mode: 'live' });
        const client = new SpectrumAnalysisClient();
        let obsoletePublished = false;
        try {
            // This uses Chromium's real receiver-sensitive clearTimeout even
            // before a worker exists, reproducing the original startup failure.
            client.cancel();
            const first = client.submit(capture(), { context: { generation: 1 },
                onResult() { obsoletePublished = true; } });
            const obsoleteWorker = client.worker;
            client.cancel();
            const completed = await new Promise((resolve, reject) => {
                client.submit(capture(), { context: { generation: 2 },
                    onResult: (observation, metadata) => resolve({ observation, metadata }),
                    onError: reason => reject(new Error(reason)) });
            });
            return {
                advancedRequest: completed.metadata.id > first,
                freshWorker: client.worker !== obsoleteWorker,
                generation: completed.metadata.context.generation,
                sampleTick: completed.observation.spectrum.provenance.sampleTick,
                grid: completed.observation.spectrum.M,
                finitePower: Number.isFinite(completed.observation.spectrum.spec.totalE)
                    && completed.observation.spectrum.spec.totalE > 0,
                parsevalError: Math.abs(completed.observation.spectrum.parseval - 1),
                cacheIntact: sample.vectors.byteLength === 12 && sample.vectors[2] === 3,
                obsoletePublished, busy: client.busy, queued: client.queued,
            };
        } finally { client.dispose(); }
    });
    expect(result).toEqual({ advancedRequest: true, freshWorker: true, generation: 2,
        sampleTick: 7, grid: 8, finitePower: true, parsevalError: expect.any(Number),
        cacheIntact: true, obsoletePublished: false, busy: false, queued: false });
    expect(result.parsevalError).toBeLessThan(1e-12);
    expect(errors).toEqual([]);
});
