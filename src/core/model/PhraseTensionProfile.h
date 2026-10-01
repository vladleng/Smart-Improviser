#pragma once
#include "core/model/ImprovisationContracts.h"
#include <optional>
#include <vector>

namespace smartimproviser::harmony
{
// Half-open [startBeat, endBeat); null means explicitly unassigned, never T1.
struct TensionSpan
{
    double startBeat = 0.0;
    double endBeat = 0.0;
    std::optional<TensionLevel> level;
};

// Description of an existing phrase, in phrase-relative beats. Not the
// harmonic-situation TensionProfile, and not a desired song curve.
struct PhraseTensionProfile
{
    bool defined = false; // False uses the legacy whole-phrase assignment.
    std::vector<TensionSpan> spans; // Gaps/explicit null spans remain unevaluated.
};
}
