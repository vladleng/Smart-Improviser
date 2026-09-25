#include "core/analysis/ContextRanking.h"

#include <algorithm>
#include <tuple>

namespace smartimproviser::harmony
{
namespace
{
int interpretationGroup(const ImprovisationResult& result,
                        const ImprovisationStrategy& strategy) noexcept
{
    if (strategy.interpretationIndependent)
        return 0;

    const int primary = result.context.primaryInterpretationIndex;
    if (primary >= 0)
    {
        if (strategy.interpretationIndex == primary)
            return 1;

        // Alternatives are retained after the selected interpretation. Their
        // Stage 2 index is only a stable display order, not a confidence score.
        return 2 + std::max(strategy.interpretationIndex, 0);
    }

    // With unresolved primary all interpretation-specific groups are peers.
    // Stable Stage 2 index ordering must not be read as an implicit winner.
    return 1 + std::max(strategy.interpretationIndex, 0);
}
}

void rankImprovisationStrategies(ImprovisationResult& result) noexcept
{
    std::stable_sort(result.strategies.begin(), result.strategies.end(),
        [&result](const ImprovisationStrategy& a, const ImprovisationStrategy& b)
        {
            const auto groupA = interpretationGroup(result, a);
            const auto groupB = interpretationGroup(result, b);
            if (groupA != groupB)
                return groupA < groupB;

            if (a.priority != b.priority)
                return a.priority > b.priority;

            if (a.ruleId != b.ruleId)
                return a.ruleId < b.ruleId;

            return a.ruleVersion < b.ruleVersion;
        });
}
}
