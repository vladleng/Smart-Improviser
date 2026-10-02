#pragma once
#include "core/analysis/PhraseLibrary.h"
#include <span>

namespace smartimproviser::harmony
{
enum class LibraryArchiveStatus : std::uint8_t { success, malformed, futureSchema, unsupportedSchema, invalidSnapshot, tooLarge };
struct LibraryArchive
{
    std::string storeId;
    std::uint64_t generation = 0;
    LibraryUserSnapshot users;
};
struct LibraryArchiveResult
{
    LibraryArchiveStatus status = LibraryArchiveStatus::success;
    std::optional<LibraryArchive> archive;
    std::vector<std::uint8_t> bytes;
    std::string explanation;
    bool succeeded() const noexcept { return status==LibraryArchiveStatus::success; }
};
inline constexpr std::size_t maxLibraryArchiveBytes=32*1024*1024;
LibraryArchiveResult encodeLibraryArchive(const LibraryArchive&);
LibraryArchiveResult decodeLibraryArchive(std::span<const std::uint8_t>);
// Strict portable import: existing IDs (including tombstones) are conflicts,
// never implicit overwrites. Entire batch succeeds or the target is unchanged.
LibraryOperation mergeLibraryUserSnapshot(LibraryUserSnapshot& target, const LibraryUserSnapshot& incoming);
}
