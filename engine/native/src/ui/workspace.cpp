#include "ui/workspace.h"

#include "imgui_internal.h"

#include <fstream>
#include <sstream>

namespace ftd::native {
namespace {

constexpr int kWorkspaceVersion = 1;

std::string join_ids(const std::vector<std::string>& ids) {
    std::ostringstream out;
    for (std::size_t i = 0; i < ids.size(); ++i) {
        if (i) out << ' ';
        out << ids[i];
    }
    return out.str();
}

std::vector<std::string> split_ids(const std::string& text) {
    std::vector<std::string> ids;
    std::istringstream in(text);
    std::string token;
    while (in >> token) ids.push_back(token);
    return ids;
}

}  // namespace

const char* workspace_kind_name(WorkspaceKind kind) {
    switch (kind) {
        case WorkspaceKind::Analysis:
            return "Analysis";
        case WorkspaceKind::Presentation:
            return "Presentation";
        case WorkspaceKind::Experiment:
        default:
            return "Experiment";
    }
}

WorkspaceKind workspace_kind_from_name(const std::string& name) {
    if (name == "Analysis") return WorkspaceKind::Analysis;
    if (name == "Presentation") return WorkspaceKind::Presentation;
    return WorkspaceKind::Experiment;
}

std::vector<std::string> default_open_ids(WorkspaceKind kind) {
    switch (kind) {
        case WorkspaceKind::Analysis:
            return {"play_bar", "telemetry", "audit", "lagrangian", "inspector"};
        case WorkspaceKind::Presentation:
            return {"play_bar"};
        case WorkspaceKind::Experiment:
        default:
            return {"scenarios", "run_config", "substrate", "play_bar", "telemetry",
                    "audit", "lagrangian", "inspector", "physics_terms", "fields",
                    "log"};
    }
}

WorkspaceState default_workspace(WorkspaceKind kind) {
    WorkspaceState state;
    state.version = kWorkspaceVersion;
    state.kind = kind;
    state.theme_name = "Graphite";
    state.open_ids = default_open_ids(kind);
    if (kind == WorkspaceKind::Experiment) state.setup_tab = "scenarios";
    return state;
}

bool save_workspace_file(const std::string& path, const WorkspaceState& state) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out << "ftd-workspace " << state.version << "\n";
    out << "kind=" << workspace_kind_name(state.kind) << "\n";
    out << "theme=" << state.theme_name << "\n";
    out << "open=" << join_ids(state.open_ids) << "\n";
    out << "setup_tab=" << state.setup_tab << "\n";
    out << "---ini---\n";
    out << state.imgui_ini;
    return static_cast<bool>(out);
}

WorkspaceLoadResult load_workspace_file(const std::string& path, WorkspaceKind expected) {
    WorkspaceLoadResult result;
    result.state = default_workspace(expected);
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        result.used_fallback = true;
        result.error = "missing workspace file";
        return result;
    }
    std::ostringstream raw;
    raw << in.rdbuf();
    const std::string text = raw.str();
    if (text.empty()) {
        result.used_fallback = true;
        result.error = "empty workspace file";
        return result;
    }
    const auto marker = text.find("---ini---");
    std::string header = marker == std::string::npos ? text : text.substr(0, marker);
    std::istringstream lines(header);
    std::string line;
    bool saw_version = false;
    WorkspaceState parsed = default_workspace(expected);
    while (std::getline(lines, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.rfind("ftd-workspace ", 0) == 0) {
            try {
                parsed.version = std::stoi(line.substr(14));
            } catch (...) {
                result.used_fallback = true;
                result.error = "bad workspace version";
                return result;
            }
            saw_version = true;
            continue;
        }
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = line.substr(0, eq);
        const std::string value = line.substr(eq + 1);
        if (key == "kind") parsed.kind = workspace_kind_from_name(value);
        else if (key == "theme") parsed.theme_name = value;
        else if (key == "open") parsed.open_ids = split_ids(value);
        else if (key == "setup_tab") parsed.setup_tab = value;
    }
    if (!saw_version || parsed.version != kWorkspaceVersion) {
        result.used_fallback = true;
        result.error = "unsupported or missing workspace version";
        return result;
    }
    if (marker != std::string::npos) {
        parsed.imgui_ini = text.substr(marker + 10);
        if (!parsed.imgui_ini.empty() && parsed.imgui_ini.front() == '\n') {
            parsed.imgui_ini.erase(parsed.imgui_ini.begin());
        }
        if (!parsed.imgui_ini.empty() && parsed.imgui_ini.front() == '\r') {
            parsed.imgui_ini.erase(parsed.imgui_ini.begin());
            if (!parsed.imgui_ini.empty() && parsed.imgui_ini.front() == '\n') {
                parsed.imgui_ini.erase(parsed.imgui_ini.begin());
            }
        }
    }
    result.ok = true;
    result.state = std::move(parsed);
    return result;
}

