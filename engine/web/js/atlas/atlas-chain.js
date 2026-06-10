// FTD Ontology Atlas — the causal chain the stepper walks (pure data).
// 14 stages (0–13): the substrate's self-contained loop (flux → manifest →
// project → forces → gravity), then the read-off out to the epistemic ghosts,
// then the declined measurement map. layersOn accumulates as the chain advances.
//
// Each stage: { id, title, layersOn:[layerId|'lattice'...], arrows:[[from,to]...],
//               camera:{az,el,zoom}, contentKey } where contentKey selects the
// detail-panel content (a LAYERS id, or 'lattice' for the opening postulates).

const RAW = [
  { id: 'postulates', title: 'Five postulates',            add: [],                  arrows: [],                                   camera: { az: 35, el: 20, zoom: 1.00 }, contentKey: 'lattice' },
  { id: 'flux',       title: 'Flux field J',               add: ['J'],               arrows: [],                                   camera: { az: 30, el: 15, zoom: 1.10 }, contentKey: 'J' },
  { id: 'clock',      title: 'Wave + (q,p) clock',         add: ['qpClock'],         arrows: [],                                   camera: { az: 20, el: 25, zoom: 1.20 }, contentKey: 'qpClock' },
  { id: 'gauss',      title: '∇·J — Gauss source',         add: ['divJ'],            arrows: [['J', 'divJ']],                      camera: { az: 40, el: 10, zoom: 1.20 }, contentKey: 'divJ' },
  { id: 'filter',     title: 'Existence Filter E=Re',      add: ['filter'],          arrows: [['divJ', 'filter']],                 camera: { az: 45, el: 15, zoom: 1.30 }, contentKey: 'filter' },
  { id: 'manifest',   title: 'Manifest s (genesis ↓)',     add: ['s'],               arrows: [['filter', 's']],                    camera: { az: 35, el: 20, zoom: 1.20 }, contentKey: 's' },
  { id: 'project',    title: 'Gauss projection ↑ + ψ⊥',    add: ['split', 'psi'],    arrows: [['s', 'J'], ['split', 'psi']],       camera: { az: 25, el: 30, zoom: 1.20 }, contentKey: 'psi' },
  { id: 'forces',     title: 'Forces (∇×J)',               add: ['curlForces'],      arrows: [['J', 'curlForces'], ['s', 'curlForces']], camera: { az: 30, el: 15, zoom: 1.10 }, contentKey: 'curlForces' },
  { id: 'movement',   title: 'Movement / clusters',        add: [],                  arrows: [],                                   camera: { az: 50, el: 20, zoom: 1.00 }, contentKey: 's' },
  { id: 'latency',    title: 'Latency L + clock √f',       add: ['latency'],         arrows: [['curlForces', 'latency']],          camera: { az: 20, el: 35, zoom: 1.10 }, contentKey: 'latency' },
  { id: 'readoff',    title: 'Statistical read-off',       add: ['readoff'],         arrows: [['psi', 'readoff']],                 camera: { az: 15, el: 20, zoom: 1.00 }, contentKey: 'readoff' },
  { id: 'psiwave',    title: 'Wavefunction Ψ',             add: ['psiWave'],         arrows: [['readoff', 'psiWave']],             camera: { az: 10, el: 15, zoom: 0.95 }, contentKey: 'psiWave' },
  { id: 'observer',   title: 'Observer’s partial view',    add: ['observer'],        arrows: [],                                   camera: { az: 5,  el: 10, zoom: 0.95 }, contentKey: 'observer' },
  { id: 'declined',   title: 'M — declined (FC-1)',        add: ['M'],               arrows: [],                                   camera: { az: 0,  el: 10, zoom: 0.90 }, contentKey: 'M' },
];

let acc = ['lattice'];
export const STAGES = RAW.map((r) => {
  acc = acc.concat(r.add);
  return { id: r.id, title: r.title, layersOn: [...acc], arrows: r.arrows, camera: r.camera, contentKey: r.contentKey };
});

export const STAGE_COUNT = STAGES.length;
