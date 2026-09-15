#include "ftd/link_energy_observer.h"

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace ftd {
namespace {

void capture(const RenderBridge& rb, std::vector<double>& out) {
    const auto& vox = rb.voxels();
    out.resize(vox.size() * 3);
    for (std::size_t i = 0; i < vox.size(); ++i) {
        out[3 * i] = vox[i].flux.x;
        out[3 * i + 1] = vox[i].flux.y;
        out[3 * i + 2] = vox[i].flux.z;
    }
}

const char* unavailable_reason(const RenderBridge& rb) {
    const auto& t = rb.toggles;
    if (rb.backend_kind() == Backend::Kind::Gpu) return "GPU backend: the observer runs on the CPU tick path only";
    if (t.verlet_wave_integrator) return "verlet_wave_integrator changes the wave step";
    if (t.symplectic_leapfrog) return "symplectic_leapfrog changes the wave step";
    if (t.lorentz_period2_floquet) return "lorentz_period2_floquet varies the wave coefficient per tick";
    if (t.lorentz_bcc_time_floquet) return "lorentz_bcc_time_floquet varies the wave coefficient per tick";
    if (!t.wave_propagation) return "wave_propagation is off";
    return nullptr;
}

std::uint32_t exchange_terms(const RenderBridge& rb) {
    const auto& t = rb.toggles;
    std::uint32_t m = 0;
    if (t.langevin) m |= LinkExchangeTerm::Langevin;
    if (t.gauss_projection) m |= LinkExchangeTerm::GaussProjection;
    if (t.damping) m |= LinkExchangeTerm::Damping;
    if (t.genesis) m |= LinkExchangeTerm::Genesis;
    if (t.coupling) m |= LinkExchangeTerm::Coupling;
    if (t.absorbing_boundary) m |= LinkExchangeTerm::AbsorbingBoundary;
    if (t.flux_boundary != FluxBoundaryMode::Periodic) m |= LinkExchangeTerm::NonPeriodicBoundary;
    if (t.flux_pump || t.flux_cell_port) m |= LinkExchangeTerm::FluxCell;
    return m;
}

}  // namespace

void LinkEnergyObserver::set_enabled(bool on) {
    if (enabled_ == on) return;
    enabled_ = on;
    have_cur_ = false;
    have_prev_ = false;
    status_ = on ? LinkEnergyStatus::Warming : LinkEnergyStatus::Off;
    reason_.clear();
}

void LinkEnergyObserver::before_tick(const RenderBridge& rb) {
    if (!enabled_) return;
    if (const char* why = unavailable_reason(rb)) {
        status_ = LinkEnergyStatus::Unavailable;
        reason_ = why;
        have_cur_ = false;
        have_prev_ = false;
        return;
    }
    capture(rb, scratch_);
    const int L = rb.lattice().size();
    const bool continuous = have_cur_ && L == L_ && scratch_.size() == cur_.size()
        && std::memcmp(scratch_.data(), cur_.data(), scratch_.size() * sizeof(double)) == 0;
    if (!continuous) {
        // An external write (scenario load, injection, resize) happened since the
        // last committed tick: restart the history from the present state.
        L_ = L;
        cur_.swap(scratch_);
        have_cur_ = true;
        have_prev_ = false;
        status_ = LinkEnergyStatus::Warming;
        reason_.clear();
    }
}

void LinkEnergyObserver::after_tick(const RenderBridge& rb) {
    if (!enabled_ || status_ == LinkEnergyStatus::Unavailable || !have_cur_) return;
    capture(rb, next_);
    tick_ = static_cast<std::uint64_t>(rb.current_tick());
    exchange_terms_ = exchange_terms(rb);
    if (have_prev_) {
        compute(C_WAVE * C_WAVE);
        status_ = LinkEnergyStatus::Ok;
    } else {
        status_ = LinkEnergyStatus::Warming;
    }
    prev_.swap(cur_);
    cur_.swap(next_);
    have_prev_ = true;
}

// u0 = u(n-1), u1 = u(n), u2 = u(n+1). Spec section 5.1:
//   e_i(n+1/2) = 1/2 |u2_i - u1_i|^2 + 1/4 c2 sum_j w (u2_i - u2_j).(u1_i - u1_j)
//   F(i->j)    = 1/4 c2 w (u1_i - u1_j).(s_i + s_j),  s = u2 - u0
//   r_i        = e_i(n+1/2) - e_i(n-1/2) + sum_j F(i->j)
void LinkEnergyObserver::compute(double c2) {
    const int L = L_;
    const std::size_t N = static_cast<std::size_t>(L) * L * L;
    const std::vector<double>& u0 = prev_;
    const std::vector<double>& u1 = cur_;
    const std::vector<double>& u2 = next_;
    links_d_.assign(9 * N, 0.0);
    residual_d_.assign(N, 0.0);
    e_before_.assign(N, 0.0);
    e_after_.assign(N, 0.0);
    outflow_.assign(N, 0.0);
    auto wrap = [L](int v) { return (v % L + L) % L; };
    for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y) for (int z = 0; z < L; ++z) {
        const std::size_t i = static_cast<std::size_t>((x * L + y) * L + z);
        double kin_before = 0.0, kin_after = 0.0;
        for (int c = 0; c < 3; ++c) {
            const double db = u1[3 * i + c] - u0[3 * i + c];
            const double da = u2[3 * i + c] - u1[3 * i + c];
            kin_before += db * db;
            kin_after += da * da;
        }
        e_before_[i] += 0.5 * kin_before;
        e_after_[i] += 0.5 * kin_after;
        for (int k = 0; k < 9; ++k) {
            const auto& d = LINK_DISPLACEMENT[k];
            const std::size_t j = static_cast<std::size_t>(
                (wrap(x + d[0]) * L + wrap(y + d[1])) * L + wrap(z + d[2]));
            double pot_before = 0.0, pot_after = 0.0, flow = 0.0;
            for (int c = 0; c < 3; ++c) {
                const double g0 = u0[3 * i + c] - u0[3 * j + c];
                const double g1 = u1[3 * i + c] - u1[3 * j + c];
                const double g2 = u2[3 * i + c] - u2[3 * j + c];
                pot_before += g1 * g0;
                pot_after += g2 * g1;
                flow += g1 * ((u2[3 * i + c] - u0[3 * i + c]) + (u2[3 * j + c] - u0[3 * j + c]));
            }
            const double q = 0.25 * c2 * LINK_WEIGHT[k];
            e_before_[i] += q * pot_before; e_before_[j] += q * pot_before;
            e_after_[i] += q * pot_after;   e_after_[j] += q * pot_after;
            const double F = q * flow;
            links_d_[9 * i + k] = F;
            outflow_[i] += F;
            outflow_[j] -= F;
        }
    }
    max_local_change_ = 0.0;
    max_residual_ = 0.0;
    double H = 0.0;
    for (std::size_t i = 0; i < N; ++i) {
        const double de = e_after_[i] - e_before_[i];
        residual_d_[i] = de + outflow_[i];
        max_local_change_ = std::max(max_local_change_, std::abs(de));
        max_residual_ = std::max(max_residual_, std::abs(residual_d_[i]));
        H += e_after_[i];
    }
    invariant_ = H;
    links_.assign(links_d_.begin(), links_d_.end());
    residual_.assign(residual_d_.begin(), residual_d_.end());
}

}  // namespace ftd
