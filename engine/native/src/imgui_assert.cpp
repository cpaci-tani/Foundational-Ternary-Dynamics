#include "native/imgui_assert.h"

#include <cstdio>
#include <cstdlib>

namespace {

void ftd_imgui_default_assert(const char* expr, const char* file, int line) {
    std::fprintf(stderr, "IM_ASSERT(%s) failed at %s:%d\n",
                 expr ? expr : "(null)",
                 file ? file : "(null)",
                 line);
    std::fflush(stderr);
    std::abort();
}

ftd::native::ImGuiAssertHandler g_handler = &ftd_imgui_default_assert;

}  // namespace

void ftd_imgui_assert_dispatch(const char* expr, const char* file, int line) {
    ftd::native::ImGuiAssertHandler handler = g_handler;
    if (handler == nullptr) {
        handler = &ftd_imgui_default_assert;
    }
    handler(expr, file, line);
}

namespace ftd::native {

ImGuiAssertHandler set_imgui_assert_handler(ImGuiAssertHandler handler) {
    ImGuiAssertHandler previous = g_handler;
    g_handler = (handler != nullptr) ? handler : &ftd_imgui_default_assert;
    return previous;
}

}  // namespace ftd::native
