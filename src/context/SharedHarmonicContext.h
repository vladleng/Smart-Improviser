#pragma once

#include <JuceHeader.h>
#include "context/SharedHarmonicContextData.h"

#include <algorithm>
#include <cstring>

#if JUCE_WINDOWS
 #include <windows.h>
#endif

// IPC bridge between Smart Improviser ARA/Event-FX and the context consumer.
// Harmonic map and transport use independent revisions so real-time transport
// publication never mutates the harmonic timeline revision.
class SharedHarmonicContextBridge final
{
public:
    static SharedHarmonicContextBridge& instance()
    {
        static SharedHarmonicContextBridge bridge;
        return bridge;
    }

    ~SharedHarmonicContextBridge()
    {
#if JUCE_WINDOWS
        if (writer && block != nullptr)
        {
            SharedHarmonicContextSnapshot empty;
            publish(empty);
            publishTransport(false, -1.0, -1.0, false);
        }

        if (block != nullptr)
            UnmapViewOfFile(block);
        if (mapping != nullptr)
            CloseHandle(mapping);
        if (writeMutex != nullptr)
            CloseHandle(writeMutex);
#endif
    }

    void prepareWriter()
    {
#if JUCE_WINDOWS
        (void) ensureWriter();
#endif
    }

    void publish(const SharedHarmonicContextSnapshot& snapshot)
    {
#if JUCE_WINDOWS
        if (! ensureWriter())
            return;

        if (writeMutex != nullptr)
            WaitForSingleObject(writeMutex, INFINITE);

        InterlockedIncrement64(&block->sequence);
        MemoryBarrier();

        block->magic = magicValue;
        block->abiVersion = abiVersion;
        block->connected = snapshot.connected ? 1u : 0u;
        block->hostContentAccessAvailable = snapshot.hostContentAccessAvailable ? 1u : 0u;
        block->musicalContextCount = snapshot.musicalContextCount;

        block->keySignaturesAvailable = snapshot.keySignaturesAvailable ? 1u : 0u;
        block->keySignatureEventCount = snapshot.keySignatureEventCount;
        block->keySignatureStoredCount = std::clamp(snapshot.keySignatureStoredCount, 0, kSmartImproviserMaxKeyEvents);
        std::memcpy(block->keySignatures, snapshot.keySignatures,
                    static_cast<std::size_t>(block->keySignatureStoredCount) * sizeof(SharedKeySignatureEvent));

        block->sheetChordsAvailable = snapshot.sheetChordsAvailable ? 1u : 0u;
        block->sheetChordEventCount = snapshot.sheetChordEventCount;
        block->sheetChordStoredCount = std::clamp(snapshot.sheetChordStoredCount, 0, kSmartImproviserMaxChordEvents);
        std::memcpy(block->sheetChords, snapshot.sheetChords,
                    static_cast<std::size_t>(block->sheetChordStoredCount) * sizeof(SharedChordEvent));

        block->tempoEntriesAvailable = snapshot.tempoEntriesAvailable ? 1u : 0u;
        block->tempoEntryEventCount = snapshot.tempoEntryEventCount;
        block->tempoEntryStoredCount = std::clamp(snapshot.tempoEntryStoredCount, 0, kSmartImproviserMaxTempoEvents);
        std::memcpy(block->tempoEntries, snapshot.tempoEntries,
                    static_cast<std::size_t>(block->tempoEntryStoredCount) * sizeof(SharedTempoEvent));

        block->barSignaturesAvailable = snapshot.barSignaturesAvailable ? 1u : 0u;
        block->barSignatureEventCount = snapshot.barSignatureEventCount;
        block->barSignatureStoredCount = std::clamp(snapshot.barSignatureStoredCount, 0, kSmartImproviserMaxBarEvents);
        std::memcpy(block->barSignatures, snapshot.barSignatures,
                    static_cast<std::size_t>(block->barSignatureStoredCount) * sizeof(SharedBarSignatureEvent));

        MemoryBarrier();
        InterlockedIncrement64(&block->sequence);

        if (writeMutex != nullptr)
            ReleaseMutex(writeMutex);
#else
        const auto transportAvailable = localSnapshot.transportAvailable;
        const auto transportRevision = localSnapshot.transportRevision;
        const auto transportSeconds = localSnapshot.transportSeconds;
        const auto transportPpq = localSnapshot.transportPpq;
        const auto transportPlaying = localSnapshot.transportPlaying;

        localSnapshot = snapshot;
        localSnapshot.revision = ++localRevision;
        localSnapshot.transportAvailable = transportAvailable;
        localSnapshot.transportRevision = transportRevision;
        localSnapshot.transportSeconds = transportSeconds;
        localSnapshot.transportPpq = transportPpq;
        localSnapshot.transportPlaying = transportPlaying;
#endif
    }

