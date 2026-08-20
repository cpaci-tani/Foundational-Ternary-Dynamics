#include "ui/panels/panels.h"

#include <algorithm>
#include <string>

namespace ftd::native {
namespace {

constexpr int kLatticeChoices[] = {9, 17, 25, 32, 33, 49, 64, 96, 128, 256};

class RunConfigPanel final : public Panel {
public:
    const char* id() const override { return "run_config"; }
    const char* title() const override { return "Run config"; }
    DockSlot default_slot() const override { return DockSlot::Setup; }

    void draw_contents(PanelContext& ctx) override {
        const int live_l = ctx.snapshot.knobs.lattice_size;
        if (staged_lattice_ == 0) staged_lattice_ = live_l > 0 ? live_l : 32;

        ImGui::TextUnformatted("Lattice");
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::BeginCombo("##lattice", std::to_string(staged_lattice_).c_str())) {
            for (int size : kLatticeChoices) {
                const bool selected = size == staged_lattice_;
                if (ImGui::Selectable(std::to_string(size).c_str(), selected)) {
                    staged_lattice_ = size;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("Apply lattice", ImVec2(-1.0f, 0.0f))) {
            const int n = std::max(4, std::min(256, staged_lattice_));
            ctx.commands.push(SetLatticeSize{n});
            ctx.commands.push(ApplyReboot{});
        }
        ImGui::Text("L live %d   dt %.3f", live_l, ctx.snapshot.knobs.dt);
    }

private:
    int staged_lattice_ = 0;
};

}  // namespace

std::unique_ptr<Panel> make_run_config_panel() {
    return std::make_unique<RunConfigPanel>();
}

}  // namespace ftd::native
