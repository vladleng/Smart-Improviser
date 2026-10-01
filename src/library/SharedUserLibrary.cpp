#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#endif
#include "library/SharedUserLibrary.h"
#include <algorithm>
#include <limits>

namespace smartimproviser::harmony
{
namespace
{
LibraryStorageResult fail(LibraryStorageStatus status,const char* why)
{ LibraryStorageResult result; result.status=status; result.explanation=why; return result; }
struct ProcessGuard
{
    juce::InterProcessLock& lock; bool entered;
    explicit ProcessGuard(juce::InterProcessLock& l):lock(l),entered(l.enter(2000)) {}
    ~ProcessGuard() { if (entered) lock.exit(); }
};
juce::String lockName(const juce::File& file)
{
    auto name=file.getFullPathName();
   #if JUCE_WINDOWS
    name=name.toLowerCase();
   #endif
    return "SmartImproviserUserLibrary-"+juce::String::toHexString(name.hashCode64());
}
bool writeNative(const juce::File& file,std::span<const std::uint8_t> bytes)
{
   #if JUCE_WINDOWS
    const auto handle=CreateFileW(file.getFullPathName().toWideCharPointer(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
    if (handle==INVALID_HANDLE_VALUE) return false;
    DWORD written=0;
    bool ok=WriteFile(handle,bytes.data(),static_cast<DWORD>(bytes.size()),&written,nullptr)!=0 && written==bytes.size();
    if (ok) ok=FlushFileBuffers(handle)!=0;
    if (!CloseHandle(handle)) ok=false;
    return ok;
   #else
    const auto path=file.getFullPathName().toStdString();
    const int fd=::open(path.c_str(),O_WRONLY|O_CREAT|O_EXCL,0600);
    if (fd<0) return false;
    std::size_t done=0;
    while(done<bytes.size()) { const auto n=::write(fd,bytes.data()+done,bytes.size()-done); if(n<=0) break; done+=static_cast<std::size_t>(n); }
    bool ok=done==bytes.size() && ::fsync(fd)==0;
    if (::close(fd)!=0) ok=false;
    return ok;
   #endif
}
bool replaceNative(const juce::File& from,const juce::File& to)
{
   #if JUCE_WINDOWS
    return MoveFileExW(from.getFullPathName().toWideCharPointer(),to.getFullPathName().toWideCharPointer(),
        MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
   #else
    return ::rename(from.getFullPathName().toRawUTF8(),to.getFullPathName().toRawUTF8())==0;
   #endif
}
bool allow(const SharedUserLibrary::WriteHook& hook,LibraryWriteStage stage)
{
    try { return !hook || hook(stage); } catch (...) { return false; }
}
}
juce::File SharedUserLibrary::defaultFile()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Moon River Studio/Smart Improviser/user-library.silibrary");
}
SharedUserLibrary::SharedUserLibrary(juce::File file,WriteHook hook)
    :storage_(std::move(file)),hook_(std::move(hook)),processLock_(lockName(storage_)) {}
LibraryStorageResult SharedUserLibrary::readUnlocked() const
{
    LibraryStorageResult out;
    if (!storage_.exists()) return out; // Missing is empty, not a corrupt initialized library.
    const auto size=storage_.getSize();
    if (!storage_.existsAsFile() || size<0 || size>static_cast<juce::int64>(maxLibraryArchiveBytes))
        return fail(LibraryStorageStatus::malformed,"Library file is invalid or too large; preserve it.");
    juce::FileInputStream stream(storage_);
    if (!stream.openedOk()) return fail(LibraryStorageStatus::ioError,"Cannot open library file.");
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    if (stream.read(bytes.data(),static_cast<int>(bytes.size()))!=static_cast<int>(bytes.size()))
        return fail(LibraryStorageStatus::ioError,"Cannot read complete library file.");
    char extra=0;
    if (stream.read(&extra,1)!=0) return fail(LibraryStorageStatus::malformed,"Library changed while reading.");
    auto decoded=decodeLibraryArchive(bytes);
    if (!decoded.succeeded())
    {
        auto result=fail(decoded.status==LibraryArchiveStatus::futureSchema ? LibraryStorageStatus::futureSchema
            : LibraryStorageStatus::malformed,"Invalid library archive; no overwrite is permitted.");
        result.explanation=decoded.explanation; return result;
    }
    out.users=std::move(decoded.archive->users);
    out.stamp.exists=true; out.stamp.storeId=decoded.archive->storeId; out.stamp.generation=decoded.archive->generation;
    for (int n=0;n<4;++n) out.stamp.checksum|=std::uint32_t(bytes[16+n])<<(8*n);
    return out;
}
LibraryStorageResult SharedUserLibrary::read() const
{
    const juce::ScopedLock thread(threadLock_); ProcessGuard process(processLock_);
    if (!process.entered) return fail(LibraryStorageStatus::busy,"Library is busy in another process.");
    return readUnlocked();
}
LibraryStorageResult SharedUserLibrary::writeUnlocked(const LibraryUserSnapshot& users,const LibraryStorageStamp& current) const
{
    if (current.generation==std::numeric_limits<std::uint64_t>::max())
        return fail(LibraryStorageStatus::conflict,"Library generation overflow; no write.");
    LibraryArchive archive{current.exists?current.storeId:makeLibraryItemId(),current.generation+1,users};
    auto encoded=encodeLibraryArchive(archive);
    if (!encoded.succeeded()) { auto result=fail(LibraryStorageStatus::invalidData,"Invalid user data; no write."); result.explanation=encoded.explanation; return result; }
    if (!allow(hook_,LibraryWriteStage::beforeWrite)) return fail(LibraryStorageStatus::writeFailed,"Write failed before touching the existing library.");
    if (storage_.getParentDirectory().createDirectory().failed())
        return fail(LibraryStorageStatus::writeFailed,"Cannot create library directory.");
    // Temporary and target share a directory/volume. Never delete the original.
    juce::TemporaryFile temp(storage_);
    if (!writeNative(temp.getFile(),encoded.bytes))
        return fail(LibraryStorageStatus::writeFailed,"Cannot completely write/flush the temporary library.");
    // Verify the actual temporary bytes before replacing the previous file.
    juce::MemoryBlock onDisk;
    if (!temp.getFile().loadFileAsData(onDisk) || onDisk.getSize()!=encoded.bytes.size()
        || !std::equal(encoded.bytes.begin(),encoded.bytes.end(),static_cast<const std::uint8_t*>(onDisk.getData())))
        return fail(LibraryStorageStatus::writeFailed,"Temporary library verification failed.");
    if (!allow(hook_,LibraryWriteStage::beforeReplace) || !replaceNative(temp.getFile(),storage_))
        return fail(LibraryStorageStatus::writeFailed,"Cannot replace library; previous file remains.");
    LibraryStorageResult out; out.users=users;
    out.stamp={true,archive.storeId,archive.generation,0};
    for (int n=0;n<4;++n) out.stamp.checksum|=std::uint32_t(encoded.bytes[16+n])<<(8*n);
    return out;
}
LibraryStorageResult SharedUserLibrary::commit(const LibraryUserSnapshot& users,const LibraryStorageStamp& expected) const
{
    const juce::ScopedLock thread(threadLock_); ProcessGuard process(processLock_);
    if (!process.entered) return fail(LibraryStorageStatus::busy,"Library is busy in another process.");
    const auto current=readUnlocked();
    if (!current.succeeded()) return current; // Never replace malformed or future-schema data.
    if (current.stamp!=expected) return fail(LibraryStorageStatus::conflict,"Another writer changed the library; reload before saving.");
    return writeUnlocked(users,current.stamp);
}
LibraryStorageResult SharedUserLibrary::loadInto(PhraseLibrary& library) const
{
    auto result=read();
    if (!result.succeeded()) return result;
    if (!result.stamp.exists && !library.userSnapshot().reservedIds.empty())
        return fail(LibraryStorageStatus::ioError,"Library disappeared; keep current in-memory content.");
    auto restored=library.restoreUserSnapshot(result.users);
    if (!restored.succeeded()) { result.status=LibraryStorageStatus::invalidData; result.explanation=restored.explanation; }
    return result; // Failed restore leaves the previous in-memory data intact.
}
LibraryStorageResult SharedUserLibrary::commitFrom(const PhraseLibrary& library,const LibraryStorageStamp& expected) const
{ return commit(library.userSnapshot(),expected); }
LibraryStorageResult SharedUserLibrary::exportData() const
{
    const juce::ScopedLock thread(threadLock_); ProcessGuard process(processLock_);
    if (!process.entered) return fail(LibraryStorageStatus::busy,"Library is busy in another process.");
    auto result=readUnlocked(); if (!result.succeeded()) return result;
    LibraryArchive archive{result.stamp.exists?result.stamp.storeId:makeLibraryItemId(),
        result.stamp.exists?result.stamp.generation:1,result.users};
    auto encoded=encodeLibraryArchive(archive);
    if (!encoded.succeeded()) return fail(LibraryStorageStatus::invalidData,"Library cannot be exported.");
    result.exportedBytes=std::move(encoded.bytes); return result;
}
LibraryStorageResult SharedUserLibrary::importData(std::span<const std::uint8_t> bytes) const
{
    auto incoming=decodeLibraryArchive(bytes);
    if (!incoming.succeeded()) return fail(incoming.status==LibraryArchiveStatus::futureSchema ? LibraryStorageStatus::futureSchema
        : LibraryStorageStatus::invalidData,"Imported archive is unsupported or invalid; no write.");
    const juce::ScopedLock thread(threadLock_); ProcessGuard process(processLock_);
    if (!process.entered) return fail(LibraryStorageStatus::busy,"Library is busy in another process.");
    auto current=readUnlocked(); if (!current.succeeded()) return current;
    auto merged=current.users;
    const auto merge=mergeLibraryUserSnapshot(merged,incoming.archive->users);
    if (!merge.succeeded()) return fail(merge.status==LibraryStatus::revisionConflict ? LibraryStorageStatus::conflict
        : LibraryStorageStatus::invalidData,"Imported IDs/revisions conflict or data is invalid; no write.");
    return writeUnlocked(merged,current.stamp);
}
}
