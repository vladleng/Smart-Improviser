#pragma once

#include "core/analysis/HarmonicFunction.h"
#include "core/context/HarmonicContext.h"
#include "core/model/AnalysisEvidence.h"
#include "core/model/HarmonicPattern.h"
#include "core/model/KeyCenter.h"
#include "core/model/ResolutionTarget.h"

namespace smartimproviser::harmony
{
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
    ResolutionTarget resolution;
    AnalysisEvidence evidence;
};

HarmonicSituation buildHarmonicSituation(const TimelineHarmonicSnapshot& snapshot) noexcept;

// Stage 2/0.2d host-neutral local-center analyzer. It enriches an already
// normalized HarmonicSituation without consulting JUCE, ARA or DAW state.
void analyzeLocalKeyCenter(HarmonicSituation& situation) noexcept;
}
