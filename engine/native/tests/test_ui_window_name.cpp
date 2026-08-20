#include "ui/panel.h"
#include "ui/panel_registry.h"

#include "ftd/test_telemetry.h"

int main() {
    ftd::test::init("test_ui_window_name");

    const auto registry = ftd::native::PanelRegistry::make_default();
    const ftd::native::Panel* scenarios = registry.find("scenarios");
    ftd::test::check("scenarios panel exists", scenarios != nullptr);
    const std::string a = ftd::native::window_name(*scenarios);
    const std::string b = ftd::native::window_name(*scenarios);
    ftd::test::check("composed name contains ###", a.find("###") != std::string::npos);
    ftd::test::check("composed name is stable", a == b);
    ftd::test::check("composed name uses the stable id",
                     a.find("###scenarios") != std::string::npos);
    ftd::test::check("title is the display prefix", a.find("Scenarios###") == 0);

    registry.for_each([&](const ftd::native::Panel& panel) {
        const std::string name = ftd::native::window_name(panel);
        ftd::test::check((std::string("### present: ") + panel.id()).c_str(),
                         name.find(std::string("###") + panel.id()) != std::string::npos);
    });

    return ftd::test::finalize();
}
