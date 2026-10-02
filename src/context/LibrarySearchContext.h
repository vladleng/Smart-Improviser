#pragma once
#include "context/TimelineContextMapper.h"
#include "core/analysis/LibrarySearch.h"
namespace smartimproviser::harmony {
LibrarySearchQuery makeLibrarySearchQuery(const SharedHarmonicContextSnapshot&,int tensionFilter);
}
