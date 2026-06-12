import { defineConfig } from 'vite';

export default defineConfig({
  root: 'engine/web',
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
  build: {
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
