#pragma once
#include <vector>

namespace smartimproviser::harmony
{
// Zero means browse all, not a fourth tension level. Labels are user metadata;
// inputs must already have passed harmonic/source compatibility checks.
inline int normalizedTensionFilter(int value) { return value >= 1 && value <= 3 ? value : 0; }
struct TensionFilterRow { int materialIndex; int level; bool baseMode; };
struct TensionFilterItem { int materialIndex; int level; bool baseMode; bool hidden; };
inline std::vector<TensionFilterRow> filteredTensionRows(
    const std::vector<TensionFilterItem>& items, int requested)
{
    const int filter = normalizedTensionFilter(requested);
    std::vector<TensionFilterRow> rows;
    for (const auto& item : items)
        if (item.baseMode && !item.hidden) rows.push_back({item.materialIndex, 0, true});
    for (int level : {1, 2, 3, 0})
        for (const auto& item : items)
            if (!item.baseMode && !item.hidden && normalizedTensionFilter(item.level) == level
                && (filter == 0 || level == filter))
                rows.push_back({item.materialIndex, level, false});
    return rows;
}
inline int filteredMaterialSelection(const std::vector<TensionFilterRow>& rows,
                                     int previous, int requested)
{
    for (const auto& row : rows) if (row.materialIndex == previous) return previous;
    if (normalizedTensionFilter(requested) != 0)
        for (const auto& row : rows) if (!row.baseMode) return row.materialIndex;
    return rows.empty() ? -1 : rows.front().materialIndex;
}
inline bool hasTensionRecommendation(const std::vector<TensionFilterRow>& rows)
{
    for (const auto& row : rows) if (!row.baseMode) return true;
    return false;
}
}
