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

enum class InterpretationStatus : std::uint8_t
{
    unknown = 0,
    unique,
    ambiguous
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
    inferredLocalCenter = 1u << 9,
    candidateLocalCenter = 1u << 10,
    tonicization = 1u << 11,
    localCadence = 1u << 12,
    modulationEvidence = 1u << 13
};

constexpr std::uint32_t evidenceMask(EvidenceFlag flag) noexcept
{
    return static_cast<std::uint32_t>(flag);
}

struct AnalysisEvidence
{
    ConfidenceLevel confidence = ConfidenceLevel::unknown;
    InterpretationStatus interpretation = InterpretationStatus::unknown;
    std::uint8_t alternativeCount = 0;
    std::uint32_t flags = 0;

    constexpr bool has(EvidenceFlag flag) const noexcept
    {
        return (flags & evidenceMask(flag)) != 0;
    }

    constexpr void add(EvidenceFlag flag) noexcept
    {
        flags |= evidenceMask(flag);
    }

    constexpr void markUnique() noexcept
    {
        interpretation = InterpretationStatus::unique;
        alternativeCount = 0;
    }

    constexpr void markAmbiguous(std::uint8_t alternatives) noexcept
    {
        interpretation = InterpretationStatus::ambiguous;
        alternativeCount = alternatives;
    }
};
}
