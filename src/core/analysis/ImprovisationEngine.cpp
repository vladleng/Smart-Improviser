#include "core/analysis/ImprovisationEngine.h"
#include <utility>

namespace smartimproviser::harmony
{
ImprovisationResult analyzeImprovisation(const HarmonicSituation& situation)
{
    ImprovisationResult result;
    result.context = situation;
    if (! situation.valid || ! situation.currentChord.valid)
    {
        result.unavailableReason = "Waiting for valid position, chord and key context.";
        return result;
    }

    const HarmonicAnalysis* harmonic = nullptr;
    const auto index = situation.primaryInterpretationIndex;
    if (index >= 0 && index < situation.interpretationCount
        && static_cast<std::size_t>(index) < situation.interpretations.size()
        && situation.interpretations[static_cast<std::size_t>(index)].valid)
        harmonic = &situation.interpretations[static_cast<std::size_t>(index)].harmonic;

    // No fallback to global interpretation when Stage 2 left primary unresolved.
    // Secondary is relative to the global key even when a local V is primary.
    result.secondaryDominant = situation.harmonic.appliedDominantConfirmed;
    result.substituteDominant = harmonic != nullptr && harmonic->substituteDominantConfirmed;
    const auto& chord = situation.currentChord;
    const bool dominant = chord.quality == ChordQuality::dominant
        || (harmonic != nullptr
            && (harmonic->effectiveFunction == HarmonicFunction::dominant
                || harmonic->effectiveFunction == HarmonicFunction::substituteDominant));
    result.contextDescription = normalizedChordSymbol(chord);
    if (dominant)
    {
        result.dominantContext = DominantContext::unresolved;
        if (situation.resolution.available && situation.resolution.confirmed
            && situation.resolution.targetChord.valid)
        {
            switch (situation.resolution.targetQuality)
            {
                case ChordQuality::major: result.dominantContext = DominantContext::toMajor; break;
                case ChordQuality::minor: result.dominantContext = DominantContext::toMinor; break;
                case ChordQuality::dominant: result.dominantContext = DominantContext::toDominant; break;
                default: result.dominantContext = DominantContext::toOther; break;
            }
            result.contextDescription += " -> " + normalizedChordSymbol(situation.resolution.targetChord);
        }
        switch (result.dominantContext)
        {
            case DominantContext::toMajor: result.contextDescription += " | dominant to major"; break;
            case DominantContext::toMinor: result.contextDescription += " | dominant to minor"; break;
            case DominantContext::toDominant: result.contextDescription += " | dominant chain"; break;
            case DominantContext::toOther: result.contextDescription += " | other target"; break;
            default: result.contextDescription += " | resolution unconfirmed"; break;
        }
        if (result.secondaryDominant) result.contextDescription += " | secondary";
        if (result.substituteDominant) result.contextDescription += " | SubV";
    }
    if (harmonic == nullptr)
        result.contextDescription += " | interpretation unresolved";

    ImprovisationStrategy strategy;
    strategy.kind = ImprovisationStrategyKind::chordToneBased;
    strategy.ruleId = "core.explicit-chord-tones";
    strategy.priority = 100;
    strategy.interpretationIndependent = true;
    strategy.interpretationIndex = harmonic != nullptr ? index : -1;
    strategy.evidence = situation.evidence;
    strategy.actualChord = chord;
    strategy.thinkingStructure = chord;
    strategy.source.kind = MaterialKind::chordTones;
    strategy.source.rootPitchClass = chord.rootPitchClass;
    strategy.source.name = normalizedChordSymbol(chord);
    for (int interval = 0; interval < kPitchClassCount; ++interval)
    {
        if (! chord.hasTone(interval)) continue;
        strategy.source.notes.push_back({ (chord.rootPitchClass + interval) % kPitchClassCount,
                                          interval, chord.degrees[static_cast<std::size_t>(interval)],
                                          MaterialNoteRole::chordTone });
    }
    if (strategy.source.notes.empty())
    {
        result.unavailableReason = "No explicit chord tones available.";
        return result;
    }
    strategy.resolution = situation.resolution;
    strategy.idea = "Build a line from the current chord tones.";
    strategy.explanation = "These tones come from the actual chord; no scale or substitution is inferred.";
    strategy.conditions = "Foundation only. Scale sources and tension selection follow in later checkpoints.";
    result.strategies.push_back(std::move(strategy));
    result.valid = true;
    return result;
}
}
