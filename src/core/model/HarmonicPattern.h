#pragma once

#include "core/model/AnalysisEvidence.h"

#include <cstdint>

namespace smartimproviser::harmony
{
enum class HarmonicPatternType : std::uint8_t
{
    undefined = 0,
    none,
    majorIiVI,
    minorIiHalfDimVi,
    dominantToTonic,
    turnaroundIVIiiV,
    secondaryDominant,
    tritoneSubstitution,
    backdoorDominant,
    minorIvToI,
    passingDiminished,
    commonToneDiminished,
    dominantChain,
    modalVamp
};

enum class PatternMemberRole : std::uint8_t
{
    undefined = 0,
    preparation,
    predominant,
    dominant,
    substituteDominant,
    tonic,
    resolution,
    passing
};

struct HarmonicPattern
{
    HarmonicPatternType type = HarmonicPatternType::undefined;
    PatternMemberRole role = PatternMemberRole::undefined;
    int positionIndex = -1;
    int length = 0;
    AnalysisEvidence evidence;

    bool recognized() const noexcept
    {
        return type != HarmonicPatternType::undefined && type != HarmonicPatternType::none;
    }
};
}
