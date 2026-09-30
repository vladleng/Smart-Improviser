#pragma once

#include "core/analysis/Explanation.h"
#include <string>

namespace smartimproviser::harmony
{
// A user preference for a source in a harmonic situation, independent of
// absolute pitch, host key spelling, interpretation index and actual target.
// This never contributes harmonic evidence or an automatic T1/T2/T3 claim.
inline std::string manualTensionKey(const ImprovisationResult& result,
                                    const ExplanationItem& item)
{
    if (item.strategyIndices.empty()
        || item.strategyIndices.front() >= result.strategies.size()
        || !item.actualChord.valid)
        return {};

    const auto& strategy = result.strategies[item.strategyIndices.front()];
    const auto& situation = result.context;
    const auto& chord = item.actualChord;
    auto wrap = [](int value) { return (value % 12 + 12) % 12; };
    const bool majorIiV = situation.incompleteCadence.valid
        || situation.pattern.type == HarmonicPatternType::majorIiVI
        || situation.localPattern.type == HarmonicPatternType::majorIiVI
        || situation.pattern.type == HarmonicPatternType::majorIiiViIiV;
    std::string context = majorIiV ? "major-ii-v"
        : result.dominantContext == DominantContext::toMajor ? "v-to-major"
        : result.dominantContext == DominantContext::toMinor ? "v-to-minor"
        : result.dominantContext == DominantContext::toDominant ? "v-to-dominant"
        : "other";
    // Do not make a borrowed iv and a plain m7 share a preference.
    if (!majorIiV && !item.interpretationIndices.empty())
        for (const auto index : item.interpretationIndices)
            if (index >= 0 && index < situation.interpretationCount
                && situation.interpretations[static_cast<std::size_t>(index)].kind
                    == HarmonicInterpretationKind::modalInterchange)
                context = "modal-" + context;

    std::string key = "relative-v1|" + context + "|"
        + strategy.ruleId + "|" + std::to_string(static_cast<int>(chord.quality))
        + "|" + std::to_string(wrap(chord.bassPitchClass - chord.rootPitchClass))
        + "|" + std::to_string(wrap(item.source.rootPitchClass - chord.rootPitchClass));
    for (int interval = 0; interval < kPitchClassCount; ++interval)
        if (chord.hasTone(interval))
            key += ":" + std::to_string(interval) + "."
                + std::to_string(chord.degrees[static_cast<std::size_t>(interval)]);
    return key;
}
}
