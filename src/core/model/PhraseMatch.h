#pragma once
#include "core/model/Phrase.h"
#include <optional>
#include <vector>

namespace smartimproviser::harmony
{
struct PhraseMatchSlot
{
    ImprovisationResult material; // Already analyzed; no caller-provided replacement source.
    double startBeat = 0.0;
    double endBeat = 0.0;
};
struct PhraseMatchRequest
{
    std::vector<PhraseMatchSlot> slots;
    std::optional<TensionLevel> requestedTension; // null = all, not T1.
};
enum class PhraseCompatibility : std::uint8_t { insufficientContext, incompatible, compatible };
enum class PhraseTensionMatch : std::uint8_t { notEvaluated, any, matches, unclassified, differentLevel };
enum class PhraseMatchReason : std::uint8_t
{
    invalidPhrase, missingContext, invalidTiming, invalidPitch, chordMismatch,
    patternMismatch, sourceUnavailable, ambiguousSource, sourceVersion,
    destinationMissing, destinationMismatch, destinationUnconfirmed,
    unsupportedNote, invalidApproach, missingApproach, unsupportedOutside,
    tensionUnclassified, tensionMismatch, matched
};
struct PhraseMatchDiagnostic
{
    PhraseMatchReason reason = PhraseMatchReason::missingContext;
    int chordIndex = -1;
    int noteIndex = -1;
    std::string explanation;
};
struct PhraseDestinationSnapshot
{
    bool available = false, confirmed = false;
    int rootPitchClass = -1;
    ChordQuality quality = ChordQuality::undefined;
    AnalysisEvidence evidence;
};
struct PhraseMatchContext
{
    NormalizedChord actualChord;
    NormalizedChord actualNextChord;
    PhraseDestinationSnapshot destination; // May be hypothetical; never replaces actualNextChord.
    ResolutionTarget resolution;
    std::string sourceRuleId;
    int sourceRuleVersion = 0;
    int sourceRootPitchClass = -1;
    AnalysisEvidence sourceEvidence;
    std::vector<AnalysisEvidence> sourceEvidenceAlternatives;
    std::vector<int> sourceInterpretationIndices;
};
struct PhraseMatchResult
{
    PhraseCompatibility harmonic = PhraseCompatibility::insufficientContext;
    PhraseTensionMatch tension = PhraseTensionMatch::notEvaluated;
    std::vector<PhraseMatchDiagnostic> diagnostics;
    std::vector<PhraseMatchContext> contexts;
    std::vector<int> notePitchClasses; // Projection only, no mutation/MIDI/transposition output.
    bool eligible() const noexcept
    {
        return harmonic == PhraseCompatibility::compatible
            && (tension == PhraseTensionMatch::any || tension == PhraseTensionMatch::matches);
    }
};
}
