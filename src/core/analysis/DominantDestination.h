#pragma once
#include "core/model/HarmonicSituation.h"

namespace smartimproviser::harmony
{
// Musical destination used to select the source family. Never replaces the
// factual nextChord, ResolutionTarget or established key in HarmonicSituation.
struct DominantDestination
{
    bool valid = false;
    bool confirmed = false;
    int rootFifths = 0;
    int rootPitchClass = -1;
    ChordQuality quality = ChordQuality::undefined;
    AnalysisEvidence evidence;
    bool minor() const noexcept { return valid && quality == ChordQuality::minor; }
};

inline DominantDestination dominantDestination(const HarmonicSituation& s) noexcept
{
    DominantDestination d;
    const auto& v = s.currentChord;
    if (!v.valid || v.quality != ChordQuality::dominant) return d;
    d.rootFifths = v.rootFifths - 1;
    d.rootPitchClass = (v.rootPitchClass + 5) % 12;
    const auto& resolution = s.resolution;
    if (resolution.available && resolution.confirmed && resolution.targetChord.valid
        && resolution.targetChord.rootPitchClass == d.rootPitchClass
        && (resolution.targetChord.quality == ChordQuality::major
            || resolution.targetChord.quality == ChordQuality::minor))
    {
        d.valid = d.confirmed = true;
        d.rootFifths = resolution.targetChord.rootFifths;
        d.quality = resolution.targetChord.quality;
        d.evidence = resolution.evidence;
        return d;
    }
    // An actual major/minor continuation supplies its written quality, even
    // when Stage 2 has not confirmed its tonic function.
    if (s.nextChordAvailable && s.nextChord.valid)
    {
        if (s.nextChord.rootPitchClass != d.rootPitchClass) return d;
        if (s.nextChord.quality == ChordQuality::major || s.nextChord.quality == ChordQuality::minor)
        {
            d.valid = true;
            d.rootFifths = s.nextChord.rootFifths;
            d.quality = s.nextChord.quality;
            d.evidence.confidence = ConfidenceLevel::low;
            d.evidence.add(EvidenceFlag::explicitChord);
            return d;
        }
    }
    else return d; // No observed root direction: do not invent a destination.

    // A changed-quality chord on the destination root does not establish a
    // major tonic. Local confirmed context takes precedence over global context.
    // The preceding ii/ii-half-diminished is recognition evidence, never a gate.
    if (!(v.hasTone(8) && v.degrees[8] == 13)
        && !(v.hasTone(1) && v.degrees[1] == 9)) return d;
    const auto& center = s.localKey.valid
        && (s.localKey.status == KeyCenterStatus::established
            || s.localKey.status == KeyCenterStatus::tonicized)
        ? s.localKey : s.globalKey;
    if (!center.valid || !center.key.hasPitchClass(d.rootPitchClass)
        || !center.key.hasPitchClass((d.rootPitchClass + 3) % 12)
        || !center.key.hasPitchClass((d.rootPitchClass + 7) % 12)
        || center.key.hasPitchClass((d.rootPitchClass + 4) % 12)) return d;
    d.valid = true;
    d.quality = ChordQuality::minor;
    d.evidence.confidence = ConfidenceLevel::low;
    d.evidence.add(EvidenceFlag::explicitKey);
    d.evidence.add(EvidenceFlag::explicitChord);
    return d;
}
}
