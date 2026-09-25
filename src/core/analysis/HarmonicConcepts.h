#pragma once
#include "core/model/ImprovisationContracts.h"

namespace smartimproviser::harmony
{
// Symbolic instructions over existing strategies, never timed Phrase/MIDI events.
void addHarmonicConcepts(ImprovisationResult& result);
std::string harmonicConceptsText(const ImprovisationResult& result);
}
