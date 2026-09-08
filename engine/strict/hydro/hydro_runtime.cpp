#include "hydro_runtime.h"
#include "hydro_tables.h"
#include "hydro_sha256.h"
#include "../frozen_tables.h"  // A9 relation alphabet: ROTATE, PHASE, EPS, BLANK, ABSORBED_RESERVE
#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace ftd::hydro {
namespace frozen = ::ftd::strict::frozen;
namespace {
using Byte = std::uint8_t;

std::size_t sites(std::uint32_t L) {
    static_assert(std::numeric_limits<std::size_t>::digits <= 64, "native transport supports at most 64-bit address spaces");
    const auto max = std::numeric_limits<std::size_t>::max();
    if (L < 3 || std::size_t(L) > max / L || std::size_t(L) * L > max / L)
        throw std::invalid_argument("invalid hydro lattice dimensions");
    const auto n = std::size_t(L) * L * L;
    if (n > (max - HEADER_BYTES) / BYTES_PER_SITE)
        throw std::invalid_argument("hydro lattice byte/count overflow");
    return n;
}

template <class T>
void array_check(const std::vector<T>& v, std::size_t size, int lo, int hi) {
    if (v.size() != size || std::any_of(v.begin(), v.end(), [=](T x) { return int(x) < lo || int(x) > hi; }))
        throw std::invalid_argument("hydro array shape/alphabet mismatch");
}

// Per-axis unit shift (periodic wrap), used by the relation geometry (gates/endpoints).
std::size_t shift(std::uint32_t L, std::size_t site, unsigned axis, int sign) {
    std::size_t xyz[3] = {site / (std::size_t(L) * L), (site / L) % L, site % L};
    auto& x = xyz[axis];
    x = sign > 0 ? (x + 1 == L ? 0 : x + 1) : (x == 0 ? L - 1 : x - 1);
    return (xyz[0] * L + xyz[1]) * L + xyz[2];
}

// General signed-offset shift (periodic wrap), used by the velocity/target tables.
std::size_t shift(std::uint32_t L, std::size_t site, const std::int8_t offset[3]) {
    std::size_t xyz[3] = {site / (std::size_t(L) * L), (site / L) % L, site % L};
    for (unsigned k = 0; k < 3; ++k) {
        int v = int(xyz[k]) + int(offset[k]);
        v %= int(L);
        if (v < 0) v += int(L);
        xyz[k] = std::size_t(v);
    }
    return (xyz[0] * L + xyz[1]) * L + xyz[2];
}

std::pair<std::size_t, std::size_t> endpoints(std::uint32_t L, std::size_t owner, unsigned p, unsigned q) {
    unsigned a = 0, b = 0;
    for (unsigned k = 0, count = 0; k < 3; ++k) if (k != p) { (count++ == 0 ? a : b) = k; }
    if (q == 0) return {owner, shift(L, shift(L, owner, a, 1), b, 1)};
    return {shift(L, owner, a, 1), shift(L, owner, b, 1)};
}

// Relation crossing (spec's relation_tick, folded to the frozen A9 rotate/phase tables):
// pol/fcc slot pair rotates every tick; a lone phase-0 token swaps sides under an even gate.
void relation(State& out, const State& in, Events& events, bool fcc, std::size_t i, unsigned a, unsigned q) {
    const auto key = fcc ? (i * 3 + a) * 2 + q : i * 3 + a;
    if (fcc ? (in.admitted_fcc[key] != 0) : (in.admitted_sc[key] != 0)) return;
    const auto& source = fcc ? in.fcc : in.sc;
    auto& target = fcc ? out.fcc : out.sc;
    const bool even = (fcc ? in.gate_fcc : in.gate_sc)[key] != 0;
    const Byte l = source[key * 2], r = source[key * 2 + 1];
    const bool lo = l != frozen::BLANK, ro = r != frozen::BLANK;
    const bool phase_zero = lo != ro && frozen::PHASE[lo ? l : r] == 0;
    const bool cross = phase_zero && even;
    target[key * 2] = frozen::ROTATE[cross ? r : l];
    target[key * 2 + 1] = frozen::ROTATE[cross ? l : r];
    if (cross) events.crossings.push_back({fcc, i, a, q, lo ? 1 : -1});
    if (phase_zero && !even) events.gate_holds.push_back({fcc, i, a, q, 0});
}

void append_u64(std::vector<Byte>& data, std::uint64_t value, unsigned bytes) {
    for (unsigned i = 0; i < bytes; ++i) data.push_back(Byte(value >> (8 * i)));
}
std::uint64_t read_u64(const std::vector<Byte>& data, std::size_t offset, unsigned bytes) {
    std::uint64_t value = 0;
    for (unsigned i = 0; i < bytes; ++i) value |= std::uint64_t(data[offset + i]) << (8 * i);
    return value;
}

void write_relation(std::ostream& out, const RelationEvent& r, bool with_direction) {
    out << "[\"" << (r.fcc ? "fcc" : "sc") << "\"," << r.owner << ",[" << r.axis;
    if (r.fcc) out << ',' << r.diagonal;
    out << ']';
    if (with_direction) out << ',' << r.direction;
    out << ']';
}
template <class T, class Writer>
void write_array(std::ostream& out, const std::vector<T>& values, Writer writer) {
    out << '[';
    bool first = true;
    for (const auto& value : values) { if (!first) out << ','; first = false; writer(out, value); }
    out << ']';
}

std::vector<std::uint32_t>& table_storage() {
    static std::vector<std::uint32_t> storage;
    return storage;
}
bool& table_loaded_flag() {
    static bool loaded = false;
    return loaded;
}

struct StepResult { State state; Events events; };

StepResult step_impl(const State& in) {
    validate(in);
    if (in.microtick == std::numeric_limits<std::uint64_t>::max())
        throw std::overflow_error("hydro physical tick counter overflow");
    StepResult result{in, {}};
    auto& out = result.state;
    auto& events = result.events;
    ++out.microtick;
    const auto n = sites(in.L);
    if (in.phase() == 0) {
        std::vector<unsigned> count(n, 0);
        for (std::size_t i = 0; i < n; ++i)
            for (unsigned c = 0; c < 192; ++c) count[i] += in.bank[i * 192 + c];
        for (std::size_t i = 0; i < n; ++i) {
            for (unsigned a = 0; a < 3; ++a) {
                const auto head = shift(in.L, i, a, 1);
                out.gate_sc[i * 3 + a] = Byte((count[i] + count[head]) % 2 == 0);
            }
            for (unsigned p = 0; p < 3; ++p)
                for (unsigned q = 0; q < 2; ++q) {
                    const auto ends = endpoints(in.L, i, p, q);
                    out.gate_fcc[(i * 3 + p) * 2 + q] = Byte((count[ends.first] + count[ends.second]) % 2 == 0);
                }
        }
        // Admission: proposals keyed by target relation; both polarities compete.
        struct Proposal { unsigned count = 0; std::size_t x = 0; unsigned c = 0; };
        std::vector<Proposal> sc_prop(3 * n), fcc_prop(6 * n);
        for (std::size_t x = 0; x < n; ++x)
            for (unsigned c = 0; c < 192; ++c) {
                if (!in.bank[x * 192 + c]) continue;
                const unsigned k = (c % 96) / 24, v = c % 24;
                if (k != 2) continue;
                const auto owner = shift(in.L, x, tables::TARGET_OFFSET[v]);
                Proposal& p = tables::TARGET_KIND[v] == 0
                    ? sc_prop[owner * 3 + std::size_t(tables::TARGET_INDEX[v][0])]
                    : fcc_prop[(owner * 3 + std::size_t(tables::TARGET_INDEX[v][0])) * 2 + std::size_t(tables::TARGET_INDEX[v][1])];
                if (++p.count == 1) { p.x = x; p.c = c; }
            }
        struct Admit { std::size_t x; unsigned c; bool fcc; std::size_t owner; unsigned idx0, idx1; };
        std::vector<Admit> admits;
        for (std::size_t owner = 0; owner < n; ++owner) {
            for (unsigned a = 0; a < 3; ++a) {
                const auto& p = sc_prop[owner * 3 + a];
                if (p.count != 1) continue;
                const auto key = owner * 3 + a;
                if (in.sc[key * 2] != frozen::BLANK || in.sc[key * 2 + 1] != frozen::BLANK) continue;
                admits.push_back({p.x, p.c, false, owner, a, 0});
            }
            for (unsigned pidx = 0; pidx < 3; ++pidx)
                for (unsigned q = 0; q < 2; ++q) {
                    const auto& p = fcc_prop[(owner * 3 + pidx) * 2 + q];
                    if (p.count != 1) continue;
                    const auto key = (owner * 3 + pidx) * 2 + q;
                    if (in.fcc[key * 2] != frozen::BLANK || in.fcc[key * 2 + 1] != frozen::BLANK) continue;
                    admits.push_back({p.x, p.c, true, owner, pidx, q});
                }
        }
        // Events are appended in ascending (x, c) order of the winning particle (Python's
        // np.nonzero order), independent of the owner-major scan above.
        std::sort(admits.begin(), admits.end(), [](const Admit& a, const Admit& b) {
            return a.x != b.x ? a.x < b.x : a.c < b.c;
        });
        for (const auto& a : admits) {
            const unsigned pol = a.c / 96;
            out.bank[a.x * 192 + a.c] = 0;
            const auto reserve = frozen::ABSORBED_RESERVE[pol];
            if (!a.fcc) {
                const auto key = a.owner * 3 + a.idx0;
                out.sc[key * 2] = frozen::BLANK;
                out.sc[key * 2 + 1] = reserve;
                out.admitted_sc[key] = 1;
            } else {
                const auto key = (a.owner * 3 + a.idx0) * 2 + a.idx1;
                out.fcc[key * 2] = frozen::BLANK;
                out.fcc[key * 2 + 1] = reserve;
                out.admitted_fcc[key] = 1;
            }
            events.absorptions.push_back({a.x, a.c, a.fcc, a.owner, a.idx0, a.idx1});
        }
    } else if (in.phase() == 1) {
        const auto& coll = table();
        for (std::size_t x = 0; x < n; ++x) {
            std::fill(out.bank.begin() + std::ptrdiff_t(x * 192), out.bank.begin() + std::ptrdiff_t(x * 192 + 192), Byte(0));
            for (unsigned pol = 0; pol < 2; ++pol) {
                std::uint32_t mask = 0;
                unsigned labels[24], nl = 0;
                for (unsigned v = 0; v < 24; ++v)
                    for (unsigned k = 0; k < 4; ++k)
                        if (in.bank[x * 192 + pol * 96 + k * 24 + v]) { mask |= 1u << v; labels[nl++] = k; }
                if (!mask) continue;
                const std::uint32_t new_mask = coll[mask];
                std::sort(labels, labels + nl);
                unsigned j = 0;
                for (unsigned v = 0; v < 24; ++v)
                    if (new_mask >> v & 1u) out.bank[x * 192 + pol * 96 + labels[j++] * 24 + v] = 1;
                if (new_mask != mask) events.collisions.push_back({x, pol == 0 ? 1 : -1, mask, new_mask});
            }
            for (unsigned a = 0; a < 3; ++a) relation(out, in, events, false, x, a, 0);
            for (unsigned p = 0; p < 3; ++p) for (unsigned q = 0; q < 2; ++q) relation(out, in, events, true, x, p, q);
        }
    } else if (in.phase() == 2) {
        std::fill(out.bank.begin(), out.bank.end(), Byte(0));
        for (std::size_t x = 0; x < n; ++x)
            for (unsigned c = 0; c < 192; ++c) {
                if (!in.bank[x * 192 + c]) continue;
                const unsigned pol = c / 96, k = (c % 96) / 24, v = c % 24;
                const auto y = shift(in.L, x, tables::VELOCITY[v]);
                unsigned k2 = (k + 1) % 4;
                if (in.s[x] != 0) k2 = (k2 + 2) % 4;
                Byte& slot = out.bank[y * 192 + pol * 96 + k2 * 24 + v];
                if (slot) throw std::logic_error("hydro streaming write collision");
                slot = 1;
            }
    } else {
        std::vector<int> incidence(n, 0);
        for (std::size_t i = 0; i < n; ++i) {
            for (unsigned a = 0; a < 3; ++a) {
                const int eps = frozen::EPS[in.sc[(i * 3 + a) * 2]];
                incidence[i] += eps; incidence[shift(in.L, i, a, 1)] -= eps;
            }
            for (unsigned p = 0; p < 3; ++p)
                for (unsigned q = 0; q < 2; ++q) {
                    const auto ends = endpoints(in.L, i, p, q);
                    const int eps = frozen::EPS[in.fcc[((i * 3 + p) * 2 + q) * 2]];
                    incidence[ends.first] += eps; incidence[ends.second] -= eps;
                }
        }
        for (std::size_t i = 0; i < n; ++i) out.s[i] = std::int8_t(((incidence[i] + 1) % 3 + 3) % 3 - 1);
        std::fill(out.admitted_sc.begin(), out.admitted_sc.end(), Byte(0));
        std::fill(out.admitted_fcc.begin(), out.admitted_fcc.end(), Byte(0));
        std::fill(out.gate_sc.begin(), out.gate_sc.end(), Byte(0));
        std::fill(out.gate_fcc.begin(), out.gate_fcc.end(), Byte(0));
    }
    validate(out);
    return result;
}

}  // namespace

