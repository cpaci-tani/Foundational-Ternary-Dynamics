#include "staged_runtime.h"
#include "frozen_tables.h"
#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace ftd::strict {
namespace {
using Byte = std::uint8_t;
std::size_t sites(std::uint32_t L) {
    static_assert(std::numeric_limits<std::size_t>::digits <= 64, "native transport supports at most 64-bit address spaces");
    const auto max = std::numeric_limits<std::size_t>::max();
    if (L < 3 || std::size_t(L) > max / L || std::size_t(L) * L > max / L)
        throw std::invalid_argument("invalid strict lattice dimensions");
    const auto n = std::size_t(L) * L * L;
    // 416 bytes/site bounds the at-most-402 token count/site on both wasm32
    // and 64-bit hosts, so the same check also prevents uint64 work overflow.
    if (n > (max - HEADER_BYTES) / BYTES_PER_SITE)
        throw std::invalid_argument("strict lattice byte/count overflow");
    return n;
}
template<class T> void array_check(const std::vector<T>& v, std::size_t size, int lo, int hi) {
    if (v.size() != size || std::any_of(v.begin(), v.end(), [=](T x) { return int(x) < lo || int(x) > hi; }))
        throw std::invalid_argument("strict array shape/alphabet mismatch");
}
std::size_t shift(std::uint32_t L, std::size_t site, unsigned axis, int sign) {
    std::size_t xyz[3] = {site / (std::size_t(L) * L), (site / L) % L, site % L};
    auto& x = xyz[axis];
    x = sign > 0 ? (x + 1 == L ? 0 : x + 1) : (x == 0 ? L - 1 : x - 1);
    return (xyz[0] * L + xyz[1]) * L + xyz[2];
}
std::pair<std::size_t, std::size_t> endpoints(std::uint32_t L, std::size_t owner, unsigned p, unsigned q) {
    unsigned a = 0, b = 0;
    for (unsigned k = 0, count = 0; k < 3; ++k) if (k != p) { (count++ == 0 ? a : b) = k; }
    if (q == 0) return {owner, shift(L, shift(L, owner, a, 1), b, 1)};
    return {shift(L, owner, a, 1), shift(L, owner, b, 1)};
}
void relation(State& out, const State& in, Events& events, bool fcc, std::size_t i, unsigned a, unsigned q) {
    const auto key = fcc ? (i * 3 + a) * 2 + q : i * 3 + a;
    if (!fcc && in.admitted_sc[key]) return;
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
    for (unsigned i = 0; i < bytes; ++i) value |= std::uint64_t(data[offset+i]) << (8*i);
    return value;
}
void write_relation(std::ostream& out, const RelationEvent& r, bool direction) {
    out << "[\"" << (r.fcc ? "fcc" : "sc") << "\"," << r.owner << ",[" << r.axis;
    if (r.fcc) out << ',' << r.diagonal;
    out << ']';
    if (direction) out << ',' << r.direction;
    out << ']';
}
template<class T, class Writer> void write_array(std::ostream& out, const std::vector<T>& values, Writer writer) {
    out << '[';
    bool first = true;
    for (const auto& value : values) { if (!first) out << ','; first = false; writer(out, value); }
    out << ']';
}
}  // namespace

State::State(std::uint32_t size) : L(size) {
    if (size == 0) return;
    const auto n = sites(size);
    s.resize(n); ell.resize(n); bank.resize(n * 384);
    sc.assign(n * 6, frozen::BLANK); fcc.assign(n * 12, frozen::BLANK);
    admitted_sc.resize(n * 3); gate_sc.resize(n * 3); gate_fcc.resize(n * 6);
}

