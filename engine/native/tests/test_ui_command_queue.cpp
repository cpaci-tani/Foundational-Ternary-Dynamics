#include "native/command_queue.h"
#include "ftd/test_telemetry.h"

#include <variant>

int main() {
    ftd::test::init("test_ui_command_queue");

    ftd::test::section("physics-parameter commands never coalesce");
    ftd::native::CommandQueue queue;
    const auto s1 = queue.push(ftd::native::SetToggle{"symplectic_leapfrog", true});
    const auto s2 = queue.push(ftd::native::SetDt{0.5});
    const auto s3 = queue.push(ftd::native::SetToggle{"symplectic_leapfrog", false});
    const auto s4 = queue.push(ftd::native::SetDt{0.7});
    const auto items = queue.drain();
    ftd::test::check("four-command W4 drain keeps every mutation", items.size() == 4);
    ftd::test::check("sequence numbers stay monotone FIFO",
                     s1 == 1 && s2 == 2 && s3 == 3 && s4 == 4
                     && items[0].seq == 1 && items[1].seq == 2
                     && items[2].seq == 3 && items[3].seq == 4);
    ftd::test::check("first toggle is on",
                     std::get<ftd::native::SetToggle>(items[0].command).value);
    ftd::test::check("first dt is 0.5",
                     std::get<ftd::native::SetDt>(items[1].command).dt == 0.5);
    ftd::test::check("second toggle is off",
                     !std::get<ftd::native::SetToggle>(items[2].command).value);
    ftd::test::check("second dt is 0.7",
                     std::get<ftd::native::SetDt>(items[3].command).dt == 0.7);

    ftd::test::section("view/request commands keep last write at last position");
    ftd::native::CommandQueue view;
    view.push(ftd::native::InspectVoxel{1, 2, 3});
    view.push(ftd::native::SetToggle{"damping", true});
    view.push(ftd::native::InspectVoxel{4, 5, 6});
    view.push(ftd::native::RequestField{ftd::VisualFieldKind::State, 2});
    view.push(ftd::native::RequestField{ftd::VisualFieldKind::FluxVector, 4});
    const auto coalesced = view.drain();
    ftd::test::check("coalescing drops earlier InspectVoxel and RequestField",
                     coalesced.size() == 3);
    ftd::test::check("toggle remains between coalesced view commands",
                     std::holds_alternative<ftd::native::SetToggle>(
                         coalesced[0].command));
    const auto voxel = std::get<ftd::native::InspectVoxel>(coalesced[1].command);
    ftd::test::check("last InspectVoxel is retained at its original later position",
                     voxel.x == 4 && voxel.y == 5 && voxel.z == 6
                     && coalesced[1].seq == 3);
    const auto field = std::get<ftd::native::RequestField>(coalesced[2].command);
    ftd::test::check("last RequestField is retained",
                     field.kind == ftd::VisualFieldKind::FluxVector && field.stride == 4);

    ftd::test::section("empty drain is empty");
    ftd::test::check("second drain has no leftover commands", queue.drain().empty());

    return ftd::test::finalize();
}
