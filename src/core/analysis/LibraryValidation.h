#pragma once
#include "core/model/LibraryRecord.h"

namespace smartimproviser::harmony
{
enum class LibraryValidationScope : std::uint8_t { structure, searchReadiness, distribution };
struct LibraryValidationDiagnostic
{
    LibraryValidationScope scope;
    std::string field;
    std::string explanation;
};
struct LibraryValidation
{
    bool structurallyValid = true;
    bool readyForSearch = false; // Completeness only, NOT harmonic compatibility.
    bool distributable = false; // Common content with explicit checked metadata.
    std::vector<LibraryValidationDiagnostic> diagnostics;
};
// Context-independent checks only. Actual suitability still uses assessPhrase.
LibraryValidation validateLibraryRecord(const LibraryRecord&);
// Returns null for incomplete/ambiguous/out-of-range register; never guesses.
std::optional<int> phraseExampleMidiPitch(const Phrase&, std::size_t noteIndex);
}
