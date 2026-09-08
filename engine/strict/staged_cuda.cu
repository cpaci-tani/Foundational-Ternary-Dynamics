// Exact research candidate kernels. Physical schedule is four CUDA launches,
// one launch per elapsed global microtick; host extraction only observes events.
#include "staged_cuda.h"
#include "frozen_tables.h"
#include <cuda_runtime.h>
#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace ftd::strict::gpu {
namespace {
void check(cudaError_t result, const char* operation) {
    if (result != cudaSuccess)
        throw std::runtime_error(std::string(operation) + ": " + cudaGetErrorString(result));
}
struct DeviceBuffer {
    void* pointer = nullptr;
    explicit DeviceBuffer(std::size_t size) { check(cudaMalloc(&pointer, size), "cudaMalloc"); }
    ~DeviceBuffer() { if (pointer) cudaFree(pointer); }
    DeviceBuffer(const DeviceBuffer&) = delete;
    DeviceBuffer& operator=(const DeviceBuffer&) = delete;
};
struct Tables {
    std::uint16_t collision[3 * 18336], U[384], half_turn[384];
    std::uint8_t channel_phase[384], rotate[9], absorbed[2], blank;
    std::int8_t tangent[384 * 3], phase[9], eps[9];
};
Tables host_tables() {
    Tables t{};
    for (unsigned i = 0; i < 3; ++i)
        std::copy_n(frozen::COLLISION[i], 18336, t.collision + i * 18336);
    std::copy_n(frozen::U, 384, t.U);
    std::copy_n(frozen::HALF_TURN, 384, t.half_turn);
    std::copy_n(frozen::CHANNEL_PHASE, 384, t.channel_phase);
    std::copy_n(frozen::TANGENT, 1152, t.tangent);
    std::copy_n(frozen::ROTATE, 9, t.rotate);
    std::copy_n(frozen::PHASE, 9, t.phase);
    std::copy_n(frozen::EPS, 9, t.eps);
    std::copy_n(frozen::ABSORBED_RESERVE, 2, t.absorbed);
    t.blank = frozen::BLANK;
    return t;
}
struct View {
    std::int8_t* s;
    std::uint8_t *ell, *bank, *sc, *fcc, *admitted, *gsc, *gfcc;
    __device__ View(std::uint8_t* bytes, std::size_t n)
        : s(reinterpret_cast<std::int8_t*>(bytes)), ell(bytes+n), bank(bytes+2*n),
          sc(bytes+386*n), fcc(bytes+392*n), admitted(bytes+404*n),
          gsc(bytes+407*n), gfcc(bytes+410*n) {}
};
__host__ __device__ std::size_t shifted(std::size_t i, unsigned L, unsigned axis, int sign) {
    std::size_t stride = axis == 0 ? std::size_t(L)*L : (axis == 1 ? L : 1);
    unsigned coordinate = unsigned((i / stride) % L);
    if (sign > 0) return coordinate + 1 == L ? i - stride*(L-1) : i + stride;
    return coordinate == 0 ? i + stride*(L-1) : i - stride;
}
__device__ int population(const View& input, std::size_t x) {
    int sum = 0;
    for (unsigned c = 0; c < 384; ++c) sum += input.bank[x*384+c];
    return sum;
}
__device__ void plane(unsigned p, unsigned& a, unsigned& b) {
    a = p == 0 ? 1 : 0;
    b = p == 2 ? 1 : 2;
}
__global__ void admission_kernel(std::uint8_t* source, std::uint8_t* dest,
                                  std::size_t n, unsigned L, const Tables* t) {
    std::size_t i = std::size_t(blockIdx.x)*blockDim.x + threadIdx.x;
    if (i >= n) return;
    View in(source,n), out(dest,n);
    int own_population = population(in,i);
    for (unsigned a = 0; a < 3; ++a) {
        const auto head = shifted(i,L,a,1);
        out.gsc[i*3+a] = (own_population + population(in,head)) % 2 == 0;
        if (in.sc[i*6+a*2] != t->blank || in.sc[i*6+a*2+1] != t->blank) continue;
        unsigned count=0, selected_channel=0;
        std::size_t selected_site=0;
        for (unsigned c=0;c<384;++c) {
            if (t->channel_phase[c] != 2) continue;
            if (t->tangent[c*3+a] == 1 && in.bank[i*384+c]) {
                ++count; selected_channel=c; selected_site=i;
            }
            if (t->tangent[c*3+a] == -1 && in.bank[head*384+c]) {
                ++count; selected_channel=c; selected_site=head;
            }
        }
        if (count==1) {
            // Each channel targets exactly one edge: exclusive bank-byte writer.
            out.bank[selected_site*384+selected_channel]=0;
            out.sc[i*6+a*2]=t->blank;
            out.sc[i*6+a*2+1]=t->absorbed[selected_channel>=192];
            out.admitted[i*3+a]=1;
        }
    }
    for (unsigned p=0;p<3;++p) {
        unsigned a,b;plane(p,a,b);
        auto ia=shifted(i,L,a,1), ib=shifted(i,L,b,1);
        out.gfcc[i*6+p*2]=(own_population+population(in,shifted(ia,L,b,1)))%2==0;
        out.gfcc[i*6+p*2+1]=(population(in,ia)+population(in,ib))%2==0;
    }
}
__device__ void relation(const std::uint8_t* in,std::uint8_t* out,bool gate,const Tables* t) {
    auto l=in[0],r=in[1];
    bool lo=l!=t->blank,ro=r!=t->blank;
    if (lo!=ro && t->phase[lo?l:r]==0 && gate) {
        out[0]=t->rotate[r];out[1]=t->rotate[l];
    } else {out[0]=t->rotate[l];out[1]=t->rotate[r];}
}
__global__ void collision_relation_kernel(std::uint8_t* source,std::uint8_t* dest,
                                           std::size_t n,const Tables* t) {
    std::size_t i=std::size_t(blockIdx.x)*blockDim.x+threadIdx.x;
    if(i>=n)return;
    View in(source,n),out(dest,n);
    for(unsigned polarity=0;polarity<2;++polarity) {
        unsigned first=0,second=0,count=0;
        for(unsigned c=0;c<192;++c) if(in.bank[i*384+polarity*192+c]) {
            if(count==0)first=c;else if(count==1)second=c;++count;
        }
        if(count==2) {
            unsigned rank=first*(383-first)/2+second-first-1;
            unsigned encoded=t->collision[in.ell[i]*18336+rank];
            auto row=out.bank+i*384+polarity*192;
            row[first]=row[second]=0;row[encoded/192]=row[encoded%192]=1;
        }
    }
    out.ell[i]=(in.ell[i]+2)%3;
    for(unsigned a=0;a<3;++a) if(!in.admitted[i*3+a])
        relation(in.sc+i*6+a*2,out.sc+i*6+a*2,in.gsc[i*3+a],t);
    for(unsigned j=0;j<6;++j)
        relation(in.fcc+i*12+j*2,out.fcc+i*12+j*2,in.gfcc[i*6+j],t);
}
__global__ void streaming_kernel(std::uint8_t* source,std::uint8_t* dest,
                                  std::size_t n,unsigned L,const Tables* t) {
    std::size_t i=std::size_t(blockIdx.x)*blockDim.x+threadIdx.x;
    if(i>=n)return;
    View in(source,n),out(dest,n);
    for(unsigned c=0;c<384;++c) if(in.bank[i*384+c]) {
        unsigned axis=0;while(t->tangent[c*3+axis]==0)++axis;
        auto y=shifted(i,L,axis,t->tangent[c*3+axis]);
        auto channel=t->U[c];
        if(in.s[i]!=0)channel=t->half_turn[channel];
        // Frozen channel map is injective including source direction. No atomics.
        out.bank[y*384+channel]=1;
    }
}
__global__ void manifestation_kernel(std::uint8_t* source,std::uint8_t* dest,
                                      std::size_t n,unsigned L,const Tables* t) {
    std::size_t i=std::size_t(blockIdx.x)*blockDim.x+threadIdx.x;
    if(i>=n)return;
    View in(source,n),out(dest,n);
    int q=0;
    for(unsigned a=0;a<3;++a)
        q+=t->eps[in.sc[i*6+a*2]]-t->eps[in.sc[shifted(i,L,a,-1)*6+a*2]];
    for(unsigned p=0;p<3;++p) {
        unsigned a,b;plane(p,a,b);
        auto ma=shifted(i,L,a,-1),mb=shifted(i,L,b,-1);
        q+=t->eps[in.fcc[i*12+p*4]]-t->eps[in.fcc[shifted(ma,L,b,-1)*12+p*4]];
        q+=t->eps[in.fcc[ma*12+p*4+2]]-t->eps[in.fcc[mb*12+p*4+2]];
    }
    out.s[i]=std::int8_t(((q+1)%3+3)%3-1);
    for(unsigned a=0;a<3;++a)out.admitted[i*3+a]=out.gsc[i*3+a]=0;
    for(unsigned j=0;j<6;++j)out.gfcc[i*6+j]=0;
}

// Deterministic forensic extraction from the actual before/after device states.
// This does not compute or repair the next state on the host.
Events observed_events(const State& before,const State& after) {
    Events ev;
    const auto n=before.s.size();
    if(before.phase()==0) {
        for(std::size_t x=0;x<n;++x) for(unsigned c=0;c<384;++c)
            if(before.bank[x*384+c]&&!after.bank[x*384+c]) {
                unsigned a=0;while(frozen::TANGENT[c*3+a]==0)++a;
                auto owner=frozen::TANGENT[c*3+a]>0?x:shifted(x,before.L,a,-1);
                ev.absorptions.push_back({x,c,owner,a});
            }
    } else if(before.phase()==1) {
        for(std::size_t x=0;x<n;++x) {
            for(unsigned p=0;p<2;++p) {
                std::vector<unsigned> old,newer;
                for(unsigned c=0;c<192;++c) {
                    if(before.bank[x*384+p*192+c])old.push_back(c);
                    if(after.bank[x*384+p*192+c])newer.push_back(c);
                }
                if(old.size()==2) {
                    if(newer.size()!=2)throw std::runtime_error("CUDA collision cardinality mismatch");
                    ev.collisions.push_back({x,p?-1:1,{old[0],old[1]},{newer[0],newer[1]}});
                }
            }
            for(unsigned kind=0;kind<2;++kind)for(unsigned j=0;j<(kind?6u:3u);++j) {
                if(!kind&&before.admitted_sc[x*3+j])continue;
                const auto& old=kind?before.fcc:before.sc;
                const auto& newer=kind?after.fcc:after.sc;
                auto index=x*(kind?12:6)+j*2;
                auto l=old[index],r=old[index+1];
                bool lo=l!=frozen::BLANK,ro=r!=frozen::BLANK,newlo=newer[index]!=frozen::BLANK;
                int direction=lo&&!newlo?1:(!lo&&newlo?-1:0);
                RelationEvent event{bool(kind),x,kind?j/2:j,kind?j%2:0,direction};
                if(direction)ev.crossings.push_back(event);
                bool gate=kind?before.gate_fcc[x*6+j]:before.gate_sc[x*3+j];
                if(lo!=ro&&frozen::PHASE[lo?l:r]==0&&!gate)ev.gate_holds.push_back(event);
            }
        }
    }
    return ev;
}
} // namespace