State::State(std::uint32_t size) : L(size) {
    if (size == 0) return;
    const auto n = sites(size);
    s.resize(n);
    bank.resize(n * 192);
    sc.assign(n * 6, frozen::BLANK);
    fcc.assign(n * 12, frozen::BLANK);
    admitted_sc.resize(n * 3);
    admitted_fcc.resize(n * 6);
    gate_sc.resize(n * 3);
    gate_fcc.resize(n * 6);
}

void validate(const State& st) {
    const auto n = sites(st.L);
    array_check(st.s, n, -1, 1);
    array_check(st.bank, n * 192, 0, 1);
    array_check(st.sc, n * 6, 0, 8);
    array_check(st.fcc, n * 12, 0, 8);
    array_check(st.admitted_sc, n * 3, 0, 1);
    array_check(st.admitted_fcc, n * 6, 0, 1);
    array_check(st.gate_sc, n * 3, 0, 1);
    array_check(st.gate_fcc, n * 6, 0, 1);
    for (std::size_t x = 0; x < n; ++x)
        for (unsigned pol = 0; pol < 2; ++pol)
            for (unsigned v = 0; v < 24; ++v) {
                unsigned count = 0;
                for (unsigned k = 0; k < 4; ++k) count += st.bank[x * 192 + pol * 96 + k * 24 + v];
                if (count > 1) throw std::invalid_argument("hydro velocity exclusion violated");
            }
    if (st.phase() == 0) {
        for (const auto* bits : {&st.admitted_sc, &st.admitted_fcc, &st.gate_sc, &st.gate_fcc})
            if (std::any_of(bits->begin(), bits->end(), [](Byte b) { return b != 0; }))
                throw std::invalid_argument("uncleared phase-zero pending state");
    }
}

