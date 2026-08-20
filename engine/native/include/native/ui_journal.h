#pragma once

#include <cstdint>
#include <string>

namespace ftd::native {

enum class JKind { Bool, Double, UInt, Enum, Boundary, ScenarioId };

struct JValue {
    JKind kind = JKind::Bool;
    bool b = false;
    double d = 0.0;
    unsigned u = 0;
    int e = 0;
    std::string s;
};

struct JournalEntry {
    int tick_applied = 0;
    std::string key;
    JValue old_value;
    JValue requested;
    JValue applied;
};

}  // namespace ftd::native
