#include "ftd/visual_field_sample.h"

#include "ftd/constants.h"
#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"
#include "ftd/visual_sample_grid.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd {
namespace {

double rho_at(const FieldSoA& fields, int idx) {
    const double x = fields.flux_x[static_cast<std::size_t>(idx)];
    const double y = fields.flux_y[static_cast<std::size_t>(idx)];
    const double z = fields.flux_z[static_cast<std::size_t>(idx)];
    return x * x + y * y + z * z;
}

void append_position(VisualFieldSample& out, int x, int y, int z) {
    out.positions.push_back(static_cast<float>(x) + 0.5f);
    out.positions.push_back(static_cast<float>(y) + 0.5f);
    out.positions.push_back(static_cast<float>(z) + 0.5f);
}

void append_vector(VisualFieldSample& out, const Vec3& value,
                   int x, int y, int z) {
    append_position(out, x, y, z);
    out.data.push_back(static_cast<float>(value.x));
    out.data.push_back(static_cast<float>(value.y));
    out.data.push_back(static_cast<float>(value.z));
}

void append_scalar(VisualFieldSample& out, double value,
                   int x, int y, int z) {
    append_position(out, x, y, z);
    out.data.push_back(static_cast<float>(value));
}

}  // namespace

bool parse_visual_field_kind(std::string_view name, VisualFieldKind& out) {
    struct Row { std::string_view name; VisualFieldKind kind; };
    static constexpr Row rows[] = {
        {"e", VisualFieldKind::Electric},
        {"b", VisualFieldKind::Magnetic},
        {"poynting", VisualFieldKind::Poynting},
        {"divJ", VisualFieldKind::Divergence},
        {"fluxVector", VisualFieldKind::FluxVector},
        {"vorticity", VisualFieldKind::Vorticity},
        {"helicity", VisualFieldKind::Helicity},
        {"kretschmann", VisualFieldKind::Kretschmann},
        {"latency", VisualFieldKind::Latency},
        {"fisher", VisualFieldKind::Fisher},
        {"coherence", VisualFieldKind::Coherence},
        {"curlJ", VisualFieldKind::Curl},
        {"state", VisualFieldKind::State},
        {"gaussResidual", VisualFieldKind::GaussResidual},
        {"em", VisualFieldKind::EmForce},
        {"gravity", VisualFieldKind::GravityForce},
        {"strong", VisualFieldKind::StrongForce},
        {"poissonLatency", VisualFieldKind::PoissonLatency},
    };
    for (const auto& row : rows) {
        if (row.name == name) {
            out = row.kind;
            return true;
        }
    }
    return false;
}

const char* visual_field_kind_name(VisualFieldKind kind) {
    switch (kind) {
        case VisualFieldKind::Electric: return "e";
        case VisualFieldKind::Magnetic: return "b";
        case VisualFieldKind::Poynting: return "poynting";
        case VisualFieldKind::Divergence: return "divJ";
        case VisualFieldKind::FluxVector: return "fluxVector";
        case VisualFieldKind::Vorticity: return "vorticity";
        case VisualFieldKind::Helicity: return "helicity";
        case VisualFieldKind::Kretschmann: return "kretschmann";
        case VisualFieldKind::Latency: return "latency";
        case VisualFieldKind::Fisher: return "fisher";
        case VisualFieldKind::Coherence: return "coherence";
        case VisualFieldKind::Curl: return "curlJ";
        case VisualFieldKind::State: return "state";
        case VisualFieldKind::GaussResidual: return "gaussResidual";
        case VisualFieldKind::EmForce: return "em";
        case VisualFieldKind::GravityForce: return "gravity";
        case VisualFieldKind::StrongForce: return "strong";
        case VisualFieldKind::PoissonLatency: return "poissonLatency";
    }
    return "unknown";
}

std::uint32_t visual_field_components(VisualFieldKind kind) {
    return is_vector_field_kind(kind) ? 3u : 1u;
}

