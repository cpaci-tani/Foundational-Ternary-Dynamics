# FTD Web Dashboard — Smoke Suite

Playwright-based smoke suite for `engine/web/index.html`. Boots a real
Chromium against the dashboard served from `python -m http.server` and
verifies scale switching, listener cleanup, and a few known regressions.

## Running

From this directory (`engine/web/tests/`):

```bash
# one-time setup
npm install
npm run install:browsers

# run the full suite
npm test

# watch UI mode (for debugging a specific test)
npm run test:ui

# run a single test by name
npx playwright test -g "Scale 11 reference frame context"
```

The config uses Playwright's `webServer` block to start / stop
`python -m http.server 8081` automatically, so you do not need to run
the dev server separately. Port **8081** is chosen so it does not
collide with the 8080 that many developers leave running manually.

## What the suite covers

- **Scale sweep**: each of the 8 engine modes (`lattice`, `particles`,
  `atoms`, `molecules`, `planetary`, `cosmic`, `meta`, `reference frame context`)
  loads without console errors or failed network requests.
- **Bridge initialization**: `window._ftdBridge` becomes non-null within
  15 s of page load.
- **Phase B.1 regression**: `window._cosmicInterval` is never set after
  the Scale 5 refactor (cosmic mode now drives physics from the rAF
  loop instead of a parallel `setInterval`).
- **Phase B.2 regression**: re-entering reference frame context mode does not leak
  event listeners. A warmup cycle runs first so that one-time init
  listeners land on the untracked baseline, then 5 further cycles are
  monitored for growth.
- **Constants contract**: `constants.js` still exports `K_B`, `ALPHA`,
  `G_STAR` as named values and `K_B === 0.511`.

## What the suite does NOT cover

- Visual regression. Three.js + GPU output is nondeterministic across
  drivers; screenshot diffing is a trap.
- Cross-browser. We use ES module importmaps and Three.js; Chromium
  only is sufficient for the dashboard.
- Physics correctness. That lives in the C++ CTest and Python pytest
  suites in `engine/tests/` and `scripts/tests/`.

## Adding tests

Put new `*.spec.js` files in this directory. The config's `testMatch`
picks them up automatically. Prefer short, focused tests over large
end-to-end flows; each test re-navigates the page anyway and the
boot cost is ~3 s.
