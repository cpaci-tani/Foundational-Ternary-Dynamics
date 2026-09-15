#pragma once
// Observation-only exact link energy current of the reference engine's wave
// step (spec 2026-09-15 native transport overlays, sections 4-5). The engine
// runs the v1 continuous-J wave map; this observer reports what that map moves
// along its 18 stencil links and how much energy other terms exchange off them.
// It never writes engine state.
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ftd {

class RenderBridge;

// The nine links each site owns, in the order fixed by spec section 4. A link
// value is positive when transport runs from the owner toward owner + d_k.
inline constexpr std::array<std::array<int, 3>, 9> LINK_DISPLACEMENT = {{
    {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
    {1, 1, 0}, {1, -1, 0}, {1, 0, 1}, {1, 0, -1}, {0, 1, 1}, {0, 1, -1}}};
inline constexpr std::array<double, 9> LINK_WEIGHT = {
    1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0,
    1.0 / 6.0, 1.0 / 6.0, 1.0 / 6.0, 1.0 / 6.0, 1.0 / 6.0, 1.0 / 6.0};

enum class LinkEnergyStatus : std::uint8_t { Off = 0, Warming = 1, Ok = 2, Unavailable = 3 };

// Enabled terms that can exchange energy at a site without moving it along a link.
namespace LinkExchangeTerm {
inline constexpr std::uint32_t Langevin = 1u << 0;
inline constexpr std::uint32_t GaussProjection = 1u << 1;
inline constexpr std::uint32_t Damping = 1u << 2;
inline constexpr std::uint32_t Genesis = 1u << 3;
inline constexpr std::uint32_t Coupling = 1u << 4;
inline constexpr std::uint32_t AbsorbingBoundary = 1u << 5;
inline constexpr std::uint32_t NonPeriodicBoundary = 1u << 6;
inline constexpr std::uint32_t FluxCell = 1u << 7;
inline constexpr std::uint32_t DeBroglieClock = 1u << 8;
inline constexpr std::uint32_t EwBackgroundSweep = 1u << 9;
inline constexpr std::uint32_t PairProductionTransmutation = 1u << 10;
}  // namespace LinkExchangeTerm

class LinkEnergyObserver {
public:
    void set_enabled(bool on);
    bool enabled() const { return enabled_; }
    // Called at the start of a tick: detects any external write since the last
    // committed tick and restarts the history if one happened.
    void before_tick(const RenderBridge& rb);
    // Called after a committed CPU tick: computes the current once three
    // consecutive committed states are available.
    void after_tick(const RenderBridge& rb);

    LinkEnergyStatus status() const { return status_; }
    const std::string& reason() const { return reason_; }
    int lattice_size() const { return L_; }
    std::uint64_t tick() const { return tick_; }
    const std::vector<float>& links() const { return links_; }
    const std::vector<float>& residual() const { return residual_; }
    const std::vector<double>& links_exact() const { return links_d_; }
    const std::vector<double>& residual_exact() const { return residual_d_; }
    double invariant() const { return invariant_; }
    double max_local_change() const { return max_local_change_; }
    double max_residual() const { return max_residual_; }
    double closure() const { return max_local_change_ > 0.0 ? max_residual_ / max_local_change_ : 0.0; }
    std::uint32_t active_exchange_terms() const { return exchange_terms_; }

private:
    void compute(double c2, bool periodic);

    bool enabled_ = false;
    bool have_cur_ = false;
    bool have_prev_ = false;
    int L_ = 0;
    std::uint64_t tick_ = 0;
    LinkEnergyStatus status_ = LinkEnergyStatus::Off;
    std::string reason_;
    std::vector<double> prev_, cur_, next_, scratch_;
    std::vector<double> e_before_, e_after_, outflow_;
    std::vector<double> links_d_, residual_d_;
    std::vector<float> links_, residual_;
    double invariant_ = 0.0;
    double max_local_change_ = 0.0;
    double max_residual_ = 0.0;
    std::uint32_t exchange_terms_ = 0;
};

}  // namespace ftd
