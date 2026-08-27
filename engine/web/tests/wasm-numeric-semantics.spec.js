import { test, expect } from '@playwright/test';

test('WASM preserves IEEE semantics and rejects invalid lattice sizes', async ({ page }) => {
  await page.goto('/');

  const result = await page.evaluate(async () => {
    const { WasmBridge } = await import('/js/bridge/wasm-bridge.js');
    const bridge = new WasmBridge();
    await bridge.init(4);
    try {
      let invalidSizeRejected = false;
      try {
        const invalidBridge = new bridge._module.RenderBridge(0);
        invalidBridge.delete();
      } catch (_error) {
        invalidSizeRejected = true;
      }

      return {
        ieeeSemantics: bridge._module.numericSemanticsAreIeee(),
        invalidSizeRejected,
      };
    } finally {
      bridge.dispose();
    }
  });

  expect(result).toEqual({
    ieeeSemantics: true,
    invalidSizeRejected: true,
  });
});
