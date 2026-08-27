import { test, expect } from '@playwright/test';
import fs from 'node:fs';
import path from 'node:path';

test('latency horizon clamp is shared by web constants and gravity proxy', async ({ page }) => {
  await page.goto('/');

  const result = await page.evaluate(async () => {
    const { LATENCY_HORIZON_CLAMP } = await import('/js/constants.js');
    const { gravitySlice } = await import('/js/scales/scale0/analysis/gravity-analysis.js');
    const N = 3;
    const mag = new Float64Array(N * N * N);
    mag[(1 * N + 1) * N + 1] = 2;
    const slice = gravitySlice(mag, N, 2, 1, 'latency', 4);
    return {
      clamp: LATENCY_HORIZON_CLAMP,
      centerLatency: slice[1 * N + 1],
    };
  });

  expect(result.clamp).toBe(0.998);
  expect(result.centerLatency).toBeCloseTo(Math.sqrt(result.clamp), 6);
});

test('latency runtime paths do not copy the clamp literal', () => {
  const engineRoot = path.resolve(process.cwd(), '..', '..');
  const runtimeFiles = [
    'src/visual_field_sample.cpp',
    'cuda/visual_field_sample.cu',
    'wasm/ftd_wasm.cpp',
    'web/js/scales/scale0/analysis/gravity-analysis.js',
  ];

  for (const relativePath of runtimeFiles) {
    const source = fs.readFileSync(path.join(engineRoot, relativePath), 'utf8');
    expect(source, relativePath).not.toContain('0.998');
    expect(source, relativePath).toContain('LATENCY_HORIZON_CLAMP');
  }
});
