// ── meta-unit.js ── 3x3x3 existential unit lattice visualization ──
// Renders the 27-site Moore neighborhood as togglable geometric layers:
// shells (center, octahedron, cuboctahedron, cube), wireframe polyhedra,
// parity coloring, symmetry elements, framework labels, and auto-rotation.

import * as THREE from 'three';
import {
    makeInstancedShell,
    makeWireframe,
    makeAxisLine,
    makeMirrorPlane,
    edgesAtDistance,
} from './meta-unit-geometry.js';

const SCALE = 1.5;

// Reused scratch colour for per-instance recolouring (avoids per-call alloc).
const _tmpRecolor = new THREE.Color();

const COLORS = {
    center:         0xFFD700,
    octahedron:     0x00CED1,
    cuboctahedron:  0xFF00FF,
    cube:           0x7FFF00,
    evenParity:     0x4488FF,
    oddParity:      0xFF4444,
    orbitRep:       0x44CC44,
    antipode:       0xFF8800,
    axis_c4:        0xFFFF00,
    axis_c3:        0xFF6600,
    axis_c2:        0x00AAFF,
    mirror:         0xFFFFFF,
    connection:     0x888888,
};

// All 27 sites in {-1,0,1}^3
function buildSiteData() {
    const sites = [];
    for (let x = -1; x <= 1; x++) {
        for (let y = -1; y <= 1; y++) {
            for (let z = -1; z <= 1; z++) {
                const d2 = x * x + y * y + z * z;
                const distance = Math.sqrt(d2);
                let shell, radius, color;
                if (d2 === 0) {
                    shell = 'center';
                    radius = 0.15;
                    color = COLORS.center;
                } else if (d2 === 1) {
                    shell = 'octahedron';
                    radius = 0.10;
                    color = COLORS.octahedron;
                } else if (d2 === 2) {
                    shell = 'cuboctahedron';
                    radius = 0.08;
                    color = COLORS.cuboctahedron;
                } else {
                    shell = 'cube';
                    radius = 0.08;
                    color = COLORS.cube;
                }
                const parity = ((Math.abs(x) + Math.abs(y) + Math.abs(z)) % 2 === 0) ? 'even' : 'odd';
                const inversionParity = (d2 > 0) ? computeInversionParity(x, y, z) : null;
                const stabilizer = computeStabilizer(x, y, z);
                const irrep = computeIrrep(shell);
                sites.push({
                    x, y, z, d2, distance, shell, radius, color,
                    parity, inversionParity, stabilizer, irrep
                });
            }
        }
    }
    return sites;
}

function computeInversionParity(x, y, z) {
    // Sites come in inversion pairs (x,y,z) <-> (-x,-y,-z). This function
    // partitions each non-center pair into an "orbit representative"
    // ('orbit_rep', the half with first nonzero coord positive) and its
    // "antipode" — i.e. a fundamental domain of the inversion operator.
    //
    // NOTE (audit P1-7, corrected 2026-05-27): the labels were formerly
    // 'gerade'/'ungerade'. That was wrong. Gerade/ungerade is the parity
    // of an IRREP under inversion (a property of a basis function — e.g.
    // the center spans A_1g, the octahedron spans T_1u; see the Moore
    // Layer Theorem §3 / §8 shell->irrep map), NOT a property of an
    // individual site. Each shell mixes g and u (the cube carries A_2u
    // AND T_1u), so a per-site g/u label is meaningless. What the
    // heuristic actually computes is an inversion fundamental domain:
    // one representative per antipodal site-pair. The 13+13 visual split
    // is the count of that fundamental domain, which is the genuine
    // content. Renamed to 'orbit_rep'/'antipode' accordingly.
    if (x > 0) return 'orbit_rep';
    if (x < 0) return 'antipode';
    if (y > 0) return 'orbit_rep';
    if (y < 0) return 'antipode';
    if (z > 0) return 'orbit_rep';
    if (z < 0) return 'antipode';
    return 'orbit_rep';
}

function computeStabilizer(x, y, z) {
    const d2 = x * x + y * y + z * z;
    if (d2 === 0) return 'O_h (full octahedral)';
    if (d2 === 1) return 'C_4v';
    if (d2 === 2) return 'C_2v';
    return 'C_3v';
}

