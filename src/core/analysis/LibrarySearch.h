#pragma once
#include "core/analysis/LibraryValidation.h"
#include "core/analysis/PhraseMatcher.h"
#include "core/analysis/TensionTimeline.h"

namespace smartimproviser::harmony
{
struct LibrarySearchQuery {
    PhraseMatchRequest match;
    std::optional<TensionCurve> curve; // If present, replaces the scalar request.
    double placementStartBeat=0;
    std::string text, tag, concept;
    std::optional<PhraseRole> role;
    std::optional<HarmonicPatternType> pattern;
    std::optional<int> patternPosition; // Zero based, relative to the requested pattern.
    std::optional<HarmonicFunction> function; // Context filter; preserve all interpretations.
};
enum class LibrarySearchState : std::uint8_t {
    eligible, draft, invalidRecord, metadataMismatch, insufficientMetadata,
    insufficientContext, harmonicMismatch, tensionUnknown, tensionMismatch, invalidTension
};
struct LibrarySearchEntry {
    LibraryRecord record; // Detached snapshot.
    LibrarySearchState state=LibrarySearchState::draft;
    LibraryValidation validation;
    PhraseMatchResult assessment;
    std::optional<PhraseCurveMatchResult> curveAssessment;
    int specificity=0; // Only ranked after eligibility; never harmonic confidence.
    std::vector<std::string> explanations;
    bool eligible() const noexcept {return state==LibrarySearchState::eligible;}
};
struct LibrarySearchResult {
    std::vector<LibrarySearchEntry> entries;
    std::size_t eligibleCount=0;
    std::string explanation;
};
LibrarySearchResult searchLibrary(const std::vector<LibraryRecord>&,const LibrarySearchQuery&);
}
