#include "ui/panels/panels.h"

namespace ftd::native {
namespace {

class LogPanel final : public Panel {
public:
    const char* id() const override { return "log"; }
    const char* title() const override { return "Log"; }
    DockSlot default_slot() const override { return DockSlot::Physics; }
    void draw_contents(PanelContext&) override {
        ImGui::TextUnformatted("Session log ships with Phase 7.");
    }
};

}  // namespace

std::unique_ptr<Panel> make_log_panel() {
    return std::make_unique<LogPanel>();
}

}  // namespace ftd::native
