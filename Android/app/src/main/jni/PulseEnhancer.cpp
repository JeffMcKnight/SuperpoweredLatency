//
// Created by JeffMcKnight on 11/16/16.
//

#include "PulseEnhancer.h"
#include "/Users/Shared/Library/Android/sdk/ndk-bundle/platforms/android-16/arch-x86/usr/include/android/log.h"

/**
 * Constructor to initialize fields and constants with default values.
 * <b>Do not directly use this method to create an instance of {@link PulseEnhancer}.  Use the factory
 * method instead. </b>
 * FILTER_FREQUENCY_HZ should match Constant.LOOPBACK_FREQUENCY
 * FILTER_RESONANCE chosen to make feedback fairly stable
 */
PulseEnhancer::PulseEnhancer(int sampleRate, const float injectedFrequencyHz) :
        SAMPLE_RATE(sampleRate),
        SAMPLES_MONO_ONE_MSEC((unsigned int) (sampleRate / 1000)),
        SAMPLES_STEREO_ONE_MSEC(2 * SAMPLES_MONO_ONE_MSEC),
        mVolume(1.0F),
        mTimeElapsed(0),
        FILTER_FREQUENCY_HZ(injectedFrequencyHz),
        FILTER_RESONANCE(2.0f),
        TARGET_PEAK(1.0f) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "::create -- SAMPLE_RATE: %d -- SAMPLES_MONO_ONE_MSEC: %d -- SAMPLES_STEREO_ONE_MSEC: %d", SAMPLE_RATE, SAMPLES_MONO_ONE_MSEC, SAMPLES_STEREO_ONE_MSEC);
}

/**
 * Factory method. This is the preferred method to create instances of {@link PulseEnhancer}.
 * It creates the object and also sets up the audio {@link #filter}
 */
PulseEnhancer *PulseEnhancer::create(int samplingRate, const float injectedFrequencyHz) {
    PulseEnhancer *enhancer = new PulseEnhancer(samplingRate, injectedFrequencyHz);
    enhancer->createFilter(static_cast<unsigned int>(samplingRate));
    enhancer->createLimiter(static_cast<unsigned int>(samplingRate));
    return enhancer;
}

/**
 * Process the raw record buffer so we can do the latency measurement test in open air without the
 * loopback dongle.
 * @param short *inputBuffer
 * @param short *outputBuffer
 * @param SLuint32 bufSizeInFrames
 * @param SLuint32 channelCount
 */
void PulseEnhancer::processForOpenAir(short *inputBuffer,
                                      short *outputBuffer,
                                      SLuint32 bufSizeInFrames,
                                      SLuint32 channelCount) {
    // Convert the raw 16 bit PCM buffer to stereo floating point;
    unsigned int bufferSizeInSamples = channelCount * bufSizeInFrames;
    float *stereoBuffer = new float[bufferSizeInSamples];
    shortIntToStereoFloatBuffer(inputBuffer, bufSizeInFrames, channelCount, stereoBuffer);
    //  Filter the mic input to reject ambient noise get a cleaner pulse
    mFilter->process(stereoBuffer, stereoBuffer, bufSizeInFrames);
    // Convert buffer back to 16 bit PCM
    stereoFloatToShortIntBuffer(stereoBuffer, outputBuffer, bufSizeInFrames, channelCount);
    // Release audio frame memory resources
    delete[] stereoBuffer;
    mTimeElapsed += bufferSizeInMsec(bufSizeInFrames);
}

/**
 * Convert stereo floating point buffer back to mono 16 bit buffer which matches the format of the
 * recording callback.
 *
 * @param float *stereoInputBuffer
 * @param short *outputBuffer
 * @param SLuint32 bufSizeInFrames
 * @param SLuint32 channelCount
 */
void PulseEnhancer::stereoFloatToShortIntBuffer(float *stereoInputBuffer,
                                                short *outputBuffer,
                                                SLuint32 bufSizeInFrames,
                                                SLuint32 channelCount) {
    if (channelCount == 1){
        float *right = new float[bufSizeInFrames];
        float *left = new float[bufSizeInFrames];
        // Deinterleave back to a mono audio frame
        SuperpoweredDeInterleave(stereoInputBuffer, left, right, bufSizeInFrames);
        // Convert floating point linear PCM back to 16bit linear PCM
        SuperpoweredFloatToShortInt(left, outputBuffer, bufSizeInFrames, channelCount);
        // Release audio frame memory resources
        delete[] left;
        delete[] right;
    } else if (channelCount == 2){
        SuperpoweredFloatToShortInt(stereoInputBuffer, outputBuffer, bufSizeInFrames, channelCount);
    } else {
        __android_log_print(ANDROID_LOG_WARN, TAG, "stereoFloatToShortIntBuffer() -- channelCount: %u!!", channelCount);
    }
}

/**
 * Convert mono 16 bit linear PCM buffer to stereo floating point buffer because most Superpowered
 * methods expect stereo float frames
 * TODO: do this without using so many buffer resources
*/
void PulseEnhancer::shortIntToStereoFloatBuffer(short *shortIntBuffer,
                                                SLuint32 bufSizeInFrames,
                                                SLuint32 channelCount,
                                                float *stereoFloatBuffer) {
    if (channelCount == 1){
        float *monoFloatBuffer = new float[channelCount * bufSizeInFrames];
        // Convert mono 16bit buffer to mono floating point buffer
        SuperpoweredShortIntToFloat(shortIntBuffer,
                                    monoFloatBuffer,
                                    bufSizeInFrames,
                                    channelCount);
        // Convert mono float buffer to stereo float buffer
        SuperpoweredInterleave(monoFloatBuffer, monoFloatBuffer, stereoFloatBuffer, bufSizeInFrames);
        delete[] monoFloatBuffer;
    } else if (channelCount == 2){
        SuperpoweredShortIntToFloat(shortIntBuffer, stereoFloatBuffer, bufSizeInFrames, channelCount);
    } else {
        __android_log_print(ANDROID_LOG_WARN, TAG, "shortIntToStereoFloatBuffer() -- channelCount: %u!!", channelCount);
    }
}

/**
 * Create the 19kHz high pass resonant filter
 */
void PulseEnhancer::createFilter(unsigned int samplingRate) {
    /** Set up highpass filter */
    mFilter = new SuperpoweredFilter(SuperpoweredFilter_Resonant_Highpass, samplingRate);
    mFilter->setResonantParameters(FILTER_FREQUENCY_HZ, FILTER_RESONANCE);
    mFilter->enable(true);
}

int PulseEnhancer::bufferSizeInMsec(int bufSizeInFrames) const {
    int chunkCount = bufSizeInFrames // frames
                     * 1000 // msec per second
                     / SAMPLE_RATE;  // samples per second
    return chunkCount;
}

void PulseEnhancer::createLimiter(unsigned int samplingRate) {
    mLimiter = new SuperpoweredLimiter(samplingRate);
    mLimiter->ceilingDb = -1.0F;
    mLimiter->thresholdDb = -1.0F;
    mLimiter->releaseSec = 0.20F;
    mLimiter->enable(true);
}








