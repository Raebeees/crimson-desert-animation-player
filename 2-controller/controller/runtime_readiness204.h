#pragma once
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#include <filesystem>
#include <memory>
#include "runtime_assets204.h"
#pragma comment(lib,"bcrypt.lib")
namespace boots204 {
struct LockedFile204 {HANDLE h=INVALID_HANDLE_VALUE;std::string bytes;~LockedFile204(){if(h!=INVALID_HANDLE_VALUE)CloseHandle(h);}};
class LockedReader204 {
 std::map<std::string,std::unique_ptr<LockedFile204>> files;
 std::size_t total=0;ULONGLONG started=GetTickCount64();
public:
 bool read(const std::string& path,const std::string*& output) {
  output=nullptr;if(GetTickCount64()-started>10000)return false;
  auto found=files.find(path);if(found!=files.end()){output=&found->second->bytes;return true;}
  // Local drive absolute paths only. Reject device/UNC/relative routing.
  if(path.size()<4||path[1]!=':'||(path[2]!='/'&&path[2]!='\\')||path.find('\0')!=std::string::npos)return false;
  if(files.size()>=96)return false;
  const auto wide=std::filesystem::u8path(path).wstring();auto f=std::make_unique<LockedFile204>();
  f->h=CreateFileW(wide.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(f->h==INVALID_HANDLE_VALUE)return false;
  LARGE_INTEGER size{};if(!GetFileSizeEx(f->h,&size)||size.QuadPart<0||size.QuadPart>32*1024*1024)return false;
  const auto length=static_cast<std::size_t>(size.QuadPart);if(total+length>256*1024*1024)return false;
  f->bytes.resize(length);std::size_t pos=0;
  while(pos<length){if(GetTickCount64()-started>10000)return false;DWORD got=0,want=static_cast<DWORD>((length-pos)>65536?65536:(length-pos));
   if(!ReadFile(f->h,f->bytes.data()+pos,want,&got,nullptr)||got!=want)return false;pos+=got;}
  total+=length;output=&f->bytes;files.emplace(path,std::move(f));return true;
 }
 bool hash(const std::string& path,std::string& out) {
  out.clear();const std::string* b=nullptr;if(!read(path,b))return false;
  BCRYPT_ALG_HANDLE a=nullptr;BCRYPT_HASH_HANDLE h=nullptr;unsigned char digest[32]{};bool ok=false;
  if(BCryptOpenAlgorithmProvider(&a,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0&&BCryptCreateHash(a,&h,nullptr,0,nullptr,0,0)>=0)
   ok=BCryptHashData(h,reinterpret_cast<PUCHAR>(const_cast<char*>(b->data())),static_cast<ULONG>(b->size()),0)>=0&&BCryptFinishHash(h,digest,32,0)>=0;
  if(h)BCryptDestroyHash(h);if(a)BCryptCloseAlgorithmProvider(a,0);
  if(!ok)return false;constexpr char hex[]="0123456789abcdef";for(auto v:digest){out+=hex[v>>4];out+=hex[v&15];}return true;
 }
};
struct RuntimeRecord204 {std::string body,header;std::uint32_t key;std::string bytes;};
inline std::uint32_t indexU32(const std::string& b,std::size_t p){return static_cast<unsigned char>(b[p])|(static_cast<std::uint32_t>(static_cast<unsigned char>(b[p+1]))<<8)|(static_cast<std::uint32_t>(static_cast<unsigned char>(b[p+2]))<<16)|(static_cast<std::uint32_t>(static_cast<unsigned char>(b[p+3]))<<24);}
inline bool privateRecord204(const std::string& body,const std::string& head,const RuntimeRecord204& expected) {
 if(head.size()<2||expected.bytes.empty())return false;
 const auto count=static_cast<unsigned char>(head[0])+(static_cast<unsigned>(static_cast<unsigned char>(head[1]))<<8);
 if(head.size()!=2+static_cast<std::size_t>(count)*8)return false;
 std::map<std::uint32_t,std::uint32_t> offsets;std::map<std::uint32_t,bool> starts;
 for(unsigned i=0;i<count;++i){const auto key=indexU32(head,2+i*8),at=indexU32(head,6+i*8);
  if(at>=body.size()||!offsets.emplace(key,at).second||!starts.emplace(at,true).second)return false;}
 auto it=offsets.find(expected.key);if(it==offsets.end())return false;
 const std::size_t at=it->second;auto next=starts.upper_bound(it->second);const auto end=next==starts.end()?body.size():next->first;
 return end-at==expected.bytes.size()&&body.compare(at,expected.bytes.size(),expected.bytes)==0;
}
// Keep this lease alive from verification through hook activation and use.
// Its open handles prevent DMM replacement while readiness is being relied on.
// Module identity and hook ownership must still be independently checked.
class RuntimeReadinessLease204 {
 LockedReader204 reader;bool attempted=false,accepted=false;
public:
 RuntimeReadinessLease204()=default;
 RuntimeReadinessLease204(const RuntimeReadinessLease204&)=delete;
 RuntimeReadinessLease204& operator=(const RuntimeReadinessLease204&)=delete;
 bool ready()const noexcept{return accepted;}
 bool verify(const std::string& manifest,const std::string& build,const std::vector<RuntimeAsset204>& assets,const std::vector<RuntimeRecord204>& records) noexcept {
 if(attempted)return false;attempted=true;
 try {
  if(records.empty())return false;const std::string* text=nullptr;
  if(!reader.read(manifest,text))return false;std::map<std::string,std::string> routes;
  if(!runtimeRoutes204(*text,build,routes)||!runtimeAssets204(routes,assets,reader))return false;
  std::map<std::pair<std::string,std::uint32_t>,bool> seen;
  for(const auto& record:records){std::string b,h;
   if(!assetKey204(record.body,b)||!assetKey204(record.header,h)||!seen.emplace(std::make_pair(b,record.key),true).second)return false;
   auto bi=routes.find(b),hi=routes.find(h);if(bi==routes.end()||hi==routes.end())return false;
   const std::string *body=nullptr,*head=nullptr;
   if(!reader.read(bi->second,body)||!reader.read(hi->second,head)||!privateRecord204(*body,*head,record))return false;
  }accepted=true;return true; // Handles remain owned by this lease.
 }catch(...){return false;}
}
};
inline bool runtimeReadiness204(const std::string& manifest,const std::string& build,const std::vector<RuntimeAsset204>& assets,const std::vector<RuntimeRecord204>& records) noexcept {RuntimeReadinessLease204 lease;return lease.verify(manifest,build,assets,records);}
} // namespace boots204
