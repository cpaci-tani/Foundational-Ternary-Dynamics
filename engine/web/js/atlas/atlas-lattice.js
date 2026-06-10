// The Moore-neighbourhood lattice — the substrate "stage".
// octahedron (6 face neighbours) + cuboctahedron (12 edge) + stella octangula
// (8 corner = two interlocked tetrahedra), plus a faint 5³ tiling.
// Returns a Group whose named children are individually .visible-toggleable.
import { ConvexGeometry } from 'three/addons/geometries/ConvexGeometry.js';

export function createLattice(THREE) {
  const group = new THREE.Group();
  group.name = 'lattice';

  const wire = (geo, color, opacity) =>
    new THREE.LineSegments(
      new THREE.EdgesGeometry(geo, 1),
      new THREE.LineBasicMaterial({ color, transparent: true, opacity, depthWrite: false }),
    );
  const hull = (pts) => new ConvexGeometry(pts.map((p) => new THREE.Vector3(p[0], p[1], p[2])));

  // Octahedron — 6 face neighbours (±1 on each axis).
  const octo = wire(new THREE.OctahedronGeometry(1, 0), 0x7c8696, 0.55);
  octo.name = 'octahedron';
  group.add(octo);

  // Cuboctahedron — 12 edge neighbours: permutations of (±1, ±1, 0).
  const cuboctPts = [];
  for (const [a, b] of [[1, 1], [1, -1], [-1, 1], [-1, -1]]) {
    cuboctPts.push([a, b, 0], [a, 0, b], [0, a, b]);
  }
  const cubo = wire(hull(cuboctPts), 0x5d6675, 0.34);
  cubo.name = 'cuboctahedron';
  group.add(cubo);

  // Stella octangula — 8 corner neighbours = two interlocked tetrahedra.
  const tetA = [[1, 1, 1], [1, -1, -1], [-1, 1, -1], [-1, -1, 1]];
  const tetB = [[-1, -1, -1], [-1, 1, 1], [1, -1, 1], [1, 1, -1]];
  const stella = new THREE.Group();
  stella.name = 'stella';
  const sA = wire(hull(tetA), 0xb39a5e, 0.5); sA.name = 'stellaA';
  const sB = wire(hull(tetB), 0x5e9ab3, 0.5); sB.name = 'stellaB';
  stella.add(sA, sB);
  stella.visible = false;
  group.add(stella);

  // Faint 5³ tiling for "lattice" context.
  const tilePts = [];
  for (let i = -2; i <= 2; i++) for (let j = -2; j <= 2; j++) for (let k = -2; k <= 2; k++) tilePts.push(i, j, k);
  const tgeo = new THREE.BufferGeometry();
  tgeo.setAttribute('position', new THREE.Float32BufferAttribute(tilePts, 3));
  const tiling = new THREE.Points(
    tgeo,
    new THREE.PointsMaterial({ color: 0x3a4150, size: 0.035, transparent: true, opacity: 0.5, depthWrite: false }),
  );
  tiling.name = 'tiling';
  tiling.visible = false;
  group.add(tiling);

  return group;
}
