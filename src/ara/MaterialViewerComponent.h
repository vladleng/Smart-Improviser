#pragma once

#include <JuceHeader.h>
#include "core/analysis/MaterialViewer.h"

class MaterialViewerComponent final : public juce::Component
{
public:
    void showMaterial(smartimproviser::harmony::MaterialView material,
                      smartimproviser::harmony::ViewerLayer visibleLayer,
                      int firstFret, int lastFret);
    void setFretDegreeLabels(bool enabled) { fretDegreeLabels = enabled; repaint(); }
    void paint(juce::Graphics& g) override;

private:
    smartimproviser::harmony::MaterialView view;
    smartimproviser::harmony::ViewerLayer layer = smartimproviser::harmony::ViewerLayer::all;
    int fretStart = 0, fretEnd = 12;
    bool fretDegreeLabels = false;
};
