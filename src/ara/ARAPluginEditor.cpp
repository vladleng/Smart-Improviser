#include "ara/ARAPluginEditor.h"
#include "ara/ARAPluginProcessor.h"
#include "ara/ARAContextDebugState.h"
#include "context/SharedHarmonicContext.h"
#include "context/TimelineContextMapper.h"
#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
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

const char* pitchClassName(int pitchClass) noexcept
{
    static constexpr const char* names[smartimproviser::harmony::kPitchClassCount] =
        { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

    pitchClass %= smartimproviser::harmony::kPitchClassCount;
    if (pitchClass < 0)
        pitchClass += smartimproviser::harmony::kPitchClassCount;
    return names[pitchClass];
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

const char* patternName(smartimproviser::harmony::HarmonicPatternType type) noexcept
{
    using smartimproviser::harmony::HarmonicPatternType;
    switch (type)
    {
        case HarmonicPatternType::none: return "None";
        case HarmonicPatternType::majorIiVI: return "Major ii-V-I";
        case HarmonicPatternType::minorIiHalfDimVi: return "Minor iio-V-i";
        case HarmonicPatternType::dominantToTonic: return "V-I";
        case HarmonicPatternType::turnaroundIVIiiV: return "I-VI-ii-V";
        case HarmonicPatternType::secondaryDominant: return "Secondary dominant";
        case HarmonicPatternType::tritoneSubstitution: return "Tritone substitution";
        case HarmonicPatternType::backdoorDominant: return "Backdoor dominant";
        case HarmonicPatternType::minorIvToI: return "Minor iv-I";
        case HarmonicPatternType::passingDiminished: return "Passing diminished";
        case HarmonicPatternType::commonToneDiminished: return "Common-tone diminished";
        case HarmonicPatternType::dominantChain: return "Dominant chain";
        case HarmonicPatternType::modalVamp: return "Modal vamp";
        case HarmonicPatternType::undefined:
        default: return "-";
    }
}

const char* patternRoleName(smartimproviser::harmony::PatternMemberRole role) noexcept
{
    using smartimproviser::harmony::PatternMemberRole;
    switch (role)
    {
        case PatternMemberRole::preparation: return "Preparation";
        case PatternMemberRole::predominant: return "Predominant";
        case PatternMemberRole::dominant: return "Dominant";
        case PatternMemberRole::substituteDominant: return "Substitute dominant";
        case PatternMemberRole::tonic: return "Tonic";
        case PatternMemberRole::resolution: return "Resolution";
        case PatternMemberRole::passing: return "Passing";
        case PatternMemberRole::undefined:
        default: return "-";
    }
}

const char* confidenceName(smartimproviser::harmony::ConfidenceLevel level) noexcept
{
    using smartimproviser::harmony::ConfidenceLevel;
    switch (level)
    {
        case ConfidenceLevel::low: return "low";
        case ConfidenceLevel::medium: return "medium";
        case ConfidenceLevel::high: return "high";
        case ConfidenceLevel::confirmed: return "confirmed";
        case ConfidenceLevel::unknown:
        default: return "unknown";
    }
}

const char* interpretationName(smartimproviser::harmony::InterpretationStatus status) noexcept
{
    using smartimproviser::harmony::InterpretationStatus;
    switch (status)
    {
        case InterpretationStatus::unique: return "unique";
        case InterpretationStatus::ambiguous: return "AMBIGUOUS";
        case InterpretationStatus::unknown:
        default: return "unknown";
    }
}

const char* keyCenterScopeName(smartimproviser::harmony::KeyCenterScope scope) noexcept
{
    using smartimproviser::harmony::KeyCenterScope;
    switch (scope)
    {
        case KeyCenterScope::global: return "global";
        case KeyCenterScope::local: return "local";
        case KeyCenterScope::temporary: return "temporary";
        case KeyCenterScope::modal: return "modal";
        case KeyCenterScope::undefined:
        default: return "undefined";
    }
}

const char* keyCenterStatusName(smartimproviser::harmony::KeyCenterStatus status) noexcept
{
    using smartimproviser::harmony::KeyCenterStatus;
    switch (status)
    {
        case KeyCenterStatus::candidate: return "candidate";
        case KeyCenterStatus::tonicized: return "tonicized";
        case KeyCenterStatus::established: return "established";
        case KeyCenterStatus::modulationCandidate: return "modulation candidate";
        case KeyCenterStatus::undefined:
        default: return "undefined";
    }
}

juce::String centerKeyDisplayName(const smartimproviser::harmony::KeyCenter& center)
{
    if (! center.valid || ! center.key.valid)
        return "-";

    return utf8String(fifthsName(center.key.rootFifths))
         + " "
         + smartimproviser::harmony::keyModeName(center.key.mode);
}

juce::String localCenterDisplayName(const smartimproviser::harmony::KeyCenter& center)
{
    if (! center.valid || ! center.key.valid)
        return "-";

    return centerKeyDisplayName(center)
         + "  |  "
         + keyCenterScopeName(center.scope)
         + "  |  "
         + keyCenterStatusName(center.status);
}

juce::String harmonicDisplay(const smartimproviser::harmony::HarmonicAnalysis& harmonic)
{
    if (! harmonic.valid)
        return "-";

    return juce::String(smartimproviser::harmony::scaleDegreeName(harmonic.rootScaleDegree))
         + "  |  "
         + smartimproviser::harmony::harmonicFunctionName(harmonic.effectiveFunction);
}

juce::String interpretationDisplay(const smartimproviser::harmony::HarmonicInterpretation& interpretation)
{
    if (! interpretation.valid)
        return "-";

    return juce::String(smartimproviser::harmony::harmonicInterpretationKindName(interpretation.kind))
         + "  |  " + centerKeyDisplayName(interpretation.center)
         + "  |  " + harmonicDisplay(interpretation.harmonic)
         + "  |  " + confidenceName(interpretation.evidence.confidence);
}

juce::String patternPositionDisplay(const smartimproviser::harmony::HarmonicPattern& pattern)
{
    if (! pattern.recognized())
        return "-";

    juce::String value(patternRoleName(pattern.role));
    if (pattern.positionIndex >= 0 && pattern.length > 0)
        value += "  |  " + juce::String(pattern.positionIndex + 1)
              + " / " + juce::String(pattern.length);
    return value;
}

juce::String resolutionDisplay(const smartimproviser::harmony::HarmonicSituation& situation)
{
    if (! situation.resolution.available || ! situation.resolution.targetChord.valid)
        return "-";

    auto value = utf8String(smartimproviser::harmony::normalizedChordSymbol(situation.resolution.targetChord));
    value += situation.resolution.confirmed ? "  |  CONFIRMED" : "  |  expected";
    return value;
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
    setSize(1020, 1010);
    improvisationDetails.setMultiLine(true, true);
    improvisationDetails.setReadOnly(true);
    improvisationDetails.setScrollbarsShown(true);
    improvisationDetails.setCaretVisible(false);
    improvisationDetails.setFont(juce::Font(juce::FontOptions(15.0f)));
    improvisationDetails.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromRGB(42, 46, 53));
    improvisationDetails.setColour(juce::TextEditor::textColourId, juce::Colour::fromRGB(225, 230, 238));
    improvisationDetails.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    improvisationDetails.setBounds(652, 96, 332, 836);
    addAndMakeVisible(improvisationDetails);
    timerCallback();
    startTimerHz(10);
}

void SmartImproviserARAEditor::timerCallback()
{
    cachedShared = SharedHarmonicContextBridge::instance().read();
    const auto ppq = cachedShared.transportAvailable ? cachedShared.transportPpq : -1.0;
    cachedSituation = smartimproviser::harmony::analyzeHarmonicSituation(
        smartimproviser::harmony::mapTimelineHarmonicSnapshot(cachedShared, ppq));
    const auto result = smartimproviser::harmony::analyzeImprovisation(cachedSituation);
    improvisationText = "STAGE 3 / 0.3d\nSource applications / targets\n\n";
    if (! result.valid)
        improvisationText += utf8String(result.unavailableReason);
    else
    {
        const auto& strategy = result.strategies.front();
        improvisationText += "CONTEXT\n" + utf8String(result.contextDescription);
        improvisationText += "\n\nSCALE SOURCES\n";
        bool hasScale = false;
        for (const auto& scalar : result.strategies)
        {
            if (scalar.source.kind != smartimproviser::harmony::MaterialKind::scale) continue;
            if (hasScale) improvisationText += "\n\n";
            hasScale = true;
            improvisationText += utf8String(scalar.source.name) + "\n";
            for (const auto& note : scalar.source.notes) improvisationText += utf8String(note.spelling) + " ";
            if (!scalar.sourceReference.empty())
            {
                improvisationText += "\n" + utf8String(scalar.idea);
                improvisationText += "\nThink: " + utf8String(smartimproviser::harmony::normalizedChordSymbol(scalar.thinkingStructure));
                improvisationText += "\nOn chord: ";
                for (const auto& note : scalar.source.chordRelativeNotes)
                    improvisationText += utf8String(note.spelling) + " ";
            }
            improvisationText += "\n" + utf8String(scalar.usageHint);
            if (!scalar.sourceTransitions.empty())
            {
                improvisationText += "\nOptional color moves: ";
                for (const auto& move : scalar.sourceTransitions)
                    improvisationText += juce::String(pitchClassName(move.fromPitchClass)) + "->"
                        + pitchClassName(move.toPitchClass) + " ";
            }
        }
        if (!hasScale) improvisationText += utf8String(result.scaleUnavailableReason);
        improvisationText += "\n\nCHORD ANCHORS\n";
        for (const auto& note : strategy.source.notes)
            improvisationText += juce::String(pitchClassName(note.pitchClass)) + " ";
        const auto notesText = [](const std::vector<smartimproviser::harmony::MaterialNote>& notes)
        {
            juce::String text;
            for (const auto& note : notes) text += juce::String(pitchClassName(note.pitchClass)) + " ";
            return text.isEmpty() ? juce::String("None") : text;
        };
        improvisationText += "\n(pitch classes)\n\nGUIDE TONES (3 / 7)\n" + notesText(strategy.guideNotes);
        improvisationText += "\n\nCHARACTERISTIC TONES\n" + notesText(strategy.characteristicNotes);
        improvisationText += "\n\nNEXT CHORD TARGETS\n";
        if (strategy.nextChord.valid)
            improvisationText += utf8String(smartimproviser::harmony::normalizedChordSymbol(strategy.nextChord))
                + ": " + notesText(strategy.targetNotes);
        else
            improvisationText += "No next chord";
        const bool confirmed = strategy.resolution.available && strategy.resolution.confirmed;
        improvisationText += confirmed ? "\n\nCONFIRMED RESOLUTION\n" : "\n\nSUGGESTED CONNECTIONS\n";
        const auto moveText = [](const smartimproviser::harmony::ResolutionMove& move)
        {
            return juce::String(pitchClassName(move.fromPitchClass)) + " -> "
                + pitchClassName(move.toPitchClass) + "  ";
        };
        if (confirmed)
        {
            improvisationText += utf8String(smartimproviser::harmony::normalizedChordSymbol(strategy.resolution.targetChord)) + "\n";
            for (std::size_t i = 0; i < strategy.resolution.moveCount; ++i)
                improvisationText += moveText(strategy.resolution.moves[i]);
            if (strategy.resolution.moveCount == 0) improvisationText += "No structural moves available";
        }
        else
        {
            for (const auto& move : strategy.suggestedTransitions) improvisationText += moveText(move);
            if (strategy.suggestedTransitions.empty()) improvisationText += "None";
            improvisationText += "\n(not a confirmed harmonic resolution)";
        }


    }
    if (improvisationDetails.getText() != improvisationText)
    {
        improvisationDetails.setText(improvisationText, false);
        improvisationDetails.moveCaretToTop(false);
    }
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
    g.drawText("Stage 1 Context + Stage 2 Harmonic Engine diagnostics",
               24, 48, 590, 22, juce::Justification::centredLeft);

    const auto debug = ARAContextDebugState::instance().getSnapshot();
    const auto& shared = cachedShared;
    const auto ppq = shared.transportAvailable ? shared.transportPpq : -1.0;
    const auto timeline = smartimproviser::harmony::mapTimelineHarmonicSnapshot(shared, ppq);
    const auto context = smartimproviser::harmony::mapHarmonicContext(shared, ppq);
    const auto& situation = cachedSituation;

    g.setColour(juce::Colour::fromRGB(42, 46, 53));
    g.fillRoundedRectangle(640.0f, 84.0f, 356.0f, 860.0f, 8.0f);
    g.setColour(juce::Colour::fromRGB(225, 230, 238));
    g.setFont(15.0f);


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
            + "  |  transport " + juce::String(static_cast<juce::int64>(shared.transportRevision))); y += 36;

    g.setColour(juce::Colour::fromRGB(77, 81, 89));
    g.drawHorizontalLine(y, 24.0f, 616.0f); y += 12;

    g.setColour(juce::Colour::fromRGB(190, 195, 205));
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText("STAGE 2 - HARMONIC ENGINE", 24, y, 590, 22, juce::Justification::centredLeft); y += 28;

    drawRow(g, y, "Situation", situation.valid ? "VALID" : "NO ANALYSIS", true); y += 24;
    drawRow(g, y, "Global function", harmonicDisplay(situation.harmonic), true); y += 24;

    drawRow(g, y, "Relation",
            situation.valid && situation.harmonic.valid
                ? juce::String(smartimproviser::harmony::harmonicRelationName(situation.harmonic.relation))
                : juce::String("-")); y += 24;

    drawRow(g, y, "Global pattern",
            situation.valid ? juce::String(patternName(situation.pattern.type)) : "-"); y += 24;
    drawRow(g, y, "Pattern position", patternPositionDisplay(situation.pattern)); y += 24;
    drawRow(g, y, "Resolution", resolutionDisplay(situation)); y += 24;

    g.setColour(juce::Colour::fromRGB(77, 81, 89));
    g.drawHorizontalLine(y, 24.0f, 616.0f); y += 10;

    drawRow(g, y, "Local center", localCenterDisplayName(situation.localKey), true); y += 24;
    drawRow(g, y, "Local function", harmonicDisplay(situation.localHarmonic), true); y += 24;
    drawRow(g, y, "Local pattern",
            situation.localPattern.recognized()
                ? juce::String(patternName(situation.localPattern.type))
                : juce::String("-")); y += 24;
    drawRow(g, y, "Local position", patternPositionDisplay(situation.localPattern)); y += 24;

    juce::String localConfidence = "-";
    if (situation.localKey.valid)
    {
        localConfidence = confidenceName(situation.localKey.evidence.confidence);
        localConfidence += "  |  ";
        localConfidence += interpretationName(situation.localKey.evidence.interpretation);
    }
    drawRow(g, y, "Local confidence", localConfidence); y += 28;

    g.setColour(juce::Colour::fromRGB(77, 81, 89));
    g.drawHorizontalLine(y, 24.0f, 616.0f); y += 10;

    juce::String confidence = "-";
    if (situation.valid)
    {
        confidence = confidenceName(situation.evidence.confidence);
        confidence += "  |  ";
        confidence += interpretationName(situation.evidence.interpretation);
        confidence += "  |  candidates " + juce::String(situation.interpretationCount);
    }
    drawRow(g, y, "Interpretation", confidence, true); y += 24;

    juce::String primary = "-";
    if (situation.primaryInterpretationIndex >= 0
        && situation.primaryInterpretationIndex < situation.interpretationCount)
    {
        primary = smartimproviser::harmony::harmonicInterpretationKindName(
            situation.interpretations[static_cast<std::size_t>(situation.primaryInterpretationIndex)].kind);
    }
    else if (situation.valid
             && situation.evidence.interpretation == smartimproviser::harmony::InterpretationStatus::ambiguous)
    {
        primary = "UNRESOLVED";
    }
    drawRow(g, y, "Primary", primary, true); y += 24;

    for (std::uint8_t i = 0; i < situation.interpretationCount; ++i)
    {
        drawRow(g,
                y,
                "Candidate " + juce::String(static_cast<int>(i) + 1),
                interpretationDisplay(situation.interpretations[i]));
        y += 24;
    }

    g.setColour(juce::Colour::fromRGB(105, 110, 120));
    g.setFont(12.5f);
    g.drawText("Diagnostic UI for Stage 1/2/3 validation. Product interface will be developed later.",
               24, getHeight() - 28, getWidth() - 48, 20,
               juce::Justification::centredLeft);
}
