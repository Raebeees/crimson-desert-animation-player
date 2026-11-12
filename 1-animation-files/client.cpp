#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <xinput.h>
#include <atomic>
#include <mutex>
#include <string>
#include <cstdio>
#include <cstdarg>
#include "audio.h"
#include "transport.h"
#include "selection_v2.h"
namespace {
using namespace lowcortisol;
Transport transport{};std::mutex bindingMutex;std::atomic<bool> running{false};
HMODULE module=nullptr;
void trace(const char* format,...) {
 static unsigned count=0;if(count++>=600)return;
 wchar_t name[32768]{};GetModuleFileNameW(module,name,32768);
 auto path=std::filesystem::path(name).parent_path()/L"LowCortisol-input.log";
 FILE* file=nullptr;if(_wfopen_s(&file,path.c_str(),L"a")||!file)return;
 fprintf(file,"%llu ",GetTickCount64());va_list args;va_start(args,format);vfprintf(file,format,args);va_end(args);fputc('\n',file);fclose(file);
}
double now(){return double(GetTickCount64())/1000.;}
struct Client final:idle::Backend {
 bool demo=false,hover=false,favorite=false,closing=false,dragVolume=false;
 double masterVolume=1.,playbackGain=0.;
 int selected=0,pending=-1,hoverRow=-1;std::filesystem::path audioDir;
 unsigned clicks=0;int tracedState=-1;bool mouseHeld=false;double lastTrace=-10;
 HWND window=nullptr;HFONT font=nullptr,titleFont=nullptr;
 Audio audio,previewAudio; double previewStarted=0, previewLeaving=0; bool previewActive=false, previewFading=false; Transport host{};idle::Token demoToken=0;bool demoStarted=false,demoStopped=false;
 idle::Player player;Observation observation{};std::wstring error;
 explicit Client(bool preview):demo(preview),player(*this,10.944240362811792){
  std::lock_guard<std::mutex> lock(bindingMutex);host=transport;
 }
 bool start(idle::Token token,idle::Token actor)override {
  if(demo){demoToken=token;demoStarted=true;demoStopped=false;return true;}
  const bool accepted=host.start&&host.start(token,actor,dance_v2::ids[selected]);trace("start request=%llu actor=%llu accepted=%d",token,actor,int(accepted));return accepted;
 }
 bool stop(idle::Token token)override {
  if(demo){demoStarted=false;demoStopped=true;demoToken=token;return true;}
  const bool accepted=host.stop&&host.stop(token);trace("stop request=%llu accepted=%d",token,int(accepted));return accepted;
 }
 bool audio_start(double pos)override{const bool ok=audio.play(pos);trace("audio start seconds=%.4f success=%d",pos,int(ok));return ok;}
 void audio_stop()override{audio.stop();}
 void volume(double value)override{playbackGain=value;audio.gain(value*masterVolume);}
 void setVolume(int x){masterVolume=std::clamp((x-100)/166.,0.,1.);audio.gain(playbackGain*masterVolume);InvalidateRect(window,nullptr,FALSE);}
 bool inputCancel()const {
  POINT cursor{};GetCursorPos(&cursor);const HWND under=WindowFromPoint(cursor);
  constexpr int keys[]={'W','A','S','D',VK_SPACE,VK_ESCAPE};
  for(int key:keys) {
   // Window selection clicks are not attacks. Movement always wins.
   if((key==VK_LBUTTON||key==VK_RBUTTON)&&(GetForegroundWindow()==window||under==window||IsChild(window,under)))continue;
   if(GetAsyncKeyState(key)&0x8000)return true;
  }
  XINPUT_STATE s{};
  for(DWORD i=0;i<4;++i)if(XInputGetState(i,&s)==ERROR_SUCCESS) {
   const auto& g=s.Gamepad;
   if(std::abs(int(g.sThumbLX))>6500||std::abs(int(g.sThumbLY))>6500||
      (g.wButtons&(XINPUT_GAMEPAD_A|XINPUT_GAMEPAD_B|XINPUT_GAMEPAD_X|XINPUT_GAMEPAD_Y|
       XINPUT_GAMEPAD_DPAD_UP|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_RIGHT))||
      g.bLeftTrigger>30||g.bRightTrigger>30)return true;
  }return false;
 }
 void observe() {
  observation={};
  if(demo){observation.context={1,true,true,GetForegroundWindow()==window,false,false,false,0,0};
   observation.token=demoToken;observation.started=demoStarted;observation.stopped=demoStopped;
   observation.clipSeconds=audio.seconds();
  } else if(!host.observe||!host.observe(&observation)||observation.size!=sizeof(Observation)||observation.version!=2){
   observation={};error=L"Animation connection unavailable";
  } else if(observation.failed)error=L"Animation connection needs a restart";
  else if(!observation.context.ready)error=L"Waiting for player state";
  else if(!observation.context.focused||!observation.context.grounded)error=L"Stand still on foot to dance";
  else error.clear();
  if(inputCancel())observation.context.movement=true;
 }
 const wchar_t* audioName(int n)const{static const wchar_t* names[]={L"spark.wav",L"caffeinated.wav",L"popular.wav",L"its_a_vibe.wav",L"big_dawgs.wav",L"verve.wav",L"california_gurls.wav",L"options.wav"};return names[n];}
 void previewBegin(int n){if(player.state!=idle::State::Idle)return;try{if(!previewAudio.open(audioDir/audioName(n))||!previewAudio.play(0))return;}catch(...){return;}previewActive=true;previewFading=false;previewStarted=now();previewAudio.gain(0);}
 void previewEnd(){if(previewActive&&!previewFading){previewFading=true;previewLeaving=now();}}
 void select(int n) {previewAudio.stop();previewActive=previewFading=false;observe();
  if(player.state!=idle::State::Idle){pending=n;player.cancel(now());return;}
  selected=n;player.duration=dance_v2::durations[n];
  try{if(!audio.open(audioDir/audioName(n))){error=L"Audio unavailable";return;}}catch(...){error=L"Audio unavailable";return;}
  const bool accepted=player.select(false,observation.context,now());trace("select dance=%d accepted=%d",n,int(accepted));InvalidateRect(window,nullptr,FALSE);
 }
 void tick() {
  observe();const double t=now();
  if(previewActive){if(!previewAudio.healthy()){previewAudio.stop();previewActive=previewFading=false;}else if(previewFading){double g=.2*(1.-(t-previewLeaving)/.45);if(g<=0){previewAudio.stop();previewActive=previewFading=false;}else previewAudio.gain(g*masterVolume);}else previewAudio.gain(.2*masterVolume*(std::min)(1.,(t-previewStarted)/.7));}
  const bool down=(GetAsyncKeyState(VK_LBUTTON)&0x8000)!=0;
  if(down&&!mouseHeld){POINT pt{};GetCursorPos(&pt);HWND under=WindowFromPoint(pt);ScreenToClient(window,&pt);trace("physical mouse down client=%ld,%ld hitMenu=%d foregroundMenu=%d",pt.x,pt.y,int(under==window),int(GetForegroundWindow()==window));}mouseHeld=down;
  if(t-lastTrace>=2.||tracedState!=int(player.state)){lastTrace=t;tracedState=int(player.state);const auto& x=observation.context;trace("state=%u clicks=%u ready=%d grounded=%d focused=%d movement=%d combat=%d interrupted=%d started=%d failed=%d",unsigned(player.state),clicks,int(x.ready),int(x.grounded),int(x.focused),int(x.movement),int(x.combat),int(x.interrupted),int(observation.started),int(observation.failed));}
  if(observation.failed)player.cancel(t);
  if(player.state==idle::State::Starting&&observation.started) {
   player.started(observation.token,observation.clipSeconds,observation.context,t);demoStarted=false;
  }
  if(player.state==idle::State::Stopping&&observation.stopped) {
   player.stopped(observation.token,!observation.failed,observation.context,t);demoStopped=false;
   if(pending>=0&&player.state==idle::State::Idle){int next=pending;pending=-1;select(next);}
  }
  if(!demo&&(player.state==idle::State::Hover||player.state==idle::State::Playing||player.state==idle::State::Leaving)){
   if(observation.stopped&&observation.clipSeconds<player.duration)player.cancel(t);
   else if(observation.clipSeconds<player.duration&&!audio.sync(observation.clipSeconds))player.cancel(t);
  }
  double position=observation.clipSeconds;
  // Allow the unchanged soundtrack to finish its final fraction of a frame;
  // the adapter still fences both native layers before a repeat can begin.
  if(!demo&&!audio.healthy()&&position>=player.duration-.100)position=player.duration;
  player.tick(observation.context,t,position,audio.healthy());
  if(closing&&player.state==idle::State::Idle){DestroyWindow(window);return;}
  InvalidateRect(window,nullptr,FALSE);
 }
 RECT rows[8]={{16,65,304,101},{16,106,304,142},{16,147,304,183},{16,188,304,224},{16,229,304,265},{16,270,304,306},{16,311,304,347},{16,352,304,388}},fav{174,17,270,49},repeat{16,398,128,432};
 void text(HDC dc,const wchar_t* value,RECT r,COLORREF color,HFONT face) {
  SetTextColor(dc,color);SelectObject(dc,face);DrawTextW(dc,value,-1,&r,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
 }
 void box(HDC dc,RECT r,COLORREF color) {
  HBRUSH b=CreateSolidBrush(color);HPEN p=CreatePen(PS_SOLID,1,color);
  auto oldB=SelectObject(dc,b),oldP=SelectObject(dc,p);RoundRect(dc,r.left,r.top,r.right,r.bottom,12,12);
  SelectObject(dc,oldB);SelectObject(dc,oldP);DeleteObject(b);DeleteObject(p);
 }
 void paint() {
  PAINTSTRUCT ps{};HDC target=BeginPaint(window,&ps);RECT bounds{};GetClientRect(window,&bounds);
  HDC dc=CreateCompatibleDC(target);HBITMAP bitmap=CreateCompatibleBitmap(target,bounds.right,bounds.bottom);
  auto old=SelectObject(dc,bitmap);SetBkMode(dc,TRANSPARENT);
  HBRUSH background=CreateSolidBrush(RGB(29,24,22));FillRect(dc,&bounds,background);DeleteObject(background);
  text(dc,L"Animations",{17,16,170,49},RGB(231,211,182),titleFont);
  box(dc,fav,RGB(42,33,30));text(dc,L"Favorites",{186,17,267,49},RGB(203,176,142),font);
  text(dc,L"\x00d7",{286,16,311,49},RGB(208,186,167),titleFont);
  const bool active=player.state==idle::State::Hover||player.state==idle::State::Playing||player.state==idle::State::Leaving;
  const wchar_t* names[]={L"SPARK",L"Caffeinated",L"Popular",L"It's a Vibe",L"Big Dawgs",L"Verve",L"California Gurls",L"Options"};
  for(int i=0;i<8;++i){box(dc,rows[i],selected==i&&active?RGB(105,47,39):hoverRow==i?RGB(61,40,35):RGB(39,31,28));RECT label=rows[i];label.left+=13;label.right-=35;text(dc,names[i],label,RGB(245,229,212),font);}
  text(dc,favorite?L"\x2665":L"\x2661",{270,65,299,101},RGB(196,134,107),titleFont);
  box(dc,repeat,player.repeat?RGB(112,55,42):RGB(48,41,36));
  text(dc,player.repeat?L"Repeat  ON":L"Repeat  OFF",{28,398,125,432},RGB(226,199,161),font);
  text(dc,L"Move to stop",{186,398,305,432},RGB(156,140,127),font);
  text(dc,L"Volume",{17,442,90,470},RGB(203,176,142),font);
  box(dc,{100,453,266,459},RGB(76,61,53));
  int knob=100+int(masterVolume*166.);box(dc,{100,453,knob,459},RGB(153,76,58));box(dc,{knob-5,448,knob+5,464},RGB(231,195,171));
  wchar_t percent[8]{};swprintf_s(percent,L"%d%%",int(masterVolume*100.+.5));text(dc,percent,{273,442,313,470},RGB(226,199,161),font);
  const wchar_t* status=demo?L"Audio preview test · no game animation":L"Ready";
  if(!error.empty())status=error.c_str();
  if(player.state==idle::State::Starting)status=L"Starting animation...";
  if(player.state==idle::State::Stopping)status=L"Stopping animation...";
  if(player.state==idle::State::Fault)status=L"Stop not acknowledged · restart required";
  text(dc,status,{17,480,305,506},RGB(168,145,125),font);
  BitBlt(target,0,0,bounds.right,bounds.bottom,dc,0,0,SRCCOPY);SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);EndPaint(window,&ps);
 }
};
LRESULT CALLBACK windowProc(HWND h,UINT msg,WPARAM w,LPARAM l) {
 auto* c=reinterpret_cast<Client*>(GetWindowLongPtrW(h,GWLP_USERDATA));
 if(msg==WM_NCCREATE){c=static_cast<Client*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);c->window=h;SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(c));}
 if(!c)return DefWindowProcW(h,msg,w,l);
 POINT p{GET_X_LPARAM(l),GET_Y_LPARAM(l)};
 switch(msg) {
 case WM_APP+20:ShowWindow(h,SW_SHOWNORMAL);SetForegroundWindow(h);SetFocus(h);return 0;
 case WM_PAINT:c->paint();return 0;
 case WM_ERASEBKGND:return 1;
 case WM_MOUSEACTIVATE:trace("WM_MOUSEACTIVATE");return MA_ACTIVATE;
 case WM_TIMER:c->tick();return 0;
 case WM_MOUSEMOVE: {
  if(c->dragVolume){c->setVolume(p.x);return 0;}
  TRACKMOUSEEVENT track{sizeof(track),TME_LEAVE,h,0};TrackMouseEvent(&track);
  int hit=-1;if(p.x<263)for(int i=0;i<8;++i)if(PtInRect(&c->rows[i],p)){hit=i;break;}
  if(hit!=c->hoverRow){if(c->hoverRow>=0)c->previewEnd();c->hoverRow=hit;if(hit>=0)c->previewBegin(hit);}return 0;
 }
 case WM_MOUSELEAVE:trace("mouse leave state=%u",unsigned(c->player.state));c->hover=false;c->hoverRow=-1;c->previewEnd();return 0;
 case WM_LBUTTONDOWN:
  if(p.y>=440&&p.y<=471&&p.x>=90&&p.x<=276){c->dragVolume=true;SetCapture(h);c->setVolume(p.x);return 0;}
  ++c->clicks;trace("WM_LBUTTONDOWN clicks=%u x=%ld y=%ld",c->clicks,p.x,p.y);
  if(p.x>280&&p.y<50){SendMessageW(h,WM_CLOSE,0,0);return 0;}
  if(PtInRect(&c->fav,p)) {
   c->player.leave(now());HMENU menu=CreatePopupMenu();
   AppendMenuW(menu,MF_STRING|(c->favorite?MF_ENABLED:MF_GRAYED),1,c->favorite?L"SPARK":L"No favorites yet");
   POINT screen{c->fav.left,c->fav.bottom};ClientToScreen(h,&screen);
   const auto command=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_NONOTIFY,screen.x,screen.y,0,h,nullptr);
   DestroyMenu(menu);if(command==1)c->select(0);return 0;
  }
  if(PtInRect(&c->repeat,p)){c->player.repeat=!c->player.repeat;return 0;}
  for(int i=0;i<8;++i)if(PtInRect(&c->rows[i],p)){if(i==0&&p.x>=263)c->favorite=!c->favorite;else c->select(i);return 0;}
  if(p.y<60){ReleaseCapture();SendMessageW(h,WM_NCLBUTTONDOWN,HTCAPTION,0);}return 0;
 case WM_LBUTTONUP:if(c->dragVolume){c->setVolume(p.x);c->dragVolume=false;ReleaseCapture();return 0;}break;
 case WM_CAPTURECHANGED:c->dragVolume=false;return 0;
 case WM_ACTIVATE:if(LOWORD(w)==WA_INACTIVE)c->previewEnd();return 0;
 case WM_CLOSE:
  c->previewAudio.stop();c->closing=true;c->player.cancel(now());
  // Keep the client and audio alive while the adapter fences the stop.
  if(!c->demo&&c->player.state!=idle::State::Idle){c->error=L"Waiting for animation stop acknowledgement";return 0;}
  DestroyWindow(h);return 0;
 case WM_DESTROY:KillTimer(h,1);PostQuitMessage(0);return 0;
 }return DefWindowProcW(h,msg,w,l);
}
int run(bool demo) {
 bool expected=false;if(!running.compare_exchange_strong(expected,true))return 1;
 HWND previous=GetForegroundWindow();DWORD previousPid=0;GetWindowThreadProcessId(previous,&previousPid);
 if(previousPid!=GetCurrentProcessId())previous=nullptr;
 int result=0;const HRESULT com=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
 try {
  trace("OPEN input diagnostic; demo=%d",int(demo));Client c(demo);wchar_t path[32768]{};GetModuleFileNameW(module,path,32768);
  c.audioDir=std::filesystem::path(path).parent_path();
  c.font=CreateFontW(-14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
  c.titleFont=CreateFontW(-18,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
  WNDCLASSW wc{};wc.lpfnWndProc=windowProc;wc.hInstance=module;wc.lpszClassName=L"LowCortisolNativeMenu1";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
  RegisterClassW(&wc);
  HWND h=CreateWindowExW(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,wc.lpszClassName,L"SPARK",WS_POPUP,100,100,320,518,nullptr,nullptr,module,&c);
  if(!h)throw std::runtime_error("Menu creation failed");
  DWORD corner=2;DwmSetWindowAttribute(h,33,&corner,sizeof(corner));
  SetTimer(h,1,16,nullptr);ShowWindow(h,SW_SHOWNORMAL);SetForegroundWindow(h);SetFocus(h);trace("created foregroundMenu=%d",int(GetForegroundWindow()==h));
  MSG message{};while(GetMessageW(&message,nullptr,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}
  DeleteObject(c.font);DeleteObject(c.titleFont);
 }catch(const std::exception& e){MessageBoxA(nullptr,e.what(),"Low Cortisol",MB_OK|MB_ICONERROR);result=2;}
 DWORD activePid=0;GetWindowThreadProcessId(GetForegroundWindow(),&activePid);
 if(previous&&IsWindow(previous)&&(!GetForegroundWindow()||activePid==GetCurrentProcessId()))SetForegroundWindow(previous);
 if(SUCCEEDED(com))CoUninitialize();running=false;return result;
}
}
extern "C" __declspec(dllexport) bool LowCortisolBindTransport1(const lowcortisol::Transport* api) {
 std::lock_guard<std::mutex> lock(bindingMutex);
 if(running||transport.observe||!api||api->size!=sizeof(*api)||api->version!=2||!api->start||!api->stop||!api->observe)return false;
 // Provider must be process-lifetime and supplied by the shared owner.
 HMODULE owner=nullptr,starter=nullptr,stopper=nullptr;
 constexpr DWORD inspect=GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
 if(!GetModuleHandleExW(inspect,reinterpret_cast<LPCWSTR>(api->observe),&owner)||
    !GetModuleHandleExW(inspect,reinterpret_cast<LPCWSTR>(api->start),&starter)||
    !GetModuleHandleExW(inspect,reinterpret_cast<LPCWSTR>(api->stop),&stopper)||owner!=starter||owner!=stopper)return false;
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(api->observe),&owner))return false;
 transport=*api;return true;
}
extern "C" __declspec(dllexport) int LowCortisolOpenMenu1(){return run(false);}
#ifdef LOWCORTISOL_EXE
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int){module=instance;return run(true);}
#else
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH){module=instance;DisableThreadLibraryCalls(instance);}return TRUE;}
#endif
