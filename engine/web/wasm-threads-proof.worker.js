// Phase-1 off-thread proof: host ftd_core_mt in a worker at pool=1 (pure serial,
// no thread spawns). With PTHREAD_POOL_SIZE=0 the module loads with ZERO nested
// workers, so no init stall — and the engine runs off the main thread.
importScripts('./wasm/ftd_core_mt.js');

let m = null, b = null, running = true, tick = 0;
const L = parseInt((self.name || '65'), 10) || 65;
const POOL = parseInt((self.POOL || '1'), 10) || 1;

createFTDModuleMT({ locateFile: (p) => './wasm/' + p }).then((mod) => {
  m = mod;
  if (typeof m.ftdSetPoolThreads === 'function') m.ftdSetPoolThreads(POOL);
  b = new m.RenderBridge(L);
  m.setupScenario(b, 's0-seed-hydrogen');
  const vol = m.getFluxVolume(b);
  const heap = vol.buffer;   // the flux view aliases the WASM heap (SAB under -pthread)
  postMessage({
    type: 'ready', heap,
    isSAB: (typeof SharedArrayBuffer !== 'undefined') && (heap instanceof SharedArrayBuffer),
    fluxPtr: vol.byteOffset, fluxLen: vol.length, N: L,
    pool: (typeof m.ftdPoolThreads === 'function') ? m.ftdPoolThreads() : 'n/a',
  });
  loop();
}).catch((e) => postMessage({ type: 'error', msg: String(e && e.message || e) }));

function loop() {
  if (m && running) {
    const t0 = performance.now();
    b.tick();
    m.getFluxVolume(b);
    const dt = performance.now() - t0;
    tick++;
    postMessage({ type: 'frame', tick, tickMs: dt });
  }
  setTimeout(loop, 0);
}
onmessage = (e) => { if (e.data && e.data.type === 'setRunning') running = !!e.data.value; };
