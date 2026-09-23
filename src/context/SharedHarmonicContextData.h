#pragma once

#include <cstdint>

inline constexpr int kSmartImproviserMaxKeyEvents = 64;
inline constexpr int kSmartImproviserMaxChordEvents = 256;
inline constexpr int kSmartImproviserMaxTempoEvents = 128;
inline constexpr int kSmartImproviserMaxBarEvents = 64;
inline constexpr int kSmartImproviserEventNameBytes = 96;

struct SharedKeySignatureEvent
{
    double position = 0.0;
    std::int32_t root = 0;
    std::uint8_t intervals[12] {};
    char name[kSmartImproviserEventNameBytes] {};
};

struct SharedChordEvent
{
    double position = 0.0;
    std::int32_t root = 0;
    std::int32_t bass = 0;
    std::uint8_t intervals[12] {};
    char name[kSmartImproviserEventNameBytes] {};
};

struct SharedTempoEvent
{
    double timePosition = 0.0;
    double quarterPosition = 0.0;
};

struct SharedBarSignatureEvent
{
    double position = 0.0;
    std::int32_t numerator = 4;
    std::int32_t denominator = 4;
};

// Transport- and host-facing snapshot produced by the ARA layer.
// This file deliberately has no JUCE/ARA dependency so the Stage 1 -> Stage 2
// mapping can be regression-tested as ordinary C++.
struct SharedHarmonicContextSnapshot
{
    // `connected` means the provider is bound to a usable Musical Context.
    // Individual content sources may still be unavailable or empty.
    bool connected = false;
    std::uint64_t revision = 0;

    bool hostContentAccessAvailable = false;
    int musicalContextCount = 0;

    bool keySignaturesAvailable = false;
    int keySignatureEventCount = 0;
    int keySignatureStoredCount = 0;
    SharedKeySignatureEvent keySignatures[kSmartImproviserMaxKeyEvents] {};

    bool sheetChordsAvailable = false;
    int sheetChordEventCount = 0;
    int sheetChordStoredCount = 0;
    SharedChordEvent sheetChords[kSmartImproviserMaxChordEvents] {};

    bool tempoEntriesAvailable = false;
    int tempoEntryEventCount = 0;
    int tempoEntryStoredCount = 0;
    SharedTempoEvent tempoEntries[kSmartImproviserMaxTempoEvents] {};

    bool barSignaturesAvailable = false;
    int barSignatureEventCount = 0;
    int barSignatureStoredCount = 0;
    SharedBarSignatureEvent barSignatures[kSmartImproviserMaxBarEvents] {};

    bool transportAvailable = false;
    std::uint64_t transportRevision = 0;
    double transportSeconds = -1.0;
    double transportPpq = -1.0;
    bool transportPlaying = false;
};
