// FTD Ontology Atlas — animated SVG chain overlay.
//
// Draws the chain's flow-arrows as 2D SVG on top of the WebGL canvas. Each
// arrow is a gentle quadratic curve between two 3D anchor points, projected to
// screen space every frame via scene.worldToScreen, with a coloured arrowhead
// (marker-end) and a flowing dashed stroke (animated stroke-dashoffset).
//
//   createOverlay(svgEl, scene) → { setArrows(list), tick(), clear() }
//   list = [ [fromVec3, toVec3, colorHex], ... ]
//
// DOM nodes are reused, keyed by arrow index; the node count grows/shrinks as
// the arrow list length changes. The svg viewBox + size track the canvas CSS
// size each tick so screen coords (canvas px) overlay 1:1.

const SVG_NS = 'http://www.w3.org/2000/svg';

export function createOverlay(svgEl, scene) {
  // One <defs> holds the arrowhead markers; one <marker> per colour, memoised.
  const defs = document.createElementNS(SVG_NS, 'defs');
  svgEl.appendChild(defs);
  const markers = new Map();        // colorHex → marker id
  let markerSeq = 0;

  // The arrow <path> nodes, indexed parallel to the arrow list.
  const paths = [];                 // SVG <path> elements
  let arrows = [];                  // current [from, to, color] list

  let dashOffset = 0;               // advanced each tick → flowing dashes
  let lastW = -1, lastH = -1;       // cached size to avoid redundant attr sets

  // Sanitise a colour into a stable, attribute-safe marker-id fragment.
  function colorKey(hex) {
    return String(hex).replace(/[^a-zA-Z0-9]/g, '') || 'def';
  }

  // Get (or lazily build) an arrowhead marker tinted to `color`; returns its id.
  function markerFor(color) {
    const key = colorKey(color);
    let id = markers.get(key);
    if (id) return id;
    id = `atlas-arrowhead-${markerSeq++}-${key}`;
    const marker = document.createElementNS(SVG_NS, 'marker');
    marker.setAttribute('id', id);
    marker.setAttribute('viewBox', '0 0 10 10');
    marker.setAttribute('refX', '9');
    marker.setAttribute('refY', '5');
    marker.setAttribute('markerWidth', '7');
    marker.setAttribute('markerHeight', '7');
    marker.setAttribute('markerUnits', 'userSpaceOnUse');
    marker.setAttribute('orient', 'auto-start-reverse');
    const tip = document.createElementNS(SVG_NS, 'path');
    tip.setAttribute('d', 'M0,0 L10,5 L0,10 L3,5 Z');
    tip.setAttribute('fill', color);
    marker.appendChild(tip);
    defs.appendChild(marker);
    markers.set(key, id);
    return id;
  }

  // Replace the chain arrow list. Grows/shrinks the pool of <path> nodes to
  // match; tick() does the per-frame projection + animation.
  function setArrows(list) {
    arrows = Array.isArray(list) ? list : [];
    // Grow: create new <path> nodes for any added arrows.
    while (paths.length < arrows.length) {
      const p = document.createElementNS(SVG_NS, 'path');
      p.setAttribute('fill', 'none');
      p.setAttribute('stroke-width', '2');
      p.setAttribute('stroke-linecap', 'round');
      p.setAttribute('stroke-dasharray', '7 6');
      svgEl.appendChild(p);
      paths.push(p);
    }
    // Shrink: remove surplus <path> nodes.
    while (paths.length > arrows.length) {
      const p = paths.pop();
      p.remove();
    }
  }

  // Per-frame: size the svg to the canvas, project endpoints, update paths.
  function tick() {
    // Match the svg box to the canvas CSS size so coords overlay 1:1.
    const w = Math.max(1, Math.round(svgEl.clientWidth));
    const h = Math.max(1, Math.round(svgEl.clientHeight));
    if (w !== lastW || h !== lastH) {
      svgEl.setAttribute('viewBox', `0 0 ${w} ${h}`);
      svgEl.setAttribute('width', String(w));
      svgEl.setAttribute('height', String(h));
      lastW = w; lastH = h;
    }

    dashOffset = (dashOffset - 0.9);          // negative → flow from→to
    if (dashOffset < -1e6) dashOffset = 0;    // guard against unbounded drift

    for (let i = 0; i < arrows.length; i++) {
      const path = paths[i];
      const [from, to, color] = arrows[i];
      const a = scene.worldToScreen(from);
      const b = scene.worldToScreen(to);

      // Hide if either endpoint is behind the camera / off the near plane.
      if (!a.visible || !b.visible) { path.setAttribute('visibility', 'hidden'); continue; }
      path.setAttribute('visibility', 'visible');

      // Gentle quadratic bow: control point at the midpoint, nudged
      // perpendicular to the chord so arrows arc rather than run straight.
      const mx = (a.x + b.x) / 2, my = (a.y + b.y) / 2;
      const dx = b.x - a.x, dy = b.y - a.y;
      const dist = Math.hypot(dx, dy) || 1;
      const bow = Math.min(40, dist * 0.18);   // curvature grows gently with length
      const cx = mx + (-dy / dist) * bow;
      const cy = my + (dx / dist) * bow;

      path.setAttribute('d', `M ${a.x.toFixed(1)} ${a.y.toFixed(1)} Q ${cx.toFixed(1)} ${cy.toFixed(1)} ${b.x.toFixed(1)} ${b.y.toFixed(1)}`);
      path.setAttribute('stroke', color);
      path.setAttribute('marker-end', `url(#${markerFor(color)})`);
      path.setAttribute('stroke-dashoffset', dashOffset.toFixed(1));
    }
  }

  // Tear down every arrow node (markers/defs persist — cheap and reusable).
  function clear() {
    while (paths.length) { const p = paths.pop(); p.remove(); }
    arrows = [];
  }

  return { setArrows, tick, clear };
}
