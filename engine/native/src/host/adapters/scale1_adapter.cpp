// host/adapters/scale1_adapter.cpp — Scale 1 (ParticleEngine) behind the seam.
//
// The R1 validation adapter: a SECOND ScaleAdapter whose engine (ParticleEngine)
// is a real ftd::ScaleEngine with continuous positions and analytical forces —
// structurally nothing like Scale 0's voxel-field RenderBridge. It exercises the
// same contract the host drives generically: boot/tick/capture/build_snapshot/
// apply, plus the live engine() the seam reserves for ScaleEngine-based scales.

#include "native/host/adapters/scale1_adapter.h"

#include "ftd/particle_engine.h"
#include "ftd/scenario_meta.h"

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace ftd::native {
namespace {

// green = positive charge, red = negative — mirrors Scale 0's capture() colours.
void colour_for_charge(int charge, NativeParticle& p) {
    if (charge >= 0) {
        p.r = 0.29f; p.g = 0.87f; p.b = 0.50f;
    } else {
        p.r = 0.97f; p.g = 0.44f; p.b = 0.44f;
    }
}

}  // namespace

Scale1Adapter::Scale1Adapter() = default;
Scale1Adapter::~Scale1Adapter() = default;

ftd::ScaleEngine* Scale1Adapter::engine() { return engine_.get(); }

void Scale1Adapter::seed_scenario(const std::string& id) {
    engine_->clear();
    const double c = box_ * 0.5;  // 16 for box 32 — where the Scale-0 camera targets

    if (id == "s1-two-charges") {
        // A minimal opposite-charge pair (the "couple of charged particles" the
        // brief allows), given small counter-velocities so total momentum ≈ 0.
        engine_->add_particle(1,  Vec3{c - 4.0, c, c}, Vec3{0.0,  0.04, 0.0});
        engine_->add_particle(-1, Vec3{c + 4.0, c, c}, Vec3{0.0, -0.04, 0.0});
        return;
    }

    // Default "s1-hydrogen-cloud": a locked positive core plus a shell of six
    // mobile negatives on the ±axes. Coulomb at r=5, soft=1 is O(1e-5)/tick², so
    // the constellation stays visible over a capture window (no annihilation)
    // while the engine still advances real ticks with nonzero KE.
    engine_->add_locked_particle(1, Vec3{c, c, c});
    const double R = 5.0;
    engine_->add_particle(-1, Vec3{c + R, c, c}, Vec3{0.0,  0.05, 0.0});
    engine_->add_particle(-1, Vec3{c - R, c, c}, Vec3{0.0, -0.05, 0.0});
    engine_->add_particle(-1, Vec3{c, c + R, c}, Vec3{-0.05, 0.0, 0.0});
    engine_->add_particle(-1, Vec3{c, c - R, c}, Vec3{ 0.05, 0.0, 0.0});
    engine_->add_particle(-1, Vec3{c, c, c + R}, Vec3{0.05, 0.0,  0.0});
    engine_->add_particle(-1, Vec3{c, c, c - R}, Vec3{-0.05, 0.0, 0.0});
}

void Scale1Adapter::boot(const ftd::ScenarioMeta& meta, const RunConfig& cfg,
                         BootReport& out) {
    (void)cfg;  // Scale 1 has no lattice_size / flux_boundary knobs to honor.
    std::string id = meta.id ? meta.id : "";
    // Normalize: a non-Scale-1 id (e.g. the Scale-0 default carried into a fresh
    // boot) falls back to the default cloud seed. This is Scale 1's own light
    // version of the W9 unknown-scenario handling.
    const bool known = (id == "s1-two-charges" || id == "s1-hydrogen-cloud");
    if (!known) id = "s1-hydrogen-cloud";

    engine_ = std::make_unique<ftd::ParticleEngine>();
    engine_->set_use_gpu(false);  // R1: CPU only (N is tiny, GPU path never triggers)
    seed_scenario(id);

    scenario_   = id;
    status_     = id + " (" + std::to_string(engine_->entity_count()) + " particles)";
    last_count_ = static_cast<std::uint32_t>(engine_->entity_count());

    out.status = ReloadStatus::Success;
    out.scenario = scenario_;
    out.status_line = status_;
}

void Scale1Adapter::tick() { engine_->tick(); }

int Scale1Adapter::current_tick() const { return engine_->current_tick(); }

bool Scale1Adapter::is_observation(const ScalePayload& payload) const {
    const Scale1Cmd* s1 = std::get_if<Scale1Cmd>(&payload);
    return s1 && std::holds_alternative<InspectParticle1>(*s1);
}

bool Scale1Adapter::is_host_write(const ScalePayload& /*payload*/) const {
    return false;  // no harness host-write commands at Scale 1 yet
}

ApplyResult Scale1Adapter::apply(const ScalePayload& payload, ParameterJournal& /*journal*/,
                                 int /*apply_tick*/, LoopControl& /*loop*/) {
    ApplyResult result;
    const Scale1Cmd* s1 = std::get_if<Scale1Cmd>(&payload);
    if (!s1) {
        result.ok = false;
        result.error_code = 1;
        result.message = "Scale 1 received a non-Scale-1 payload";
        return result;
    }
    std::visit(
        [&](const auto& c) {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_same_v<T, Seed1>) {
                seed_scenario(c.scenario.empty() ? scenario_ : c.scenario);
                if (!c.scenario.empty()) scenario_ = c.scenario;
            } else if constexpr (std::is_same_v<T, AddParticle1>) {
                engine_->add_particle(c.charge >= 0 ? 1 : -1,
                                      Vec3{c.x, c.y, c.z});
            }
        },
        *s1);
    return result;
}