    void publishTransport(bool available, double seconds, double ppq, bool playing) noexcept
    {
#if JUCE_WINDOWS
        if (block == nullptr || ! writer)
            return;

        InterlockedIncrement64(&block->transportSequence);
        MemoryBarrier();
        block->transportAvailable = available ? 1u : 0u;
        block->transportSeconds = seconds;
        block->transportPpq = ppq;
        block->transportPlaying = playing ? 1u : 0u;
        MemoryBarrier();
        InterlockedIncrement64(&block->transportSequence);
#else
        localSnapshot.transportAvailable = available;
        localSnapshot.transportSeconds = seconds;
        localSnapshot.transportPpq = ppq;
        localSnapshot.transportPlaying = playing;
        localSnapshot.transportRevision = ++localTransportRevision;
#endif
    }

    SharedHarmonicContextSnapshot read()
    {
#if JUCE_WINDOWS
        if (! ensureReader())
            return {};

        SharedHarmonicContextSnapshot result;
        bool harmonicReadSucceeded = false;

        for (int attempt = 0; attempt < 8; ++attempt)
        {
            const auto before = static_cast<std::uint64_t>(InterlockedCompareExchange64(&block->sequence, 0, 0));
            if ((before & 1u) != 0u)
                continue;

            MemoryBarrier();
            const auto magic = block->magic;
            const auto version = block->abiVersion;

            result.connected = block->connected != 0;
            result.hostContentAccessAvailable = block->hostContentAccessAvailable != 0;
            result.musicalContextCount = block->musicalContextCount;

            result.keySignaturesAvailable = block->keySignaturesAvailable != 0;
            result.keySignatureEventCount = block->keySignatureEventCount;
            result.keySignatureStoredCount = std::clamp(static_cast<int>(block->keySignatureStoredCount), 0, kSmartImproviserMaxKeyEvents);
            std::memcpy(result.keySignatures, block->keySignatures,
                        static_cast<std::size_t>(result.keySignatureStoredCount) * sizeof(SharedKeySignatureEvent));

            result.sheetChordsAvailable = block->sheetChordsAvailable != 0;
            result.sheetChordEventCount = block->sheetChordEventCount;
            result.sheetChordStoredCount = std::clamp(static_cast<int>(block->sheetChordStoredCount), 0, kSmartImproviserMaxChordEvents);
            std::memcpy(result.sheetChords, block->sheetChords,
                        static_cast<std::size_t>(result.sheetChordStoredCount) * sizeof(SharedChordEvent));

            result.tempoEntriesAvailable = block->tempoEntriesAvailable != 0;
            result.tempoEntryEventCount = block->tempoEntryEventCount;
            result.tempoEntryStoredCount = std::clamp(static_cast<int>(block->tempoEntryStoredCount), 0, kSmartImproviserMaxTempoEvents);
            std::memcpy(result.tempoEntries, block->tempoEntries,
                        static_cast<std::size_t>(result.tempoEntryStoredCount) * sizeof(SharedTempoEvent));

            result.barSignaturesAvailable = block->barSignaturesAvailable != 0;
            result.barSignatureEventCount = block->barSignatureEventCount;
            result.barSignatureStoredCount = std::clamp(static_cast<int>(block->barSignatureStoredCount), 0, kSmartImproviserMaxBarEvents);
            std::memcpy(result.barSignatures, block->barSignatures,
                        static_cast<std::size_t>(result.barSignatureStoredCount) * sizeof(SharedBarSignatureEvent));

            MemoryBarrier();
            const auto after = static_cast<std::uint64_t>(InterlockedCompareExchange64(&block->sequence, 0, 0));
            if (before != after || (after & 1u) != 0u)
                continue;
            if (magic != magicValue || version != abiVersion)
                return {};

            result.revision = after / 2u;
            harmonicReadSucceeded = true;
            break;
        }

        if (! harmonicReadSucceeded)
            return {};

        for (int attempt = 0; attempt < 8; ++attempt)
        {
            const auto before = static_cast<std::uint64_t>(InterlockedCompareExchange64(&block->transportSequence, 0, 0));
            if ((before & 1u) != 0u)
                continue;

            MemoryBarrier();
            const auto available = block->transportAvailable;
            const auto seconds = block->transportSeconds;
            const auto ppq = block->transportPpq;
            const auto playing = block->transportPlaying;
            MemoryBarrier();

            const auto after = static_cast<std::uint64_t>(InterlockedCompareExchange64(&block->transportSequence, 0, 0));
            if (before != after || (after & 1u) != 0u)
                continue;

            result.transportAvailable = available != 0;
            result.transportSeconds = seconds;
            result.transportPpq = ppq;
            result.transportPlaying = playing != 0;
            result.transportRevision = after / 2u;
            break;
        }

        return result;
#else
        return localSnapshot;
#endif
    }

private:
    SharedHarmonicContextBridge() = default;

#if JUCE_WINDOWS
    struct SharedBlock
    {
        volatile LONG64 sequence = 0;
        volatile LONG64 transportSequence = 0;
        std::uint32_t magic = 0;
        std::uint32_t abiVersion = 0;
        std::uint32_t connected = 0;
        std::uint32_t hostContentAccessAvailable = 0;
        std::int32_t musicalContextCount = 0;

