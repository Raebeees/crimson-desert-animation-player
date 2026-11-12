#include "expected_runtime204.h"
#include <iostream>
int main(){using namespace boots204;unsigned n=0;auto check=[&](bool b){++n;if(!b)throw n;};
const std::string manifest="C:\\Users\\Administrator\\Documents\\ChatGPT\\cd\\outputs\\migration-204-20261030-183747\\build-26361232-review\\runtime-readiness-r1\\fixture.tsv",build="build\toffline-fixture-only\t1";auto a=expectedAssets204();auto r=expectedRecords204();
check(runtimeReadiness204(manifest,build,a,r));check(!runtimeReadiness204(manifest,"build\twrong\t1",a,r));
auto bad=a;bad[0].sha256[0]=bad[0].sha256[0]=='a'?'b':'a';check(!runtimeReadiness204(manifest,build,bad,r));
auto wrong=r;wrong[0].bytes[0]^=1;check(!runtimeReadiness204(manifest,build,a,wrong));wrong=r;wrong[0].key=1;check(!runtimeReadiness204(manifest,build,a,wrong));wrong=r;wrong.push_back(r[0]);check(!runtimeReadiness204(manifest,build,a,wrong));check(!runtimeReadiness204(manifest,build,a,{}));
RuntimeReadinessLease204 lease;check(!lease.ready());check(lease.verify(manifest,build,a,r));check(lease.ready());check(!lease.verify(manifest,build,a,r));
for(const auto& e:a){auto badEach=a;for(auto& item:badEach)if(item.path==e.path)item.sha256[0]=item.sha256[0]=='a'?'b':'a';check(!runtimeReadiness204(manifest,build,badEach,r));}
for(std::size_t i=0;i<r.size();++i){auto badEach=r;badEach[i].bytes[0]^=1;check(!runtimeReadiness204(manifest,build,a,badEach));}
LockedReader204 reader;const std::string* bytes=nullptr;check(!reader.read("relative",bytes));check(reader.read(manifest,bytes));
HANDLE writer=CreateFileW(std::filesystem::u8path(manifest).c_str(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr);check(writer==INVALID_HANDLE_VALUE);if(writer!=INVALID_HANDLE_VALUE)CloseHandle(writer);
std::cout<<"runtime readiness checks passed: "<<n<<" assertions\n";
}
