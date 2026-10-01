#include "core/analysis/FretboardLayout.h"

#include <cstdlib>
#include <iostream>
#include <string_view>
#include <utility>

using namespace smartimproviser::harmony;

int main()
{
    const auto expect = [](bool condition, const char* message)
    {
        if (!condition) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
    };
    expect(fretCellCenter(0, 0, 60.0f, 40.0f) == 80.0f
           && nutBoundary(0, 60.0f, 40.0f) == 100.0f
           && fretCellCenter(0, 1, 60.0f, 40.0f) == 120.0f,
           "nut lies between open strings and fret 1, never at fret 0");
    expect(fretCellCenter(12, 12, 60.0f, 40.0f) == 80.0f,
           "a shifted visible range still starts with the actual selected fret");
    for (const auto [fret, label] : {
            std::pair{3, "III"}, {5, "V"}, {7, "VII"}, {9, "IX"},
            {12, "XII"}, {15, "XV"}, {17, "XVII"}, {19, "XIX"}, {21, "XXI"}})
        expect(std::string_view(positionMarker(fret)) == label,
               "position dot frets have their requested Roman numeral");
    for (const int fret : {0, 1, 2, 4, 6, 8, 10, 11, 13, 24})
        expect(std::string_view(positionMarker(fret)).empty(),
               "unmarked frets and open strings have no numeric label");
    std::cout << "Fretboard geometry tests passed\n";
}
