// FTD Ontology Atlas — field-layer renderers.
//
// One renderer per LAYER id (atlas-content.js). Each is a self-contained
// `Layer` object whose `root` Object3D holds all of its geometry; the atlas
// adds every root to the scene once, then toggles `root.visible`.
//
//   REAL layers  render INSIDE the substrate cube  [-1,1]³  (J, s, ∇·J, …)
//   GHOST layers render OUTSIDE it (centre.x > 1)   — desaturated, dashed,
//   translucent — so "not part of the substrate" is literally visible.
//
// Data is static & analytic (atlas-data.js). The only motion is each layer's
// per-frame `update(t)` flourish; the atlas skips it under reduced-motion.
//
// Layer = { id, group, root, anchor, setVisible(bool), update(tSeconds), bounds() }

// ── Scene constants (shared by every renderer) ────────────────────────────
const CHARGES = [
  { pos: { x: -0.4, y: 0, z: 0 }, q: 1 },
  { pos: { x: 0.4, y: 0, z: 0 }, q: -1 },
];
const MASSES = [{ pos: { x: 0, y: -0.25, z: 0 }, m: 0.6 }];

// ── Ontological group colours (must match atlas-content.js GROUPS) ─────────
const COL = {
  full: 0x378add,        // blue   — ontic-real
  manifestation: 0x639922, // green
  derived: 0x1d9e75,     // teal
  imposed: 0x9c9a92,     // grey
  partial: 0xef9f27,     // gold
  epistemic: 0x7f77dd,   // purple
  declined: 0xe24b4a,    // red
};
// Per-layer accent colours called out in the task.
const WARM = 0xe0a030, COOL = 0x50a0e0;
const RED = 0xe24b4a, BLUE = 0x378add, TEAL = 0x1d9e75, GOLD = 0xef9f27, PURPLE = 0x7f77dd, GREY = 0x9c9a92;

const v3 = (THREE, p) => new THREE.Vector3(p.x, p.y, p.z);

// Blue ramp by normalised magnitude m∈[0,1]: dark→bright blue.
function blueRamp(THREE, m) {
  const c = new THREE.Color(COL.full);
  // lerp toward white as |E| grows, but keep it blue-dominant.
  return c.lerp(new THREE.Color(0xcfe6ff), Math.min(1, Math.max(0, m)));
}