        std::uint32_t keySignaturesAvailable = 0;
        std::int32_t keySignatureEventCount = 0;
        std::int32_t keySignatureStoredCount = 0;
        SharedKeySignatureEvent keySignatures[kSmartImproviserMaxKeyEvents] {};

        std::uint32_t sheetChordsAvailable = 0;
        std::int32_t sheetChordEventCount = 0;
        std::int32_t sheetChordStoredCount = 0;
        SharedChordEvent sheetChords[kSmartImproviserMaxChordEvents] {};

        std::uint32_t tempoEntriesAvailable = 0;
        std::int32_t tempoEntryEventCount = 0;
        std::int32_t tempoEntryStoredCount = 0;
        SharedTempoEvent tempoEntries[kSmartImproviserMaxTempoEvents] {};

        std::uint32_t barSignaturesAvailable = 0;
        std::int32_t barSignatureEventCount = 0;
        std::int32_t barSignatureStoredCount = 0;
        SharedBarSignatureEvent barSignatures[kSmartImproviserMaxBarEvents] {};

        std::uint32_t transportAvailable = 0;
        double transportSeconds = -1.0;
        double transportPpq = -1.0;
        std::uint32_t transportPlaying = 0;
    };

    static constexpr std::uint32_t magicValue = 0x53494D52; // "SIMR"
    static constexpr std::uint32_t abiVersion = 1;
    static constexpr const wchar_t* mappingName = L"Local\\MoonRiverStudio_SmartImproviser_HarmonicContext_v1";
    static constexpr const wchar_t* mutexName = L"Local\\MoonRiverStudio_SmartImproviser_HarmonicContext_Write_v1";

    bool ensureWriter()
    {
        if (block != nullptr && writer)
            return true;

        if (block != nullptr && ! writer)
        {
            UnmapViewOfFile(block);
            block = nullptr;
        }
        if (mapping != nullptr)
        {
            CloseHandle(mapping);
            mapping = nullptr;
        }

        mapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                     static_cast<DWORD>(sizeof(SharedBlock)), mappingName);
        if (mapping == nullptr)
            return false;

        block = static_cast<SharedBlock*>(MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
        if (block == nullptr)
        {
            CloseHandle(mapping);
            mapping = nullptr;
            return false;
        }

        if (writeMutex == nullptr)
            writeMutex = CreateMutexW(nullptr, FALSE, mutexName);

        writer = true;
        return true;
    }

    bool ensureReader()
    {
        if (block != nullptr)
            return true;

        mapping = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, mappingName);
        if (mapping == nullptr)
            return false;

        block = static_cast<SharedBlock*>(MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
        if (block == nullptr)
        {
            CloseHandle(mapping);
            mapping = nullptr;
            return false;
        }
        return true;
    }

    HANDLE mapping = nullptr;
    HANDLE writeMutex = nullptr;
    SharedBlock* block = nullptr;
    bool writer = false;
#else
    SharedHarmonicContextSnapshot localSnapshot;
    std::uint64_t localRevision = 0;
    std::uint64_t localTransportRevision = 0;
#endif
};
