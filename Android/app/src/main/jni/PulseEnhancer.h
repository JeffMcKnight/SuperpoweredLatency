//
// Created by JeffMcKnight on 11/16/16.
//

#ifndef LOOPBACKAPP_PULSE_ENHANCER_H
#define LOOPBACKAPP_PULSE_ENHANCER_H

//static const int WARMUP_PERIOD_MSEC = 20;

static const char *const TAG = "PulseEnhancer";

#include <SLES/OpenSLES.h>
#include <SuperpoweredAudioBuffers.h>
#include <SuperpoweredFilter.h>
#include <SuperpoweredLimiter.h>
#include <SuperpoweredSimple.h>


class PulseEnhancer {
public:
    static PulseEnhancer *create(int samplingRate, const float injectedFrequencyHz);
    void processForOpenAir(short *inputBuffer,
                           short *outputBuffer,
                           SLuint32 bufSizeInFrames,
                           SLuint32 channelCount);

private:
    PulseEnhancer(int sampleRate, const float injectedFrequencyHz);

    const float FILTER_FREQUENCY_HZ;
    const float FILTER_RESONANCE;
    const float TARGET_PEAK;
    const unsigned int SAMPLES_MONO_ONE_MSEC;
    const unsigned int SAMPLES_STEREO_ONE_MSEC;
    const int SAMPLE_RATE;
    int mTimeElapsed;
    float mVolume;
    SuperpoweredFilter *mFilter;
    SuperpoweredLimiter *mLimiter;

    void createFilter(unsigned int samplingRate);
    void createLimiter(unsigned int samplingRate);
    void stereoFloatToShortIntBuffer(float *stereoInputBuffer,
                                     short *outputBuffer,
                                     SLuint32 bufSizeInFrames,
                                     SLuint32 channelCount);
    void shortIntToStereoFloatBuffer(short *shortIntBuffer,
                                     SLuint32 bufSizeInFrames,
                                     SLuint32 channelCount,
                                     float *stereoFloatBuffer);
    int bufferSizeInMsec(int bufSizeInFrames) const;
};


#endif //LOOPBACKAPP_PULSE_ENHANCER_H
