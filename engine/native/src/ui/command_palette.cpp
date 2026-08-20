#include "ui/command_palette.h"

#include <algorithm>
#include <utility>

namespace ftd::native {
namespace {

std::string ascii_lower(std::string_view in) {
    std::string out(in.begin(), in.end());
    for (char& c : out) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return out;
}

bool starts_with(std::string_view hay, std::string_view needle) {
    return hay.size() >= needle.size()
        && hay.substr(0, needle.size()) == needle;
}

int fuzzy_score(std::string_view hay, std::string_view needle) {
    if (needle.empty()) return 0;
    std::size_t hi = 0;
    std::size_t first = std::string_view::npos;
    std::size_t last = 0;
    for (char n : needle) {
        while (hi < hay.size() && hay[hi] != n) ++hi;
        if (hi >= hay.size()) return -1;
        if (first == std::string_view::npos) first = hi;
        last = hi;
        ++hi;
    }
    const int span = static_cast<int>(last - first + 1);
    return static_cast<int>(needle.size()) * 1000 - span - static_cast<int>(first);
}

bool host_open(const PaletteHostState& host, const std::string& id) {
    const auto it = host.panel_open.find(id);
    return it != host.panel_open.end() && it->second;
}

bool host_floating(const PaletteHostState& host, const std::string& id) {
    const auto it = host.panel_floating.find(id);
    return it != host.panel_floating.end() && it->second;
}

const char* panel_state(const PaletteHostState& host, const std::string& id) {
    if (!host_open(host, id)) return "hidden";
    if (host_floating(host, id)) return "floating";
    return "visible";
}

void add_action(std::vector<PaletteEntry>& out, std::string id, std::string title,
                std::string state, std::string keywords = {}) {
    PaletteEntry entry;
    entry.kind = PaletteKind::Action;
    entry.id = std::move(id);
    entry.title = std::move(title);
    entry.state = std::move(state);
    entry.keywords = std::move(keywords);
    out.push_back(std::move(entry));
}

}  // namespace

const char* palette_kind_label(PaletteKind kind) {
    switch (kind) {
        case PaletteKind::Action:
            return "Action";
        case PaletteKind::Panel:
            return "Panel";
        case PaletteKind::Toggle:
            return "Toggle";
        case PaletteKind::Field:
            return "Field";
        case PaletteKind::Scenario:
            return "Scenario";
    }
    return "Action";
}

int palette_kind_rank(PaletteKind kind) {
    return static_cast<int>(kind);
}

bool palette_state_is_enabled(std::string_view state) {
    return state.size() < 9 || state.substr(0, 9) != "disabled:";
}

std::vector<PaletteEntry> build_phase3b_catalog(const PanelRegistry& registry,
                                                const PaletteHostState& host) {
    std::vector<PaletteEntry> out;

    add_action(out, "action.pause", "Pause simulation",
               host.paused ? "disabled: already paused" : "enabled", "space");
    add_action(out, "action.resume", "Resume simulation",
               host.paused ? "enabled" : "disabled: already running", "play space");
    add_action(out, "action.step", "Step one tick", "enabled", "s");
    add_action(out, "action.reset", "Reset scenario",
               host.scenario.empty() ? "disabled: no scenario loaded" : "enabled",
               "r restart");
    add_action(out, "action.reset_defaults", "Reset to shipping defaults",
               "enabled", "toggles");
    add_action(out, "action.workspace.experiment", "Switch to Experiment workspace",
               host.workspace == WorkspaceKind::Experiment
                   ? "disabled: already active"
                   : "enabled");
    add_action(out, "action.workspace.analysis", "Switch to Analysis workspace",
               host.workspace == WorkspaceKind::Analysis
                   ? "disabled: already active"
                   : "enabled");
    add_action(out, "action.workspace.presentation",
               "Switch to Presentation workspace",
               host.workspace == WorkspaceKind::Presentation
                   ? "disabled: already active"
                   : "enabled");
    add_action(out, "action.theme.graphite", "Theme: Graphite",
               host.theme_name == "Graphite" ? "disabled: already active" : "enabled");
    add_action(out, "action.theme.contrast", "Theme: Contrast",
               host.theme_name == "Contrast" ? "disabled: already active" : "enabled");
    add_action(out, "action.theme.slate", "Theme: Slate",
               host.theme_name == "Slate" ? "disabled: already active" : "enabled");
    add_action(out, "action.theme.carbon", "Theme: Carbon",
               host.theme_name == "Carbon" ? "disabled: already active" : "enabled");
    add_action(out, "action.chrome.particles", "Toggle particles overlay",
               "enabled");
    add_action(out, "action.chrome.flux", "Toggle flux overlay", "enabled");
    add_action(out, "action.chrome.lattice_box", "Toggle lattice box", "enabled");
    add_action(out, "action.reset_camera", "Reset camera", "enabled");
    add_action(out, "action.quit", "Exit", "enabled", "quit close");

    registry.for_each([&](const Panel& panel) {
        PaletteEntry entry;
        entry.kind = PaletteKind::Panel;
        entry.id = panel.id();
        entry.title = panel.title();
        entry.state = panel_state(host, panel.id());
        out.push_back(std::move(entry));
    });
    return out;
}

std::vector<PaletteMatch> rank_palette(std::string_view query,
                                       const std::vector<PaletteEntry>& entries) {
    const std::string q = ascii_lower(query);
    std::vector<PaletteMatch> out;
    out.reserve(entries.size());
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const PaletteEntry& entry = entries[i];
        const std::string title = ascii_lower(entry.title);
        const std::string id = ascii_lower(entry.id);
        const std::string keywords = ascii_lower(entry.keywords);
        PaletteMatch match;
        match.entry = entry;
        match.source_index = i;
        if (q.empty()) {
            out.push_back(std::move(match));
            continue;
        }
        const bool prefix = starts_with(title, q) || starts_with(id, q);
        int score = fuzzy_score(title, q);
        score = std::max(score, fuzzy_score(id, q));
        score = std::max(score, fuzzy_score(keywords, q));
        if (!prefix && score < 0) continue;
        match.prefix = prefix;
        match.fuzzy_score = score < 0 ? 0 : score;
        out.push_back(std::move(match));
    }
    std::sort(out.begin(), out.end(), [](const PaletteMatch& a, const PaletteMatch& b) {
        if (a.prefix != b.prefix) return a.prefix && !b.prefix;
        const int ka = palette_kind_rank(a.entry.kind);
        const int kb = palette_kind_rank(b.entry.kind);
        if (ka != kb) return ka < kb;
        if (a.fuzzy_score != b.fuzzy_score) return a.fuzzy_score > b.fuzzy_score;
        const std::string ta = ascii_lower(a.entry.title);
        const std::string tb = ascii_lower(b.entry.title);
        if (ta != tb) return ta < tb;
        return a.source_index < b.source_index;
    });
    return out;
}

