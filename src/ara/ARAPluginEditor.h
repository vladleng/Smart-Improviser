#pragma once

#include <JuceHeader.h>

class SmartImproviserARAProcessor;

class SmartImproviserARAEditor final : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit SmartImproviserARAEditor(SmartImproviserARAProcessor& processor);
    ~SmartImproviserARAEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override {}

private:
    void timerCallback() override;

    SmartImproviserARAProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SmartImproviserARAEditor)
};
