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
// Context supplies the quality of the unplayed destination. The ii-V shape
// supplies its root; an explicit alteration supports, but cannot create, this
// reading. A changed-quality continuation is retained as the factual chord.
inline bool hasContextualMinorIiVTarget(const HarmonicSituation& s) noexcept
{
    const auto& c = s.incompleteCadence;
    if (!c.valid || !c.v.valid || !c.actualContinuation.valid) return false;
    const int target = circleOfFifthsToPitchClass(c.missingTonicRootFifths);
    if ((c.v.rootPitchClass + 5) % 12 != target) return false;
    if (!(c.v.hasTone(8) && c.v.degrees[8] == 13)
        && !(c.v.hasTone(1) && c.v.degrees[1] == 9)) return false;
    if (s.resolution.confirmed && s.resolution.targetChord.valid
        && s.resolution.targetChord.rootPitchClass == target
        && s.resolution.targetChord.quality == ChordQuality::major) return false;
    const auto& center = s.localKey.valid
        && (s.localKey.status == KeyCenterStatus::established
            || s.localKey.status == KeyCenterStatus::tonicized)
        ? s.localKey : s.globalKey;
    return center.valid && center.key.hasPitchClass(target)
        && center.key.hasPitchClass((target + 3) % 12)
        && center.key.hasPitchClass((target + 7) % 12)
        && !center.key.hasPitchClass((target + 4) % 12);
}

}
