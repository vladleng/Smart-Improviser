#pragma once

#include "core/model/HarmonicSituation.h"

namespace smartimproviser::harmony
{
// Stage 2 host-neutral entry point. It consumes only the normalized timeline
// contract and enriches the base HarmonicSituation with context-sensitive
// analysis. No JUCE/ARA/shared-memory types are allowed in this layer.
HarmonicSituation analyzeHarmonicSituation(const TimelineHarmonicSnapshot& snapshot) noexcept;
}
