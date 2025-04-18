#include "PluginProcessor.h"
#include "PluginEditor.h"

LLMEffectsAudioProcessorEditor::LLMEffectsAudioProcessorEditor (LLMEffectsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (900, 500);

    
    chatHistory.setMultiLine(true);
    chatHistory.setReadOnly(true);
    chatHistory.setScrollbarsShown(true);
    chatHistory.setText("Hey! Describe how you'd like your reverb to sound.\n");
    addAndMakeVisible(chatHistory);

    messageBox.setMultiLine(false);
    messageBox.setReturnKeyStartsNewLine(false);
    messageBox.setScrollbarsShown(true);
    messageBox.addListener(this);
    addAndMakeVisible(messageBox);

    sendButton.setButtonText("Send");
    sendButton.addListener(this);
    addAndMakeVisible(sendButton);

    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& name)
    {
        slider.setSliderStyle(juce::Slider::Rotary);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
        slider.addListener(this);
        addAndMakeVisible(slider);

        label.setText(name, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    setupSlider(decayTimeSlider,   decayTimeLabel,   "Decay Time (s)");
    decayTimeSlider.setRange(0.1, 5.0, 0.01);
    decayTimeSlider.setValue(audioProcessor.getDecayTime());

    setupSlider(preDelaySlider,    preDelayLabel,    "Pre-Delay (s)");
    preDelaySlider.setRange(0.0, 0.5, 0.001);
    preDelaySlider.setValue(audioProcessor.getPreDelay());

    setupSlider(sizeSlider,        sizeLabel,        "Size");
    sizeSlider.setRange(0.5, 2.0, 0.01);
    sizeSlider.setValue(audioProcessor.getSize());

    setupSlider(diffusionSlider,   diffusionLabel,   "Diffusion");
    diffusionSlider.setRange(0.0, 1.0, 0.01);
    diffusionSlider.setValue(audioProcessor.getDiffusion());

    setupSlider(densitySlider,     densityLabel,     "Density");
    densitySlider.setRange(0.0, 1.0, 0.01);
    densitySlider.setValue(audioProcessor.getDensity());

    setupSlider(dampingSlider,     dampingLabel,     "Damping");
    dampingSlider.setRange(0.0, 1.0, 0.01);
    dampingSlider.setValue(audioProcessor.getDamping());

    setupSlider(eqLowSlider, eqLowLabel, "EQ Low (dB)");
    eqLowSlider.setRange(-12.0, 12.0, 0.1);
    eqLowSlider.setValue(audioProcessor.getEQLow());

    setupSlider(eqMidSlider, eqMidLabel, "EQ Mid (dB)");
    eqMidSlider.setRange(-12.0, 12.0, 0.1);
    eqMidSlider.setValue(audioProcessor.getEQMid());

    setupSlider(eqHighSlider, eqHighLabel, "EQ High (dB)");
    eqHighSlider.setRange(-12.0, 12.0, 0.1);
    eqHighSlider.setValue(audioProcessor.getEQHigh());

    setupSlider(spreadSlider,      spreadLabel,      "Spread");
    spreadSlider.setRange(0.0, 1.0, 0.01);
    spreadSlider.setValue(audioProcessor.getSpread());

    setupSlider(modulationSlider,  modulationLabel,  "Modulation");
    modulationSlider.setRange(0.0, 10.0, 0.1);
    modulationSlider.setValue(audioProcessor.getModulation());

    setupSlider(wetDryMixSlider,   wetDryMixLabel,   "Wet/Dry");
    wetDryMixSlider.setRange(0.0, 1.0, 0.01);
    wetDryMixSlider.setValue(audioProcessor.getWetDryMix());
}

LLMEffectsAudioProcessorEditor::~LLMEffectsAudioProcessorEditor() {}

void LLMEffectsAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
}

void LLMEffectsAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    
    auto chatArea = bounds.removeFromLeft(bounds.getWidth() * 0.6);
    chatHistory.setBounds(chatArea.removeFromTop(getHeight() - 50).reduced(10));
    auto bottomChat = chatArea.reduced(10);
    messageBox.setBounds(bottomChat.removeFromLeft(chatArea.getWidth() - 80));
    sendButton.setBounds(bottomChat);

    
    auto reverbArea = bounds.reduced(10);
    int numCols = 4;
    int numRows = 3;
    int sliderWidth = reverbArea.getWidth() / numCols;
    int sliderHeight = reverbArea.getHeight() / numRows;
    int labelHeight = 20;
    int sliderAreaHeight = sliderHeight - labelHeight;
    
    struct KnobLayout { juce::Slider* slider; juce::Label* label; };
    KnobLayout knobs[] = {
        { &decayTimeSlider,   &decayTimeLabel },
        { &preDelaySlider,    &preDelayLabel },
        { &sizeSlider,        &sizeLabel },
        { &diffusionSlider,   &diffusionLabel },
        { &densitySlider,     &densityLabel },
        { &dampingSlider,     &dampingLabel },
        { &eqLowSlider,       &eqLowLabel },
        { &eqMidSlider,       &eqMidLabel },
        { &eqHighSlider,      &eqHighLabel },
        { &spreadSlider,      &spreadLabel },
        { &modulationSlider,  &modulationLabel },
        { &wetDryMixSlider,   &wetDryMixLabel }
    };
    int numKnobs = sizeof(knobs) / sizeof(knobs[0]);
    for (int i = 0; i < numKnobs; ++i)
    {
        int row = i / numCols;
        int col = i % numCols;
        int x = reverbArea.getX() + col * sliderWidth;
        int y = reverbArea.getY() + row * sliderHeight;
        knobs[i].label->setBounds(x, y, sliderWidth, labelHeight);
        knobs[i].slider->setBounds(x, y + labelHeight, sliderWidth, sliderAreaHeight);
    }
}

void LLMEffectsAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &sendButton)
        sendMessage();
}

void LLMEffectsAudioProcessorEditor::textEditorReturnKeyPressed (juce::TextEditor& editor)
{
    if (&editor == &messageBox)
        sendMessage();
}

void LLMEffectsAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &decayTimeSlider)
        audioProcessor.setDecayTime((float)decayTimeSlider.getValue());
    else if (slider == &preDelaySlider)
        audioProcessor.setPreDelay((float)preDelaySlider.getValue());
    else if (slider == &sizeSlider)
        audioProcessor.setSize((float)sizeSlider.getValue());
    else if (slider == &diffusionSlider)
        audioProcessor.setDiffusion((float)diffusionSlider.getValue());
    else if (slider == &densitySlider)
        audioProcessor.setDensity((float)densitySlider.getValue());
    else if (slider == &dampingSlider)
        audioProcessor.setDamping((float)dampingSlider.getValue());
    else if (slider == &eqLowSlider)
        audioProcessor.setEQLow((float)eqLowSlider.getValue());
    else if (slider == &eqMidSlider)
        audioProcessor.setEQMid((float)eqMidSlider.getValue());
    else if (slider == &eqHighSlider)
        audioProcessor.setEQHigh((float)eqHighSlider.getValue());
    else if (slider == &spreadSlider)
        audioProcessor.setSpread((float)spreadSlider.getValue());
    else if (slider == &modulationSlider)
        audioProcessor.setModulation((float)modulationSlider.getValue());
    else if (slider == &wetDryMixSlider)
        audioProcessor.setWetDryMix((float)wetDryMixSlider.getValue());
}

