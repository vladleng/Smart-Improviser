#include "core/model/HarmonicSituation.h"

namespace smartimproviser::harmony
{
namespace
{
constexpr int wrap12(int value) noexcept
{
    value %= kPitchClassCount;
    return value < 0 ? value + kPitchClassCount : value;
}

constexpr int shortestSemitoneDelta(int fromPitchClass, int toPitchClass) noexcept
{
    auto delta = wrap12(toPitchClass - fromPitchClass);
    if (delta > 6)
        delta -= 12;
    return delta;
}

int targetThirdInterval(ChordQuality quality) noexcept
{
    switch (quality)
    {
        case ChordQuality::major:
        case ChordQuality::dominant:
        case ChordQuality::augmented:
            return 4;
        case ChordQuality::minor:
        case ChordQuality::diminished:
        case ChordQuality::halfDiminished:
            return 3;
        default:
            return -1;
    }
}

ResolutionTarget buildResolutionTarget(const NormalizedChord& currentChord,
                                       const NormalizedChord& nextChord,
                                       const HarmonicAnalysis& harmonic) noexcept
{
    ResolutionTarget target;
    if (! harmonic.dominantResolutionConfirmed || ! nextChord.valid)
        return target;

    target.available = true;
    target.confirmed = true;
    target.targetChord = nextChord;
    target.targetPitchClass = nextChord.rootPitchClass;
    target.targetQuality = nextChord.quality;
    target.evidence.confidence = ConfidenceLevel::confirmed;
    target.evidence.markUnique();
    target.evidence.add(EvidenceFlag::nextChord);
    target.evidence.add(EvidenceFlag::confirmedResolution);

    if (currentChord.valid && currentChord.hasTone(4))
    {
        const auto from = wrap12(currentChord.rootPitchClass + 4);
        const auto to = nextChord.rootPitchClass;
        target.addMove({ from,
                         to,
                         shortestSemitoneDelta(from, to),
                         ResolutionImportance::structural });
    }

    const auto thirdInterval = targetThirdInterval(nextChord.quality);
    if (currentChord.valid && currentChord.hasTone(10)
        && thirdInterval >= 0 && nextChord.hasTone(thirdInterval))
    {
        const auto from = wrap12(currentChord.rootPitchClass + 10);
        const auto to = wrap12(nextChord.rootPitchClass + thirdInterval);
        target.addMove({ from,
                         to,
                         shortestSemitoneDelta(from, to),
                         ResolutionImportance::structural });
    }

    return target;
}
}

HarmonicSituation buildHarmonicSituation(const TimelineHarmonicSnapshot& snapshot) noexcept
{
    HarmonicSituation result;
    result.ppq = snapshot.ppq;

    result.currentChord = normalizeChord(snapshot.currentChord);
    const auto normalizedGlobalKey = normalizeKey(snapshot.globalKey);
    result.globalKey = makeGlobalKeyCenter(normalizedGlobalKey);

    if (snapshot.previousChordAvailable)
    {
        result.previousChord = normalizeChord(snapshot.previousChord);
        result.previousChordAvailable = result.previousChord.valid;
    }

    if (snapshot.nextChordAvailable)
    {
        result.nextChord = normalizeChord(snapshot.nextChord);
        result.nextChordAvailable = result.nextChord.valid;
    }

    // Stage 1 -> Stage 2 contract: a musical situation is analyzable only when
    // its timeline coordinate is known and both current chord and global key
    // are structurally defined. Missing data is a safe invalid state, not an
    // invitation to invent host context.
    if (! snapshot.positionAvailable
        || ! result.currentChord.valid
        || ! result.globalKey.valid)
        return result;

    result.valid = true;
    result.evidence.confidence = ConfidenceLevel::high;
    result.evidence.add(EvidenceFlag::explicitChord);
    result.evidence.add(EvidenceFlag::explicitKey);

    if (result.previousChordAvailable)
        result.evidence.add(EvidenceFlag::previousChord);

    if (result.nextChordAvailable)
        result.evidence.add(EvidenceFlag::nextChord);

    if (result.nextChordAvailable)
        result.harmonic = analyzeHarmonicFunction(result.currentChord,
                                                  result.globalKey.key,
                                                  result.nextChord);
    else
        result.harmonic = analyzeHarmonicFunction(result.currentChord,
                                                  result.globalKey.key);

    if (result.harmonic.relation == HarmonicRelation::diatonic)
        result.evidence.add(EvidenceFlag::diatonicMatch);
    else if (result.harmonic.relation == HarmonicRelation::chromatic)
        result.evidence.add(EvidenceFlag::chromaticRelation);

    if (result.harmonic.modalInterchangeCandidate)
        result.evidence.add(EvidenceFlag::modalInterchange);

    if (result.nextChordAvailable)
        result.resolution = buildResolutionTarget(result.currentChord,
                                                  result.nextChord,
                                                  result.harmonic);

    if (result.resolution.confirmed)
    {
        result.evidence.add(EvidenceFlag::confirmedResolution);
        result.evidence.confidence = ConfidenceLevel::confirmed;
    }

    // Pattern recognition and local key inference are intentionally left to
    // dedicated Stage 2 analyzers. The data-model contract carries their output
    // without guessing them in the data-model layer.
    result.pattern.type = HarmonicPatternType::none;

    return result;
}
}
