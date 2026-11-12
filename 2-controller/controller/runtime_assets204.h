#pragma once
#include <string>
#include <map>
#include <vector>
#include <sstream>
namespace boots204 {
struct RuntimeAsset204 {std::string path,sha256;};
inline bool assetKey204(std::string value,std::string& result) {
 result.clear();if(value.empty()||value.size()>1024)return false;
 for(char& c:value){if(c=='\\')c='/';if(c>='A'&&c<='Z')c=static_cast<char>(c-'A'+'a');if(static_cast<unsigned char>(c)<32||c==':')return false;}
 if(value.front()=='/'||value.back()=='/')return false;
 std::istringstream s(value);std::string part;while(std::getline(s,part,'/'))if(part.empty()||part=="."||part=="..")return false;
 result=value;return true;
}
inline bool runtimeRoutes204(const std::string& text,const std::string& expectedBuild,std::map<std::string,std::string>& routes) {
 routes.clear();if(text.size()>16*1024*1024||expectedBuild.empty())return false;
 std::map<std::string,bool> pathcSeen;bool version=false,build=false;std::istringstream input(text);std::string line;
 while(std::getline(input,line)) {
  if(!line.empty()&&line.back()=='\r')line.pop_back();if(line.empty()||line[0]=='#')continue;
  if(line=="dmm-runtime\t1"){if(version)return false;version=true;continue;}
  if(line.rfind("build\t",0)==0){if(build||line!=expectedBuild)return false;build=true;continue;}
  if(line.rfind("pathc\t",0)==0){
   const auto split=line.find('\t',6);if(split==std::string::npos||line.find('\t',split+1)!=std::string::npos)return false;
   const auto key=line.substr(6,split-6),path=line.substr(split+1);
   if((key!="meta/0.pathc"&&key!="dlc/dlc_01/meta/0.pathc")||!pathcSeen.emplace(key,true).second)return false;
   if(path.size()<4||path[1]!=':'||(path[2]!='/'&&path[2]!='\\')||path.find('\0')!=std::string::npos)return false;
   continue; // DMM path-cache metadata; not a loose asset override.
  }
  if(line.rfind("file\t",0)!=0)return false;
  const auto split=line.find('\t',5);if(split==std::string::npos||line.find('\t',split+1)!=std::string::npos)return false;
  std::string key;if(!assetKey204(line.substr(5,split-5),key))return false;
  const auto path=line.substr(split+1);if(path.empty()||path.find('\0')!=std::string::npos)return false;
  if(!routes.emplace(key,path).second)return false;
 }
 return version&&build;
}
// Reader.hash(path,out) must perform bounded reads and reject file changes.
// Caller separately pins module identities and compares manifest before/after.
template<class Reader> bool runtimeAssets204(const std::map<std::string,std::string>& routes,const std::vector<RuntimeAsset204>& expected,Reader& reader) {
 if(expected.empty())return false;std::map<std::string,bool> seen;
 for(const auto& e:expected){std::string key;if(!assetKey204(e.path,key)||!seen.emplace(key,true).second||e.sha256.size()!=64)return false;
  for(char c:e.sha256)if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))return false;
  auto it=routes.find(key);if(it==routes.end())return false;std::string actual;
  if(!reader.hash(it->second,actual)||actual!=e.sha256)return false;
 }return true;
}
} // namespace boots204
