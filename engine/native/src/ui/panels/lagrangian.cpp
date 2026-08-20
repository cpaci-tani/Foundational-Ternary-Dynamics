#include "ui/panels/panels.h"

namespace ftd::native {
namespace {

class LagrangianPanel final : public Panel {
public:
    const char* id() const override { return "lagrangian"; }
    const char* title() const override { return "Lagrangian"; }
    DockSlot default_slot() const override { return DockSlot::Instruments; }
    void draw_contents(PanelContext&) override {
        ImGui::TextUnformatted("Lagrangian readout ships in Phase 6.");
    }
};

}  // namespace

std::unique_ptr<Panel> make_lagrangian_panel() {
    return std::make_unique<LagrangianPanel>();
}

}  // namespace ftd::native