std::uint64_t work_units(const State& st) {
    validate(st);
    std::uint64_t count = 0;
    for (auto b : st.bank) count += b;
    for (auto b : st.sc) count += b != frozen::BLANK;
    for (auto b : st.fcc) count += b != frozen::BLANK;
    return count;
}

Events step(State& state) {
    auto result = step_impl(state);
    state = std::move(result.state);
    return std::move(result.events);
}

Events advance(State& state, std::uint64_t microticks) {
    validate(state);
    if (microticks > std::numeric_limits<std::uint64_t>::max() - state.microtick)
        throw std::overflow_error("hydro batch tick counter overflow");
    State candidate = state;
    Events combined;
    for (std::uint64_t i = 0; i < microticks; ++i) {
        auto result = step_impl(candidate);
        candidate = std::move(result.state);
        auto& ev = result.events;
        combined.absorptions.insert(combined.absorptions.end(), ev.absorptions.begin(), ev.absorptions.end());
        combined.collisions.insert(combined.collisions.end(), ev.collisions.begin(), ev.collisions.end());
        combined.crossings.insert(combined.crossings.end(), ev.crossings.begin(), ev.crossings.end());
        combined.gate_holds.insert(combined.gate_holds.end(), ev.gate_holds.begin(), ev.gate_holds.end());
    }
    state = std::move(candidate);
    return combined;
}

