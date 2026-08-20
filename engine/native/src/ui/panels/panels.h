#pragma once

#include "ui/panel.h"

#include <memory>

namespace ftd::native {

std::unique_ptr<Panel> make_scenario_browser_panel();
std::unique_ptr<Panel> make_run_config_panel();
std::unique_ptr<Panel> make_play_bar_panel();
std::unique_ptr<Panel> make_telemetry_panel();
std::unique_ptr<Panel> make_audit_panel();
std::unique_ptr<Panel> make_lagrangian_panel();
std::unique_ptr<Panel> make_inspector_panel();
std::unique_ptr<Panel> make_physics_terms_panel();
std::unique_ptr<Panel> make_fields_panel();
std::unique_ptr<Panel> make_log_panel();
std::unique_ptr<Panel> make_substrate_panel();

}  // namespace ftd::native
