#pragma once

#include "core/model/TensionProfile.h"

namespace smartimproviser::harmony
{
// 0.4b contract adapter. Takes the Core result as given; does not reinterpret
// harmony or classify unclassified source material. Later level policies will
// submit supported/explicitly unavailable bands with note roles and reasons.
TensionProfile buildTensionProfile(const ImprovisationResult& result);
}
