#pragma once
#include "core/model/PhraseMatch.h"
#include "core/model/TensionCurve.h"

namespace smartimproviser::harmony
{
enum class TensionTimelineReason : std::uint8_t
{
    invalidTiming, invalidLevel, overlappingSpans, undefinedProfileData,
    outsidePhrase, invalidPlacement, profileUnclassified, levelMismatch
};
enum class TensionTimelineScope : std::uint8_t { phraseProfile, desiredCurve, placement, comparison };
struct TensionTimelineDiagnostic
{
    TensionTimelineReason reason = TensionTimelineReason::invalidTiming;
    int spanIndex = -1;
    double startBeat = 0.0, endBeat = 0.0;
    std::string explanation;
    TensionTimelineScope scope = TensionTimelineScope::phraseProfile;
};
struct TensionTimelineValidation
{
    bool valid = true;
    std::vector<TensionTimelineDiagnostic> diagnostics;
};
TensionTimelineValidation validatePhraseTensionProfile(const Phrase&);
TensionTimelineValidation validateTensionCurve(const TensionCurve&);

// Scalar selection uses the same descriptive profile as curve matching.
PhraseTensionMatch matchPhraseTension(const Phrase&, std::optional<TensionLevel>);

enum class TensionCurveMatch : std::uint8_t
{
    notEvaluated, unconstrained, matches, unclassified, differentLevel, invalidData
};
struct TensionCurveComparison
{
    double songStartBeat = 0.0, songEndBeat = 0.0;
    std::optional<TensionLevel> desired, descriptive;
};
struct PhraseCurveMatchResult
{
    PhraseMatchResult harmonicMatch; // Evaluated with All, before any curve test.
    TensionCurveMatch tension = TensionCurveMatch::notEvaluated;
    std::vector<TensionTimelineDiagnostic> diagnostics;
    std::vector<TensionCurveComparison> comparisons;
    bool eligible() const noexcept
    {
        return harmonicMatch.harmonic == PhraseCompatibility::compatible
            && (tension == TensionCurveMatch::unconstrained || tension == TensionCurveMatch::matches);
    }
};
// Slots remain phrase-relative, curve song-absolute. The explicit placement
// converts time without mutating the phrase, profile, curve or harmonic slots.
// No library lookup, ranking, editor, persistence or audio-thread work.
PhraseCurveMatchResult assessPhraseAgainstCurve(const Phrase&,
    const std::vector<PhraseMatchSlot>&, const TensionCurve&, double placementStartBeat);
}
