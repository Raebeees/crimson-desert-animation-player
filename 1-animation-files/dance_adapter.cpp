#include <windows.h>
#include "native_adapter.h"
#include "idle_shared_api.h"
extern "C" bool LowCortisolBindTransport1(const lowcortisol::Transport*);
extern "C" __declspec(dllexport) bool DanceInitialize1(std::uintptr_t game,std::uintptr_t cc,idle_shared::Register registerBridge){
 if(!registerBridge||!lowcortisol_engine::adapter.configure(game,cc))return false;
 lowcortisol::Transport t;t.start=lowcortisol_engine::request;t.stop=lowcortisol_engine::stop;t.observe=lowcortisol_engine::observe;
 if(!LowCortisolBindTransport1(&t))return false;
 const auto r=lowcortisol_engine::registration();return registerBridge(&r);
}
extern "C" __declspec(dllexport) void DanceService1(bool ready){lowcortisol_engine::adapter.service(ready);}
