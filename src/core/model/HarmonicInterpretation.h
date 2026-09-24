#pragma once

#include "core/analysis/HarmonicFunction.h"
#include "core/model/AnalysisEvidence.h"
#include "core/model/HarmonicPattern.h"
#include "core/model/KeyCenter.h"

#include <cstdint>

namespace smartimproviser::harmony
{
enum class HarmonicInterpretationKind : std::uint8_t
{
    undefined = 0,
    globalContext,
    localCenter,
    modalInterchange
};

struct HarmonicInterpretation
{
    bool valid = false;
    HarmonicInterpretationKind kind = HarmonicInterpretationKind::undefined;
    KeyCenter center;
    HarmonicAnalysis harmonic;
    HarmonicPattern pattern;
    AnalysisEvidence evidence;
};

inline const char* harmonicInterpretationKindName(HarmonicInterpretationKind kind) noexcept
{
    switch (kind)
    {
        case HarmonicInterpretationKind::globalContext: return "Global";
        case HarmonicInterpretationKind::localCenter: return "Local center";
        case HarmonicInterpretationKind::modalInterchange: return "Modal interchange";
        case HarmonicInterpretationKind::undefined:
        default: return "Undefined";
    }
}
}
