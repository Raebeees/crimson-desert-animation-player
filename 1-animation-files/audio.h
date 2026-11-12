#pragma once
#include <windows.h>
#include <xaudio2.h>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <atomic>
namespace lowcortisol {
struct Wave {
 WAVEFORMATEX format{};
 std::vector<BYTE> pcm;
 double duration() const {return double(pcm.size())/format.nAvgBytesPerSec;}
 static Wave load(const std::filesystem::path& path) {
  std::ifstream f(path,std::ios::binary|std::ios::ate);
  if(!f)throw std::runtime_error("Cannot open low-cortisol.wav");
  const auto size=f.tellg();
  if(size<44 || size>128*1024*1024)throw std::runtime_error("Invalid WAV size");
  std::vector<BYTE> bytes(static_cast<size_t>(size));f.seekg(0);
  if(!f.read(reinterpret_cast<char*>(bytes.data()),size))throw std::runtime_error("WAV read failed");
  auto u32=[&](size_t at){DWORD v;std::memcpy(&v,bytes.data()+at,4);return v;};
  if(std::memcmp(bytes.data(),"RIFF",4)||std::memcmp(bytes.data()+8,"WAVE",4)||
     std::uint64_t(u32(4))+8!=bytes.size())throw std::runtime_error("Invalid RIFF header");
  Wave w;bool fmt=false,data=false;
  for(size_t at=12;at+8<=bytes.size();) {
   const size_t n=u32(at+4),begin=at+8;
   if(n>bytes.size()-begin)throw std::runtime_error("Truncated WAV chunk");
   if(!std::memcmp(bytes.data()+at,"fmt ",4)) {
    if(fmt||n<16)throw std::runtime_error("Invalid WAV format chunk");
    std::memcpy(&w.format,bytes.data()+begin,16);fmt=true;
   }
   if(!std::memcmp(bytes.data()+at,"data",4)) {
    if(data)throw std::runtime_error("Duplicate PCM chunk");
    w.pcm.assign(bytes.begin()+begin,bytes.begin()+begin+n);data=true;
   }
   at=begin+n+(n&1);
  }
  if(!fmt||!data||w.pcm.empty()||w.format.wFormatTag!=WAVE_FORMAT_PCM||
     w.format.nChannels!=2||(w.format.nSamplesPerSec!=44100 && w.format.nSamplesPerSec!=48000)||w.format.wBitsPerSample!=16||
     w.format.nBlockAlign!=4||w.format.nAvgBytesPerSec!=w.format.nSamplesPerSec*4||w.pcm.size()%4)
   throw std::runtime_error("Expected 44.1 or 48 kHz stereo PCM16");
  return w;
 }
};
class Audio final:private IXAudio2EngineCallback {
 IXAudio2* engine=nullptr;IXAudio2MasteringVoice* master=nullptr;IXAudio2SourceVoice* voice=nullptr;
 Wave wave;double offset=0,lastGain=0;std::atomic<bool> fault{false};bool registered=false;
 void STDMETHODCALLTYPE OnProcessingPassStart()override{}
 void STDMETHODCALLTYPE OnProcessingPassEnd()override{}
 void STDMETHODCALLTYPE OnCriticalError(HRESULT)override{fault=true;}
public:
 Audio()=default;Audio(const Audio&)=delete;Audio& operator=(const Audio&)=delete;
 ~Audio(){close();}
 void close(){if(voice){voice->DestroyVoice();voice=nullptr;}if(master){master->DestroyVoice();master=nullptr;}if(engine){if(registered)engine->UnregisterForCallbacks(this);engine->Release();engine=nullptr;}registered=false;}
 bool open(const std::filesystem::path& path) {
  close();fault=false;wave=Wave::load(path);
  if(FAILED(XAudio2Create(&engine,0,XAUDIO2_DEFAULT_PROCESSOR))||
     FAILED(engine->CreateMasteringVoice(&master))){close();return false;}
  if(FAILED(engine->RegisterForCallbacks(this))){close();return false;}registered=true;return true;
 }
 bool play(double seconds) {
  stop();if(!engine||fault||!std::isfinite(seconds)||seconds<0||seconds>=wave.duration())return false;
  if(FAILED(engine->CreateSourceVoice(&voice,&wave.format))){voice=nullptr;return false;}
  XAUDIO2_BUFFER buffer{};buffer.Flags=XAUDIO2_END_OF_STREAM;
  const size_t first=static_cast<size_t>(seconds*wave.format.nSamplesPerSec)*wave.format.nBlockAlign;
  buffer.AudioBytes=static_cast<UINT32>(wave.pcm.size()-first);buffer.pAudioData=wave.pcm.data()+first;
  offset=double(first)/wave.format.nAvgBytesPerSec;
  if(FAILED(voice->SetVolume(0))||FAILED(voice->SubmitSourceBuffer(&buffer))||FAILED(voice->Start())){stop();return false;}return true;
 }
 void stop(){if(voice){voice->DestroyVoice();voice=nullptr;}}
 void gain(double value){if(!std::isfinite(value)){fault=true;stop();return;}lastGain=std::clamp(value,0.,1.);if(voice&&FAILED(voice->SetVolume(static_cast<float>(lastGain))))fault=true;}
 bool sync(double engineSeconds){
  if(!voice||fault||!std::isfinite(engineSeconds)||engineSeconds<0||engineSeconds>=duration())return false;
  const double error=engineSeconds-seconds();
  if(std::abs(error)>.150){const double saved=lastGain;if(!play(engineSeconds))return false;gain(saved);return healthy();}
  const float ratio=static_cast<float>(std::clamp(1.+error*.25,.98,1.02));
  if(FAILED(voice->SetFrequencyRatio(ratio))){fault=true;return false;}return true;
 }
 double seconds()const {if(!voice)return 0;XAUDIO2_VOICE_STATE s{};voice->GetState(&s);return offset+double(s.SamplesPlayed)/wave.format.nSamplesPerSec;}
 bool healthy()const {if(!voice||fault)return false;XAUDIO2_VOICE_STATE s{};voice->GetState(&s,XAUDIO2_VOICE_NOSAMPLESPLAYED);return s.BuffersQueued!=0;}
 double duration()const{return wave.duration();}
};
}
