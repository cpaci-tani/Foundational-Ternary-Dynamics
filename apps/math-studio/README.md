# Curve Geometry Studio

A standalone, local-first laboratory for authoring, analyzing, animating, and recording parametric curves in synchronized 2D and 3D views.

## Run

From the repository root, double-click `start_math_studio.bat`. It installs dependencies when needed, starts Vite, and opens the studio automatically.

Or run it manually:

```powershell
cd apps/math-studio
npm install
npm run dev
```

The development server opens at `http://127.0.0.1:4173` by default.

## Current Capabilities

- Editable `x(t)`, `y(t)`, and `z(t)` expressions evaluated with Math.js
- Up to 25 dyadic modes, each with independent amplitude, phase, and continuous chirality controls
- Manual numeric entry plus editable minimum and maximum bounds for every parameter
- Scientific parameter registers with symbols, exact values, bounds, increments, live animated readouts, and a dense dyadic mode matrix
- Global progressive-draw state that persists across model changes, plus keyboard stepping on every exact numeric entry (`Shift` for 10× and `Alt` for 0.1× increments)
- Global frequency, rotation, axis scale, offset, lift depth, lift frequency, and lift phase controls
- A general parametric curve laboratory, a dedicated 25-mode dyadic Fourier laboratory, and an elliptic-integral laboratory
- Legendre integrals of the first, second, and third kinds in complete and incomplete forms
- Complementary integrals, the AGM identity, elliptic nome, Legendre relation, ellipse perimeter, and nonlinear pendulum period
- Gamma-quarter and Jacobi-theta bridges, including the editable G* reference parameter and complex modular coordinate τ = iK'/K
- Toggleable point, radius-vector, projection, angle, arc-length, Frenet-frame, osculating-circle, integral, gamma, theta, identity, modular, and application layers
- Live differential geometry: position, velocity, speed, arc length, curvature, torsion, and osculating radius
- Synchronized tangent, normal, binormal, and osculating-circle constructions in both views
- Synchronized Canvas 2D and Three.js views with pan, zoom, orbit, target framing, and fit-to-curve
- Parameter and camera keyframes with smooth, linear, or hold interpolation
- Timeline playback, scrubbing, duration, frame-rate, and loop controls
- Optional progressive scene drawing synchronized to timeline position in both 2D and 3D
- Equation overlays and a 1920×1080 composite program monitor
- Live alias-risk diagnostics when active frequencies exceed the current sampling resolution
- Client-side WebM timeline recording
- Versioned project JSON import/export, browser autosave, and undo/redo

## Commands

```powershell
npm run typecheck
npm test
npm run build
```

The app is intentionally independent of `engine/web`. Shared code should be promoted into a separate package only after a second real consumer appears.
