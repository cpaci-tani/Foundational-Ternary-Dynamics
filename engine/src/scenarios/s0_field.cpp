// ==========================================================================
//  engine/src/scenarios/s0_field.cpp
//
//  Group: s0-field-* (16 scenarios)
//  Canonical seed implementation; the former JS mirror is archived.
//
//  Split out of engine/src/scenarios.cpp (ticket S1).
//
//  Universal scenario seeding (2026-09-16): every meaningful hardcoded
//  construction literal below is now bound through ftd::seed::{real,
//  integer,choice} (declared in ftd/scenario_seed.h). With no
//  ftd::seed::Context installed, record() returns each default_value
//  unchanged and never touches the RNG, so the legacy no-context dispatch
//  path is byte-identical to the pre-seeding source. Grouping convention:
//    source.*      — absolute placement coordinates of a marker/source
//    geometry.*     — widths, fractions, cutoffs, radii, separations
//    packet.*       — field amplitudes, multipliers of a fixed-law
//                     constant, mode numbers, carrier phases
//    field.*        — imposed uniform background field magnitudes
//    constituentN.* — per-lane initial data for a scenario with more than
//                     one independently placed source (sound-collision)
//  Fixed-law constants (K_B, ALPHA, C_SPEED, PI) and the FTD-0298/0299
//  boundary-declared SOUND_PROXY_SPEED divisor are left hardcoded per
//  FTD-0371/SCOPE_CONSUMPTION_PROGRAM discipline: exposing an amplitude
//  multiplier or width is not a physics claim, but making a fixed constant
//  or a boundary-declared ratio itself editable would be.
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <cmath>

