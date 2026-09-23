#include "context/TimelineContextMapper.h"

#include <cmath>
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

void setMajorTriad(std::uint8_t (&intervals)[12])
{
    intervals[0] = 0xFFu;
    intervals[4] = 0xFFu;
    intervals[7] = 0xFFu;
}

void setMinorSeventh(std::uint8_t (&intervals)[12])
{
    intervals[0] = 0xFFu;
    intervals[3] = 0xFFu;
    intervals[7] = 0xFFu;
    intervals[10] = 0xFFu;
}

void setMajorKey(std::uint8_t (&intervals)[12])
{
    for (const auto semitone : { 0, 2, 4, 5, 7, 9, 11 })
        intervals[semitone] = 0xFFu;
}
}

int main()
{
    // A valid ARA/Musical Context may be connected even when individual
    // harmonic content sources are unavailable.
    SharedHarmonicContextSnapshot empty;
    empty.connected = true;
    empty.hostContentAccessAvailable = true;
    empty.musicalContextCount = 1;

    const auto emptyContext = mapHarmonicContext(empty, 4.0);
    expect(emptyContext.providerConnected, "connected provider survives missing content sources");
    expect(emptyContext.positionAvailable, "explicit PPQ remains available");
    expect(! emptyContext.chord.available, "missing chord source is safe");
    expect(! emptyContext.key.available, "missing key source is safe");
    expect(! emptyContext.timeSignature.available, "missing bar source is safe");

    const auto emptyTimeline = mapTimelineHarmonicSnapshot(empty, 4.0);
    expect(emptyTimeline.positionAvailable, "timeline position survives missing sources");
    expect(! emptyTimeline.currentChord.available, "timeline current chord is unavailable");
    expect(! emptyTimeline.previousChordAvailable, "timeline previous chord is unavailable");
    expect(! emptyTimeline.nextChordAvailable, "timeline next chord is unavailable");
    expect(! emptyTimeline.globalKey.available, "timeline global key is unavailable");
    expect(nextChordStartAfter(empty, 4.0) < 0.0, "missing chord source has no next boundary");
    expect(secondsAtPpq(empty, 4.0) < 0.0, "missing tempo source cannot convert PPQ");
    expect(ppqAtSeconds(empty, 2.0) < 0.0, "missing tempo source cannot convert seconds");

    SharedHarmonicContextSnapshot timeline;
    timeline.connected = true;
    timeline.hostContentAccessAvailable = true;
    timeline.musicalContextCount = 1;
    timeline.sheetChordsAvailable = true;
    timeline.sheetChordEventCount = 4;
    timeline.sheetChordStoredCount = 4;

    timeline.sheetChords[0].position = 8.0;
    timeline.sheetChords[0].root = 0; // C
    timeline.sheetChords[0].bass = 0;
    setMajorTriad(timeline.sheetChords[0].intervals);

    timeline.sheetChords[1].position = 12.0;
    timeline.sheetChords[1].root = 3; // A
    timeline.sheetChords[1].bass = 3;
    setMinorSeventh(timeline.sheetChords[1].intervals);

    timeline.sheetChords[2].position = 16.0; // explicit no-chord event

    timeline.sheetChords[3].position = 20.0;
    timeline.sheetChords[3].root = 1; // G
    timeline.sheetChords[3].bass = 1;
    setMajorTriad(timeline.sheetChords[3].intervals);

    timeline.keySignaturesAvailable = true;
    timeline.keySignatureEventCount = 2;
    timeline.keySignatureStoredCount = 2;
    timeline.keySignatures[0].position = 0.0;
    timeline.keySignatures[0].root = 0;
    setMajorKey(timeline.keySignatures[0].intervals);
    timeline.keySignatures[1].position = 24.0; // explicit undefined key
    timeline.keySignatures[1].root = 1;

    const auto beforeFirst = mapTimelineHarmonicSnapshot(timeline, 4.0);
    expect(! beforeFirst.currentChord.available, "before first event has no current chord");
    expect(! beforeFirst.previousChordAvailable, "before first event has no previous chord");
    expect(beforeFirst.nextChordAvailable, "before first event exposes first upcoming chord");
    expect(beforeFirst.nextChord.startPpq == 8.0, "first upcoming chord starts at PPQ 8");
    expect(beforeFirst.globalKey.available && beforeFirst.globalKey.defined,
           "global key is available before first chord");

    const auto exactBoundary = mapTimelineHarmonicSnapshot(timeline, 12.0);
    expect(exactBoundary.currentChord.available && exactBoundary.currentChord.defined,
           "exact boundary activates new chord");
    expect(exactBoundary.currentChord.startPpq == 12.0, "current chord boundary is exact");
    expect(exactBoundary.previousChordAvailable && exactBoundary.previousChord.startPpq == 8.0,
           "previous chord is preserved at boundary");
    expect(exactBoundary.nextChordAvailable && exactBoundary.nextChord.startPpq == 16.0,
           "next chord is preserved at boundary");

    const auto explicitNoChord = mapTimelineHarmonicSnapshot(timeline, 18.0);
    expect(explicitNoChord.currentChord.available, "explicit no-chord event is available content");
    expect(! explicitNoChord.currentChord.defined, "explicit no-chord event is undefined harmonically");
    expect(explicitNoChord.previousChordAvailable && explicitNoChord.previousChord.startPpq == 12.0,
           "no-chord event retains previous context");
    expect(explicitNoChord.nextChordAvailable && explicitNoChord.nextChord.startPpq == 20.0,
           "no-chord event exposes following chord");

    const auto afterLast = mapTimelineHarmonicSnapshot(timeline, 28.0);
    expect(afterLast.currentChord.available && afterLast.currentChord.startPpq == 20.0,
           "last chord remains active after final event");
    expect(afterLast.previousChordAvailable && afterLast.previousChord.startPpq == 16.0,
           "after last event previous chord is preserved");
    expect(! afterLast.nextChordAvailable, "after last event has no next chord");
    expect(afterLast.globalKey.available && ! afterLast.globalKey.defined,
           "explicit undefined key is represented safely");

    expect(nextChordStartAfter(timeline, 12.0) == 16.0, "next chord boundary is exact");
    expect(nextChordStartAfter(timeline, 20.0) < 0.0, "no boundary after final chord");

    SharedHarmonicContextSnapshot tempo;
    tempo.tempoEntriesAvailable = true;
    tempo.tempoEntryEventCount = 2;
    tempo.tempoEntryStoredCount = 2;
    tempo.tempoEntries[0].timePosition = 0.0;
    tempo.tempoEntries[0].quarterPosition = 0.0;
    tempo.tempoEntries[1].timePosition = 2.0;
    tempo.tempoEntries[1].quarterPosition = 4.0; // 120 BPM

    expect(std::abs(secondsAtPpq(tempo, 2.0) - 1.0) < 1.0e-9,
           "PPQ to seconds interpolation works");
    expect(std::abs(ppqAtSeconds(tempo, 1.0) - 2.0) < 1.0e-9,
           "seconds to PPQ interpolation works");

    SharedHarmonicContextSnapshot oneTempo;
    oneTempo.tempoEntriesAvailable = true;
    oneTempo.tempoEntryEventCount = 1;
    oneTempo.tempoEntryStoredCount = 1;
    oneTempo.tempoEntries[0].timePosition = 3.0;
    oneTempo.tempoEntries[0].quarterPosition = 6.0;
    expect(secondsAtPpq(oneTempo, 6.0) == 3.0, "single tempo anchor maps its exact PPQ");
    expect(secondsAtPpq(oneTempo, 7.0) < 0.0, "single tempo anchor does not invent a tempo slope");

    // Even structurally valid chord/key data must not produce a valid musical
    // situation when the timeline coordinate itself is unavailable.
    TimelineHarmonicSnapshot unknownPosition;
    unknownPosition.currentChord.available = true;
    unknownPosition.currentChord.defined = true;
    unknownPosition.currentChord.root = 0;
    unknownPosition.currentChord.bass = 0;
    unknownPosition.currentChord.intervals.values[0] = 0xFFu;
    unknownPosition.currentChord.intervals.values[4] = 0xFFu;
    unknownPosition.currentChord.intervals.values[7] = 0xFFu;
    unknownPosition.globalKey.available = true;
    unknownPosition.globalKey.defined = true;
    unknownPosition.globalKey.root = 0;
    for (const auto semitone : { 0, 2, 4, 5, 7, 9, 11 })
        unknownPosition.globalKey.intervals.values[semitone] = 0xFFu;
    expect(! buildHarmonicSituation(unknownPosition).valid,
           "unknown timeline position cannot produce a valid HarmonicSituation");

    std::cout << "SmartImproviser TimelineContextMapperTests: OK\n";
    return 0;
}
