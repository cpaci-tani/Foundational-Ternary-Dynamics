#pragma once
// Typed inputs for scenario construction only. No tick-law parameters live here.
// With no Context installed the helpers return the original constructor values
// unchanged, preserving the legacy ID-only dispatch and its RNG sequence.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ftd::seed {

using Overrides = std::map<std::string, double>;
using Options = std::vector<std::pair<double, std::string>>;

struct Property {
    std::string key, label, description, units, group, type;
    double default_value = 0, value = 0, minimum = 0, maximum = 0, step = 1;
    double recommended_min = 0, recommended_max = 0;
    std::string recommendation_basis;
    Options options;
};

struct Context {
    Overrides overrides;
    std::vector<Property> properties;
    std::string scenario;

    explicit Context(std::string id, Overrides values = {})
        : overrides(std::move(values)), scenario(std::move(id)) {
        if (overrides.size() > 2048) throw std::invalid_argument("Too many seed inputs");
        for (const auto& item : overrides)
            if (item.first.empty() || item.first.size() > 160 || !std::isfinite(item.second))
                throw std::invalid_argument("Invalid seed input: " + item.first);
    }
    void validate_consumed() const {
        for (const auto& item : overrides) {
            const auto found = std::find_if(properties.begin(), properties.end(),
                [&](const Property& p) { return p.key == item.first; });
            if (found == properties.end())
                throw std::invalid_argument("Unknown or inactive seed property: " + item.first);
        }
    }
};

inline thread_local Context* active = nullptr;
class Scope {
    Context* previous_;
public:
    explicit Scope(Context& context) : previous_(active) { active = &context; }
    ~Scope() { active = previous_; }
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;
};

inline double record(Property p) {
    if (!active) return p.default_value;
    if (p.key.empty() || p.label.empty() || p.description.empty()
        || !std::isfinite(p.minimum) || !std::isfinite(p.maximum)
        || p.minimum > p.maximum || p.step <= 0 || !std::isfinite(p.step))
        throw std::invalid_argument("Incomplete seed descriptor: " + p.key);
    if (std::any_of(active->properties.begin(), active->properties.end(),
        [&](const Property& old) { return old.key == p.key; }))
        throw std::invalid_argument("Duplicate seed binding: " + p.key);
    const auto it = active->overrides.find(p.key);
    p.value = it == active->overrides.end() ? p.default_value : it->second;
    if (!std::isfinite(p.value) || p.value < p.minimum || p.value > p.maximum
        || (p.type != "real" && std::floor(p.value) != p.value))
        throw std::invalid_argument(p.label + ": value outside legal bounds");
    if (!p.options.empty() && std::none_of(p.options.begin(), p.options.end(),
        [&](const auto& option) { return option.first == p.value; }))
        throw std::invalid_argument(p.label + ": choose a legal state");
    p.group = p.key.substr(0, p.key.find('.'));
    if (!std::isfinite(p.recommended_min) || !std::isfinite(p.recommended_max)) {
        const double a = p.default_value == 0 ? -10 * p.step : p.default_value * .5;
        const double b = p.default_value == 0 ? 10 * p.step : p.default_value * 2;
        p.recommended_min = std::clamp(std::min(a, b), p.minimum, p.maximum);
        p.recommended_max = std::clamp(std::max(a, b), p.minimum, p.maximum);
        if (p.type != "real") {
            p.recommended_min = std::ceil(p.recommended_min);
            p.recommended_max = std::floor(p.recommended_max);
        }
        p.recommendation_basis = "Exploratory starting range around the preset; not a validated physical range.";
    } else {
        p.recommended_min = std::clamp(p.recommended_min, p.minimum, p.maximum);
        p.recommended_max = std::clamp(p.recommended_max, p.minimum, p.maximum);
        p.recommendation_basis = "Constructor-authored preparation range; physical qualification remains that of the original preset.";
    }
    if (!p.options.empty()) {
        p.recommended_min = p.minimum;
        p.recommended_max = p.maximum;
        p.recommendation_basis = "Discrete states admitted by this preparation binding.";
    }
    const double result = p.value;
    active->properties.push_back(std::move(p));
    return result;
}

