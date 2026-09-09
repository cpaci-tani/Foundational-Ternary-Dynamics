// Offline exact counter. SPEC_STRICT_FULL_MEMORY_EXECUTION_V1.md is its contract.
// No engine/CMake integration and no microscopic state evolution interface.
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#include <cstdio>
#include <fcntl.h>
#include <io.h>
#endif

using Clock = std::chrono::steady_clock;
constexpr uint32_t N = 11740, FULL = (1u << 24)-1;
constexpr uint64_t TOTAL = 34462770;

struct UInt512 {
    std::array<uint32_t,16> limb{};
    explicit UInt512(uint32_t x=0) { limb[0]=x; }
    bool zero() const { for(auto x:limb) if(x) return false; return true; }
    int compare(const UInt512& b) const {
        for(int i=15;i>=0;--i) if(limb[i]!=b.limb[i]) return limb[i]<b.limb[i]?-1:1;
        return 0;
    }
    void multiply(uint32_t x) {
        uint64_t carry=0;
        for(auto& value:limb) {
            uint64_t wide=uint64_t(value)*x+carry;
            value=uint32_t(wide); carry=wide>>32;
        }
        if(carry) throw std::overflow_error("512-bit product overflow");
    }
    void add(const UInt512& b) {
        uint64_t carry=0;
        for(unsigned i=0;i<16;++i) {
            uint64_t wide=uint64_t(limb[i])+b.limb[i]+carry;
            limb[i]=uint32_t(wide); carry=wide>>32;
        }
        if(carry) throw std::overflow_error("512-bit addition overflow");
    }
    void subtract(const UInt512& b) {
        if(compare(b)<0) throw std::logic_error("negative unsigned subtraction");
        uint64_t borrow=0;
        for(unsigned i=0;i<16;++i) {
            uint64_t sub=uint64_t(b.limb[i])+borrow, old=limb[i];
            limb[i]=uint32_t(old-sub); borrow=old<sub;
        }
        if(borrow) throw std::logic_error("subtraction borrow");
    }
    uint32_t divide(uint32_t d) {
        uint64_t rem=0;
        for(int i=15;i>=0;--i) {
            uint64_t wide=(rem<<32)|limb[i];
            limb[i]=uint32_t(wide/d); rem=wide%d;
        }
        return uint32_t(rem);
    }
    std::string decimal() const {
        if(zero()) return "0";
        UInt512 copy=*this;
        std::vector<uint32_t> chunks;
        while(!copy.zero()) chunks.push_back(copy.divide(1000000000));
        std::string out=std::to_string(chunks.back());
        for(int i=int(chunks.size())-2;i>=0;--i) {
            std::string s=std::to_string(chunks[i]); out+=std::string(9-s.size(),'0')+s;
        }
        return out;
    }
};

struct Signed512 {
    bool negative=false;
    UInt512 magnitude;
    void add_product(const UInt512& weight,int coefficient) {
        if(coefficient==0 || weight.zero()) return;
        UInt512 term=weight;
        term.multiply(uint32_t(coefficient<0?-coefficient:coefficient));
        bool sign=coefficient<0;
        if(negative==sign) magnitude.add(term);
        else if(magnitude.compare(term)>=0) magnitude.subtract(term);
        else { term.subtract(magnitude); magnitude=term; negative=sign; }
        if(magnitude.zero()) negative=false;
        if(magnitude.limb[15]&0x80000000u) throw std::overflow_error("signed512 overflow");
    }
    std::string decimal() const { return (negative?"-":"")+magnitude.decimal(); }
};

