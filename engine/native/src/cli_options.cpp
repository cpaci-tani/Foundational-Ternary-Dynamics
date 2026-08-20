#include "native/cli_options.h"

#include <string>

namespace ftd::native {

NativeDesktopCli parse_native_cli(int argc, const char* const* argv) {
    NativeDesktopCli parsed;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--cpu") {
            parsed.options.force_cpu = true;
        } else if (arg == "--gpu") {
            parsed.options.force_cpu = false;
        } else if (arg == "--no-ui") {
            parsed.options.no_ui = true;
        } else if (arg == "--lattice" && i + 1 < argc) {
            parsed.options.lattice_size = std::stoi(argv[++i]);
        } else if (arg == "--scenario" && i + 1 < argc) {
            parsed.options.scenario = argv[++i];
        } else if (arg == "--help") {
            parsed.help = true;
        }
    }
    return parsed;
}

}  // namespace ftd::native
