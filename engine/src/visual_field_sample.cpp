#include "ftd/visual_field_sample.h"

#include "ftd/constants.h"
#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd {
namespace {

constexpr std::size_t kMaxDenseVisualSamples = 262144u;

bool is_vector_kind(VisualFieldKind kind) {
    switch (kind) {
        case VisualFieldKind::Electric:
        case VisualFieldKind::Magnetic:
        case VisualFieldKind::Poynting:
        case VisualFieldKind::FluxVector:
        case VisualFieldKind::Curl:
        case VisualFieldKind::EmForce:
        case VisualFieldKind::GravityForce:
        case VisualFieldKind::StrongForce:
            return true;
        default:
            return false;
    }
}

bool is_interior_kind(VisualFieldKind kind) {
    switch (kind) {
        case VisualFieldKind::Vorticity:
        case VisualFieldKind::Helicity:
        case VisualFieldKind::Kretschmann:
        case VisualFieldKind::Fisher:
        case VisualFieldKind::Coherence:
        case VisualFieldKind::Curl:
            return true;
        default:
            return false;
    }
}

int bounded_visual_stride(int lattice_size, int requested, bool interior) {
    int stride = std::max(1, requested);
    const int extent = std::max(0, lattice_size - (interior ? 2 : 0));
    auto samples_for = [extent](int s) -> std::size_t {
        const std::size_t n = static_cast<std::size_t>((extent + s - 1) / s);
        return n * n * n;
    };
    while (samples_for(stride) > kMaxDenseVisualSamples) ++stride;
    return stride;
}

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
    return is_vector_kind(kind) ? 3u : 1u;
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
    const bool interior = is_interior_kind(kind);
    out.effective_stride = bounded_visual_stride(n, stride, interior);
    out.origin = interior ? 1 : 0;
    if (interior && n < 3) return;

    const int start = out.origin;
    const int end = interior ? n - 1 : n;
    const int axis_count = (std::max(0, end - start) + out.effective_stride - 1)
                         / out.effective_stride;
    const std::size_t max_points = static_cast<std::size_t>(axis_count)
                                 * axis_count * axis_count;
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

    for (int z = start; z < end; z += out.effective_stride) {
        for (int y = start; y < end; y += out.effective_stride) {
            for (int x = start; x < end; x += out.effective_stride) {
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
                    const int bx_end = std::min(x + out.effective_stride, end);
                    const int by_end = std::min(y + out.effective_stride, end);
                    const int bz_end = std::min(z + out.effective_stride, end);
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
