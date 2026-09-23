#pragma once

#include "core/model/HarmonicPattern.h"
#include "core/model/ImprovisationContracts.h"

#include <cstdint>
#include <string>
#include <vector>

namespace smartimproviser::harmony
{
struct RelativePitch
{
    int chordIndex = -1;
    int degree = 0;
    int chromaticOffset = 0;
};

struct PhraseNote
{
    RelativePitch pitch;
    double beatOffset = 0.0;
    double durationBeats = 0.0;
    bool target = false;
};

struct Phrase
{
    std::string id;
    std::string name;
    HarmonicPatternType harmonicPattern = HarmonicPatternType::undefined;
    TensionLevel tensionLevel = TensionLevel::stable;
    PhraseRole role = PhraseRole::undefined;
    int startDegree = 0;
    int targetDegree = 0;
    std::vector<PhraseNote> notes;
    AnalysisEvidence evidence;
};
}