std::string device_json() {
    int device=0;check(cudaGetDevice(&device),"cudaGetDevice");
    cudaDeviceProp properties{};check(cudaGetDeviceProperties(&properties,device),"cudaGetDeviceProperties");
    std::ostringstream out;
    out<<"{\"backend\":\"cuda_device_kernels\",\"name\":\""<<properties.name
       <<"\",\"compute_major\":"<<properties.major<<",\"compute_minor\":"<<properties.minor
       <<",\"device\":"<<device<<",\"canonical_adoption\":false}";
    return out.str();
}

std::vector<Events> advance(State& state,std::uint64_t ticks) {
    validate(state);
    if(ticks>std::numeric_limits<std::uint64_t>::max()-state.microtick)
        throw std::overflow_error("CUDA microtick overflow");
    if(!ticks)return {};
    auto bytes=encode(state);
    const auto n=state.s.size(),size=n*BYTES_PER_SITE;
    constexpr unsigned threads=128;
    const auto required_blocks=n/threads+(n%threads!=0);
    int device=0;
    cudaDeviceProp properties{};
    check(cudaGetDevice(&device),"cudaGetDevice");
    check(cudaGetDeviceProperties(&properties,device),"cudaGetDeviceProperties");
    if(required_blocks>std::numeric_limits<unsigned>::max()
       ||required_blocks>std::size_t(properties.maxGridSize[0]))
        throw std::overflow_error("CUDA state exceeds supported finite launch grid");
    const auto blocks=static_cast<unsigned>(required_blocks);
    DeviceBuffer first(size),second(size),table_buffer(sizeof(Tables));
    auto tables=host_tables();
    check(cudaMemcpy(table_buffer.pointer,&tables,sizeof(Tables),cudaMemcpyHostToDevice),"upload tables");
    check(cudaMemcpy(first.pointer,bytes.data()+HEADER_BYTES,size,cudaMemcpyHostToDevice),"upload state");
    auto input=static_cast<std::uint8_t*>(first.pointer),output=static_cast<std::uint8_t*>(second.pointer);
    auto device_tables=static_cast<const Tables*>(table_buffer.pointer);
    State current=state;
    const auto work=work_units(state);
    std::vector<Events> logs;
    for(std::uint64_t tick=0;tick<ticks;++tick) {
        check(cudaMemcpy(output,input,size,cudaMemcpyDeviceToDevice),"copy immutable pre-state");
        switch(current.phase()) {
        case 0:admission_kernel<<<blocks,threads>>>(input,output,n,state.L,device_tables);break;
        case 1:collision_relation_kernel<<<blocks,threads>>>(input,output,n,device_tables);break;
        case 2:
            check(cudaMemset(output+2*n,0,384*n),"clear streaming destination");
            streaming_kernel<<<blocks,threads>>>(input,output,n,state.L,device_tables);break;
        default:manifestation_kernel<<<blocks,threads>>>(input,output,n,state.L,device_tables);break;
        }
        check(cudaGetLastError(),"launch staged kernel");
        check(cudaDeviceSynchronize(),"complete staged physical tick");
        check(cudaMemcpy(bytes.data()+HEADER_BYTES,output,size,cudaMemcpyDeviceToHost),"download actual state");
        auto clock=current.microtick+1;
        for(unsigned i=0;i<8;++i)bytes[12+i]=std::uint8_t(clock>>(8*i));
        State next=decode(bytes);
        if(work_units(next)!=work)throw std::runtime_error("CUDA work conservation failure");
        logs.push_back(observed_events(current,next));
        current=std::move(next);
        std::swap(input,output);
    }
    state=std::move(current); // Single commit after all device and validation work.
    return logs;
}
StepResult step(const State& state) {
    State copy=state;
    auto logs=ftd::strict::gpu::advance(copy,1);
    return {std::move(copy),std::move(logs.front())};
}
} // namespace ftd::strict::gpu