function computeIrrep(shell) {
    if (shell === 'center') return 'A_1g';
    if (shell === 'octahedron') return 'T_1u';
    if (shell === 'cuboctahedron') return 'T_2g + E_g';
    return 'A_2u + T_1u';
}

export class MetaUnit {
    constructor(scene, camera, renderer) {
        this._scene = scene;
        this._camera = camera;
        this._renderer = renderer;

        this._root = new THREE.Group();
        this._root.name = 'MetaUnit';
        this._scene.add(this._root);

        this._sites = buildSiteData();
        this._meshes = [];          // the 4 per-shell InstancedMeshes
        this._shellMeshes = {};     // shell name -> InstancedMesh
        this._siteRefs = [];        // parallel to this._sites: { mesh, instanceId }
        this._originalColors = [];

        this._groups = {};
        this._labelContainer = null;
        this._labelElements = [];
        this._autoRotate = false;
        this._rotationSpeed = 0.15;

        this._buildShellGroups();
        this._buildWireframes();
        this._buildConnections();
        this._buildSymmetryElements();
        this._buildFrameworkLabels();
    }

    // ── Shell groups ────────────────────────────────────────────────

    _buildShellGroups() {
        // Shell → canonical sublattice mapping per Moore Layer Theorem §4:
        //   center           → central voxel
        //   octahedron (k=1) → SC (simple cubic) sublattice
        //   cuboctahedron (k=2) → FCC (face-centered cubic) sublattice
        //   cube (k=3)       → BCC (body-centered cubic) sublattice
        // The pre-2026-05-27 code labelled by coord-sum parity, which got
        // cube corners and octahedral sites swapped (audit P0-17).
        const _SHELL_TO_SUBLATTICE = {
            center: 'central',
            octahedron: 'SC',
            cuboctahedron: 'FCC',
            cube: 'BCC',
        };

        // F-11: one InstancedMesh per shell (4 draw calls total) instead of
        // 27 individual Meshes. Per-shell keeps the existing visibility
        // toggles (this._groups.<shell>.visible) and per-shell colouring
        // working, while per-instance colour/position carry each site's
        // exact appearance.
        const SHELL_NAMES = ['center', 'octahedron', 'cuboctahedron', 'cube'];
        const sitesByShell = { center: [], octahedron: [], cuboctahedron: [], cube: [] };
        for (const site of this._sites) sitesByShell[site.shell].push(site);

        const tmpMatrix = new THREE.Matrix4();
        const tmpColor = new THREE.Color();

        for (const shellName of SHELL_NAMES) {
            const shellSites = sitesByShell[shellName];
            // All sites in a shell share the same radius (set in buildSiteData).
            const radius = shellSites[0].radius;
            const inst = makeInstancedShell(radius, shellSites.length);
            inst.name = 'shell_' + shellName;

            for (let k = 0; k < shellSites.length; k++) {
                const site = shellSites[k];
                tmpMatrix.makeTranslation(site.x * SCALE, site.y * SCALE, site.z * SCALE);
                inst.setMatrixAt(k, tmpMatrix);
                inst.setColorAt(k, tmpColor.setHex(site.color));

                // Per-instance metadata for click-to-inspect (instanceId -> site).
                inst.userData[k] = {
                    position: [site.x, site.y, site.z],
                    shell: site.shell,
                    distance: site.distance,
                    sublattice: _SHELL_TO_SUBLATTICE[site.shell] || 'unknown',
                    // Coord-sum parity preserved separately for the 13+13
                    // visual partition (renamed from misleading 'BCC/FCC' label).
                    coordSumParity: site.parity,
                    stabilizer: site.stabilizer,
                    irrep: site.irrep,
                };
            }
            inst.instanceMatrix.needsUpdate = true;
            if (inst.instanceColor) inst.instanceColor.needsUpdate = true;

            this._shellMeshes[shellName] = inst;
            this._groups[shellName] = inst;
            this._meshes.push(inst);
            this._root.add(inst);
        }

        // Per-site reference table, parallel to this._sites, so the colour
        // toggles (_resetColors / _applyBCCFCC / _applyInversionDomain) can
        // still iterate 0..26 in the same order and recolour by instance.
        const cursor = { center: 0, octahedron: 0, cuboctahedron: 0, cube: 0 };
        for (const site of this._sites) {
            const mesh = this._shellMeshes[site.shell];
            this._siteRefs.push({ mesh, instanceId: cursor[site.shell]++ });
            this._originalColors.push(site.color);
        }
    }

