#pragma once

#include <cstdint>

namespace smartimproviser::harmony
{
enum class ConfidenceLevel : std::uint8_t
{
    unknown = 0,
    low,
    medium,
    high,
    confirmed
};

enum class EvidenceFlag : std::uint32_t
{
    none = 0,
    explicitChord = 1u << 0,
    explicitKey = 1u << 1,
    previousChord = 1u << 2,
    nextChord = 1u << 3,
    diatonicMatch = 1u << 4,
    chromaticRelation = 1u << 5,
    confirmedResolution = 1u << 6,
    patternMatch = 1u << 7,
    modalInterchange = 1u << 8,
    inferredLocalCenter = 1u << 9
};

constexpr std::uint32_t evidenceMask(EvidenceFlag flag) noexcept
{
    return static_cast<std::uint32_t>(flag);
}

struct AnalysisEvidence
{
    ConfidenceLevel confidence = ConfidenceLevel::unknown;
    std::uint32_t flags = 0;

    constexpr bool has(EvidenceFlag flag) const noexcept
    {
        return (flags & evidenceMask(flag)) != 0;
    }

    constexpr void add(EvidenceFlag flag) noexcept
    {
        flags |= evidenceMask(flag);
    }
};
}
