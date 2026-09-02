// host/adapters/scale1_adapter.cpp — Scale 1 (ParticleEngine) behind the seam.
//
// The R1 validation adapter: a SECOND ScaleAdapter whose engine (ParticleEngine)
// is a real ftd::ScaleEngine with continuous positions and analytical forces —
// structurally nothing like Scale 0's voxel-field RenderBridge. It exercises the
// same contract the host drives generically: boot/tick/capture/build_snapshot/
// apply, plus the live engine() the seam reserves for ScaleEngine-based scales.

#include "native/host/adapters/scale1_adapter.h"

#include "ftd/particle_engine.h"
#include "native/scenario_catalog.h"  // ftd::native::ScenarioMeta (self-contained; not the untracked ftd/scenario_meta.h)

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
    native_replay_ = (id == "s1-native-m3-replay");
    if (native_replay_) {
        native_replay_snapshot_ = NativeMatterObserver::m3_registered_replay();
        snapshot_ = native_replay_snapshot_;
        return;
    }
    const double c = box_ * 0.5;  // 16 for box 32 — where the Scale-0 camera targets

    if (id == "s1-two-charges") {
        // A minimal opposite-charge pair (the "couple of charged particles" the
        // brief allows), given small counter-velocities so total momentum ≈ 0.
        engine_->add_particle(1,  Vec3{c - 4.0, c, c}, Vec3{0.0,  0.04, 0.0});
        engine_->add_particle(-1, Vec3{c + 4.0, c, c}, Vec3{0.0, -0.04, 0.0});
        return;
    }

    // "s1-effective-charge-cloud": an explicitly imposed effective laboratory
    // seed. It is not called hydrogen because no atomic state is recovered.
    engine_->add_locked_particle(1, Vec3{c, c, c});
    const double R = 5.0;
    engine_->add_particle(-1, Vec3{c + R, c, c}, Vec3{0.0,  0.05, 0.0});
    engine_->add_particle(-1, Vec3{c - R, c, c}, Vec3{0.0, -0.05, 0.0});
    engine_->add_particle(-1, Vec3{c, c + R, c}, Vec3{-0.05, 0.0, 0.0});
    engine_->add_particle(-1, Vec3{c, c - R, c}, Vec3{ 0.05, 0.0, 0.0});
    engine_->add_particle(-1, Vec3{c, c, c + R}, Vec3{0.05, 0.0,  0.0});
    engine_->add_particle(-1, Vec3{c, c, c - R}, Vec3{-0.05, 0.0, 0.0});
}

void Scale1Adapter::boot(const ScenarioMeta& meta, const RunConfig& cfg,
                         BootReport& out) {
    (void)cfg;  // Scale 1 has no lattice_size / flux_boundary knobs to honor.
    std::string id = meta.id ? meta.id : "";
    const bool known = (id == "s1-native-m3-replay" || id == "s1-two-charges"
                        || id == "s1-effective-charge-cloud");
    if (!known) id = "s1-native-m3-replay";

    engine_ = std::make_unique<ftd::ParticleEngine>();
    engine_->set_use_gpu(false);  // R1: CPU only (N is tiny, GPU path never triggers)
    seed_scenario(id);

    scenario_   = id;
    const int count = native_replay_
        ? static_cast<int>(native_replay_snapshot_.objects.size()) : engine_->entity_count();
    status_ = native_replay_
        ? "FTD-0760 qualified selected relational-matter replay; read-only"
        : id + " (" + std::to_string(count) + " effective records)";
    last_count_ = static_cast<std::uint32_t>(count);

    out.status = ReloadStatus::Success;
    out.scenario = scenario_;
    out.status_line = status_;
}

void Scale1Adapter::tick() {
    if (!native_replay_) engine_->tick();
}

int Scale1Adapter::current_tick() const {
    return native_replay_ ? static_cast<int>(native_replay_snapshot_.core.tick)
                          : engine_->current_tick();
}

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
                if (native_replay_) {
                    result.ok = false;
                    result.error_code = 2;
                    result.message = "Native Matter replay is read-only";
                    return;
                }
                engine_->add_particle(c.charge >= 0 ? 1 : -1,
                                      Vec3{c.x, c.y, c.z});
            }
        },
        *s1);
    return result;
}