    // ── Wireframe polyhedra ─────────────────────────────────────────

    _buildWireframes() {
        // Octahedron: 6 face-center sites at distance 1
        const octVerts = this._sites
            .filter(s => s.d2 === 1)
            .map(s => [s.x, s.y, s.z]);
        // Edges of the octahedron connect vertices at distance sqrt(2) apart
        const octEdges = edgesAtDistance(octVerts, 2);
        this._groups.wireOctahedron = new THREE.Group();
        this._groups.wireOctahedron.name = 'wire_octahedron';
        this._groups.wireOctahedron.add(makeWireframe(octVerts, octEdges, COLORS.octahedron));
        this._groups.wireOctahedron.visible = false;
        this._root.add(this._groups.wireOctahedron);

        // Cuboctahedron: 12 edge-center sites at distance sqrt(2)
        const cuboctVerts = this._sites
            .filter(s => s.d2 === 2)
            .map(s => [s.x, s.y, s.z]);
        // Edges of cuboctahedron connect vertices at distance sqrt(2) apart (d2=2)
        const cuboctEdges = edgesAtDistance(cuboctVerts, 2);
        this._groups.wireCuboctahedron = new THREE.Group();
        this._groups.wireCuboctahedron.name = 'wire_cuboctahedron';
        this._groups.wireCuboctahedron.add(makeWireframe(cuboctVerts, cuboctEdges, COLORS.cuboctahedron));
        this._groups.wireCuboctahedron.visible = false;
        this._root.add(this._groups.wireCuboctahedron);

        // Full cube: 8 corner sites at distance sqrt(3)
        const cubeVerts = this._sites
            .filter(s => s.d2 === 3)
            .map(s => [s.x, s.y, s.z]);
        // Edges of the cube connect vertices at distance 2 apart (d2=4)
        const cubeEdges = edgesAtDistance(cubeVerts, 4);
        this._groups.wireCube = new THREE.Group();
        this._groups.wireCube.name = 'wire_cube';
        this._groups.wireCube.add(makeWireframe(cubeVerts, cubeEdges, COLORS.cube));
        this._groups.wireCube.visible = false;
        this._root.add(this._groups.wireCube);

        // Tetrahedra from the 8 cube corners, split by parity
        const tetraPlusVerts = [];
        const tetraMinusVerts = [];
        for (const s of this._sites) {
            if (s.d2 !== 3) continue;
            const paritySum = s.x * s.y * s.z;
            if (paritySum > 0) {
                tetraPlusVerts.push([s.x, s.y, s.z]);
            } else {
                tetraMinusVerts.push([s.x, s.y, s.z]);
            }
        }
        // Tetrahedron edges: all pairs (4 vertices, 6 edges)
        const tetraEdgesAll = [[0, 1], [0, 2], [0, 3], [1, 2], [1, 3], [2, 3]];

        this._groups.tetraPlus = new THREE.Group();
        this._groups.tetraPlus.name = 'wire_tetra_plus';
        this._groups.tetraPlus.add(makeWireframe(tetraPlusVerts, tetraEdgesAll, 0x00FFAA));
        this._groups.tetraPlus.visible = false;
        this._root.add(this._groups.tetraPlus);

        this._groups.tetraMinus = new THREE.Group();
        this._groups.tetraMinus.name = 'wire_tetra_minus';
        this._groups.tetraMinus.add(makeWireframe(tetraMinusVerts, tetraEdgesAll, 0xFF5555));
        this._groups.tetraMinus.visible = false;
        this._root.add(this._groups.tetraMinus);
    }

    // ── Connection lines ────────────────────────────────────────────

