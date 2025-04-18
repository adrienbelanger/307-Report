#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class LLMEffectsAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                              public juce::Button::Listener,
                                              public juce::TextEditor::Listener,
                                              public juce::Slider::Listener
{
public:
    LLMEffectsAudioProcessorEditor (LLMEffectsAudioProcessor&);
    ~LLMEffectsAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (juce::Button* button) override;
    void textEditorReturnKeyPressed (juce::TextEditor& editor) override;
    void sliderValueChanged (juce::Slider* slider) override;

private:
    LLMEffectsAudioProcessor& audioProcessor;

    //chat
    juce::TextEditor chatHistory;
    juce::TextEditor messageBox;
    juce::TextButton sendButton;

    // knob
    juce::Slider decayTimeSlider      { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider preDelaySlider       { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider sizeSlider           { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider diffusionSlider      { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider densitySlider        { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider dampingSlider        { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider eqLowSlider          { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider eqMidSlider          { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider eqHighSlider         { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider spreadSlider         { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider modulationSlider     { juce::Slider::Rotary, juce::Slider::NoTextBox };
    juce::Slider wetDryMixSlider      { juce::Slider::Rotary, juce::Slider::NoTextBox };

    juce::Label decayTimeLabel        { {}, "Decay Time (s)" };
    juce::Label preDelayLabel         { {}, "Pre-Delay (s)" };
    juce::Label sizeLabel             { {}, "Size" };
    juce::Label diffusionLabel        { {}, "Diffusion" };
    juce::Label densityLabel          { {}, "Density" };
    juce::Label dampingLabel          { {}, "Damping" };
    juce::Label eqLowLabel            { {}, "EQ Low (dB)" };
    juce::Label eqMidLabel            { {}, "EQ Mid (dB)" };
    juce::Label eqHighLabel           { {}, "EQ High (dB)" };
    juce::Label spreadLabel           { {}, "Spread" };
    juce::Label modulationLabel       { {}, "Modulation" };
    juce::Label wetDryMixLabel        { {}, "Wet/Dry Mix" };

    void sendMessage();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LLMEffectsAudioProcessorEditor)
};
