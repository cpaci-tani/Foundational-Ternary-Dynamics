#pragma once

#include "ui/panel.h"

#include <memory>
#include <string>
#include <vector>

namespace ftd::native {

class PanelRegistry {
public:
    static PanelRegistry make_default();

    void add(std::unique_ptr<Panel> panel);
    Panel* find(const std::string& id) const;
    const std::vector<std::unique_ptr<Panel>>& panels() const { return panels_; }

    template <typename Fn>
    void for_each(Fn&& fn) {
        for (auto& panel : panels_) {
            if (panel) fn(*panel);
        }
    }

    template <typename Fn>
    void for_each(Fn&& fn) const {
        for (const auto& panel : panels_) {
            if (panel) fn(*panel);
        }
    }

private:
    std::vector<std::unique_ptr<Panel>> panels_;
};

}  // namespace ftd::native