    _buildConnections() {
        this._groups.connections = new THREE.Group();
        this._groups.connections.name = 'connections';
        this._groups.connections.visible = false;

        const positions = [];
        for (const site of this._sites) {
            if (site.d2 === 0) continue;
            positions.push(0, 0, 0);
            positions.push(site.x * SCALE, site.y * SCALE, site.z * SCALE);
        }
        const geo = new THREE.BufferGeometry();
        geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
        const mat = new THREE.LineBasicMaterial({
            color: COLORS.connection,
            transparent: true,
            opacity: 0.3,
        });
        this._groups.connections.add(new THREE.LineSegments(geo, mat));
        this._root.add(this._groups.connections);
    }

    // ── Symmetry elements ───────────────────────────────────────────

    _buildSymmetryElements() {
        const axisExtent = 1.8;

        // Rotation axes group
        this._groups.rotationAxes = new THREE.Group();
        this._groups.rotationAxes.name = 'rotation_axes';
        this._groups.rotationAxes.visible = false;

        // C4 axes: through opposite face centers (along x, y, z)
        const c4Directions = [[1, 0, 0], [0, 1, 0], [0, 0, 1]];
        for (const d of c4Directions) {
            const from = [d[0] * -axisExtent, d[1] * -axisExtent, d[2] * -axisExtent];
            const to = [d[0] * axisExtent, d[1] * axisExtent, d[2] * axisExtent];
            this._groups.rotationAxes.add(makeAxisLine(from, to, COLORS.axis_c4));
        }

        // C3 axes: through opposite cube corners (body diagonals)
        const c3Directions = [[1, 1, 1], [1, 1, -1], [1, -1, 1], [1, -1, -1]];
        for (const d of c3Directions) {
            const len = Math.sqrt(3);
            const norm = [d[0] / len, d[1] / len, d[2] / len];
            const from = [norm[0] * -axisExtent, norm[1] * -axisExtent, norm[2] * -axisExtent];
            const to = [norm[0] * axisExtent, norm[1] * axisExtent, norm[2] * axisExtent];
            this._groups.rotationAxes.add(makeAxisLine(from, to, COLORS.axis_c3));
        }

        // C2 axes: through opposite edge midpoints
        const c2Directions = [
            [1, 1, 0], [1, -1, 0], [1, 0, 1], [1, 0, -1], [0, 1, 1], [0, 1, -1]
        ];
        for (const d of c2Directions) {
            const len = Math.sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
            const norm = [d[0] / len, d[1] / len, d[2] / len];
            const from = [norm[0] * -axisExtent, norm[1] * -axisExtent, norm[2] * -axisExtent];
            const to = [norm[0] * axisExtent, norm[1] * axisExtent, norm[2] * axisExtent];
            this._groups.rotationAxes.add(makeAxisLine(from, to, COLORS.axis_c2));
        }

        this._root.add(this._groups.rotationAxes);

        // Mirror planes group
        this._groups.mirrorPlanes = new THREE.Group();
        this._groups.mirrorPlanes.name = 'mirror_planes';
        this._groups.mirrorPlanes.visible = false;

        // 3 coordinate planes
        const coordNormals = [[1, 0, 0], [0, 1, 0], [0, 0, 1]];
        for (const n of coordNormals) {
            this._groups.mirrorPlanes.add(makeMirrorPlane(n, COLORS.mirror));
        }

        // 6 diagonal planes
        const diagNormals = [
            [1, 1, 0], [1, -1, 0],
            [1, 0, 1], [1, 0, -1],
            [0, 1, 1], [0, 1, -1],
        ];
        for (const n of diagNormals) {
            this._groups.mirrorPlanes.add(makeMirrorPlane(n, COLORS.mirror));
        }

        this._root.add(this._groups.mirrorPlanes);
    }

    // ── Framework labels ────────────────────────────────────────────

