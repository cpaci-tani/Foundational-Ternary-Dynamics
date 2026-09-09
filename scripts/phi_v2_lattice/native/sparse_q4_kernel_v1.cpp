#include "sparse_q4_kernel_v1.hpp"
#include <algorithm>
#include <climits>
#include <limits>
#include <new>
#include <tuple>
#include <utility>

namespace ftd::q4native_v1 {
static_assert(CHAR_BIT==8 && sizeof(std::uint32_t)==4 && sizeof(std::int32_t)==4);
static_assert(sizeof(Payload)<=80 && sizeof(Events)<=384);
static_assert(sizeof(WIRE_MAGIC)-1==16);
namespace {
constexpr unsigned axes[6][3]={{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
constexpr char digits[]="0123456789abcdef";
[[noreturn]] void fail(ErrorCategory c,const char* text) { throw Error(c,text); }
void require(bool condition,ErrorCategory c,const char* text) { if(!condition) fail(c,text); }
unsigned hex_digit(char c) {
    if(c>='0' && c<='9') return static_cast<unsigned>(c-'0');
    if(c>='a' && c<='f') return static_cast<unsigned>(c-'a'+10);
    fail(ErrorCategory::INVALID_RECORD,"canonical lowercase hexadecimal required");
}
unsigned check_tick(std::string_view tick) {
    require(!tick.empty() && (tick.size()==1 || tick.front()!='0'),ErrorCategory::INVALID_RECORD,"noncanonical ordinal");
    unsigned p=0;
    for(char c:tick) p=(16*p+hex_digit(c))%19;
    return p;
}
std::string next_tick(std::string_view tick) {
    std::string out(tick);
    for(std::size_t i=out.size();i>0;--i) {
        const unsigned d=hex_digit(out[i-1]);
        if(d<15) { out[i-1]=digits[d+1]; return out; }
        out[i-1]='0';
    }
    out.insert(out.begin(),'1');
    return out;
}
std::uint32_t volume(const Payload& p) { return std::uint32_t(p.L)*p.L*p.L; }
std::uint32_t shift(const Payload& p,std::uint32_t site,unsigned axis,int sign) {
    const auto L=std::int32_t(p.L);
    std::int32_t xyz[3]={std::int32_t(site/(p.L*p.L)),std::int32_t((site/p.L)%p.L),std::int32_t(site%p.L)};
    xyz[axis]+=sign;
    if(xyz[axis]<0) xyz[axis]+=L;
    if(xyz[axis]>=L) xyz[axis]-=L;
    return std::uint32_t((xyz[0]*L+xyz[1])*L+xyz[2]);
}
bool carrier_less(const Carrier&a,const Carrier&b) { return std::tie(a.site,a.slot)<std::tie(b.site,b.slot); }
bool edge_less(const Edge&a,const Edge&b) { return std::tie(a.owner,a.axis)<std::tie(b.owner,b.axis); }
template<class T,std::size_t N,class Less> void sort_prefix(std::array<T,N>&a,unsigned count,Less less) {
    require(count<=N,ErrorCategory::INTERNAL_INVARIANT_FAILURE,"sort prefix exceeds fixed capacity");
    for(std::size_t i=1;i<N && i<count;++i)
        for(std::size_t j=i;j>0 && less(a[j],a[j-1]);--j) std::swap(a[j],a[j-1]);
}
void validate_payload(const Payload& p) {
    require(p.L>=4 && !(p.L%2),ErrorCategory::INVALID_RECORD,"even L>=4 required");
    require(p.L<=64,ErrorCategory::UNSUPPORTED_DOMAIN,"native capability requires L<=64");
    require(p.origin_code<48 && p.eta<2 && p.edge_count<=4,ErrorCategory::INVALID_RECORD,"background/edge count outside alphabet");
    unsigned plus=0,credit=0;
    std::array<std::pair<std::uint32_t,std::int32_t>,12> residual{};
    std::size_t n=0;
    auto add=[&](std::uint32_t site,int value) {
        for(std::size_t i=0;i<n;++i) if(residual[i].first==site) {residual[i].second+=value;return;}
        require(n<residual.size(),ErrorCategory::INTERNAL_INVARIANT_FAILURE,"Gauss scratch capacity");
        residual[n++]={site,value};
    };
    for(std::size_t i=0;i<4;++i) {
        const auto& c=p.carriers[i];
        require(c.site<volume(p) && c.slot<2 && c.direction>=1 && c.direction<=6 && c.credit<2 && c.attempted<2,
                ErrorCategory::INVALID_RECORD,"carrier outside finite alphabet");
        require(i==0 || carrier_less(p.carriers[i-1],c),ErrorCategory::INVALID_RECORD,"carrier order/duplicate key");
        plus+=c.slot==0; credit+=c.credit; add(c.site,c.slot==0?-1:1);
    }
    require(plus==2 && p.edge_count+credit==4,ErrorCategory::INVALID_RECORD,"not the two-plus two-minus Q4 sector");
    for(std::size_t i=0;i<4;++i) {
        const auto&e=p.edges[i];
        if(i>=p.edge_count) {
            require(e.owner==0 && e.axis==0 && e.q==0,ErrorCategory::INVALID_RECORD,"nonzero inactive edge row");
            continue;
        }
        require(e.owner<volume(p) && e.axis<3 && (e.q==1 || e.q==-1),ErrorCategory::INVALID_RECORD,"edge outside finite alphabet");
        require(i==0 || edge_less(p.edges[i-1],e),ErrorCategory::INVALID_RECORD,"edge order/duplicate key");
        add(e.owner,e.q); add(shift(p,e.owner,e.axis,1),-e.q);
    }
    for(std::size_t i=0;i<n;++i) require(residual[i].second==0,ErrorCategory::INVALID_RECORD,"Gauss incidence mismatch");
}
int find_carrier(const Payload&p,std::uint32_t x,unsigned slot) {
    for(int i=0;i<4;++i) if(p.carriers[i].site==x && p.carriers[i].slot==slot) return i;
    return -1;
}
int flux_at(const Payload&p,std::uint32_t x,unsigned axis) {
    for(unsigned i=0;i<p.edge_count;++i) if(p.edges[i].owner==x && p.edges[i].axis==axis) return p.edges[i].q;
    return 0;
}
template<std::size_t W,std::size_t C> void append(EventList<W,C>&list,const std::array<std::int32_t,W>&row) {
    require(list.count<C,ErrorCategory::INTERNAL_INVARIANT_FAILURE,"event capacity exceeded");
    list.rows[list.count++]=row;
}
void u8(std::vector<std::uint8_t>&out,unsigned x) { out.push_back(static_cast<std::uint8_t>(x)); }
void le(std::vector<std::uint8_t>&out,std::uint64_t x,unsigned n) {
    for(unsigned i=0;i<n;++i) {u8(out,unsigned(x&255));x>>=8;}
}
void hash_bytes(std::vector<std::uint8_t>&out,const char*hash) {
    for(unsigned i=0;i<64;i+=2) u8(out,16*hex_digit(hash[i])+hex_digit(hash[i+1]));
}
class Reader {
public:
    Reader(const std::uint8_t*p,std::size_t n):p_(p),n_(n) {
        require(p!=nullptr || n==0,ErrorCategory::INVALID_WIRE,"null input buffer");
    }
    std::uint64_t read(unsigned n) {
        require(n<=8 && n<=left(),ErrorCategory::INVALID_WIRE,"truncated integer");
        std::uint64_t out=0;
        for(unsigned i=0;i<n;++i) out|=std::uint64_t(p_[at_++])<<(8*i);
        return out;
    }
    const std::uint8_t* take(std::size_t n) {
        require(n<=left(),ErrorCategory::INVALID_WIRE,"truncated payload");
        const auto*out=p_+at_;at_+=n;return out;
    }
    std::size_t left() const { return n_-at_; }
private:
    const std::uint8_t*p_;std::size_t n_,at_=0;
};
void expect_hash(Reader&r,const char*hash) {
    for(unsigned i=0;i<64;i+=2) require(r.read(1)==16*hex_digit(hash[i])+hex_digit(hash[i+1]),ErrorCategory::INVALID_WIRE,"foreign fixed identity");
}
bool site_word(std::int32_t x) {return x>=0 && std::uint32_t(x)<=MAX_SITE;}
bool polarity(std::int32_t x) {return x==1 || x==-1;}
bool bit(std::int32_t x) {return x==0 || x==1;}
bool heading(std::int32_t x) {return x>=1 && x<=6;}
bool ternary(std::int32_t x) {return x>=-1 && x<=1;}
template<std::size_t W,std::size_t C,class Check> void check_rows(const EventList<W,C>&list,Check check) {
    require(list.count<=C,ErrorCategory::INVALID_WIRE,"event count exceeds capacity");
    for(unsigned i=0;i<list.count;++i) require(check(list.rows[i]),ErrorCategory::INVALID_WIRE,"event word outside domain");
}
void validate_events(const Events&e) {
    check_rows(e.moves,[](const auto&r){return polarity(r[0]) && site_word(r[1]) && site_word(r[2]) && site_word(r[3]) && r[1]!=r[2] && (r[3]==r[1] || r[3]==r[2]) && r[4]>=0 && r[4]<3 && ternary(r[5]) && ternary(r[6]) && r[6]==r[5]-r[0]*(r[3]==r[1]?1:-1) && bit(r[7]) && bit(r[8]) && r[8]-r[7]+r[6]*r[6]-r[5]*r[5]==0;});
    check_rows(e.redirects,[](const auto&r){return polarity(r[0]) && site_word(r[1]) && heading(r[2]) && heading(r[3]) && r[4]>=2 && r[4]<=4;});
    check_rows(e.capacity_holds,[](const auto&r){return polarity(r[0]) && site_word(r[1]) && r[2]>=0 && r[2]<3;});
    check_rows(e.attempt_marks,[](const auto&r){return polarity(r[0]) && site_word(r[1]) && r[2]>=0 && r[2]<=4;});
    check_rows(e.attempt_expiries,[](const auto&r){return polarity(r[0]) && site_word(r[1]);});
    check_rows(e.onsite_alignments,[](const auto&r){return site_word(r[0]) && polarity(r[1]) && heading(r[2]) && heading(r[3]) && heading(r[4]) && r[4]==r[r[1]==1?2:3];});
    check_rows(e.credit_exchanges,[](const auto&r){return site_word(r[0]) && r[1]>=0 && r[1]<3 && polarity(r[2]) && site_word(r[3]) && r[4]==-r[2] && site_word(r[5]) && r[6]==1 && r[7]==0 && r[8]==0 && r[9]==1 && heading(r[10]) && heading(r[11]) && heading(r[12]);});
    const bool reset=e.attempt_expiries.count || e.onsite_alignments.count;
    const bool exchange=e.credit_exchanges.count;
    const bool hop=e.moves.count || e.redirects.count || e.capacity_holds.count || e.attempt_marks.count;
    require(unsigned(reset)+unsigned(exchange)+unsigned(hop)<=1,ErrorCategory::INVALID_WIRE,"mixed physical-stage events");
    if(hop) {
        const auto epsilon=e.capacity_holds.count?e.capacity_holds.rows[0][0]:e.attempt_marks.rows[0][0];
        for(unsigned i=0;i<e.moves.count;++i) require(e.moves.rows[i][0]==epsilon,ErrorCategory::INVALID_WIRE,"mixed hop polarity");
        for(unsigned i=0;i<e.redirects.count;++i) require(e.redirects.rows[i][0]==epsilon,ErrorCategory::INVALID_WIRE,"mixed hop polarity");
        for(unsigned i=0;i<e.attempt_marks.count;++i) require(e.attempt_marks.rows[i][0]==epsilon,ErrorCategory::INVALID_WIRE,"mixed hop polarity");
    }
    require(e.moves.count+e.redirects.count+2*e.capacity_holds.count<=2,ErrorCategory::INVALID_WIRE,"more active carriers than Q4");
    require(e.attempt_marks.count>=e.moves.count+e.redirects.count && e.attempt_marks.count<=e.moves.count+e.redirects.count+2*e.capacity_holds.count,
            ErrorCategory::INVALID_WIRE,"hop attempt count mismatch");
    for(unsigned i=0;i<e.moves.count;++i) {
        const auto&r=e.moves.rows[i];unsigned found=0;
        for(unsigned j=0;j<e.attempt_marks.count;++j) {const auto&m=e.attempt_marks.rows[j];found+=(m[0]==r[0] && m[1]==r[1] && m[2]==accepted);}
        require(found==1,ErrorCategory::INVALID_WIRE,"accepted move mark mismatch");
    }
    for(unsigned i=0;i<e.redirects.count;++i) {
        const auto&r=e.redirects.rows[i];unsigned found=0;
        for(unsigned j=0;j<e.attempt_marks.count;++j) {const auto&m=e.attempt_marks.rows[j];found+=(m[0]==r[0] && m[1]==r[1] && m[2]==r[4]);}
        require(found==1,ErrorCategory::INVALID_WIRE,"redirect mark mismatch");
    }
    for(unsigned i=0;i<e.attempt_marks.count;++i) {
        const auto&m=e.attempt_marks.rows[i];unsigned found=0;
        for(unsigned j=0;j<i;++j)
            require(m[0]!=e.attempt_marks.rows[j][0] || m[1]!=e.attempt_marks.rows[j][1],ErrorCategory::INVALID_WIRE,"duplicate attempt mark");
        if(m[2]==capacity) {
            found=e.capacity_holds.count==1 && e.capacity_holds.rows[0][0]==m[0];
        } else if(m[2]==accepted) {
            for(unsigned j=0;j<e.moves.count;++j) found+=e.moves.rows[j][0]==m[0] && e.moves.rows[j][1]==m[1];
        } else {
            for(unsigned j=0;j<e.redirects.count;++j) {
                const auto&r=e.redirects.rows[j];found+=r[0]==m[0] && r[1]==m[1] && r[4]==m[2];
            }
        }
        require(found==1,ErrorCategory::INVALID_WIRE,"attempt mark lacks its matching event");
    }
}
template<std::size_t W,std::size_t C> void event_write(std::vector<std::uint8_t>&out,const EventList<W,C>&list) {
    for(unsigned i=0;i<list.count;++i) for(auto x:list.rows[i]) le(out,static_cast<std::uint32_t>(x),4);
}
template<std::size_t W,std::size_t C> void event_read(Reader&r,EventList<W,C>&list) {
    require(list.count<=C,ErrorCategory::INVALID_WIRE,"event count overflow");
    for(unsigned i=0;i<list.count;++i) for(auto&x:list.rows[i]) {
        const auto u=r.read(4);
        x=static_cast<std::int32_t>(u<=0x7fffffffULL ? static_cast<std::int64_t>(u) : static_cast<std::int64_t>(u)-0x100000000LL);
    }
}
} // namespace

const char* category_name(ErrorCategory c) noexcept {
    switch(c) {
    case ErrorCategory::INVALID_RECORD:return "INVALID_RECORD";
    case ErrorCategory::UNSUPPORTED_DOMAIN:return "UNSUPPORTED_DOMAIN";
    case ErrorCategory::INVALID_WIRE:return "INVALID_WIRE";
    case ErrorCategory::ALLOCATION_FAILURE:return "ALLOCATION_FAILURE";
    case ErrorCategory::INTERNAL_INVARIANT_FAILURE:return "INTERNAL_INVARIANT_FAILURE";
    }
    return "INTERNAL_INVARIANT_FAILURE";
}
Error::Error(ErrorCategory c,const std::string&text):std::runtime_error(std::string(category_name(c))+": "+text),category_(c) {}
State::State(const Payload&p,std::string t):payload_(p),microtick_(std::move(t)) {}
const Payload& payload(const State&s) {return s.payload_;}
const std::string& microtick_hex(const State&s) {return s.microtick_;}
State admit(const Payload&p,std::string_view tick) {
    try {validate_payload(p);check_tick(tick);return State(p,std::string(tick));}
    catch(const std::bad_alloc&) {fail(ErrorCategory::ALLOCATION_FAILURE,"state admission allocation");}
    catch(const std::length_error&) {fail(ErrorCategory::ALLOCATION_FAILURE,"ordinal exceeds host string capacity");}
}
unsigned phase(const State&s) {return check_tick(microtick_hex(s));}
unsigned background_code(const State&s,std::uint32_t x) {
    const auto&p=payload(s);
    require(x<volume(p),ErrorCategory::INVALID_RECORD,"background site out of range");
    const unsigned xyz[3]={x/(p.L*p.L),(x/p.L)%p.L,x%p.L};
    unsigned code=p.origin_code;
    for(unsigned a=0;a<3;++a) code^=(xyz[axes[p.origin_code/8][a]]%2)<<a;
    return code;
}
std::uint32_t shift_site(const State&s,std::uint32_t x,unsigned a,int sign) {
    require(x<volume(payload(s)) && a<3 && (sign==1 || sign==-1),ErrorCategory::INVALID_RECORD,"invalid shift arguments");
    return shift(payload(s),x,a,sign);
}
MatchingEdge matching_edge(const State&s,std::uint32_t x,unsigned column,unsigned parity) {
    require(column<3 && parity<2,ErrorCategory::INVALID_RECORD,"invalid matching arguments");
    const unsigned code=background_code(s,x),axis=axes[code/8][column];
    const auto owner=((code>>column)&1)==parity ? x : shift_site(s,x,axis,-1);
    return {owner,shift_site(s,owner,axis,1),static_cast<std::uint8_t>(axis)};
}

StepResult step(const State&s) {
    try {
        const auto&in=payload(s);
        // A moved-from State retains a valid payload but no ordinal. Reject it.
        const unsigned p=phase(s);
        Payload out=in;Events events;
        if(p==0) {
            for(unsigned i=0;i<4;++i) {
                const auto&c=in.carriers[i];
                if(c.attempted) append(events.attempt_expiries,{{c.slot==0?1:-1,std::int32_t(c.site)}});
                out.carriers[i].attempted=0;
            }
            for(unsigned i=0;i<4;++i) {
                const auto&c=in.carriers[i];
                if(c.slot!=0) continue;
                const int minus=find_carrier(in,c.site,1);
                if(minus<0) continue;
                const auto heading_value=in.carriers[in.eta?unsigned(minus):i].direction;
                out.carriers[i].direction=out.carriers[unsigned(minus)].direction=heading_value;
                append(events.onsite_alignments,{{std::int32_t(c.site),in.eta?-1:1,c.direction,in.carriers[unsigned(minus)].direction,heading_value}});
            }
        } else if(p<=6) {
            for(unsigned i=0;i<in.edge_count;++i) {
                const auto&e=in.edges[i];
                const auto match=matching_edge(s,e.owner,(p-1)/2,(p-1)%2);
                if(match.owner!=e.owner || match.axis!=e.axis) continue;
                const unsigned left_slot=e.q==1?0:1;
                const int li=find_carrier(in,match.owner,left_slot),ri=find_carrier(in,match.head,left_slot^1);
                if(li<0 || ri<0) continue;
                const auto&left=in.carriers[unsigned(li)];const auto&right=in.carriers[unsigned(ri)];
                if(left.credit==right.credit) continue;
                const unsigned donor=left.credit?unsigned(li):unsigned(ri),recipient=left.credit?unsigned(ri):unsigned(li);
                const auto heading_value=std::uint8_t(2*match.axis+(left.credit?1:2));
                out.carriers[donor].credit=0;out.carriers[recipient].credit=1;
                out.carriers[donor].direction=out.carriers[recipient].direction=heading_value;
                const auto&d=in.carriers[donor];const auto&r=in.carriers[recipient];
                append(events.credit_exchanges,{{std::int32_t(match.owner),match.axis,d.slot==0?1:-1,std::int32_t(d.site),r.slot==0?1:-1,std::int32_t(r.site),1,0,0,1,d.direction,r.direction,heading_value}});
            }
        } else {
            const unsigned r=p-7,slot=(r/6)^in.eta;const int epsilon=slot==0?1:-1;
            std::array<MatchingEdge,2> matches{};unsigned count=0;
            for(const auto&c:in.carriers) if(c.slot==slot) {
                auto m=matching_edge(s,c.site,(r%6)/2,r%2);
                bool duplicate=false;
                for(unsigned i=0;i<count;++i) duplicate|=matches[i].owner==m.owner && matches[i].axis==m.axis;
                if(!duplicate) {
                    require(count<matches.size(),ErrorCategory::INTERNAL_INVARIANT_FAILURE,"matching edge capacity");
                    matches[count++]=m;
                }
            }
            sort_prefix(matches,count,[](const auto&a,const auto&b){return std::tie(a.owner,a.head,a.axis)<std::tie(b.owner,b.head,b.axis);});
            std::array<Edge,6> scratch{};unsigned edge_count=in.edge_count;
            std::copy(in.edges.begin(),in.edges.begin()+in.edge_count,scratch.begin());
            auto write_flux=[&](std::uint32_t owner,unsigned axis,int q) {
                for(unsigned i=0;i<edge_count;++i) if(scratch[i].owner==owner && scratch[i].axis==axis) {
                    if(q) scratch[i].q=std::int8_t(q);
                    else {scratch[i]=scratch[--edge_count];scratch[edge_count]={};}
                    return;
                }
                if(q) {
                    require(edge_count<scratch.size(),ErrorCategory::INTERNAL_INVARIANT_FAILURE,"temporary edge capacity");
                    scratch[edge_count++]={owner,std::uint8_t(axis),std::int8_t(q)};
                }
            };
            for(unsigned i=0;i<count;++i) {
                const auto&m=matches[i];
                const int li=find_carrier(in,m.owner,slot),ri=find_carrier(in,m.head,slot);
                if(li>=0 && ri>=0) {
                    append(events.capacity_holds,{{epsilon,std::int32_t(m.owner),m.axis}});
                    for(int j=0;j<2;++j) {
                        const unsigned k=unsigned(j==0?li:ri);const auto&c=in.carriers[k];
                        if(c.direction==2*m.axis+1+j && !c.attempted) {
                            out.carriers[k].attempted=1;
                            append(events.attempt_marks,{{epsilon,std::int32_t(c.site),capacity}});
                        }
                    }
                    continue;
                }
                require(li>=0 || ri>=0,ErrorCategory::INTERNAL_INVARIANT_FAILURE,"unoccupied enumerated matching edge");
                const unsigned index=unsigned(li>=0?li:ri);const auto&c=in.carriers[index];
                const int sigma=li>=0?1:-1;const auto destination=li>=0?m.head:m.owner;
                if(c.attempted || c.direction!=2*m.axis+(sigma==1?1:2)) continue;
                const int q=flux_at(in,m.owner,m.axis),qn=q-epsilon*sigma,kn=int(c.credit)-(qn*qn-q*q);
                std::int32_t reason=accepted;out.carriers[index].attempted=1;
                if(qn>=-1 && qn<=1 && kn>=0 && kn<=1) {
                    out.carriers[index].site=destination;out.carriers[index].credit=std::uint8_t(kn);
                    write_flux(m.owner,m.axis,qn);
                    append(events.moves,{{epsilon,std::int32_t(c.site),std::int32_t(destination),std::int32_t(m.owner),m.axis,q,qn,c.credit,kn}});
                } else {
                    unsigned ports=0,port=0;
                    for(unsigned a=0;a<3;++a) {
                        if(flux_at(in,c.site,a)==epsilon) {++ports;port=2*a+1;}
                        if(-flux_at(in,shift(in,c.site,a,-1),a)==epsilon) {++ports;port=2*a+2;}
                    }
                    const unsigned heading_value=c.credit==0 && ports==1 ? port : ((c.direction-1)^1)+1;
                    out.carriers[index].direction=std::uint8_t(heading_value);
                    reason=qn>1 || qn<-1?flux_capacity:kn<0?credit_deficit:credit_capacity;
                    append(events.redirects,{{epsilon,std::int32_t(c.site),c.direction,std::int32_t(heading_value),reason}});
                }
                append(events.attempt_marks,{{epsilon,std::int32_t(c.site),reason}});
            }
            require(edge_count<=4,ErrorCategory::INTERNAL_INVARIANT_FAILURE,"final flux exceeds Q4");
            sort_prefix(scratch,edge_count,edge_less);
            out.edges={};out.edge_count=std::uint8_t(edge_count);
            std::copy(scratch.begin(),scratch.begin()+edge_count,out.edges.begin());
            sort_prefix(out.carriers,4,carrier_less);
        }
        validate_events(events);
        try { return {admit(out,next_tick(microtick_hex(s))),events}; }
        catch(const Error&e) {
            if(e.category()==ErrorCategory::ALLOCATION_FAILURE) throw;
            throw Error(ErrorCategory::INTERNAL_INVARIANT_FAILURE,e.what());
        }
    } catch(const std::bad_alloc&) {fail(ErrorCategory::ALLOCATION_FAILURE,"step allocation");}
      catch(const std::length_error&) {fail(ErrorCategory::ALLOCATION_FAILURE,"step ordinal capacity");}
}

std::vector<std::uint8_t> encode_state(const State&s) {
    try {
        const auto&p=payload(s);validate_payload(p);check_tick(microtick_hex(s));
        std::vector<std::uint8_t> out;
        require(microtick_hex(s).size()<=out.max_size()-192,ErrorCategory::ALLOCATION_FAILURE,"state wire length overflow");
        out.reserve(192+microtick_hex(s).size());
        for(unsigned i=0;i<16;++i) u8(out,WIRE_MAGIC[i]);
        hash_bytes(out,RULE_HASH);hash_bytes(out,FRAME_HASH);hash_bytes(out,DENSE_ENCODING);
        le(out,p.L,2);u8(out,p.origin_code);u8(out,p.eta);u8(out,p.edge_count);le(out,0,3);
        for(const auto&c:p.carriers) {le(out,c.site,4);u8(out,c.slot);u8(out,c.direction);u8(out,c.credit);u8(out,c.attempted);}
        for(const auto&e:p.edges) {le(out,e.owner,4);u8(out,e.axis);u8(out,e.q==-1?255:unsigned(e.q));le(out,0,2);}
        le(out,microtick_hex(s).size(),8);
        for(char c:microtick_hex(s)) u8(out,static_cast<unsigned char>(c));
        return out;
    } catch(const std::bad_alloc&) {fail(ErrorCategory::ALLOCATION_FAILURE,"state wire allocation");}
      catch(const std::length_error&) {fail(ErrorCategory::ALLOCATION_FAILURE,"state wire capacity");}
}
State decode_state(const std::uint8_t*bytes,std::size_t length) {
    try {
        Reader r(bytes,length);
        for(unsigned i=0;i<16;++i) require(r.read(1)==std::uint8_t(WIRE_MAGIC[i]),ErrorCategory::INVALID_WIRE,"foreign state magic");
        expect_hash(r,RULE_HASH);expect_hash(r,FRAME_HASH);expect_hash(r,DENSE_ENCODING);
        Payload p;p.L=std::uint16_t(r.read(2));p.origin_code=std::uint8_t(r.read(1));p.eta=std::uint8_t(r.read(1));p.edge_count=std::uint8_t(r.read(1));
        require(r.read(3)==0,ErrorCategory::INVALID_WIRE,"nonzero header reserved bytes");
        for(auto&c:p.carriers) {c.site=std::uint32_t(r.read(4));c.slot=std::uint8_t(r.read(1));c.direction=std::uint8_t(r.read(1));c.credit=std::uint8_t(r.read(1));c.attempted=std::uint8_t(r.read(1));}
        for(auto&e:p.edges) {
            e.owner=std::uint32_t(r.read(4));e.axis=std::uint8_t(r.read(1));const auto q=r.read(1);
            require(q==0 || q==1 || q==255,ErrorCategory::INVALID_WIRE,"nonternary edge byte");
            e.q=q==255?std::int8_t(-1):std::int8_t(q);
            require(r.read(2)==0,ErrorCategory::INVALID_WIRE,"nonzero edge reserved bytes");
        }
        const auto h=r.read(8);
        require(h>0 && h<=std::numeric_limits<std::size_t>::max() && h==r.left(),ErrorCategory::INVALID_WIRE,"ordinal length or trailing bytes");
        const auto*text=r.take(static_cast<std::size_t>(h));
        return admit(p,std::string_view(reinterpret_cast<const char*>(text),static_cast<std::size_t>(h)));
    } catch(const std::bad_alloc&) {fail(ErrorCategory::ALLOCATION_FAILURE,"decode state allocation");}
}
std::vector<std::uint8_t> encode_events(const Events&e) {
    try {
        validate_events(e);std::vector<std::uint8_t> out;out.reserve(112);
        for(unsigned x:{e.moves.count,e.redirects.count,e.capacity_holds.count,e.attempt_marks.count,e.attempt_expiries.count,e.onsite_alignments.count,e.credit_exchanges.count}) u8(out,x);
        u8(out,0);
        event_write(out,e.moves);event_write(out,e.redirects);event_write(out,e.capacity_holds);event_write(out,e.attempt_marks);event_write(out,e.attempt_expiries);event_write(out,e.onsite_alignments);event_write(out,e.credit_exchanges);
        require(out.size()<=112,ErrorCategory::INTERNAL_INVARIANT_FAILURE,"event wire exceeds single-stage bound");
        return out;
    } catch(const std::bad_alloc&) {fail(ErrorCategory::ALLOCATION_FAILURE,"event wire allocation");}
}
Events decode_events(const std::uint8_t*bytes,std::size_t length) {
    Reader r(bytes,length);Events e;
    e.moves.count=std::uint8_t(r.read(1));e.redirects.count=std::uint8_t(r.read(1));e.capacity_holds.count=std::uint8_t(r.read(1));e.attempt_marks.count=std::uint8_t(r.read(1));e.attempt_expiries.count=std::uint8_t(r.read(1));e.onsite_alignments.count=std::uint8_t(r.read(1));e.credit_exchanges.count=std::uint8_t(r.read(1));
    require(r.read(1)==0,ErrorCategory::INVALID_WIRE,"nonzero event reserved byte");
    event_read(r,e.moves);event_read(r,e.redirects);event_read(r,e.capacity_holds);event_read(r,e.attempt_marks);event_read(r,e.attempt_expiries);event_read(r,e.onsite_alignments);event_read(r,e.credit_exchanges);
    require(r.left()==0,ErrorCategory::INVALID_WIRE,"trailing event bytes");validate_events(e);return e;
}
} // namespace ftd::q4native_v1
