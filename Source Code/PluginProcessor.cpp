#include "PluginProcessor.h"
#include "PluginEditor.h"

LLMEffectsAudioProcessor::LLMEffectsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
     )
#endif
{
}

LLMEffectsAudioProcessor::~LLMEffectsAudioProcessor() {}

const juce::String LLMEffectsAudioProcessor::getName() const { return JucePlugin_Name; }

bool LLMEffectsAudioProcessor::acceptsMidi() const { return false; }
bool LLMEffectsAudioProcessor::producesMidi() const { return false; }
bool LLMEffectsAudioProcessor::isMidiEffect() const { return false; }
double LLMEffectsAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int LLMEffectsAudioProcessor::getNumPrograms() { return 1; }
int LLMEffectsAudioProcessor::getCurrentProgram() { return 0; }
void LLMEffectsAudioProcessor::setCurrentProgram (int index) {}
const juce::String LLMEffectsAudioProcessor::getProgramName (int index) { return {}; }
void LLMEffectsAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void LLMEffectsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    fs = sampleRate;
    int maxDelaySamples = static_cast<int>(sampleRate * 2.0);
    int numChannels = getTotalNumOutputChannels();
    delayBuffers.clear();
    writePositions.clear();
    lastDelayedSamples.clear();
    delayBuffers.resize(numChannels);
    writePositions.resize(numChannels, 0);
    lastDelayedSamples.resize(numChannels, 0.0f);
    
    eqLowState.clear();
    eqHighState.clear();
    eqHighLastInput.clear();
    eqLowState.resize(numChannels, 0.0f);
    eqHighState.resize(numChannels, 0.0f);
    eqHighLastInput.resize(numChannels, 0.0f);
    
    for (int ch = 0; ch < numChannels; ++ch)
    {
        delayBuffers[ch].resize(maxDelaySamples, 0.0f);
    }
}

void LLMEffectsAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LLMEffectsAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
  #if ! JucePlugin_IsSynth
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
  #endif
    return true;
#endif
}
#endif


void LLMEffectsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    int totalNumInputChannels  = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();
    int numSamples = buffer.getNumSamples();

    
    
    float fc_low = 200.0f;
    float a_low = std::exp(-2.0f * juce::MathConstants<float>::pi * fc_low / (float)fs);
    
    float fc_high = 3000.0f;
    float a_high = std::exp(-2.0f * juce::MathConstants<float>::pi * fc_high / (float)fs);

    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        auto& delayBuffer = delayBuffers[channel];
        int bufferSize = static_cast<int>(delayBuffer.size());
        int& writePos = writePositions[channel];

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float in = channelData[sample];

            int preDelaySamples = static_cast<int>(preDelay * fs);
            int sizeDelaySamples = static_cast<int>(size * decayTime * fs / 2.0f);
            float mod = std::sin(2.0f * juce::MathConstants<float>::pi * modulation * sample / (float)fs);
            int modulatedDelay = preDelaySamples + sizeDelaySamples + static_cast<int>(mod * 10.0f);
            modulatedDelay = juce::jlimit(1, bufferSize - 1, modulatedDelay);

            int readPos = (writePos + bufferSize - modulatedDelay) % bufferSize;
            float delayedSample = delayBuffer[readPos];

            float smoothing = juce::jmap(diffusion, 0.0f, 1.0f, 0.8f, 0.95f);
            float dampedSample = damping * delayedSample + (1.0f - damping) * lastDelayedSamples[channel];
            dampedSample = smoothing * dampedSample + (1.0f - smoothing) * delayedSample;
            lastDelayedSamples[channel] = dampedSample;


            
            float feedbackGain = juce::jmap(decayTime * density, 0.1f, 5.0f, 0.3f, 0.9f);
            delayBuffer[writePos] = in + feedbackGain * dampedSample;
            

            







            // eq
            float lowOut = (1.0f - a_low) * dampedSample + a_low * eqLowState[channel];
            eqLowState[channel] = lowOut;

            
            
            float highOut = a_high * (eqHighState[channel] + dampedSample - eqHighLastInput[channel]);
            eqHighState[channel] = highOut;
            eqHighLastInput[channel] = dampedSample;
            
            
            float midOut = dampedSample - lowOut - highOut;
            
            
            float lowGain = std::pow(10.0f, eqLow / 20.0f);
            float midGain = std::pow(10.0f, eqMid / 20.0f);
            float highGain = std::pow(10.0f, eqHigh / 20.0f);
            
            
            float wetEQ = lowOut * lowGain + midOut * midGain + highOut * highGain;
            
            
            channelData[sample] = (1.0f - wetDryMix) * in + wetDryMix * wetEQ;

            
            writePos = (writePos + 1) % bufferSize;
        }
    }
    
    
    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear(channel, 0, numSamples);
}

bool LLMEffectsAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* LLMEffectsAudioProcessor::createEditor()
{
    return new LLMEffectsAudioProcessorEditor (*this);
}

void LLMEffectsAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {}
void LLMEffectsAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {}

void LLMEffectsAudioProcessor::setDecayTime (float newDecayTime)
{
    decayTime = juce::jlimit(0.1f, 5.0f, newDecayTime);
}
void LLMEffectsAudioProcessor::setPreDelay (float newPreDelay)
{
    preDelay = juce::jlimit(0.0f, 0.5f, newPreDelay);
}
void LLMEffectsAudioProcessor::setSize (float newSize)
{
    size = juce::jlimit(0.5f, 2.0f, newSize);
}
void LLMEffectsAudioProcessor::setDiffusion (float newDiffusion)
{
    diffusion = juce::jlimit(0.0f, 1.0f, newDiffusion);
}
void LLMEffectsAudioProcessor::setDensity (float newDensity)
{
    density = juce::jlimit(0.0f, 1.0f, newDensity);
}
void LLMEffectsAudioProcessor::setDamping (float newDamping)
{
    damping = juce::jlimit(0.0f, 1.0f, newDamping);
}
void LLMEffectsAudioProcessor::setEQLow (float newEQLow)
{
    eqLow = juce::jlimit(-12.0f, 12.0f, newEQLow);
}
void LLMEffectsAudioProcessor::setEQMid (float newEQMid)
{
    eqMid = juce::jlimit(-12.0f, 12.0f, newEQMid);
}
void LLMEffectsAudioProcessor::setEQHigh (float newEQHigh)
{
    eqHigh = juce::jlimit(-12.0f, 12.0f, newEQHigh);
}
void LLMEffectsAudioProcessor::setSpread (float newSpread)
{
    spread = juce::jlimit(0.0f, 1.0f, newSpread);
}
void LLMEffectsAudioProcessor::setModulation (float newModulation)
{
    modulation = newModulation;
}
void LLMEffectsAudioProcessor::setWetDryMix (float newWetDryMix)
{
    wetDryMix = juce::jlimit(0.0f, 1.0f, newWetDryMix);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LLMEffectsAudioProcessor();
}
