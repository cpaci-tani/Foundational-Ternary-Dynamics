#include "ui/panels/panels.h"

namespace ftd::native {
namespace {

class FieldsPanel final : public Panel {
public:
    const char* id() const override { return "fields"; }
    const char* title() const override { return "Fields"; }
    DockSlot default_slot() const override { return DockSlot::Physics; }
    void draw_contents(PanelContext&) override {
        ImGui::TextUnformatted("Field kinds ship in Phase 5.");
    }
};

}  // namespace

std::unique_ptr<Panel> make_fields_panel() {
    return std::make_unique<FieldsPanel>();
}

}  // namespace ftd::native
