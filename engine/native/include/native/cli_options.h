#pragma once

#include "native/engine_session.h"

#include <string>

namespace ftd::native {

struct NativeDesktopCli {
    NativeEngineOptions options;
    bool help = false;
    bool parse_error = false;
    std::string error;
};

NativeDesktopCli parse_native_cli(int argc, const char* const* argv);

}  // namespace ftd::native
