#include "ftd/render_bridge.h"
#include "ftd/ws_command_validation.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {
int failures = 0;
void check(const std::string& name, bool passed) {
    if (!passed) { ++failures; std::cerr << "FAIL: " << name << '\n'; }
}
void test_size(int size) {
    ftd::RenderBridge cpu(size); cpu.force_cpu();
    auto& voxels = cpu.voxels();
    for (std::size_t i = 0; i < voxels.size(); ++i) {
        const int p = ((i % size) + ((i / size) % size) + i / (size * size)) & 1;
        if (!p) voxels[i].flux = {3.0, 4.0, 0.0};
    }
    const auto reference = cpu.capture_flux_sectors();
    const auto sites = static_cast<std::uint64_t>(size)*size*size;
    check("even population", reference.sectors[0].total_sites == (sites+1)/2);
    check("odd population", reference.sectors[1].total_sites == sites/2);
    check("known norm", reference.sectors[0].squared_norm == 25.0*((sites+1)/2));
    check("zero sector", reference.sectors[1].squared_norm == 0.0 && reference.sectors[1].nonzero_sites == 0);
    check("component maximum", reference.sectors[0].max_abs_component == 4.0);
#ifdef FTD_ENABLE_CUDA
    ftd::RenderBridge gpu(size); gpu.set_interactive_gpu_mode(true);
    check("CUDA active", gpu.backend_kind() == ftd::Backend::Kind::Gpu);
    gpu.voxels() = voxels;
    ftd::DynamicalStateDigest before, after;
    check("before digest", gpu.capture_dynamical_state_digest(before));
    const auto result = gpu.capture_flux_sectors();
    check("after digest", gpu.capture_dynamical_state_digest(after));
    check("read preserves hash", before.hash_lo == after.hash_lo && before.hash_hi == after.hash_hi
          && before.state_version == after.state_version);
    for (int p = 0; p < 2; ++p) {
        check("CPU/GPU sites", result.sectors[p].total_sites == reference.sectors[p].total_sites);
        check("CPU/GPU nonzero", result.sectors[p].nonzero_sites == reference.sectors[p].nonzero_sites);
        check("CPU/GPU norm", result.sectors[p].squared_norm == reference.sectors[p].squared_norm);
        check("CPU/GPU maximum", result.sectors[p].max_abs_component == reference.sectors[p].max_abs_component);
    }
    check("finite field", result.nonfinite_value_count == 0);
    gpu.voxels()[0].flux = {std::numeric_limits<double>::quiet_NaN(), 0, 0};
    check("GPU NaN detected", gpu.capture_flux_sectors().nonfinite_value_count > 0);
    gpu.voxels()[0].flux = {std::numeric_limits<double>::max(), 0, 0};
    check("GPU overflow detected", gpu.capture_flux_sectors().nonfinite_value_count > 0);
    gpu.voxels()[0].flux = {1e-200, 0, 0};
    const auto tiny = gpu.capture_flux_sectors();
    check("GPU underflow stays nonzero", tiny.sectors[0].nonzero_sites == reference.sectors[0].nonzero_sites);
#endif
}
}
int main() {
    for (int size : {4, 5, 17, 97}) test_size(size);
    ftd::FluxSectors values; values.lattice_size = 4;
    ftd::accumulate_flux_sector(values, 0, 1e-200, 0, 0);
    check("underflow is nonzero", values.sectors[0].nonzero_sites == 1 && values.sectors[0].squared_norm == 0);
    ftd::accumulate_flux_sector(values, 1, std::numeric_limits<double>::infinity(), 0, 0);
    check("nonfinite rejected", values.nonfinite_value_count == 1);
    values.sectors[0].squared_norm = std::numeric_limits<double>::infinity();
    ftd::validate_flux_sector_sums(values);
    check("overflow flag", values.nonfinite_value_count == 2 && values.sectors[0].squared_norm == 0);
    std::cout << "flux sectors failures: " << failures << '\n';
    return failures ? 1 : 0;
}
