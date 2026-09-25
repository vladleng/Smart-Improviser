#include "context/TimelineContextMapper.h"

#include <algorithm>
#include <cmath>
#include <iterator>

namespace smartimproviser::harmony
{
namespace
{
constexpr double kTimelineEpsilon = 1.0e-12;
constexpr double kStoppedCursorTolerancePpq = 1.0e-6;
// Studio Pro may report a stopped transport position a few thousandths of a
// quarter note before an ARA chord event that is drawn on the same visible
// grid boundary. Snap only chord selection while STOPped; PLAY stays sample-
// strict so the musical context never advances early during playback.
constexpr double kStoppedChordBoundarySnapPpq = 1.0e-2;

IntervalMask copyIntervals(const std::uint8_t (&source)[kPitchClassCount]) noexcept
{
    IntervalMask result;
    std::copy(std::begin(source), std::end(source), result.values.begin());
    return result;
}

int pitchClassForFifths(std::int32_t fifths) noexcept
{
    auto pitchClass = static_cast<int>((fifths * 7) % kPitchClassCount);
    if (pitchClass < 0)
        pitchClass += kPitchClassCount;
    return pitchClass;
}

bool namedRootFifths(const char* name,
                     std::int32_t rawRoot,
                     std::int32_t& result) noexcept
{
    if (name == nullptr || name[0] == '\0')
        return false;

    std::int32_t fifths = 0;
    switch (name[0])
    {
        case 'C': case 'c': fifths = 0; break;
        case 'D': case 'd': fifths = 2; break;
        case 'E': case 'e': fifths = 4; break;
        case 'F': case 'f': fifths = -1; break;
        case 'G': case 'g': fifths = 1; break;
        case 'A': case 'a': fifths = 3; break;
        case 'B': case 'b': fifths = 5; break;
        default: return false;
    }

    const auto b1 = static_cast<unsigned char>(name[1]);
    const auto b2 = static_cast<unsigned char>(name[2]);
    const auto b3 = static_cast<unsigned char>(name[3]);
    if (name[1] == 'b')
        fifths -= 7;
    else if (name[1] == '#')
        fifths += 7;
    else if (b1 == 0xE2u && b2 == 0x99u && b3 == 0xADu) // U+266D MUSIC FLAT SIGN
        fifths -= 7;
    else if (b1 == 0xE2u && b2 == 0x99u && b3 == 0xAFu) // U+266F MUSIC SHARP SIGN
        fifths += 7;

    // The host name is spelling metadata only. Never let a malformed label
    // change the sounding pitch received in the structured ARA root field.
    if (pitchClassForFifths(fifths) != pitchClassForFifths(rawRoot))
        return false;

    result = fifths;
    return true;
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

    // ARA's structured root may be enharmonically canonicalized by the host
    // even when the chord-track label preserves the user's spelling. Keep the
    // structured pitch as authority, but recover an equivalent spelling from
    // the label (Db vs C#, etc.) when it describes the same pitch class.
    std::int32_t spelledRoot = chord.root;
    if (namedRootFifths(chord.name, chord.root, spelledRoot))
    {
        const auto bassWasRoot = pitchClassForFifths(chord.bass) == pitchClassForFifths(chord.root);
        result.root = spelledRoot;
        if (bassWasRoot)
            result.bass = spelledRoot;
    }

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

double chordToleranceFor(const SharedHarmonicContextSnapshot& shared) noexcept
{
    return shared.transportPlaying ? kTimelineEpsilon : kStoppedChordBoundarySnapPpq;
}

int findActiveChordIndex(const SharedHarmonicContextSnapshot& shared,
                         double ppq) noexcept
{
    return findActiveEventIndex(shared.sheetChords,
                                shared.sheetChordStoredCount,
                                ppq,
                                chordToleranceFor(shared));
}

PatternTimelineWindow buildPatternWindow(const SharedHarmonicContextSnapshot& shared,
                                         int chordIndex) noexcept
{
    PatternTimelineWindow result;
    if (chordIndex < 0 || ! shared.sheetChordsAvailable || shared.sheetChordStoredCount <= 0)
        return result;

    constexpr int halfWindow = static_cast<int>(kMaxPatternWindowChords / 2);
    int begin = std::max(0, chordIndex - halfWindow);
    int end = std::min(shared.sheetChordStoredCount,
                       begin + static_cast<int>(kMaxPatternWindowChords));

    // Near the end of the timeline, shift the bounded window backwards so the
    // current chord still has as much preceding pattern evidence as possible.
    begin = std::max(0, end - static_cast<int>(kMaxPatternWindowChords));

    result.chordCount = static_cast<std::uint8_t>(end - begin);
    result.currentIndex = chordIndex - begin;

    for (int sourceIndex = begin; sourceIndex < end; ++sourceIndex)
    {
        const auto destination = static_cast<std::size_t>(sourceIndex - begin);
        result.chords[destination] = makeChordContext(shared.sheetChords[sourceIndex], true);
    }

    return result;
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

    const auto chordIndex = findActiveChordIndex(shared, ppq);
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

    const auto chordIndex = findActiveChordIndex(shared, ppq);
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
             && ppq + chordToleranceFor(shared) < shared.sheetChords[0].position)
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

PatternTimelineWindow mapPatternTimelineWindow(
    const SharedHarmonicContextSnapshot& shared,
    double ppq) noexcept
{
    if (ppq < 0.0)
        return {};

    return buildPatternWindow(shared, findActiveChordIndex(shared, ppq));
}

double nextChordStartAfter(const SharedHarmonicContextSnapshot& shared,
                           double ppq) noexcept
{
    if (! shared.sheetChordsAvailable || shared.sheetChordStoredCount <= 0 || ppq < 0.0)
        return -1.0;

    const auto threshold = ppq + chordToleranceFor(shared);
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