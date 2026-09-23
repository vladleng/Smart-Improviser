#pragma once

#include "core/context/HarmonicContext.h"

#include <array>
#include <cstdint>

namespace smartimproviser::harmony
{
enum class KeyMode : std::uint8_t
{
    undefined = 0,
    major,
    minor,
    custom
};

struct NormalizedKey
{
    bool valid = false;
    std::int32_t rootFifths = 0;
    int rootPitchClass = 0;
    KeyMode mode = KeyMode::undefined;
    std::array<bool, kPitchClassCount> tones {};

    bool hasRelativeTone(int semitones) const noexcept;
    bool hasPitchClass(int pitchClass) const noexcept;
};

NormalizedKey normalizeKey(const KeyContext& source) noexcept;
const char* keyModeName(KeyMode mode) noexcept;
int scaleDegreeForPitchClass(const NormalizedKey& key, int absolutePitchClass) noexcept;
}
