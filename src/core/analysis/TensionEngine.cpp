#include "core/analysis/TensionEngine.h"

namespace smartimproviser::harmony
{
TensionProfile buildTensionProfile(const ImprovisationResult& result)
{
    TensionProfile profile;
    profile.context = result.context;
    profile.valid = result.valid && result.context.valid;
    if (!profile.valid) return profile;

    for (const auto& strategy : result.strategies)
    {
        // Default TensionLevel::stable is a storage value, not T1 evidence.
        if (!strategy.tensionClassified) continue;
        const auto* band = profile.band(strategy.tension);
        if (!band) continue;
        auto& destination = profile.bands[static_cast<unsigned>(strategy.tension) - 1];
        destination.assessment = TensionAssessment::supported;
        destination.alternatives.push_back({strategy, {}});
    }
    return profile;
}
}
