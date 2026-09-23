#include "core/analysis/HarmonicFunction.h"

namespace smartimproviser::harmony
{
namespace
{
constexpr int wrap12(int value) noexcept
{
    value %= kPitchClassCount;
    return value < 0 ? value + kPitchClassCount : value;
}

HarmonicFunction functionForDegree(int degree) noexcept
{
    switch (degree)
    {
        case 1:
        case 3:
        case 6:
            return HarmonicFunction::tonic;
        case 2:
        case 4:
            return HarmonicFunction::predominant;
        case 5:
        case 7:
            return HarmonicFunction::dominant;
        default:
            return HarmonicFunction::other;
    }
}

bool allChordTonesBelongToKey(const NormalizedChord& chord,
                              const NormalizedKey& key) noexcept
{
    if (! chord.valid || ! key.valid)
        return false;

    bool sawTone = false;
    for (int relative = 0; relative < kPitchClassCount; ++relative)
    {
        if (! chord.tones[static_cast<std::size_t>(relative)])
            continue;

        sawTone = true;
        const auto absolute = wrap12(chord.rootPitchClass + relative);
        if (! key.hasPitchClass(absolute))
            return false;
    }
    return sawTone;
}

bool pitchBelongsToMode(int absolutePitchClass, int tonicPitchClass, KeyMode mode) noexcept
{
    static constexpr bool majorMask[kPitchClassCount] =
        { true, false, true, false, true, true, false, true, false, true, false, true };
    static constexpr bool minorMask[kPitchClassCount] =
        { true, false, true, true, false, true, false, true, true, false, true, false };

    if (mode != KeyMode::major && mode != KeyMode::minor)
        return false;

    const auto relative = wrap12(absolutePitchClass - tonicPitchClass);
    return mode == KeyMode::major ? majorMask[relative] : minorMask[relative];
}

bool allChordTonesBelongToMode(const NormalizedChord& chord,
                               int tonicPitchClass,
                               KeyMode mode) noexcept
{
    if (! chord.valid)
        return false;

    bool sawTone = false;
    for (int relative = 0; relative < kPitchClassCount; ++relative)
    {
        if (! chord.tones[static_cast<std::size_t>(relative)])
            continue;

        sawTone = true;
        const auto absolute = wrap12(chord.rootPitchClass + relative);
        if (! pitchBelongsToMode(absolute, tonicPitchClass, mode))
            return false;
    }
    return sawTone;
}

KeyMode parallelMode(KeyMode mode) noexcept
{
    switch (mode)
    {
        case KeyMode::major: return KeyMode::minor;
        case KeyMode::minor: return KeyMode::major;
        default: return KeyMode::undefined;
    }
}

void addModalInterchangeEvidence(HarmonicAnalysis& result,
                                 const NormalizedChord& chord,
                                 const NormalizedKey& key) noexcept
{
    if (result.relation != HarmonicRelation::chromatic)
        return;

    const auto sourceMode = parallelMode(key.mode);
    if (sourceMode == KeyMode::undefined)
        return;

    if (allChordTonesBelongToMode(chord, key.rootPitchClass, sourceMode))
    {
        result.modalInterchangeCandidate = true;
        result.modalInterchangeSource = sourceMode;
    }
}

HarmonicAnalysis analyzeStatic(const NormalizedChord& chord,
                               const NormalizedKey& key) noexcept
{
    HarmonicAnalysis result;
    if (! chord.valid || ! key.valid)
        return result;

    result.valid = true;
    result.rootScaleDegree = scaleDegreeForPitchClass(key, chord.rootPitchClass);
    result.rootFunction = functionForDegree(result.rootScaleDegree);
    result.effectiveFunction = result.rootFunction;
    result.chordTonesDiatonic = allChordTonesBelongToKey(chord, key);
    result.relation = result.chordTonesDiatonic
        ? HarmonicRelation::diatonic
        : HarmonicRelation::chromatic;

    if (chord.quality == ChordQuality::dominant)
    {
        // Ordinary dominant motion resolves down a fifth / up a fourth.
        const auto targetPitchClass = wrap12(chord.rootPitchClass + 5);
        const auto targetDegree = scaleDegreeForPitchClass(key, targetPitchClass);
        const auto isPrimaryDominant = result.rootScaleDegree == 5 && targetDegree == 1;

        if (targetDegree > 0 && ! isPrimaryDominant)
        {
            result.appliedDominantCandidate = true;
            result.appliedTargetPitchClass = targetPitchClass;
            result.appliedTargetScaleDegree = targetDegree;
            result.effectiveFunction = HarmonicFunction::dominant;
        }

        // A tritone-substitute dominant resolves by semitone descent in the
        // root: Db7 -> C, Ab7 -> G, Bb7 -> A, etc. We can identify a candidate
        // from the global key without inventing a local key. Confirmation still
        // requires the real next chord.
        const auto substituteTargetPitchClass = wrap12(chord.rootPitchClass - 1);
        const auto substituteTargetDegree = scaleDegreeForPitchClass(key, substituteTargetPitchClass);
        if (substituteTargetDegree > 0)
        {
            result.substituteDominantCandidate = true;
            result.substituteTargetPitchClass = substituteTargetPitchClass;
            result.substituteTargetScaleDegree = substituteTargetDegree;

            // If the chord has only a substitute-dominant interpretation in the
            // current global key, expose that effective function immediately.
            // Ambiguous dual candidates are intentionally left for Stage 2/0.2e.
            if (! result.appliedDominantCandidate && ! isPrimaryDominant)
                result.effectiveFunction = HarmonicFunction::substituteDominant;
        }
    }

    addModalInterchangeEvidence(result, chord, key);
    return result;
}
}

HarmonicAnalysis analyzeHarmonicFunction(const NormalizedChord& chord,
                                         const NormalizedKey& key) noexcept
{
    return analyzeStatic(chord, key);
}

HarmonicAnalysis analyzeHarmonicFunction(const NormalizedChord& chord,
                                         const NormalizedKey& key,
                                         const NormalizedChord& nextChord) noexcept
{
    auto result = analyzeStatic(chord, key);
    if (! result.valid || ! nextChord.valid)
        return result;

    result.nextChordAvailable = true;
    result.nextChordRootPitchClass = nextChord.rootPitchClass;
    result.nextChordQuality = nextChord.quality;

    if (result.appliedDominantCandidate
        && result.appliedTargetPitchClass >= 0
        && nextChord.rootPitchClass == result.appliedTargetPitchClass)
    {
        result.appliedDominantConfirmed = true;
        result.effectiveFunction = HarmonicFunction::dominant;
    }

    if (result.substituteDominantCandidate
        && result.substituteTargetPitchClass >= 0
        && nextChord.rootPitchClass == result.substituteTargetPitchClass)
    {
        result.substituteDominantConfirmed = true;
        result.effectiveFunction = HarmonicFunction::substituteDominant;
    }

    if (chord.quality == ChordQuality::dominant)
    {
        const auto expectedTarget = wrap12(chord.rootPitchClass + 5);
        if (nextChord.rootPitchClass == expectedTarget)
        {
            result.dominantResolutionConfirmed = true;
            result.dominantTargetPitchClass = nextChord.rootPitchClass;
            result.dominantTargetQuality = nextChord.quality;
        }
    }

    return result;
}

const char* harmonicFunctionName(HarmonicFunction function) noexcept
{
    switch (function)
    {
        case HarmonicFunction::tonic: return "Tonic";
        case HarmonicFunction::predominant: return "Predominant";
        case HarmonicFunction::dominant: return "Dominant";
        case HarmonicFunction::substituteDominant: return "Substitute dominant";
        case HarmonicFunction::other: return "Other";
        case HarmonicFunction::undefined:
        default: return "Undefined";
    }
}

const char* harmonicRelationName(HarmonicRelation relation) noexcept
{
    switch (relation)
    {
        case HarmonicRelation::diatonic: return "Diatonic";
        case HarmonicRelation::chromatic: return "Chromatic";
        case HarmonicRelation::undefined:
        default: return "Undefined";
    }
}

const char* scaleDegreeName(int degree) noexcept
{
    switch (degree)
    {
        case 1: return "I";
        case 2: return "II";
        case 3: return "III";
        case 4: return "IV";
        case 5: return "V";
        case 6: return "VI";
        case 7: return "VII";
        default: return "-";
    }
}
}
