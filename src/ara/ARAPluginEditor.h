#pragma once

#include <JuceHeader.h>
#include "context/SharedHarmonicContextData.h"
#include "core/model/HarmonicSituation.h"

class SmartImproviserARAProcessor;

class SmartImproviserARAEditor final : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit SmartImproviserARAEditor(SmartImproviserARAProcessor& processor);
    ~SmartImproviserARAEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    enum class Panel
    {
        material,
        sources,
        harmonic,
        ara
    };

    void timerCallback() override;
    void setActivePanel(Panel panel);
    void refreshPanelView(bool resetScroll);
    void updatePanelButtons();

    SmartImproviserARAProcessor& processor;
    SharedHarmonicContextSnapshot cachedShared;
    smartimproviser::harmony::HarmonicSituation cachedSituation;

    Panel activePanel = Panel::material;

    juce::String summaryContext;
    juce::String summaryMeta;
    juce::String materialText;
    juce::String sourcesText;
    juce::String harmonicText;
    juce::String araText;

    juce::TextButton materialButton;
    juce::TextButton sourcesButton;
    juce::TextButton harmonicButton;
    juce::TextButton araButton;
    juce::TextEditor detailsView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SmartImproviserARAEditor)
};
