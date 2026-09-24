#pragma once
#include "core/model/ImprovisationContracts.h"

namespace smartimproviser::harmony
{
// Message/control-thread API: owns strings/vectors; not for the audio callback.
ImprovisationResult analyzeImprovisation(const HarmonicSituation& situation);
}
