// Isolated Embind adapter of the same finite hydro kernel/codec as the native CLI
// (mirrors ../wasm_bindings.cpp). Unlike the strict candidate, this runtime needs a
// 64 MiB collision-table blob installed before any state can be constructed, and its
// checkpoint format is the Python oracle's JSON schema (base64 arrays), not the native
// FTDHY01 binary transport -- so bindings-local base64/JSON plumbing is unavoidable.
#include "hydro_runtime.h"
#include "hydro_tables.h"
#include "../frozen_tables.h"  // A9 relation alphabet: EPS, BLANK (shared by counts/fields)
#include <emscripten/bind.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using ftd::hydro::State;
using emscripten::val;
namespace tables = ftd::hydro::tables;
namespace frozen = ::ftd::strict::frozen;
using Byte = std::uint8_t;

std::uint64_t decimal(const std::string& text) {
    if (text.empty() || (text.size() > 1 && text[0] == '0'))
        throw std::invalid_argument("expected canonical unsigned decimal string");
    std::uint64_t value = 0;
    for (char c : text) {
        if (c < '0' || c > '9' || value > (std::numeric_limits<std::uint64_t>::max() - (c - '0')) / 10)
            throw std::invalid_argument("decimal outside uint64 alphabet");
        value = value * 10 + (c - '0');
    }
    return value;
}

long long signed_integer(const std::string& text) {
    if (text.empty()) throw std::invalid_argument("empty integer literal in moments observable");
    std::size_t i = 0;
    const bool neg = text[0] == '-';
    if (neg) i = 1;
    if (i >= text.size() || (text.size() - i > 1 && text[i] == '0') || text == "-0")
        throw std::invalid_argument("invalid integer literal in moments observable");
    const auto limit = std::uint64_t(std::numeric_limits<long long>::max()) + (neg ? 1u : 0u);
    std::uint64_t value = 0;
    for (; i < text.size(); ++i) {
        const char c = text[i];
        if (c < '0' || c > '9' || value > (limit - (c - '0')) / 10)
            throw std::invalid_argument("moments integer outside int64 alphabet");
        value = value * 10 + (c - '0');
    }
    if (neg && value == limit) return std::numeric_limits<long long>::min();
    return neg ? -static_cast<long long>(value) : static_cast<long long>(value);
}

std::string string_value(const val& input) {
    if (input.typeOf().as<std::string>() != "string")
        throw std::invalid_argument("expected a string without coercion");
    return input.as<std::string>();
}

template <std::size_t N>
void object_keys(const val& input, const std::array<const char*, N>& expected) {
    if (input.isNull() || input.typeOf().as<std::string>() != "object" ||
        val::global("Array").call<bool>("isArray", input))
        throw std::invalid_argument("checkpoint fields must be JSON objects");
    const auto keys = val::global("Object").call<val>("keys", input);
    if (keys["length"].as<unsigned>() != N)
        throw std::invalid_argument("checkpoint object key mismatch");
    for (unsigned i = 0; i < N; ++i) {
        const auto key = keys[i].as<std::string>();
        if (std::none_of(expected.begin(), expected.end(), [&](const char* item) { return key == item; }))
            throw std::invalid_argument("checkpoint object key mismatch");
    }
}

double integer_number(const val& input, double minimum, double maximum) {
    if (input.typeOf().as<std::string>() != "number")
        throw std::invalid_argument("expected a numeric integer without coercion");
    const double value = input.as<double>();
    if (!std::isfinite(value) || std::floor(value) != value || value < minimum || value > maximum ||
        (value == 0 && std::signbit(value)))
        throw std::invalid_argument("numeric integer outside admitted range");
    return value;
}

