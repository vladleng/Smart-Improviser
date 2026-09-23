#pragma once

#include "core/model/AnalysisEvidence.h"

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
    sideSlipOutside
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

struct ImprovisationStrategy
{
    ImprovisationStrategyKind kind = ImprovisationStrategyKind::undefined;
    TensionLevel tension = TensionLevel::stable;
    AnalysisEvidence evidence;
};
}