std::vector<Byte> encode(const State& state) {
    validate(state);
    std::vector<Byte> bytes = {'F', 'T', 'D', 'H', 'Y', '0', '1', 0};
    bytes.reserve(HEADER_BYTES + sites(state.L) * BYTES_PER_SITE);
    append_u64(bytes, state.L, 4);
    append_u64(bytes, state.microtick, 8);
    for (const auto* hash : {tables::TABLE_HASH, tables::ENCODING_HASH, tables::LAW_HASH})
        bytes.insert(bytes.end(), hash, hash + 32);
    for (auto value : state.s) bytes.push_back(Byte(value));
    for (const auto* array : {&state.bank, &state.sc, &state.fcc, &state.admitted_sc,
                              &state.admitted_fcc, &state.gate_sc, &state.gate_fcc})
        bytes.insert(bytes.end(), array->begin(), array->end());
    return bytes;
}

State decode(const std::vector<Byte>& data) {
    constexpr Byte magic[8] = {'F', 'T', 'D', 'H', 'Y', '0', '1', 0};
    if (data.size() < HEADER_BYTES || !std::equal(magic, magic + 8, data.begin()))
        throw std::invalid_argument("hydro binary schema/magic mismatch");
    std::size_t offset = 20;
    for (const auto* hash : {tables::TABLE_HASH, tables::ENCODING_HASH, tables::LAW_HASH}) {
        if (!std::equal(hash, hash + 32, data.begin() + std::ptrdiff_t(offset)))
            throw std::invalid_argument("hydro binary law/table/encoding mismatch");
        offset += 32;
    }
    const auto L = std::uint32_t(read_u64(data, 8, 4));
    const auto n = sites(L);
    if (data.size() != HEADER_BYTES + n * BYTES_PER_SITE)
        throw std::invalid_argument("hydro binary exact length mismatch");
    State state(L);
    state.microtick = read_u64(data, 12, 8);
    offset = HEADER_BYTES;
    for (auto& value : state.s) { const auto b = data[offset++]; value = std::int8_t(b < 128 ? int(b) : int(b) - 256); }
    for (auto* array : {&state.bank, &state.sc, &state.fcc, &state.admitted_sc,
                        &state.admitted_fcc, &state.gate_sc, &state.gate_fcc}) {
        std::copy_n(data.begin() + std::ptrdiff_t(offset), array->size(), array->begin());
        offset += array->size();
    }
    validate(state);
    return state;
}

