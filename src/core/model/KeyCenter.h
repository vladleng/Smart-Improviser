#pragma once

#include "core/model/AnalysisEvidence.h"
#include "core/model/KeyModel.h"

#include <cstdint>

namespace smartimproviser::harmony
{
enum class KeyCenterScope : std::uint8_t
{
    undefined = 0,
    global,
    local,
    temporary,
    modal
};

struct KeyCenter
{
    bool valid = false;
    NormalizedKey key;
    KeyCenterScope scope = KeyCenterScope::undefined;
    AnalysisEvidence evidence;
};

inline KeyCenter makeGlobalKeyCenter(const NormalizedKey& key) noexcept
{
    KeyCenter center;
    center.valid = key.valid;
    center.key = key;
    center.scope = key.valid ? KeyCenterScope::global : KeyCenterScope::undefined;

    if (key.valid)
    {
        center.evidence.confidence = ConfidenceLevel::confirmed;
        center.evidence.add(EvidenceFlag::explicitKey);
    }

    return center;
}
}
