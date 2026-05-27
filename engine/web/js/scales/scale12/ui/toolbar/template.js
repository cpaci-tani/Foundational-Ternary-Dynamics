export function getScale12MetaToolbarTemplate() {
    return `
        <div class="tb-group tb-group-meta scale12-only" id="meta-controls">
            <span class="tb-label tb-group-meta-title">Existential Unit</span>
            <button class="view-toggle active" id="meta-toggle-center" title="Center (CM point i)">Center</button>
            <button class="view-toggle active" id="meta-toggle-oct" title="Octahedron (6 SC)">Oct</button>
            <button class="view-toggle active" id="meta-toggle-cuboct" title="Cuboctahedron (12 FCC)">Cuboct</button>
            <button class="view-toggle active" id="meta-toggle-cube" title="Cube (8 BCC)">Cube</button>
            <span class="field-sep"></span>
            <button class="view-toggle" id="meta-toggle-tetra-plus" title="Tetrahedron T+ (even)">T+</button>
            <button class="view-toggle" id="meta-toggle-tetra-minus" title="Tetrahedron T- (odd)">T-</button>
            <button class="view-toggle" id="meta-toggle-connections" title="Neighbor connections">Links</button>
            <span class="field-sep"></span>
            <button class="view-toggle" id="meta-toggle-bcc-fcc" title="Coord-sum parity coloring (13 even / 14 odd). Site userData.sublattice carries the canonical shell→sublattice mapping (center / SC / FCC / BCC) per Moore Layer Theorem §4; this toggle is the parity visual, not the canonical labelling — audit P0-17 fix, 2026-05-27.">BCC/FCC</button>
            <button class="view-toggle" id="meta-toggle-gerade" title="Antipode partition (13+13 inversion fundamental domain). Historical name 'gerade/ungerade' retained for compatibility; this is NOT representation-theoretic g/u parity — see meta-unit.js audit P1-7.">g/u</button>
            <span class="field-sep"></span>
            <button class="view-toggle" id="meta-toggle-axes" title="Rotation axes (C2,C3,C4)">Axes</button>
            <button class="view-toggle" id="meta-toggle-mirrors" title="Mirror planes">Mirrors</button>
            <span class="field-sep"></span>
            <button class="view-toggle active" id="meta-toggle-labels" title="Framework labels">Labels</button>
            <button class="view-toggle active" id="meta-toggle-rotate" title="Auto-rotate">Rotate</button>
        </div>
    `;
}
