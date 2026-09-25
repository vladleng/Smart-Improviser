#pragma once

#include "core/analysis/HarmonicFunction.h"
#include "core/context/HarmonicContext.h"
#include "core/model/AnalysisEvidence.h"
#include "core/model/HarmonicInterpretation.h"
#include "core/model/HarmonicPattern.h"
#include "core/model/KeyCenter.h"
#include "core/model/ResolutionTarget.h"

#include <array>
#include <cstdint>

namespace smartimproviser::harmony
{
constexpr std::size_t kMaxHarmonicInterpretations = 4;
constexpr std::size_t kMaxPatternWindowChords = 9;
constexpr std::size_t kMaxNestedPatterns = 2;

enum class PatternContextStatus : std::uint8_t
{
    none = 0,
    candidate,
    confirmed,
    completed
};

// Stage 3-only bounded reconstruction input. It is deliberately separate from
// TimelineHarmonicSnapshot so the accepted Stage 1 previous/current/next
// contract remains unchanged. The adapter rebuilds it from the host timeline
// on every analysis pass, preventing stale seek/edit/reopen state.
struct PatternTimelineWindow
{
    std::array<ChordContext, kMaxPatternWindowChords> chords {};
    std::uint8_t chordCount = 0;
    int currentIndex = -1;
};

struct PatternContext
{
    bool valid = false;
    HarmonicPattern topLevel;
    NormalizedKey center;
    PatternContextStatus status = PatternContextStatus::none;
    double startPpq = -1.0;
    double resolutionPpq = -1.0;
    std::array<HarmonicPattern, kMaxNestedPatterns> nestedPatterns {};
    std::uint8_t nestedPatternCount = 0;
};

// Accepted Stage 1 -> Stage 2 contract. Do not add arbitrary timeline history
// here: PatternTimelineWindow is a separate Stage 3 analysis input.
struct TimelineHarmonicSnapshot
{
    bool positionAvailable = false;
    double ppq = -1.0;

    bool previousChordAvailable = false;
    ChordContext previousChord;

    ChordContext currentChord;

    bool nextChordAvailable = false;
    ChordContext nextChord;

    KeyContext globalKey;
};

struct HarmonicSituation
{
    bool valid = false;
    double ppq = -1.0;

    bool previousChordAvailable = false;
    NormalizedChord previousChord;

    NormalizedChord currentChord;

    bool nextChordAvailable = false;
    NormalizedChord nextChord;

    KeyCenter globalKey;
    KeyCenter localKey;

    HarmonicAnalysis harmonic;
    HarmonicAnalysis localHarmonic;
    HarmonicPattern pattern;
    HarmonicPattern localPattern;
    PatternContext patternContext;
    ResolutionTarget resolution;
    AnalysisEvidence evidence;

    std::array<HarmonicInterpretation, kMaxHarmonicInterpretations> interpretations {};
    std::uint8_t interpretationCount = 0;
    int primaryInterpretationIndex = -1;
};

HarmonicSituation buildHarmonicSituation(const TimelineHarmonicSnapshot& snapshot) noexcept;

// Stage 2/0.2d host-neutral local-center analyzer. It enriches an already
// normalized HarmonicSituation without consulting JUCE, ARA or DAW state.
void analyzeLocalKeyCenter(HarmonicSituation& situation) noexcept;

// Stage 2/0.2e host-neutral ambiguity layer. It keeps multiple plausible
// interpretations when evidence is insufficient for a single musical reading.
void analyzeAmbiguityAndConfidence(HarmonicSituation& situation) noexcept;
}
