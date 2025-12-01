#ifndef STATEHOLDER_H
#define STATEHOLDER_H

struct BioreactorState {
    float targetpH;
    float targetTemp;
    float targetRpm;
    float currentPH;
    float currentRPM;
    float currentTemp;
};

extern BioreactorState reactorState;

#endif
