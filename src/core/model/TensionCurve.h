#pragma once
#include "core/model/PhraseTensionProfile.h"

namespace smartimproviser::harmony
{
// Desired levels in absolute song beats (not seconds or fixed 4/4 bars).
// Empty/gaps/null spans impose no tension constraint. Harmony still applies.
struct TensionCurve
{
    std::vector<TensionSpan> spans;
};
}
