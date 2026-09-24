#include "core/analysis/LocalKeyCenterAnalyzer.h"
#include "core/analysis/AmbiguityAnalyzer.h"

namespace smartimproviser::harmony
{
namespace
{
constexpr int wrap12(int value) noexcept
{
    value %= kPitchClassCount;
    return value < 0 ? value + kPitchClassCount : value;
}

std::int32_t preferredFifthsForPitchClass(int pitchClass) noexcept
{
    switch (wrap12(pitchClass))
    {
        case 0: return 0;
        case 1: return -5;
        case 2: return 2;
        case 3: return -3;
        case 4: return 4;
        case 5: return -1;
        case 6: return 6;
        case 7: return 1;
        case 8: return -4;
        case 9: return 3;
        case 10: return -2;
        case 11: return 5;
        default: return 0;
    }
}

NormalizedKey makeKey(int rootPitchClass, KeyMode mode) noexcept
{
    NormalizedKey key;
    if (mode != KeyMode::major && mode != KeyMode::minor)
        return key;

    static constexpr int majorIntervals[] = { 0, 2, 4, 5, 7, 9, 11 };
    static constexpr int minorIntervals[] = { 0, 2, 3, 5, 7, 8, 10 };

    key.valid = true;
    key.rootPitchClass = wrap12(rootPitchClass);
    key.rootFifths = preferredFifthsForPitchClass(key.rootPitchClass);
    key.mode = mode;

    const auto* intervals = mode == KeyMode::major ? majorIntervals : minorIntervals;
    for (int i = 0; i < 7; ++i)
        key.tones[static_cast<std::size_t>(intervals[i])] = true;

    return key;
}

bool sameKey(const NormalizedKey& a, const NormalizedKey& b) noexcept
{
    return a.valid && b.valid
        && a.rootPitchClass == b.rootPitchClass
        && a.mode == b.mode;
}

KeyMode modeFromTargetChord(const NormalizedChord& chord) noexcept
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

KeyMode modeFromPredominant(const NormalizedChord& chord) noexcept
{
    if (chord.quality == ChordQuality::minor)
        return KeyMode::major;
    if (chord.quality == ChordQuality::halfDiminished)
        return KeyMode::minor;
    return KeyMode::undefined;
}

bool isOrdinaryDominantOfRoot(const NormalizedChord& chord, int targetRoot) noexcept
{
    return chord.valid
        && chord.quality == ChordQuality::dominant
        && wrap12(chord.rootPitchClass - targetRoot) == 7;
}

bool isSubstituteDominantOfRoot(const NormalizedChord& chord, int targetRoot) noexcept
{
    return chord.valid
        && chord.quality == ChordQuality::dominant
        && wrap12(chord.rootPitchClass - targetRoot) == 1;
}

bool isPredominantForRoot(const NormalizedChord& chord,
                          int targetRoot,
                          KeyMode mode) noexcept
{
    if (! chord.valid || wrap12(chord.rootPitchClass - targetRoot) != 2)
        return false;

    if (mode == KeyMode::major)
        return chord.quality == ChordQuality::minor;
    if (mode == KeyMode::minor)
        return chord.quality == ChordQuality::halfDiminished;
    return false;
}

HarmonicPattern makeLocalPattern(HarmonicPatternType type,
                                 PatternMemberRole role,
                                 int position,
                                 int length,
                                 ConfidenceLevel confidence,
                                 bool previous = false,
                                 bool next = false,
                                 bool resolution = false) noexcept
{
    HarmonicPattern pattern;
    pattern.type = type;
    pattern.role = role;
    pattern.positionIndex = position;
    pattern.length = length;
    pattern.evidence.confidence = confidence;
    pattern.evidence.markUnique();
    pattern.evidence.add(EvidenceFlag::patternMatch);
    if (previous)
        pattern.evidence.add(EvidenceFlag::previousChord);
    if (next)
        pattern.evidence.add(EvidenceFlag::nextChord);
    if (resolution)
        pattern.evidence.add(EvidenceFlag::confirmedResolution);
    return pattern;
}

KeyCenter makeCenter(const NormalizedKey& key,
                     KeyCenterScope scope,
                     KeyCenterStatus status,
                     ConfidenceLevel confidence,
                     bool previous = false,
                     bool next = false,
                     bool resolution = false) noexcept
{
    KeyCenter center;
    if (! key.valid)
        return center;

    center.valid = true;
    center.key = key;
    center.scope = scope;
    center.status = status;
    center.evidence.confidence = confidence;
    center.evidence.markUnique();
    center.evidence.add(EvidenceFlag::inferredLocalCenter);

    if (status == KeyCenterStatus::candidate)
        center.evidence.add(EvidenceFlag::candidateLocalCenter);
    if (status == KeyCenterStatus::tonicized)
        center.evidence.add(EvidenceFlag::tonicization);
    if (status == KeyCenterStatus::established)
        center.evidence.add(EvidenceFlag::localCadence);
    if (status == KeyCenterStatus::modulationCandidate)
        center.evidence.add(EvidenceFlag::modulationEvidence);
    if (previous)
        center.evidence.add(EvidenceFlag::previousChord);
    if (next)
        center.evidence.add(EvidenceFlag::nextChord);
    if (resolution)
        center.evidence.add(EvidenceFlag::confirmedResolution);

    return center;
}

void applyCenter(HarmonicSituation& situation,
                 const KeyCenter& center,
                 const HarmonicPattern& localPattern) noexcept
{
    if (! center.valid || sameKey(center.key, situation.globalKey.key))
    {
        situation.localKey = {};
        situation.localHarmonic = {};
        situation.localPattern = {};
        analyzeAmbiguityAndConfidence(situation);
        return;
    }

    situation.localKey = center;
    situation.localPattern = localPattern;

    if (situation.nextChordAvailable)
        situation.localHarmonic = analyzeHarmonicFunction(situation.currentChord,
                                                          center.key,
                                                          situation.nextChord);
    else
        situation.localHarmonic = analyzeHarmonicFunction(situation.currentChord,
                                                          center.key);

    situation.evidence.add(EvidenceFlag::inferredLocalCenter);
    if (center.evidence.has(EvidenceFlag::candidateLocalCenter))
        situation.evidence.add(EvidenceFlag::candidateLocalCenter);
    if (center.evidence.has(EvidenceFlag::tonicization))
        situation.evidence.add(EvidenceFlag::tonicization);
    if (center.evidence.has(EvidenceFlag::localCadence))
        situation.evidence.add(EvidenceFlag::localCadence);
    if (center.evidence.has(EvidenceFlag::modulationEvidence))
        situation.evidence.add(EvidenceFlag::modulationEvidence);

    analyzeAmbiguityAndConfidence(situation);
}

bool isDiatonicTo(const NormalizedChord& chord, const NormalizedKey& key) noexcept
{
    return analyzeHarmonicFunction(chord, key).relation == HarmonicRelation::diatonic;
}
}

void analyzeLocalKeyCenter(HarmonicSituation& situation) noexcept
{
    situation.localKey = {};
    situation.localHarmonic = {};
    situation.localPattern = {};

    // Seed the ambiguity layer after global pattern recognition even when no
    // local center is eventually found. applyCenter() refreshes it whenever a
    // local interpretation is added or deliberately rejected as redundant.
    analyzeAmbiguityAndConfidence(situation);

    if (! situation.valid || ! situation.globalKey.valid)
        return;

    if (situation.previousChordAvailable
        && situation.nextChordAvailable
        && situation.currentChord.quality == ChordQuality::dominant)
    {
        const auto mode = modeFromTargetChord(situation.nextChord);
        const auto targetRoot = situation.nextChord.rootPitchClass;
        const auto ordinary = isOrdinaryDominantOfRoot(situation.currentChord, targetRoot);
        const auto substitute = isSubstituteDominantOfRoot(situation.currentChord, targetRoot);

        if ((ordinary || substitute)
            && isPredominantForRoot(situation.previousChord, targetRoot, mode))
        {
            const auto key = makeKey(targetRoot, mode);
            const auto type = substitute
                ? HarmonicPatternType::tritoneSubstitution
                : (mode == KeyMode::minor
                    ? HarmonicPatternType::minorIiHalfDimVi
                    : HarmonicPatternType::majorIiVI);
            const auto role = substitute
                ? PatternMemberRole::substituteDominant
                : PatternMemberRole::dominant;

            applyCenter(situation,
                        makeCenter(key,
                                   KeyCenterScope::local,
                                   KeyCenterStatus::established,
                                   ConfidenceLevel::confirmed,
                                   true,
                                   true,
                                   true),
                        makeLocalPattern(type,
                                         role,
                                         1,
                                         3,
                                         ConfidenceLevel::confirmed,
                                         true,
                                         true,
                                         true));
            if (situation.localKey.valid)
                return;
        }
    }

    if (situation.nextChordAvailable)
    {
        const auto mode = modeFromPredominant(situation.currentChord);
        if (mode != KeyMode::undefined)
        {
            const auto targetRoot = wrap12(situation.currentChord.rootPitchClass - 2);
            const auto ordinary = isOrdinaryDominantOfRoot(situation.nextChord, targetRoot);
            const auto substitute = isSubstituteDominantOfRoot(situation.nextChord, targetRoot);
            if (ordinary || substitute)
            {
                const auto key = makeKey(targetRoot, mode);
                const auto type = substitute
                    ? HarmonicPatternType::tritoneSubstitution
                    : (mode == KeyMode::minor
                        ? HarmonicPatternType::minorIiHalfDimVi
                        : HarmonicPatternType::majorIiVI);

                applyCenter(situation,
                            makeCenter(key,
                                       KeyCenterScope::temporary,
                                       KeyCenterStatus::candidate,
                                       ConfidenceLevel::high,
                                       false,
                                       true,
                                       false),
                            makeLocalPattern(type,
                                             PatternMemberRole::predominant,
                                             0,
                                             3,
                                             ConfidenceLevel::high,
                                             false,
                                             true,
                                             false));
                if (situation.localKey.valid)
                    return;
            }
        }
    }

    if (situation.previousChordAvailable
        && situation.currentChord.quality == ChordQuality::dominant)
    {
        const auto mode = modeFromPredominant(situation.previousChord);
        if (mode != KeyMode::undefined)
        {
            const auto targetRoot = wrap12(situation.previousChord.rootPitchClass - 2);
            const auto ordinary = isOrdinaryDominantOfRoot(situation.currentChord, targetRoot);
            const auto substitute = isSubstituteDominantOfRoot(situation.currentChord, targetRoot);
            if (ordinary || substitute)
            {
                const auto key = makeKey(targetRoot, mode);
                const auto type = substitute
                    ? HarmonicPatternType::tritoneSubstitution
                    : (mode == KeyMode::minor
                        ? HarmonicPatternType::minorIiHalfDimVi
                        : HarmonicPatternType::majorIiVI);
                const auto role = substitute
                    ? PatternMemberRole::substituteDominant
                    : PatternMemberRole::dominant;

                applyCenter(situation,
                            makeCenter(key,
                                       KeyCenterScope::temporary,
                                       KeyCenterStatus::candidate,
                                       ConfidenceLevel::high,
                                       true,
                                       false,
                                       false),
                            makeLocalPattern(type,
                                             role,
                                             1,
                                             3,
                                             ConfidenceLevel::high,
                                             true,
                                             false,
                                             false));
                if (situation.localKey.valid)
                    return;
            }
        }
    }

    if (situation.nextChordAvailable
        && situation.currentChord.quality == ChordQuality::dominant)
    {
        const auto mode = modeFromTargetChord(situation.nextChord);
        const auto targetRoot = situation.nextChord.rootPitchClass;
        const auto ordinary = isOrdinaryDominantOfRoot(situation.currentChord, targetRoot);
        const auto substitute = isSubstituteDominantOfRoot(situation.currentChord, targetRoot);

        if ((ordinary || substitute) && mode != KeyMode::undefined)
        {
            const auto key = makeKey(targetRoot, mode);
            applyCenter(situation,
                        makeCenter(key,
                                   KeyCenterScope::temporary,
                                   KeyCenterStatus::tonicized,
                                   ConfidenceLevel::confirmed,
                                   false,
                                   true,
                                   true),
                        makeLocalPattern(substitute
                                             ? HarmonicPatternType::tritoneSubstitution
                                             : HarmonicPatternType::dominantToTonic,
                                         substitute
                                             ? PatternMemberRole::substituteDominant
                                             : PatternMemberRole::dominant,
                                         0,
                                         2,
                                         ConfidenceLevel::confirmed,
                                         false,
                                         true,
                                         true));
            if (situation.localKey.valid)
                return;
        }
    }

    if (situation.previousChordAvailable)
    {
        const auto mode = modeFromTargetChord(situation.currentChord);
        const auto targetRoot = situation.currentChord.rootPitchClass;
        const auto ordinary = isOrdinaryDominantOfRoot(situation.previousChord, targetRoot);
        const auto substitute = isSubstituteDominantOfRoot(situation.previousChord, targetRoot);

        if ((ordinary || substitute) && mode != KeyMode::undefined)
        {
            const auto key = makeKey(targetRoot, mode);
            auto scope = KeyCenterScope::temporary;
            auto status = KeyCenterStatus::tonicized;
            auto confidence = ConfidenceLevel::confirmed;

            if (situation.nextChordAvailable
                && isDiatonicTo(situation.nextChord, key)
                && ! isDiatonicTo(situation.nextChord, situation.globalKey.key))
            {
                scope = KeyCenterScope::local;
                status = KeyCenterStatus::modulationCandidate;
                confidence = ConfidenceLevel::high;
            }

            applyCenter(situation,
                        makeCenter(key,
                                   scope,
                                   status,
                                   confidence,
                                   true,
                                   situation.nextChordAvailable,
                                   true),
                        makeLocalPattern(substitute
                                             ? HarmonicPatternType::tritoneSubstitution
                                             : HarmonicPatternType::dominantToTonic,
                                         PatternMemberRole::resolution,
                                         1,
                                         2,
                                         confidence,
                                         true,
                                         situation.nextChordAvailable,
                                         true));
        }
    }
}
}