PaletteEffect effect_for(const PaletteEntry& entry) {
    PaletteEffect effect;
    if (entry.kind == PaletteKind::Panel) {
        effect.kind = PaletteEffectKind::ShowPanel;
        effect.arg = entry.id;
        return effect;
    }
    if (entry.kind != PaletteKind::Action || !palette_state_is_enabled(entry.state)) {
        return effect;
    }
    if (entry.id == "action.pause") effect.kind = PaletteEffectKind::Pause;
    else if (entry.id == "action.resume") effect.kind = PaletteEffectKind::Run;
    else if (entry.id == "action.step") effect.kind = PaletteEffectKind::Step;
    else if (entry.id == "action.reset") effect.kind = PaletteEffectKind::ResetScenario;
    else if (entry.id == "action.reset_defaults") {
        effect.kind = PaletteEffectKind::ResetDefaults;
    } else if (entry.id == "action.workspace.experiment") {
        effect.kind = PaletteEffectKind::Workspace;
        effect.arg = "Experiment";
    } else if (entry.id == "action.workspace.analysis") {
        effect.kind = PaletteEffectKind::Workspace;
        effect.arg = "Analysis";
    } else if (entry.id == "action.workspace.presentation") {
        effect.kind = PaletteEffectKind::Workspace;
        effect.arg = "Presentation";
    } else if (entry.id == "action.theme.graphite") {
        effect.kind = PaletteEffectKind::Theme;
        effect.arg = "Graphite";
    } else if (entry.id == "action.theme.contrast") {
        effect.kind = PaletteEffectKind::Theme;
        effect.arg = "Contrast";
    } else if (entry.id == "action.theme.slate") {
        effect.kind = PaletteEffectKind::Theme;
        effect.arg = "Slate";
    } else if (entry.id == "action.theme.carbon") {
        effect.kind = PaletteEffectKind::Theme;
        effect.arg = "Carbon";
    } else if (entry.id == "action.chrome.particles") {
        effect.kind = PaletteEffectKind::ToggleParticles;
    } else if (entry.id == "action.chrome.flux") {
        effect.kind = PaletteEffectKind::ToggleFlux;
    } else if (entry.id == "action.chrome.lattice_box") {
        effect.kind = PaletteEffectKind::ToggleLatticeBox;
    } else if (entry.id == "action.reset_camera") {
        effect.kind = PaletteEffectKind::ResetCamera;
    } else if (entry.id == "action.quit") {
        effect.kind = PaletteEffectKind::Quit;
    }
    return effect;
}

void apply_palette_effect(PaletteEffect effect, const PaletteHostState& host,
                          CommandSink& commands, ViewChrome* chrome) {
    switch (effect.kind) {
        case PaletteEffectKind::Pause:
            commands.push(Pause{});
            if (chrome && chrome->paused) chrome->paused->store(true);
            break;
        case PaletteEffectKind::Run:
            commands.push(Run{});
            if (chrome && chrome->paused) chrome->paused->store(false);
            break;
        case PaletteEffectKind::Step:
            commands.push(Pause{});
            commands.push(Step{1});
            if (chrome && chrome->paused) chrome->paused->store(true);
            break;
        case PaletteEffectKind::ResetScenario:
            if (!host.scenario.empty()) commands.push(LoadScenario{host.scenario});
            break;
        case PaletteEffectKind::ResetDefaults:
            commands.push(ResetToDefaults{});
            break;
        case PaletteEffectKind::ToggleParticles:
            if (chrome && chrome->particles) *chrome->particles = !*chrome->particles;
            break;
        case PaletteEffectKind::ToggleFlux:
            if (chrome && chrome->flux) *chrome->flux = !*chrome->flux;
            break;
        case PaletteEffectKind::ToggleLatticeBox:
            if (chrome && chrome->lattice_box) {
                *chrome->lattice_box = !*chrome->lattice_box;
            }
            break;
        case PaletteEffectKind::ResetCamera:
            if (chrome && chrome->reset_camera) *chrome->reset_camera = true;
            break;
        case PaletteEffectKind::Quit:
            if (chrome && chrome->request_quit) *chrome->request_quit = true;
            break;
        default:
            break;
    }
}

}  // namespace ftd::native
