#include "native_desktop/command_applier.h"
#include "native_desktop/command_queue.h"

#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"
#include "golden_hash.h"

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

namespace {

void seed_gpu(ftd::RenderBridge& rb) {
    rb.set_interactive_gpu_mode(true);
    rb.seed_rng(42);
    rb.set_state(2, 2, 2, 1);
}

using Cmd = ftd::native_desktop::UiCommand;

std::array<std::vector<Cmd>, 6> orders() {
    const Cmd continuity = ftd::native_desktop::RequestContinuity{};
    const Cmd voxel = ftd::native_desktop::InspectVoxel{2, 2, 2};
    const Cmd force = ftd::native_desktop::InspectForce{2, 2, 2};
    return {{
        {continuity, voxel, force},
        {continuity, force, voxel},
        {voxel, continuity, force},
        {voxel, force, continuity},
        {force, continuity, voxel},
        {force, voxel, continuity},
    }};
}

}  // namespace

int main() {
    ftd::test::init("test_ui_fixed_flush_order_gpu");
    ftd::test::section("N5: observer order cannot change continuity provenance");

    std::uint64_t ready_hash = 0;
    int ready_L = -1;
    bool saw_ready = false;

    for (const auto& order : orders()) {
        ftd::RenderBridge rb(5);
        seed_gpu(rb);
        ftd::test::check("GPU backend is active",
                         rb.backend().kind() == ftd::Backend::Kind::Gpu);
        for (int i = 0; i < 4; ++i) rb.tick();

        (void)rb.voxels();

        ftd::native_desktop::CommandQueue queue;
        ftd::native_desktop::SnapshotPublisher publisher;
        ftd::native_desktop::UiBoundaryState state;
        state.publisher = &publisher;
        for (const auto& command : order) queue.push(command);
        ftd::native_desktop::process_ui_boundary(rb, nullptr, queue, state);

        const auto immediate = publisher.acquire();
        ftd::test::check("immediate snapshot exists", immediate != nullptr);
        ftd::test::check("continuity is pending after the host upload",
                         immediate && immediate->continuity.status
                             == ftd::native_desktop::ObservationStatus::PendingAfterHostUpload);

        rb.tick();
        state.did_tick = true;
        ftd::native_desktop::process_ui_boundary(rb, nullptr, queue, state);
        const auto ready = publisher.acquire();
        ftd::test::check("retained continuity becomes ready after the next tick",
                         ready && ready->continuity.status
                             == ftd::native_desktop::ObservationStatus::Ready);
        ftd::test::check("ready continuity has lattice extent",
                         ready && ready->continuity.L == rb.lattice().size());
        const auto hash = ftd::test::compute_state_only_hash(rb);
        if (!saw_ready) {
            ready_hash = hash;
            ready_L = ready->continuity.L;
            saw_ready = true;
        } else {
            ftd::test::check("all observer orders share the same state hash",
                             hash == ready_hash);
            ftd::test::check("all observer orders share continuity extent",
                             ready->continuity.L == ready_L);
        }
    }

    return ftd::test::finalize();
}
