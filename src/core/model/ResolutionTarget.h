#pragma once

#include "core/model/AnalysisEvidence.h"
#include "core/model/ChordModel.h"

#include <array>
#include <cstdint>

namespace smartimproviser::harmony
{
enum class ResolutionImportance : std::uint8_t
{
    optional = 0,
    preferred,
    structural
};

struct ResolutionMove
{
    int fromPitchClass = -1;
    int toPitchClass = -1;
    int semitoneDelta = 0;
    ResolutionImportance importance = ResolutionImportance::optional;
};

inline constexpr std::size_t kMaxResolutionMoves = 8;

struct ResolutionTarget
{
    bool available = false;
    bool confirmed = false;
    NormalizedChord targetChord;
    int targetPitchClass = -1;
    ChordQuality targetQuality = ChordQuality::undefined;
    std::array<ResolutionMove, kMaxResolutionMoves> moves {};
    std::size_t moveCount = 0;
    AnalysisEvidence evidence;

    bool addMove(const ResolutionMove& move) noexcept
    {
        if (moveCount >= moves.size())
            return false;

        moves[moveCount++] = move;
        return true;
    }
};
}
