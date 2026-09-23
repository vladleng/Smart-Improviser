#pragma once

#include "core/model/ChordModel.h"
#include "core/model/KeyModel.h"

#include <cstdint>

namespace smartimproviser::harmony
{
enum class HarmonicFunction : std::uint8_t
{
    undefined = 0,
    tonic,
    predominant,
    dominant,
    other
};

enum class HarmonicRelation : std::uint8_t
{
    undefined = 0,
    diatonic,
    chromatic
};

struct HarmonicAnalysis
{
    bool valid = false;

    int rootScaleDegree = 0;
    HarmonicFunction rootFunction = HarmonicFunction::undefined;
    HarmonicFunction effectiveFunction = HarmonicFunction::undefined;
    HarmonicRelation relation = HarmonicRelation::undefined;
    bool chordTonesDiatonic = false;

    bool appliedDominantCandidate = false;
    int appliedTargetPitchClass = -1;
    int appliedTargetScaleDegree = 0;

    bool nextChordAvailable = false;
    int nextChordRootPitchClass = -1;
    ChordQuality nextChordQuality = ChordQuality::undefined;
    bool appliedDominantConfirmed = false;

    bool dominantResolutionConfirmed = false;
    int dominantTargetPitchClass = -1;
    ChordQuality dominantTargetQuality = ChordQuality::undefined;

    bool modalInterchangeCandidate = false;
    KeyMode modalInterchangeSource = KeyMode::undefined;
};

HarmonicAnalysis analyzeHarmonicFunction(const NormalizedChord& chord,
                                         const NormalizedKey& key) noexcept;
HarmonicAnalysis analyzeHarmonicFunction(const NormalizedChord& chord,
                                         const NormalizedKey& key,
                                         const NormalizedChord& nextChord) noexcept;

const char* harmonicFunctionName(HarmonicFunction function) noexcept;
const char* harmonicRelationName(HarmonicRelation relation) noexcept;
const char* scaleDegreeName(int degree) noexcept;
}
