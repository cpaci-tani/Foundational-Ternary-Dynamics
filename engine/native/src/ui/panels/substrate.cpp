#include "ui/panels/panels.h"

#include "ftd/constants.h"

#include <algorithm>
#include <cstdint>
#include <string>

namespace ftd::native {
namespace {

class SubstratePanel final : public Panel {
public:
    const char* id() const override { return "substrate"; }
    const char* title() const override { return "Substrate"; }
    DockSlot default_slot() const override { return DockSlot::Setup; }

    void draw_contents(PanelContext& ctx) override {
        const int live_l = ctx.snapshot.knobs.lattice_size;
        if (live_l > 0 && !centered_) {
            x_ = y_ = z_ = live_l / 2;
            centered_ = true;
        }
        const int max_i = live_l > 0 ? live_l - 1 : 0;

        ImGui::TextUnformatted("Inject");
        if (ImGui::Button("Particle", ImVec2(-1.0f, 0.0f))) {
            ctx.commands.push(InjectWavepacket{x_, y_, z_, state_});
        }
        if (ImGui::Button("Wave", ImVec2(-1.0f, 0.0f))) {
            ctx.commands.push(InjectWavepacket{x_, y_, z_, state_});
        }
        if (ImGui::Button("Flux", ImVec2(-1.0f, 0.0f))) {
            ctx.commands.push(
                InjectFluxAdd{x_, y_, z_, ftd::K_B * 0.8, 0.0, 0.0});
        }
        if (ImGui::Button("Pair", ImVec2(-1.0f, 0.0f))) {
            ctx.commands.push(
                CreateEntangledPair{x_, y_, z_, ftd::K_B, 0.0, 0.0});
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Position");
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputInt("X", &x_);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputInt("Y", &y_);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputInt("Z", &z_);
        if (ImGui::Button("Center", ImVec2(-1.0f, 0.0f)) && live_l > 0) {
            x_ = y_ = z_ = live_l / 2;
        }
        if (max_i > 0) {
            x_ = std::clamp(x_, 0, max_i);
            y_ = std::clamp(y_, 0, max_i);
            z_ = std::clamp(z_, 0, max_i);
        }

        ImGui::Separator();
        ImGui::TextUnformatted("State");
        if (ImGui::RadioButton("+1", state_ > 0)) state_ = 1;
        ImGui::SameLine();
        if (ImGui::RadioButton("-1", state_ < 0)) state_ = -1;

        ImGui::Separator();
        ImGui::TextUnformatted("Field");
        if (ImGui::Button("Clear field", ImVec2(-1.0f, 0.0f))) {
            ctx.commands.push(ClearField{});
        }
        if (ImGui::Button("Random flux", ImVec2(-1.0f, 0.0f))) {
            ctx.commands.push(SeedRandomFlux{});
        }
        ImGui::TextDisabled("Random flux is not replayable.");
    }

private:
    int x_ = 16;
    int y_ = 16;
    int z_ = 16;
    int state_ = 1;
    bool centered_ = false;
};

}  // namespace

std::unique_ptr<Panel> make_substrate_panel() {
    return std::make_unique<SubstratePanel>();
}

}  // namespace ftd::native
