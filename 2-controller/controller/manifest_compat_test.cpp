#include "runtime_assets204.h"
#include <cassert>
#include <iostream>
int main(){
 using namespace boots204;std::map<std::string,std::string> r;
 const std::string head="dmm-runtime\t1\nbuild\ttest\nfile\ta/b.paa\tC:/asset.paa\n";
 assert(runtimeRoutes204(head,"build\ttest",r));
 const std::string cache="pathc\tmeta/0.pathc\tC:/cache/0.pathc\n";
 assert(runtimeRoutes204(head+cache,"build\ttest",r)&&r.size()==1);
 assert(runtimeRoutes204(head+cache+"pathc\tdlc/dlc_01/meta/0.pathc\tC:/cache/1.pathc\n","build\ttest",r));
 assert(!runtimeRoutes204(head+cache+cache,"build\ttest",r));
 assert(!runtimeRoutes204(head+"pathc\tother\tC:/x\n","build\ttest",r));
 assert(!runtimeRoutes204(head+"pathc\tmeta/0.pathc\trelative\n","build\ttest",r));
 assert(!runtimeRoutes204(head+"unknown\tx\ty\n","build\ttest",r));
 assert(!runtimeRoutes204(head+cache,"build\twrong",r));
 std::cout<<"Manifest path-cache compatibility tests passed\n";
}
