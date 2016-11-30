#ifndef Header_latencyMeasurer
#define Header_latencyMeasurer

#include "PulseEnhancer.h"

typedef enum measurementStates {
    measure_average_loudness_for_1_sec,
    playing_and_listening,
    waiting,
    passthrough,
    idle
} measurementStates;

class latencyMeasurer {
public:
    int state; // -1: passthrough, 0: idle, 1..10 taking measurement steps, 11 finished
    int samplerate;
    int latencyMs;
    int buffersize;

    latencyMeasurer(int maxMeasurements, int _samplerate);
    void processInput(short int *audio, int samplerate, int numberOfSamples);
    void processOutput(short int *audio);
    void toggle();
    void togglePassThrough();

private:
    measurementStates measurementState, nextMeasurementState;
    const int maxMesausements;
    float burstFreqHz;
    float roundTripLatencyMs[10], sineWave, rampdec;
    int sum, samplesElapsed;
    short int threshold;
    PulseEnhancer *pulseEnhancer;
};

#endif
