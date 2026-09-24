// @ts-check
import { defineConfig } from '@playwright/test';
import baseConfig from './playwright.config.js';

export default defineConfig({
  ...baseConfig,
  testIgnore: [],
  testMatch: [/manual[\\/].*\.manual\.js/, /take_gallery_screenshots\.spec\.js/],
});
