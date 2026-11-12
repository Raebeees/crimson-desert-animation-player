#pragma once
#include <cmath>
#include <cstdint>

// Offline-tested policy, NOT a native game adapter. All calls must be serialized
// by the host on its verified game thread. No process access or hooks here.
namespace idle {
using Token = std::uint64_t;
struct Context {
    Token actor=0; // host-owned actor identity + lifetime generation, never a raw pointer
    bool ready=false, grounded=false, focused=false;
    bool movement=false, combat=false, interrupted=false;
    double stick=0, speed=0;
    bool allowed() const {
        return actor && ready && grounded && focused && !movement && !combat &&
            !interrupted && std::isfinite(stick) && std::isfinite(speed) &&
            std::abs(stick)<.20 && std::abs(speed)<.02;
    }
};
struct Backend {
    virtual ~Backend()=default;
    // Nonblocking requests. Start at clip time zero; host later sends started().
    // False means rejected WITHOUT acquiring any resources.
    virtual bool start(Token request, Token actor)=0;
    // Cancels pending starts AND active animation for this exact token. The host
    // MUST fence queued starts before stopped(success=true); never acknowledge
    // while a delayed start could still acquire resources. Idempotent.
    virtual bool stop(Token request)=0;
    // Start audio at the actual acknowledged animation position, not UI click time.
    virtual bool audio_start(double seconds)=0;
    virtual void audio_stop()=0;
    virtual void volume(double gain)=0;
};
enum class State { Idle, Starting, Hover, Playing, Leaving, Stopping, Fault };
class Player {
    Backend& b;
    Token next=0, active=0, actor=0;
    bool audition=false, queued=false, queuedHover=false;
    double transition=0, gain=0, from=0, target=0, rampSeconds=0;
    void ramp(double now, double to, double seconds) {
        transition=now; from=gain; target=to; rampSeconds=seconds;
    }
    bool begin(bool hover, const Context& c, double now) {
        if(!c.allowed() || !std::isfinite(now))return false;
        active=++next; actor=c.actor; audition=hover; state=State::Starting;
        transition=now; gain=0; b.volume(0);
        if(!b.start(active,actor)) {state=State::Idle;active=0;return false;}
        return true;
    }
public:
    State state=State::Idle;
    bool repeat=true;
    // Manifest supplies common audio/animation boundary; no independent loops.
    double duration;
    explicit Player(Backend& backend, double length):b(backend),duration(length){}
    Token token() const {return active;}
    bool select(bool hover, const Context& c, double now) {
        if(!c.allowed() || !std::isfinite(now) || !std::isfinite(duration) || duration<=0) {
            cancel(now);return false;
        }
        if(state==State::Fault || state==State::Stopping)return false;
        if(hover && (state==State::Playing || state==State::Starting || state==State::Hover))return false;
        if(state!=State::Idle) {
            cancel(now);queued=state==State::Stopping;queuedHover=hover;
            return queued;
        }
        return begin(hover,c,now);
    }
    void cancel(double now) {
        queued=false;b.audio_stop();gain=0;b.volume(0);
        if(state==State::Idle || state==State::Stopping || state==State::Fault)return;
        state=State::Stopping;transition=now;
        if(!b.stop(active))state=State::Fault;
    }
    void leave(double now) {
        if(state==State::Starting && audition)cancel(now);
        else if(state==State::Hover) {state=State::Leaving;ramp(now,0,.45);}
    }
    void started(Token request, double clipPosition, const Context& c, double now) {
        if(request!=active)return; // stopped() fencing guarantees stale requests cannot run
        if(state!=State::Starting)return;
        if(c.actor!=actor || !c.allowed() || !std::isfinite(clipPosition) ||
           clipPosition<0 || clipPosition>=duration || !b.audio_start(clipPosition)) {
            cancel(now);return;
        }
        state=audition?State::Hover:State::Playing;
        ramp(now,audition?.20:1.,audition?.70:.15);
    }
    void stopped(Token request, bool success, const Context& c, double now) {
        if(request!=active || state!=State::Stopping)return;
        if(!success){queued=false;state=State::Fault;return;}
        bool again=queued && c.actor==actor && c.allowed();
        bool hover=queuedHover;queued=false;active=0;state=State::Idle;
        if(again)begin(hover,c,now);
    }
    // clipPosition is the ENGINE animation clock, not wall-clock estimation.
    // audioHealthy=false includes decoder/device failure or early EOF.
    void tick(const Context& c, double now, double clipPosition, bool audioHealthy=true) {
        if(state==State::Idle || state==State::Fault)return;
        if(state==State::Stopping) {
            if(!std::isfinite(now) || now-transition>2.)state=State::Fault;
            return;
        }
        if(!std::isfinite(now) || c.actor!=actor || !c.allowed()) {cancel(now);return;}
        if(state==State::Starting) {
            if(now-transition>2.)cancel(now);
            return;
        }
        if(!std::isfinite(clipPosition) || clipPosition<0) {cancel(now);return;}
        if(clipPosition>=duration) {
            bool again=state==State::Playing && repeat;
            cancel(now);queued=again && state==State::Stopping;queuedHover=false;return;
        }
        if(!audioHealthy){cancel(now);return;}
        double t=std::fmax(0.,std::fmin(1.,(now-transition)/rampSeconds));
        gain=from+(target-from)*t*t*(3.-2.*t);b.volume(gain);
        if(state==State::Leaving && t>=1.)cancel(now);
    }
};
}
