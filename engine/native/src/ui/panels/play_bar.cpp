#include "ui/panels/panels.h"

#include <algorithm>
#include <string>

namespace ftd::native {
namespace {

ImVec4 vec(const Rgba& c) { return {c.r, c.g, c.b, c.a}; }

class PlayBarPanel final : public Panel {
public:
    const char* id() const override { return "play_bar"; }
    const char* title() const override { return "Play bar"; }
    DockSlot default_slot() const override { return DockSlot::Viewport; }

    void draw_contents(PanelContext& ctx) override {
        const float s = ctx.dpi_scale > 0.0f ? ctx.dpi_scale : 1.0f;
        const ImVec2 play_sz(52.0f * s, 36.0f * s);
        const ImVec2 small_sz(36.0f * s, 36.0f * s);
        const bool paused =
            !ctx.chrome || !ctx.chrome->paused || ctx.chrome->paused->load();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(7.0f * s, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f * s);

        const ImVec4 accent = vec(ctx.theme.accent);
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(accent.x, accent.y, accent.z, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(accent.x, accent.y, accent.z, 0.50f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(accent.x, accent.y, accent.z, 0.70f));
        if (paused) {
            if (ImGui::Button("Play", play_sz)) {
                ctx.commands.push(Run{});
                if (ctx.chrome && ctx.chrome->paused) ctx.chrome->paused->store(false);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip("Play / Pause (Space)");
            }
        } else {
            if (ImGui::Button("Pause", play_sz)) {
                ctx.commands.push(Pause{});
                if (ctx.chrome && ctx.chrome->paused) ctx.chrome->paused->store(true);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip("Play / Pause (Space)");
            }
        }
        ImGui::PopStyleColor(3);

        const float row_y = ImGui::GetItemRectMin().y;
        const float row_h = ImGui::GetItemRectSize().y;
        auto align_small = [&]() {
            ImGui::SameLine();
            ImGui::SetCursorScreenPos(ImVec2(
                ImGui::GetCursorScreenPos().x,
                row_y + (row_h - small_sz.y) * 0.5f));
        };

        align_small();
        if (ImGui::Button("Step", small_sz)) {
            ctx.commands.push(Pause{});
            ctx.commands.push(Step{1});
            if (ctx.chrome && ctx.chrome->paused) ctx.chrome->paused->store(true);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("Step (S)");
        }

        align_small();
        if (ImGui::Button("Reset", small_sz)) {
            const std::string id = ctx.snapshot.frame.scenario;
            if (!id.empty()) ctx.commands.push(LoadScenario{id});
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("Reset (R)");
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, vec(ctx.theme.text_dim));
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("|");
        ImGui::PopStyleColor();

        if (ctx.chrome && ctx.chrome->tick_hz) {
            int hz = ctx.chrome->tick_hz->load();
            align_small();
            if (ImGui::Button("-", small_sz) && hz > 1) {
                ctx.chrome->tick_hz->store(hz - 1);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip("Slower");
            }

            ImGui::SameLine();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%d /s", std::max(1, hz));

            align_small();
            if (ImGui::Button("+", small_sz) && hz < 60) {
                ctx.chrome->tick_hz->store(hz + 1);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip("Faster");
            }
        }

        ImGui::PopStyleVar(2);
    }
};

}  // namespace

std::unique_ptr<Panel> make_play_bar_panel() {
    return std::make_unique<PlayBarPanel>();
}

}  // namespace ftd::native
