#pragma once

#include "core/model/HarmonicPattern.h"
#include "core/model/ImprovisationContracts.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace smartimproviser::harmony
{
struct RelativePitch
{
    int chordIndex = -1;
    int degree = 0; // Major-scale degree 1..13 relative to the slot's actual chord.
    int chromaticOffset = 0;
};

enum class PhraseNoteRole : std::uint8_t
{
    sourceTone, chordAnchor, guideTone, characteristicTone, passingApproach,
    resolutionTarget, outsideTone
};

struct PhraseNote
{
    RelativePitch pitch;
    double beatOffset = 0.0;
    double durationBeats = 0.0;
    bool target = false;
    PhraseNoteRole harmonicRole = PhraseNoteRole::sourceTone;
};

// A source application in a harmonic slot, independent of absolute transposition.
struct PhraseSlotRequirement
{
    int chordIndex = -1;
    std::string sourceRuleId;
    int sourceRuleVersion = 0; // Zero accepts the current catalog version.
    int sourceRootOffset = -1; // 0..11 over actual chord; -1 only if unambiguous.
    ChordQuality chordQuality = ChordQuality::undefined;
    ChordQuality destinationQuality = ChordQuality::undefined;
    bool confirmedDestinationRequired = false;
    HarmonicPatternType harmonicPattern = HarmonicPatternType::undefined;
};

// Complete approach/enclosure including its landing, from an existing concept.
struct PhraseApproach
{
    int chordIndex = -1;
    std::string conceptRuleId;
    std::vector<std::size_t> noteIndices;
};

struct Phrase
{
    std::string id;
    std::string name;
    HarmonicPatternType harmonicPattern = HarmonicPatternType::undefined;
    TensionLevel tensionLevel = TensionLevel::stable;
    PhraseRole role = PhraseRole::undefined;
    int startDegree = 0;
    int targetDegree = 0;
    std::vector<PhraseNote> notes;
    AnalysisEvidence evidence;
    bool tensionClassified = false; // Default enum is not an assigned level.
    std::vector<PhraseSlotRequirement> harmonicRequirements;
    std::vector<PhraseApproach> approaches;
};
}
