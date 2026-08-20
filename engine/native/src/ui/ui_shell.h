#pragma once

#include "native/scene_rect.h"
#include "ui/command_palette.h"
#include "ui/history.h"
#include "ui/panel_registry.h"
#include "ui/theme.h"
#include "ui/workspace.h"

#include <string>
#include <unordered_map>

namespace ftd::native {

class UiShell {
public:
    explicit UiShell(std::string storage_dir);

    void set_dpi_scale(float dpi_scale);
    void set_theme(const Theme& theme);
    const Theme& theme() const { return theme_; }
    float dpi_scale() const { return dpi_scale_; }

    void draw(const UiSnapshot& snapshot, CommandSink& commands, ViewChrome chrome);

    SceneRect scene_rect() const { return scene_rect_; }
    DataNeeds last_demand() const { return last_demand_; }
    WorkspaceKind workspace() const { return workspace_.kind; }
    const char* workspace_name() const { return workspace_kind_name(workspace_.kind); }

    const PanelRegistry& registry() const { return registry_; }
    bool panel_open(const std::string& id) const;
    bool palette_open() const { return palette_open_; }
    void open_palette();
    bool run_palette_entry(const std::string& id, const UiSnapshot& snapshot,
                           CommandSink& commands, ViewChrome& chrome);

    void persist();

private:
    void draw_menu(ViewChrome& chrome);
    void draw_status(const UiSnapshot& snapshot, const ViewChrome& chrome);
    void draw_setup_stack(const UiSnapshot& snapshot, CommandSink& commands,
                          ViewChrome& chrome);
    void draw_play_bar(const UiSnapshot& snapshot, CommandSink& commands,
                       ViewChrome& chrome);
    void draw_palette(const UiSnapshot& snapshot, CommandSink& commands,
                      ViewChrome& chrome);
    PaletteHostState make_palette_host(const UiSnapshot& snapshot,
                                       const ViewChrome& chrome) const;
    void apply_palette_choice(const PaletteEntry& entry, const UiSnapshot& snapshot,
                              CommandSink& commands, ViewChrome& chrome);
    void ensure_layout(ImGuiID dockspace_id);
    void switch_workspace(WorkspaceKind kind);
    void apply_open_set(const std::vector<std::string>& ids);
    std::string workspace_path(WorkspaceKind kind) const;
    void load_active();
    bool* open_flag(const std::string& id);

    std::string storage_dir_;
    PanelRegistry registry_;
    Theme theme_;
    History history_;
    WorkspaceState workspace_;
    std::unordered_map<std::string, bool> open_;
    std::string selected_setup_id_ = "scenarios";
    SceneRect scene_rect_{};
    DataNeeds last_demand_{};
    DataNeeds pending_demand_{};
    float dpi_scale_ = 1.0f;
    bool layout_dirty_ = true;
    bool style_dirty_ = true;
    bool palette_open_ = false;
    bool palette_focus_ = false;
    int palette_selected_ = 0;
    char palette_query_[128] = {};
};

}  // namespace ftd::native