// JSON.parse validates grammar but discards duplicate keys and rounds number
// tokens. The checkpoint permits only integer number tokens (L and legacy tick).
void validate_json_tokens(const std::string& text) {
    std::vector<std::vector<std::string>> objects;
    const auto whitespace = [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '{') objects.emplace_back();
        else if (text[i] == '}') objects.pop_back();
        else if (text[i] == '"') {
            const auto start = i++;
            for (; i < text.size() && text[i] != '"'; ++i)
                if (text[i] == '\\') ++i;
            auto next = i + 1;
            while (next < text.size() && whitespace(text[next])) ++next;
            if (next < text.size() && text[next] == ':') {
                const auto key = val::global("JSON").call<val>("parse", val(text.substr(start, i - start + 1))).as<std::string>();
                auto& keys = objects.back();
                if (std::find(keys.begin(), keys.end(), key) != keys.end())
                    throw std::invalid_argument("checkpoint duplicate object key");
                keys.push_back(key);
            }
        } else if (text[i] == '-' || (text[i] >= '0' && text[i] <= '9')) {
            const auto start = i;
            while (i + 1 < text.size() &&
                   ((text[i + 1] >= '0' && text[i + 1] <= '9') || text[i + 1] == '.' ||
                    text[i + 1] == 'e' || text[i + 1] == 'E' || text[i + 1] == '+' || text[i + 1] == '-')) ++i;
            const auto token = text.substr(start, i - start + 1);
            if (token == "-0" || token.find_first_of(".eE") != std::string::npos)
                throw std::invalid_argument("checkpoint number token must be an integer without exponent or negative zero");
        }
    }
}

// Same realm-independent Uint8Array brand check as ../wasm_bindings.cpp's `bytes()`
// (rejects SharedArrayBuffer and disguised/foreign buffers); used only by loadTable.
std::vector<Byte> bytes(const val& input) {
    if (!input.instanceof(val::global("Uint8Array")))
        throw std::invalid_argument("table blob must be Uint8Array");
    const auto descriptor = val::global("Object").call<val>("getOwnPropertyDescriptor",
        val::global("ArrayBuffer")["prototype"], val("byteLength"));
    const auto typedPrototype = val::global("Object").call<val>("getPrototypeOf", val::global("Uint8Array")["prototype"]);
    const auto bufferDescriptor = val::global("Object").call<val>("getOwnPropertyDescriptor", typedPrototype, val("buffer"));
    const auto buffer = bufferDescriptor["get"].call<val>("call", input);
    descriptor["get"].call<val>("call", buffer);
    return emscripten::convertJSArrayToNumberVector<Byte>(input);
}

std::string hex_string(const unsigned char* data, std::size_t n) {
    static const char digits[] = "0123456789abcdef";
    std::string out;
    out.reserve(n * 2);
    for (std::size_t i = 0; i < n; ++i) {
        out += digits[data[i] >> 4];
        out += digits[data[i] & 0xF];
    }
    return out;
}

const std::string& table_hash_hex() {
    static const std::string cached = hex_string(tables::TABLE_HASH, 32);
    return cached;
}
const std::string& encoding_hash_hex() {
    static const std::string cached = hex_string(tables::ENCODING_HASH, 32);
    return cached;
}

std::string base64_encode(const std::vector<Byte>& data) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve((data.size() + 2) / 3 * 4);
    std::size_t i = 0;
    for (; i + 3 <= data.size(); i += 3) {
        const std::uint32_t v = (std::uint32_t(data[i]) << 16) | (std::uint32_t(data[i + 1]) << 8) | data[i + 2];
        out += table[(v >> 18) & 0x3F]; out += table[(v >> 12) & 0x3F];
        out += table[(v >> 6) & 0x3F]; out += table[v & 0x3F];
    }
    const auto rem = data.size() - i;
    if (rem == 1) {
        const std::uint32_t v = std::uint32_t(data[i]) << 16;
        out += table[(v >> 18) & 0x3F]; out += table[(v >> 12) & 0x3F]; out += "==";
    } else if (rem == 2) {
        const std::uint32_t v = (std::uint32_t(data[i]) << 16) | (std::uint32_t(data[i + 1]) << 8);
        out += table[(v >> 18) & 0x3F]; out += table[(v >> 12) & 0x3F]; out += table[(v >> 6) & 0x3F]; out += '=';
    }
    return out;
}

