#pragma once
#include "core/model/PhraseMatch.h"
#include "core/analysis/TensionFilter.h"
namespace smartimproviser::harmony
{
// Bridge from the accepted UI filter: All=0; only explicit 1/2/3 requests.
inline std::optional<TensionLevel> requestedPhraseTension(int uiFilter)
{
    const int level = normalizedTensionFilter(uiFilter);
    return level == 0 ? std::nullopt : std::optional<TensionLevel>(static_cast<TensionLevel>(level));
}

// Control-thread contract: harmonic assessment precedes user tension filtering.
// No phrase lookup, ranking, generation, editing or note mutation.
PhraseMatchResult assessPhrase(const Phrase&, const PhraseMatchRequest&);
}
