#include "core/analysis/HarmonicEngine.h"

namespace smartimproviser::harmony
{
namespace
{
constexpr int wrap12(int value) noexcept
{
    value %= kPitchClassCount;
    return value < 0 ? value + kPitchClassCount : value;
}

NormalizedKey makeKeyFromRootAndMode(int rootPitchClass, KeyMode mode) noexcept
{
    NormalizedKey key;
    if (mode != KeyMode::major && mode != KeyMode::minor)
        return key;

    static constexpr int majorIntervals[] = { 0, 2, 4, 5, 7, 9, 11 };
    static constexpr int minorIntervals[] = { 0, 2, 3, 5, 7, 8, 10 };

    key.valid = true;
    key.rootPitchClass = wrap12(rootPitchClass);
    key.mode = mode;

    const auto* intervals = mode == KeyMode::major ? majorIntervals : minorIntervals;
    for (int i = 0; i < 7; ++i)
        key.tones[static_cast<std::size_t>(intervals[i])] = true;

    return key;
}

KeyMode targetModeFromChord(const NormalizedChord& chord) noexcept
{
    switch (chord.quality)
    {
        case ChordQuality::minor:
        case ChordQuality::diminished:
        case ChordQuality::halfDiminished:
            return KeyMode::minor;
        case ChordQuality::major:
        case ChordQuality::dominant:
        case ChordQuality::augmented:
            return KeyMode::major;
        default:
            return KeyMode::undefined;
    }
}

KeyCenter inferTemporaryCenter(const HarmonicSituation& situation) noexcept
{
    KeyCenter center;

    if (! situation.valid
        || ! situation.nextChordAvailable
        || ! situation.harmonic.appliedDominantConfirmed)
        return center;

    const auto mode = targetModeFromChord(situation.nextChord);
    const auto key = makeKeyFromRootAndMode(situation.nextChord.rootPitchClass, mode);
    if (! key.valid)
        return center;

    center.valid = true;
    center.key = key;
    center.scope = KeyCenterScope::temporary;
    center.evidence.confidence = ConfidenceLevel::high;
    center.evidence.markUnique();
    center.evidence.add(EvidenceFlag::nextChord);
    center.evidence.add(EvidenceFlag::confirmedResolution);
    center.evidence.add(EvidenceFlag::inferredLocalCenter);
    return center;
}

bool isDegree(const NormalizedChord& chord, const NormalizedKey& key, int degree) noexcept
{
    return chord.valid && key.valid
        && scaleDegreeForPitchClass(key, chord.rootPitchClass) == degree;
}

bool isMinorFamily(const NormalizedChord& chord) noexcept
{
    return chord.quality == ChordQuality::minor
        || chord.quality == ChordQuality::halfDiminished;
}

HarmonicPattern makePattern(HarmonicPatternType type,
                            PatternMemberRole role,
                            int positionIndex,
                            int length,
                            ConfidenceLevel confidence) noexcept
{
    HarmonicPattern pattern;
    pattern.type = type;
    pattern.role = role;
    pattern.positionIndex = positionIndex;
    pattern.length = length;
    pattern.evidence.confidence = confidence;
    pattern.evidence.markUnique();
    pattern.evidence.add(EvidenceFlag::patternMatch);
    return pattern;
}

HarmonicPattern recognizePattern(const HarmonicSituation& situation) noexcept
{
    HarmonicPattern none;
    none.type = HarmonicPatternType::none;

    if (! situation.valid)
        return none;

    const auto& key = situation.globalKey.key;

    if (situation.previousChordAvailable
        && situation.nextChordAvailable
        && key.mode == KeyMode::major
        && isDegree(situation.previousChord, key, 2)
        && situation.previousChord.quality == ChordQuality::minor
        && isDegree(situation.currentChord, key, 5)
        && situation.currentChord.quality == ChordQuality::dominant
        && isDegree(situation.nextChord, key, 1)
        && situation.nextChord.quality == ChordQuality::major
        && situation.harmonic.dominantResolutionConfirmed)
    {
        return makePattern(HarmonicPatternType::majorIiVI,
                           PatternMemberRole::dominant,
                           1,
                           3,
                           ConfidenceLevel::confirmed);
    }

    if (situation.previousChordAvailable
        && situation.nextChordAvailable
        && key.mode == KeyMode::minor
        && isDegree(situation.previousChord, key, 2)
        && situation.previousChord.quality == ChordQuality::halfDiminished
        && isDegree(situation.currentChord, key, 5)
        && situation.currentChord.quality == ChordQuality::dominant
        && isDegree(situation.nextChord, key, 1)
        && isMinorFamily(situation.nextChord)
        && situation.harmonic.dominantResolutionConfirmed)
    {
        return makePattern(HarmonicPatternType::minorIiHalfDimVi,
                           PatternMemberRole::dominant,
                           1,
                           3,
                           ConfidenceLevel::confirmed);
    }

    if (situation.nextChordAvailable
        && situation.currentChord.quality == ChordQuality::dominant
        && situation.harmonic.dominantResolutionConfirmed)
    {
        return makePattern(HarmonicPatternType::dominantToTonic,
                           PatternMemberRole::dominant,
                           0,
                           2,
                           ConfidenceLevel::confirmed);
    }

    if (situation.harmonic.appliedDominantConfirmed)
    {
        return makePattern(HarmonicPatternType::secondaryDominant,
                           PatternMemberRole::dominant,
                           0,
                           2,
                           ConfidenceLevel::confirmed);
    }

    return none;
}
}

HarmonicSituation analyzeHarmonicSituation(const TimelineHarmonicSnapshot& snapshot) noexcept
{
    auto result = buildHarmonicSituation(snapshot);
    if (! result.valid)
        return result;

    result.localKey = inferTemporaryCenter(result);
    if (result.localKey.valid)
        result.evidence.add(EvidenceFlag::inferredLocalCenter);

    result.pattern = recognizePattern(result);
    if (result.pattern.recognized())
    {
        result.evidence.add(EvidenceFlag::patternMatch);
        if (result.pattern.evidence.confidence == ConfidenceLevel::confirmed)
            result.evidence.confidence = ConfidenceLevel::confirmed;
    }

    if (result.evidence.interpretation == InterpretationStatus::unknown)
        result.evidence.markUnique();

    return result;
}
}
