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

bool isOrdinaryDominantForRoot(const NormalizedChord& dominant,
                               int targetRootPitchClass) noexcept
{
    return dominant.valid
        && dominant.quality == ChordQuality::dominant
        && wrap12(dominant.rootPitchClass - targetRootPitchClass) == 7;
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

bool chordContainsPitchClass(const NormalizedChord& chord,
                             int pitchClass) noexcept
{
    if (! chord.valid)
        return false;

    pitchClass = wrap12(pitchClass);
    for (int interval = 0; interval < kPitchClassCount; ++interval)
    {
        if (chord.tones[static_cast<std::size_t>(interval)]
            && wrap12(chord.rootPitchClass + interval) == pitchClass)
        {
            return true;
        }
    }
    return false;
}

int impliedRootlessDominantRoot(const NormalizedChord& chord,
                                const NormalizedKey& key) noexcept
{
    if (! chord.valid || ! key.valid || chord.quality != ChordQuality::diminished)
        return -1;

    // A fully diminished seventh built from b9-3-5-b7 of V is the classic
    // rootless V7(b9) sonority. Anchor the alias to the explicit global key so
    // symmetrical diminished spellings do not create four arbitrary winners.
    const auto dominantRoot = wrap12(key.rootPitchClass + 7);
    static constexpr int requiredIntervals[] = { 1, 4, 7, 10 };
    for (const auto interval : requiredIntervals)
    {
        if (! chordContainsPitchClass(chord, dominantRoot + interval))
            return -1;
    }

    return dominantRoot;
}

void applyImpliedDominantReading(HarmonicSituation& situation) noexcept
{
    const auto root = impliedRootlessDominantRoot(situation.currentChord,
                                                  situation.globalKey.key);
    if (root < 0)
        return;

    situation.impliedDominant.valid = true;
    situation.impliedDominant.rootPitchClass = root;
    situation.impliedDominant.flatNinth = true;
    situation.impliedDominant.thirteenth =
        chordContainsPitchClass(situation.currentChord, root + 9);
    situation.impliedDominant.evidence.confidence = ConfidenceLevel::high;
    situation.impliedDominant.evidence.markUnique();
    situation.impliedDominant.evidence.add(EvidenceFlag::explicitKey);
    situation.impliedDominant.evidence.add(EvidenceFlag::chromaticRelation);

    // Preserve the written diminished chord, but expose its effective function
    // to the rest of Stage 3. No tonic resolution is claimed here.
    situation.harmonic.effectiveFunction = HarmonicFunction::dominant;
    situation.harmonic.relation = HarmonicRelation::chromatic;
    situation.evidence.add(EvidenceFlag::chromaticRelation);
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

HarmonicPattern recognizePattern(const HarmonicSituation& situation,
                                 bool allowIncompleteCadenceCandidates) noexcept
{
    HarmonicPattern none;
    none.type = HarmonicPatternType::none;

    if (! situation.valid)
        return none;

    const auto& key = situation.globalKey.key;
    const auto currentImpliedDominantRoot =
        impliedRootlessDominantRoot(situation.currentChord, key);
    const auto nextImpliedDominantRoot = situation.nextChordAvailable
        ? impliedRootlessDominantRoot(situation.nextChord, key)
        : -1;

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

    if (situation.previousChordAvailable
        && situation.nextChordAvailable
        && key.mode == KeyMode::minor
        && isDegree(situation.previousChord, key, 4)
        && situation.previousChord.quality == ChordQuality::minor
        && isDegree(situation.currentChord, key, 5)
        && situation.currentChord.quality == ChordQuality::dominant
        && isDegree(situation.nextChord, key, 1)
        && isMinorFamily(situation.nextChord)
        && situation.harmonic.dominantResolutionConfirmed)
    {
        return makePattern(HarmonicPatternType::minorIvVi,
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

    // Corcovado 0.3f fix3: D7/A -> Abdim is more usefully read as
    // V/V -> rootless V7(b9) in C than as a diminished bridge to G minor.
    if (situation.nextChordAvailable
        && situation.currentChord.quality == ChordQuality::dominant
        && nextImpliedDominantRoot >= 0
        && isOrdinaryDominantForRoot(situation.currentChord,
                                     nextImpliedDominantRoot))
    {
        return makePattern(HarmonicPatternType::dominantChain,
                           PatternMemberRole::dominant,
                           0,
                           2,
                           ConfidenceLevel::high,
                           false,
                           true);
    }

    if (situation.previousChordAvailable
        && currentImpliedDominantRoot >= 0
        && isOrdinaryDominantForRoot(situation.previousChord,
                                     currentImpliedDominantRoot))
    {
        return makePattern(HarmonicPatternType::dominantChain,
                           PatternMemberRole::dominant,
                           1,
                           2,
                           ConfidenceLevel::high,
                           true,
                           false);
    }

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

    // A partial I-VI-ii-V is allowed only while the explicit I is still in the
    // Stage-1 window. 0.3f fix2 no longer reconstructs a missing I from VI-ii-V.
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

    if (allowIncompleteCadenceCandidates
        && key.mode == KeyMode::major
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

    if (allowIncompleteCadenceCandidates
        && key.mode == KeyMode::minor
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

    if (allowIncompleteCadenceCandidates
        && key.mode == KeyMode::minor
        && situation.nextChordAvailable
        && isDegree(situation.currentChord, key, 4)
        && situation.currentChord.quality == ChordQuality::minor
        && isDegree(situation.nextChord, key, 5)
        && situation.nextChord.quality == ChordQuality::dominant)
    {
        return makePattern(HarmonicPatternType::minorIvVi,
                           PatternMemberRole::predominant,
                           0,
                           3,
                           ConfidenceLevel::high,
                           false,
                           true);
    }

    if (situation.previousChordAvailable
        && isDegree(situation.previousChord, key, 5)
        && situation.previousChord.quality == ChordQuality::dominant
        && isDegree(situation.currentChord, key, 1)
        && ((key.mode == KeyMode::major && situation.currentChord.quality == ChordQuality::major)
            || (key.mode == KeyMode::minor && isMinorFamily(situation.currentChord))))
    {
        return makePattern(HarmonicPatternType::dominantToTonic,
                           PatternMemberRole::resolution,
                           1,
                           2,
                           ConfidenceLevel::high,
                           true,
                           false,
                           true);
    }

    if (allowIncompleteCadenceCandidates
        && key.mode == KeyMode::major
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

    if (allowIncompleteCadenceCandidates
        && key.mode == KeyMode::minor
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

bool sameKey(const NormalizedKey& a, const NormalizedKey& b) noexcept
{
    return a.valid && b.valid
        && a.rootPitchClass == b.rootPitchClass
        && a.mode == b.mode;
}

NormalizedKey makePatternCenter(const NormalizedChord& tonic, KeyMode mode) noexcept
{
    NormalizedKey key;
    if (! tonic.valid || (mode != KeyMode::major && mode != KeyMode::minor))
        return key;

    static constexpr int majorIntervals[] = { 0, 2, 4, 5, 7, 9, 11 };
    static constexpr int minorIntervals[] = { 0, 2, 3, 5, 7, 8, 10 };

    key.valid = true;
    key.rootFifths = tonic.rootFifths;
    key.rootPitchClass = tonic.rootPitchClass;
    key.mode = mode;

    const auto* intervals = mode == KeyMode::major ? majorIntervals : minorIntervals;
    for (int i = 0; i < 7; ++i)
        key.tones[static_cast<std::size_t>(intervals[i])] = true;

    return key;
}

bool rootIs(const NormalizedChord& chord, const NormalizedChord& tonic, int semitones) noexcept
{
    return chord.valid && tonic.valid
        && wrap12(chord.rootPitchClass - tonic.rootPitchClass) == semitones;
}

bool chordIsTonicForRoot(const NormalizedChord& chord,
                         int targetRoot,
                         KeyMode mode) noexcept
{
    if (! chord.valid || chord.rootPitchClass != wrap12(targetRoot))
        return false;
    if (mode == KeyMode::major)
        return chord.quality == ChordQuality::major;
    if (mode == KeyMode::minor)
        return isMinorFamily(chord);
    return false;
}

bool matchesMajorIiVI(const NormalizedChord& ii,
                      const NormalizedChord& v,
                      const NormalizedChord& tonic) noexcept
{
    return tonic.valid
        && tonic.quality == ChordQuality::major
        && ii.quality == ChordQuality::minor
        && v.quality == ChordQuality::dominant
        && rootIs(ii, tonic, 2)
        && rootIs(v, tonic, 7);
}

bool matchesMinorIiVI(const NormalizedChord& ii,
                      const NormalizedChord& v,
                      const NormalizedChord& tonic) noexcept
{
    return tonic.valid
        && isMinorFamily(tonic)
        && ii.quality == ChordQuality::halfDiminished
        && v.quality == ChordQuality::dominant
        && rootIs(ii, tonic, 2)
        && rootIs(v, tonic, 7);
}

bool matchesMinorIvVI(const NormalizedChord& iv,
                      const NormalizedChord& v,
                      const NormalizedChord& tonic) noexcept
{
    return tonic.valid
        && isMinorFamily(tonic)
        && iv.quality == ChordQuality::minor
        && v.quality == ChordQuality::dominant
        && rootIs(iv, tonic, 5)
        && rootIs(v, tonic, 7);
}

bool matchesMajorCadentialChain(const NormalizedChord& iii,
                                const NormalizedChord& viDominant,
                                const NormalizedChord& ii,
                                const NormalizedChord& v,
                                const NormalizedChord& tonic) noexcept
{
    return tonic.valid
        && tonic.quality == ChordQuality::major
        && iii.quality == ChordQuality::minor
        && viDominant.quality == ChordQuality::dominant
        && ii.quality == ChordQuality::minor
        && v.quality == ChordQuality::dominant
        && rootIs(iii, tonic, 4)
        && rootIs(viDominant, tonic, 9)
        && rootIs(ii, tonic, 2)
        && rootIs(v, tonic, 7)
        && isDominantOf(viDominant, ii);
}

bool matchesMajorTurnaround(const NormalizedChord& tonic,
                            const NormalizedChord& vi,
                            const NormalizedChord& ii,
                            const NormalizedChord& v,
                            const NormalizedKey& key) noexcept
{
    return key.valid
        && key.mode == KeyMode::major
        && isDegree(tonic, key, 1)
        && tonic.quality == ChordQuality::major
        && isTurnaroundSix(vi, key)
        && isDegree(ii, key, 2)
        && ii.quality == ChordQuality::minor
        && isDegree(v, key, 5)
        && v.quality == ChordQuality::dominant;
}

bool matchesMajorIiiViIiV(const NormalizedChord& iii,
                          const NormalizedChord& vi,
                          const NormalizedChord& ii,
                          const NormalizedChord& v,
                          const NormalizedKey& key) noexcept
{
    return key.valid
        && key.mode == KeyMode::major
        && isDegree(iii, key, 3)
        && iii.quality == ChordQuality::minor
        && isDegree(vi, key, 6)
        && vi.quality == ChordQuality::minor
        && isDegree(ii, key, 2)
        && ii.quality == ChordQuality::minor
        && isDegree(v, key, 5)
        && v.quality == ChordQuality::dominant;
}

bool incompleteCadenceContradictedByWindow(const PatternTimelineWindow& window) noexcept
{
    const auto count = static_cast<int>(window.chordCount);
    const auto current = window.currentIndex;
    if (current < 0 || current + 2 >= count)
        return false;

    const auto first = normalizeChord(window.chords[static_cast<std::size_t>(current)]);
    const auto dominant = normalizeChord(window.chords[static_cast<std::size_t>(current + 1)]);
    const auto future = normalizeChord(window.chords[static_cast<std::size_t>(current + 2)]);
    if (! first.valid || ! dominant.valid || ! future.valid
        || dominant.quality != ChordQuality::dominant)
        return false;

    bool candidateFound = false;
    bool futureSupportsCandidate = false;

    if (first.quality == ChordQuality::minor)
    {
        const auto majorTarget = wrap12(first.rootPitchClass - 2);
        if (isOrdinaryDominantForRoot(dominant, majorTarget)
            || isSubstituteDominantForRoot(dominant, majorTarget))
        {
            candidateFound = true;
            futureSupportsCandidate = futureSupportsCandidate
                || chordIsTonicForRoot(future, majorTarget, KeyMode::major);
        }

        const auto minorTarget = wrap12(first.rootPitchClass - 5);
        if (isOrdinaryDominantForRoot(dominant, minorTarget))
        {
            candidateFound = true;
            futureSupportsCandidate = futureSupportsCandidate
                || chordIsTonicForRoot(future, minorTarget, KeyMode::minor);
        }
    }
    else if (first.quality == ChordQuality::halfDiminished)
    {
        const auto minorTarget = wrap12(first.rootPitchClass - 2);
        if (isOrdinaryDominantForRoot(dominant, minorTarget)
            || isSubstituteDominantForRoot(dominant, minorTarget))
        {
            candidateFound = true;
            futureSupportsCandidate = futureSupportsCandidate
                || chordIsTonicForRoot(future, minorTarget, KeyMode::minor);
        }
    }

    return candidateFound && ! futureSupportsCandidate;
}

PatternMemberRole patternRole(HarmonicPatternType type,
                              int position,
                              int length) noexcept
{
    if (type == HarmonicPatternType::majorCadentialChain && length == 5)
    {
        switch (position)
        {
            case 0: return PatternMemberRole::preparation;
            case 1: return PatternMemberRole::dominant;
            case 2: return PatternMemberRole::predominant;
            case 3: return PatternMemberRole::dominant;
            case 4: return PatternMemberRole::resolution;
            default: return PatternMemberRole::undefined;
        }
    }

    if (type == HarmonicPatternType::turnaroundIVIiiV && length == 4)
    {
        switch (position)
        {
            case 0: return PatternMemberRole::tonic;
            case 1: return PatternMemberRole::preparation;
            case 2: return PatternMemberRole::predominant;
            case 3: return PatternMemberRole::dominant;
            default: return PatternMemberRole::undefined;
        }
    }

    if (type == HarmonicPatternType::majorIiiViIiV && length == 4)
    {
        switch (position)
        {
            case 0: return PatternMemberRole::preparation;
            case 1: return PatternMemberRole::preparation;
            case 2: return PatternMemberRole::predominant;
            case 3: return PatternMemberRole::dominant;
            default: return PatternMemberRole::undefined;
        }
    }

    if (type == HarmonicPatternType::passingDiminished && length == 3)
    {
        switch (position)
        {
            case 0: return PatternMemberRole::dominant;
            case 1: return PatternMemberRole::passing;
            case 2: return PatternMemberRole::resolution;
            default: return PatternMemberRole::undefined;
        }
    }

    switch (position)
    {
        case 0: return PatternMemberRole::predominant;
        case 1: return PatternMemberRole::dominant;
        case 2: return PatternMemberRole::resolution;
        default: return PatternMemberRole::undefined;
    }
}

bool patternEndsInResolution(HarmonicPatternType type) noexcept
{
    return type != HarmonicPatternType::turnaroundIVIiiV
        && type != HarmonicPatternType::majorIiiViIiV;
}

void addNestedPattern(PatternContext& context, const HarmonicPattern& pattern) noexcept
{
    if (context.nestedPatternCount >= context.nestedPatterns.size())
        return;
    context.nestedPatterns[context.nestedPatternCount++] = pattern;
}

PatternContext makePatternContext(HarmonicPatternType type,
                                  const NormalizedKey& center,
                                  int position,
                                  int length,
                                  double startPpq,
                                  double resolutionPpq) noexcept
{
    PatternContext context;
    context.valid = center.valid;
    context.center = center;
    context.status = position == length - 1
        ? PatternContextStatus::completed
        : PatternContextStatus::confirmed;
    context.startPpq = startPpq;
    context.resolutionPpq = resolutionPpq;
    const auto resolutionEvidence = position == length - 1
        && patternEndsInResolution(type);
    context.topLevel = makePattern(type,
                                   patternRole(type, position, length),
                                   position,
                                   length,
                                   ConfidenceLevel::confirmed,
                                   position > 0,
                                   position + 1 < length,
                                   resolutionEvidence);
    return context;
}

void applyPatternContext(HarmonicSituation& situation, const PatternContext& context) noexcept
{
    if (! context.valid || ! context.topLevel.recognized())
        return;

    situation.patternContext = context;

    if (sameKey(context.center, situation.globalKey.key))
    {
        situation.pattern = context.topLevel;
        mergePatternEvidence(situation);
        return;
    }

    const auto tonicizationOnly = context.topLevel.type == HarmonicPatternType::passingDiminished;

    situation.localKey.valid = true;
    situation.localKey.key = context.center;
    situation.localKey.scope = tonicizationOnly
        ? KeyCenterScope::temporary
        : KeyCenterScope::local;
    situation.localKey.status = tonicizationOnly
        ? KeyCenterStatus::tonicized
        : KeyCenterStatus::established;
    situation.localKey.evidence.confidence = ConfidenceLevel::confirmed;
    situation.localKey.evidence.markUnique();
    situation.localKey.evidence.add(EvidenceFlag::inferredLocalCenter);
    situation.localKey.evidence.add(EvidenceFlag::patternMatch);
    if (tonicizationOnly)
        situation.localKey.evidence.add(EvidenceFlag::tonicization);
    else
        situation.localKey.evidence.add(EvidenceFlag::localCadence);
    if (context.topLevel.evidence.has(EvidenceFlag::confirmedResolution))
        situation.localKey.evidence.add(EvidenceFlag::confirmedResolution);

    situation.localPattern = context.topLevel;
    if (situation.nextChordAvailable)
        situation.localHarmonic = analyzeHarmonicFunction(situation.currentChord,
                                                          context.center,
                                                          situation.nextChord);
    else
        situation.localHarmonic = analyzeHarmonicFunction(situation.currentChord,
                                                          context.center);

    situation.evidence.add(EvidenceFlag::inferredLocalCenter);
    situation.evidence.add(EvidenceFlag::patternMatch);
    if (tonicizationOnly)
        situation.evidence.add(EvidenceFlag::tonicization);
    else
        situation.evidence.add(EvidenceFlag::localCadence);
    if (context.topLevel.evidence.has(EvidenceFlag::confirmedResolution))
        situation.evidence.add(EvidenceFlag::confirmedResolution);

    analyzeAmbiguityAndConfidence(situation);
}

void applyPatternWindowContext(HarmonicSituation& situation,
                               const PatternTimelineWindow& patternWindow) noexcept
{
    const auto count = static_cast<int>(patternWindow.chordCount);
    const auto current = patternWindow.currentIndex;
    if (! situation.valid || count <= 0 || current < 0 || current >= count)
        return;

    std::array<NormalizedChord, kMaxPatternWindowChords> chords {};
    for (int index = 0; index < count; ++index)
        chords[static_cast<std::size_t>(index)] =
            normalizeChord(patternWindow.chords[static_cast<std::size_t>(index)]);

    // Existing 0.3f fix1 five-member local cadence remains the most specific
    // top-level event and therefore wins over shorter nested relations.
    for (int start = 0; start + 4 < count; ++start)
    {
        if (current < start || current > start + 4)
            continue;

        const auto& iii = chords[static_cast<std::size_t>(start)];
        const auto& viDominant = chords[static_cast<std::size_t>(start + 1)];
        const auto& ii = chords[static_cast<std::size_t>(start + 2)];
        const auto& v = chords[static_cast<std::size_t>(start + 3)];
        const auto& tonic = chords[static_cast<std::size_t>(start + 4)];
        if (! matchesMajorCadentialChain(iii, viDominant, ii, v, tonic))
            continue;

        const auto position = current - start;
        auto context = makePatternContext(
            HarmonicPatternType::majorCadentialChain,
            makePatternCenter(tonic, KeyMode::major),
            position,
            5,
            patternWindow.chords[static_cast<std::size_t>(start)].startPpq,
            patternWindow.chords[static_cast<std::size_t>(start + 4)].startPpq);

        if (position == 1)
        {
            addNestedPattern(context,
                             makePattern(HarmonicPatternType::secondaryDominant,
                                         PatternMemberRole::dominant,
                                         0,
                                         2,
                                         ConfidenceLevel::confirmed,
                                         false,
                                         true,
                                         true));
        }
        else if (position == 2)
        {
            addNestedPattern(context,
                             makePattern(HarmonicPatternType::secondaryDominant,
                                         PatternMemberRole::resolution,
                                         1,
                                         2,
                                         ConfidenceLevel::confirmed,
                                         true,
                                         false,
                                         true));
            addNestedPattern(context,
                             makePattern(HarmonicPatternType::majorIiVI,
                                         PatternMemberRole::predominant,
                                         0,
                                         3,
                                         ConfidenceLevel::confirmed,
                                         false,
                                         true));
        }
        else if (position == 3)
        {
            addNestedPattern(context,
                             makePattern(HarmonicPatternType::majorIiVI,
                                         PatternMemberRole::dominant,
                                         1,
                                         3,
                                         ConfidenceLevel::confirmed,
                                         true,
                                         true,
                                         true));
        }
        else if (position == 4)
        {
            addNestedPattern(context,
                             makePattern(HarmonicPatternType::majorIiVI,
                                         PatternMemberRole::resolution,
                                         2,
                                         3,
                                         ConfidenceLevel::confirmed,
                                         true,
                                         false,
                                         true));
        }

        applyPatternContext(situation, context);
        return;
    }

    // Full four-member cycles are reconstructed only when every member is
    // actually present in the bounded timeline. This prevents VI-ii-V from
    // fabricating a missing I and gives iii-vi-ii-V first-class identity.
    for (int start = 0; start + 3 < count; ++start)
    {
        if (current < start || current > start + 3)
            continue;

        const auto& first = chords[static_cast<std::size_t>(start)];
        const auto& second = chords[static_cast<std::size_t>(start + 1)];
        const auto& third = chords[static_cast<std::size_t>(start + 2)];
        const auto& fourth = chords[static_cast<std::size_t>(start + 3)];

        HarmonicPatternType type = HarmonicPatternType::undefined;
        if (matchesMajorTurnaround(first, second, third, fourth,
                                   situation.globalKey.key))
        {
            type = HarmonicPatternType::turnaroundIVIiiV;
        }
        else if (matchesMajorIiiViIiV(first, second, third, fourth,
                                      situation.globalKey.key))
        {
            type = HarmonicPatternType::majorIiiViIiV;
        }

        if (type == HarmonicPatternType::undefined)
            continue;

        const auto position = current - start;
        const auto context = makePatternContext(
            type,
            situation.globalKey.key,
            position,
            4,
            patternWindow.chords[static_cast<std::size_t>(start)].startPpq,
            -1.0);
        applyPatternContext(situation, context);
        return;
    }

    for (int start = 0; start + 2 < count; ++start)
    {
        if (current < start || current > start + 2)
            continue;

        const auto& first = chords[static_cast<std::size_t>(start)];
        const auto& dominant = chords[static_cast<std::size_t>(start + 1)];
        const auto& tonic = chords[static_cast<std::size_t>(start + 2)];

        HarmonicPatternType type = HarmonicPatternType::undefined;
        KeyMode mode = KeyMode::undefined;
        if (matchesMajorIiVI(first, dominant, tonic))
        {
            type = HarmonicPatternType::majorIiVI;
            mode = KeyMode::major;
        }
        else if (matchesMinorIiVI(first, dominant, tonic))
        {
            type = HarmonicPatternType::minorIiHalfDimVi;
            mode = KeyMode::minor;
        }
        else if (matchesMinorIvVI(first, dominant, tonic))
        {
            type = HarmonicPatternType::minorIvVi;
            mode = KeyMode::minor;
        }

        if (type == HarmonicPatternType::undefined)
            continue;

        const auto position = current - start;
        const auto context = makePatternContext(
            type,
            makePatternCenter(tonic, mode),
            position,
            3,
            patternWindow.chords[static_cast<std::size_t>(start)].startPpq,
            patternWindow.chords[static_cast<std::size_t>(start + 2)].startPpq);
        applyPatternContext(situation, context);
        return;
    }
}

HarmonicSituation analyzeBaseSituation(const TimelineHarmonicSnapshot& snapshot,
                                       const PatternTimelineWindow* patternWindow) noexcept
{
    auto result = buildHarmonicSituation(snapshot);
    if (! result.valid)
        return result;

    applyImpliedDominantReading(result);

    const auto allowIncompleteCadenceCandidates = patternWindow == nullptr
        || ! incompleteCadenceContradictedByWindow(*patternWindow);

    result.pattern = recognizePattern(result, allowIncompleteCadenceCandidates);
    mergePatternEvidence(result);
    analyzeLocalKeyCenter(result, allowIncompleteCadenceCandidates);
    return result;
}
}

HarmonicSituation analyzeHarmonicSituation(const TimelineHarmonicSnapshot& snapshot) noexcept
{
    auto result = analyzeBaseSituation(snapshot, nullptr);
    if (result.valid && result.evidence.interpretation == InterpretationStatus::unknown)
        result.evidence.markUnique();
    return result;
}

HarmonicSituation analyzeHarmonicSituation(const TimelineHarmonicSnapshot& snapshot,
                                           const PatternTimelineWindow& patternWindow) noexcept
{
    auto result = analyzeBaseSituation(snapshot, &patternWindow);
    if (! result.valid)
        return result;

    applyPatternWindowContext(result, patternWindow);

    if (result.evidence.interpretation == InterpretationStatus::unknown)
        result.evidence.markUnique();

    return result;
}
}
