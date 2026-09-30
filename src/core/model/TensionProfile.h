#pragma once

#include "core/model/ImprovisationContracts.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace smartimproviser::harmony
{
// A pitch's job in an applied strategy, independent of instrument, voicing,
// pitch class membership and the requested T1/T2/T3 level. The role is only
// assigned by a later policy; no note receives one just from its scale name.
enum class TensionRole : std::uint8_t
{
    unassigned = 0,
    chordAnchor,
    guideTone,
    contextualColor,
    passingApproach,
    resolutionTarget,
    outsideTone
};

enum class TensionAssessment : std::uint8_t
{
    notEvaluated = 0,
    supported,
    unavailable
};

enum class TensionNoteScope : std::uint8_t
{
    undefined = 0,
    currentChord,
    nextChord
};

struct TensionRoleAssignment
{
    MaterialNote note;
    TensionRole role = TensionRole::unassigned;
    TensionNoteScope scope = TensionNoteScope::undefined;
};

struct TensionCandidate
{
    ImprovisationStrategy strategy;
    std::vector<TensionRoleAssignment> noteRoles;
};

struct TensionBand
{
    TensionLevel level = TensionLevel::stable;
    TensionAssessment assessment = TensionAssessment::notEvaluated;
    // Alternatives retain every interpretation. Ranking/selection is a later
    // policy step, never a reason to replace the HarmonicSituation's primary.
    std::vector<TensionCandidate> alternatives;
    std::string unavailableReason;

    bool hasRecommendation() const noexcept
    {
        return assessment == TensionAssessment::supported && !alternatives.empty();
    }
};

// One ephemeral message-thread snapshot for one analyzed HarmonicSituation.
// It is not a phrase's descriptive tension profile or a saved song curve.
struct TensionProfile
{
    bool valid = false;
    HarmonicSituation context;
    std::array<TensionBand, 3> bands {{
        {TensionLevel::stable},
        {TensionLevel::color},
        {TensionLevel::outsideMaximum}
    }};

    const TensionBand* band(TensionLevel level) const noexcept
    {
        const auto value = static_cast<unsigned>(level);
        return value >= 1 && value <= bands.size() ? &bands[value - 1] : nullptr;
    }
};
}
