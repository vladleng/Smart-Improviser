#pragma once

#include "core/model/HarmonicSituation.h"

namespace smartimproviser::harmony
{
// The ii m7–V7 shape points provisionally to a major I even when the actual
// next chord is a minor chord on that root or starts another turn. Preserve
// any independently confirmed resolution to that actual chord; it does not
// turn the unplayed major I into a played tonic or remove the ii–V palette.
inline bool hasIncompleteMajorIiVPalette(const HarmonicSituation& situation) noexcept
{
    const auto& incomplete = situation.incompleteCadence;
    return incomplete.valid && incomplete.positionIndex == 1
        && situation.currentChord.valid
        && situation.currentChord.quality == ChordQuality::dominant
        && situation.currentChord.rootPitchClass == incomplete.v.rootPitchClass
        && situation.nextChordAvailable && incomplete.actualContinuation.valid
        && (situation.currentChord.rootPitchClass + 5) % 12
            == circleOfFifthsToPitchClass(incomplete.missingTonicRootFifths);
}
}
