#include "ui/panels/panels.h"

namespace ftd::native {
namespace {

const char* boundary_label(ftd::FluxBoundaryMode mode) {
    switch (mode) {
        case ftd::FluxBoundaryMode::Periodic:
            return "Periodic";
        case ftd::FluxBoundaryMode::Reflective:
            return "Reflective";
        case ftd::FluxBoundaryMode::Dispersal:
            return "Dispersal";
        default:
            return "Periodic";
    }
}

class PhysicsTermsPanel final : public Panel {
public:
    const char* id() const override { return "physics_terms"; }
    const char* title() const override { return "Term toggles"; }
    DockSlot default_slot() const override { return DockSlot::Physics; }

    void draw_contents(PanelContext& ctx) override {
        ImGui::TextUnformatted("Full 43-row table ships in Phase 4.");
        const auto current = ctx.snapshot.term_toggles.flux_boundary;
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::BeginCombo("Flux boundary", boundary_label(current))) {
            for (int i = 0; i < 3; ++i) {
                const auto mode = static_cast<ftd::FluxBoundaryMode>(i);
                const bool selected = mode == current;
                if (ImGui::Selectable(boundary_label(mode), selected)) {
                    ctx.commands.push(SetBoundary{mode});
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
};

}  // namespace

std::unique_ptr<Panel> make_physics_terms_panel() {
    return std::make_unique<PhysicsTermsPanel>();
}

}  // namespace ftd::native