std::vector<Byte> base64_decode(const std::string& text) {
    const auto value = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+') return 62;
        if (c == '/') return 63;
        return -1;
    };
    if (text.size() % 4) throw std::invalid_argument("checkpoint base64 needs canonical padding");
    std::vector<Byte> out;
    out.reserve(text.size() / 4 * 3);
    for (std::size_t i = 0; i < text.size(); i += 4) {
        const int a = value(text[i]), b = value(text[i + 1]);
        const bool pad2 = text[i + 2] == '=', pad3 = text[i + 3] == '=';
        const int c = pad2 ? 0 : value(text[i + 2]), d = pad3 ? 0 : value(text[i + 3]);
        if (a < 0 || b < 0 || c < 0 || d < 0 || (pad2 && !pad3) ||
            ((pad2 || pad3) && i + 4 != text.size()) ||
            (pad2 && (b & 15)) || (pad3 && !pad2 && (c & 3)))
            throw std::invalid_argument("checkpoint array is not canonical base64");
        const std::uint32_t buffer = (std::uint32_t(a) << 18) | (std::uint32_t(b) << 12) |
                                     (std::uint32_t(c) << 6) | std::uint32_t(d);
        out.push_back(Byte(buffer >> 16));
        if (!pad2) out.push_back(Byte(buffer >> 8));
        if (!pad3) out.push_back(Byte(buffer));
    }
    return out;
}

// ---- checkpoint JSON: schema "ftd-hydro-checkpoint-2", matching
// scripts/phi_v2_lattice/hydro/codec.py's checkpoint()/restore() exactly (field names,
// hex table/encoding hashes, base64 array order s/bank/sc/fcc/admitted_sc/admitted_fcc/
// gate_sc/gate_fcc). Parsing reuses the host's JSON.parse via `val` rather than a
// hand-rolled parser; serialization is hand-rolled (as events_json()/diagnostics() are).

void decode_bytes_into(const std::string& text, const char* name, std::vector<Byte>& target) {
    const auto raw = base64_decode(text);
    if (raw.size() != target.size())
        throw std::invalid_argument(std::string("checkpoint array size mismatch: ") + name);
    std::copy(raw.begin(), raw.end(), target.begin());
}

State parse_checkpoint(const std::string& text) {
    const val parsed = val::global("JSON").call<val>("parse", val(text));
    validate_json_tokens(text);
    object_keys(parsed, std::array<const char*, 8>{"schema", "law", "table", "encoding", "boundary", "L", "microtick", "arrays"});
    const auto schema = string_value(parsed["schema"]);
    if (schema != "ftd-hydro-checkpoint-1" && schema != "ftd-hydro-checkpoint-2")
        throw std::invalid_argument("checkpoint schema mismatch");
    if (string_value(parsed["law"]) != tables::LAW_ID)
        throw std::invalid_argument("checkpoint law mismatch");
    if (string_value(parsed["table"]) != table_hash_hex())
        throw std::invalid_argument("checkpoint table mismatch");
    if (string_value(parsed["encoding"]) != encoding_hash_hex() || string_value(parsed["boundary"]) != "periodic")
        throw std::invalid_argument("checkpoint encoding/boundary mismatch");
    const auto L = std::uint32_t(integer_number(parsed["L"], 3, std::numeric_limits<std::uint32_t>::max()));
    const auto microtick = schema == "ftd-hydro-checkpoint-2" ? decimal(string_value(parsed["microtick"]))
        : std::uint64_t(integer_number(parsed["microtick"], 0, 9007199254740991.0));
    const auto max = std::numeric_limits<std::size_t>::max();
    if (std::size_t(L) > max / L || std::size_t(L) * L > max / L)
        throw std::invalid_argument("checkpoint dimensions overflow");
    const auto n = std::size_t(L) * L * L;
    if (n > (max - ftd::hydro::HEADER_BYTES) / ftd::hydro::BYTES_PER_SITE)
        throw std::invalid_argument("checkpoint byte count overflow");
    const val arrays = parsed["arrays"];
    const std::array<const char*, 8> names{"s", "bank", "sc", "fcc", "admitted_sc", "admitted_fcc", "gate_sc", "gate_fcc"};
    const std::array<unsigned, 8> sizes{1, 192, 6, 12, 3, 6, 3, 6};
    object_keys(arrays, names);
    std::array<std::string, 8> encoded;
    for (unsigned i = 0; i < names.size(); ++i) {
        encoded[i] = string_value(arrays[names[i]]);
        const auto expected = ((std::uint64_t(n) * sizes[i] + 2) / 3) * 4;
        if (encoded[i].size() != expected)
            throw std::invalid_argument(std::string("checkpoint encoded array size mismatch: ") + names[i]);
    }
    State state(L);  // All dimensions and supplied array lengths checked before allocation.
    {
        const auto raw = base64_decode(encoded[0]);
        if (raw.size() != state.s.size()) throw std::invalid_argument("checkpoint array size mismatch: s");
        for (std::size_t i = 0; i < raw.size(); ++i)
            state.s[i] = std::int8_t(raw[i] < 128 ? int(raw[i]) : int(raw[i]) - 256);
    }
    decode_bytes_into(encoded[1], "bank", state.bank);
    decode_bytes_into(encoded[2], "sc", state.sc);
    decode_bytes_into(encoded[3], "fcc", state.fcc);
    decode_bytes_into(encoded[4], "admitted_sc", state.admitted_sc);
    decode_bytes_into(encoded[5], "admitted_fcc", state.admitted_fcc);
    decode_bytes_into(encoded[6], "gate_sc", state.gate_sc);
    decode_bytes_into(encoded[7], "gate_fcc", state.gate_fcc);
    state.microtick = microtick;
    ftd::hydro::validate(state);
    return state;
}

