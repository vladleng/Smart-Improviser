#pragma once

#include <JuceHeader.h>
#include "context/SharedHarmonicContextData.h"
#include "core/model/HarmonicSituation.h"
#include "core/analysis/Explanation.h"
#include "ara/MaterialViewerComponent.h"

#include <string>
#include <vector>

class SmartImproviserARAProcessor;

class SmartImproviserARAEditor final : public juce::AudioProcessorEditor,
                                       private juce::Timer,
                                       private juce::ListBoxModel
{
public:
    explicit SmartImproviserARAEditor(SmartImproviserARAProcessor& processor);
    ~SmartImproviserARAEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    enum class Panel
    {
        context,
        material,
        sources,
        harmonic,
        ara
    };

    void timerCallback() override;
    void setActivePanel(Panel panel);
    void refreshPanelView(bool resetScroll);
    void updatePanelButtons();
    void updateMaterialSelection();
    juce::String selectedMaterialText() const;
    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height,
                          bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& event) override;
    void selectedRowsChanged(int row) override;
    void selectMaterial(int index);
    void openTensionMenu(int index, const juce::MouseEvent& event);
    void rebuildStrategyRows();
    int rowForMaterial(int index) const;
    bool redundantMaterial(int index) const;
    bool isBaseMode(int index) const;
    std::string tensionKey(int index) const;
    int manualTension(int index) const;

    SmartImproviserARAProcessor& processor;
    SharedHarmonicContextSnapshot cachedShared;
    smartimproviser::harmony::HarmonicSituation cachedSituation;
    smartimproviser::harmony::ImprovisationResult cachedResult;
    smartimproviser::harmony::ExplanationResult cachedExplanation;
    juce::String selectedMaterialKey;
    juce::StringArray strategyLabels;
    struct StrategyRow { int materialIndex; int level; bool baseMode; };
    std::vector<StrategyRow> strategyRows;
    int selectedMaterialIndex = -1;
    bool updatingSelector = false;

    Panel activePanel = Panel::context;

    juce::String summaryContext;
    juce::AttributedString summaryContextDisplay;
    juce::String summaryMeta;
    juce::String summaryGlobalFunction;
    juce::String summaryLocal;
    juce::String summaryPattern;
    juce::AttributedString summaryPatternDisplay;
    struct PatternDisplayMember
    {
        juce::String roman;
        juce::String chord;
        bool current = false;
        bool expected = false;
    };
    std::vector<PatternDisplayMember> patternMembers;
    int patternDisplayPosition = -1;
    juce::String materialText;
    juce::String sourcesText;
    juce::String harmonicText;
    juce::String araText;

    juce::TextButton contextButton;
    juce::TextButton materialButton;
    juce::TextButton sourcesButton;
    juce::TextButton harmonicButton;
    juce::TextButton araButton;
    juce::TextEditor detailsView;
    juce::ListBox strategyList;
    juce::TextButton fretLabelButton;
    juce::ComboBox layerSelector;
    juce::ComboBox fretSelector;
    MaterialViewerComponent materialViewer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SmartImproviserARAEditor)
};
