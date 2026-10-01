#pragma once
#include "core/analysis/Explanation.h"

namespace smartimproviser::harmony
{
// Presentation relationships. The full source and its classified subset keep
// their own notes and evidence while occupying a single compact list row.
inline int stableSubsetIndex(const ExplanationResult& explanation, int parent)
{
    if (parent < 0 || parent >= static_cast<int>(explanation.items.size())) return -1;
    const auto& item = explanation.items[static_cast<std::size_t>(parent)];
    if (item.sourceRuleId != "boyko.melodic-minor.V") return -1;
    for (std::size_t i = 0; i < explanation.items.size(); ++i)
    {
        const auto& subset = explanation.items[i];
        if (subset.sourceRuleId == "project.t1.major-V-m6"
            && subset.source.rootFifths == item.source.rootFifths
            && explanation_detail::sameChordIdentity(subset.actualChord, item.actualChord)
            && subset.missingTonicApplication == item.missingTonicApplication)
            return static_cast<int>(i);
    }
    return -1;
}

inline bool compactMaterialHidden(const ExplanationResult& explanation, int index)
{
    if (index < 0 || index >= static_cast<int>(explanation.items.size())) return true;
    const auto& item = explanation.items[static_cast<std::size_t>(index)];
    if (item.sourceRuleId == "core.explicit-chord-tones"
        || item.sourceRuleId == "project.t1.explicit-anchors") return true;
    if (item.sourceRuleId == "project.t1.major-V-m6")
        for (std::size_t i = 0; i < explanation.items.size(); ++i)
            if (stableSubsetIndex(explanation, static_cast<int>(i)) == index) return true;
    return false;
}

inline int literalChordIndex(const ExplanationResult& explanation)
{
    for (std::size_t i = 0; i < explanation.items.size(); ++i)
        if (explanation.items[i].sourceRuleId == "core.explicit-chord-tones")
            return static_cast<int>(i);
    return -1;
}
}
