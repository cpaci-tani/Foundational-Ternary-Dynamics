// Isolated Embind adapter of the same finite kernel/codec as the native CLI.
#include "staged_runtime.h"
#include "frozen_tables.h"
#include <emscripten/bind.h>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {
using ftd::strict::State;
using emscripten::val;

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

std::vector<std::uint8_t> bytes(const val& input) {
    if (!input.instanceof(val::global("Uint8Array")))
        throw std::invalid_argument("checkpoint must be Uint8Array");
    // The intrinsic ArrayBuffer getter performs a realm-independent brand
    // check. Unlike instanceof/toStringTag it rejects foreign or disguised SAB.
    const auto descriptor = val::global("Object").call<val>("getOwnPropertyDescriptor",
        val::global("ArrayBuffer")["prototype"], val("byteLength"));
    const auto typedPrototype = val::global("Object").call<val>("getPrototypeOf", val::global("Uint8Array")["prototype"]);
    const auto bufferDescriptor = val::global("Object").call<val>("getOwnPropertyDescriptor", typedPrototype, val("buffer"));
    const auto buffer = bufferDescriptor["get"].call<val>("call", input);
    descriptor["get"].call<val>("call", buffer);
    return emscripten::convertJSArrayToNumberVector<std::uint8_t>(input);
}

std::string diagnostics(const State& state) {
    std::ostringstream out;
    out << "{\"law_id\":\"" << ftd::strict::LAW_ID << "\",\"microtick\":\""
        << state.microtick << "\",\"phase\":\"" << state.phase()
        << "\",\"lattice_size\":\"" << state.L << "\",\"token_count\":\""
        << ftd::strict::work_units(state) << "\",\"physical_calibration\":\"unidentified\"}";
    return out.str();
}

template<class Sequence>
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

// Passive exact owner-based block observation; this does not advance State.
std::string counts(const State& state, std::uint32_t width) {
    using namespace ftd::strict;
    if (!width || state.L % width) throw std::invalid_argument("width must divide lattice size");
    const std::size_t side = state.L / width;
    const std::size_t count = side * side * side;
    std::vector<std::int64_t> incidence(count, 0);
    std::vector<std::uint64_t> field(count, 0), relation(count, 0);
    std::vector<std::array<std::uint64_t, 3>> manifestation(count, {0, 0, 0});
    const auto coords = [&](std::size_t i) {
        const std::size_t L = state.L;
        return std::array<std::size_t, 3>{i / (L * L), (i / L) % L, i % L};
    };
    const auto block = [&](std::size_t i) {
        const auto p = coords(i);
        return ((p[0] / width) * side + p[1] / width) * side + p[2] / width;
    };
    const auto shift = [&](std::size_t i, unsigned axis) {
        auto p = coords(i);
        p[axis] = (p[axis] + 1) % state.L;
        return (p[0] * state.L + p[1]) * state.L + p[2];
    };
    const auto add = [&](std::size_t tail, std::size_t head, std::uint8_t primary) {
        const int eps = frozen::EPS[primary];
        incidence[block(tail)] += eps;
        incidence[block(head)] -= eps;
    };
    for (std::size_t i = 0; i < state.s.size(); ++i) {
        const auto b = block(i);
        ++manifestation[b][state.s[i] + 1];
        for (unsigned c = 0; c < 384; ++c) field[b] += state.bank[i * 384 + c];
        for (unsigned axis = 0; axis < 3; ++axis) {
            const auto index = i * 6 + axis * 2;
            relation[b] += state.sc[index] != frozen::BLANK;
            relation[b] += state.sc[index + 1] != frozen::BLANK;
            add(i, shift(i, axis), state.sc[index]);
        }
        for (unsigned plane = 0; plane < 3; ++plane) {
            unsigned axes[2], next = 0;
            for (unsigned axis = 0; axis < 3; ++axis) if (axis != plane) axes[next++] = axis;
            for (unsigned diagonal = 0; diagonal < 2; ++diagonal) {
                const auto index = i * 12 + plane * 4 + diagonal * 2;
                relation[b] += state.fcc[index] != frozen::BLANK;
                relation[b] += state.fcc[index + 1] != frozen::BLANK;
                const auto tail = diagonal ? shift(i, axes[0]) : i;
                const auto head = diagonal ? shift(i, axes[1]) : shift(shift(i, axes[0]), axes[1]);
                add(tail, head, state.fcc[index]);
            }
        }
    }
    std::ostringstream out;
    std::string collision_hash;
    constexpr char hex[] = "0123456789ABCDEF";
    for (const auto byte : frozen::COLLISION_HASH) {
        collision_hash += hex[byte >> 4];
        collision_hash += hex[byte & 15];
    }
    out << "{\"law_id\":\"" << LAW_ID << "\",\"collision_hash\":\"" << collision_hash
        << "\",\"tick_start\":\"" << state.microtick << "\",\"tick_end\":\"" << state.microtick
        << "\",\"spatial_support\":\"aligned_periodic_blocks\",\"length_unit\":\"microscopic_node\""
        << ",\"time_unit\":\"staged_microtick\",\"observer_kind\":\"external_diagnostic\""
        << ",\"physical_calibration\":\"unidentified\",\"lattice_size\":\"" << state.L << "\",\"width\":\"" << width
        << "\",\"microtick\":\"" << state.microtick << "\",\"phase\":\"" << state.phase()
        << "\",\"status\":\"exact_observation\",\"incidence\":";
    integer_array(out, incidence);
    out << ",\"field_tokens\":"; integer_array(out, field);
    out << ",\"relation_tokens\":"; integer_array(out, relation);
    out << ",\"manifestation_counts\":[";
    for (std::size_t b = 0; b < count; ++b) {
        if (b) out << ',';
        integer_array(out, manifestation[b]);
    }
    out << "]}";
    return out.str();
}

class StrictState {
    State state_;
public:
    explicit StrictState(const val& checkpoint) : state_(ftd::strict::decode(bytes(checkpoint))) {}
    val checkpoint() const {
        const auto encoded = ftd::strict::encode(state_);
        // Constructing Uint8Array from a typed view copies before encoded expires.
        return val::global("Uint8Array").new_(val(emscripten::typed_memory_view(encoded.size(), encoded.data())));
    }
    void restore(const val& checkpoint) {
        auto candidate = ftd::strict::decode(bytes(checkpoint));
        state_ = std::move(candidate);
    }
    std::string advance(const std::string& ticks) {
        State candidate = state_;
        const auto events = ftd::strict::advance(candidate, decimal(ticks));
        const std::string result = "{\"diagnostics\":" + ::diagnostics(candidate)
            + ",\"events\":" + ftd::strict::events_json(events) + "}";
        state_ = std::move(candidate);
        return result;
    }
    std::string diagnostics() const { return ::diagnostics(state_); }
    std::string observe(const std::string& width, const std::string& observable) const {
        if (observable != "counts") throw std::invalid_argument("unsupported strict observable");
        const auto parsed = decimal(width);
        if (parsed > state_.L) throw std::invalid_argument("width exceeds lattice size");
        return counts(state_, static_cast<std::uint32_t>(parsed));
    }
};
}  // namespace

EMSCRIPTEN_BINDINGS(ftd_strict_candidate) {
    emscripten::class_<StrictState>("StrictState")
        .constructor<const val&>()
        .function("advance", &StrictState::advance)
        .function("checkpoint", &StrictState::checkpoint)
        .function("restore", &StrictState::restore)
        .function("diagnostics", &StrictState::diagnostics)
        .function("observe", &StrictState::observe);
}
