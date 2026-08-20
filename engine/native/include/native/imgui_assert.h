#pragma once

namespace ftd::native {

using ImGuiAssertHandler = void (*)(const char* expr, const char* file, int line);

// Replace the process-wide IM_ASSERT handler. Passing nullptr restores the
// default aborting handler. Returns the previous handler.
ImGuiAssertHandler set_imgui_assert_handler(ImGuiAssertHandler handler);

class ScopedImGuiAssertHandler {
public:
    explicit ScopedImGuiAssertHandler(ImGuiAssertHandler handler)
        : previous_(set_imgui_assert_handler(handler)) {}
    ~ScopedImGuiAssertHandler() { set_imgui_assert_handler(previous_); }

    ScopedImGuiAssertHandler(const ScopedImGuiAssertHandler&) = delete;
    ScopedImGuiAssertHandler& operator=(const ScopedImGuiAssertHandler&) = delete;

private:
    ImGuiAssertHandler previous_;
};

}  // namespace ftd::native
