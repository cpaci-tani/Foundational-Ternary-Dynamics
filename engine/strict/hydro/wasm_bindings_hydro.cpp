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
    if (i >= text.size() || (text.size() - i > 1 && text[i] == '0'))
        throw std::invalid_argument("invalid integer literal in moments observable");
    long long value = 0;
    for (; i < text.size(); ++i) {
        const char c = text[i];
        if (c < '0' || c > '9') throw std::invalid_argument("invalid integer literal in moments observable");
        value = value * 10 + (c - '0');
    }
    return neg ? -value : value;
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
    std::vector<Byte> out;
    out.reserve(text.size() / 4 * 3);
    int buffer = 0, bits = 0;
    for (char c : text) {
        if (c == '=' || c == '\n' || c == '\r') continue;
        const int v = value(c);
        if (v < 0) throw std::invalid_argument("checkpoint array is not valid base64");
        buffer = (buffer << 6) | v;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(Byte((buffer >> bits) & 0xFF));
        }
    }
    return out;
}

// ---- checkpoint JSON: schema "ftd-hydro-checkpoint-1", matching
// scripts/phi_v2_lattice/hydro/codec.py's checkpoint()/restore() exactly (field names,
// hex table/encoding hashes, base64 array order s/bank/sc/fcc/admitted_sc/admitted_fcc/
// gate_sc/gate_fcc). Parsing reuses the host's JSON.parse via `val` rather than a
// hand-rolled parser; serialization is hand-rolled (as events_json()/diagnostics() are).

void decode_bytes_into(const val& arrays, const char* name, std::vector<Byte>& target) {
    const auto raw = base64_decode(arrays[name].as<std::string>());
    if (raw.size() != target.size())
        throw std::invalid_argument(std::string("checkpoint array size mismatch: ") + name);
    std::copy(raw.begin(), raw.end(), target.begin());
}

State parse_checkpoint(const std::string& text) {
    const val parsed = val::global("JSON").call<val>("parse", val(text));
    if (parsed.isNull() || parsed.isUndefined() || parsed["arrays"].isUndefined())
        throw std::invalid_argument("checkpoint must be a JSON object with an arrays field");
    if (parsed["schema"].as<std::string>() != "ftd-hydro-checkpoint-1")
        throw std::invalid_argument("checkpoint schema mismatch");
    if (parsed["law"].as<std::string>() != tables::LAW_ID)
        throw std::invalid_argument("checkpoint law mismatch");
    if (parsed["table"].as<std::string>() != table_hash_hex())
        throw std::invalid_argument("checkpoint table mismatch");
    const auto L = std::uint32_t(parsed["L"].as<double>());
    // microtick is a plain JSON number in the oracle's schema (matching Python's int),
    // so like Python/JSON.parse it is exact only up to 2^53; fine for realistic runs.
    const auto microtick = std::uint64_t(parsed["microtick"].as<double>());
    State state(L);
    const val arrays = parsed["arrays"];
    {
        const auto raw = base64_decode(arrays["s"].as<std::string>());
        if (raw.size() != state.s.size()) throw std::invalid_argument("checkpoint array size mismatch: s");
        for (std::size_t i = 0; i < raw.size(); ++i)
            state.s[i] = std::int8_t(raw[i] < 128 ? int(raw[i]) : int(raw[i]) - 256);
    }
    decode_bytes_into(arrays, "bank", state.bank);
    decode_bytes_into(arrays, "sc", state.sc);
    decode_bytes_into(arrays, "fcc", state.fcc);
    decode_bytes_into(arrays, "admitted_sc", state.admitted_sc);
    decode_bytes_into(arrays, "admitted_fcc", state.admitted_fcc);
    decode_bytes_into(arrays, "gate_sc", state.gate_sc);
    decode_bytes_into(arrays, "gate_fcc", state.gate_fcc);
    state.microtick = microtick;
    ftd::hydro::validate(state);
    return state;
}

std::string build_checkpoint(const State& state) {
    ftd::hydro::validate(state);
    std::vector<Byte> s_bytes(state.s.size());
    for (std::size_t i = 0; i < state.s.size(); ++i) s_bytes[i] = Byte(state.s[i]);
    std::ostringstream out;
    out << "{\"schema\":\"ftd-hydro-checkpoint-1\",\"law\":\"" << tables::LAW_ID
        << "\",\"table\":\"" << table_hash_hex() << "\",\"encoding\":\"" << encoding_hash_hex()
        << "\",\"boundary\":\"periodic\",\"L\":" << state.L << ",\"microtick\":" << state.microtick
        << ",\"arrays\":{"
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
        const double phase = factor * (double(kx) * double(p[0]) + double(ky) * double(p[1]) + double(kz) * double(p[2]));
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
        << "\",\"status\":\"exact_observation\",\"k\":[" << kx << ',' << ky << ',' << kz << "],\"polarity\":" << pol
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
    explicit HydroState(const std::string& checkpointJson) : state_(construct_state(checkpointJson)) {}
    std::string checkpoint() const { return build_checkpoint(state_); }
    void restore(const std::string& checkpointJson) { state_ = construct_state(checkpointJson); }
    std::string advance(const std::string& ticks) {
        State candidate = state_;
        const auto events = ftd::hydro::advance(candidate, decimal(ticks));
        const std::string result = "{\"diagnostics\":" + ::diagnostics(candidate)
            + ",\"events\":" + ftd::hydro::events_json(events) + "}";
        state_ = std::move(candidate);
        return result;
    }
    std::string diagnostics() const { return ::diagnostics(state_); }
    std::string observe(const std::string& width, const std::string& observable) const {
        return ::observe(state_, width, observable);
    }
};

void loadTable(const val& input) {
    ftd::hydro::load_table(bytes(input));
}

}  // namespace

EMSCRIPTEN_BINDINGS(ftd_hydro_candidate) {
    emscripten::function("loadTable", &loadTable);
    emscripten::class_<HydroState>("HydroState")
        .constructor<const std::string&>()
        .function("advance", &HydroState::advance)
        .function("checkpoint", &HydroState::checkpoint)
        .function("restore", &HydroState::restore)
        .function("diagnostics", &HydroState::diagnostics)
        .function("observe", &HydroState::observe);
}
