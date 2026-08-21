// app/app_options.cpp — native_app CLI parsing (see app/app_options.h).

#include "app/app_options.h"

#include <algorithm>
#include <cstdlib>

namespace ftd::native::app {

AppOptions parse_app_options(const std::vector<std::string>& args) {
    AppOptions o;
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--capture-frames" && i + 1 < args.size()) {
            o.capture_frames = std::max(1, std::atoi(args[++i].c_str()));
        } else if (args[i] == "--paused") {
            o.start_paused = true;
        } else if (args[i] == "--run") {
            o.start_paused = false;
        } else if (args[i] == "--scale" && i + 1 < args.size()) {
            o.scale = std::max(0, std::atoi(args[++i].c_str()));
        } else if (args[i] == "--field" && i + 1 < args.size()) {
            o.field = args[++i];
        } else if (args[i] == "--overlays" && i + 1 < args.size()) {
            o.overlays = args[++i];
        } else if (args[i] == "--sheet-height" && i + 1 < args.size()) {
            // "<name>,<frac>" — the comma splits the overlay name from the height.
            const std::string spec = args[++i];
            const auto comma = spec.find(',');
            if (comma != std::string::npos && comma > 0) {
                const std::string nm = spec.substr(0, comma);
                const float frac = static_cast<float>(std::atof(spec.c_str() + comma + 1));
                o.sheet_heights.emplace_back(nm, frac);
            }
        } else if (args[i] == "--no-prime-tick") {
            o.prime_tick = false;
        } else if (args[i] == "--prime-tick") {
            o.prime_tick = true;
        } else if (args[i] == "--png-out" && i + 1 < args.size()) {
            o.png_out = args[++i];
        } else if (args[i] == "--inspect-voxel" && i + 1 < args.size()) {
            o.inspect_voxel = args[++i];
        } else if (args[i] == "--inspect-particle" && i + 1 < args.size()) {
            o.inspect_particle = std::atoi(args[++i].c_str());
            o.have_inspect_particle = true;
        } else if (args[i] == "--pick-scenario" && i + 1 < args.size()) {
            o.pick_scenario = args[++i];
        } else if (args[i] == "--open-physics") {
            o.open_physics = true;
        } else if (args[i] == "--open-config") {
            o.open_config = true;
        } else if (args[i] == "--open-overlays") {
            o.open_overlays = true;
        } else if (args[i] == "--open-controls") {   // both sections
            o.open_physics = true;
            o.open_config = true;
        } else if (args[i] == "--expand-all-tog") {
            o.expand_all_tog = true;
        } else if (args[i] == "--expand-tog-group" && i + 1 < args.size()) {
            o.expand_tog_groups.push_back(args[++i]);
        } else if (args[i] == "--no-scroll") {
            o.no_scroll = true;
        } else if (args[i] == "--open-telemetry") {
            o.open_telemetry = true;
        } else if (args[i] == "--toggle-on" && i + 1 < args.size()) {
            o.toggles_on.push_back(args[++i]);
        } else if (args[i] == "--toggle-off" && i + 1 < args.size()) {
            o.toggles_off.push_back(args[++i]);
        } else if (args[i] == "--set-dt" && i + 1 < args.size()) {
            o.set_dt = true;
            o.dt_value = std::atof(args[++i].c_str());
        } else if (args[i] == "--set-sor" && i + 1 < args.size()) {
            o.set_sor = true;
            o.sor_value = std::atoi(args[++i].c_str());
        } else if (args[i] == "--set-boundary" && i + 1 < args.size()) {
            o.set_boundary = true;
            o.boundary_value = std::atoi(args[++i].c_str());
        } else if (args[i] == "--set-lattice" && i + 1 < args.size()) {
            o.set_lattice = std::atoi(args[++i].c_str());
        } else if (args[i] == "--force-style" && i + 1 < args.size()) {
            o.force_style = args[++i];
        } else if (args[i] == "--profile-ui" && i + 1 < args.size()) {
            o.profile_ui_frames = std::max(1, std::atoi(args[++i].c_str()));
        } else if (args[i] == "--scn-open") {
            o.scn_open = true;
        } else if (args[i] == "--scn-expand-cat" && i + 1 < args.size()) {
            o.scn_expand_cat = std::atoi(args[++i].c_str());
            o.scn_open = true;
        } else if (args[i] == "--profile-freeze-status") {
            o.profile_freeze_status = true;
        }
    }
    return o;
}

ftd::native::ForceStyle parse_force_style(const std::string& s) {
    if (s == "heatmap") return ftd::native::ForceStyle::Heatmap;
    if (s == "flow")    return ftd::native::ForceStyle::Flow;
    if (s == "glyphs")  return ftd::native::ForceStyle::Glyphs;
    return ftd::native::ForceStyle::Arrows;  // "arrows" / empty / unknown
}

}  // namespace ftd::native::app
