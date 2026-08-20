#pragma once

#include "ui/panel.h"
#include "ui/panel_registry.h"
#include "ui/workspace.h"

#include "native/command_queue.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ftd::native {

// Kind order is the §4.3 ranking order: Action → Panel → Toggle → Field → Scenario.
// Phase 3b ships Action and Panel only; later phases append the rest to the same index.
enum class PaletteKind { Action, Panel, Toggle, Field, Scenario };

struct PaletteEntry {
    PaletteKind kind = PaletteKind::Action;
    std::string id;
    std::string title;
    std::string state;
    std::string keywords;
};

struct PaletteMatch {
    PaletteEntry entry;
    bool prefix = false;
    int fuzzy_score = 0;
    std::size_t source_index = 0;
};

struct PaletteHostState {
    bool paused = true;
    std::string scenario;
    std::string theme_name = "Graphite";
    WorkspaceKind workspace = WorkspaceKind::Experiment;
    bool particles = true;
    bool flux = true;
    bool lattice_box = true;
    std::unordered_map<std::string, bool> panel_open;
    std::unordered_map<std::string, bool> panel_floating;
};

enum class PaletteEffectKind {
    None,
    ShowPanel,
    Pause,
    Run,
    Step,
    ResetScenario,
    ResetDefaults,
    Workspace,
    Theme,
    ToggleParticles,
    ToggleFlux,
    ToggleLatticeBox,
    ResetCamera,
    Quit
};

struct PaletteEffect {
    PaletteEffectKind kind = PaletteEffectKind::None;
    std::string arg;
};

const char* palette_kind_label(PaletteKind kind);
int palette_kind_rank(PaletteKind kind);
bool palette_state_is_enabled(std::string_view state);

std::vector<PaletteEntry> build_phase3b_catalog(const PanelRegistry& registry,
                                                const PaletteHostState& host);

std::vector<PaletteMatch> rank_palette(std::string_view query,
                                       const std::vector<PaletteEntry>& entries);

PaletteEffect effect_for(const PaletteEntry& entry);

void apply_palette_effect(PaletteEffect effect, const PaletteHostState& host,
                          CommandSink& commands, ViewChrome* chrome);

}  // namespace ftd::native