uint32_t popcount(uint32_t x) { uint32_t n=0; while(x) { x&=x-1; ++n; } return n; }
uint32_t compact(uint32_t occupied,uint32_t mask) {
    uint32_t code=0,bit=0;
    for(unsigned c=0;c<24;++c) if(mask&(1u<<c)) code|=((occupied>>c)&1u)<<bit++;
    return code;
}
uint64_t number(const std::string& text) {
    if(text.empty() || text.find_first_not_of("0123456789")!=std::string::npos)
        throw std::invalid_argument("noncanonical integer argument");
    size_t used=0; auto v=std::stoull(text,&used);
    if(used!=text.size()) throw std::invalid_argument("invalid integer argument");
    return v;
}
struct Block { uint32_t input,output,m,n; std::array<uint32_t,16> counts; };
struct Job {
    std::array<std::array<int,4>,24> labels;
    std::vector<uint32_t> eligible;
    std::array<Block,18> blocks;
    std::array<std::pair<unsigned,unsigned>,14> reps;
    std::vector<std::array<uint8_t,18>> incoming,outgoing;
};
uint32_t read32(const std::vector<uint8_t>& bytes,size_t& pos) {
    if(pos+4>bytes.size()) throw std::invalid_argument("truncated job");
    uint32_t x=0; for(unsigned j=0;j<4;++j) x|=uint32_t(bytes[pos++])<<(8*j);
    return x;
}
Job read_job() {
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(std::cin)),{});
    const char magic[16]={'F','T','D','K','1','C','E','N','T','R','A','L','1',0,0,0};
    if(bytes.size()!=48644 || std::memcmp(bytes.data(),magic,16))
        throw std::invalid_argument("foreign job magic or length");
    size_t pos=16;
    for(uint32_t expected:{1u,N,18u,14u,432u})
        if(read32(bytes,pos)!=expected) throw std::invalid_argument("foreign job dimensions");
    Job job;
    std::vector<std::array<int,4>> labels;
    for(int a=-1;a<=1;++a) for(int b=-1;b<=1;++b)
    for(int c=-1;c<=1;++c) for(int d=-1;d<=1;++d)
        if(a*a+b*b+c*c+d*d==2) labels.push_back({a,b,c,d});
    for(unsigned i=0;i<24;++i) for(unsigned a=0;a<4;++a) {
        int value=int(int8_t(bytes[pos++]));
        if(value!=labels[i][a]) throw std::invalid_argument("foreign labels");
        job.labels[i][a]=value;
    }
    for(unsigned i=0;i<N;++i) {
        uint32_t x=read32(bytes,pos);
        if(x>FULL || popcount(x)!=12 || (i && x<=job.eligible.back()))
            throw std::invalid_argument("invalid eligible word ordering");
        for(unsigned a=0;a<4;++a) {
            int momentum=0;
            for(unsigned c=0;c<24;++c) if(x&(1u<<c)) momentum+=labels[c][a];
            if(momentum) throw std::invalid_argument("eligible momentum");
        }
        job.eligible.push_back(x);
    }
    for(unsigned i=0;i<N;++i)
        if(job.eligible[N-1-i]!=(FULL^job.eligible[i])) throw std::invalid_argument("eligible complement");
    std::set<std::array<int,3>> velocity_set;
    for(auto v:labels) velocity_set.insert({v[0],v[1],v[2]});
    std::vector<std::array<int,3>> velocities(velocity_set.begin(),velocity_set.end());
    for(unsigned b=0;b<18;++b) {
        auto& block=job.blocks[b];
        block.input=read32(bytes,pos);block.output=read32(bytes,pos);
        block.m=read32(bytes,pos);block.n=read32(bytes,pos);
        uint32_t in=0,out=0;
        for(unsigned c=0;c<24;++c) {
            auto v=labels[c];
            if(std::array<int,3>{v[0],v[1],v[2]}==velocities[b]) in|=1u<<c;
            if(std::array<int,3>{-v[0],-v[1],-v[2]}==velocities[b]) out|=1u<<c;
        }
        if(block.input!=in || block.output!=out || block.m!=popcount(in) || block.n!=popcount(out))
            throw std::invalid_argument("foreign opposite-velocity blocks");
        uint64_t sum=0;
        for(unsigned j=0;j<16;++j) {
            block.counts[j]=read32(bytes,pos);
            if(block.counts[j]>(1u<<24) || (j>=(1u<<(block.m+block.n)) && block.counts[j]))
                throw std::invalid_argument("joint count padding or alphabet");
            sum+=block.counts[j];
        }
        if(sum!=(1u<<24)) throw std::invalid_argument("joint count sum");
        for(unsigned x=0;x<(1u<<block.m);++x) {
            uint64_t total=0;
            for(unsigned y=0;y<(1u<<block.n);++y) total+=block.counts[x|(y<<block.m)];
            if(total!=(1u<<(24-block.m))) throw std::invalid_argument("incoming marginal");
        }
        for(unsigned y=0;y<(1u<<block.n);++y) {
            uint64_t total=0;
            for(unsigned x=0;x<(1u<<block.m);++x) total+=block.counts[x|(y<<block.m)];
            if(total!=(1u<<(24-block.n))) throw std::invalid_argument("outgoing marginal");
        }
    }
    std::vector<std::array<unsigned,24>> maps;
    std::array<int,3> perm={0,1,2};
    do {
        for(int s0:{-1,1}) for(int s1:{-1,1}) for(int s2:{-1,1}) for(int s3:{-1,1}) {
            std::array<unsigned,24> map;
            for(unsigned c=0;c<24;++c) {
                auto v=labels[c];
                std::array<int,4> image={s0*v[perm[0]],s1*v[perm[1]],s2*v[perm[2]],s3*v[3]};
                map[c]=unsigned(std::find(labels.begin(),labels.end(),image)-labels.begin());
            }
            maps.push_back(map);
        }
    } while(std::next_permutation(perm.begin(),perm.end()));
    std::set<std::pair<unsigned,unsigned>> unseen;
    for(unsigned a=0;a<24;++a) for(unsigned i=0;i<24;++i) unseen.insert({a,i});
    for(unsigned r=0;r<14;++r) {
        auto expected=*unseen.begin();
        unsigned a=read32(bytes,pos),i=read32(bytes,pos);
        if(std::make_pair(a,i)!=expected) throw std::invalid_argument("foreign channel representative");
        job.reps[r]={a,i};
        for(auto map:maps) { unseen.erase({map[a],map[i]});unseen.erase({map[i],map[a]}); }
    }
    if(!unseen.empty() || pos!=bytes.size()) throw std::invalid_argument("incomplete channel orbits");
    job.incoming.resize(N);job.outgoing.resize(N);
    for(unsigned i=0;i<N;++i) for(unsigned b=0;b<18;++b) {
        job.incoming[i][b]=uint8_t(compact(job.eligible[i],job.blocks[b].input));
        job.outgoing[i][b]=uint8_t(compact(job.eligible[i],job.blocks[b].output));
    }
    return job;
}

