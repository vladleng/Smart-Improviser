#include "context/ARAContextProvider.h"
#include "context/SharedHarmonicContext.h"
#include "context/TimelineContextMapper.h"

namespace smartimproviser::harmony
{
HarmonicContext ARAContextProvider::currentContext() noexcept
{
    const auto shared = SharedHarmonicContextBridge::instance().read();
    const auto ppq = shared.transportAvailable ? shared.transportPpq : -1.0;
    return mapHarmonicContext(shared, ppq);
}

HarmonicContext ARAContextProvider::contextAt(double ppq) noexcept
{
    return mapHarmonicContext(SharedHarmonicContextBridge::instance().read(), ppq);
}

TimelineHarmonicSnapshot ARAContextProvider::currentTimelineSnapshot() noexcept
{
    const auto shared = SharedHarmonicContextBridge::instance().read();
    const auto ppq = shared.transportAvailable ? shared.transportPpq : -1.0;
    return mapTimelineHarmonicSnapshot(shared, ppq);
}

TimelineHarmonicSnapshot ARAContextProvider::timelineSnapshotAt(double ppq) noexcept
{
    return mapTimelineHarmonicSnapshot(SharedHarmonicContextBridge::instance().read(), ppq);
}

double ARAContextProvider::nextChordStartAfter(double ppq) noexcept
{
    return ::smartimproviser::harmony::nextChordStartAfter(
        SharedHarmonicContextBridge::instance().read(), ppq);
}

double ARAContextProvider::secondsAtPpq(double ppq) noexcept
{
    return ::smartimproviser::harmony::secondsAtPpq(
        SharedHarmonicContextBridge::instance().read(), ppq);
}

double ARAContextProvider::ppqAtSeconds(double seconds) noexcept
{
    return ::smartimproviser::harmony::ppqAtSeconds(
        SharedHarmonicContextBridge::instance().read(), seconds);
}
}
