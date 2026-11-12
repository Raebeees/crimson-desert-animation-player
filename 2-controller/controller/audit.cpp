#include "expected_runtime204.h"
#include <iostream>
int wmain(int argc,wchar_t** argv){
    if(argc!=2)return 1;
    std::string path=std::filesystem::path(argv[1]).u8string();
    const std::string build="build\t0200040008000100000006000000646c635f303107000000973ad5e4\t326872984";
    auto assets=expectedAssets204();auto records=expectedRecords204();
    boots204::LockedReader204 reader;
    std::map<std::string,std::string> routes;const std::string* manifest=nullptr;
    if(!reader.read(path,manifest)||!boots204::runtimeRoutes204(*manifest,build,routes))return 2;
    unsigned matched=0,missing=0,different=0;
    for(const auto& asset:assets){
        auto found=routes.find(asset.path);std::string hash;
        if(found==routes.end()){++missing;std::cout<<"Missing "<<asset.path<<"\n";}
        else if(!reader.hash(found->second,hash)||hash!=asset.sha256){++different;std::cout<<"Different/unreadable "<<asset.path<<"\n";}
        else ++matched;
    }
    unsigned passing=0;
    for(const auto& record:records){
        auto b=routes.find(record.body),h=routes.find(record.header);
        const std::string *body=nullptr,*header=nullptr;
        if(b!=routes.end()&&h!=routes.end()&&reader.read(b->second,body)&&reader.read(h->second,header)&&
           boots204::privateRecord204(*body,*header,record))++passing;
        else std::cout<<"Unverified record "<<record.key<<" "<<record.body<<"\n";
    }
    boots204::RuntimeReadinessLease204 lease;
    const bool ready=lease.verify(path,build,assets,records);
    std::cout<<"Assets matched "<<matched<<", missing "<<missing<<", different/unreadable "<<different
             <<". Records matched "<<passing<<"/"<<records.size()<<". Lease ready "<<ready<<"\n";
    return ready==(matched==assets.size()&&passing==records.size())?0:3;
}
