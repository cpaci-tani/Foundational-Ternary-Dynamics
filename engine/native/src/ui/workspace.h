#pragma once

#include "ui/panel_registry.h"

#include "imgui.h"

#include <string>
#include <vector>

namespace ftd::native {

enum class WorkspaceKind { Experiment, Analysis, Presentation };

struct WorkspaceState {
    int version = 1;
    WorkspaceKind kind = WorkspaceKind::Experiment;
    std::string theme_name = "Graphite";
    std::vector<std::string> open_ids;
    std::string setup_tab = "scenarios";
    std::string imgui_ini;
};

inline constexpr const char* kSetupHostWindowName = "Setup###setup";

struct WorkspaceLoadResult {
    bool ok = false;
    bool used_fallback = false;
    WorkspaceState state;
    std::string error;
};

const char* workspace_kind_name(WorkspaceKind kind);
WorkspaceKind workspace_kind_from_name(const std::string& name);

std::vector<std::string> default_open_ids(WorkspaceKind kind);
WorkspaceState default_workspace(WorkspaceKind kind);

bool save_workspace_file(const std::string& path, const WorkspaceState& state);
WorkspaceLoadResult load_workspace_file(const std::string& path, WorkspaceKind expected);
bool setup_host_is_docked_in_ini(const std::string& imgui_ini);

void apply_dock_recipe(ImGuiID dockspace_id, WorkspaceKind kind,
                       const PanelRegistry& registry);

}  // namespace ftd::native
