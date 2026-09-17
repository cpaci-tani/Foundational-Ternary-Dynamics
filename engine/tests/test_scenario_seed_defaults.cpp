// Independent pre-refactor receipts for the complete supported initial state.
// Explicit --capture is used once BEFORE editing constructors. Normal tests
// only compare; they never rewrite a changed expected result.
#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/dynamical_state_digest.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "support/scenario_seed_receipt.h"

int main(int argc, char** argv) {
#ifdef _OPENMP
    // The immutable capture includes all 32 native RNG streams. CTest's
    // scheduling policy otherwise changes their count to eight, including
    // for an empty lattice, without any constructor/state change.
    omp_set_num_threads(32);
#endif
    try {
        const bool capture = argc == 3 && std::string(argv[1]) == "--capture";
        const bool describe = argc == 3 && std::string(argv[1]) == "--describe";
        if (describe) {
            std::ofstream schemas(argv[2]);
            schemas << "{\"schemaVersion\":2,\"presets\":{";
            bool comma = false;
            for (const int size : {17, 32, 33}) for (const auto name : ftd::scale0_scenario_ids()) {
                ftd::RenderBridge rb(size); rb.force_cpu();
                ftd::seed::Context context{std::string(name)};
                if (!ftd::dispatch_scenario_seed(rb, context)) throw std::runtime_error("Unhandled seed");
                if (comma) schemas << ',';
                comma = true;
                schemas << ftd::seed::quote(std::string(name) + "@" + std::to_string(size)) << ':'
                        << ftd::seed::describe_json(context);
            }
            schemas << "}}\n";
            return schemas ? 0 : 1;
        }
        const auto path = capture ? std::filesystem::path(argv[2])
            : std::filesystem::path(__FILE__).parent_path().parent_path() / "config/scenario_seed_defaults.txt";
        if (capture && std::filesystem::exists(path)) throw std::runtime_error("Refusing to overwrite baseline");
        std::ifstream input;
        std::ofstream output;
        if (capture) output.open(path); else input.open(path);
        if (capture ? !output : !input) throw std::runtime_error("Cannot open baseline: " + path.string());
        int checked = 0, failed = 0;
        for (const int size : {17, 32, 33}) {
            for (const auto name : ftd::scale0_scenario_ids()) {
                const std::string id(name);
                ftd::RenderBridge rb(size);
                rb.force_cpu();
                if (!ftd::dispatch_scenario(rb, id)) throw std::runtime_error("Unhandled scenario: " + id);
                const auto actual = seed_receipt(rb, id);
                if (capture) output << actual << '\n';
                else {
                    std::string expected;
                    if (!std::getline(input, expected) || expected != actual) {
                        std::cerr << "Default state changed: " << id << " L=" << size << '\n';
                        ++failed;
                    }
                    try {
                        ftd::RenderBridge seeded(size); seeded.force_cpu();
                        ftd::seed::Context context(id);
                        if (!ftd::dispatch_scenario_seed(seeded, context) || seed_receipt(seeded, id) != expected) {
                            std::cerr << "Typed default state changed: " << id << " L=" << size << '\n';
                            ++failed;
                        }
                    } catch (const std::exception& error) {
                        std::cerr << id << " L=" << size << ": " << error.what() << '\n'; ++failed;
                    }
                }
                ++checked;
            }
        }
        std::string trailing;
        if (!capture && std::getline(input, trailing)) throw std::runtime_error("Unmatched baseline row");
        std::cout << checked << " native seed defaults " << (capture ? "captured" : "checked") << "; " << failed << " failures\n";
        return failed ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
