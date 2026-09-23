#include "context/TimelineContextMapper.h"

#include <algorithm>
#include <cmath>

namespace smartimproviser::harmony
{
namespace
{
constexpr double kTimelineEpsilon = 1.0e-12;
constexpr double kStoppedCursorTolerancePpq = 1.0e-6;

IntervalMask copyIntervals(const std::uint8_t (&source)[kPitchClassCount]) noexcept
{
    IntervalMask result;
    std::copy(std::begin(source), std::end(source), result.values.begin());
    return result;
}

template <typename Event>
int findActiveEventIndex(const Event* events, int count, double ppq, double tolerance) noexcept
{
    if (count <= 0 || ppq < 0.0)
        return -1;

    int active = -1;
    for (int index = 0; index < count; ++index)
    {
        if (events[index].position <= ppq + tolerance)
            active = index;
        else
            break;
    }
    return active;
}

bool intervalsUndefined(const std::uint8_t (&intervals)[kPitchClassCount]) noexcept
{
    for (const auto value : intervals)
        if (value != 0)
            return false;
    return true;
}

ChordContext makeChordContext(const SharedChordEvent& chord, bool available) noexcept
{
    ChordContext result;
    result.available = available;
    result.defined = ! intervalsUndefined(chord.intervals);
    result.startPpq = chord.position;
    result.root = chord.root;
    result.bass = chord.bass;
    result.intervals = copyIntervals(chord.intervals);
    return result;
}

KeyContext makeKeyContext(const SharedKeySignatureEvent& key, bool available) noexcept
{
    KeyContext result;
    result.available = available;
    result.defined = ! intervalsUndefined(key.intervals);
    result.startPpq = key.position;
    result.root = key.root;
    result.intervals = copyIntervals(key.intervals);
    return result;
}

double toleranceFor(const SharedHarmonicContextSnapshot& shared) noexcept
{
    return shared.transportPlaying ? kTimelineEpsilon : kStoppedCursorTolerancePpq;
}
}

HarmonicContext mapHarmonicContext(const SharedHarmonicContextSnapshot& shared,
                                   double ppq) noexcept
{
    HarmonicContext result;
    result.providerConnected = shared.connected;
    result.positionAvailable = ppq >= 0.0;
    result.playing = shared.transportPlaying;
    result.ppq = ppq;
    result.harmonicRevision = shared.revision;
    result.transportRevision = shared.transportRevision;

    if (! result.positionAvailable)
        return result;

    const auto tolerance = toleranceFor(shared);

    const auto chordIndex = findActiveEventIndex(shared.sheetChords,
                                                 shared.sheetChordStoredCount,
                                                 ppq,
                                                 tolerance);
    if (chordIndex >= 0)
        result.chord = makeChordContext(shared.sheetChords[chordIndex], shared.sheetChordsAvailable);

    const auto keyIndex = findActiveEventIndex(shared.keySignatures,
                                               shared.keySignatureStoredCount,
                                               ppq,
                                               tolerance);
    if (keyIndex >= 0)
        result.key = makeKeyContext(shared.keySignatures[keyIndex], shared.keySignaturesAvailable);

    const auto barIndex = findActiveEventIndex(shared.barSignatures,
                                               shared.barSignatureStoredCount,
                                               ppq,
                                               tolerance);
    if (barIndex >= 0)
    {
        const auto& bar = shared.barSignatures[barIndex];
        result.timeSignature.available = shared.barSignaturesAvailable;
        result.timeSignature.startPpq = bar.position;
        result.timeSignature.numerator = bar.numerator;
        result.timeSignature.denominator = bar.denominator;
    }

    return result;
}

TimelineHarmonicSnapshot mapTimelineHarmonicSnapshot(
    const SharedHarmonicContextSnapshot& shared,
    double ppq) noexcept
{
    TimelineHarmonicSnapshot result;
    result.positionAvailable = ppq >= 0.0;
    result.ppq = ppq;

    if (! result.positionAvailable)
        return result;

    const auto tolerance = toleranceFor(shared);

    const auto chordIndex = findActiveEventIndex(shared.sheetChords,
                                                 shared.sheetChordStoredCount,
                                                 ppq,
                                                 tolerance);
    if (chordIndex >= 0)
    {
        result.currentChord = makeChordContext(shared.sheetChords[chordIndex], shared.sheetChordsAvailable);

        if (chordIndex > 0)
        {
            result.previousChordAvailable = true;
            result.previousChord = makeChordContext(shared.sheetChords[chordIndex - 1], shared.sheetChordsAvailable);
        }

        if (chordIndex + 1 < shared.sheetChordStoredCount)
        {
            result.nextChordAvailable = true;
            result.nextChord = makeChordContext(shared.sheetChords[chordIndex + 1], shared.sheetChordsAvailable);
        }
    }
    else if (shared.sheetChordsAvailable && shared.sheetChordStoredCount > 0
             && ppq + tolerance < shared.sheetChords[0].position)
    {
        // Before the first chord event there is deliberately no current chord,
        // but exposing the first upcoming event makes the Stage 2 contract
        // useful for pickup measures and count-ins.
        result.nextChordAvailable = true;
        result.nextChord = makeChordContext(shared.sheetChords[0], true);
    }

    const auto keyIndex = findActiveEventIndex(shared.keySignatures,
                                               shared.keySignatureStoredCount,
                                               ppq,
                                               tolerance);
    if (keyIndex >= 0)
        result.globalKey = makeKeyContext(shared.keySignatures[keyIndex], shared.keySignaturesAvailable);

    return result;
}

double nextChordStartAfter(const SharedHarmonicContextSnapshot& shared,
                           double ppq) noexcept
{
    if (! shared.sheetChordsAvailable || shared.sheetChordStoredCount <= 0 || ppq < 0.0)
        return -1.0;

    const auto threshold = ppq + toleranceFor(shared);
    for (int index = 0; index < shared.sheetChordStoredCount; ++index)
        if (shared.sheetChords[index].position > threshold)
            return shared.sheetChords[index].position;

    return -1.0;
}

double secondsAtPpq(const SharedHarmonicContextSnapshot& shared,
                    double ppq) noexcept
{
    const auto count = shared.tempoEntryStoredCount;
    if (! shared.tempoEntriesAvailable || count <= 0 || ppq < 0.0)
        return -1.0;

    if (count == 1)
    {
        const auto& point = shared.tempoEntries[0];
        return std::abs(ppq - point.quarterPosition) <= kStoppedCursorTolerancePpq
            ? point.timePosition : -1.0;
    }

    int right = 1;
    while (right < count && shared.tempoEntries[right].quarterPosition < ppq)
        ++right;
    if (right >= count)
        right = count - 1;

    const auto left = right - 1;
    const auto& a = shared.tempoEntries[left];
    const auto& b = shared.tempoEntries[right];
    const auto deltaQuarter = b.quarterPosition - a.quarterPosition;
    if (std::abs(deltaQuarter) <= kTimelineEpsilon)
        return -1.0;

    const auto alpha = (ppq - a.quarterPosition) / deltaQuarter;
    return a.timePosition + alpha * (b.timePosition - a.timePosition);
}

double ppqAtSeconds(const SharedHarmonicContextSnapshot& shared,
                    double seconds) noexcept
{
    const auto count = shared.tempoEntryStoredCount;
    if (! shared.tempoEntriesAvailable || count <= 0 || seconds < 0.0)
        return -1.0;

    if (count == 1)
    {
        const auto& point = shared.tempoEntries[0];
        return std::abs(seconds - point.timePosition) <= 1.0e-9
            ? point.quarterPosition : -1.0;
    }

    int right = 1;
    while (right < count && shared.tempoEntries[right].timePosition < seconds)
        ++right;
    if (right >= count)
        right = count - 1;

    const auto left = right - 1;
    const auto& a = shared.tempoEntries[left];
    const auto& b = shared.tempoEntries[right];
    const auto deltaTime = b.timePosition - a.timePosition;
    if (std::abs(deltaTime) <= kTimelineEpsilon)
        return -1.0;

    const auto alpha = (seconds - a.timePosition) / deltaTime;
    return a.quarterPosition + alpha * (b.quarterPosition - a.quarterPosition);
}
}
