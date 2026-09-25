#pragma once

#include "context/SharedHarmonicContextData.h"
#include "core/context/HarmonicContext.h"
#include "core/model/HarmonicSituation.h"

namespace smartimproviser::harmony
{
HarmonicContext mapHarmonicContext(const SharedHarmonicContextSnapshot& shared,
                                   double ppq) noexcept;

TimelineHarmonicSnapshot mapTimelineHarmonicSnapshot(
    const SharedHarmonicContextSnapshot& shared,
    double ppq) noexcept;

// Stage 3-only bounded pattern reconstruction input. Kept outside the accepted
// Stage 1 TimelineHarmonicSnapshot previous/current/next contract.
PatternTimelineWindow mapPatternTimelineWindow(
    const SharedHarmonicContextSnapshot& shared,
    double ppq) noexcept;

double nextChordStartAfter(const SharedHarmonicContextSnapshot& shared,
                           double ppq) noexcept;

double secondsAtPpq(const SharedHarmonicContextSnapshot& shared,
                    double ppq) noexcept;

double ppqAtSeconds(const SharedHarmonicContextSnapshot& shared,
                    double seconds) noexcept;
}
