#include "ui/ui_shell.h"

#include "ui/command_palette.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace ftd::native {
namespace {

const char* backend_label(BackendKindUi kind) {
    switch (kind) {
        case BackendKindUi::Gpu:
            return "GPU";
        case BackendKindUi::Cpu:
            return "CPU";
        default:
            return "unknown";
    }
}

void ensure_directory(const std::string& dir) {
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
}

}  // namespace

UiShell::UiShell(std::string storage_dir)
    : storage_dir_(std::move(storage_dir)),
      registry_(PanelRegistry::make_default()),
      theme_(make_graphite()),
      workspace_(default_workspace(WorkspaceKind::Experiment)) {
    registry_.for_each([&](const Panel& panel) { open_[panel.id()] = true; });
    apply_open_set(workspace_.open_ids);
    load_active();
    if (!workspace_.setup_tab.empty()) selected_setup_id_ = workspace_.setup_tab;
}

void UiShell::set_dpi_scale(float dpi_scale) {
    dpi_scale_ = dpi_scale > 0.0f ? dpi_scale : 1.0f;
    style_dirty_ = true;
}

void UiShell::set_theme(const Theme& theme) {
    theme_ = theme;
    workspace_.theme_name = theme.name;
    style_dirty_ = true;
}

bool UiShell::panel_open(const std::string& id) const {
    const auto it = open_.find(id);
    return it != open_.end() && it->second;
}

void UiShell::open_palette() {
    palette_open_ = true;
    palette_focus_ = true;
    palette_selected_ = 0;
    std::memset(palette_query_, 0, sizeof(palette_query_));
}

PaletteHostState UiShell::make_palette_host(const UiSnapshot& snapshot,
                                            const ViewChrome& chrome) const {
    PaletteHostState host;
    host.paused = !chrome.paused || chrome.paused->load();
    host.scenario = snapshot.frame.scenario;
    host.theme_name = theme_.name;
    host.workspace = workspace_.kind;
    host.particles = chrome.particles && *chrome.particles;
    host.flux = chrome.flux && *chrome.flux;
    host.lattice_box = chrome.lattice_box && *chrome.lattice_box;
    registry_.for_each([&](const Panel& panel) {
        const bool open = panel_open(panel.id());
        host.panel_open[panel.id()] = open;
        if (panel.default_slot() == DockSlot::Viewport) {
            host.panel_floating[panel.id()] = open;
            return;
        }
        if (panel.default_slot() == DockSlot::Setup) {
            host.panel_floating[panel.id()] = false;
            return;
        }
        const std::string name = window_name(panel);
        const ImGuiWindow* win = ImGui::FindWindowByName(name.c_str());
        host.panel_floating[panel.id()] = open && win && !win->DockIsActive;
    });
    return host;
}

void UiShell::apply_palette_choice(const PaletteEntry& entry,
                                   const UiSnapshot& snapshot, CommandSink& commands,
                                   ViewChrome& chrome) {
    const PaletteHostState host = make_palette_host(snapshot, chrome);
    const PaletteEffect effect = effect_for(entry);
    if (effect.kind == PaletteEffectKind::ShowPanel) {
        open_[effect.arg] = true;
        const Panel* panel = registry_.find(effect.arg);
        if (panel && panel->default_slot() == DockSlot::Setup) {
            selected_setup_id_ = effect.arg;
        }
        return;
    }
    if (effect.kind == PaletteEffectKind::Workspace) {
        switch_workspace(workspace_kind_from_name(effect.arg));
        return;
    }
    if (effect.kind == PaletteEffectKind::Theme) {
        set_theme(builtin_theme_by_name(effect.arg));
        return;
    }
    apply_palette_effect(effect, host, commands, &chrome);
}

bool UiShell::run_palette_entry(const std::string& id, const UiSnapshot& snapshot,
                                CommandSink& commands, ViewChrome& chrome) {
    const PaletteHostState host = make_palette_host(snapshot, chrome);
    const auto catalog = build_phase3b_catalog(registry_, host);
    for (const auto& entry : catalog) {
        if (entry.id != id) continue;
        apply_palette_choice(entry, snapshot, commands, chrome);
        return palette_state_is_enabled(entry.state)
            || entry.kind == PaletteKind::Panel;
    }
    return false;
}

