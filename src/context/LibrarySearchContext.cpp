#include "context/LibrarySearchContext.h"
#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/TensionEngine.h"
#include <cmath>
namespace smartimproviser::harmony {
LibrarySearchQuery makeLibrarySearchQuery(const SharedHarmonicContextSnapshot& shared,int filter) {
    LibrarySearchQuery query;query.match.requestedTension=requestedPhraseTension(filter);
    if(!shared.connected || !shared.transportAvailable || !std::isfinite(shared.transportPpq) || shared.transportPpq<0)return query;
    const double origin=shared.transportPpq;query.placementStartBeat=origin;
    double start=origin;
    for(int slot=0;slot<16;++slot) {
        const double end=nextChordStartAfter(shared,start);
        if(!std::isfinite(end) || end<=start || end-origin>64)break; // No invented last-chord duration.
        const auto timeline=mapTimelineHarmonicSnapshot(shared,start);
        const auto window=mapPatternTimelineWindow(shared,start);
        auto material=analyzeImprovisation(analyzeHarmonicSituation(timeline,window));
        if(!material.valid)break;
        const auto stable=analyzeStableTension(material);
        if(const auto* band=stable.band(TensionLevel::stable))
            for(const auto& candidate:band->alternatives)material.strategies.push_back(candidate.strategy);
        query.match.slots.push_back({std::move(material),start-origin,end-origin});
        start=end;
    }
    return query;
}
}
