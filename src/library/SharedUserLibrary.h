#pragma once
#include "core/analysis/LibraryArchive.h"
#include <juce_core/juce_core.h>

namespace smartimproviser::harmony
{
enum class LibraryStorageStatus : std::uint8_t { success, busy, ioError, malformed, futureSchema, invalidData, conflict, writeFailed };
struct LibraryStorageStamp
{
    bool exists=false;
    std::string storeId;
    std::uint64_t generation=0;
    std::uint32_t checksum=0;
    bool operator==(const LibraryStorageStamp&) const = default;
};
struct LibraryStorageResult
{
    LibraryStorageStatus status=LibraryStorageStatus::success;
    LibraryUserSnapshot users;
    LibraryStorageStamp stamp;
    std::vector<std::uint8_t> exportedBytes;
    std::string explanation;
    bool succeeded() const noexcept { return status==LibraryStorageStatus::success; }
};
enum class LibraryWriteStage : std::uint8_t { beforeWrite, beforeReplace };
class SharedUserLibrary
{
public:
    using WriteHook=std::function<bool(LibraryWriteStage)>; // Failure injection for storage tests.
    static juce::File defaultFile();
    explicit SharedUserLibrary(juce::File file=defaultFile(), WriteHook hook={});
    LibraryStorageResult read() const;
    LibraryStorageResult commit(const LibraryUserSnapshot&,const LibraryStorageStamp& expected) const;
    LibraryStorageResult exportData() const;
    LibraryStorageResult importData(std::span<const std::uint8_t>) const;
    LibraryStorageResult loadInto(PhraseLibrary&) const;
    LibraryStorageResult commitFrom(const PhraseLibrary&,const LibraryStorageStamp&) const;
private:
    LibraryStorageResult readUnlocked() const;
    LibraryStorageResult writeUnlocked(const LibraryUserSnapshot&,const LibraryStorageStamp&) const;
    juce::File storage_;
    WriteHook hook_;
    mutable juce::InterProcessLock processLock_;
    inline static juce::CriticalSection threadLock_;
};
}
