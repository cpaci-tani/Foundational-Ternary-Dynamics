#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"
#include "ftd/volumetric_measure.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace {

ftd::LagrangianDiag directed_reference(const ftd::RenderBridge& rb) {
    ftd::LagrangianDiag d;
    const auto& lattice = rb.lattice();
    const auto& voxels = rb.voxels();
    const int n = static_cast<int>(lattice.total_sites());

    for (int i = 0; i < n; ++i) {
        const auto& v = voxels[i];
        const double div_j = rb.divergence_flux(i);
        const double rho = static_cast<double>(v.state);
        const double fk = ftd::field_kinetic_term(v.wave_vel);
        const double fg = ftd::field_gradient_term(
            v.flux, lattice.neighbors_6(i), lattice.neighbors_12(i), voxels);
        const double bi = ftd::born_infeld_term(v);
        const double coupling = ftd::coupling_term(v, div_j);
        const double velocity_coupling = ftd::velocity_coupling_term(v);
        const double gauss = ftd::gauss_term(div_j, rho);
        const double dissipation = ftd::rayleigh_dissipation(v);

        d.field_kinetic_sum += ftd::integrate_voxel_density(fk);
        d.field_gradient_sum += ftd::integrate_voxel_density(fg);
        d.born_infeld_sum += ftd::integrate_voxel_density(bi);
        d.coupling_sum += ftd::integrate_voxel_density(coupling);
        d.velocity_coupling_sum +=
            ftd::integrate_voxel_density(velocity_coupling);
        d.gauss_sum += ftd::integrate_voxel_density(gauss);
        d.dissipation_sum += ftd::integrate_voxel_density(dissipation);
        d.total_lagrangian += ftd::integrate_voxel_density(
            fk + fg + bi + coupling + velocity_coupling + gauss);
        d.total_hamiltonian += ftd::integrate_voxel_density(
            ftd::hamiltonian_density(v, div_j, rho));

        const double residual = div_j - rho;
        d.gauss_violation += residual * residual;
        d.max_gauss_error = std::max(d.max_gauss_error, std::abs(residual));
        d.total_flux_mag += v.density();
        d.total_wave_energy += ftd::integrate_voxel_density(
            ftd::quadratic_field_energy_density(v.wave_vel.mag2()));
        if (v.state != 0) {
            ++d.manifested_count;
            if (v.locked) ++d.locked_count;
        }
    }
    d.total_action = d.total_lagrangian;
    return d;
}

std::uint64_t next_bits(std::uint64_t& state) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

double signed_unit(std::uint64_t& state) {
    constexpr double inv = 1.0 / 1048575.0;
    return 2.0 * static_cast<double>(next_bits(state) & 0xfffffU) * inv - 1.0;
}

void seed_snapshot(ftd::RenderBridge& rb, std::uint64_t seed) {
    auto& voxels = rb.voxels();
    for (std::size_t i = 0; i < voxels.size(); ++i) {
        auto& v = voxels[i];
        v.flux = {signed_unit(seed), signed_unit(seed), signed_unit(seed)};
        v.wave_vel = {0.2 * signed_unit(seed), 0.2 * signed_unit(seed),
                      0.2 * signed_unit(seed)};
        v.velocity = {0.1 * signed_unit(seed), 0.1 * signed_unit(seed),
                      0.1 * signed_unit(seed)};
        v.latency = 0.15 * std::abs(signed_unit(seed));
        const int state_code = static_cast<int>(next_bits(seed) % 5U);
        v.state = state_code == 0 ? -1 : (state_code == 1 ? 1 : 0);
        v.locked = v.state != 0 && ((next_bits(seed) & 1U) != 0U);
    }
}

void check_close_scaled(const char* label, double actual, double expected) {
    const double scale = std::max({1.0, std::abs(actual), std::abs(expected)});
    ftd::test::check_close(label, actual, expected, 2e-12 * scale);
}

void check_all_fields(const ftd::LagrangianDiag& actual,
                      const ftd::LagrangianDiag& expected) {
    check_close_scaled("field kinetic", actual.field_kinetic_sum,
                       expected.field_kinetic_sum);
    check_close_scaled("field gradient", actual.field_gradient_sum,
                       expected.field_gradient_sum);
    check_close_scaled("Born-Infeld", actual.born_infeld_sum,
                       expected.born_infeld_sum);
    check_close_scaled("coupling", actual.coupling_sum, expected.coupling_sum);
    check_close_scaled("velocity coupling", actual.velocity_coupling_sum,
                       expected.velocity_coupling_sum);
    check_close_scaled("Gauss term", actual.gauss_sum, expected.gauss_sum);
    check_close_scaled("dissipation", actual.dissipation_sum,
                       expected.dissipation_sum);
    check_close_scaled("total Lagrangian", actual.total_lagrangian,
                       expected.total_lagrangian);
    check_close_scaled("Hamiltonian", actual.total_hamiltonian,
                       expected.total_hamiltonian);
    check_close_scaled("action", actual.total_action, expected.total_action);
    check_close_scaled("Gauss violation", actual.gauss_violation,
                       expected.gauss_violation);
    check_close_scaled("maximum Gauss error", actual.max_gauss_error,
                       expected.max_gauss_error);
    check_close_scaled("total flux magnitude", actual.total_flux_mag,
                       expected.total_flux_mag);
    check_close_scaled("total wave energy", actual.total_wave_energy,
                       expected.total_wave_energy);
    ftd::test::check("manifested count",
                     actual.manifested_count == expected.manifested_count);
    ftd::test::check("locked count", actual.locked_count == expected.locked_count);
    check_close_scaled("cell volume", actual.cell_volume, expected.cell_volume);
}

}  // namespace

int main() {
    ftd::test::init("lagrangian_global_gradient");

    ftd::test::section("pairs-once sum matches directed half-share reference");
    // L=17 spans 3 fixed reduction chunks; L=33 spans 18.  The smaller
    // lattices pin periodic multiplicities and odd/even behavior.
    constexpr std::array<int, 8> sizes{{1, 2, 3, 4, 5, 8, 17, 33}};
    constexpr std::array<ftd::FluxBoundaryMode, 3> boundary_modes{{
        ftd::FluxBoundaryMode::Periodic,
        ftd::FluxBoundaryMode::Reflective,
        ftd::FluxBoundaryMode::Dispersal,
    }};
    for (const int size : sizes) {
        for (const auto boundary : boundary_modes) {
            ftd::RenderBridge rb(size);
            rb.force_cpu();
            rb.toggles.flux_boundary = boundary;
            seed_snapshot(rb, 0x9e3779b97f4a7c15ULL
                              ^ static_cast<std::uint64_t>(size));
            check_all_fields(ftd::compute_lagrangian_diagnostics(rb),
                             directed_reference(rb));
        }
    }

    ftd::test::section("small periodic multiplicities remain exact");
    for (const int size : {1, 2, 3}) {
        ftd::RenderBridge rb(size);
        rb.force_cpu();
        rb.toggles.disable_all();
        rb.voxel_at(0, 0, 0).flux = {1.25, -0.5, 0.75};
        const auto actual = ftd::compute_lagrangian_diagnostics(rb);
        const auto expected = directed_reference(rb);
        check_close_scaled("small-L gradient multiplicity",
                           actual.field_gradient_sum,
                           expected.field_gradient_sum);
    }

    return ftd::test::finalize();
}
