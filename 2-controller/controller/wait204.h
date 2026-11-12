#pragma once
// No hooks, slot changes or native callbacks are allowed inside probe.
// Require two successful samples, and enforce the deadline even for a slow read.
template<class Probe,class Clock,class Pause>
bool waitForConditions204(Probe probe, Clock now, Pause pause){
    const auto start=now();
    unsigned matches=0;
    while(now()-start<120000){
        const bool matched=probe();
        if(now()-start>=120000)return false;
        matches=matched?matches+1:0;
        if(matches==2)return true;
        pause();
    }
    return false;
}
