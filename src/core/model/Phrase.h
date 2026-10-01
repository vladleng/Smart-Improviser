#pragma once

#include "core/model/HarmonicPattern.h"
#include "core/model/ImprovisationContracts.h"
#include "core/model/PhraseTensionProfile.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <optional>
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

// Optional performance reference; not part of the musical identity.
struct PhraseFingering
{
    int stringNumber = 0; // 1-based; no tuning/position is invented.
    int fret = -1;
};

// Written example root in MIDI numbering (C4 = 60), separate from pitch class.
// Each used slot needs its own reference to preserve cross-chord contour.
struct PhraseRegisterReference
{
    int chordIndex = -1;
    int rootMidiNote = -1;
};

struct PhraseNote
{
    RelativePitch pitch;
    double beatOffset = 0.0;
    double durationBeats = 0.0;
    bool target = false;
    PhraseNoteRole harmonicRole = PhraseNoteRole::sourceTone;
    // Semitone pitch = slot root + compound degree + accidental + 12 * offset.
    // null means unknown register; zero is an explicit assignment.
    std::optional<int> octaveOffset;
    std::optional<PhraseFingering> fingering;
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
    PhraseTensionProfile tensionProfile; // Description; never the desired curve.
    std::vector<PhraseRegisterReference> registerReferences;
    std::vector<std::string> conceptRuleIds; // Concepts used by this phrase, not UI tags.
};
}