namespace ftd {

bool setup_s0_field_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("s0-field-", 0) != 0) return false;
    const int    N    = rb.lattice().size();
    const double midF = (N - 1) * 0.5;
    const int    mc   = RND(midF);

    if (name == "s0-field-plane-wave") {
        // Scenario ID: s0-field-plane-wave
        // Physical Purpose: Exact traveling eigenmode of the native linear
        // kick-drift wave operator; no electromagnetic identity is assumed.
        // Initial Condition: mode n=4, z polarization, +x propagation.
        // Expected Behaviour: J_z=A sin(kx-omega*t) at the exact lattice pole.
        configure_free_wave_terms(rb, false);
        const int modeN = seed::integer("packet.modeNumber", 4, 1, std::max(N, 64),
            "Harmonic mode number", "Integer wavenumber index n of the seeded plane harmonic along x (k = 2*pi*n/N).");
        const double ampMult = seed::real("packet.amplitudeMultiplier", 2.0, 0.0, 40.0,
            "Amplitude multiplier", "Multiplier on K_B giving the peak J_z/W_z amplitude of the seeded harmonic.");
        const int dir = seed::choice("packet.direction", 1, {{1.0, "+x"}, {-1.0, "-x"}},
            "Propagation direction", "Sign of the seeded harmonic's travel direction along x.");
        inject_plane_harmonic_x(rb, modeN, K_B * ampMult, dir);
    }
    else if (name == "s0-field-standing-wave") {
        // Scenario ID: s0-field-standing-wave
        // Physical Purpose: Exact standing eigenmode of the native linear
        // kick-drift wave operator; no cavity or material boundary is implied.
        // Initial Condition: mode n=4, z polarization, exact pre-kick phase.
        // Expected Behaviour: J_z=A sin(kx) cos(omega*t), with fixed nodes.
        configure_free_wave_terms(rb, false);
        const int modeN = seed::integer("packet.modeNumber", 4, 1, std::max(N, 64),
            "Harmonic mode number", "Integer wavenumber index n of the seeded standing harmonic along x (k = 2*pi*n/N).");
        const double ampMult = seed::real("packet.amplitudeMultiplier", 2.0, 0.0, 40.0,
            "Amplitude multiplier", "Multiplier on K_B giving the peak J_z amplitude of the seeded harmonic.");
        inject_standing_harmonic_x(rb, modeN, K_B * ampMult);
    }
    else if (name == "s0-field-uniform-e") {
        configure_static_seed_terms(rb);
        // Scenario ID: s0-field-uniform-e
        // Physical Purpose: Establishes a uniform electric field.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Spatially uniform vector potential background along x.
        // Discrepancy: the phrase "flux background" is misleading - the behaviour
        // test asserts flux.mag2() == 0.0 for this scenario. What is seeded is the
        // potential, not a nonzero J.
        // genesis=false (audit-2 2026-04-28): static uniform E shouldn't
        // fill the lattice with manifested particles. Mirrors JS.
        rb.toggles.genesis = false;
        const double eMag = seed::real("field.magnitude", 0.1, 0.0, 20.0,
            "Field magnitude", "Magnitude of the uniform background wave-velocity (W_axis = -this) imposed at every site.");
        const int eAxis = seed::choice("field.axis", 0, {{0.0, "x"}, {1.0, "y"}, {2.0, "z"}},
            "Field axis", "Cartesian axis carrying the uniform background wave-velocity.");
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double wv[3] = {0, 0, 0};
            wv[eAxis] = -eMag;
            IW(rb, x, y, z, wv[0], wv[1], wv[2]);
        }
    }
    else if (name == "s0-field-uniform-b") {
        configure_static_seed_terms(rb);
        // Scenario ID: s0-field-uniform-b
        // Physical Purpose: Establishes a uniform magnetic field.
        // Initial Condition Parameters: None.
        // Expected Behaviour: Rotational flux field pattern representing a uniform magnetic field.
        // Discrepancy: None.
        const double bMag = seed::real("field.magnitude", 0.05, 0.0, 20.0,
            "Field magnitude", "Magnitude of the imposed uniform curl-vector-potential field B_axis.");
        const int bAxis = seed::choice("field.axis", 2, {{0.0, "x"}, {1.0, "y"}, {2.0, "z"}},
            "Field axis", "Cartesian axis the uniform curl-vector-potential field points along.");
        const double half = (N - 1) / 2.0;
        const int a = bAxis, b = (bAxis + 1) % 3, c = (bAxis + 2) % 3;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            const double p[3] = {double(x) - half, double(y) - half, double(z) - half};
            double comp[3] = {0, 0, 0};
            comp[b] = -bMag * p[c] / 2;
            comp[c] =  bMag * p[b] / 2;
            (void)a;
            if (std::fabs(comp[0]) > 1e-12 || std::fabs(comp[1]) > 1e-12 || std::fabs(comp[2]) > 1e-12)
                IF(rb, x, y, z, comp[0], comp[1], comp[2]);
        }
    }
    else if (name == "s0-field-photon-pulse") {
        // Scenario ID: s0-field-photon-pulse
        // Physical Purpose: Tests a broad transverse packet as a photon candidate.
        // Initial Condition Parameters: None.
        // Qualification: closed negative for the current seed. At L=48 over
        // 20 ticks its energy centroid speed is 0.462 (not C_SPEED=0.577) and
        // its width grows by 1.646, failing the preregistered speed/coherence
        // gates. The initial field remains exactly transverse and unmanifested.
        configure_free_wave_terms(rb);
        const double sigmaX = seed::real("packet.sigmaX", double(std::max(3, N / 8)), 1.0, std::max(N * 0.5, 64.0),
            "Packet width (x)", "Gaussian width (lattice sites) of the packet along its direction of travel.");
        const double sigmaT = seed::real("packet.sigmaT", std::max(6.0, N / 4.0), 1.0, std::max(N * 0.5, 64.0),
            "Packet width (transverse)", "Gaussian width (lattice sites) of the packet transverse to its direction of travel.");
        const double ampMult = seed::real("packet.amplitudeMultiplier", 2.0, 0.0, 40.0,
            "Amplitude multiplier", "Multiplier on K_B giving the peak amplitude of the seeded curl-potential packet.");
        const int dir = seed::choice("packet.direction", 1, {{1.0, "+x"}, {-1.0, "-x"}},
            "Propagation direction", "Sign of the packet's seeded travel direction along x.");
        const double carrierK = seed::real("packet.carrierK", 2.0 * PI / (4.0 * sigmaX), 0.0, PI,
            "Carrier wavenumber", "Spatial wavenumber of the cosine carrier riding inside the packet envelope; defaults to a value coupled to the packet width above.");
        const int px = seed::integer("source.x", mc, 0, N - 1,
            "Packet center x", "Lattice x-coordinate of the packet center.");
        const int py = seed::integer("source.y", mc, 0, N - 1,
            "Packet center y", "Lattice y-coordinate of the packet center.");
        const int pz = seed::integer("source.z", mc, 0, N - 1,
            "Packet center z", "Lattice z-coordinate of the packet center.");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        inject_transverse_packet_x(rb, px, py, pz, sigmaX, sigmaT, K_B * ampMult, dir, carrierK, seed_wave_0_phase);
    }
    else if (name == "s0-field-thomson-scattering") {
        // Scenario ID: s0-field-thomson-scattering
        // Physical Purpose: Locked-source linear-superposition null test.
        // The four-arm observatory finds no interaction residual or recoil, so
        // the Thomson-scattering interpretation is closed for this profile.
        configure_locked_coupled_field_terms(rb);

        const int sx = seed::integer("source.x", mc, 0, N - 1,
            "Source x", "Lattice x-coordinate of the locked marker.");
        const int sy = seed::integer("source.y", mc, 0, N - 1,
            "Source y", "Lattice y-coordinate of the locked marker.");
        const int sz = seed::integer("source.z", mc, 0, N - 1,
            "Source z", "Lattice z-coordinate of the locked marker.");
        IP(rb, sx, sy, sz, -1);
        rb.voxels()[rb.lattice().index(sx, sy, sz)].locked = true;

        const int mode_n = seed::integer("packet.modeNumber", 4, 1, std::max(N, 64),
            "Wave mode number", "Integer wavenumber index n of the incident transverse plane wave (k = 2*pi*n/N).");
        const double amp = seed::real("packet.amplitude", 0.05, 0.0, 20.0,
            "Wave amplitude", "Peak amplitude of the incident transverse J_y/W_y plane wave.");
        const double k = 2.0 * PI * static_cast<double>(mode_n) / static_cast<double>(N);
        const double omega = lattice_harmonic_omega(k);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            const double jy = amp * std::sin(k * x);
            const double wy = amp * ((1.0 - std::cos(omega)) * std::sin(k*x)
                                     - std::sin(omega) * std::cos(k*x));
            if (std::fabs(jy) > 1e-12) IF(rb, x, y, z, 0, jy, 0);
            if (std::fabs(wy) > 1e-12) IW(rb, x, y, z, 0, wy, 0);
        }
    }
    else if (name == "s0-field-thomson-unlocked-recoil") {
        // Scenario ID: s0-field-thomson-unlocked-recoil
        // Physical Purpose: Native flux-gradient recoil probe with one mobile
        // negative-polarity manifested site in a transverse lattice wave.
        // Initial Condition Parameters: None.
        // Expected Behaviour: The native emergent-force path produces a
        // deterministic beam-minus-no-beam displacement.  This is not a
        // Thomson cross-section or QED-scattering claim.
        // Discrepancy: The legacy native force path has no resolved recoil;
        // the response depends on the selected emergent-forces extension.
        configure_emergent_recoil_terms(rb);

        const int sx = seed::integer("source.x", mc, 0, N - 1,
            "Source x", "Lattice x-coordinate of the mobile marker.");
        const int sy = seed::integer("source.y", mc, 0, N - 1,
            "Source y", "Lattice y-coordinate of the mobile marker.");
        const int sz = seed::integer("source.z", mc, 0, N - 1,
            "Source z", "Lattice z-coordinate of the mobile marker.");
        IP(rb, sx, sy, sz, -1);
        rb.voxels()[rb.lattice().index(sx, sy, sz)].locked = false;

        const int mode_n = seed::integer("packet.modeNumber", 4, 1, std::max(N, 64),
            "Wave mode number", "Integer wavenumber index n of the incident transverse plane wave (k = 2*pi*n/N).");
        const double amp = seed::real("packet.amplitude", 0.05, 0.0, 20.0,
            "Wave amplitude", "Peak amplitude of the incident transverse J_y/W_y plane wave.");
        const double k = 2.0 * PI * static_cast<double>(mode_n) / static_cast<double>(N);
        const double omega = 2.0 * std::asin(C_SPEED * std::fabs(std::sin(k * 0.5)));
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            const double jy = amp * std::sin(k * x);
            const double wy = amp * ((1.0 - std::cos(omega)) * std::sin(k*x)
                                     - std::sin(omega) * std::cos(k*x));
            if (std::fabs(jy) > 1e-12) IF(rb, x, y, z, 0, jy, 0);
            if (std::fabs(wy) > 1e-12) IW(rb, x, y, z, 0, wy, 0);
        }
    }
    else if (name == "s0-field-spacetime-forcing-boundary") {
        // Scenario ID: s0-field-spacetime-forcing-boundary
        // Physical Purpose: Native point-response locality-cone probe.
        // This is only the production wave map. The diffusion comparison in
        // the legacy demo is a counterfactual and is not an engine scenario.
        configure_free_wave_terms(rb, false);
        const int sx = seed::integer("source.x", mc, 0, N - 1,
            "Source x", "Lattice x-coordinate of the point impulse.");
        const int sy = seed::integer("source.y", mc, 0, N - 1,
            "Source y", "Lattice y-coordinate of the point impulse.");
        const int sz = seed::integer("source.z", mc, 0, N - 1,
            "Source z", "Lattice z-coordinate of the point impulse.");
        const double amp = seed::real("packet.amplitude", 1.0, 0.0, 40.0,
            "Impulse amplitude", "Magnitude of the single-tick J_z and W_z impulse seeded at the source site.");
        IF(rb, sx, sy, sz, 0.0, 0.0, amp);
        IW(rb, sx, sy, sz, 0.0, 0.0, amp);
    }
    else if (name == "s0-field-electric-dipole") {
        // Scenario ID: s0-field-electric-dipole
        // Physical Purpose: Imposed softened opposite-source Coulomb-shaped
        // flux profile. This is imported initial data, not emergent EM.
        configure_static_seed_terms(rb);
        const int sep = seed::integer("geometry.separation", std::max(2, N / 8), 2, std::max(N, 64),
            "Charge separation", "Distance (lattice sites) between the two opposite-polarity source markers.");
        const double ampMult = seed::real("packet.chargeAmplitudeMultiplier", 1.0, 0.0, 40.0,
            "Charge amplitude multiplier", "Multiplier on the fixed ALPHA/(4*pi) Coulomb-shaped prefactor.");
        const int axis = seed::choice("geometry.axis", 0, {{0.0, "x"}, {1.0, "y"}, {2.0, "z"}},
            "Separation axis", "Cartesian axis along which the two opposite-polarity source markers are separated.");
        const int cx = seed::integer("source.x", mc, 0, N - 1,
            "Pair center x", "Lattice x-coordinate about which the two source markers are symmetrically offset.");
        const int cy = seed::integer("source.y", mc, 0, N - 1,
            "Pair center y", "Lattice y-coordinate about which the two source markers are symmetrically offset.");
        const int cz = seed::integer("source.z", mc, 0, N - 1,
            "Pair center z", "Lattice z-coordinate about which the two source markers are symmetrically offset.");
        const int half = sep / 2;
        int posPos[3] = {cx, cy, cz}, negPos[3] = {cx, cy, cz};
        posPos[axis] += half; negPos[axis] -= half;
        IP(rb, posPos[0], posPos[1], posPos[2], +1);
        IP(rb, negPos[0], negPos[1], negPos[2], -1);
        const double alpha_amp = (ALPHA / (4.0 * PI)) * ampMult;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double jx = 0, jy = 0, jz = 0;
            const int p[3] = {x, y, z};
            double d1[3], d2[3];
            for (int i = 0; i < 3; i++) { d1[i] = p[i] - posPos[i]; d2[i] = p[i] - negPos[i]; }
            double r2_1 = d1[0]*d1[0] + d1[1]*d1[1] + d1[2]*d1[2] + 1.0;
            double f1 = alpha_amp / std::pow(r2_1, 1.5);
            jx += f1 * d1[0]; jy += f1 * d1[1]; jz += f1 * d1[2];
            double r2_2 = d2[0]*d2[0] + d2[1]*d2[1] + d2[2]*d2[2] + 1.0;
            double f2 = -alpha_amp / std::pow(r2_2, 1.5);
            jx += f2 * d2[0]; jy += f2 * d2[1]; jz += f2 * d2[2];
            double mag = std::sqrt(jx*jx + jy*jy + jz*jz);
            if (mag > 1e-6) IF(rb, x, y, z, jx, jy, jz);
        }
    }
    else if (name == "s0-field-magnetic-dipole") {
        // Scenario ID: s0-field-magnetic-dipole
        // Physical Purpose: Imposed softened dipole vector-potential ansatz
        // A = mu x r / (r^2 + a^2)^(3/2), with mu parallel to +z.
        // It is not a native derivation of magnetism or a material current loop.
        configure_static_seed_terms(rb);
        const double momentMult = seed::real("packet.momentMultiplier", 1.0, 0.0, 40.0,
            "Moment multiplier", "Multiplier on the fixed K_B/(4*pi) dipole-moment prefactor.");
        const int momentAxis = seed::choice("geometry.momentAxis", 2, {{0.0, "x"}, {1.0, "y"}, {2.0, "z"}},
            "Moment axis", "Cartesian axis the imposed dipole moment mu points along.");
        const double half = (N - 1) / 2.0;
        const double cx = seed::real("source.x", half, 0, N-1, "Dipole center x", "Moves the imposed dipole profile along x.");
        const double cy = seed::real("source.y", half, 0, N-1, "Dipole center y", "Moves the imposed dipole profile along y.");
        const double cz = seed::real("source.z", half, 0, N-1, "Dipole center z", "Moves the imposed dipole profile along z.");
        const double mu_amp = (K_B / (4.0 * PI)) * momentMult;
        const int b = (momentAxis + 1) % 3, c = (momentAxis + 2) % 3;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            const double p[3] = {double(x) - cx, double(y) - cy, double(z) - cz};
            const double denom = std::pow(p[0]*p[0] + p[1]*p[1] + p[2]*p[2] + 1.0, 1.5);
            double comp[3] = {0, 0, 0};
            comp[b] = -mu_amp * p[c] / denom;
            comp[c] =  mu_amp * p[b] / denom;
            if (std::sqrt(comp[0]*comp[0] + comp[1]*comp[1] + comp[2]*comp[2]) > 1e-8)
                IF(rb, x, y, z, comp[0], comp[1], comp[2]);
        }
    }
    else if (name == "s0-field-vortex-line") {
        // Scenario ID: s0-field-vortex-line
        // Physical Purpose: Imposed azimuthal 1/r vector profile about the
        // z-axis. No electromagnetic, fluid, or quantized-vortex identity.
        configure_static_seed_terms(rb);
        const double circMult = seed::real("packet.circulationMultiplier", 4.0, 0.0, 80.0,
            "Circulation multiplier", "Multiplier on K_B giving the imposed azimuthal circulation strength.");
        const double gamma = K_B * circMult;
        const double half = (N - 1) / 2.0;
        const double cx = seed::real("source.x", half, 0, N-1, "Vortex center x", "Moves the axis of the z-invariant vortex along x.");
        const double cy = seed::real("source.y", half, 0, N-1, "Vortex center y", "Moves the axis of the z-invariant vortex along y.");
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - cx, ry = y - cy;
            double r = std::sqrt(rx * rx + ry * ry);
            if (r < 1.0) r = 1.0;
            double mag = gamma / (2.0 * PI * r);
            if (mag < 1e-6) continue;
            IF(rb, x, y, z, -mag * ry / r, mag * rx / r, 0);
        }
    }
    else if (name == "s0-field-shear-layer") {
        // Scenario ID: s0-field-shear-layer
        // Physical Purpose: contrast scenario for the hydrodynamics program. A sheared flux
        // layer J_x(y) = A f(y) g(z), uniform in x, so div J = 0 by construction. Under the
        // single-substrate wave map (wave_propagation only, gauss off) the layer does not
        // diffuse: d'Alembert splits each edge of f into two profiles moving at +-c along y,
        // so each edge's width grows linearly in time (c t), not as sqrt(nu t).
        // Initial Condition Parameters: A = 0.03, delta = 2.5 sites, sigma_z = N/6. f(y) is a
        // periodic DOUBLE layer with edges at y1 = N/4 and y2 = 3N/4:
        //   f(y) = tanh((y - y1)/delta) - tanh((y - y2)/delta) - 1
        // which is -1 at both ends of the periodic y-axis (y = 0 and y = N-1 meet at the wrap
        // with a mismatch of only ~1e-4*A at delta = 2.5) and +1 between the two edges, so the
        // seed closes smoothly on the periodic axis instead of introducing a spurious third,
        // sharper step at the wrap point.
        // Expected Behaviour: <J_x>(y, t) = 0.5 [f(y - c t) + f(y + c t)] to lattice-dispersion
        // accuracy (the z-average of a periodic 3D wave-equation solution obeys the 1D wave
        // equation exactly, so the z-Gaussian envelope drops out of the averaged prediction).
        // Discrepancy: this is a wave field with div J = s, not a fluid; the lattice-gas fluid
        // lives in the strict laboratory (engine/strict/web/hydro/).
        configure_free_wave_terms(rb, false);
        {
            const double A = seed::real("packet.amplitude", 0.030, 0.0, 20.0,
                "Layer amplitude", "Peak magnitude A of the sheared J_x flux layer.");
            const double delta = seed::real("geometry.edgeWidth", 2.5, 0.25, std::max(N * 0.5, 64.0),
                "Edge width", "Width (lattice sites) delta of each tanh edge of the double shear layer.");
            const double sigma_z = seed::real("geometry.sigmaZ", std::max(2.0, N / 6.0), 0.5, std::max(N * 0.5, 64.0),
                "z-envelope width", "Gaussian width (lattice sites) of the layer's envelope along z.");
            const double edgeFracLow = seed::real("geometry.edgeFractionLow", 0.25, 0.0, 1.0,
                "Low edge fraction", "Fraction of N giving the y-position of the first (rising) layer edge.");
            const double edgeFracHigh = seed::real("geometry.edgeFractionHigh", 0.75, 0.0, 1.0,
                "High edge fraction", "Fraction of N giving the y-position of the second (falling) layer edge.");
            const double centerZ = seed::real("source.z", mc, 0, N-1, "Layer envelope center z", "Moves the Gaussian envelope of the shear layer along z.");
            const double y1 = edgeFracLow * N, y2 = edgeFracHigh * N;
            for (int z = 0; z < N; z++)
            for (int y = 0; y < N; y++) {
                const double dz = z - centerZ, gz = std::exp(-(dz * dz) / (2.0 * sigma_z * sigma_z));
                const double f = std::tanh((y - y1) / delta) - std::tanh((y - y2) / delta) - 1.0;
                const double jx = A * f * gz;
                if (std::fabs(jx) < 1e-12) continue;
                for (int x = 0; x < N; x++) IF(rb, x, y, z, jx, 0, 0);
            }
        }
    }

    else if (name == "s0-field-rf-lattice-wave") {
        // Scenario ID: s0-field-rf-lattice-wave
        // Physical Purpose: Selected n=1 transverse lattice harmonic.
        // Initial Condition Parameters: None.
        // Expected Behaviour: exact n=1 plane-average kick-drift pole.
        // Discrepancy: no mapping to SI radio frequency.
        // Ported from JS seedSpectrumComparator (RF lane): modeN=1, sigmaFrac=0.12,
        // amp=0.034, phase=0, y-component, waveSpeed=C_SPEED.
        configure_free_wave_terms(rb, false);
        {
            const double centerY = seed::real("source.y", mc, 0, N-1, "Wave envelope center y", "Moves the transverse Gaussian envelope along y.");
            const double centerZ = seed::real("source.z", mc, 0, N-1, "Wave envelope center z", "Moves the transverse Gaussian envelope along z.");
            const double sigmaFrac = seed::real("geometry.sigmaFraction", 0.12, 0.01, 1.0,
                "Envelope width fraction", "Fraction of N giving the Gaussian transverse envelope width.");
            const double amp_w = seed::real("packet.amplitude", 0.034, 0.0, 20.0,
                "Wave amplitude", "Peak amplitude of the seeded transverse J_y/W_y harmonic.");
            const double ph0 = seed::real("packet.phase", 0.0, -2.0 * PI, 2.0 * PI,
                "Carrier phase", "Phase offset (radians) of the seeded harmonic's spatial carrier.");
            const int rawModeN = seed::integer("packet.modeNumber", 1, 1, std::max(N, 64),
                "Harmonic mode number", "Integer wavenumber index n before the N/2-1 lattice-Nyquist clamp.");
            const double cutSigmas = seed::real("geometry.cutoffSigmas", 2.4, 0.5, 20.0,
                "Envelope cutoff", "Half-width of the transverse dressing box, in units of the envelope width above.");
            const double sigma  = std::max(1.15, N * sigmaFrac);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, rawModeN));
            const double k      = 2.0 * PI * modeN / N;
            const double omega  = 2.0 * std::asin(C_SPEED * std::abs(std::sin(k / 2.0)));
            const double cut    = sigma * cutSigmas, cut2 = cut * cut;
            const int zlo = std::max(0,   (int)std::floor(centerZ - cut));
            const int zhi = std::min(N-1, (int)std::ceil (centerZ + cut));
            const int ylo = std::max(0,   (int)std::floor(centerY - cut));
            const int yhi = std::min(N-1, (int)std::ceil (centerY + cut));
            for (int z = zlo; z <= zhi; z++)
            for (int y = ylo; y <= yhi; y++)
            for (int x = 0;   x < N;    x++) {
                const double dy = y - centerY, dz = z - centerZ;
                const double r2 = dy*dy + dz*dz;
                if (r2 > cut2) continue;
                const double g  = std::exp(-r2 / (2.0 * sigma * sigma));
                if (g < 1e-4) continue;
                const double ph = k * x + ph0;
                const double j  = amp_w * g * std::sin(ph);
                const double w  = amp_w * g * ((1.0 - std::cos(omega)) * std::sin(ph)
                                                - std::sin(omega) * std::cos(ph));
                if (std::fabs(j) > 1e-12) IF(rb, x, y, z, 0, j, 0);
                if (std::fabs(w) > 1e-12) IW(rb, x, y, z, 0, w, 0);
            }
        }
    }
    else if (name == "s0-field-light-lattice-wave") {
        // Scenario ID: s0-field-light-lattice-wave
        // Physical Purpose: Selected n=6 transverse lattice harmonic.
        // Initial Condition Parameters: None.
        // Expected Behaviour: exact n=6 plane-average kick-drift pole.
        // Discrepancy: no mapping to SI light, color, or photon identity.
        // Ported from JS seedSpectrumComparator (light lane): modeN=6, sigmaFrac=0.10,
        // amp=0.032, phase=PI*0.15, y-component, waveSpeed=C_SPEED.
        configure_free_wave_terms(rb, false);
        {
            const double centerY = seed::real("source.y", mc, 0, N-1, "Wave envelope center y", "Moves the transverse Gaussian envelope along y.");
            const double centerZ = seed::real("source.z", mc, 0, N-1, "Wave envelope center z", "Moves the transverse Gaussian envelope along z.");
            const double sigmaFrac = seed::real("geometry.sigmaFraction", 0.10, 0.01, 1.0,
                "Envelope width fraction", "Fraction of N giving the Gaussian transverse envelope width.");
            const double amp_w = seed::real("packet.amplitude", 0.032, 0.0, 20.0,
                "Wave amplitude", "Peak amplitude of the seeded transverse J_y/W_y harmonic.");
            const double ph0 = seed::real("packet.phase", PI * 0.15, -2.0 * PI, 2.0 * PI,
                "Carrier phase", "Phase offset (radians) of the seeded harmonic's spatial carrier.");
            const int rawModeN = seed::integer("packet.modeNumber", 6, 1, std::max(N, 64),
                "Harmonic mode number", "Integer wavenumber index n before the N/2-1 lattice-Nyquist clamp.");
            const double cutSigmas = seed::real("geometry.cutoffSigmas", 2.4, 0.5, 20.0,
                "Envelope cutoff", "Half-width of the transverse dressing box, in units of the envelope width above.");
            const double sigma  = std::max(1.15, N * sigmaFrac);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, rawModeN));
            const double k      = 2.0 * PI * modeN / N;
            const double omega  = 2.0 * std::asin(C_SPEED * std::abs(std::sin(k / 2.0)));
            const double cut    = sigma * cutSigmas, cut2 = cut * cut;
            const int zlo = std::max(0,   (int)std::floor(centerZ - cut));
            const int zhi = std::min(N-1, (int)std::ceil (centerZ + cut));
            const int ylo = std::max(0,   (int)std::floor(centerY - cut));
            const int yhi = std::min(N-1, (int)std::ceil (centerY + cut));
            for (int z = zlo; z <= zhi; z++)
            for (int y = ylo; y <= yhi; y++)
            for (int x = 0;   x < N;    x++) {
                const double dy = y - centerY, dz = z - centerZ;
                const double r2 = dy*dy + dz*dz;
                if (r2 > cut2) continue;
                const double g  = std::exp(-r2 / (2.0 * sigma * sigma));
                if (g < 1e-4) continue;
                const double ph = k * x + ph0;
                const double j  = amp_w * g * std::sin(ph);
                const double w  = amp_w * g * ((1.0 - std::cos(omega)) * std::sin(ph)
                                                - std::sin(omega) * std::cos(ph));
                if (std::fabs(j) > 1e-12) IF(rb, x, y, z, 0, j, 0);
                if (std::fabs(w) > 1e-12) IW(rb, x, y, z, 0, w, 0);
            }
        }
    }
    else if (name == "s0-field-sound-lattice-wave") {
        // Scenario ID: s0-field-sound-lattice-wave
        // Physical Purpose: Closed-negative test of a selected c/8 longitudinal seed.
        // Initial Condition Parameters: None.
        // Expected Behaviour: native C_SPEED pole despite the slow seed momentum.
        // Discrepancy: no acoustic medium exists in the frozen vector-wave sector.
        // Ported from JS seedSpectrumComparator (sound lane): modeN=4, sigmaFrac=0.11,
        // amp=0.030, phase=PI*0.10, x-component (longitudinal), waveSpeed=C_SPEED/8.
        configure_free_wave_terms(rb, false);
        {
            // SOUND_PROXY_SPEED: c/8 is a pedagogical PROXY, not a real acoustic
            // eigenmode. FTD has no sound — the single flux sector re-propagates
            // this wave at c = 1/sqrt(3) (declared [BOUNDARY], FTD-0298/0299); the
            // slow appearance is an initial-condition/visual artifact only. The /8
            // coefficient is imposed initial momentum; it is editable without changing the wave law.
            const double SOUND_PROXY_SPEED = seed::real("packet.seedSpeed", C_SPEED / 8.0, 0, C_SPEED, "Initial wave-momentum speed", "Sets the initial W-to-J relation only. The tick law still propagates at its native speed; this does not create an acoustic medium.", "cells per tick", .001);
            const double centerY = seed::real("source.y", mc, 0, N-1, "Wave envelope center y", "Moves the transverse Gaussian envelope along y.");
            const double centerZ = seed::real("source.z", mc, 0, N-1, "Wave envelope center z", "Moves the transverse Gaussian envelope along z.");
            const double sigmaFrac = seed::real("geometry.sigmaFraction", 0.11, 0.01, 1.0,
                "Envelope width fraction", "Fraction of N giving the Gaussian transverse envelope width.");
            const double amp_w = seed::real("packet.amplitude", 0.030, 0.0, 20.0,
                "Wave amplitude", "Peak amplitude of the seeded longitudinal J_x/W_x harmonic.");
            const double ph0 = seed::real("packet.phase", PI * 0.10, -2.0 * PI, 2.0 * PI,
                "Carrier phase", "Phase offset (radians) of the seeded harmonic's spatial carrier.");
            const int rawModeN = seed::integer("packet.modeNumber", 4, 1, std::max(N, 64),
                "Harmonic mode number", "Integer wavenumber index n before the N/2-1 lattice-Nyquist clamp.");
            const double cutSigmas = seed::real("geometry.cutoffSigmas", 2.4, 0.5, 20.0,
                "Envelope cutoff", "Half-width of the transverse dressing box, in units of the envelope width above.");
            const double sigma  = std::max(1.15, N * sigmaFrac);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, rawModeN));
            const double k      = 2.0 * PI * modeN / N;
            const double seedOmega = 2.0 * SOUND_PROXY_SPEED * std::abs(std::sin(k / 2.0));
            const double cut    = sigma * cutSigmas, cut2 = cut * cut;
            const int zlo = std::max(0,   (int)std::floor(centerZ - cut));
            const int zhi = std::min(N-1, (int)std::ceil (centerZ + cut));
            const int ylo = std::max(0,   (int)std::floor(centerY - cut));
            const int yhi = std::min(N-1, (int)std::ceil (centerY + cut));
            for (int z = zlo; z <= zhi; z++)
            for (int y = ylo; y <= yhi; y++)
            for (int x = 0;   x < N;    x++) {
                const double dy = y - centerY, dz = z - centerZ;
                const double r2 = dy*dy + dz*dz;
                if (r2 > cut2) continue;
                const double g  = std::exp(-r2 / (2.0 * sigma * sigma));
                if (g < 1e-4) continue;
                const double ph = k * x + ph0;
                const double j  = amp_w * g * std::sin(ph);
                const double w  = -seedOmega * amp_w * g * std::cos(ph);
                if (std::fabs(j) > 1e-12) IF(rb, x, y, z, j, 0, 0);
                if (std::fabs(w) > 1e-12) IW(rb, x, y, z, w, 0, 0);
            }
        }
    }
    else if (name == "s0-field-sound-collision") {
        // Scenario ID: s0-field-sound-collision
        // Physical Purpose: Collision setup for two longitudinal proxy packets.
        // Initial Condition Parameters: None.
        // Expected Behaviour: substantial overlap with exact linear
        // superposition and no collision residual.
        // Discrepancy: no acoustic medium, physical sound identity, or
        // nonlinear collision interaction.
        // Ported from JS seedSpectrumComparator (sound-collision: 2 lanes).
        // Left pulse: pulseCenterOffsetFrac=-0.25, right-going (speedMult=+1).
        // Right pulse: pulseCenterOffsetFrac=+0.25, left-going (speedMult=-1).
        configure_free_wave_terms(rb, false);
        {
            // SOUND_PROXY_SPEED: c/8 is a pedagogical PROXY, not a real acoustic
            // eigenmode. FTD has no sound — the single flux sector re-propagates
            // this wave at c = 1/sqrt(3) (declared [BOUNDARY], FTD-0298/0299); the
            // slow appearance is an initial-condition/visual artifact only. The /8
            // coefficient is imposed initial momentum; it is editable without changing the wave law.
            const double SOUND_PROXY_SPEED = seed::real("packet.seedSpeed", C_SPEED / 8.0, 0, C_SPEED, "Initial wave-momentum speed", "Sets the initial W-to-J relation only. The tick law still propagates at its native speed; this does not create an acoustic medium.", "cells per tick", .001);
            const double pulseFrac = seed::real("geometry.pulseFraction", 0.15, 0.01, 1.0,
                "Pulse width fraction", "Fraction of N giving each longitudinal pulse's width along x.");
            const double centerY = seed::real("source.y", mc, 0, N-1, "Wave envelope center y", "Moves the transverse Gaussian envelope along y.");
            const double centerZ = seed::real("source.z", mc, 0, N-1, "Wave envelope center z", "Moves the transverse Gaussian envelope along z.");
            const double sigmaFrac = seed::real("geometry.sigmaFraction", 0.11, 0.01, 1.0,
                "Envelope width fraction", "Fraction of N giving the Gaussian transverse envelope width.");
            const double amp_w = seed::real("packet.amplitude", 0.030, 0.0, 20.0,
                "Wave amplitude", "Peak amplitude of each seeded longitudinal J_x/W_x pulse.");
            const int rawModeN = seed::integer("packet.modeNumber", 4, 1, std::max(N, 64),
                "Harmonic mode number", "Integer wavenumber index n before the N/2-1 lattice-Nyquist clamp.");
            const double cutSigmas = seed::real("geometry.cutoffSigmas", 2.4, 0.5, 20.0,
                "Envelope cutoff", "Half-width of the transverse dressing box, in units of the envelope width above.");
            const double lane0OffsetFrac = seed::real("constituent0.offsetFraction", -0.25, -1.0, 1.0,
                "Lane-0 center offset", "Fraction of N offsetting the first pulse's center x from the lattice midpoint.");
            const int lane0Dir = seed::choice("constituent0.direction", 1, {{1.0, "+x"}, {-1.0, "-x"}},
                "Lane-0 direction", "Sign of the first pulse's seeded travel direction.");
            const double lane1OffsetFrac = seed::real("constituent1.offsetFraction", 0.25, -1.0, 1.0,
                "Lane-1 center offset", "Fraction of N offsetting the second pulse's center x from the lattice midpoint.");
            const int lane1Dir = seed::choice("constituent1.direction", -1, {{1.0, "+x"}, {-1.0, "-x"}},
                "Lane-1 direction", "Sign of the second pulse's seeded travel direction.");
            const double sigma      = std::max(1.15, N * sigmaFrac);
            const double pulseSigma = std::max(1.5, N * pulseFrac * 0.5);
            const int    modeN  = std::max(1, std::min(N / 2 - 1, rawModeN));
            const double k      = 2.0 * PI * modeN / N;
            const double seedOmega = 2.0 * SOUND_PROXY_SPEED * std::abs(std::sin(k / 2.0));
            const double cut    = sigma * cutSigmas, cut2 = cut * cut;
            struct Lane { double offsetFrac; double speedMult; };
            const Lane lanes[2] = {{lane0OffsetFrac, double(lane0Dir)}, {lane1OffsetFrac, double(lane1Dir)}};
            for (const auto& lane : lanes) {
                const double centerX = midF + lane.offsetFrac * N;
                const int zlo = std::max(0,   (int)std::floor(centerZ - cut));
                const int zhi = std::min(N-1, (int)std::ceil (centerZ + cut));
                const int ylo = std::max(0,   (int)std::floor(centerY - cut));
                const int yhi = std::min(N-1, (int)std::ceil (centerY + cut));
                for (int z = zlo; z <= zhi; z++)
                for (int y = ylo; y <= yhi; y++)
                for (int x = 0;   x < N;    x++) {
                    const double dy = y - centerY, dz = z - centerZ;
                    const double r2 = dy*dy + dz*dz;
                    if (r2 > cut2) continue;
                    const double dx  = x - centerX;
                    const double gx  = std::exp(-(dx * dx) / (2.0 * pulseSigma * pulseSigma));
                    const double g   = gx * std::exp(-r2 / (2.0 * sigma * sigma));
                    if (g < 1e-4) continue;
                    const double ph = k * x;
                    const double j  = amp_w * g * std::sin(ph);
                    const double w  = lane.speedMult * (-seedOmega * amp_w * g * std::cos(ph));
                    if (std::fabs(j) > 1e-12) IF(rb, x, y, z, j, 0, 0);
                    if (std::fabs(w) > 1e-12) IW(rb, x, y, z, w, 0, 0);
                }
            }
        }
    }
    else {
        return false;
    }
    return true;
}

}  // namespace ftd