void RenderBridge::copy_visual_field_sample(VisualFieldKind kind, int stride,
                                            VisualFieldSample& out) {
    assert_sim_thread();
    if (backend_) {
        backend_->flush_host_mutations();
        if (backend_->copy_visual_field_sample(kind, stride, out)) return;
    }

    out = {};
    out.components = visual_field_components(kind);
    const int n = lattice_.size();

    // Center-anchored sample grid, shared with the CUDA + WASM samplers so all
    // three agree on which voxels are sampled (see visual_sample_grid.h).
    const VisualSampleGrid grid = visual_sample_grid(n, stride, is_interior_field_kind(kind));
    out.effective_stride = grid.stride;
    out.origin = grid.origin;
    if (grid.count == 0) return;  // lattice too small (e.g. interior kind on n < 3)

    const std::size_t max_points = static_cast<std::size_t>(grid.count)
                                 * grid.count * grid.count;
    out.positions.reserve(max_points * 3u);
    out.data.reserve(max_points * out.components);

    const auto& fields_ref = fields();
    const auto& ternary = ternary_field();
    const auto& force = force_diag();

    double max_rho = 0.0;
    std::vector<float> latency_grid;
    if (kind == VisualFieldKind::Latency || kind == VisualFieldKind::Kretschmann) {
        for (int i = 0, total = n * n * n; i < total; ++i)
            max_rho = std::max(max_rho, rho_at(fields_ref, i));
        if (max_rho < 1e-30) return;
        if (kind == VisualFieldKind::Kretschmann) {
            latency_grid.resize(static_cast<std::size_t>(n) * n * n);
            const double inv = 1.0 / max_rho;
            for (int i = 0, total = n * n * n; i < total; ++i) {
                latency_grid[static_cast<std::size_t>(i)] = static_cast<float>(
                    std::sqrt(std::min(rho_at(fields_ref, i) * inv, 0.998)));
            }
        }
    }

    for (int z = grid.origin; z < grid.end(); z += grid.stride) {
        for (int y = grid.origin; y < grid.end(); y += grid.stride) {
            for (int x = grid.origin; x < grid.end(); x += grid.stride) {
                // A regular point sampler can miss a one-voxel Wilson loop,
                // IC4 seed, or vortex core whenever the bounded visualization
                // stride is >1.  FluxVector instead represents each regular
                // output cell by the strongest canonical-flux site in its
                // bounded source block.  The position deliberately remains
                // the block anchor so FTS2 origin/stride metadata and FTV2's
                // dense-bin reconstruction stay a regular-grid contract.
                if (kind == VisualFieldKind::FluxVector) {
                    double best_rho = 0.0;
                    Vec3 best{};
                    const int bx_end = std::min(x + grid.stride, grid.end());
                    const int by_end = std::min(y + grid.stride, grid.end());
                    const int bz_end = std::min(z + grid.stride, grid.end());
                    for (int bz = z; bz < bz_end; ++bz) {
                        for (int by = y; by < by_end; ++by) {
                            for (int bx = x; bx < bx_end; ++bx) {
                                const int block_idx = lattice_.index(bx, by, bz);
                                const double block_rho = rho_at(fields_ref, block_idx);
                                if (block_rho > best_rho) {
                                    best_rho = block_rho;
                                    best = fields_ref.flux_at(
                                        static_cast<std::size_t>(block_idx));
                                }
                            }
                        }
                    }
                    if (best_rho >= 1e-30) append_vector(out, best, x, y, z);
                    continue;
                }

                const int idx = lattice_.index(x, y, z);
                const Vec3 j = fields_ref.flux_at(static_cast<std::size_t>(idx));

                switch (kind) {
                    case VisualFieldKind::Electric: {
                        const Vec3 value = fields_ref.wave_vel_at(
                            static_cast<std::size_t>(idx)) * -1.0;
                        if (value.mag() >= 1e-15) append_vector(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::Magnetic:
                    case VisualFieldKind::Curl: {
                        const Vec3 value = curl_flux_op(fields_ref, lattice_, idx);
                        if (value.mag() >= 1e-15) append_vector(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::Poynting: {
                        const Vec3 e = fields_ref.wave_vel_at(
                            static_cast<std::size_t>(idx)) * -1.0;
                        const Vec3 b = curl_flux_op(fields_ref, lattice_, idx);
                        const double c2 = C_SPEED * C_SPEED;
                        const Vec3 value{
                            c2 * (e.y * b.z - e.z * b.y),
                            c2 * (e.z * b.x - e.x * b.z),
                            c2 * (e.x * b.y - e.y * b.x),
                        };
                        if (value.mag() >= 1e-15) append_vector(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::Divergence: {
                        const double value = divergence_flux_op(fields_ref, lattice_, idx);
                        if (std::abs(value) >= 1e-15) append_scalar(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::FluxVector:
                        // Handled by bounded block-representative sampling
                        // above, including the stride==1 case.
                        break;
                    case VisualFieldKind::Vorticity: {
                        const double value = curl_flux_op(fields_ref, lattice_, idx).mag();
                        if (value >= 1e-15) append_scalar(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::Helicity: {
                        const Vec3 curl = curl_flux_op(fields_ref, lattice_, idx);
                        const double value = j.x * curl.x + j.y * curl.y + j.z * curl.z;
                        if (std::abs(value) >= 1e-15) append_scalar(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::Coherence: {
                        const Vec3 curl = curl_flux_op(fields_ref, lattice_, idx);
                        const double jm = j.mag();
                        const double cm = curl.mag();
                        if (jm >= 1e-10 && cm >= 1e-10) {
                            append_scalar(out,
                                (j.x * curl.x + j.y * curl.y + j.z * curl.z)
                                    / (jm * cm), x, y, z);
                        }
                        break;
                    }
                    case VisualFieldKind::Fisher: {
                        const auto nb = lattice_.neighbors_6(idx);
                        const double rho = rho_at(fields_ref, idx);
                        if (rho < 1e-8) break;
                        const double dx = 0.5 * (rho_at(fields_ref, nb[0]) - rho_at(fields_ref, nb[1]));
                        const double dy = 0.5 * (rho_at(fields_ref, nb[2]) - rho_at(fields_ref, nb[3]));
                        const double dz = 0.5 * (rho_at(fields_ref, nb[4]) - rho_at(fields_ref, nb[5]));
                        const double value = (dx * dx + dy * dy + dz * dz) / rho;
                        if (value >= 1e-12) append_scalar(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::Latency: {
                        const double value = std::sqrt(
                            std::min(rho_at(fields_ref, idx) / max_rho, 0.998));
                        if (value >= 1e-6) append_scalar(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::Kretschmann: {
                        const auto face = lattice_.neighbors_6(idx);
                        const auto edge = lattice_.neighbors_12(idx);
                        double lap = -4.0 * latency_grid[static_cast<std::size_t>(idx)];
                        for (int q : face) lap += latency_grid[static_cast<std::size_t>(q)] / 3.0;
                        for (int q : edge) lap += latency_grid[static_cast<std::size_t>(q)] / 6.0;
                        const double value = lap * lap;
                        if (value >= 1e-18) append_scalar(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::State: {
                        const int value = ternary.state_at(idx);
                        if (value != 0) append_scalar(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::GaussResidual: {
                        const double value = divergence_flux_op(fields_ref, lattice_, idx)
                                           - static_cast<double>(ternary.state_at(idx));
                        if (std::abs(value) >= 1e-6) append_scalar(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::EmForce: {
                        const Vec3 value = force[static_cast<std::size_t>(idx)].f_coulomb
                                         + force[static_cast<std::size_t>(idx)].f_magnetic;
                        if (value.mag() >= 1e-15) append_vector(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::GravityForce: {
                        const Vec3 value = force[static_cast<std::size_t>(idx)].f_gravity;
                        if (value.mag() >= 1e-15) append_vector(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::StrongForce: {
                        const Vec3 value = force[static_cast<std::size_t>(idx)].f_strong;
                        if (value.mag() >= 1e-15) append_vector(out, value, x, y, z);
                        break;
                    }
                    case VisualFieldKind::PoissonLatency: {
                        const double value = voxels_[static_cast<std::size_t>(idx)].latency;
                        if (value >= 1e-15) append_scalar(out, value, x, y, z);
                        break;
                    }
                }
            }
        }
    }
}

}  // namespace ftd