std::string build_checkpoint(const State& state) {
    ftd::hydro::validate(state);
    std::vector<Byte> s_bytes(state.s.size());
    for (std::size_t i = 0; i < state.s.size(); ++i) s_bytes[i] = Byte(state.s[i]);
    std::ostringstream out;
    out << "{\"schema\":\"ftd-hydro-checkpoint-2\",\"law\":\"" << tables::LAW_ID
        << "\",\"table\":\"" << table_hash_hex() << "\",\"encoding\":\"" << encoding_hash_hex()
        << "\",\"boundary\":\"periodic\",\"L\":" << state.L << ",\"microtick\":\"" << state.microtick
        << "\",\"arrays\":{"
        << "\"s\":\"" << base64_encode(s_bytes) << "\""
        << ",\"bank\":\"" << base64_encode(state.bank) << "\""
        << ",\"sc\":\"" << base64_encode(state.sc) << "\""
        << ",\"fcc\":\"" << base64_encode(state.fcc) << "\""
        << ",\"admitted_sc\":\"" << base64_encode(state.admitted_sc) << "\""
        << ",\"admitted_fcc\":\"" << base64_encode(state.admitted_fcc) << "\""
        << ",\"gate_sc\":\"" << base64_encode(state.gate_sc) << "\""
        << ",\"gate_fcc\":\"" << base64_encode(state.gate_fcc) << "\""
        << "}}";
    return out.str();
}

std::string diagnostics(const State& state) {
    std::ostringstream out;
    out << "{\"law\":\"" << tables::LAW_ID << "\",\"table_hash\":\"" << table_hash_hex()
        << "\",\"L\":\"" << state.L << "\",\"microtick\":\"" << state.microtick
        << "\",\"phase\":\"" << state.phase() << "\",\"work_units\":\"" << ftd::hydro::work_units(state) << "\"}";
    return out.str();
}

template <class Sequence>
void integer_array(std::ostream& out, const Sequence& values) {
    out << '[';
    bool first = true;
    for (auto value : values) {
        if (!first) out << ',';
        first = false;
        out << '"' << value << '"';
    }
    out << ']';
}

std::array<std::size_t, 3> coords_of(std::uint32_t L, std::size_t i) {
    const std::size_t Lsz = L;
    return {i / (Lsz * Lsz), (i / Lsz) % Lsz, i % Lsz};
}

std::size_t shift_axis(std::uint32_t L, std::size_t i, unsigned axis) {
    auto p = coords_of(L, i);
    p[axis] = (p[axis] + 1) % L;
    return (p[0] * L + p[1]) * L + p[2];
}

