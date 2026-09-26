#pragma once

#include "core/context/HarmonicContext.h"
#include "core/model/AnalysisEvidence.h"
#include "core/model/ChordModel.h"
#include "core/model/HarmonicInterpretation.h"
#include "core/model/HarmonicPattern.h"
#include "core/model/KeyCenter.h"
#include "core/model/KeyModel.h"
#include "core/model/PatternContext.h"
#include "core/model/ResolutionTarget.h"

#include <array>
#include <cstdint>

namespace smartimproviser::harmony
{
struct HarmonicSituation
{
    bool valid = false;
    bool positionAvailable = false;
    double ppq = 0.0;

    KeyCenter globalKey;
    KeyCenter localKey;

    bool previousChordAvailable = false;
    NormalizedChord previousChord;
    NormalizedChord currentChord;
    bool nextChordAvailable = false;
    NormalizedChord nextChord;

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
// 0.3f fix2 may veto provisional cadence candidates when bounded future
// evidence already contradicts their expected tonic.
void analyzeLocalKeyCenter(HarmonicSituation& situation,
                           bool allowIncompleteCadenceCandidates = true) noexcept;

// Stage 2/0.2e host-neutral ambiguity layer. It keeps multiple plausible
// interpretations when evidence is insufficient for a single musical reading.
void analyzeAmbiguityAndConfidence(HarmonicSituation& situation) noexcept;
}
