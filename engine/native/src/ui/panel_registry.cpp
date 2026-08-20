#include "ui/panel_registry.h"
#include "ui/panels/panels.h"

namespace ftd::native {

void PanelRegistry::add(std::unique_ptr<Panel> panel) {
    if (panel) panels_.push_back(std::move(panel));
}

Panel* PanelRegistry::find(const std::string& id) const {
    for (const auto& panel : panels_) {
        if (panel && id == panel->id()) return panel.get();
    }
    return nullptr;
}

PanelRegistry PanelRegistry::make_default() {
    PanelRegistry registry;
    registry.add(make_scenario_browser_panel());
    registry.add(make_run_config_panel());
    registry.add(make_play_bar_panel());
    registry.add(make_substrate_panel());
    registry.add(make_telemetry_panel());
    registry.add(make_audit_panel());
    registry.add(make_lagrangian_panel());
    registry.add(make_inspector_panel());
    registry.add(make_physics_terms_panel());
    registry.add(make_fields_panel());
    registry.add(make_log_panel());
    return registry;
}

}  // namespace ftd::native