// Passive exact owner-based block observation, the same lab as ../wasm_bindings.cpp's
// `counts()` adapted to the hydro bank's 192 channels (hydro's sc/fcc relation records
// share the strict candidate's shape and A9 alphabet, so the incidence/relation-token
// bookkeeping carries over unchanged). Does not advance State.
std::string counts_lab(const State& state, std::uint32_t width) {
    if (!width || state.L % width) throw std::invalid_argument("width must divide lattice size");
    const std::size_t side = state.L / width;
    const std::size_t count = side * side * side;
    std::vector<std::int64_t> incidence(count, 0);
    std::vector<std::uint64_t> field(count, 0), relation(count, 0);
    std::vector<std::array<std::uint64_t, 3>> manifestation(count, {0, 0, 0});
    const auto block = [&](std::size_t i) {
        const auto p = coords_of(state.L, i);
        return ((p[0] / width) * side + p[1] / width) * side + p[2] / width;
    };
    const auto add = [&](std::size_t tail, std::size_t head, Byte primary) {
        const int eps = frozen::EPS[primary];
        incidence[block(tail)] += eps;
        incidence[block(head)] -= eps;
    };
    const auto n = state.s.size();
    for (std::size_t i = 0; i < n; ++i) {
        const auto b = block(i);
        ++manifestation[b][std::size_t(state.s[i] + 1)];
        for (unsigned c = 0; c < unsigned(tables::N_CHANNELS); ++c) field[b] += state.bank[i * tables::N_CHANNELS + c];
        for (unsigned axis = 0; axis < 3; ++axis) {
            const auto index = i * 6 + axis * 2;
            relation[b] += state.sc[index] != frozen::BLANK;
            relation[b] += state.sc[index + 1] != frozen::BLANK;
            add(i, shift_axis(state.L, i, axis), state.sc[index]);
        }
        for (unsigned plane = 0; plane < 3; ++plane) {
            unsigned axes[2], next = 0;
            for (unsigned axis = 0; axis < 3; ++axis) if (axis != plane) axes[next++] = axis;
            for (unsigned diagonal = 0; diagonal < 2; ++diagonal) {
                const auto index = i * 12 + plane * 4 + diagonal * 2;
                relation[b] += state.fcc[index] != frozen::BLANK;
                relation[b] += state.fcc[index + 1] != frozen::BLANK;
                const auto tail = diagonal ? shift_axis(state.L, i, axes[0]) : i;
                const auto head = diagonal ? shift_axis(state.L, i, axes[1])
                                            : shift_axis(state.L, shift_axis(state.L, i, axes[0]), axes[1]);
                add(tail, head, state.fcc[index]);
            }
        }
    }
    std::ostringstream out;
    out << "{\"law\":\"" << tables::LAW_ID << "\",\"table_hash\":\"" << table_hash_hex()
        << "\",\"L\":\"" << state.L << "\",\"width\":\"" << width << "\",\"microtick\":\"" << state.microtick
        << "\",\"phase\":\"" << state.phase() << "\",\"status\":\"exact_observation\",\"incidence\":";
    integer_array(out, incidence);
    out << ",\"field_tokens\":"; integer_array(out, field);
    out << ",\"relation_tokens\":"; integer_array(out, relation);
    out << ",\"manifestation_counts\":[";
    for (std::size_t b = 0; b < count; ++b) { if (b) out << ','; integer_array(out, manifestation[b]); }
    out << "]}";
    return out.str();
}