bool setup_host_is_docked_in_ini(const std::string& imgui_ini) {
    const auto start = imgui_ini.find("[Window][setup]");
    if (start == std::string::npos) return false;
    const auto end = imgui_ini.find("\n[", start + 1);
    const auto section = imgui_ini.substr(start, end == std::string::npos
                                                    ? std::string::npos
                                                    : end - start);
    return section.find("DockId=") != std::string::npos;
}

void apply_dock_recipe(ImGuiID dockspace_id, WorkspaceKind kind,
                       const PanelRegistry& registry) {
    const auto named = [&](const char* id) -> std::string {
        const Panel* panel = registry.find(id);
        return panel ? window_name(*panel) : std::string();
    };

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    const ImVec2 dock_size = ImGui::GetMainViewport()
                                 ? ImGui::GetMainViewport()->WorkSize
                                 : ImVec2(1920.0f, 1080.0f);
    ImGui::DockBuilderSetNodeSize(dockspace_id,
                                  dock_size.x > 0.0f && dock_size.y > 0.0f
                                      ? dock_size
                                      : ImVec2(1920.0f, 1080.0f));

    if (kind == WorkspaceKind::Presentation) {
        ImGui::DockBuilderFinish(dockspace_id);
        return;
    }

    ImGuiID remaining = dockspace_id;

    if (kind == WorkspaceKind::Experiment) {
        ImGuiID setup = 0;
        ImGui::DockBuilderSplitNode(remaining, ImGuiDir_Left, 0.175f, &setup, &remaining);
        ImGuiID instruments = 0;
        ImGui::DockBuilderSplitNode(remaining, ImGuiDir_Right, 0.159f, &instruments,
                                    &remaining);
        ImGuiID physics = 0;
        ImGui::DockBuilderSplitNode(remaining, ImGuiDir_Down, 0.137f, &physics,
                                    &remaining);

        ImGui::DockBuilderDockWindow(kSetupHostWindowName, setup);
        if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(setup)) {
            node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar
                | ImGuiDockNodeFlags_NoDockingOverMe;
        }
        for (const char* id : {"telemetry", "audit", "lagrangian", "inspector"}) {
            const std::string name = named(id);
            if (!name.empty()) ImGui::DockBuilderDockWindow(name.c_str(), instruments);
        }
        for (const char* id : {"physics_terms", "fields", "log"}) {
            const std::string name = named(id);
            if (!name.empty()) ImGui::DockBuilderDockWindow(name.c_str(), physics);
        }
    } else {
        ImGuiID instruments = remaining;
        for (const char* id : {"telemetry", "audit", "lagrangian", "inspector"}) {
            const std::string name = named(id);
            if (!name.empty()) ImGui::DockBuilderDockWindow(name.c_str(), instruments);
        }
    }

    ImGui::DockBuilderFinish(dockspace_id);
}

}  // namespace ftd::native
