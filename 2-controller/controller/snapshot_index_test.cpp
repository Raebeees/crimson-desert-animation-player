#include "resolver204.h"
#include "legacy_resolver_test.h"
#include <vector>
#include <utility>
#include <random>
#include <chrono>
#include <iostream>
#include <stdexcept>
using P=std::uintptr_t;
unsigned checks=0;
void check(bool ok){++checks;if(!ok)throw std::runtime_error("differential mismatch");}
struct Reader {
    std::vector<std::pair<P,std::size_t>> trace;
    unsigned mutate=0,fail=0;
    bool read(P p,void* out,std::size_t n){
        trace.emplace_back(p,n);
        if(trace.size()==fail)return false;
        auto value=static_cast<std::uint64_t>(p*0x9e3779b97f4a7c15ULL);
        if(mutate && trace.size()>=mutate)value^=1;
        std::memcpy(out,&value,n);return true;
    }
};
template<class S> bool readSize(S& s,P p,unsigned size,std::uint64_t& value){
    if(size==1){std::uint8_t v=91;auto b=s.read(p,v);value=v;return b;}
    if(size==2){std::uint16_t v=91;auto b=s.read(p,v);value=v;return b;}
    if(size==4){std::uint32_t v=91;auto b=s.read(p,v);value=v;return b;}
    value=91;return s.read(p,value);
}
void scenario(const std::vector<std::pair<P,unsigned>>& ops,unsigned fail,unsigned mutate,bool stable){
    Reader a,b;a.fail=b.fail=fail;a.mutate=b.mutate=mutate;
    legacy204::Snapshot<Reader> old(a);boots204::Snapshot<Reader> now(b);
    for(const auto& op:ops){
        std::uint64_t x=0,y=0;const bool bx=readSize(old,op.first,op.second,x),by=readSize(now,op.first,op.second,y);
        check(bx==by && x==y && int(old.failure)==int(now.failure) && old.reads()==now.reads());
    }
    if(stable)check(old.stable()==now.stable());
    check(int(old.failure)==int(now.failure) && old.reads()==now.reads() && a.trace==b.trace);
}
// Benchmark isolates journal CPU work; no game or kernel-memory timing claim.
struct FastReader {
    __declspec(noinline) bool read(P p,void* out,std::size_t n){auto v=static_cast<std::uint64_t>(p*17);std::memcpy(out,&v,n);return true;}
};
volatile std::uint64_t sink=0;
template<template<class> class S> double bench(unsigned count){
    FastReader r;auto start=std::chrono::steady_clock::now();std::uint64_t sum=0;
    for(unsigned round=0;round<20000;++round){
        S<FastReader> s(r);std::uint64_t v=0;
        for(unsigned i=0;i<count;++i){s.read(0x100000+P(i)*24,v);sum+=v;}
        for(unsigned i=0;i<count;++i){s.read(0x100000+P(i)*24,v);sum+=v;}
        sum+=s.stable();
    }
    sink=sum;return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
}
int main(){
    std::vector<std::pair<P,unsigned>> ops;
    for(unsigned i=0;i<390;++i)ops.emplace_back(0x100000+P(i)*8,8);
    scenario(ops,0,0,true); // full journal and overflow
    ops.assign(800,{0x100000,8});scenario(ops,0,0,true); // raw-read limit
    ops={{0,8},{0xffff,1},{0x7fffffffffffULL,8},{0x100000,1},{0x100000,2},{0x100000,4},{0x100000,8}};
    scenario(ops,0,0,true); // same address, distinct sizes; invalid bounds
    ops.clear();for(unsigned i=0;i<100;++i)ops.emplace_back(0x100000+P(i)*8,8);
    for(unsigned i=1;i<=200;++i){scenario(ops,i,0,true);scenario(ops,0,i,true);}
    // Force a collision chain rather than relying on random coverage.
    ops.clear();for(P p=0x100000;ops.size()<384;++p){auto k=p^(8ULL*0x9e3779b97f4a7c15ULL);k^=k>>33;k*=0xff51afd7ed558ccdULL;k^=k>>33;if((k&1023u)==0)ops.emplace_back(p,8);}
    scenario(ops,0,0,true);
    std::mt19937 rng(20410);
    for(unsigned run=0;run<400;++run){
        ops.clear();for(unsigned i=0;i<500;++i)ops.emplace_back(0x100000+P(rng()%150)*8,1u<<(rng()%4));
        scenario(ops,run%3==0?1+rng()%500:0,run%3==1?1+rng()%500:0,true);
    }
    std::cout<<"Differential checks passed: "<<checks<<"; identical read order, results, failures and budgets\n";
    for(unsigned n:{6u,32u,96u,192u,256u}){
        const auto a=bench<legacy204::Snapshot>(n),b=bench<boots204::Snapshot>(n);
        std::cout<<"journal entries="<<n<<" R9_ms="<<a<<" R10_ms="<<b<<" ratio="<<b/a<<"\n";
    }
}