std::pair<unsigned,unsigned> pair_at(uint64_t index) {
    unsigned lo=0,hi=N/2;
    while(lo+1<hi) {
        unsigned mid=(lo+hi)/2;
        if(uint64_t(mid)*(N+1-mid)<=index) lo=mid; else hi=mid;
    }
    return {lo,unsigned(lo+index-uint64_t(lo)*(N+1-lo))};
}
int spin(uint32_t word,unsigned bit) { return (word&(1u<<bit))?1:-1; }

void emit(uint64_t start,uint64_t stop,uint64_t done,uint64_t ordered,
          const std::array<Signed512,14>& sums,const char* status,const std::string& error="") {
    std::cout<<"{\"schema\":\"strict-full-memory-native-range-1\",\"start\":"<<start
             <<",\"stop\":"<<stop<<",\"completed_stop\":"<<done<<",\"checked\":"<<(done-start)
             <<",\"ordered_pairs\":"<<ordered<<",\"denominator_exponent\":432,\"numerators\":[";
    for(unsigned i=0;i<14;++i) { if(i) std::cout<<",";std::cout<<"\""<<sums[i].decimal()<<"\""; }
    std::cout<<"],\"status\":\""<<status<<"\"";
    if(!error.empty()) {
        std::string clean=error;
        for(auto& c:clean) if(c=='"' || c=='\\' || c<' ') c=' ';
        std::cout<<",\"error\":\""<<clean<<"\"";
    }
    std::cout<<"}"<<std::endl;
}

