#pragma once

#include <JuceHeader.h>
#include <vector>

class LLMEffectsAudioProcessor  : public juce::AudioProcessor
{
public:
    LLMEffectsAudioProcessor();
    ~LLMEffectsAudioProcessor() override;

    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;


    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;


    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;


    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //Paremetners
    void setDecayTime   (float newDecayTime);
    void setPreDelay    (float newPreDelay);
    void setSize        (float newSize);
    void setDiffusion   (float newDiffusion);
    void setDensity     (float newDensity);
    void setDamping     (float newDamping);
    void setEQLow       (float newEQLow);
    void setEQMid       (float newEQMid);
    void setEQHigh      (float newEQHigh);
    void setSpread      (float newSpread);
    void setModulation  (float newModulation);
    void setWetDryMix   (float newWetDryMix);

    float getDecayTime()    const { return decayTime; }
    float getPreDelay()     const { return preDelay; }
    float getSize()         const { return size; }
    float getDiffusion()    const { return diffusion; }
    float getDensity()      const { return density; }
    float getDamping()      const { return damping; }
    float getEQLow()        const { return eqLow; }
    float getEQMid()        const { return eqMid; }
    float getEQHigh()       const { return eqHigh; }
    float getSpread()       const { return spread; }
    float getModulation()   const { return modulation; }
    float getWetDryMix()    const { return wetDryMix; }

private:
    // defaults
    float decayTime   { 1.0f };
    float preDelay    { 0.05f };
    float size        { 1.0f };
    float diffusion   { 0.5f };
    float density     { 0.5f };
    float damping     { 0.5f };
    float eqLow       { 0.0f };
    float eqMid       { 0.0f };
    float eqHigh      { 0.0f };
    float spread      { 0.5f };
    float modulation  { 0.0f };
    float wetDryMix   { 0.5f };

    double fs { 44100.0 };

    // For each output channel, we keep a delay buffer and a write position.
    std::vector<std::vector<float>> delayBuffers;
    std::vector<int> writePositions;
    // For smoothing: store last delayed sample per channel.
    std::vector<float> lastDelayedSamples;
    
    // For EQ filtering state per channel
    std::vector<float> eqLowState;
    std::vector<float> eqHighState;
    std::vector<float> eqHighLastInput;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LLMEffectsAudioProcessor)
};
