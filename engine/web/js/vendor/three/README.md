# Vendored Three.js

Source: https://github.com/mrdoob/three.js (MIT)
Version: 0.169.0 (byte-identical to the cdn.jsdelivr.net files the import map previously pointed at)

Why vendored: import-map prefix mappings (`"three/addons/"`) cannot carry SRI
integrity attributes, so the CDN copy was the last un-pinned external
executable in the dashboard. Vendoring pins the bytes, makes the dashboard
fully offline-capable, and keeps every executable same-origin (consistent
with `serve.py`'s COEP setup). Precedent: `js/ui/charts/vendor/` (uPlot).

Layout mirrors the npm package (`build/` + `examples/jsm/…`) so the
`"three/addons/"` prefix mapping and the addons' relative imports work
unchanged. Consumers: `index.html` and `fields-atlas.html` import maps.

Files (directly imported):
  - build/three.module.js                            — core (self-contained, no imports)
  - examples/jsm/controls/OrbitControls.js
  - examples/jsm/geometries/ConvexGeometry.js
  - examples/jsm/loaders/RGBELoader.js
  - examples/jsm/postprocessing/EffectComposer.js
  - examples/jsm/postprocessing/RenderPass.js
  - examples/jsm/postprocessing/UnrealBloomPass.js

Files (transitive imports of the above):
  - examples/jsm/math/ConvexHull.js                  — ConvexGeometry
  - examples/jsm/postprocessing/Pass.js              — RenderPass, ShaderPass, MaskPass, UnrealBloomPass
  - examples/jsm/postprocessing/ShaderPass.js        — EffectComposer
  - examples/jsm/postprocessing/MaskPass.js          — EffectComposer
  - examples/jsm/shaders/CopyShader.js               — EffectComposer, UnrealBloomPass
  - examples/jsm/shaders/LuminosityHighPassShader.js — UnrealBloomPass

SHA-256 (as vendored, 2026-07-02):
  0a3368c165eea773490aec7b77c22de70e3eac288503409256fdbf4d12578416  build/three.module.js
  80efaadea4f8a636a65fb0bd08bfef62f3d93a0bb94e2e7500f23176c5c07f4e  examples/jsm/controls/OrbitControls.js
  9bcf80281a538592fbac81507628adf2c8450dd52f15530492725d76085ebf92  examples/jsm/geometries/ConvexGeometry.js
  f0e87d0008d9484d31358b32befd1bf80e4301f77573cc9a7cf7d871cc3f64b4  examples/jsm/loaders/RGBELoader.js
  782e7e422c8d308cd686bacfd23356bda884f01727f9fc4a7a75bcff441bd1ad  examples/jsm/math/ConvexHull.js
  d234e578618fa816955ebdc059c049c577e203e650e33cf22bde3f232c29e669  examples/jsm/postprocessing/EffectComposer.js
  328cf7db0da5d9be83ffe39d54b01d5ac1fddf108cc98182ddbb056f5c8b537f  examples/jsm/postprocessing/MaskPass.js
  b3c6128340eaa37e40a6a2f1b738e894c855239417d50959759b34a2b5e89f92  examples/jsm/postprocessing/Pass.js
  6c9b8a539ea16e898f65e4760f14937ef9ea94043bd9842c141e0301f41903e8  examples/jsm/postprocessing/RenderPass.js
  3b28a1ee27e0eb96c0eab137a1f442ccf127a926904eced2d51e125ec44af781  examples/jsm/postprocessing/ShaderPass.js
  3bd23a1097af75c7002d0ffc21a6c14f45c4dd701dbaf737030dfc61fb7c64d9  examples/jsm/postprocessing/UnrealBloomPass.js
  4e3346db194db56a596cd074e9bdb39fb5eb52040c333e0d29dc4eb1324d3b1d  examples/jsm/shaders/CopyShader.js
  9f4866f9abb2d96fd83eec46ba4bf2165b22155a7a37ff425c0f60eba18007cb  examples/jsm/shaders/LuminosityHighPassShader.js

Upgrade:
  for f in build/three.module.js examples/jsm/...; do
    curl -fsSL https://cdn.jsdelivr.net/npm/three@<version>/$f -o $f
  done
  Then re-verify the import closure: grep the vendored files for
  `from '...'` — every relative specifier must resolve to a vendored file
  and the only bare specifier must be 'three'. New three versions may add
  or move transitive imports (e.g. r170+ splits build/three.core.js out of
  three.module.js — it would need vendoring too). Update the hashes above.