std::string events_json(const Events& ev) {
    std::ostringstream out;
    out << "{\"absorptions\":";
    write_array(out, ev.absorptions, [](std::ostream& s, const Absorption& a) {
        s << '[' << a.x << ',' << a.c << ',' << (a.fcc ? 1 : 0) << ',' << a.owner << ',' << a.idx0 << ',' << a.idx1 << ']';
    });
    out << ",\"collisions\":";
    write_array(out, ev.collisions, [](std::ostream& s, const Collision& c) {
        s << '[' << c.site << ',' << c.polarity << ',' << c.before << ',' << c.after << ']';
    });
    out << ",\"crossings\":";
    write_array(out, ev.crossings, [](std::ostream& s, const RelationEvent& r) { write_relation(s, r, true); });
    out << ",\"gate_holds\":";
    write_array(out, ev.gate_holds, [](std::ostream& s, const RelationEvent& r) { write_relation(s, r, false); });
    out << '}';
    return out.str();
}

void load_table(const std::vector<Byte>& bytes) {
    constexpr std::size_t expected_bytes = (std::size_t(1) << 24) * 4;
    if (bytes.size() != expected_bytes) throw std::invalid_argument("hydro collision table has wrong length");
    const auto digest = sha256(bytes.data(), bytes.size());
    if (!std::equal(digest.begin(), digest.end(), tables::TABLE_HASH))
        throw std::invalid_argument("hydro collision table hash mismatch");
    std::vector<std::uint32_t> parsed(std::size_t(1) << 24);
    for (std::size_t i = 0; i < parsed.size(); ++i) {
        parsed[i] = std::uint32_t(bytes[4 * i]) | (std::uint32_t(bytes[4 * i + 1]) << 8) |
                    (std::uint32_t(bytes[4 * i + 2]) << 16) | (std::uint32_t(bytes[4 * i + 3]) << 24);
    }
    table_storage() = std::move(parsed);
    table_loaded_flag() = true;
}

void load_table(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("cannot open hydro collision table blob");
    std::vector<Byte> bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    load_table(bytes);
}

const std::vector<std::uint32_t>& table() {
    if (!table_loaded_flag()) throw std::runtime_error("hydro collision table not loaded");
    return table_storage();
}

}  // namespace ftd::hydro
