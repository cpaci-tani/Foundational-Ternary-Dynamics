#include "ftd/eft/dual_cell_flow.h"

#include <cmath>

namespace ftd {
namespace eft {

double canonical_flux_energy(const DualCellFields& fields,
                             double cell_volume,
                             double face_area) {
    if (fields.L <= 0 || face_area == 0.0) return 0.0;
    double sum = 0.0;
    const double inv_area = 1.0 / face_area;
    for (int i = 0; i < fields.total_sites(); ++i) {
        const double jx = fields.phi_x[static_cast<size_t>(i)] * inv_area;
        const double jy = fields.phi_y[static_cast<size_t>(i)] * inv_area;
        const double jz = fields.phi_z[static_cast<size_t>(i)] * inv_area;
        sum += jx * jx + jy * jy + jz * jz;
    }
    return 0.5 * cell_volume * sum;
}

double native_static_response_coefficient(double operator_symbol,
                                          double inverse_kernel_symbol) {
    if (inverse_kernel_symbol == 0.0) return 0.0;
    return operator_symbol / inverse_kernel_symbol;
}

double canonical_current_flux_vertex(const DualCellContinuity& current,
                                     const DualCellFields& flux,
                                     double cell_volume,
                                     double face_area) {
    if (current.L <= 0 || flux.L <= 0 || current.L != flux.L ||
        face_area == 0.0) {
        return 0.0;
    }
    double sum = 0.0;
    const double inv_area = 1.0 / face_area;
    for (int i = 0; i < current.total_sites(); ++i) {
        const double ix = current.current_x[static_cast<size_t>(i)] * inv_area;
        const double iy = current.current_y[static_cast<size_t>(i)] * inv_area;
        const double iz = current.current_z[static_cast<size_t>(i)] * inv_area;
        const double px = flux.phi_x[static_cast<size_t>(i)] * inv_area;
        const double py = flux.phi_y[static_cast<size_t>(i)] * inv_area;
        const double pz = flux.phi_z[static_cast<size_t>(i)] * inv_area;
        sum += ix * px + iy * py + iz * pz;
    }
    return cell_volume * sum;
}

NativeB2FlowReport measure_native_b2_flow(const DualCellFields& fine,
                                          double tolerance) {
    NativeB2FlowReport report;
    report.fine_L = fine.L;
    report.total_source_fine = total_source(fine);
    report.gauss_residual_fine = max_gauss_residual(fine);
    report.flux_energy_fine = canonical_flux_energy(fine, 1.0, 1.0);

    const DualCellFields coarse = block_dual_cell_b2(fine);
    report.coarse_L = coarse.L;
    report.total_source_coarse = total_source(coarse);
    report.gauss_residual_coarse = max_gauss_residual(coarse);
    report.flux_energy_coarse = canonical_flux_energy(coarse, 8.0, 4.0);
    report.flux_energy_ratio =
        (report.flux_energy_fine > 0.0)
            ? report.flux_energy_coarse / report.flux_energy_fine
            : 0.0;
    report.source_conserved =
        (report.total_source_fine == report.total_source_coarse);
    report.gauss_preserved =
        report.gauss_residual_fine <= tolerance &&
        report.gauss_residual_coarse <= tolerance;
    return report;
}

}  // namespace eft
}  // namespace ftd
