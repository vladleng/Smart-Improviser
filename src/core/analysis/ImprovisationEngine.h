#pragma once
#include "core/model/ImprovisationContracts.h"
#include "core/analysis/Explanation.h"

namespace smartimproviser::harmony
{
// Message/control-thread API: owns strings/vectors; not for the audio callback.
ImprovisationResult analyzeImprovisation(const HarmonicSituation& situation);

// 0.3g presentation API. This consumes the already analyzed result and does not
// run harmonic/context analysis again.
inline ExplanationResult explainImprovisation(const ImprovisationResult& result)
{
    return buildExplanation(result);
}
}
