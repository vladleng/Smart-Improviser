#pragma once

#include "core/model/Phrase.h"
#include <optional>
#include <variant>

namespace smartimproviser::harmony
{
enum class LibraryDomain : std::uint8_t { common, user };
enum class LibraryDerivation : std::uint8_t { copy, variant };

// Exact revision of a library item, distinct from a harmonic source-rule version.
struct LibraryItemReference
{
    LibraryDomain domain = LibraryDomain::user;
    std::string id;
    std::uint64_t revision = 0;
};
struct LibraryLineage
{
    LibraryDerivation kind = LibraryDerivation::copy;
    LibraryItemReference parent; // Immediate source; retain older ancestors.
    std::vector<LibraryItemReference> ancestors;
};
enum class ContentPermission : std::uint8_t { unknown, ownWork, publicDomain, licensed };
struct LibrarySourceMetadata
{
    std::string source;
    std::string author;
    std::string license;
    std::string permissionEvidence; // Attribution/licence basis, never inferred from a URL.
    ContentPermission permission = ContentPermission::unknown;
};

// Every omitted field stays unknown. No zero-duration or degree-0 placeholder
// is promoted into a ready Phrase.
struct IdeaNote
{
    std::optional<RelativePitch> pitch; // chordIndex -1 may mean an unknown slot.
    std::optional<double> beatOffset;
    std::optional<double> durationBeats;
    std::optional<int> octaveOffset;
    std::optional<PhraseNoteRole> harmonicRole;
    std::optional<bool> target;
    std::optional<PhraseFingering> fingering;
};
struct Idea
{
    std::string text;
    std::vector<std::string> conceptRuleIds;
    std::string rhythmNotes;
    std::string harmonyNotes;
    std::vector<IdeaNote> notes;
    std::vector<PhraseRegisterReference> registerReferences;
    std::optional<HarmonicPatternType> harmonicPattern;
};

// A discriminated value owns exactly one musical payload. A title or explicit
// choice of Phrase does not bypass validation. Phrase::id/name remain legacy
// musical payload metadata; this outer id/revision is authoritative for the library.
struct LibraryRecord
{
    std::string id;
    std::uint64_t revision = 0;
    LibraryDomain domain = LibraryDomain::user;
    std::string name;
    std::vector<std::string> tags;
    std::string explanation;
    LibrarySourceMetadata source;
    std::optional<LibraryLineage> lineage;
    std::variant<Idea, Phrase> content = Idea{};
};
}