int arithmetic() {
    Signed512 sum;
    int coefficient;unsigned count;
    while(std::cin>>coefficient>>count) {
        if(count>64 || coefficient<-16 || coefficient>16) throw std::invalid_argument("arithmetic test dimensions");
        UInt512 weight(1);
        for(unsigned i=0;i<count;++i) {
            uint64_t factor;
            if(!(std::cin>>factor) || factor>std::numeric_limits<uint32_t>::max())
                throw std::invalid_argument("arithmetic factor");
            weight.multiply(uint32_t(factor));
        }
        sum.add_product(weight,coefficient);
        std::cout<<sum.decimal()<<std::endl;
    }
    if(!std::cin.eof()) throw std::invalid_argument("arithmetic input");
    return 0;
}

int main(int argc,char** argv) {
    uint64_t start=0,stop=0,done=0,ordered=0;
    std::array<Signed512,14> sums;
    try {
#ifdef _WIN32
        if(_setmode(_fileno(stdin),_O_BINARY)==-1) throw std::runtime_error("binary stdin setup");
#endif
        if(argc==2 && std::string(argv[1])=="--binary-check") {
            std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(std::cin)),{});
            if(bytes.size()>1048576) throw std::invalid_argument("binary test size");
            const char* hex="0123456789abcdef";
            for(auto byte:bytes) std::cout<<hex[byte>>4]<<hex[byte&15];
            std::cout<<std::endl;return 0;
        }
        if(argc==2 && std::string(argv[1])=="--arithmetic") return arithmetic();
        if(argc!=7 || std::string(argv[1])!="--start" || std::string(argv[3])!="--stop"
           || std::string(argv[5])!="--seconds") throw std::invalid_argument("required range arguments");
        start=number(argv[2]);stop=number(argv[4]);done=start;
        size_t used=0;double seconds=std::stod(argv[6],&used);
        if(used!=std::strlen(argv[6]) || !std::isfinite(seconds) || seconds<=0 || seconds>1800
           || start>stop || stop>TOTAL) throw std::invalid_argument("invalid range or deadline");
        auto deadline=Clock::now()+std::chrono::duration<double>(seconds);
        Job job=read_job();
        emit(start,stop,done,ordered,sums,"RUNNING");
        auto pair=start<TOTAL?pair_at(start):std::make_pair(N/2,N/2);
        unsigned i=pair.first,j=pair.second;
        for(;done<stop;++done) {
            if((done-start)%4096==0) {
                if(Clock::now()>=deadline) { emit(start,stop,done,ordered,sums,"TIMEOUT");return 124; }
                if(done!=start) emit(start,stop,done,ordered,sums,"RUNNING");
            }
            UInt512 weight(1);
            for(unsigned b=0;b<18;++b) {
                auto& block=job.blocks[b];
                uint32_t count=block.counts[job.incoming[i][b] | (job.outgoing[j][b]<<block.m)];
                weight.multiply(count);
                if(!count) break;
            }
            bool short_orbit=(i==j || i+j==N-1);
            ordered+=short_orbit?2:4;
            uint32_t x=job.eligible[i],y=job.eligible[j];
            for(unsigned r=0;r<14;++r) {
                auto [a,c]=job.reps[r];
                int feature=8*spin(x,c)*spin(y,a);
                if(!short_orbit) feature+=8*spin(y,c)*spin(x,a);
                sums[r].add_product(weight,feature);
            }
            if(++j>N-1-i) { ++i;j=i; }
        }
        if(Clock::now()>=deadline) { emit(start,stop,done,ordered,sums,"TIMEOUT");return 124; }
        emit(start,stop,done,ordered,sums,"PASS");
        return 0;
    } catch(const std::exception& error) {
        emit(start,stop,done,ordered,sums,"FAIL",error.what());
        std::cerr<<error.what()<<std::endl;
        return 2;
    }
}
