// ==========================================================================
//  engine/src/scenarios/cell.cpp
//
//  s0-cell-* scenario group: flux cells (bounded field reservoirs).
//
//  A flux cell is a localized finite field configuration whose energy is
//  meant to stay above vacuum after its pump is disconnected. These bodies
//  seed only initial data on an isolated profile; every storage, hold, or
//  leakage statement is measured by tests/test_flux_cell_scenario_physics.cpp
//  through the regional ledger in ftd/flux_cell.h. None of the ids below
//  asserts a capacitor, inductor, battery, or particle identity.
//
//  Build order (V0 -> V2 of the flux-cell programme):
//    s0-cell-capacitor        V0  locked plate pair, Gauss-charged flux gap
//    s0-cell-torus            V1  azimuthal ring reservoir, periodic hold
//    s0-cell-torus-reverse    V1  opposite-circulation control
//    s0-cell-torus-scrambled  V1  phase-scrambled, zero-circulation control
//    s0-cell-torus-open       V1  dispersal-boundary (no-membrane) control
//    s0-cell-torus-walled     V1  reflective-box (walled-membrane) hold
//    s0-cell-triad            V2  three-axis standing arms, zero net momentum
//    s0-cell-torus-membrane        ring inside a locked clocked shell (membrane)
//    s0-cell-torus-membrane-gated  same, with a scheduled aperture (port)
//    s0-cell-membrane-pumped       empty membrane charged by the flux pump
//    s0-cell-membrane-transfer     two tangent membranes, port between them
//    s0-cell-membrane-pumped-resonant  pump increments spaced by the cell period
//
//  The membrane, pump, and port mechanisms and their physics justification
//  live in ftd/flux_cell.h. Every body registers its cell region so the
//  energy audit carries the regional ledger in the dashboard.
//
//  Not implemented here, by design: a phase-winding gate with packetized
//  discharge (V3) needs a finite phase carrier and a gated transaction the
//  engine does not have; the charged-versus-empty inertia and gravity
//  comparison (V4) is a measurement campaign, not initial data.
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/flux_cell.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace ftd {

// Resonant pump spacing (ticks between increments) for
// s0-cell-membrane-pumped-resonant, chosen by the test scan of the
// engine-booked pump work W_in(P) on the membrane cell (see
// tests/test_flux_cell_scenario_physics.cpp, "resonant pump scan").
constexpr int kResonantPumpPeriod = 8;

namespace {

// Seed one standing transverse arm along `axis` (0=x, 1=y, 2=z): the
// co-located superposition of a +direction and a -direction transverse packet
// of the inject_transverse_packet_x construction, rotated to the chosen axis.
//
//   psi = amp*sigma_t * exp(-(d_a^2/sigma_a^2 + (d_b^2+d_c^2)/sigma_t^2)/2) * cos(k d_a)
//   J   = curl(psi e_a) = (0, D_c psi, -D_b psi) in the cyclic (a,b,c) frame
//
// so the centred-difference divergence vanishes identically. The two
// counter-propagating members cancel the one-way drift term -c*D_a J, leaving
// only the direction-free kick-drift phase term
//
//   W = -(c^2/2) * L18(J),
//
// which places W on the pre-kick time phase of a standing configuration
// (W_0 = J_0 (1 - cos omega) for every lattice pole). Net wave momentum is
// zero by construction; nothing about spin, photons, or confinement is
// implied.
inline void inject_standing_arm(RenderBridge& rb, int axis,
                                double cx, double cy, double cz,
                                double sigma_along, double sigma_t,
                                double amp, double carrier_k) {
    const int N = rb.lattice().size();
    const double sa = std::max(1.0, sigma_along);
    const double st = std::max(1.0, sigma_t);
    const double psi_amp = amp * st;
    const int a = axis % 3, b = (axis + 1) % 3, c = (axis + 2) % 3;
    const double ctr[3] = {cx, cy, cz};

    auto periodic_delta = [N](double p, double q) {
        double d = p - q;
        while (d >  0.5 * N) d -= N;
        while (d < -0.5 * N) d += N;
        return d;
    };
    auto psi = [&](const double p[3]) {
        double d[3];
        for (int i = 0; i < 3; ++i) d[i] = periodic_delta(p[i], ctr[i]);
        const double r2 = d[a]*d[a]/(sa*sa) + (d[b]*d[b] + d[c]*d[c])/(st*st);
        if (r2 > 18.0) return 0.0;
        return psi_amp * std::exp(-0.5 * r2) * std::cos(carrier_k * d[a]);
    };
    auto field = [&](double x, double y, double z) {
        double pcp[3] = {x, y, z}, pcm[3] = {x, y, z};
        double pbp[3] = {x, y, z}, pbm[3] = {x, y, z};
        pcp[c] += 1.0; pcm[c] -= 1.0;
        pbp[b] += 1.0; pbm[b] -= 1.0;
        double comp[3] = {0.0, 0.0, 0.0};
        comp[b] =  0.5 * (psi(pcp) - psi(pcm));
        comp[c] = -0.5 * (psi(pbp) - psi(pbm));
        return Vec3(comp[0], comp[1], comp[2]);
    };

    const int face[6][3] = {
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
    };
    const int edge[12][3] = {
        {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
        {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
        {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
    };
    for (int z = 0; z < N; ++z)
    for (int y = 0; y < N; ++y)
    for (int x = 0; x < N; ++x) {
        const Vec3 j = field(x, y, z);
        Vec3 face_sum;
        Vec3 edge_sum;
        for (const auto& o : face) face_sum += field(x + o[0], y + o[1], z + o[2]);
        for (const auto& o : edge) edge_sum += field(x + o[0], y + o[1], z + o[2]);
        const Vec3 lap = face_sum * (1.0 / 3.0) + edge_sum * (1.0 / 6.0) - j * 4.0;
        const Vec3 w = lap * (-0.5 * C_SPEED * C_SPEED);
        if (j.mag2() > 1e-20) IF(rb, x, y, z, j.x, j.y, j.z);
        if (w.mag2() > 1e-20) IW(rb, x, y, z, w.x, w.y, w.z);
    }
}

// Canonical ring-reservoir profile: the bare periodic wave map with the
// requested flux-cell torus. Callers override the boundary law afterwards.
void seed_ring_reservoir(RenderBridge& rb, const FluxCellTorusSpec& spec) {
    configure_free_wave_terms(rb, false);
    rb.toggles.flux_boundary = FluxBoundaryMode::Periodic;
    seed_flux_cell_torus(rb, spec, 1.0);
    rb.set_flux_cell_region({spec.cx, spec.cy, spec.cz,
                             spec.major_radius + 3.0 * spec.tube_sigma});
}

// Membrane profile: the bare wave map plus the [IMPOSED] de Broglie clock at
// omega0 = 1 (rad/tick), so the locked shell carries a mass gap and every
// mode below it is evanescent inside the wall (ftd/flux_cell.h).
void configure_membrane_terms(RenderBridge& rb) {
    configure_free_wave_terms(rb, false);
    rb.toggles.flux_boundary = FluxBoundaryMode::Periodic;
    rb.toggles.de_broglie_clock = true;
    rb.toggles.omega0 = 1.0;
}

}  // namespace

bool setup_cell_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("s0-cell-", 0) != 0) return false;
    const int    N    = rb.lattice().size();
    const double midF = (N - 1) * 0.5;
    const int    mc   = RND(midF);

    if (name == "s0-cell-capacitor") {
        // Scenario ID: s0-cell-capacitor
        // Physical Purpose: V0 flux capacitor. Two locked square plates of
        // opposite polarity face each other across a neutral gap with zero
        // seeded field. The engine's own Gauss law div(J) = s builds the gap
        // flux on the first projection; the wave map then evolves it.
        // Initial Condition Parameters: plates at x = mc -/+ max(2, N/8),
        // side 2*max(1, N/8) + 1, states +1 (low x) and -1 (high x), locked.
        // Expected Behaviour: zero gap energy before the first tick, positive
        // flux-potential energy U_J in the gap afterwards; plates stay inert.
        // Discrepancy: the stored channel is U_J (the flux-potential channel),
        // not the electric energy |E|^2/2; C_eff = Q^2/(2 U_J) is a lattice
        // observable, not a physical capacitance.
        configure_free_wave_terms(rb, true);
        rb.toggles.flux_boundary = FluxBoundaryMode::Periodic;
        const int gap_half   = std::max(2, N / 8);
        const int plate_half = std::max(1, N / 8);
        for (int dy = -plate_half; dy <= plate_half; ++dy)
        for (int dz = -plate_half; dz <= plate_half; ++dz) {
            IP(rb, mc - gap_half, mc + dy, mc + dz, +1);
            LOCK(rb, mc - gap_half, mc + dy, mc + dz);
            IP(rb, mc + gap_half, mc + dy, mc + dz, -1);
            LOCK(rb, mc + gap_half, mc + dy, mc + dz);
        }
        rb.set_flux_cell_region({midF, midF, midF, gap_half - 0.5});
    }
    else if (name == "s0-cell-torus") {
        // Scenario ID: s0-cell-torus
        // Physical Purpose: V1 circulating-flux accumulator. A Gaussian-tube
        // ring of azimuthal J (R = max(3, N/4), sigma = max(1.25, N/16),
        // peak 0.3) with zero canonical momentum on the periodic free wave
        // map: the pump is disconnected at tick 0 and the hold is measured.
        // Expected Behaviour: the kick-drift Hamiltonian is conserved, the
        // electric channel U_E rises from zero as the magnetic channel U_B
        // falls (LC-like exchange), net Poynting flow is zero by symmetry.
        // Discrepancy: the bare wave map has no confinement term, so the ring
        // disperses; retention inside the ring region is a measured number,
        // not a claim of a persistent current or an inductor.
        seed_ring_reservoir(rb, default_flux_cell_torus_spec(N));
    }
    else if (name == "s0-cell-torus-reverse") {
        // Scenario ID: s0-cell-torus-reverse
        // Physical Purpose: opposite-winding control for s0-cell-torus.
        // Expected Behaviour: identical stored energy, equal and opposite
        // ring circulation and disk flux.
        FluxCellTorusSpec spec = default_flux_cell_torus_spec(N);
        spec.circulation_sign = -1;
        seed_ring_reservoir(rb, spec);
    }
    else if (name == "s0-cell-torus-scrambled") {
        // Scenario ID: s0-cell-torus-scrambled
        // Physical Purpose: phase-scrambled / zero-circulation control. The
        // sign of J_phi alternates between the four azimuthal quadrants, so
        // the pointwise |J| (and U_J) equal the coherent ring exactly while
        // the ring circulation vanishes by symmetry.
        // Expected Behaviour: same U_J, zero Gamma_J, larger gradient energy;
        // the free wave map then acts on the higher-k content.
        FluxCellTorusSpec spec = default_flux_cell_torus_spec(N);
        spec.sign_sectors = 2;
        seed_ring_reservoir(rb, spec);
    }
    else if (name == "s0-cell-torus-open") {
        // Scenario ID: s0-cell-torus-open
        // Physical Purpose: no-membrane control. Same ring under the Dispersal
        // boundary law (exact-zero outer shell, one-way closure), so field that
        // reaches the box faces leaves and is never re-injected.
        // Expected Behaviour: the box Hamiltonian falls toward zero; the
        // periodic hold cannot be credited to storage if this control does
        // not lose its energy.
        seed_ring_reservoir(rb, default_flux_cell_torus_spec(N));
        rb.toggles.flux_boundary = FluxBoundaryMode::Dispersal;
    }
    else if (name == "s0-cell-torus-walled") {
        // Scenario ID: s0-cell-torus-walled
        // Physical Purpose: walled-membrane hold. Same ring under the
        // Reflective boundary law (Neumann ghost shell), the only closed
        // boundary the engine provides; locked marker sheets are already a
        // closed negative for confinement (quantum-well).
        // Expected Behaviour: the interior wave energy stays inside the box;
        // the interior Hamiltonian retention is measured, not assumed.
        seed_ring_reservoir(rb, default_flux_cell_torus_spec(N));
        rb.toggles.flux_boundary = FluxBoundaryMode::Reflective;
    }
    else if (name == "s0-cell-triad") {
        // Scenario ID: s0-cell-triad
        // Physical Purpose: V2 three-axis counter-propagating flux cell. One
        // standing transverse arm per axis (each the co-located sum of a +
        // and a - packet) crosses the centre, so every axis carries equal
        // energy and zero net current.
        // Initial Condition Parameters: sigma_along = max(3, N/6),
        // sigma_t = max(1.5, N/20), peak 0.3, carrier wavelength max(6, N/5).
        // Expected Behaviour: zero net Poynting flow and zero net flux vector,
        // equal diagonal flux moments sum J_a J_a on the three axes, conserved
        // Hamiltonian.
        // Discrepancy: the arms overlap at the centre, so the flux dyad
        // sum J_a J_b carries small off-diagonal overlap terms (measured 1.9%
        // of the trace at L=33); the dyad is an observer-level flux moment,
        // not a stress-energy tensor; no matter-clock or particle identity.
        configure_free_wave_terms(rb, false);
        rb.toggles.flux_boundary = FluxBoundaryMode::Periodic;
        const double sigma_along = std::max(3.0, N / 6.0);
        const double sigma_t     = std::max(1.5, N / 20.0);
        const double wavelength  = std::max(6.0, N / 5.0);
        const double k           = 2.0 * PI / wavelength;
        for (int axis = 0; axis < 3; ++axis)
            inject_standing_arm(rb, axis, midF, midF, midF,
                                sigma_along, sigma_t, 0.3, k);
        rb.set_flux_cell_region({midF, midF, midF, 2.0 * sigma_along});
    }
    else if (name == "s0-cell-torus-membrane") {
        // Scenario ID: s0-cell-torus-membrane
        // Physical Purpose: walled ring reservoir. The ring sits inside a
        // three-cell locked shell of alternating polarity; with the de Broglie
        // clock on, the shell is a mass-gap wall (omega0 = 1) and every ring
        // mode below the gap is evanescent inside it.
        // Initial Condition Parameters: shell outer radius (N-1)/2 - 0.5,
        // thickness 3; ring R = 0.45 r_in, sigma = 0.11 r_in, peak 0.3.
        // Expected Behaviour: the inner-ball energy is retained far above the
        // bare-map uniform-fill value; with omega0 -> 0 the same shell is
        // transparent and retention falls back to the bare-map value.
        // Discrepancy: the shell is imposed initial data under an [IMPOSED]
        // clock term; no self-confinement, no particle or matter identity.
        configure_membrane_terms(rb);
        const FluxCellMembraneSpec shell = default_flux_cell_membrane_spec(N, 3.0);
        const FluxCellTorusSpec ring = flux_cell_membrane_ring_spec(shell);
        seed_flux_cell_membrane(rb, shell);
        seed_flux_cell_torus(rb, ring, 1.0);
        rb.set_flux_cell_region({shell.cx, shell.cy, shell.cz,
                                 shell.inner_radius - 0.5});
    }
    else if (name == "s0-cell-torus-membrane-gated") {
        // Scenario ID: s0-cell-torus-membrane-gated
        // Physical Purpose: Phase-4 discharge port. Same walled ring; at tick
        // 150 the shell sites inside a radius-3 hole on the +x side expire, and
        // the engine integrates the Poynting flux through the opened sites.
        // Expected Behaviour: no port flux before opening; after opening the
        // inner-ball energy falls and the integrated port flux W_out accounts
        // for it up to the site-versus-face discretization of S.
        // Discrepancy: an aperture in an imposed shell; the outgoing packet's
        // coherence and the port ledger are measured, not asserted.
        configure_membrane_terms(rb);
        rb.toggles.flux_cell_port = true;
        const FluxCellMembraneSpec shell = default_flux_cell_membrane_spec(N, 3.0);
        const FluxCellTorusSpec ring = flux_cell_membrane_ring_spec(shell);
        seed_flux_cell_membrane(rb, shell);
        seed_flux_cell_torus(rb, ring, 1.0);
        FluxCellPortSpec port;
        port.cx = shell.cx + shell.inner_radius + 0.5 * shell.thickness;
        port.cy = shell.cy;
        port.cz = shell.cz;
        port.nx = 1.0; port.ny = 0.0; port.nz = 0.0;
        port.radius = std::max(2.0, shell.thickness);
        port.surface_offset = 0.5 * shell.thickness - 0.5;  // outer face of the plug
        port.open_tick = 150;
        rb.set_flux_cell_port(port);
        rb.set_flux_cell_region({shell.cx, shell.cy, shell.cz,
                                 shell.inner_radius - 0.5});
    }
    else if (name == "s0-cell-membrane-pumped") {
        // Scenario ID: s0-cell-membrane-pumped
        // Physical Purpose: Phase-2 dynamical charging. The membrane starts
        // empty; the flux pump adds 1/20 of the ring profile before each of the
        // first 20 ticks and then switches off. The engine books the injected
        // work W_in exactly per tick.
        // Expected Behaviour: W_in equals the kick-drift Hamiltonian at
        // disconnection; afterwards the held energy is constant and the
        // inner-ball retention matches the walled ring.
        // Discrepancy: the pump is an imposed source term; disconnection is a
        // hard switch, not a self-regulating transducer.
        configure_membrane_terms(rb);
        rb.toggles.flux_pump = true;
        const FluxCellMembraneSpec shell = default_flux_cell_membrane_spec(N, 3.0);
        const FluxCellTorusSpec ring = flux_cell_membrane_ring_spec(shell);
        seed_flux_cell_membrane(rb, shell);
        rb.set_flux_pump(ring, 20);
        rb.set_flux_cell_region({shell.cx, shell.cy, shell.cz,
                                 shell.inner_radius - 0.5});
    }
    else if (name == "s0-cell-membrane-pumped-resonant") {
        // Scenario ID: s0-cell-membrane-pumped-resonant
        // Physical Purpose: resonant charging. Same empty membrane and the
        // same 20 increments as s0-cell-membrane-pumped, but spaced by the
        // cell's breathing period so successive increments meet the field in
        // phase (the -2 J.L(delta) term of the booked work).
        // Expected Behaviour: the engine-booked W_in exceeds the every-tick
        // pump's by a large factor; stored energy grows faster than linearly
        // in the increment count.
        // Discrepancy: an imposed periodic source; the period is a measured
        // engine number, not a derived constant.
        configure_membrane_terms(rb);
        rb.toggles.flux_pump = true;
        const FluxCellMembraneSpec shell = default_flux_cell_membrane_spec(N, 3.0);
        const FluxCellTorusSpec ring = flux_cell_membrane_ring_spec(shell);
        seed_flux_cell_membrane(rb, shell);
        rb.set_flux_pump(ring, 20, kResonantPumpPeriod);
        rb.set_flux_cell_region({shell.cx, shell.cy, shell.cz,
                                 shell.inner_radius - 0.5});
    }
    else if (name == "s0-cell-membrane-transfer") {
        // Scenario ID: s0-cell-membrane-transfer
        // Physical Purpose: energy transfer between two cells. Two tangent
        // clocked shells along x (source A with a ring, receiver B empty);
        // at tick 100 a radius-3 hole opens at the contact point through
        // both walls and the engine integrates the wave-Hamiltonian current
        // across the contact plane.
        // Initial Condition Parameters: cell centres mid -/+ floor(N/4),
        // outer radius floor(N/4) - 0.5, wall thickness min(3, r_out - 3.5);
        // ring R = 0.45 r_in inside A.
        // Expected Behaviour: B's clock-inclusive Hamiltonian is zero until
        // the port opens and then rises; the port work books the transfer.
        // Discrepancy: an imposed channel between imposed walls; no receiver
        // identity, work packet, or transducer is claimed.
        configure_membrane_terms(rb);
        rb.toggles.flux_cell_port = true;
        const double half_sep = std::max(2.0, std::floor(N / 4.0));
        const double r_out = half_sep - 0.5;
        // Three-cell walls whenever the cell is big enough to keep an interior
        // (a two-cell wall around a small ring leaked 18% per 100 ticks).
        const double wall = std::min(3.0, std::max(1.0, r_out - 3.5));
        FluxCellMembraneSpec a;
        a.cx = midF - half_sep; a.cy = midF; a.cz = midF;
        a.thickness = wall;
        a.inner_radius = std::max(1.0, r_out - wall);
        FluxCellMembraneSpec b = a;
        b.cx = midF + half_sep;
        seed_flux_cell_membrane(rb, a);
        seed_flux_cell_membrane(rb, b);
        seed_flux_cell_torus(rb, flux_cell_membrane_ring_spec(a), 1.0);
        FluxCellPortSpec port;
        port.cx = midF; port.cy = midF; port.cz = midF;   // the contact point
        port.nx = 1.0; port.ny = 0.0; port.nz = 0.0;      // from A into B
        port.radius = 3.0;
        port.surface_offset = 0.0;                        // the contact plane
        port.open_tick = 100;
        rb.set_flux_cell_port(port);
        // The registered region is the receiver: the dashboard's Flux Cell
        // rows show energy arriving in B.
        rb.set_flux_cell_region({b.cx, b.cy, b.cz, r_out - 0.5});
    }
    else {
        return false;
    }
    return true;
}

}  // namespace ftd
