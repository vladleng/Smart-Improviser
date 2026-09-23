#pragma once

#include "core/context/HarmonicContext.h"
#include "core/model/HarmonicSituation.h"

namespace smartimproviser::harmony
{
class ARAContextProvider final : public IHarmonicContextProvider
{
public:
    HarmonicContext currentContext() noexcept override;
    HarmonicContext contextAt(double ppq) noexcept override;
    double nextChordStartAfter(double ppq) noexcept override;
    double secondsAtPpq(double ppq) noexcept override;
    double ppqAtSeconds(double seconds) noexcept override;

    // Stage 1 contract: expose a Core-ready timeline snapshot without leaking
    // ARA, JUCE or Fender Studio Pro types into Smart Improviser Core.
    TimelineHarmonicSnapshot currentTimelineSnapshot() noexcept;
    TimelineHarmonicSnapshot timelineSnapshotAt(double ppq) noexcept;
};
}
