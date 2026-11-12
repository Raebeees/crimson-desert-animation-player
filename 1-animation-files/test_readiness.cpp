#include "expected_runtime204.h"
#include <iostream>
int main(int argc,char** argv){
 if(argc!=3)return 2;
 constexpr char build[]="build\t0200040008000100000006000000646c635f303107000000973ad5e4\t326872984";
 if(!boots204::runtimeReadiness204(argv[1],build,expectedAssets204(),expectedRecords204())){std::cerr<<"joint candidate readiness failed\n";return 1;}
 if(boots204::runtimeReadiness204(argv[2],build,expectedAssets204(),expectedRecords204())){std::cerr<<"unmerged installation wrongly accepted\n";return 1;}
 std::cout<<"PASS: joint staged manifest meets all "<<expectedAssets204().size()<<" asset hashes and "<<expectedRecords204().size()<<" private record checks; old manifest correctly rejected\n";
 return 0;
}
