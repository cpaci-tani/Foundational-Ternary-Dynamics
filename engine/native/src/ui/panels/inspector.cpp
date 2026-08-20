#include "ui/panels/panels.h"

namespace ftd::native {
namespace {

class InspectorPanel final : public Panel {
public:
    const char* id() const override { return "inspector"; }
    const char* title() const override { return "Inspector"; }
    DockSlot default_slot() const override { return DockSlot::Instruments; }
    void draw_contents(PanelContext&) override {
        ImGui::TextUnformatted("Voxel inspect ships in Phase 5.");
    }
};

}  // namespace

std::unique_ptr<Panel> make_inspector_panel() {
    return std::make_unique<InspectorPanel>();
}

}  // namespace ftd::native
