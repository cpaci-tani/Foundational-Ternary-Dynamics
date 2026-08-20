#include "ui/panels/panels.h"

namespace ftd::native {
namespace {

class AuditPanel final : public Panel {
public:
    const char* id() const override { return "audit"; }
    const char* title() const override { return "Audit"; }
    DockSlot default_slot() const override { return DockSlot::Instruments; }
    void draw_contents(PanelContext&) override {
        ImGui::TextUnformatted("Energy audit ships in Phase 6.");
    }
};

}  // namespace

std::unique_ptr<Panel> make_audit_panel() {
    return std::make_unique<AuditPanel>();
}

}  // namespace ftd::native
