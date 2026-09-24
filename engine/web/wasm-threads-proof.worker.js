// Off-main-thread proof: host the serial core in one dedicated Web Worker.
// The pthread artifact has a compile-time worker pool and therefore cannot be
// converted to pool=0 after initialization; attempting that deadlocks on its
// `loading-workers` run dependency. The serial artifact is the correct owner
// for a one-worker/zero-nested-worker proof.
postMessage({ type: 'boot', stage: 'loading serial WASM core' });
importScripts('./wasm/ftd_core.js');

let m = null, b = null, running = true, tick = 0;
const L = parseInt((self.name || '33'), 10) || 33;

createFTDModule({ locateFile: (p) => './wasm/' + p }).then((mod) => {
  m = mod;
  b = new m.RenderBridge(L);
  m.setupScenario(b, 's0-seed-hydrogen');
  postMessage({
    type: 'ready',
    isSAB: false,
    N: L,
    pool: 0,
  });
  loop();
}).catch((e) => postMessage({ type: 'error', msg: String(e && e.message || e) }));

function loop() {
  if (m && running) {
    const t0 = performance.now();
    b.tick();
    const vol = m.getFluxVolume(b);
    let maxFlux = 0;
    for (let i = 0; i < vol.length; i += 64) if (vol[i] > maxFlux) maxFlux = vol[i];
    const dt = performance.now() - t0;
    tick++;
    postMessage({ type: 'frame', tick, tickMs: dt, maxFlux });
  }
  setTimeout(loop, 0);
}
onmessage = (e) => { if (e.data && e.data.type === 'setRunning') running = !!e.data.value; };
