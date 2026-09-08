#pragma once
// Bounded native command JSON. Numbers retain their decimal token so integer
// validation cannot accept fractions rounded to integers by binary64.
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ftd {
struct JsonValue {
    enum class Kind { Null, Boolean, Number, String, Object, Array };
    Kind kind = Kind::Null;
    std::string text;
    bool flag = false;
    double numeric = 0;
    std::map<std::string, JsonValue> members;
    std::vector<JsonValue> elements;

    bool has(const std::string& key) const;
    const JsonValue& at(const std::string& key) const;
    const std::map<std::string, JsonValue>& object() const;
    const std::string& string() const;
    double number() const;
    bool boolean() const;
    // Bounds must be inside the JSON/JS exact-integer interval +/- (2^53-1).
    std::int64_t integer(std::int64_t lo, std::int64_t hi) const;
    const std::string& string(const std::string& key) const { return at(key).string(); }
    double number(const std::string& key) const { return at(key).number(); }
    bool boolean(const std::string& key) const { return at(key).boolean(); }
    std::int64_t integer(const std::string& key, std::int64_t lo, std::int64_t hi) const {
        return at(key).integer(lo, hi);
    }
    std::string string_or(const std::string& key, std::string fallback = {}) const;
    double number_or(const std::string& key, double fallback = 0) const;
    bool boolean_or(const std::string& key, bool fallback = false) const;
    std::int64_t integer_or(const std::string& key, std::int64_t fallback,
                           std::int64_t lo, std::int64_t hi) const;
};
inline constexpr std::int64_t kJsonSafeInteger = 9007199254740991LL;
// One complete object, <=65536 input bytes and <=64 nested containers. Strict
// JSON syntax/UTF-8, paired Unicode surrogates, duplicate decoded keys rejected
// at every depth. Numbers must convert to finite binary64 without conversion range errors
// (overflow and underflow beyond representable range are rejected).
// Throws invalid_argument; no input or engine state is modified.
JsonValue parse_json_object(const std::string& input);
} // namespace ftd