    _buildFrameworkLabels() {
        this._labelContainer = document.createElement('div');
        this._labelContainer.style.position = 'absolute';
        this._labelContainer.style.top = '0';
        this._labelContainer.style.left = '0';
        this._labelContainer.style.pointerEvents = 'none';
        this._labelContainer.style.display = 'none';
        this._renderer.domElement.parentElement.appendChild(this._labelContainer);

        const labels = [
            { text: 'N_c = 3', worldPos: new THREE.Vector3(0, 0, 1.2 * SCALE), desc: 'color charges (cube corners / tetra pair)' },
            { text: 'N_base = 4', worldPos: new THREE.Vector3(1.2 * SCALE, 0, 0), desc: 'base multiplicity (tetrahedron vertices)' },
            { text: 'b_3 = 7', worldPos: new THREE.Vector3(0, 1.2 * SCALE, 0), desc: 'shell sum 1+6 (center + octahedron)' },
            { text: 'N_eff = 13', worldPos: new THREE.Vector3(-1.0 * SCALE, -1.0 * SCALE, 0), desc: 'effective neighbors (1+6+12/2)' },
        ];

        for (const lbl of labels) {
            const el = document.createElement('div');
            el.style.position = 'absolute';
            el.style.color = '#FFD700';
            el.style.fontFamily = 'monospace';
            el.style.fontSize = '13px';
            el.style.fontWeight = 'bold';
            el.style.textShadow = '0 0 4px rgba(0,0,0,0.8)';
            el.style.whiteSpace = 'nowrap';
            el.textContent = lbl.text;
            el.title = lbl.desc;
            this._labelContainer.appendChild(el);
            this._labelElements.push({ el, worldPos: lbl.worldPos });
        }
    }

    _updateLabelPositions() {
        if (!this._labelContainer || this._labelContainer.style.display === 'none') return;
        const canvas = this._renderer.domElement;
        const halfW = canvas.clientWidth / 2;
        const halfH = canvas.clientHeight / 2;
        const tempVec = new THREE.Vector3();

        for (const lbl of this._labelElements) {
            tempVec.copy(lbl.worldPos);
            this._root.localToWorld(tempVec);
            tempVec.project(this._camera);

            const x = (tempVec.x * halfW) + halfW;
            const y = -(tempVec.y * halfH) + halfH;

            if (tempVec.z > 1) {
                lbl.el.style.display = 'none';
            } else {
                lbl.el.style.display = '';
                lbl.el.style.left = x + 'px';
                lbl.el.style.top = y + 'px';
            }
        }
    }

    // ── Parity coloring ─────────────────────────────────────────────

    // Recolour by writing per-instance colour. The shell material keeps the
    // colour===emissive invariant via the shader injection in
    // makeInstancedShell, so one setColorAt per site updates both diffuse and
    // emissive exactly as the legacy per-material color/emissive pair did.
    _setSiteColor(i, hex) {
        const ref = this._siteRefs[i];
        _tmpRecolor.setHex(hex);
        ref.mesh.setColorAt(ref.instanceId, _tmpRecolor);
        if (ref.mesh.instanceColor) ref.mesh.instanceColor.needsUpdate = true;
    }

    _resetColors() {
        for (let i = 0; i < this._siteRefs.length; i++) {
            this._setSiteColor(i, this._originalColors[i]);
        }
    }

    _applyBCCFCC() {
        for (let i = 0; i < this._siteRefs.length; i++) {
            const site = this._sites[i];
            if (site.d2 === 0) continue;
            const c = site.parity === 'even' ? COLORS.evenParity : COLORS.oddParity;
            this._setSiteColor(i, c);
        }
    }

    _applyInversionDomain() {
        for (let i = 0; i < this._siteRefs.length; i++) {
            const site = this._sites[i];
            if (site.d2 === 0) continue;
            const c = site.inversionParity === 'orbit_rep' ? COLORS.orbitRep : COLORS.antipode;
            this._setSiteColor(i, c);
        }
    }

    // ── Toggle methods ──────────────────────────────────────────────

    toggleCenter(on) {
        const v = on !== undefined ? on : !this._groups.center.visible;
        this._groups.center.visible = v;
        return v;
    }

    toggleOctahedron(on) {
        const v = on !== undefined ? on : !this._groups.octahedron.visible;
        this._groups.octahedron.visible = v;
        this._groups.wireOctahedron.visible = v;
        return v;
    }

    toggleCuboctahedron(on) {
        const v = on !== undefined ? on : !this._groups.cuboctahedron.visible;
        this._groups.cuboctahedron.visible = v;
        this._groups.wireCuboctahedron.visible = v;
        return v;
    }