void Scale1Adapter::begin_boundary() { snapshot_ = Scale1Snapshot{}; }

bool Scale1Adapter::observe(const ScalePayload& payload) {
    const Scale1Cmd* s1 = std::get_if<Scale1Cmd>(&payload);
    if (!s1) return false;
    const InspectParticle1* ip = std::get_if<InspectParticle1>(s1);
    if (!ip) return false;
    // observe() runs before build_snapshot() (see ScaleHost::process_ui_boundary),
    // and build_snapshot() only touches the energy/status fields, so the
    // inspection payload written here survives into the published snapshot.
    const std::vector<ftd::Particle>& ps = engine_->particles();
    if (ip->index < 0 || ip->index >= static_cast<int>(ps.size())) {
        snapshot_.insp_present = false;   // out-of-range ⇒ a valid "cleared" read
        return true;
    }
    const ftd::Particle& p = ps[static_cast<std::size_t>(ip->index)];
    snapshot_.insp_present = true;
    snapshot_.insp_index = ip->index;
    snapshot_.insp_charge = p.charge;
    snapshot_.insp_locked = p.locked;
    snapshot_.insp_pos[0] = p.position.x;
    snapshot_.insp_pos[1] = p.position.y;
    snapshot_.insp_pos[2] = p.position.z;
    snapshot_.insp_vel[0] = p.velocity.x;
    snapshot_.insp_vel[1] = p.velocity.y;
    snapshot_.insp_vel[2] = p.velocity.z;
    return true;
}

void Scale1Adapter::build_snapshot(const DataNeeds& /*needs*/) {
    const ftd::ParticleDiagnostics d = engine_->diagnostics();
    snapshot_.particle_count = d.particle_count;
    snapshot_.total_energy = d.total_energy;
    snapshot_.total_ke = d.total_ke;
    snapshot_.total_pe = d.total_pe;
    snapshot_.status = status_;
}

ScaleSnapshot Scale1Adapter::take_scale_snapshot() {
    ScaleSnapshot out = std::move(snapshot_);
    snapshot_ = Scale1Snapshot{};
    return out;
}

NativeFrame Scale1Adapter::capture() {
    NativeFrame frame;
    frame.tick = engine_->current_tick();
    frame.lattice_size = box_;  // frames the presenter camera on the box centre
    frame.total_manifested = static_cast<std::uint32_t>(engine_->entity_count());
    frame.particles.reserve(engine_->particles().size());
    for (const ftd::Particle& p : engine_->particles()) {
        NativeParticle np;
        np.x = static_cast<float>(p.position.x);
        np.y = static_cast<float>(p.position.y);
        np.z = static_cast<float>(p.position.z);
        colour_for_charge(p.charge, np);
        np.size = p.locked ? 0.9f : 0.6f;
        frame.particles.push_back(np);
    }
    frame.scenario = scenario_;
    frame.backend = backend_name();
    frame.status = status_;
    last_count_ = frame.total_manifested;
    return frame;
}

}  // namespace ftd::native