void UiShell::draw_palette(const UiSnapshot& snapshot, CommandSink& commands,
                           ViewChrome& chrome) {
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_K)) {
        open_palette();
    }
    if (!palette_open_) return;

    const PaletteHostState host = make_palette_host(snapshot, chrome);
    const auto catalog = build_phase3b_catalog(registry_, host);
    const auto ranked = rank_palette(palette_query_, catalog);
    if (palette_selected_ < 0) palette_selected_ = 0;
    if (!ranked.empty() && palette_selected_ >= static_cast<int>(ranked.size())) {
        palette_selected_ = static_cast<int>(ranked.size()) - 1;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float s = dpi_scale_ > 0.0f ? dpi_scale_ : 1.0f;
    ImGui::SetNextWindowSize(ImVec2(520.0f * s, 380.0f * s), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(
        ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
               viewport->WorkPos.y + viewport->WorkSize.y * 0.32f),
        ImGuiCond_Appearing, ImVec2(0.5f, 0.0f));
    ImGui::OpenPopup("Command palette###palette");
    if (!ImGui::BeginPopupModal("Command palette###palette", &palette_open_,
                                ImGuiWindowFlags_NoSavedSettings
                                    | ImGuiWindowFlags_NoDocking
                                    | ImGuiWindowFlags_NoResize)) {
        palette_open_ = false;
        return;
    }

    ImGui::TextUnformatted("Search panels and actions");
    if (palette_focus_) {
        ImGui::SetKeyboardFocusHere();
        palette_focus_ = false;
    }
    char previous[128];
    std::memcpy(previous, palette_query_, sizeof(previous));
    const bool enter = ImGui::InputText("##palette_query", palette_query_,
                                        sizeof(palette_query_),
                                        ImGuiInputTextFlags_EnterReturnsTrue);
    if (std::memcmp(previous, palette_query_, sizeof(previous)) != 0) {
        palette_selected_ = 0;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && !ranked.empty()) {
        palette_selected_ = (palette_selected_ + 1) % static_cast<int>(ranked.size());
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && !ranked.empty()) {
        palette_selected_ = (palette_selected_ + static_cast<int>(ranked.size()) - 1)
            % static_cast<int>(ranked.size());
    }

    ImGui::BeginChild("##palette_results", ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing()),
                      ImGuiChildFlags_Borders);
    for (int i = 0; i < static_cast<int>(ranked.size()); ++i) {
        const PaletteMatch& match = ranked[static_cast<std::size_t>(i)];
        const bool enabled = match.entry.kind == PaletteKind::Panel
            || palette_state_is_enabled(match.entry.state);
        ImGui::PushID(match.entry.id.c_str());
        if (!enabled) {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        }
        const bool chosen = ImGui::Selectable(
            match.entry.title.c_str(), i == palette_selected_,
            enabled ? 0 : ImGuiSelectableFlags_Disabled);
        if (ImGui::IsItemHovered() && enabled) palette_selected_ = i;
        ImGui::SameLine();
        ImGui::TextDisabled("%s  %s", palette_kind_label(match.entry.kind),
                            match.entry.state.c_str());
        if (!enabled) ImGui::PopStyleColor();
        if (chosen && enabled) {
            apply_palette_choice(match.entry, snapshot, commands, chrome);
            palette_open_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    if (enter && !ranked.empty()) {
        const PaletteMatch& match =
            ranked[static_cast<std::size_t>(palette_selected_)];
        const bool enabled = match.entry.kind == PaletteKind::Panel
            || palette_state_is_enabled(match.entry.state);
        if (enabled) {
            apply_palette_choice(match.entry, snapshot, commands, chrome);
            palette_open_ = false;
            ImGui::CloseCurrentPopup();
        }
    }

    ImGui::EndPopup();
}

bool* UiShell::open_flag(const std::string& id) {
    return &open_[id];
}

void UiShell::apply_open_set(const std::vector<std::string>& ids) {
    for (auto& entry : open_) entry.second = false;
    if (ids.empty() && workspace_.kind != WorkspaceKind::Presentation) {
        for (auto& entry : open_) entry.second = true;
        return;
    }
    for (const auto& id : ids) open_[id] = true;
    registry_.for_each([&](const Panel& panel) {
        if (panel.default_slot() == DockSlot::Viewport) open_[panel.id()] = true;
    });
}

std::string UiShell::workspace_path(WorkspaceKind kind) const {
    std::string name = workspace_kind_name(kind);
    for (char& c : name) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return storage_dir_ + "/" + name + ".workspace";
}

void UiShell::load_active() {
    if (storage_dir_.empty()) return;
    ensure_directory(storage_dir_);
    WorkspaceKind kind = WorkspaceKind::Experiment;
    {
        std::ifstream in(storage_dir_ + "/active");
        std::string name;
        if (in >> name) kind = workspace_kind_from_name(name);
    }
    const auto loaded = load_workspace_file(workspace_path(kind), kind);
    workspace_ = loaded.state;
    workspace_.kind = kind;
    theme_ = builtin_theme_by_name(workspace_.theme_name);
    apply_open_set(workspace_.open_ids);
    if (!workspace_.setup_tab.empty()) selected_setup_id_ = workspace_.setup_tab;
    if (ImGui::GetCurrentContext() == nullptr) {
        layout_dirty_ = true;
        style_dirty_ = true;
        return;
    }
    if (!workspace_.imgui_ini.empty()) {
        ImGui::LoadIniSettingsFromMemory(workspace_.imgui_ini.c_str(),
                                         workspace_.imgui_ini.size());
        layout_dirty_ = !setup_host_is_docked_in_ini(workspace_.imgui_ini);
    } else {
        layout_dirty_ = true;
    }
    style_dirty_ = true;
}

void UiShell::persist() {
    if (storage_dir_.empty() || ImGui::GetCurrentContext() == nullptr) return;
    ensure_directory(storage_dir_);
    workspace_.open_ids.clear();
    registry_.for_each([&](const Panel& panel) {
        if (panel_open(panel.id())) workspace_.open_ids.emplace_back(panel.id());
    });
    workspace_.theme_name = theme_.name;
    workspace_.setup_tab = selected_setup_id_;
    size_t ini_size = 0;
    const char* ini = ImGui::SaveIniSettingsToMemory(&ini_size);
    workspace_.imgui_ini = ini ? std::string(ini, ini_size) : std::string();
    save_workspace_file(workspace_path(workspace_.kind), workspace_);
    std::ofstream active(storage_dir_ + "/active", std::ios::binary);
    active << workspace_kind_name(workspace_.kind);
}

void UiShell::switch_workspace(WorkspaceKind kind) {
    persist();
    workspace_ = default_workspace(kind);
    const auto loaded = load_workspace_file(workspace_path(kind), kind);
    if (loaded.ok) workspace_ = loaded.state;
    workspace_.kind = kind;
    apply_open_set(workspace_.open_ids);
    if (!workspace_.setup_tab.empty()) selected_setup_id_ = workspace_.setup_tab;
    if (!workspace_.imgui_ini.empty() && !loaded.used_fallback) {
        ImGui::LoadIniSettingsFromMemory(workspace_.imgui_ini.c_str(),
                                         workspace_.imgui_ini.size());
        layout_dirty_ = ImGui::DockBuilderGetNode(ImGui::GetID("FTDDock")) == nullptr
            || !setup_host_is_docked_in_ini(workspace_.imgui_ini);
    } else {
        ImGui::ClearIniSettings();
        layout_dirty_ = true;
    }
}

void UiShell::ensure_layout(ImGuiID dockspace_id) {
    if (!layout_dirty_ && ImGui::DockBuilderGetNode(dockspace_id) != nullptr) {
        return;
    }
    apply_dock_recipe(dockspace_id, workspace_.kind, registry_);
    layout_dirty_ = false;
}

void UiShell::draw_menu(ViewChrome& chrome) {
    if (!ImGui::BeginMenuBar()) return;
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Command palette...", "Ctrl+K")) {
            open_palette();
        }
        if (ImGui::MenuItem("Exit")) {
            if (chrome.request_quit) *chrome.request_quit = true;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Experiment workspace", nullptr,
                            workspace_.kind == WorkspaceKind::Experiment)) {
            switch_workspace(WorkspaceKind::Experiment);
        }
        if (ImGui::MenuItem("Analysis workspace", nullptr,
                            workspace_.kind == WorkspaceKind::Analysis)) {
            switch_workspace(WorkspaceKind::Analysis);
        }
        if (ImGui::MenuItem("Presentation workspace", nullptr,
                            workspace_.kind == WorkspaceKind::Presentation)) {
            switch_workspace(WorkspaceKind::Presentation);
        }
        ImGui::Separator();
        if (chrome.particles) {
            ImGui::MenuItem("Particles", nullptr, chrome.particles);
        }
        if (chrome.flux) {
            ImGui::MenuItem("Flux", nullptr, chrome.flux);
        }
        if (chrome.lattice_box) {
            ImGui::MenuItem("Lattice box", nullptr, chrome.lattice_box);
        }
        if (ImGui::MenuItem("Reset camera") && chrome.reset_camera) {
            *chrome.reset_camera = true;
        }
        ImGui::Separator();
        registry_.for_each([&](const Panel& panel) {
            bool* flag = open_flag(panel.id());
            if (ImGui::MenuItem(panel.title(), nullptr, flag)) {
                if (panel.default_slot() == DockSlot::Setup && *flag) {
                    selected_setup_id_ = panel.id();
                }
            }
        });
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Theme")) {
        const char* names[] = {"Graphite", "Contrast", "Slate", "Carbon"};
        for (const char* name : names) {
            if (ImGui::MenuItem(name, nullptr, theme_.name == name)) {
                set_theme(builtin_theme_by_name(name));
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
        ImGui::MenuItem("FTD Native Desktop", nullptr, false, false);
        ImGui::TextUnformatted("Graphite shell  ·  Scale 0");
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void UiShell::draw_setup_stack(const UiSnapshot& snapshot, CommandSink& commands,
                               ViewChrome& chrome) {
    std::vector<Panel*> tabs;
    registry_.for_each([&](Panel& panel) {
        if (panel.default_slot() == DockSlot::Setup && panel_open(panel.id())) {
            tabs.push_back(&panel);
        }
    });
    if (tabs.empty()) return;

    bool selected_visible = false;
    for (Panel* panel : tabs) {
        if (panel->id() == selected_setup_id_) {
            selected_visible = true;
            break;
        }
    }
    if (!selected_visible) selected_setup_id_ = tabs.front()->id();

    ImGuiWindowClass host_class;
    host_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar
        | ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::SetNextWindowClass(&host_class);

    bool host_open = true;
    const ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    if (!ImGui::Begin(kSetupHostWindowName, &host_open, host_flags)) {
        ImGui::End();
        if (!host_open) {
            for (Panel* panel : tabs) *open_flag(panel->id()) = false;
        }
        return;
    }
    if (!host_open) {
        for (Panel* panel : tabs) *open_flag(panel->id()) = false;
        ImGui::End();
        return;
    }

    float rail_w = ImGui::CalcTextSize("Run config").x
        + ImGui::GetStyle().FramePadding.x * 4.0f;
    for (Panel* panel : tabs) {
        rail_w = std::max(
            rail_w, ImGui::CalcTextSize(panel->title()).x
                        + ImGui::GetStyle().FramePadding.x * 4.0f);
    }
    const float avail = ImGui::GetContentRegionAvail().x;
    if (avail > 0.0f) rail_w = std::min(rail_w, avail * 0.42f);

    const ImVec2 body_pad = ImGui::GetStyle().WindowPadding;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 2.0f));
    const ImVec2 area = ImGui::GetContentRegionAvail();
    if (ImGui::BeginTable("##setup_split", 2,
                          ImGuiTableFlags_SizingFixedFit
                              | ImGuiTableFlags_NoPadOuterX
                              | ImGuiTableFlags_NoPadInnerX,
                          area)) {
        ImGui::TableSetupColumn("rail", ImGuiTableColumnFlags_WidthFixed, rail_w);
        ImGui::TableSetupColumn("body", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextColumn();
        const float tab_h = ImGui::GetFrameHeight() * 1.65f;
        for (Panel* panel : tabs) {
            const bool on = selected_setup_id_ == panel->id();
            if (on) {
                ImGui::PushStyleColor(ImGuiCol_Header,
                                      ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
            }
            if (ImGui::Selectable(panel->title(), on, 0, ImVec2(-1.0f, tab_h))) {
                selected_setup_id_ = panel->id();
            }
            if (on) ImGui::PopStyleColor();
        }
        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, body_pad);
        ImGui::BeginChild("##setup_body", ImVec2(0.0f, 0.0f),
                          ImGuiChildFlags_AlwaysUseWindowPadding);
        Panel* active = nullptr;
        for (Panel* panel : tabs) {
            if (panel->id() == selected_setup_id_) {
                active = panel;
                break;
            }
        }
        if (active) {
            PanelContext ctx{snapshot, commands, theme_, history_,
                             open_flag(active->id()), dpi_scale_, &chrome};
            active->draw_contents(ctx);
            pending_demand_ = pending_demand_ | active->needs();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::EndTable();
    }
    ImGui::PopStyleVar(2);
    ImGui::End();
}

void UiShell::draw_status(const UiSnapshot& snapshot, const ViewChrome& chrome) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float height = ImGui::GetFrameHeight();
    if (!ImGui::BeginViewportSideBar("##FTDStatus", viewport, ImGuiDir_Down, height,
                                     ImGuiWindowFlags_NoScrollbar
                                         | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::End();
        return;
    }
    const char* scenario =
        snapshot.frame.scenario.empty() ? "—" : snapshot.frame.scenario.c_str();
    ImGui::Text("%s  ·  %s  ·  interop %s  ·  L=%d  tick=%d  particles %u  ·  %s",
                scenario, backend_label(snapshot.env.backend),
                chrome.interop_active ? "on" : "off", snapshot.knobs.lattice_size,
                snapshot.frame.tick, snapshot.frame.total_manifested,
                snapshot.frame.status.empty() ? "ready" : snapshot.frame.status.c_str());
    ImGui::End();
}

void UiShell::draw_play_bar(const UiSnapshot& snapshot, CommandSink& commands,
                            ViewChrome& chrome) {
    Panel* panel = registry_.find("play_bar");
    if (!panel || !panel_open(panel->id())) return;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float margin = 12.0f * dpi_scale_;
    ImVec2 pos;
    if (scene_rect_.width > 0 && scene_rect_.height > 0) {
        pos.x = viewport->Pos.x + static_cast<float>(scene_rect_.x)
                + static_cast<float>(scene_rect_.width) * 0.5f;
        pos.y = viewport->Pos.y + static_cast<float>(scene_rect_.y)
                + static_cast<float>(scene_rect_.height) - margin;
    } else {
        pos.x = viewport->WorkPos.x + viewport->WorkSize.x * 0.5f;
        pos.y = viewport->WorkPos.y + viewport->WorkSize.y - margin;
    }
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowBgAlpha(0.92f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f * dpi_scale_);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(12.0f * dpi_scale_, 10.0f * dpi_scale_));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;

    bool* open = open_flag(panel->id());
    const std::string name = window_name(*panel);
    if (ImGui::Begin(name.c_str(), open, flags)) {
        PanelContext ctx{snapshot, commands, theme_, history_, open, dpi_scale_,
                         &chrome};
        panel->draw_contents(ctx);
        pending_demand_ = pending_demand_ | panel->needs();
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
}

void UiShell::draw(const UiSnapshot& snapshot, CommandSink& commands,
                   ViewChrome chrome) {
    if (ImGui::GetCurrentContext() == nullptr) return;
    if (style_dirty_) {
        apply_theme(theme_, dpi_scale_);
        style_dirty_ = false;
    }

    last_demand_ = pending_demand_;
    pending_demand_ = {};

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    // PassthruCentralNode only punches a hole for the D3D12 lattice if this
    // host does not paint WindowBg over that hole (imgui.h; DockSpaceOverViewport).
    ImGuiWindowFlags host_flags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar
        | ImGuiWindowFlags_NoBackground;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("FTD Dockspace###ftd.dock", nullptr, host_flags);
    ImGui::PopStyleVar(3);

    draw_menu(chrome);

    const ImGuiID dockspace_id = ImGui::GetID("FTDDock");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
                     ImGuiDockNodeFlags_PassthruCentralNode);
    ensure_layout(dockspace_id);

    if (const ImGuiDockNode* central = ImGui::DockBuilderGetCentralNode(dockspace_id)) {
        const ImVec2 origin = viewport->Pos;
        scene_rect_.x = static_cast<std::int32_t>(central->Pos.x - origin.x);
        scene_rect_.y = static_cast<std::int32_t>(central->Pos.y - origin.y);
        scene_rect_.width = static_cast<std::uint32_t>(central->Size.x);
        scene_rect_.height = static_cast<std::uint32_t>(central->Size.y);
    }

    ImGui::End();

    draw_setup_stack(snapshot, commands, chrome);

    registry_.for_each([&](Panel& panel) {
        if (panel.default_slot() == DockSlot::Setup) return;
        if (panel.default_slot() == DockSlot::Viewport) return;
        bool* open = open_flag(panel.id());
        if (!*open) return;
        const std::string name = window_name(panel);
        PanelContext ctx{snapshot, commands, theme_, history_, open, dpi_scale_,
                         &chrome};
        if (ImGui::Begin(name.c_str(), open, panel.flags())) {
            panel.draw_contents(ctx);
            pending_demand_ = pending_demand_ | panel.needs();
        }
        ImGui::End();
    });

    draw_status(snapshot, chrome);
    draw_play_bar(snapshot, commands, chrome);
    draw_palette(snapshot, commands, chrome);

    if (ImGui::GetIO().WantSaveIniSettings) {
        persist();
        ImGui::GetIO().WantSaveIniSettings = false;
    }
}

}  // namespace ftd::native
