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

    // Full ordinary major ii-V-I.
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

    // Full ordinary minor iiø-V-i.
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

    // Full ii-SubV-I in major.
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

    // Full iiø-SubV-i in minor.
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

    // Linked dominants are more specific than a single secondary dominant.
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

    // Tritone-substitution boundary positions.
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

    // The end-of-turnaround pair ii-V is only a boundary candidate when the
    // future event is genuinely unavailable. If next is known, stronger full
    // patterns above must validate it; a contradictory next suppresses the
    // guess instead of inventing I-VI-ii-V from previous/current alone.
    if (key.mode == KeyMode::major
        && situation.previousChordAvailable
        && ! situation.nextChordAvailable
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

    result.pattern = recognizePattern(result);
    mergePatternEvidence(result);

    // 0.2d: infer the active local/sub-tonal center independently from the
    // DAW project key. This analyzer may expose candidate, temporary, local or
    // modulation-candidate states, but never mutates the explicit global key.
    analyzeLocalKeyCenter(result);

    if (result.evidence.interpretation == InterpretationStatus::unknown)
        result.evidence.markUnique();

    return result;
}
}