inline double real(const std::string& key, double value, double lo, double hi,
                   const std::string& label, const std::string& explanation,
                   const std::string& units = "lattice units", double step = .01,
                   double recommended_lo = std::numeric_limits<double>::quiet_NaN(),
                   double recommended_hi = std::numeric_limits<double>::quiet_NaN()) {
    Property p;
    p.key = key; p.label = label; p.description = explanation; p.units = units;
    p.type = "real"; p.default_value = value; p.minimum = lo; p.maximum = hi; p.step = step;
    p.recommended_min = recommended_lo; p.recommended_max = recommended_hi;
    return record(std::move(p));
}

inline int integer(const std::string& key, int value, int lo, int hi,
                   const std::string& label, const std::string& explanation,
                   const std::string& units = "count") {
    Property p;
    p.key = key; p.label = label; p.description = explanation; p.units = units;
    p.type = "integer"; p.default_value = value; p.minimum = lo; p.maximum = hi;
    p.recommended_min = p.recommended_max = std::numeric_limits<double>::quiet_NaN();
    return static_cast<int>(record(std::move(p)));
}

inline int choice(const std::string& key, int value, Options options,
                  const std::string& label, const std::string& explanation) {
    if (options.empty()) throw std::invalid_argument("Empty seed choice: " + key);
    Property p;
    p.key = key; p.label = label; p.description = explanation; p.type = "choice";
    p.default_value = value;
    p.minimum = p.maximum = options.front().first;
    for (const auto& option : options) {
        p.minimum = std::min(p.minimum, option.first);
        p.maximum = std::max(p.maximum, option.first);
    }
    p.options = std::move(options);
    return static_cast<int>(record(std::move(p)));
}

inline std::uint32_t uint32(const std::string& key, std::uint32_t value,
                           const std::string& label, const std::string& explanation) {
    Property p;
    p.key = key; p.label = label; p.description = explanation; p.type = "integer";
    p.default_value = value; p.minimum = 0; p.maximum = 4294967295.0;
    p.recommended_min = p.minimum; p.recommended_max = p.maximum;
    return static_cast<std::uint32_t>(record(std::move(p)));
}

inline bool boolean(const std::string& key, bool value, const std::string& label,
                    const std::string& explanation) {
    return choice(key, value ? 1 : 0, {{0, "Off"}, {1, "On"}}, label, explanation) != 0;
}

inline std::string quote(const std::string& value) {
    std::ostringstream out;
    out << '"';
    for (const unsigned char c : value) {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 32) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << int(c) << std::dec;
        else out << c;
    }
    out << '"';
    return out.str();
}

inline std::string describe_json(const Context& context) {
    std::ostringstream out;
    out << std::setprecision(17) << "{\"schemaVersion\":2,\"scenarioId\":" << quote(context.scenario) << ",\"properties\":[";
    bool comma = false;
    for (const auto& p : context.properties) {
        if (comma) out << ',';
        comma = true;
        out << "{\"key\":" << quote(p.key) << ",\"label\":" << quote(p.label)
            << ",\"description\":" << quote(p.description) << ",\"units\":" << quote(p.units)
            << ",\"group\":" << quote(p.group) << ",\"type\":" << quote(p.type)
            << ",\"default\":" << p.default_value << ",\"value\":" << p.value
            << ",\"min\":" << p.minimum << ",\"max\":" << p.maximum << ",\"step\":" << p.step
            << ",\"recommended\":[" << p.recommended_min << ',' << p.recommended_max << ']'
            << ",\"recommendationBasis\":" << quote(p.recommendation_basis)
            << ",\"binding\":" << quote("native:" + context.scenario + ":" + p.key) << ",\"options\":[";
        bool option_comma = false;
        for (const auto& option : p.options) {
            if (option_comma) out << ',';
            option_comma = true;
            out << '[' << option.first << ',' << quote(option.second) << ']';
        }
        out << "]}";
    }
    out << "]}";
    return out.str();
}
} // namespace ftd::seed
