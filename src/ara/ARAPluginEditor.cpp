#include "ara/ARAPluginEditor.h"
#include "ara/ARAPluginProcessor.h"
#include "ara/ARAContextDebugState.h"
#include "context/SharedHarmonicContext.h"
#include "context/TimelineContextMapper.h"
#include "core/model/ChordModel.h"
#include "core/model/KeyModel.h"

#include <algorithm>
#include <cmath>
#include <string>

#ifndef SMART_IMPROVISER_BUILD_VERSION
#define SMART_IMPROVISER_BUILD_VERSION "dev"
#endif

namespace
{
juce::String yesNo(bool value)
{
    return value ? "YES" : "NO";
}

juce::String utf8String(const std::string& value)
{
    return juce::String::fromUTF8(value.c_str());
}

std::string fifthsName(std::int32_t fifths)
{
    switch (fifths)
    {
        case -7: return "Cb";
        case -6: return "Gb";
        case -5: return "Db";
        case -4: return "Ab";
        case -3: return "Eb";
        case -2: return "Bb";
        case -1: return "F";
        case  0: return "C";
        case  1: return "G";
        case  2: return "D";
        case  3: return "A";
        case  4: return "E";
        case  5: return "B";
        case  6: return "F#";
        case  7: return "C#";
        case  8: return "G#";
        case  9: return "D#";
        case 10: return "A#";
        case 11: return "E#";
        default: break;
    }

    static constexpr const char* pitchClassNames[smartimproviser::harmony::kPitchClassCount] =
        { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
    return pitchClassNames[smartimproviser::harmony::circleOfFifthsToPitchClass(fifths)];
}

juce::String chordDisplayName(const smartimproviser::harmony::ChordContext& context)
{
    if (! context.available)
        return "-";

    const auto chord = smartimproviser::harmony::normalizeChord(context);
    return chord.valid
        ? utf8String(smartimproviser::harmony::normalizedChordSymbol(chord))
        : juce::String("(no chord)");
}

juce::String keyDisplayName(const smartimproviser::harmony::KeyContext& context)
{
    if (! context.available)
        return "-";

    const auto key = smartimproviser::harmony::normalizeKey(context);
    if (! key.valid)
        return "(no key)";

    return utf8String(fifthsName(key.rootFifths))
         + " "
         + smartimproviser::harmony::keyModeName(key.mode);
}

double localBpm(const SharedHarmonicContextSnapshot& shared, double ppq) noexcept
{
    const auto count = shared.tempoEntryStoredCount;
    if (! shared.tempoEntriesAvailable || count < 2 || ppq < 0.0)
        return -1.0;

    int right = 1;
    while (right < count && shared.tempoEntries[right].quarterPosition < ppq)
        ++right;
    right = std::clamp(right, 1, count - 1);

    const auto& a = shared.tempoEntries[right - 1];
    const auto& b = shared.tempoEntries[right];
    const auto deltaTime = b.timePosition - a.timePosition;
    const auto deltaQuarter = b.quarterPosition - a.quarterPosition;
    if (std::abs(deltaTime) <= 1.0e-12)
        return -1.0;

    return 60.0 * deltaQuarter / deltaTime;
}

void drawRow(juce::Graphics& g,
             int y,
             const juce::String& label,
             const juce::String& value,
             bool emphasize = false)
{
    g.setColour(juce::Colour::fromRGB(165, 170, 180));
    g.setFont(15.0f);
    g.drawText(label, 24, y, 190, 22, juce::Justification::centredLeft);

    g.setColour(emphasize ? juce::Colour::fromRGB(235, 240, 245)
                          : juce::Colour::fromRGB(205, 210, 220));
    g.setFont(emphasize ? 15.5f : 15.0f);
    g.drawText(value, 220, y, 390, 22, juce::Justification::centredLeft);
}
}

SmartImproviserARAEditor::SmartImproviserARAEditor(SmartImproviserARAProcessor& p)
    : juce::AudioProcessorEditor(p), processor(p)
{
    setSize(640, 570);
    startTimerHz(10);
}

void SmartImproviserARAEditor::timerCallback()
{
    repaint();
}

void SmartImproviserARAEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(31, 33, 37));

    g.setColour(juce::Colour::fromRGB(245, 246, 248));
    g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    g.drawText("Smart Improviser " SMART_IMPROVISER_BUILD_VERSION,
               24, 18, 590, 30, juce::Justification::centredLeft);

    g.setColour(juce::Colour::fromRGB(150, 156, 168));
    g.setFont(14.0f);
    g.drawText("Stage 1 - ARA Context Monitor",
               24, 48, 590, 22, juce::Justification::centredLeft);

    const auto debug = ARAContextDebugState::instance().getSnapshot();
    const auto shared = SharedHarmonicContextBridge::instance().read();
    const auto ppq = shared.transportAvailable ? shared.transportPpq : -1.0;
    const auto timeline = smartimproviser::harmony::mapTimelineHarmonicSnapshot(shared, ppq);
    const auto context = smartimproviser::harmony::mapHarmonicContext(shared, ppq);

    int y = 84;
    drawRow(g, y, "ARA binding", processor.isAraBound() ? "BOUND" : "NOT BOUND", true); y += 25;
    drawRow(g, y, "Document controller", yesNo(debug.documentControllerCreated)); y += 25;
    drawRow(g, y, "Host content access", yesNo(debug.hostContentAccessAvailable)); y += 25;

    juce::String contextsText = juce::String(debug.musicalContextCount);
    if (debug.selectedMusicalContextIndex >= 0)
        contextsText += "  |  selected " + juce::String(debug.selectedMusicalContextIndex + 1);
    drawRow(g, y, "Musical contexts", contextsText); y += 25;
    drawRow(g, y, "Shared context", yesNo(shared.connected), true); y += 34;

    const auto transportText = shared.transportAvailable
        ? (shared.transportPlaying ? "PLAY" : "STOP")
        : "UNAVAILABLE";
    drawRow(g, y, "Transport", transportText, true); y += 25;
    drawRow(g, y, "PPQ", shared.transportAvailable ? juce::String(shared.transportPpq, 3) : "-"); y += 25;
    drawRow(g, y, "Seconds", shared.transportAvailable ? juce::String(shared.transportSeconds, 3) : "-"); y += 34;

    drawRow(g, y, "Key", keyDisplayName(timeline.globalKey), true); y += 25;
    drawRow(g, y, "Previous chord",
            timeline.previousChordAvailable ? chordDisplayName(timeline.previousChord) : "-"); y += 25;
    drawRow(g, y, "Current chord", chordDisplayName(timeline.currentChord), true); y += 25;
    drawRow(g, y, "Next chord",
            timeline.nextChordAvailable ? chordDisplayName(timeline.nextChord) : "-"); y += 25;

    juce::String timeSignature = "-";
    if (context.timeSignature.available)
    {
        timeSignature = juce::String(context.timeSignature.numerator)
                      + "/"
                      + juce::String(context.timeSignature.denominator);
    }
    drawRow(g, y, "Time signature", timeSignature); y += 25;

    const auto bpm = localBpm(shared, ppq);
    drawRow(g, y, "Tempo", bpm > 0.0 ? juce::String(bpm, 2) + " BPM" : "-"); y += 34;

    const auto counts = "Key " + juce::String(shared.keySignatureEventCount)
                      + "  |  Chords " + juce::String(shared.sheetChordEventCount)
                      + "  |  Tempo " + juce::String(shared.tempoEntryEventCount)
                      + "  |  Bars " + juce::String(shared.barSignatureEventCount);
    drawRow(g, y, "ARA events", counts); y += 25;
    drawRow(g, y, "Revisions",
            "harmonic " + juce::String(static_cast<juce::int64>(shared.revision))
            + "  |  transport " + juce::String(static_cast<juce::int64>(shared.transportRevision)));

    g.setColour(juce::Colour::fromRGB(105, 110, 120));
    g.setFont(12.5f);
    g.drawText("Diagnostic UI for Stage 1. Product interface will be developed later.",
               24, getHeight() - 30, getWidth() - 48, 20,
               juce::Justification::centredLeft);
}
