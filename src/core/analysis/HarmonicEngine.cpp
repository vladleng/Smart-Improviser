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

    // Full minor iv-V-i. This is distinct from iiø-V-i while all three
    // members are visible; the two cadences intentionally collapse to V-i
    // at the final tonic when the older predominant has left the Stage 1 window.
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

    // On the tonic only V-I / V-i is actually visible. Do not invent the
    // earlier predominant (ii, iiø or iv) from a two-chord window.
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

PatternMemberRole cadenceRole(int position, int length) noexcept
{
    if (length == 5)
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

    switch (position)
    {
        case 0: return PatternMemberRole::predominant;
        case 1: return PatternMemberRole::dominant;
        case 2: return PatternMemberRole::resolution;
        default: return PatternMemberRole::undefined;
    }
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
    context.topLevel = makePattern(type,
                                   cadenceRole(position, length),
                                   position,
                                   length,
                                   ConfidenceLevel::confirmed,
                                   position > 0,
                                   position + 1 < length,
                                   position == length - 1);
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

    situation.localKey.valid = true;
    situation.localKey.key = context.center;
    situation.localKey.scope = KeyCenterScope::local;
    situation.localKey.status = KeyCenterStatus::established;
    situation.localKey.evidence.confidence = ConfidenceLevel::confirmed;
    situation.localKey.evidence.markUnique();
    situation.localKey.evidence.add(EvidenceFlag::inferredLocalCenter);
    situation.localKey.evidence.add(EvidenceFlag::localCadence);
    situation.localKey.evidence.add(EvidenceFlag::patternMatch);
    if (context.topLevel.positionIndex == context.topLevel.length - 1)
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
    situation.evidence.add(EvidenceFlag::localCadence);
    situation.evidence.add(EvidenceFlag::patternMatch);
    if (context.topLevel.positionIndex == context.topLevel.length - 1)
        situation.evidence.add(EvidenceFlag::confirmedResolution);

    analyzeAmbiguityAndConfidence(situation);
}

void applyPatternWindowContext(HarmonicSituation& situation,
                               const TimelineHarmonicSnapshot& snapshot) noexcept
{
    const auto count = static_cast<int>(snapshot.patternWindow.chordCount);
    const auto current = snapshot.patternWindow.currentIndex;
    if (! situation.valid || count <= 0 || current < 0 || current >= count)
        return;

    std::array<NormalizedChord, kMaxPatternWindowChords> chords {};
    for (int index = 0; index < count; ++index)
        chords[static_cast<std::size_t>(index)] =
            normalizeChord(snapshot.patternWindow.chords[static_cast<std::size_t>(index)]);

    // A five-member cadential chain is the top-level event. Its nested V/ii→ii
    // and ii-V-I relations remain available as PatternContext evidence, but do
    // not replace the user's primary cadence label.
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
            snapshot.patternWindow.chords[static_cast<std::size_t>(start)].startPpq,
            snapshot.patternWindow.chords[static_cast<std::size_t>(start + 4)].startPpq);

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

    // Confirmed three-member cadences remain intact on the resolution chord.
    // This is the carried PatternContext evidence that 0.3f lacked.
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
            snapshot.patternWindow.chords[static_cast<std::size_t>(start)].startPpq,
            snapshot.patternWindow.chords[static_cast<std::size_t>(start + 2)].startPpq);
        applyPatternContext(situation, context);
        return;
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

    // 0.3f fix1: rebuild a bounded PatternContext from timeline events on each
    // analysis pass. This preserves confirmed cadence identity on the tonic and
    // recognizes the explicit iii-VI7-ii-V-I chain without stale runtime memory.
    applyPatternWindowContext(result, snapshot);

    if (result.evidence.interpretation == InterpretationStatus::unknown)
        result.evidence.markUnique();

    return result;
}
}