// Per-block field-token count and lattice-gas momentum (integer sum of the occupied
// channels' velocity vectors), split by polarity: {"field_tokens":[pol0,pol1],
// "momentum":[[px,py,pz] pol0,[px,py,pz] pol1]}.
std::string fields_lab(const State& state, std::uint32_t width) {
    if (!width || state.L % width) throw std::invalid_argument("width must divide lattice size");
    const std::size_t side = state.L / width;
    const std::size_t count = side * side * side;
    std::vector<std::array<std::uint64_t, 2>> field_tokens(count, {0, 0});
    std::vector<std::array<std::array<std::int64_t, 3>, 2>> momentum(
        count, std::array<std::array<std::int64_t, 3>, 2>{std::array<std::int64_t, 3>{0, 0, 0}, std::array<std::int64_t, 3>{0, 0, 0}});
    const auto block = [&](std::size_t i) {
        const auto p = coords_of(state.L, i);
        return ((p[0] / width) * side + p[1] / width) * side + p[2] / width;
    };
    for (std::size_t i = 0; i < state.s.size(); ++i) {
        const auto b = block(i);
        for (unsigned pol = 0; pol < 2; ++pol) {
            for (unsigned k = 0; k < 4; ++k) {
                for (unsigned v = 0; v < 24; ++v) {
                    if (!state.bank[i * tables::N_CHANNELS + pol * 96 + k * 24 + v]) continue;
                    field_tokens[b][pol] += 1;
                    for (unsigned d = 0; d < 3; ++d) momentum[b][pol][d] += tables::VELOCITY[v][d];
                }
            }
        }
    }
    std::ostringstream out;
    out << "{\"law\":\"" << tables::LAW_ID << "\",\"table_hash\":\"" << table_hash_hex()
        << "\",\"L\":\"" << state.L << "\",\"width\":\"" << width << "\",\"microtick\":\"" << state.microtick
        << "\",\"phase\":\"" << state.phase() << "\",\"status\":\"exact_observation\",\"blocks\":[";
    for (std::size_t b = 0; b < count; ++b) {
        if (b) out << ',';
        out << "{\"field_tokens\":[\"" << field_tokens[b][0] << "\",\"" << field_tokens[b][1] << "\"]"
            << ",\"momentum\":[[\"" << momentum[b][0][0] << "\",\"" << momentum[b][0][1] << "\",\"" << momentum[b][0][2] << "\"]"
            << ",[\"" << momentum[b][1][0] << "\",\"" << momentum[b][1][1] << "\",\"" << momentum[b][1][2] << "\"]]}";
    }
    out << "]}";
    return out.str();
}

// "moments:kx,ky,kz,pol": [[re,im] x4] for sum_x e^{-ik.x} (N_x, Px_x, Py_x, Pz_x) of the
// requested polarity, k = 2*pi*(kx,ky,kz)/L. Accumulated in double, printed to 17
// significant digits (round-trip precision for IEEE-754 double); not a block observation.
std::string moments_lab(const State& state, const std::string& spec) {
    std::size_t pos = 0;
    long long parts[4];
    for (int idx = 0; idx < 4; ++idx) {
        const auto comma = idx < 3 ? spec.find(',', pos) : std::string::npos;
        if (idx < 3 && comma == std::string::npos)
            throw std::invalid_argument("moments observable needs kx,ky,kz,pol");
        parts[idx] = signed_integer(idx < 3 ? spec.substr(pos, comma - pos) : spec.substr(pos));
        pos = comma + 1;
    }
    const auto kx = parts[0], ky = parts[1], kz = parts[2];
    if (parts[3] != 0 && parts[3] != 1) throw std::invalid_argument("moments polarity must be 0 or 1");
    const unsigned pol = unsigned(parts[3]);
    // Periodic aliases must reach identical floating arithmetic even for int64
    // requests. Choose the same representative in (-L/2, L/2] using integers.
    const auto reduced = [&](long long k) {
        const auto L = static_cast<long long>(state.L);
        auto r = k % L;
        if (r < 0) r += L;
        if (r > L / 2) r -= L;
        return r;
    };
    const auto phase_kx = reduced(kx), phase_ky = reduced(ky), phase_kz = reduced(kz);
    const double two_pi = 2.0 * std::acos(-1.0);
    const double factor = two_pi / double(state.L);
    double sum_re[4] = {0, 0, 0, 0}, sum_im[4] = {0, 0, 0, 0};
    for (std::size_t i = 0; i < state.s.size(); ++i) {
        const auto p = coords_of(state.L, i);
        std::int64_t counts[4] = {0, 0, 0, 0};  // N, Px, Py, Pz
        for (unsigned k = 0; k < 4; ++k) {
            for (unsigned v = 0; v < 24; ++v) {
                if (!state.bank[i * tables::N_CHANNELS + pol * 96 + k * 24 + v]) continue;
                counts[0] += 1;
                counts[1] += tables::VELOCITY[v][0];
                counts[2] += tables::VELOCITY[v][1];
                counts[3] += tables::VELOCITY[v][2];
            }
        }
        const double phase = factor * (double(phase_kx) * double(p[0]) + double(phase_ky) * double(p[1]) + double(phase_kz) * double(p[2]));
        const double c = std::cos(phase), s = std::sin(phase);  // e^{-ik.x} = cos(phase) - i*sin(phase)
        for (int idx = 0; idx < 4; ++idx) {
            sum_re[idx] += double(counts[idx]) * c;
            sum_im[idx] += -double(counts[idx]) * s;
        }
    }
    std::ostringstream out;
    out << std::setprecision(17);
    out << "{\"law\":\"" << tables::LAW_ID << "\",\"table_hash\":\"" << table_hash_hex()
        << "\",\"L\":\"" << state.L << "\",\"microtick\":\"" << state.microtick << "\",\"phase\":\"" << state.phase()
        << "\",\"status\":\"approximate_observation\",\"arithmetic\":\"ieee754_binary64\",\"error_bound_status\":\"not_certified\""
        << ",\"k\":[\"" << kx << "\",\"" << ky << "\",\"" << kz << "\"],\"polarity\":\"" << pol << '"'
        << ",\"moments\":[";
    for (int idx = 0; idx < 4; ++idx) {
        if (idx) out << ',';
        out << '[' << sum_re[idx] << ',' << sum_im[idx] << ']';
    }
    out << "]}";
    return out.str();
}

