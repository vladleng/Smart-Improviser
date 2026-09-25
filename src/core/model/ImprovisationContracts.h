#pragma once

#include "core/model/AnalysisEvidence.h"
#include "core/model/HarmonicSituation.h"
#include <string>
#include <vector>

#include <cstdint>

namespace smartimproviser::harmony
{
enum class TensionLevel : std::uint8_t
{
    stable = 1,
    color = 2,
    outsideMaximum = 3
};

enum class ImprovisationStrategyKind : std::uint8_t
{
    undefined = 0,
    chordToneBased,
    guideToneResolution,
    diatonicColor,
    lydianDominantColor,
    melodicMinorApplication,
    alteredDominant,
    diminishedDominant,
    tritoneSuperimposition,
    chromaticEnclosure,
    sideSlipOutside,
    diminishedApplication
};

enum class PhraseRole : std::uint8_t
{
    undefined = 0,
    statement,
    development,
    preparation,
    climax,
    resolution,
    release
};

enum class MaterialKind : std::uint8_t { undefined, chordTones, scale };
enum class MaterialNoteRole : std::uint8_t { chordTone, guideTone, colorTone, passingTone, scaleTone };
enum class DiatonicMode : std::uint8_t { none, ionian, dorian, phrygian, lydian, mixolydian, aeolian, locrian };

enum class DominantContext : std::uint8_t
{
    notDominant, unresolved, toMajor, toMinor, toDominant, toOther
};

struct MaterialNote
{
    int pitchClass = -1;
    int semitonesFromRoot = -1;
    int degree = 0;
    MaterialNoteRole role = MaterialNoteRole::chordTone;
    bool characteristic = false; // Explicit extension/alteration or sus-defining tone.
    std::string spelling; // Degree-aware spelling, populated for scale sources.
};

struct SourceMaterial
{
    MaterialKind kind = MaterialKind::undefined;
    DiatonicMode mode = DiatonicMode::none;
    int rootFifths = 0;
    int rootPitchClass = -1; // Source root, never an inferred song key.
    std::string name;
    std::vector<MaterialNote> notes; // Deterministic ascending source-relative intervals.
    std::vector<MaterialNote> chordRelativeNotes; // Same pitches, degrees/spelling relative to actual chord.
};

struct ImprovisationStrategy
{
    ImprovisationStrategyKind kind = ImprovisationStrategyKind::undefined;
    TensionLevel tension = TensionLevel::stable;
    AnalysisEvidence evidence; // Harmonic evidence, not recommendation priority.
    std::string ruleId;
    int ruleVersion = 1;
    int priority = 0; // Higher first; ties resolved by ruleId.
    int interpretationIndex = -1; // -1: no selected interpretation.
    bool interpretationIndependent = false;
    bool tensionClassified = false; // Stage 4 policy; default enum is not a claim.
    NormalizedChord actualChord;
    NormalizedChord thinkingStructure;
    SourceMaterial source;
    ResolutionTarget resolution;
    std::vector<MaterialNote> guideNotes;
    std::vector<MaterialNote> characteristicNotes;
    NormalizedChord nextChord; // Actual next harmony, not necessarily a functional resolution.
    std::vector<MaterialNote> targetNotes; // Explicit tones of nextChord, relative to its root.
    std::vector<ResolutionMove> suggestedTransitions; // Melodic options, never harmonic evidence.
    std::string idea;
    std::string explanation;
    std::string conditions;
    std::string sourceReference;
    std::string usageHint; // Compact diagnostic instruction, not a tension classification.
    std::vector<int> omittedChordTones; // Pitch classes explicitly omitted by this application.
    std::vector<ResolutionMove> sourceTransitions; // Optional color-to-target moves; never harmonic evidence.
};

struct ImprovisationResult
{
    bool valid = false;
    DominantContext dominantContext = DominantContext::notDominant;
    bool secondaryDominant = false;
    bool substituteDominant = false;
    HarmonicSituation context; // Preserve all interpretations, including unresolved primary.
    std::vector<ImprovisationStrategy> strategies;
    std::string contextDescription;
    std::string unavailableReason;
    std::string scaleUnavailableReason;
};
}