    toggleCube(on) {
        const v = on !== undefined ? on : !this._groups.cube.visible;
        this._groups.cube.visible = v;
        this._groups.wireCube.visible = v;
        return v;
    }

    toggleTetraPlus(on) {
        const v = on !== undefined ? on : !this._groups.tetraPlus.visible;
        this._groups.tetraPlus.visible = v;
        return v;
    }

    toggleTetraMinus(on) {
        const v = on !== undefined ? on : !this._groups.tetraMinus.visible;
        this._groups.tetraMinus.visible = v;
        return v;
    }

    toggleBCCFCC(on) {
        const active = on !== undefined ? on : true;
        if (active) {
            this._applyBCCFCC();
        } else {
            this._resetColors();
        }
        return active;
    }

    toggleInversionDomain(on) {
        const active = on !== undefined ? on : true;
        if (active) {
            this._applyInversionDomain();
        } else {
            this._resetColors();
        }
        return active;
    }

    // Back-compat alias: the toggle was historically (mis)named "gerade/
    // ungerade". No in-repo caller uses this name anymore (owned files call
    // toggleInversionDomain); kept only so any external caller still
    // resolves. See audit P1-7.
    toggleGeradeUngerade(on) {
        return this.toggleInversionDomain(on);
    }

    toggleConnections(on) {
        const v = on !== undefined ? on : !this._groups.connections.visible;
        this._groups.connections.visible = v;
        return v;
    }

    toggleRotationAxes(on) {
        const v = on !== undefined ? on : !this._groups.rotationAxes.visible;
        this._groups.rotationAxes.visible = v;
        return v;
    }

    toggleMirrorPlanes(on) {
        const v = on !== undefined ? on : !this._groups.mirrorPlanes.visible;
        this._groups.mirrorPlanes.visible = v;
        return v;
    }

    toggleFrameworkLabels(on) {
        const v = on !== undefined ? on : (this._labelContainer.style.display === 'none');
        this._labelContainer.style.display = v ? '' : 'none';
        return v;
    }

    toggleAutoRotate(on) {
        const v = on !== undefined ? on : !this._autoRotate;
        this._autoRotate = v;
        return v;
    }

    // ── Update (per frame) ──────────────────────────────────────────

    update(deltaTime) {
        if (this._autoRotate) {
            this._root.rotation.y += this._rotationSpeed * deltaTime;
        }
        this._updateLabelPositions();
    }

    // ── Click-to-inspect ────────────────────────────────────────────

    inspectSite(raycaster) {
        // Raycast against the 4 per-shell InstancedMeshes. Only visible shells
        // participate, matching the legacy per-mesh behaviour. Each hit carries
        // an `instanceId`; the per-instance site metadata lives in the
        // InstancedMesh's userData keyed by that id (set in _buildShellGroups).
        const shellMeshes = [];
        for (const m of this._meshes) {
            if (m && m.visible) shellMeshes.push(m);
        }
        const hits = raycaster.intersectObjects(shellMeshes, false);
        if (hits.length === 0) return null;

        const hit = hits[0];
        const instanceId = hit.instanceId;
        if (instanceId === undefined || instanceId === null) return null;
        return hit.object.userData[instanceId] || null;
    }

    // ── Lifecycle ───────────────────────────────────────────────────

    dispose() {
        // Remove label overlay
        if (this._labelContainer && this._labelContainer.parentElement) {
            this._labelContainer.parentElement.removeChild(this._labelContainer);
        }

        // Free per-shell InstancedMesh instance buffers (traverse below frees
        // their geometry/material; .dispose() releases the instanced attrs).
        for (const m of this._meshes) {
            if (m && typeof m.dispose === 'function') m.dispose();
        }

        // Dispose all Three.js objects
        this._root.traverse(child => {
            if (child.geometry) child.geometry.dispose();
            if (child.material) {
                if (Array.isArray(child.material)) {
                    child.material.forEach(m => m.dispose());
                } else {
                    child.material.dispose();
                }
            }
        });

        this._scene.remove(this._root);
        this._meshes = [];
        this._shellMeshes = {};
        this._siteRefs = [];
        this._labelElements = [];
        this._groups = {};
    }
}