std::uint32_t width_of(const std::string& text, std::uint32_t L) {
    const auto parsed = decimal(text);
    if (parsed == 0 || parsed > L) throw std::invalid_argument("width must be positive and at most the lattice size");
    return std::uint32_t(parsed);
}

std::string observe(const State& state, const std::string& width, const std::string& observable) {
    if (observable == "counts") return counts_lab(state, width_of(width, state.L));
    if (observable == "fields") return fields_lab(state, width_of(width, state.L));
    if (observable.size() > 8 && observable.compare(0, 8, "moments:") == 0)
        return moments_lab(state, observable.substr(8));
    throw std::invalid_argument("unsupported hydro observable");
}

// Forces the "table not loaded" error at construction time (before parsing a checkpoint
// that would otherwise appear to succeed) so a HydroState can never exist unusable.
State construct_state(const std::string& checkpointJson) {
    (void)ftd::hydro::table();
    return parse_checkpoint(checkpointJson);
}

class HydroState {
    State state_;
public:
    explicit HydroState(const val& checkpointJson) : state_(construct_state(string_value(checkpointJson))) {}
    std::string checkpoint() const { return build_checkpoint(state_); }
    void restore(const val& checkpointJson) { state_ = construct_state(string_value(checkpointJson)); }
    std::string advance(const val& ticks) {
        State candidate = state_;
        const auto events = ftd::hydro::advance(candidate, decimal(string_value(ticks)));
        const std::string result = "{\"diagnostics\":" + ::diagnostics(candidate)
            + ",\"events\":" + ftd::hydro::events_json(events) + "}";
        state_ = std::move(candidate);
        return result;
    }
    std::string diagnostics() const { return ::diagnostics(state_); }
    std::string observe(const val& width, const val& observable) const {
        return ::observe(state_, string_value(width), string_value(observable));
    }
};

void loadTable(const val& input) {
    ftd::hydro::load_table(bytes(input));
}

}  // namespace

EMSCRIPTEN_BINDINGS(ftd_hydro_candidate) {
    emscripten::function("loadTable", &loadTable);
    emscripten::class_<HydroState>("HydroState")
        .constructor<const val&>()
        .function("advance", &HydroState::advance)
        .function("checkpoint", &HydroState::checkpoint)
        .function("restore", &HydroState::restore)
        .function("diagnostics", &HydroState::diagnostics)
        .function("observe", &HydroState::observe);
}
