#pragma once

namespace smartimproviser::harmony
{
// Open strings occupy their own cell. The nut is the boundary before fret 1.
constexpr float fretCellCenter(int firstFret, int fret, float left, float cellWidth) noexcept
{
    return left + (static_cast<float>(fret - firstFret) + 0.5f) * cellWidth;
}

constexpr float nutBoundary(int firstFret, float left, float cellWidth) noexcept
{
    return left + static_cast<float>(1 - firstFret) * cellWidth;
}

constexpr const char* positionMarker(int fret) noexcept
{
    switch (fret)
    {
        case 3: return "III";
        case 5: return "V";
        case 7: return "VII";
        case 9: return "IX";
        case 12: return "XII";
        case 15: return "XV";
        case 17: return "XVII";
        case 19: return "XIX";
        case 21: return "XXI";
        default: return "";
    }
}
}
