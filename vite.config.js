import { defineConfig } from 'vite';
import { copyFileSync, cpSync, mkdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const webRoot = fileURLToPath(new URL('./engine/web/', import.meta.url));
const distRoot = fileURLToPath(new URL('./dist/', import.meta.url));

function preserveRuntimeAssets() {
  return {
    name: 'preserve-runtime-assets',
    writeBundle(options) {
      const outputDir = resolve(options.dir ?? distRoot);
      if (outputDir !== resolve(distRoot)) {
        throw new Error(`Unexpected dashboard output directory: ${outputDir}`);
      }
      // The deferred and on-demand sheets use runtime URLs, not CSS imports.
      cpSync(join(webRoot, 'css'), join(outputDir, 'css'), { recursive: true });
      cpSync(join(webRoot, 'wasm'), join(outputDir, 'wasm'), { recursive: true });
      cpSync(join(webRoot, 'js/vendor/three'), join(outputDir, 'js/vendor/three'), { recursive: true });
      for (const [source, destination] of [
        ['coi-serviceworker.js', 'coi-serviceworker.js'],
        ['js/ui/charts/vendor/uPlot.iife.min.js', 'js/ui/charts/vendor/uPlot.iife.min.js'],
        ['js/ui/charts/vendor/uPlot.min.css', 'js/ui/charts/vendor/uPlot.min.css'],
        ['js/ui/components/gpu-server-card.js', 'js/ui/components/gpu-server-card.js'],
        // These worker-relative URLs remain literal after Vite bundles the workers.
        ['js/bridge/sampler-registry.classic.js', 'assets/sampler-registry.classic.js'],
        ['js/bridge/sampler-cadence.classic.js', 'assets/sampler-cadence.classic.js'],
        ['js/bridge/flux-publication.classic.js', 'assets/flux-publication.classic.js'],
        ['js/vendor/minisearch/7.2.0/index.js', 'vendor/minisearch/7.2.0/index.js'],
      ]) {
        const target = join(outputDir, destination);
        mkdirSync(dirname(target), { recursive: true });
        copyFileSync(join(webRoot, source), target);
      }
    },
  };
}

export default defineConfig({
  root: 'engine/web',
  plugins: [preserveRuntimeAssets()],
  optimizeDeps: {
    exclude: [
      'three',
      'three/addons/controls/OrbitControls.js',
      'three/addons/postprocessing/EffectComposer.js',
      'three/addons/postprocessing/RenderPass.js',
      'three/addons/postprocessing/UnrealBloomPass.js',
      'three/addons/geometries/ConvexGeometry.js'
    ]
  },
  server: {
    port: 8080,
    headers: {
      'Cross-Origin-Opener-Policy': 'same-origin',
      'Cross-Origin-Embedder-Policy': 'require-corp',
    },
  },
  worker: {
    format: 'es',
  },
  build: {
    target: 'es2022',
    outDir: '../../dist',
    minify: 'esbuild',
    sourcemap: true,
    emptyOutDir: true,
    rollupOptions: {
      external: [
        'three',
        /^three\/.*/
      ]
    }
  },
});
