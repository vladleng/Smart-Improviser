#pragma once

#include "core/model/TensionProfile.h"

namespace smartimproviser::harmony
{
// 0.4b contract adapter. Takes the Core result as given; does not reinterpret
// harmony or classify unclassified source material.
TensionProfile buildTensionProfile(const ImprovisationResult& result);

// 0.4c opt-in Level 1 policy. The ordinary Mixolydian baseline remains a
// separate Core source; V->major T1 is its compatible m6 thinking structure.
// Higher Core levels remain notEvaluated; user labels are a separate policy.
TensionProfile analyzeStableTension(const ImprovisationResult& result);
}
