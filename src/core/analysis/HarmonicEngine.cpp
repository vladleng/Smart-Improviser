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

bool isTurnaroundSix(const NormalizedChord& chord, const NormalizedKey& key) noexcept
{
    return isDegree(chord, key, 6)
        && (chord.quality == ChordQuality::minor
            || chord.quality == ChordQuality::dominant);
}

bool isDominantOf(const NormalizedChord& dominant,
                  const NormalizedChord& target) noexcept
{
    return dominant.valid
        && target.valid
        && dominant.quality == ChordQuality::dominant
        && wrap12(dominant.rootPitchClass - target.rootPitchClass) == 7;
}

bool isSubstituteDominantOf(const NormalizedChord& dominant,
                            const NormalizedChord& target) noexcept
{
    return dominant.valid
        && target.valid
        && dominant.quality == ChordQuality::dominant
        && wrap12(dominant.rootPitchClass - target.rootPitchClass) == 1;
}

bool isSubstituteDominantForRoot(const NormalizedChord& dominant,
                                 int targetRootPitchClass) noexcept
{
    return dominant.valid
        && dominant.quality == ChordQuality::dominant
        && wrap12(dominant.rootPitchClass - targetRootPitchClass) == 1;
}

HarmonicPattern makePattern(HarmonicPatternType type,
                            PatternMemberRole role,
                            int positionIndex,
                            int length,
                            ConfidenceLevel confidence,
                            bool previousEvidence = false,
                            bool nextEvidence = false,
                            bool resolutionEvidence = false) noexcept
{
    HarmonicPattern pattern;
    pattern.type = type;
    pattern.role = role;
    pattern.positionIndex = positionIndex;
    pattern.length = length;
    pattern.evidence.confidence = confidence;
    pattern.evidence.markUnique();
    pattern.evidence.add(EvidenceFlag::patternMatch);

    if (previousEvidence)
        pattern.evidence.add(EvidenceFlag::previousChord);
    if (nextEvidence)
        pattern.evidence.add(EvidenceFlag::nextChord);
    if (resolutionEvidence)
        pattern.evidence.add(EvidenceFlag::confirmedResolution);

    return pattern;
}

