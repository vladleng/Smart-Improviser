#pragma once

#include <array>
#include <cstdint>

namespace smartimproviser::harmony
{
inline constexpr int kPitchClassCount = 12;

struct IntervalMask
{
    std::array<std::uint8_t, kPitchClassCount> values {};

    bool any() const noexcept
    {
        for (const auto value : values)
            if (value != 0)
                return true;

        return false;
    }
};

// Host-neutral representation of the active chord. ARA/provider-specific pitch
// encoding is normalized by ChordModel before it enters the analysis layer.
struct ChordContext
{
    bool available = false;
    bool defined = false;
    double startPpq = -1.0;
    std::int32_t root = 0;
    std::int32_t bass = 0;
    IntervalMask intervals;
};

struct KeyContext
{
    bool available = false;
    bool defined = false;
    double startPpq = -1.0;
    std::int32_t root = 0;
    IntervalMask intervals;
};

struct TimeSignatureContext
{
    bool available = false;
    double startPpq = -1.0;
    std::int32_t numerator = 4;
    std::int32_t denominator = 4;
};

struct HarmonicContext
{
    bool providerConnected = false;
    bool positionAvailable = false;
    bool playing = false;
    double ppq = -1.0;
    std::uint64_t harmonicRevision = 0;
    std::uint64_t transportRevision = 0;

    ChordContext chord;
    KeyContext key;
    TimeSignatureContext timeSignature;
};

class IHarmonicContextProvider
{
public:
    virtual ~IHarmonicContextProvider() = default;

    virtual HarmonicContext currentContext() noexcept = 0;
    virtual HarmonicContext contextAt(double ppq) noexcept = 0;

    virtual double nextChordStartAfter(double) noexcept { return -1.0; }
    virtual double secondsAtPpq(double) noexcept { return -1.0; }
    virtual double ppqAtSeconds(double) noexcept { return -1.0; }
};
}