//llm
void LLMEffectsAudioProcessorEditor::sendMessage()
{
    juce::String userMessage = messageBox.getText().trim();
    if (userMessage.isNotEmpty())
    {
        chatHistory.moveCaretToEnd();
        chatHistory.insertTextAtCaret("\nYou: " + userMessage + "\n");
        messageBox.clear();
        
        juce::DynamicObject::Ptr rootObj = new juce::DynamicObject();
        juce::DynamicObject::Ptr paramsObj = new juce::DynamicObject();
        paramsObj->setProperty("decayTime",   audioProcessor.getDecayTime());
        paramsObj->setProperty("preDelay",    audioProcessor.getPreDelay());
        paramsObj->setProperty("size",        audioProcessor.getSize());
        paramsObj->setProperty("diffusion",   audioProcessor.getDiffusion());
        paramsObj->setProperty("density",     audioProcessor.getDensity());
        paramsObj->setProperty("damping",     audioProcessor.getDamping());
        paramsObj->setProperty("eqLow",       audioProcessor.getEQLow());
        paramsObj->setProperty("eqMid",       audioProcessor.getEQMid());
        paramsObj->setProperty("eqHigh",      audioProcessor.getEQHigh());
        paramsObj->setProperty("spread",      audioProcessor.getSpread());
        paramsObj->setProperty("modulation",  audioProcessor.getModulation());
        paramsObj->setProperty("wetDryMix",   audioProcessor.getWetDryMix());
        rootObj->setProperty("currentParameters", juce::var(paramsObj.get()));
        rootObj->setProperty("userPrompt", userMessage);
        juce::String payload = juce::JSON::toString(juce::var(rootObj.get()));
        
        juce::String apiKey = "YOUR KEY HERE";
        juce::String url = "https://api.openai.com/v1/chat/completions";
        juce::String model = "gpt-4o-mini";
        
        juce::String systemMessage = "You are an audio plugin parameter modifier. When given a JSON payload containing 'currentParameters' and 'userPrompt', respond strictly with a valid JSON object containing exactly two keys: 'parameters' and 'explanation'. The 'parameters' object must include only numeric values for the reverb parameters, and the 'explanation' should be a concise string that describes what changes you made. Do not include any additional text, markdown formatting, or commentary outside of the JSON. Make sure the explanation clearly states what you did.";
        
        juce::String userContent = juce::JSON::toString(payload);
        
        juce::String fullPayload = "{\"model\": \"" + model + "\", \"messages\": ["
            "{\"role\": \"system\", \"content\": " + juce::JSON::toString(systemMessage) + "},"
            "{\"role\": \"user\", \"content\": " + juce::JSON::toString(userContent) + "}"
            "] }";
        
        juce::URL requestURL(url);
        requestURL = requestURL.withPOSTData(fullPayload);
        
        juce::String extraHeaders = "Content-Type: application/json\r\nAuthorization: Bearer " + apiKey;
        
        std::unique_ptr<juce::InputStream> stream(requestURL.createInputStream(
            juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
                .withExtraHeaders(extraHeaders)
                .withConnectionTimeoutMs(10000)
                .withNumRedirectsToFollow(5)
        ));
        
        if (stream)
        {
            juce::String response = stream->readEntireStreamAsString();
            juce::var jsonResponse = juce::JSON::parse(response);
            if (jsonResponse.isObject())
            {
                auto* jsonObject = jsonResponse.getDynamicObject();
                juce::var choices = jsonObject->getProperty("choices");
                if (choices.isArray() && choices.getArray()->size() > 0)
                {
                    juce::var messageObj = (*choices.getArray())[0].getProperty("message", juce::var());
                    if (messageObj.isObject())
                    {
                        auto* messageDynamic = messageObj.getDynamicObject();
                        juce::String llmResponse = messageDynamic->getProperty("content").toString();
                        juce::var responseJson = juce::JSON::parse(llmResponse);
                        if (responseJson.isObject())
                        {
                            auto* respObj = responseJson.getDynamicObject();
                            juce::var newParams = respObj->getProperty("parameters");
                            juce::var explanation = respObj->getProperty("explanation");
                            
                            if (!explanation.toString().isEmpty())
                            {
                                if (newParams.isObject())
                                {
                                    auto* pObj = newParams.getDynamicObject();
                                    if (pObj->hasProperty("decayTime"))
                                        audioProcessor.setDecayTime((float) static_cast<double>(pObj->getProperty("decayTime")));
                                    if (pObj->hasProperty("preDelay"))
                                        audioProcessor.setPreDelay((float) static_cast<double>(pObj->getProperty("preDelay")));
                                    if (pObj->hasProperty("size"))
                                        audioProcessor.setSize((float) static_cast<double>(pObj->getProperty("size")));
                                    if (pObj->hasProperty("diffusion"))
                                        audioProcessor.setDiffusion((float) static_cast<double>(pObj->getProperty("diffusion")));
                                    if (pObj->hasProperty("density"))
                                        audioProcessor.setDensity((float) static_cast<double>(pObj->getProperty("density")));
                                    if (pObj->hasProperty("damping"))
                                        audioProcessor.setDamping((float) static_cast<double>(pObj->getProperty("damping")));
                                    if (pObj->hasProperty("eqLow"))
                                        audioProcessor.setEQLow((float) static_cast<double>(pObj->getProperty("eqLow")));
                                    if (pObj->hasProperty("eqMid"))
                                        audioProcessor.setEQMid((float) static_cast<double>(pObj->getProperty("eqMid")));
                                    if (pObj->hasProperty("eqHigh"))
                                        audioProcessor.setEQHigh((float) static_cast<double>(pObj->getProperty("eqHigh")));
                                    if (pObj->hasProperty("spread"))
                                        audioProcessor.setSpread((float) static_cast<double>(pObj->getProperty("spread")));
                                    if (pObj->hasProperty("modulation"))
                                        audioProcessor.setModulation((float) static_cast<double>(pObj->getProperty("modulation")));
                                    if (pObj->hasProperty("wetDryMix"))
                                        audioProcessor.setWetDryMix((float) static_cast<double>(pObj->getProperty("wetDryMix")));
                                }
                                
                                decayTimeSlider.setValue(audioProcessor.getDecayTime());
                                preDelaySlider.setValue(audioProcessor.getPreDelay());
                                sizeSlider.setValue(audioProcessor.getSize());
                                diffusionSlider.setValue(audioProcessor.getDiffusion());
                                densitySlider.setValue(audioProcessor.getDensity());
                                dampingSlider.setValue(audioProcessor.getDamping());
                                eqLowSlider.setValue(audioProcessor.getEQLow());
                                eqMidSlider.setValue(audioProcessor.getEQMid());
                                eqHighSlider.setValue(audioProcessor.getEQHigh());
                                spreadSlider.setValue(audioProcessor.getSpread());
                                modulationSlider.setValue(audioProcessor.getModulation());
                                wetDryMixSlider.setValue(audioProcessor.getWetDryMix());
                                
                                juce::MessageManager::callAsync([this, explanation]()
                                {
                                    chatHistory.moveCaretToEnd();
                                    chatHistory.insertTextAtCaret("\nLLM Explanation: " + juce::String(explanation.toString()) + "\n");
                                });
                            }
                            else
                            {
                                juce::MessageManager::callAsync([this]()
                                {
                                    chatHistory.moveCaretToEnd();
                                    chatHistory.insertTextAtCaret("\nLLM did not provide an explanation.\n");
                                });
                            }
                        }
                        else
                        {
                            juce::MessageManager::callAsync([this, llmResponse]()
                            {
                                chatHistory.moveCaretToEnd();
                                chatHistory.insertTextAtCaret("\nLLM response is not valid JSON: " + llmResponse + "\n");
                            });
                        }
                    }
                }
            }
        }
        else
        {
            juce::MessageManager::callAsync([this]()
            {
                chatHistory.moveCaretToEnd();
                chatHistory.insertTextAtCaret("\nFailed to connect to LLM API.\n");
            });
        }
    }
}
