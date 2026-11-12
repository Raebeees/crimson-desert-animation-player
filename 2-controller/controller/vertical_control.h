#pragma once
#include <cmath>
#include <cstdint>
#include <cwchar>

namespace boots {
struct VerticalTuning { float acceleration=8.0f, deceleration=8.0f; };
// Set once during startup, before hooks are enabled. Units are velocity per second.
inline VerticalTuning verticalTuning{};
inline float parseVerticalRate(const wchar_t* text) {
    if (!text || !*text) return 8.0f;
    wchar_t* end=nullptr;
    float value=std::wcstof(text,&end);
    while (*end==L' ' || *end==L'\t') ++end;
    return end!=text && !*end && std::isfinite(value) && value>=0.5f && value<=30.0f ? value : 8.0f;
}
// Pure control policy. The adapter must establish ownership, active flight,
// available stamina, gameplay input and focus before setting eligible=true.
// Outputs are velocities, not per-frame displacement. Apply before collision
// resolution with the game's timestep; do not add them to solved positions.
struct VerticalInput {
    uintptr_t actor=0;
    bool eligible=false, up=false, down=false, cancel=false;
    float dt=0;
};
struct VerticalOutput { bool overrideVertical=false; float speed=0; };
class VerticalControl {
    uintptr_t actor_=0;
    bool armed_=false;
    float speed_=0;
public:
    void reset() { actor_=0; armed_=false; speed_=0; }
    VerticalOutput update(const VerticalInput& in) {
        if(!in.actor || !in.eligible || !std::isfinite(in.dt) || in.dt<=0 || in.dt>0.1f) {
            reset(); return {};
        }
        if(actor_!=in.actor) { reset(); actor_=in.actor; }
        // Require a neutral sample on entry/re-entry: held buttons from a
        // menu, previous character or previous flight cannot activate control.
        if(!armed_) {
            armed_=!in.up && !in.down && !in.cancel;
            // Hover immediately, but require release of the activation key
            // before accepting a new ascent/descent request.
            return {!in.cancel,0};
        }
        if(in.cancel) { reset(); return {}; }
        const bool directional=in.up!=in.down;
        const float target=directional?(in.up?3.0f:-3.0f):0.0f;
        // Preserve the original arithmetic exactly when both settings agree.
        // On reversal, brake to zero first and use any remaining time to accelerate.
        const auto tuning=verticalTuning;
        if(tuning.acceleration!=tuning.deceleration && speed_!=0 && target!=0 &&
           ((speed_>0)!=(target>0))) {
            const float brakeTime=std::fabs(speed_)/tuning.deceleration;
            if(in.dt<=brakeTime) {
                const float step=tuning.deceleration*in.dt;
                speed_ += speed_>0 ? -step : step;
                return {true,speed_};
            }
            const float step=tuning.acceleration*(in.dt-brakeTime);
            speed_=target>0?std::fmin(step,target):std::fmax(-step,target);
            return {true,speed_};
        }
        const bool braking=target==0;
        const float step=(braking?tuning.deceleration:tuning.acceleration)*in.dt;
        if(speed_<target) speed_=std::fmin(speed_+step,target);
        else speed_=std::fmax(speed_-step,target);
        return {true,speed_};
    }
};
}