HarmonicPattern recognizePattern(const HarmonicSituation& situation) noexcept
{
    HarmonicPattern none;
    none.type = HarmonicPatternType::none;

    if (! situation.valid)
        return none;

    const auto& key = situation.globalKey.key;

    // Full ordinary ii-V-I cadence window.
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
                           ConfidenceLevel::confirmed,
                           true,
                           true,
                           true);
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
                           ConfidenceLevel::confirmed,
                           true,
                           true,
                           true);
    }

    // Full ii-SubV-I cadence. The substitute dominant sits a semitone above
    // the target tonic and resolves by root semitone descent.
    if (situation.previousChordAvailable
        && situation.nextChordAvailable
        && key.mode == KeyMode::major
        && isDegree(situation.previousChord, key, 2)
        && situation.previousChord.quality == ChordQuality::minor
        && isSubstituteDominantOf(situation.currentChord, situation.nextChord)
        && isDegree(situation.nextChord, key, 1)
        && situation.nextChord.quality == ChordQuality::major
        && situation.harmonic.substituteDominantConfirmed)
    {
        return makePattern(HarmonicPatternType::tritoneSubstitution,
                           PatternMemberRole::substituteDominant,
                           1,
                           3,
                           ConfidenceLevel::confirmed,
                           true,
                           true,
                           true);
    }

    if (situation.previousChordAvailable
        && situation.nextChordAvailable
        && key.mode == KeyMode::minor
        && isDegree(situation.previousChord, key, 2)
        && situation.previousChord.quality == ChordQuality::halfDiminished
        && isSubstituteDominantOf(situation.currentChord, situation.nextChord)
        && isDegree(situation.nextChord, key, 1)
        && isMinorFamily(situation.nextChord)
        && situation.harmonic.substituteDominantConfirmed)
    {
        return makePattern(HarmonicPatternType::tritoneSubstitution,
                           PatternMemberRole::substituteDominant,
                           1,
                           3,
                           ConfidenceLevel::confirmed,
                           true,
                           true,
                           true);
    }

    // A dominant chain is more specific than a single secondary dominant.
    if (situation.previousChordAvailable
        && situation.nextChordAvailable
        && isDominantOf(situation.previousChord, situation.currentChord)
        && isDominantOf(situation.currentChord, situation.nextChord))
    {
        return makePattern(HarmonicPatternType::dominantChain,
                           PatternMemberRole::dominant,
                           1,
                           3,
                           ConfidenceLevel::confirmed,
                           true,
                           true,
                           true);
    }

    // I-VI-ii-V middle positions.
    if (key.mode == KeyMode::major
        && situation.previousChordAvailable
        && situation.nextChordAvailable
        && isDegree(situation.previousChord, key, 1)
        && situation.previousChord.quality == ChordQuality::major
        && isTurnaroundSix(situation.currentChord, key)
        && isDegree(situation.nextChord, key, 2)
        && situation.nextChord.quality == ChordQuality::minor)
    {
        return makePattern(HarmonicPatternType::turnaroundIVIiiV,
                           PatternMemberRole::preparation,
                           1,
                           4,
                           ConfidenceLevel::high,
                           true,
                           true);
    }

    if (key.mode == KeyMode::major
        && situation.previousChordAvailable
        && situation.nextChordAvailable
        && isTurnaroundSix(situation.previousChord, key)
        && isDegree(situation.currentChord, key, 2)
        && situation.currentChord.quality == ChordQuality::minor
        && isDegree(situation.nextChord, key, 5)
        && situation.nextChord.quality == ChordQuality::dominant)
    {
        return makePattern(HarmonicPatternType::turnaroundIVIiiV,
                           PatternMemberRole::predominant,
                           2,
                           4,
                           ConfidenceLevel::high,
                           true,
                           true);
    }

    // Ordinary cadence boundary positions.
    if (key.mode == KeyMode::major
        && situation.nextChordAvailable
        && isDegree(situation.currentChord, key, 2)
        && situation.currentChord.quality == ChordQuality::minor
        && isDegree(situation.nextChord, key, 5)
        && situation.nextChord.quality == ChordQuality::dominant)
    {
        return makePattern(HarmonicPatternType::majorIiVI,
                           PatternMemberRole::predominant,
                           0,
                           3,
                           ConfidenceLevel::high,
                           false,
                           true);
    }

    if (key.mode == KeyMode::major
        && situation.previousChordAvailable
        && isDegree(situation.previousChord, key, 5)
        && situation.previousChord.quality == ChordQuality::dominant
        && isDegree(situation.currentChord, key, 1)
        && situation.currentChord.quality == ChordQuality::major)
    {
        return makePattern(HarmonicPatternType::majorIiVI,
                           PatternMemberRole::resolution,
                           2,
                           3,
                           ConfidenceLevel::high,
                           true,
                           false);
    }

    if (key.mode == KeyMode::minor
        && situation.nextChordAvailable
        && isDegree(situation.currentChord, key, 2)
        && situation.currentChord.quality == ChordQuality::halfDiminished
        && isDegree(situation.nextChord, key, 5)
        && situation.nextChord.quality == ChordQuality::dominant)
    {
        return makePattern(HarmonicPatternType::minorIiHalfDimVi,
                           PatternMemberRole::predominant,
                           0,
                           3,
                           ConfidenceLevel::high,
                           false,
                           true);
    }

    if (key.mode == KeyMode::minor
        && situation.previousChordAvailable
        && isDegree(situation.previousChord, key, 5)
        && situation.previousChord.quality == ChordQuality::dominant
        && isDegree(situation.currentChord, key, 1)
        && isMinorFamily(situation.currentChord))
    {
        return makePattern(HarmonicPatternType::minorIiHalfDimVi,
                           PatternMemberRole::resolution,
                           2,
                           3,
                           ConfidenceLevel::high,
                           true,
                           false);
    }

    // Tritone-substitution boundary positions. We only infer what is visible:
    // ii + SubV or SubV + I is high-confidence, not a confirmed full cadence.
    if (key.mode == KeyMode::major
        && situation.nextChordAvailable
        && isDegree(situation.currentChord, key, 2)
        && situation.currentChord.quality == ChordQuality::minor
        && isSubstituteDominantForRoot(situation.nextChord, key.rootPitchClass))
    {
        return makePattern(HarmonicPatternType::tritoneSubstitution,
                           PatternMemberRole::predominant,
                           0,
                           3,
                           ConfidenceLevel::high,
                           false,
                           true);
    }

    if (key.mode == KeyMode::minor
        && situation.nextChordAvailable
        && isDegree(situation.currentChord, key, 2)
        && situation.currentChord.quality == ChordQuality::halfDiminished
        && isSubstituteDominantForRoot(situation.nextChord, key.rootPitchClass))
    {
        return makePattern(HarmonicPatternType::tritoneSubstitution,
                           PatternMemberRole::predominant,
                           0,
                           3,
                           ConfidenceLevel::high,
                           false,
                           true);
    }

    if (situation.previousChordAvailable
        && isDegree(situation.currentChord, key, 1)
        && isSubstituteDominantOf(situation.previousChord, situation.currentChord)
        && ((key.mode == KeyMode::major && situation.currentChord.quality == ChordQuality::major)
            || (key.mode == KeyMode::minor && isMinorFamily(situation.currentChord))))
    {
        return makePattern(HarmonicPatternType::tritoneSubstitution,
                           PatternMemberRole::resolution,
                           2,
                           3,
                           ConfidenceLevel::high,
                           true,
                           false);
    }

    // Turnaround boundary candidates.
    if (key.mode == KeyMode::major
        && situation.nextChordAvailable
        && isDegree(situation.currentChord, key, 1)
        && situation.currentChord.quality == ChordQuality::major
        && isTurnaroundSix(situation.nextChord, key))
    {
        return makePattern(HarmonicPatternType::turnaroundIVIiiV,
                           PatternMemberRole::tonic,
                           0,
                           4,
                           ConfidenceLevel::medium,
                           false,
                           true);
    }

    if (key.mode == KeyMode::major
        && situation.previousChordAvailable
        && isDegree(situation.previousChord, key, 2)
        && situation.previousChord.quality == ChordQuality::minor
        && isDegree(situation.currentChord, key, 5)
        && situation.currentChord.quality == ChordQuality::dominant)
    {
        return makePattern(HarmonicPatternType::turnaroundIVIiiV,
                           PatternMemberRole::dominant,
                           3,
                           4,
                           ConfidenceLevel::medium,
                           true,
                           false);
    }

    // A confirmed substitute dominant must win over generic chromatic/applied
    // dominant interpretations when the real next chord confirms semitone
    // resolution.
    if (situation.harmonic.substituteDominantConfirmed)
    {
        return makePattern(HarmonicPatternType::tritoneSubstitution,
                           PatternMemberRole::substituteDominant,
                           0,
                           2,
                           ConfidenceLevel::confirmed,
                           false,
                           true,
                           true);
    }

    // Applied dominants are more specific than generic dominant-to-target.
    if (situation.harmonic.appliedDominantConfirmed)
    {
        return makePattern(HarmonicPatternType::secondaryDominant,
                           PatternMemberRole::dominant,
                           0,
                           2,
                           ConfidenceLevel::confirmed,
                           false,
                           true,
                           true);
    }

    if (situation.nextChordAvailable
        && situation.currentChord.quality == ChordQuality::dominant
        && situation.harmonic.dominantResolutionConfirmed)
    {
        return makePattern(HarmonicPatternType::dominantToTonic,
                           PatternMemberRole::dominant,
                           0,
                           2,
                           ConfidenceLevel::confirmed,
                           false,
                           true,
                           true);
    }

    return none;
}

void mergePatternEvidence(HarmonicSituation& situation) noexcept
{
    if (! situation.pattern.recognized())
        return;

    situation.evidence.add(EvidenceFlag::patternMatch);

    if (situation.pattern.evidence.has(EvidenceFlag::previousChord))
        situation.evidence.add(EvidenceFlag::previousChord);
    if (situation.pattern.evidence.has(EvidenceFlag::nextChord))
        situation.evidence.add(EvidenceFlag::nextChord);
    if (situation.pattern.evidence.has(EvidenceFlag::confirmedResolution))
        situation.evidence.add(EvidenceFlag::confirmedResolution);

    if (static_cast<int>(situation.pattern.evidence.confidence)
        > static_cast<int>(situation.evidence.confidence))
    {
        situation.evidence.confidence = situation.pattern.evidence.confidence;
    }
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
    mergePatternEvidence(result);

    if (result.evidence.interpretation == InterpretationStatus::unknown)
        result.evidence.markUnique();

    return result;
}
}