void Scale1Adapter::begin_boundary() {
    snapshot_ = native_replay_ ? native_replay_snapshot_
                               : Scale1Snapshot{};
}

bool Scale1Adapter::observe(const ScalePayload& payload) {
    const Scale1Cmd* s1 = std::get_if<Scale1Cmd>(&payload);
    if (!s1) return false;
    const InspectParticle1* ip = std::get_if<InspectParticle1>(s1);
    if (!ip) return false;
    // observe() runs before build_snapshot() (see ScaleHost::process_ui_boundary),
    // and build_snapshot() only touches the energy/status fields, so the
    // inspection payload written here survives into the published snapshot.
    const int count = native_replay_ ? static_cast<int>(snapshot_.objects.size())
                                     : static_cast<int>(engine_->particles().size());
    if (ip->index < 0 || ip->index >= count) {
        snapshot_.insp_present = false;   // out-of-range ⇒ a valid "cleared" read
        return true;
    }
    if (native_replay_) {
        const auto& p = snapshot_.objects[static_cast<std::size_t>(ip->index)];
        snapshot_.insp_present = true;
        snapshot_.insp_index = ip->index;
        snapshot_.insp_charge = p.effective_state;
        snapshot_.insp_locked = p.locked;
        snapshot_.insp_pos[0] = p.position.x;
        snapshot_.insp_pos[1] = p.position.y;
        snapshot_.insp_pos[2] = p.position.z;
        snapshot_.insp_vel[0] = p.velocity.x;
        snapshot_.insp_vel[1] = p.velocity.y;
        snapshot_.insp_vel[2] = p.velocity.z;
        return true;
    }
    const std::vector<ftd::Particle>& ps = engine_->particles();
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
    if (native_replay_) {
        snapshot_.status = status_;
        return;
    }
    const bool insp_present = snapshot_.insp_present;
    const int insp_index = snapshot_.insp_index;
    const int insp_charge = snapshot_.insp_charge;
    const bool insp_locked = snapshot_.insp_locked;
    const double insp_pos[3] = {snapshot_.insp_pos[0], snapshot_.insp_pos[1], snapshot_.insp_pos[2]};
    const double insp_vel[3] = {snapshot_.insp_vel[0], snapshot_.insp_vel[1], snapshot_.insp_vel[2]};
    snapshot_ = engine_->snapshot(scenario_, backend_name());
    snapshot_.status = status_;
    snapshot_.insp_present = insp_present;
    snapshot_.insp_index = insp_index;
    snapshot_.insp_charge = insp_charge;
    snapshot_.insp_locked = insp_locked;
    for (int axis = 0; axis < 3; ++axis) {
        snapshot_.insp_pos[axis] = insp_pos[axis];
        snapshot_.insp_vel[axis] = insp_vel[axis];
    }
}

ScaleSnapshot Scale1Adapter::take_scale_snapshot() {
    ScaleSnapshot out = std::move(snapshot_);
    snapshot_ = Scale1Snapshot{};
    return out;
}

NativeFrame Scale1Adapter::capture() {
    NativeFrame frame;
    frame.tick = current_tick();
    frame.lattice_size = box_;  // frames the presenter camera on the box centre
    if (native_replay_) {
        const double c = box_ * 0.5;
        frame.total_manifested = static_cast<std::uint32_t>(native_replay_snapshot_.objects.size());
        frame.particles.reserve(native_replay_snapshot_.objects.size());
        for (const auto& p : native_replay_snapshot_.objects) {
            NativeParticle np;
            np.x = static_cast<float>(p.position.x + c);
            np.y = static_cast<float>(p.position.y + c);
            np.z = static_cast<float>(p.position.z + c);
            colour_for_charge(p.effective_state, np);
            np.size = p.constituent ? 0.75f : 0.6f;
            frame.particles.push_back(np);
        }
    } else {
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
    }
    frame.scenario = scenario_;
    frame.backend = backend_name();
    frame.status = status_;
    last_count_ = frame.total_manifested;
    return frame;
}

}  // namespace ftd::native