// ──────────────────────────────────────────────────────────────────────────
// createLayers — build every Layer and return Map<id, Layer>.
// ──────────────────────────────────────────────────────────────────────────
export function createLayers(THREE, scene, data) {
  const layers = new Map();

  // Generic Layer factory: wires the common surface around a built root.
  function makeLayer(id, group, root, anchor, update) {
    root.name = `layer:${id}`;
    root.visible = false;
    const layer = {
      id,
      group,
      root,
      anchor: anchor || new THREE.Vector3(0, 0, 0),
      setVisible(b) { root.visible = !!b; },
      update: update || (() => {}),
      bounds() { return new THREE.Box3().setFromObject(root); },
    };
    layers.set(id, layer);
    return layer;
  }

  // Helpers shared by several renderers ------------------------------------
  // Collect transparent materials of a root for opacity-pulse updates.
  function collectMats(root) {
    const mats = [];
    root.traverse((o) => { if (o.material) mats.push(o.material); });
    return mats;
  }

  // ════════════════════════════════════════════════════════════════════════
  // REAL LAYERS (inside the cube)
  // ════════════════════════════════════════════════════════════════════════

  // ── J — flux field: ArrowHelper per grid sample, length∝|E|, blue ramp ──
  {
    const root = new THREE.Group();
    const samples = data.sampleGrid(5, (p) => data.fluxFromCharges(CHARGES, p));
    const arrows = [];
    for (const { p, v } of samples) {
      const mag = Math.hypot(v.x, v.y, v.z);
      if (mag < 0.05 || mag > 30) continue;          // skip weak / near-singular
      const dir = new THREE.Vector3(v.x, v.y, v.z).normalize();
      const len = Math.min(0.22, Math.max(0.05, mag * 0.4));
      const m = Math.min(1, Math.log10(1 + mag) / 1.2);
      const color = blueRamp(THREE, m);
      const a = new THREE.ArrowHelper(dir, v3(THREE, p), len, color.getHex(), len * 0.45, len * 0.28);
      a.line.material.transparent = true;
      a.cone.material.transparent = true;
      arrows.push(a);
      root.add(a);
    }
    // gentle opacity/flow pulse
    const update = (t) => {
      const o = 0.7 + 0.3 * Math.sin(t * 2.2);
      for (const a of arrows) { a.line.material.opacity = o; a.cone.material.opacity = o; }
    };
    makeLayer('J', 'full', root, v3(THREE, CHARGES[0].pos), update);
  }

  // ── (q,p) clock — torus ring + rotating hand ──────────────────────────
  {
    const root = new THREE.Group();
    root.position.set(0.7, 0.7, 0.5);              // tucked in a corner of the cell
    const ringGeo = new THREE.TorusGeometry(0.18, 0.012, 12, 48);
    const ring = new THREE.Mesh(ringGeo, new THREE.MeshBasicMaterial({ color: BLUE, transparent: true, opacity: 0.85 }));
    root.add(ring);
    const handGeo = new THREE.BoxGeometry(0.16, 0.012, 0.012);
    const hand = new THREE.Mesh(handGeo, new THREE.MeshBasicMaterial({ color: 0xcfe6ff }));
    hand.position.x = 0.08;                          // pivot at centre
    const handPivot = new THREE.Group();
    handPivot.add(hand);
    root.add(handPivot);
    const update = (t) => { handPivot.rotation.z = t * 1.2; };  // demo ω ≈ 1.2 rad/s
    makeLayer('qpClock', 'full', root, root.position.clone(), update);
  }

  // ── s — state spheres where stateFromDiv(divFlux)≠0; warm(+1)/cool(−1) ─
  {
    const root = new THREE.Group();
    const samples = data.sampleGrid(5, (p) => p);
    const sph = new THREE.SphereGeometry(0.05, 14, 12);
    const meshes = [];
    for (const { p } of samples) {
      const st = data.stateFromDiv(data.divFlux(CHARGES, p));
      if (st === 0) continue;
      const color = st > 0 ? WARM : COOL;
      const m = new THREE.Mesh(sph, new THREE.MeshStandardMaterial({
        color, emissive: color, emissiveIntensity: 0.5, roughness: 0.4, metalness: 0.1,
      }));
      m.position.copy(v3(THREE, p));
      meshes.push(m);
      root.add(m);
    }
    const update = (t) => {
      const s = 1 + 0.12 * Math.sin(t * 3.0);       // subtle scale pop
      for (const m of meshes) m.scale.setScalar(s);
    };
    const anchor = meshes.length ? meshes[0].position.clone() : new THREE.Vector3();
    makeLayer('s', 'manifestation', root, anchor, update);
  }

  // ── filter — translucent gate membrane near manifestation region ───────
  {
    const root = new THREE.Group();
    const disk = new THREE.Mesh(
      new THREE.CircleGeometry(0.55, 40),
      new THREE.MeshBasicMaterial({ color: COL.manifestation, transparent: true, opacity: 0.18, side: THREE.DoubleSide, depthWrite: false }),
    );
    disk.rotation.y = Math.PI / 2;                  // face along x (the projection axis)
    root.add(disk);
    const ring = new THREE.Mesh(
      new THREE.TorusGeometry(0.55, 0.008, 10, 48),
      new THREE.MeshBasicMaterial({ color: COL.manifestation, transparent: true, opacity: 0.6 }),
    );
    ring.rotation.y = Math.PI / 2;
    root.add(ring);
    // a thin "sweep" bar that slides across the disk (projection sweep)
    const sweep = new THREE.Mesh(
      new THREE.PlaneGeometry(0.04, 1.1),
      new THREE.MeshBasicMaterial({ color: 0xcfeec0, transparent: true, opacity: 0.5, side: THREE.DoubleSide, depthWrite: false }),
    );
    sweep.rotation.y = Math.PI / 2;
    root.add(sweep);
    const update = (t) => { sweep.position.y = 0.5 * Math.sin(t * 1.6); };
    makeLayer('filter', 'manifestation', root, new THREE.Vector3(0, 0, 0), update);
  }

  // ── ∇·J — glow points at the two charges, red(+q)/blue(−q) ─────────────
  {
    const root = new THREE.Group();
    const glow = (pos, color) => {
      const m = new THREE.Mesh(
        new THREE.SphereGeometry(0.11, 18, 14),
        new THREE.MeshBasicMaterial({ color, transparent: true, opacity: 0.55, depthWrite: false }),
      );
      m.position.copy(v3(THREE, pos));
      return m;
    };
    const gPos = glow(CHARGES[0].pos, RED);          // +q
    const gNeg = glow(CHARGES[1].pos, BLUE);         // −q
    root.add(gPos, gNeg);
    const update = (t) => {
      const o = 0.4 + 0.3 * (0.5 + 0.5 * Math.sin(t * 2.6));
      gPos.material.opacity = o; gNeg.material.opacity = o;
    };
    makeLayer('divJ', 'derived', root, v3(THREE, CHARGES[0].pos), update);
  }

  // ── ψ⊥ — translucent disk at origin; hue cycles (arg ψ rotation) ───────
  {
    const root = new THREE.Group();
    const mat = new THREE.MeshBasicMaterial({ color: 0x1d9e75, transparent: true, opacity: 0.12, side: THREE.DoubleSide, depthWrite: false });
    const disk = new THREE.Mesh(new THREE.CircleGeometry(0.6, 48), mat);
    // disk lies in the x-y plane (the transverse J_x+iJ_y plane); default normal +z is fine.
    root.add(disk);
    const rim = new THREE.Mesh(
      new THREE.TorusGeometry(0.6, 0.008, 10, 64),
      new THREE.MeshBasicMaterial({ color: 0x1d9e75, transparent: true, opacity: 0.7 }),
    );
    root.add(rim);
    const update = (t) => {
      const h = 0.5 + 0.08 * Math.sin(t * 0.5);      // gentle teal↔cyan drift = arg ψ winding
      mat.color.setHSL(h, 0.5, 0.5);
    };
    makeLayer('psi', 'derived', root, new THREE.Vector3(0, 0, 0), update);
  }

  // ── L — latency well: subdivided plane displaced DOWN + a clock ring ────
  {
    const root = new THREE.Group();
    const N = 24, size = 2.2;
    const geo = new THREE.PlaneGeometry(size, size, N, N);
    geo.rotateX(-Math.PI / 2);                       // lie flat in x-z
    const pos = geo.attributes.position;
    for (let i = 0; i < pos.count; i++) {
      const x = pos.getX(i), z = pos.getZ(i);
      const w = data.latencyWell(MASSES, { x, y: 0, z });
      pos.setY(i, -0.25 - w * 0.9);                  // dip downward, below the cube
    }
    geo.computeVertexNormals();
    const sheet = new THREE.Mesh(geo, new THREE.MeshBasicMaterial({ color: TEAL, wireframe: true, transparent: true, opacity: 0.5 }));
    root.add(sheet);
    // a small clock ring near the well whose pulse period scales with √f.
    const clock = new THREE.Mesh(
      new THREE.TorusGeometry(0.14, 0.01, 10, 40),
      new THREE.MeshBasicMaterial({ color: TEAL, transparent: true, opacity: 0.8 }),
    );
    const wellDepth = data.latencyWell(MASSES, MASSES[0].pos);
    const f = Math.max(0.05, 1 - wellDepth * wellDepth);  // lapse f = 1 − L²
    clock.position.set(0.6, -0.4, 0.6);
    root.add(clock);
    const update = (t) => {
      // dilation ripple: scale pulse on the clock at rate √f (slower in the well)
      const rate = 2.0 * Math.sqrt(f);
      clock.scale.setScalar(1 + 0.18 * Math.sin(t * rate));
      sheet.material.opacity = 0.4 + 0.15 * (0.5 + 0.5 * Math.sin(t * 1.2));
    };
    makeLayer('latency', 'derived', root, v3(THREE, MASSES[0].pos), update);
  }

  // ── ∥⊥ split — grey double-arrow along x (∥) + teal ring (⊥) ───────────
  {
    const root = new THREE.Group();
    const par1 = new THREE.ArrowHelper(new THREE.Vector3(1, 0, 0), new THREE.Vector3(0, 0, 0), 0.7, GREY, 0.16, 0.1);
    const par2 = new THREE.ArrowHelper(new THREE.Vector3(-1, 0, 0), new THREE.Vector3(0, 0, 0), 0.7, GREY, 0.16, 0.1);
    root.add(par1, par2);
    const ring = new THREE.Mesh(
      new THREE.TorusGeometry(0.45, 0.012, 12, 56),
      new THREE.MeshBasicMaterial({ color: TEAL, transparent: true, opacity: 0.75 }),
    );
    ring.rotation.y = Math.PI / 2;                   // ⊥ ring stands in the y-z plane
    root.add(ring);
    const mats = collectMats(root);
    const update = (t) => {
      const o = 0.6 + 0.25 * Math.sin(t * 1.8);
      for (const m of mats) { if (m.transparent) m.opacity = o; }
    };
    makeLayer('split', 'derived', root, new THREE.Vector3(0, 0, 0), update);
  }

  // ── ∇×J forces — two arrows on the charges pointing toward each other ──
  {
    const root = new THREE.Group();
    // +q at x=−0.4 pulled toward −q at x=+0.4 → arrow points +x; and vice-versa.
    const fA = new THREE.ArrowHelper(new THREE.Vector3(1, 0, 0), v3(THREE, CHARGES[0].pos), 0.3, TEAL, 0.1, 0.07);
    const fB = new THREE.ArrowHelper(new THREE.Vector3(-1, 0, 0), v3(THREE, CHARGES[1].pos), 0.3, TEAL, 0.1, 0.07);
    root.add(fA, fB);
    const update = (t) => {
      const s = 1 + 0.2 * Math.sin(t * 2.4);
      fA.setLength(0.3 * s, 0.1 * s, 0.07 * s);
      fB.setLength(0.3 * s, 0.1 * s, 0.07 * s);
    };
    makeLayer('curlForces', 'derived', root, v3(THREE, CHARGES[0].pos), update);
  }

  // ── L/R dual substrate — two interlocked tetra wireframes, offset ±x ───
  {
    const root = new THREE.Group();
    const tetA = [[1, 1, 1], [1, -1, -1], [-1, 1, -1], [-1, -1, 1]];          // stella corner set
    const tetB = [[-1, -1, -1], [-1, 1, 1], [1, -1, 1], [1, 1, -1]];          // its complement
    // tetra edge list (all 6 pairs of the 4 corners).
    const edges = [[0, 1], [0, 2], [0, 3], [1, 2], [1, 3], [2, 3]];
    const makeTet = (pts, color, dx) => {
      const verts = [];
      for (const [a, b] of edges) {
        verts.push(pts[a][0] * 0.5 + dx, pts[a][1] * 0.5, pts[a][2] * 0.5);
        verts.push(pts[b][0] * 0.5 + dx, pts[b][1] * 0.5, pts[b][2] * 0.5);
      }
      const g = new THREE.BufferGeometry();
      g.setAttribute('position', new THREE.Float32BufferAttribute(verts, 3));
      return new THREE.LineSegments(g, new THREE.LineBasicMaterial({ color, transparent: true, opacity: 0.7 }));
    };
    const L = makeTet(tetA, 0xb39a5e, +0.04);        // L tint, +x offset
    const R = makeTet(tetB, 0x5e9ab3, -0.04);        // R tint, −x offset
    root.add(L, R);
    const update = (t) => {
      const o = 0.55 + 0.2 * Math.sin(t * 1.5);
      L.material.opacity = o; R.material.opacity = o;
    };
    makeLayer('lr', 'imposed', root, new THREE.Vector3(0, 0, 0), update);
  }

  // ════════════════════════════════════════════════════════════════════════
  // GHOST LAYERS (outside the cube — centre.x > 1, desaturated/dashed)
  // ════════════════════════════════════════════════════════════════════════

  // ── observer — wireframe frustum at ~(2.3,−0.3,0), gold, with blind notch ─
  {
    const root = new THREE.Group();
    root.position.set(2.3, -0.3, 0);
    // a truncated pyramid (frustum): small near face, large far face, pointing −x toward the cube.
    const near = 0.18, far = 0.5, depth = 0.7;
    const f = [
      // near face (toward cube, −x)
      [-depth / 2, -near, -near], [-depth / 2, near, -near], [-depth / 2, near, near], [-depth / 2, -near, near],
      // far face (+x)
      [depth / 2, -far, -far], [depth / 2, far, -far], [depth / 2, far, far], [depth / 2, -far, far],
    ];
    const E = [
      [0, 1], [1, 2], [2, 3], [3, 0],   // near
      [4, 5], [5, 6], [6, 7], [7, 4],   // far
      [0, 4], [1, 5], [2, 6], [3, 7],   // struts
    ];
    const verts = [];
    for (const [a, b] of E) { verts.push(...f[a], ...f[b]); }
    const g = new THREE.BufferGeometry();
    g.setAttribute('position', new THREE.Float32BufferAttribute(verts, 3));
    const frustum = new THREE.LineSegments(g, new THREE.LineBasicMaterial({ color: GOLD, transparent: true, opacity: 0.5 }));
    root.add(frustum);
    // dark "blind-spot" notch: a small black quad on the near face.
    const notch = new THREE.Mesh(
      new THREE.PlaneGeometry(0.14, 0.14),
      new THREE.MeshBasicMaterial({ color: 0x101218, transparent: true, opacity: 0.85, side: THREE.DoubleSide, depthWrite: false }),
    );
    notch.position.set(-depth / 2 - 0.001, 0, 0);
    notch.rotation.y = Math.PI / 2;
    root.add(notch);
    const update = (t) => { frustum.material.opacity = 0.4 + 0.15 * Math.sin(t * 1.3); };
    makeLayer('observer', 'partial', root, root.position.clone(), update);
  }

  // ── readoff — small arrow/marker at ~(1.7,0.5,0), epistemic purple ─────
  {
    const root = new THREE.Group();
    root.position.set(1.7, 0.5, 0);
    // an arrow pointing outward (cube → ghost, i.e. +x) marking the read-off.
    const arrow = new THREE.ArrowHelper(new THREE.Vector3(1, 0, 0), new THREE.Vector3(-0.25, 0, 0), 0.5, PURPLE, 0.14, 0.09);
    arrow.line.material.transparent = true; arrow.cone.material.transparent = true;
    root.add(arrow);
    const update = (t) => {
      const o = 0.5 + 0.4 * (0.5 + 0.5 * Math.sin(t * 2.0));
      arrow.line.material.opacity = o; arrow.cone.material.opacity = o;
    };
    makeLayer('readoff', 'epistemic', root, root.position.clone(), update);
  }

  // ── psiWave — translucent diffuse point-cloud blob at ~(2.4,1.1,0) + dashed box ─
  {
    const root = new THREE.Group();
    root.position.set(2.4, 1.1, 0);
    // diffuse desaturated-purple point cloud (a fuzzy probability blob).
    const pts = [];
    const NP = 260;
    let seed = 1234567;
    const rnd = () => { seed = (seed * 1103515245 + 12345) & 0x7fffffff; return seed / 0x7fffffff; };
    for (let i = 0; i < NP; i++) {
      // gaussian-ish via sum of uniforms, scaled to a small ellipsoid.
      const gx = (rnd() + rnd() + rnd() - 1.5) * 0.32;
      const gy = (rnd() + rnd() + rnd() - 1.5) * 0.32;
      const gz = (rnd() + rnd() + rnd() - 1.5) * 0.32;
      pts.push(gx, gy, gz);
    }
    const cloudGeo = new THREE.BufferGeometry();
    cloudGeo.setAttribute('position', new THREE.Float32BufferAttribute(pts, 3));
    const cloud = new THREE.Points(cloudGeo, new THREE.PointsMaterial({
      color: 0x9a93d8, size: 0.04, transparent: true, opacity: 0.5, depthWrite: false,
    }));
    root.add(cloud);
    // dashed bounding box: LineSegments of EdgesGeometry(BoxGeometry).
    const boxEdges = new THREE.EdgesGeometry(new THREE.BoxGeometry(0.9, 0.9, 0.9));
    const box = new THREE.LineSegments(boxEdges, new THREE.LineDashedMaterial({
      color: PURPLE, transparent: true, opacity: 0.55, dashSize: 0.08, gapSize: 0.05,
    }));
    box.computeLineDistances();                       // required for dashing
    root.add(box);
    const update = (t) => {
      cloud.rotation.y = t * 0.2;                     // slow shimmer
      cloud.material.opacity = 0.35 + 0.2 * (0.5 + 0.5 * Math.sin(t * 1.1));
    };
    makeLayer('psiWave', 'epistemic', root, root.position.clone(), update);
  }

  // ── M — struck "M" sprite at ~(2.5,0.4,0), red with ✗ — INERT (no update) ─
  {
    const root = new THREE.Group();
    root.position.set(2.5, 0.4, 0);
    const tex = makeStruckMTexture(THREE);
    if (tex) {
      const sprite = new THREE.Sprite(new THREE.SpriteMaterial({ map: tex, transparent: true, opacity: 0.85, depthWrite: false }));
      sprite.scale.set(0.6, 0.6, 1);
      root.add(sprite);
    } else {
      // fallback if 2d canvas is unavailable: a red plane with an X of lines.
      const plane = new THREE.Mesh(
        new THREE.PlaneGeometry(0.5, 0.5),
        new THREE.MeshBasicMaterial({ color: RED, transparent: true, opacity: 0.3, side: THREE.DoubleSide, depthWrite: false }),
      );
      root.add(plane);
      const xVerts = [-0.25, -0.25, 0, 0.25, 0.25, 0, -0.25, 0.25, 0, 0.25, -0.25, 0];
      const xg = new THREE.BufferGeometry();
      xg.setAttribute('position', new THREE.Float32BufferAttribute(xVerts, 3));
      root.add(new THREE.LineSegments(xg, new THREE.LineBasicMaterial({ color: RED })));
    }
    // INERT — declined. No update function (default no-op).
    makeLayer('M', 'declined', root, root.position.clone(), null);
  }

  return layers;
}

// Draw a red "M" with a strike-through ✗ onto a canvas → THREE.CanvasTexture.
// Returns null if a 2D canvas context is unavailable (defensive).
function makeStruckMTexture(THREE) {
  if (typeof document === 'undefined') return null;
  const c = document.createElement('canvas');
  c.width = 128; c.height = 128;
  const ctx = c.getContext('2d');
  if (!ctx) return null;
  ctx.clearRect(0, 0, 128, 128);
  // the M
  ctx.fillStyle = '#e24b4a';
  ctx.font = 'bold 96px serif';
  ctx.textAlign = 'center';
  ctx.textBaseline = 'middle';
  ctx.fillText('M', 64, 60);
  // the strike ✗ (two diagonals)
  ctx.strokeStyle = '#e24b4a';
  ctx.lineWidth = 8;
  ctx.beginPath();
  ctx.moveTo(18, 18); ctx.lineTo(110, 110);
  ctx.moveTo(110, 18); ctx.lineTo(18, 110);
  ctx.stroke();
  const tex = new THREE.CanvasTexture(c);
  tex.needsUpdate = true;
  return tex;
}
