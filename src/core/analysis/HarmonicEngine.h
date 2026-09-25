#pragma once

#include "core/model/HarmonicSituation.h"

namespace smartimproviser::harmony
{
// Stage 2 host-neutral entry point. It consumes only the accepted normalized
// Stage 1 timeline contract (previous/current/next/global key).
HarmonicSituation analyzeHarmonicSituation(const TimelineHarmonicSnapshot& snapshot) noexcept;

// Stage 3/0.3f fix1 overload. PatternTimelineWindow is a separate bounded
// reconstruction input and deliberately does not alter TimelineHarmonicSnapshot.
HarmonicSituation analyzeHarmonicSituation(const TimelineHarmonicSnapshot& snapshot,
                                           const PatternTimelineWindow& patternWindow) noexcept;
}
