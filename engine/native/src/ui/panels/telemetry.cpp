#include "ui/panels/panels.h"

namespace ftd::native {
namespace {

class TelemetryPanel final : public Panel {
public:
    const char* id() const override { return "telemetry"; }
    const char* title() const override { return "Telemetry"; }
    DockSlot default_slot() const override { return DockSlot::Instruments; }
    void draw_contents(PanelContext&) override {
        ImGui::TextUnformatted("Charts ship in Phase 6.");
    }
};

}  // namespace

std::unique_ptr<Panel> make_telemetry_panel() {
    return std::make_unique<TelemetryPanel>();
}

}  // namespace ftd::native