void validate(const State& st) {
    const auto n = sites(st.L);
    array_check(st.s, n, -1, 1); array_check(st.ell, n, 0, 2);
    array_check(st.bank, n*384, 0, 1); array_check(st.sc, n*6, 0, 8); array_check(st.fcc, n*12, 0, 8);
    array_check(st.admitted_sc, n*3, 0, 1); array_check(st.gate_sc, n*3, 0, 1); array_check(st.gate_fcc, n*6, 0, 1);
    if (st.phase() == 0) {
        for (const auto* bits : {&st.admitted_sc, &st.gate_sc, &st.gate_fcc})
            if (std::any_of(bits->begin(), bits->end(), [](Byte b) { return b != 0; }))
                throw std::invalid_argument("uncleared phase-zero pending state");
    } else {
        for (std::size_t key = 0; key < n*3; ++key) if (st.admitted_sc[key]) {
            const auto r = st.sc[key*2+1];
            if (st.sc[key*2] != frozen::BLANK ||
                (r != frozen::ABSORBED_RESERVE[0] && r != frozen::ABSORBED_RESERVE[1]))
                throw std::invalid_argument("illegal admitted relation payload");
        }
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

StepResult step(const State& in) {
    validate(in);
    if (in.microtick == std::numeric_limits<std::uint64_t>::max())
        throw std::overflow_error("strict physical tick counter overflow");
    StepResult result{in, {}};
    auto& out = result.state;
    auto& events = result.events;
    ++out.microtick;
    const auto n = sites(in.L);
    if (in.phase() == 0) {
        std::vector<unsigned> count(n, 0), proposals(n*3, 0);
        std::vector<std::size_t> sources(n*3), channels(n*3), order;
        for (std::size_t i = 0; i < n; ++i)
            for (unsigned c = 0; c < 384; ++c) count[i] += in.bank[i*384+c];
        for (std::size_t i = 0; i < n; ++i) {
            for (unsigned a = 0; a < 3; ++a)
                out.gate_sc[i*3+a] = Byte((count[i] + count[shift(in.L,i,a,1)]) % 2 == 0);
            for (unsigned p = 0; p < 3; ++p) for (unsigned q = 0; q < 2; ++q) {
                const auto ends = endpoints(in.L,i,p,q);
                out.gate_fcc[(i*3+p)*2+q] = Byte((count[ends.first]+count[ends.second]) % 2 == 0);
            }
            for (unsigned c = 0; c < 384; ++c) if (in.bank[i*384+c] && frozen::CHANNEL_PHASE[c] == 2) {
                unsigned a = 0;
                while (frozen::TANGENT[c*3+a] == 0) ++a;
                const auto owner = frozen::TANGENT[c*3+a] > 0 ? i : shift(in.L,i,a,-1);
                const auto key = owner*3+a;
                if (proposals[key]++ == 0) { order.push_back(key); sources[key]=i; channels[key]=c; }
            }
        }
        // Preserve Python's first-presentation insertion order in external logs.
        for (auto key : order) if (proposals[key] == 1 && in.sc[key*2] == frozen::BLANK && in.sc[key*2+1] == frozen::BLANK) {
            const auto x = sources[key], c = channels[key];
            out.bank[x*384+c] = 0;
            out.sc[key*2] = frozen::BLANK;
            out.sc[key*2+1] = frozen::ABSORBED_RESERVE[c < 192 ? 0 : 1];
            out.admitted_sc[key] = 1;
            events.absorptions.push_back({x,c,key/3,key%3});
        }
    } else if (in.phase() == 1) {
        for (std::size_t i = 0; i < n; ++i) {
            for (unsigned polarity = 0; polarity < 2; ++polarity) {
                unsigned count = 0, pair[2] = {};
                for (unsigned c = 0; c < 192; ++c) if (in.bank[i*384+polarity*192+c]) {
                    if (count < 2) pair[count] = c;
                    ++count;
                }
                if (count == 2) {
                    const auto a=pair[0], b=pair[1];
                    const auto encoded = frozen::COLLISION[in.ell[i]][a*(383-a)/2+b-a-1];
                    const unsigned c=encoded/192, d=encoded%192;
                    const auto start=i*384+polarity*192;
                    out.bank[start+a]=0; out.bank[start+b]=0; out.bank[start+c]=1; out.bank[start+d]=1;
                    events.collisions.push_back({i,polarity == 0 ? 1 : -1,{a,b},{c,d}});
                }
            }
            out.ell[i]=Byte((in.ell[i]+2)%3);
            for (unsigned a=0;a<3;++a) relation(out,in,events,false,i,a,0);
            for (unsigned p=0;p<3;++p) for (unsigned q=0;q<2;++q) relation(out,in,events,true,i,p,q);
        }
    } else if (in.phase() == 2) {
        std::fill(out.bank.begin(), out.bank.end(), Byte(0));
        for (std::size_t i=0;i<n;++i) for (unsigned c=0;c<384;++c) if (in.bank[i*384+c]) {
            unsigned a=0;
            while (frozen::TANGENT[c*3+a] == 0) ++a;
            const auto y=shift(in.L,i,a,frozen::TANGENT[c*3+a]);
            unsigned target=frozen::U[c];
            if (in.s[i] != 0) target=frozen::HALF_TURN[target];
            if (out.bank[y*384+target]) throw std::logic_error("strict streaming write collision");
            out.bank[y*384+target]=1;
        }
    } else {
        std::vector<int> incidence(n,0);
        for (std::size_t i=0;i<n;++i) {
            for (unsigned a=0;a<3;++a) {
                const int eps=frozen::EPS[in.sc[(i*3+a)*2]];
                incidence[i]+=eps; incidence[shift(in.L,i,a,1)]-=eps;
            }
            for (unsigned p=0;p<3;++p) for (unsigned q=0;q<2;++q) {
                const auto ends=endpoints(in.L,i,p,q);
                const int eps=frozen::EPS[in.fcc[((i*3+p)*2+q)*2]];
                incidence[ends.first]+=eps; incidence[ends.second]-=eps;
            }
        }
        for (std::size_t i=0;i<n;++i) out.s[i]=std::int8_t(((incidence[i]+1)%3+3)%3-1);
        std::fill(out.admitted_sc.begin(),out.admitted_sc.end(),Byte(0));
        std::fill(out.gate_sc.begin(),out.gate_sc.end(),Byte(0));
        std::fill(out.gate_fcc.begin(),out.gate_fcc.end(),Byte(0));
    }
    validate(out);
    return result;
}

std::vector<Events> advance(State& state, std::uint64_t ticks) {
    validate(state);
    if (ticks > std::numeric_limits<std::uint64_t>::max()-state.microtick)
        throw std::overflow_error("strict batch tick counter overflow");
    State candidate=state;
    std::vector<Events> events;
    for (std::uint64_t i=0;i<ticks;++i) {
        auto next=step(candidate);
        candidate=std::move(next.state);
        events.push_back(std::move(next.events));
    }
    state=std::move(candidate);
    return events;
}

std::vector<Byte> encode(const State& state) {
    validate(state);
    std::vector<Byte> bytes={'F','T','D','S','C','0','1',0};
    bytes.reserve(HEADER_BYTES+sites(state.L)*BYTES_PER_SITE);
    append_u64(bytes,state.L,4); append_u64(bytes,state.microtick,8);
    for (const auto* hash : {frozen::COLLISION_HASH,frozen::ENCODING_HASH,frozen::LAW_HASH})
        bytes.insert(bytes.end(),hash,hash+32);
    for (auto value : state.s) bytes.push_back(Byte(value));
    for (const auto* array : {&state.ell,&state.bank,&state.sc,&state.fcc,&state.admitted_sc,&state.gate_sc,&state.gate_fcc})
        bytes.insert(bytes.end(),array->begin(),array->end());
    return bytes;
}

State decode(const std::vector<Byte>& data) {
    constexpr Byte magic[8]={'F','T','D','S','C','0','1',0};
    if (data.size()<HEADER_BYTES || !std::equal(magic,magic+8,data.begin()))
        throw std::invalid_argument("strict binary schema/magic mismatch");
    std::size_t offset=20;
    for (const auto* hash : {frozen::COLLISION_HASH,frozen::ENCODING_HASH,frozen::LAW_HASH}) {
        if (!std::equal(hash,hash+32,data.begin()+offset)) throw std::invalid_argument("strict binary law/table/encoding mismatch");
        offset+=32;
    }
    const auto L=std::uint32_t(read_u64(data,8,4));
    const auto n=sites(L);
    if (data.size()!=HEADER_BYTES+n*BYTES_PER_SITE) throw std::invalid_argument("strict binary exact length mismatch");
    State state(L);
    state.microtick=read_u64(data,12,8);
    for (auto& value:state.s) { const auto b=data[offset++]; value=std::int8_t(b<128?int(b):int(b)-256); }
    for (auto* array:{&state.ell,&state.bank,&state.sc,&state.fcc,&state.admitted_sc,&state.gate_sc,&state.gate_fcc}) {
        std::copy_n(data.begin()+offset,array->size(),array->begin()); offset+=array->size();
    }
    validate(state);
    return state;
}

std::string events_json(const std::vector<Events>& events) {
    std::ostringstream out;
    write_array(out,events,[](std::ostream& o,const Events& ev) {
        o << "{\"absorptions\":";
        write_array(o,ev.absorptions,[](std::ostream& s,const auto& a){s<<'['<<a[0]<<','<<a[1]<<','<<a[2]<<','<<a[3]<<']';});
        o << ",\"collisions\":";
        write_array(o,ev.collisions,[](std::ostream& s,const Collision& c){s<<'['<<c.site<<','<<c.polarity<<",["<<c.before[0]<<','<<c.before[1]<<"],["<<c.after[0]<<','<<c.after[1]<<"]]";});
        o << ",\"crossings\":";
        write_array(o,ev.crossings,[](std::ostream& s,const RelationEvent& r){write_relation(s,r,true);});
        o << ",\"gate_holds\":";
        write_array(o,ev.gate_holds,[](std::ostream& s,const RelationEvent& r){write_relation(s,r,false);});
        o << '}';
    });
    return out.str();
}
}  // namespace ftd::strict
