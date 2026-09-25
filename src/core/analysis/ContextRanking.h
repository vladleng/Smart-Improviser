#pragma once

#include "core/model/ImprovisationContracts.h"

namespace smartimproviser::harmony
{
// Stage 3 / 0.3f: deterministic presentation order only. Harmonic confidence
// remains in AnalysisEvidence and is never converted into recommendation priority.
void rankImprovisationStrategies(ImprovisationResult& result) noexcept;
}
