/**
 * CLI demo scenarios extracted from main.cpp.
 *
 * These are the interactive demonstration scenarios dispatched from
 * the ftd_sim command-line entry point. Each function runs a
 * self-contained experiment and prints results to stdout, optionally
 * exporting CSV artifacts to an output directory.
 *
 * The scenarios are pedagogical / exploratory — not the formal test
 * suite. For that, see engine/tests/.
 */
#pragma once

#include <string>

namespace ftd {
namespace cli_demos {

// Banner printed at CLI startup.
void print_header();

// ---- Scenarios without file output ----
void scenario_A(int lattice_size, int num_ticks);
void scenario_B(int lattice_size, int num_ticks);
void scenario_default(int lattice_size, int num_ticks);  // Scenario D
void scenario_E(int lattice_size, int num_ticks);
void scenario_F(int lattice_size, int num_ticks);
void scenario_G(int lattice_size, int num_ticks);

// ---- Scenarios that export CSV artifacts to outdir ----
void scenario_H(int lattice_size, int num_ticks, const std::string& outdir);
void scenario_I(int lattice_size, int num_ticks, const std::string& outdir);
void scenario_J(int lattice_size, int num_ticks, const std::string& outdir);
void scenario_K(int lattice_size, int num_ticks, const std::string& outdir);
void scenario_V(int lattice_size, int num_ticks, const std::string& outdir,
                int frame_interval = 0, int spatial_stride = 1);

}  // namespace cli_demos
}  // namespace ftd
