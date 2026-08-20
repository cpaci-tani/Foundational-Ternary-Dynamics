#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ftd::native {

struct SeriesKey {
    std::uint8_t group = 0;
    std::uint16_t field = 0;

    bool operator<(const SeriesKey& other) const {
        if (group != other.group) return group < other.group;
        return field < other.field;
    }
};

struct Series {
    std::vector<int> tick;
    std::vector<double> value;
};

class History {
public:
    static constexpr std::size_t kCapacity = 4096;
    const Series* find(SeriesKey) const { return nullptr; }
};

}  // namespace ftd::native
