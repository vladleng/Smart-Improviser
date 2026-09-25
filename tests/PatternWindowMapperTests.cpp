#include "context/TimelineContextMapper.h"

#include <cstdlib>
#include <iostream>

using namespace smartimproviser::harmony;

namespace
{
void expect(bool condition, const char* message)
{
    if (! condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

void setMinorSeventh(std::uint8_t (&intervals)[12])
{
    intervals[0] = 0xFFu;
    intervals[3] = 0xFFu;
    intervals[7] = 0xFFu;
    intervals[10] = 0xFFu;
}

void setDominantSeventh(std::uint8_t (&intervals)[12])
{
    intervals[0] = 0xFFu;
    intervals[4] = 0xFFu;
    intervals[7] = 0xFFu;
    intervals[10] = 0xFFu;
}

void setMajorSeventh(std::uint8_t (&intervals)[12])
{
    intervals[0] = 0xFFu;
    intervals[4] = 0xFFu;
    intervals[7] = 0xFFu;
    intervals[11] = 0xFFu;
}
}

int main()
{
    SharedHarmonicContextSnapshot shared;
    shared.sheetChordsAvailable = true;
    shared.sheetChordEventCount = 5;
    shared.sheetChordStoredCount = 5;

    // Am7 -> D7 -> Gm7 -> C7 -> Fmaj7
    const std::int32_t roots[] = { 3, 2, 1, 0, -1 };
    for (int i = 0; i < 5; ++i)
    {
        shared.sheetChords[i].position = static_cast<double>(i * 4);
        shared.sheetChords[i].root = roots[i];
        shared.sheetChords[i].bass = roots[i];
    }
    setMinorSeventh(shared.sheetChords[0].intervals);
    setDominantSeventh(shared.sheetChords[1].intervals);
    setMinorSeventh(shared.sheetChords[2].intervals);
    setDominantSeventh(shared.sheetChords[3].intervals);
    setMajorSeventh(shared.sheetChords[4].intervals);

    // Stage 1 snapshot remains unchanged: only previous/current/next.
    const auto stage1 = mapTimelineHarmonicSnapshot(shared, 16.0);
    expect(stage1.previousChordAvailable && stage1.previousChord.startPpq == 12.0,
           "Stage 1 previous remains immediate C7");
    expect(stage1.currentChord.startPpq == 16.0,
           "Stage 1 current remains Fmaj7");
    expect(! stage1.nextChordAvailable,
           "Stage 1 next remains unavailable after final chord");

    const auto directSeekWindow = mapPatternTimelineWindow(shared, 16.0);
    expect(directSeekWindow.chordCount == 5,
           "direct seek reconstructs five-member bounded cadence window");
    expect(directSeekWindow.currentIndex == 4,
           "direct seek points to Fmaj7 inside reconstructed window");
    expect(directSeekWindow.chords[0].startPpq == 0.0
           && directSeekWindow.chords[4].startPpq == 16.0,
           "reconstructed window retains cadence event identities");

    // Seek backwards is deterministic and does not depend on prior calls.
    const auto backwardSeekWindow = mapPatternTimelineWindow(shared, 8.0);
    expect(backwardSeekWindow.chordCount == 5
           && backwardSeekWindow.currentIndex == 2,
           "backward seek reconstructs current Gm7 deterministically");

    // A chord edit is reflected immediately; no runtime PatternContext cache is
    // involved in the mapper.
    shared.sheetChords[1] = {};
    shared.sheetChords[1].position = 4.0;
    shared.sheetChords[1].root = 2;
    shared.sheetChords[1].bass = 2;
    setMinorSeventh(shared.sheetChords[1].intervals); // Dm7 instead of D7

    const auto editedWindow = mapPatternTimelineWindow(shared, 16.0);
    expect(editedWindow.currentIndex == 4,
           "edited timeline keeps deterministic current event");
    const auto editedD = normalizeChord(editedWindow.chords[1]);
    expect(editedD.quality == ChordQuality::minor,
           "pattern window is rebuilt from edited timeline content");

    const auto invalidWindow = mapPatternTimelineWindow(shared, -1.0);
    expect(invalidWindow.chordCount == 0 && invalidWindow.currentIndex < 0,
           "missing position yields empty safe pattern window");

    std::cout << "SmartImproviser PatternWindowMapperTests: OK\n";
    return 0;
}
