#pragma once
#include "flight_gate204.h"
#include "condition_calls204.h"
#include "skill_samples204.h"
namespace boots204 {
inline ConditionDecision existingSkillDecision204(bool ownedEquippedReady,
 const std::array<std::uint32_t,5>& words,Word native) {
 if(!ownedEquippedReady || (native&255)>1)return {};
 bool exact=false;for(const auto& sample:skillSamples204)if(sample==words){exact=true;break;}
 if(!exact)return {};
 switch(words[1]){
 case 10235:case 10244:case 10245:case 10246:case 15066:return {true,0};
 case 15048:case 15029:case 15045:case 10264:case 15046:return {true,1};
 default:return {};
 }
}
// Post-native observation: ownership/equipment/descriptor share one journal.
// This does not replace pre-native private gate handling.
template<class Reader> ConditionScope inspectExistingSkill204(Reader& reader,Word base,Word cc,
 const ConditionArguments& args) {
 ConditionScope scope{};Snapshot<Reader> journal(reader);IdentityEquipment owner;
 if(!readIdentityEquipment(journal,base,cc,owner) || owner.boots!=EquipmentPresence::present)return scope;
 if(args.a<0x10000 || args.a>=0x800000000000ULL-16 || args.a%8 ||
    args.b<0x10000 || args.b>=0x800000000000ULL-8 || args.b%8)return scope;
 Word actor=0,descriptor=0;
 if(!journal.pointer(args.a+8,actor) || actor!=owner.actor || !journal.read(args.b,descriptor) || descriptor<0x10000 || descriptor>0x800000000000ULL-20)return scope;
 std::array<std::uint32_t,5> words{};
 if(!journal.read(descriptor,words[0]) || words[0]!=238 || !journal.read(descriptor+4,words[1]))return scope;
 bool known=false;for(const auto& sample:skillSamples204)if(sample[1]==words[1]){known=true;break;}
 if(!known)return scope;
 for(unsigned i=2;i<5;++i)if(!journal.read(descriptor+i*4,words[i]))return scope;
 bool exact=false;for(const auto& sample:skillSamples204)if(sample==words){exact=true;break;}
 if(!exact || !journal.stable())return scope;
 scope.owned=true;scope.descriptor_exact=true;scope.words=words;
 // Caller must independently establish current runtime/assets/slot readiness.
 return scope;
}
} // namespace boots204

