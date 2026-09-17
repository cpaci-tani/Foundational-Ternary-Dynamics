#pragma once
#include "ftd/render_bridge.h"
#include "ftd/dynamical_state_digest.h"
#include <iomanip>
#include <sstream>

inline std::string seed_receipt(ftd::RenderBridge& rb, const std::string& id) {
    ftd::DynamicalStateDigest digest;
    if (!rb.capture_dynamical_state_digest(digest)) throw std::runtime_error("digest unavailable");
    std::uint64_t extras = 1469598103934665603ULL;
    const auto add = [&](std::uint64_t value) { extras = (extras ^ value) * 1099511628211ULL; };
    for (const auto& v : rb.voxels()) {
        add(ftd::digest_detail::canonical_double_bits(v.tau));
        add(ftd::digest_detail::canonical_double_bits(v.phase));
        add(static_cast<std::uint64_t>(v.particle_id));
        add(static_cast<std::uint64_t>(v.pair_id));
    }
    std::ostringstream out;
    out << std::setprecision(17) << rb.lattice().size() << ' ' << id << ' '
        << digest.hash_lo << ' ' << digest.hash_hi << ' ' << digest.nonfinite_value_count
        << ' ' << extras << ' ' << rb.rng_state_hash();
    for (const auto& t : ftd::TOGGLE_SPECS) out << ' ' << (rb.toggles.*t.field);
    const auto& t = rb.toggles;
    out << ' ' << int(t.flux_boundary) << ' ' << int(t.periodic_axis) << ' ' << int(t.bcc_stencil)
        << ' ' << int(t.langevin_site_filter) << ' ' << t.langevin_T << ' ' << t.langevin_gamma
        << ' ' << t.langevin_seed << ' ' << t.coulomb_charge_coupling << ' ' << t.coulomb_source_scale
        << ' ' << t.omega0 << ' ' << t.kinetic_drain << ' ' << rb.dt() << ' ' << rb.sor_iterations();
    const auto& region = rb.flux_cell_region();
    out << ' ' << region.cx << ' ' << region.cy << ' ' << region.cz << ' ' << region.radius;
    const auto& pump = rb.flux_pump_spec();
    out << ' ' << rb.flux_pump_configured() << ' ' << pump.cx << ' ' << pump.cy << ' ' << pump.cz
        << ' ' << pump.major_radius << ' ' << pump.tube_sigma << ' ' << pump.amplitude
        << ' ' << pump.circulation_sign << ' ' << pump.sign_sectors << ' ' << pump.cutoff_sigmas
        << ' ' << rb.flux_pump_ticks_total() << ' ' << rb.flux_pump_period();
    const auto& port = rb.flux_cell_port();
    out << ' ' << rb.flux_cell_port_configured() << ' ' << port.cx << ' ' << port.cy << ' ' << port.cz
        << ' ' << port.nx << ' ' << port.ny << ' ' << port.nz << ' ' << port.radius
        << ' ' << port.surface_offset << ' ' << port.open_tick;
    return out.str();
}

