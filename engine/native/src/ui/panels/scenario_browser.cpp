#include "ui/panels/panels.h"

#include "ftd/scenario_meta.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

namespace ftd::native {
namespace {

constexpr const char* kCategoryOrder[] = {
    "1. Validated Native Dynamics",
    "2. Validated State Dynamics",
    "3. Qualified Selected Extensions",
    "4. Validated Initial Data",
    "5. Macroscopic Physics & Measurement",
};

bool contains_ci(const char* hay, const char* needle) {
    if (!needle || !*needle) return true;
    if (!hay) return false;
    auto lower = [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    };
    std::string a = hay;
    std::string b = needle;
    std::transform(a.begin(), a.end(), a.begin(), lower);
    std::transform(b.begin(), b.end(), b.begin(), lower);
    return a.find(b) != std::string::npos;
}

bool row_matches(const ftd::ScenarioMeta& row, const char* filter) {
    return contains_ci(row.title, filter) || contains_ci(row.id, filter)
        || contains_ci(row.tags, filter) || contains_ci(row.category, filter)
        || contains_ci(row.epistemic_status, filter);
}

const char* epistemic_head(const char* status) {
    if (!status || status[0] != '[') return "";
    const char* end = std::strchr(status, ']');
    if (!end) return status;
    thread_local char buf[48];
    const std::size_t n = static_cast<std::size_t>(end - status + 1);
    if (n >= sizeof(buf)) return status;
    std::memcpy(buf, status, n);
    buf[n] = '\0';
    return buf;
}

class ScenarioBrowserPanel final : public Panel {
public:
    const char* id() const override { return "scenarios"; }
    const char* title() const override { return "Scenarios"; }
    DockSlot default_slot() const override { return DockSlot::Setup; }

    void draw_contents(PanelContext& ctx) override {
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##filter", "Filter title, id, tag", filter_,
                                 sizeof(filter_));

        const std::string current = ctx.snapshot.frame.scenario;
        const ftd::ScenarioMeta* current_meta =
            current.empty() ? nullptr : ftd::find_scenario_meta(current);

        if (ImGui::BeginChild("##ids", ImVec2(-1.0f, -110.0f))) {
            for (const char* category : kCategoryOrder) {
                std::vector<const ftd::ScenarioMeta*> rows;
                for (const auto& row : ftd::SCENARIO_META) {
                    if (std::strcmp(row.category, category) != 0) continue;
                    if (!row_matches(row, filter_)) continue;
                    rows.push_back(&row);
                }
                if (rows.empty()) continue;
                ImGui::SeparatorText(category);
                for (const ftd::ScenarioMeta* row : rows) {
                    const bool selected =
                        selected_ == row->id || current == row->id;
                    ImGui::PushID(row->id);
                    if (ImGui::Selectable(row->title, selected,
                                          ImGuiSelectableFlags_AllowDoubleClick)) {
                        selected_ = row->id;
                        if (ImGui::IsItemHovered()
                            && ImGui::IsMouseDoubleClicked(0)) {
                            ctx.commands.push(LoadScenario{row->id});
                        }
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                        ImGui::SetTooltip("%s\n%s", row->id, row->epistemic_status);
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                    ImGui::PopID();
                }
            }
        }
        ImGui::EndChild();

        if (ImGui::Button("Load scenario", ImVec2(-1.0f, 0.0f))) {
            if (!selected_.empty()) ctx.commands.push(LoadScenario{selected_});
            else if (!current.empty()) ctx.commands.push(LoadScenario{current});
        }

        const char* detail_id =
            !selected_.empty() ? selected_.c_str() : current.c_str();
        const ftd::ScenarioMeta* detail =
            detail_id && *detail_id ? ftd::find_scenario_meta(detail_id)
                                    : current_meta;
        if (detail) {
            ImGui::TextUnformatted(detail->title);
            ImGui::TextDisabled("%s  ·  %s", detail->id,
                                epistemic_head(detail->epistemic_status));
            if (detail->description[0] == '\0') {
                ImGui::TextDisabled("No description authored yet");
            } else {
                ImGui::TextWrapped("%s", detail->description);
            }
            if (detail->min_lattice > 0) {
                ImGui::TextDisabled("%s  ·  L ≥ %d", detail->admission_status,
                                    detail->min_lattice);
            } else {
                ImGui::TextDisabled("%s  ·  L unconstrained",
                                    detail->admission_status);
            }
        } else if (!current.empty()) {
            ImGui::TextUnformatted(current.c_str());
        }
    }

private:
    char filter_[128]{};
    std::string selected_;
};

}  // namespace

std::unique_ptr<Panel> make_scenario_browser_panel() {
    return std::make_unique<ScenarioBrowserPanel>();
}

}  // namespace ftd::native
