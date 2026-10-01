#include "core/analysis/PhraseMatcher.h"
#include "core/analysis/DominantDestination.h"
#include "core/analysis/TensionTimeline.h"
#include <algorithm>
#include <cmath>

namespace smartimproviser::harmony
{
namespace
{
constexpr double epsilon = 1.0e-9;
int wrap(int n) { return (n % 12 + 12) % 12; }
bool validLevel(TensionLevel level) { const auto n = static_cast<int>(level); return n >= 1 && n <= 3; }
bool contains(const std::vector<MaterialNote>& notes, int pc)
{
    return std::any_of(notes.begin(), notes.end(), [pc](const auto& n) { return n.pitchClass == pc; });
}
bool sameChord(const NormalizedChord& a, const NormalizedChord& b)
{
    return a.valid && b.valid && a.rootPitchClass == b.rootPitchClass && a.quality == b.quality
        && a.tones == b.tones && a.degrees == b.degrees && a.bassPitchClass == b.bassPitchClass;
}
bool patternMatches(const HarmonicSituation& s, HarmonicPatternType type)
{
    if (s.pattern.type == type || s.localPattern.type == type || s.patternContext.topLevel.type == type) return true;
    for (std::size_t i = 0; i < s.patternContext.nestedPatternCount && i < s.patternContext.nestedPatterns.size(); ++i)
        if (s.patternContext.nestedPatterns[i].type == type) return true;
    return false;
}
}
PhraseMatchResult assessPhrase(const Phrase& phrase, const PhraseMatchRequest& request)
{
    PhraseMatchResult out;
    out.harmonic = PhraseCompatibility::compatible;
    auto report = [&](PhraseCompatibility state, PhraseMatchReason reason, int slot, int note, const char* text)
    {
        if (state == PhraseCompatibility::incompatible || out.harmonic == PhraseCompatibility::compatible)
            out.harmonic = state;
        out.diagnostics.push_back({reason, slot, note, text});
    };
    if (phrase.notes.empty() || (!phrase.tensionProfile.defined && phrase.tensionClassified && !validLevel(phrase.tensionLevel))
        || (request.requestedTension && !validLevel(*request.requestedTension)))
    {
        report(PhraseCompatibility::incompatible, PhraseMatchReason::invalidPhrase, -1, -1, "Empty phrase or invalid tension metadata.");
        return out;
    }
    if (request.slots.empty())
    {
        report(PhraseCompatibility::insufficientContext, PhraseMatchReason::missingContext, -1, -1, "No harmonic slots supplied.");
        return out;
    }
    const auto validSlot = [&](int n) { return n >= 0 && n < static_cast<int>(request.slots.size()); };
    std::vector<bool> used(request.slots.size(), false);
    std::vector<const ImprovisationStrategy*> selected(request.slots.size(), nullptr);
    std::vector<bool> covered(phrase.notes.size(), false);
    out.contexts.resize(request.slots.size());
    out.notePitchClasses.resize(phrase.notes.size(), -1);
    double lastStart = -1.0;
    for (std::size_t i = 0; i < phrase.notes.size(); ++i)
    {
        const auto& note = phrase.notes[i];
        const int slot = note.pitch.chordIndex;
        if (!validSlot(slot))
        {
            report(PhraseCompatibility::insufficientContext, PhraseMatchReason::missingContext, slot, static_cast<int>(i), "Note refers to a missing harmonic slot.");
            continue;
        }
        used[static_cast<std::size_t>(slot)] = true;
        if (static_cast<int>(note.harmonicRole) > static_cast<int>(PhraseNoteRole::outsideTone))
            report(PhraseCompatibility::incompatible, PhraseMatchReason::invalidPhrase, slot, static_cast<int>(i), "Unknown note-role metadata.");
        const auto& context = request.slots[static_cast<std::size_t>(slot)];
        if (!std::isfinite(note.beatOffset) || !std::isfinite(note.durationBeats)
            || note.durationBeats <= 0.0 || note.beatOffset < 0.0 || note.beatOffset + epsilon < lastStart)
        {
            report(PhraseCompatibility::incompatible, PhraseMatchReason::invalidTiming, slot, static_cast<int>(i), "Notes need ordered finite times and positive durations.");
            continue;
        }
        lastStart = note.beatOffset;
        if (!std::isfinite(context.startBeat) || !std::isfinite(context.endBeat) || context.endBeat <= context.startBeat
            || note.beatOffset + epsilon < context.startBeat || note.beatOffset >= context.endBeat)
            report(PhraseCompatibility::incompatible, PhraseMatchReason::invalidTiming, slot, static_cast<int>(i), "Note onset is outside its harmonic slot.");
        else if (note.beatOffset + note.durationBeats > context.endBeat + epsilon)
            report(PhraseCompatibility::insufficientContext, PhraseMatchReason::invalidTiming, slot, static_cast<int>(i), "A sustained note across a chord boundary needs explicit split/tie context.");
        if (note.pitch.degree < 1 || note.pitch.degree > 13 || note.pitch.chromaticOffset < -2 || note.pitch.chromaticOffset > 2)
        {
            report(PhraseCompatibility::incompatible, PhraseMatchReason::invalidPitch, slot, static_cast<int>(i), "Relative degree must be 1..13 with a supported accidental (-2..2).");
            continue;
        }
        if (context.material.valid && context.material.context.currentChord.valid)
        {
            constexpr int major[] = {0,2,4,5,7,9,11};
            out.notePitchClasses[i] = wrap(context.material.context.currentChord.rootPitchClass
                + major[(note.pitch.degree - 1) % 7] + note.pitch.chromaticOffset);
        }
    }
    for (const auto& group : phrase.approaches)
        if (validSlot(group.chordIndex)) used[static_cast<std::size_t>(group.chordIndex)] = true;
    std::vector<bool> bound(request.slots.size(), false);
    for (const auto& requirement : phrase.harmonicRequirements)
    {
        const int slot = requirement.chordIndex;
        if (!validSlot(slot))
        {
            report(PhraseCompatibility::insufficientContext, PhraseMatchReason::missingContext, slot, -1, "Source requirement refers to a missing slot.");
            continue;
        }
        used[static_cast<std::size_t>(slot)] = true;
        if (bound[static_cast<std::size_t>(slot)])
        {
            report(PhraseCompatibility::incompatible, PhraseMatchReason::invalidPhrase, slot, -1, "Duplicate source requirements for one slot.");
            continue;
        }
        bound[static_cast<std::size_t>(slot)] = true;
        const auto& result = request.slots[static_cast<std::size_t>(slot)].material;
        if (!result.valid || !result.context.currentChord.valid) continue;
        if (requirement.sourceRootOffset < -1 || requirement.sourceRootOffset > 11 || requirement.sourceRuleVersion < 0)
            report(PhraseCompatibility::incompatible, PhraseMatchReason::invalidPhrase, slot, -1, "Invalid source application metadata.");
        if (requirement.chordQuality != ChordQuality::undefined && requirement.chordQuality != result.context.currentChord.quality)
            report(PhraseCompatibility::incompatible, PhraseMatchReason::chordMismatch, slot, -1, "Written chord quality does not match the phrase requirement.");
        if (requirement.harmonicPattern != HarmonicPatternType::undefined
            && requirement.harmonicPattern != HarmonicPatternType::none
            && !patternMatches(result.context, requirement.harmonicPattern))
            report(result.context.pattern.recognized() || result.context.localPattern.recognized()
                ? PhraseCompatibility::incompatible : PhraseCompatibility::insufficientContext,
                PhraseMatchReason::patternMismatch, slot, -1, "The required slot-local harmonic turn is absent.");
        const auto destination = dominantDestination(result.context);
        if (requirement.destinationQuality != ChordQuality::undefined || requirement.confirmedDestinationRequired)
        {
            if (!destination.valid)
                report(PhraseCompatibility::insufficientContext, PhraseMatchReason::destinationMissing, slot, -1, "No supported dominant destination is available.");
            else if (requirement.destinationQuality != ChordQuality::undefined && requirement.destinationQuality != destination.quality)
                report(PhraseCompatibility::incompatible, PhraseMatchReason::destinationMismatch, slot, -1, "Major/minor destination does not match the phrase requirement.");
            if (destination.valid && requirement.confirmedDestinationRequired && !destination.confirmed)
                report(PhraseCompatibility::insufficientContext, PhraseMatchReason::destinationUnconfirmed, slot, -1, "The requested destination is only hypothetical/unconfirmed.");
        }
        if (requirement.sourceRuleId.empty()) continue; // Chord/destination-only requirement.
        std::vector<const ImprovisationStrategy*> matches;
        for (const auto& strategy : result.strategies)
            if (strategy.ruleId == requirement.sourceRuleId && sameChord(strategy.actualChord, result.context.currentChord)
                && (requirement.sourceRootOffset == -1 || wrap(strategy.source.rootPitchClass - strategy.actualChord.rootPitchClass) == requirement.sourceRootOffset))
                matches.push_back(&strategy);
        if (matches.empty())
            report(PhraseCompatibility::incompatible, PhraseMatchReason::sourceUnavailable, slot, -1, "The required source application is absent from this compatible catalog.");
        else
        {
            const auto* first = matches.front();
            const bool ambiguous = std::any_of(matches.begin(), matches.end(), [first](const auto* other)
            {
                if (other->source.rootPitchClass != first->source.rootPitchClass || other->source.notes.size() != first->source.notes.size()) return true;
                for (std::size_t i = 0; i < first->source.notes.size(); ++i)
                    if (other->source.notes[i].pitchClass != first->source.notes[i].pitchClass) return true;
                return false;
            });
            if (ambiguous)
                report(PhraseCompatibility::insufficientContext, PhraseMatchReason::ambiguousSource, slot, -1, "Several source applications match; specify the relative source root.");
            else if (requirement.sourceRuleVersion != 0 && requirement.sourceRuleVersion != first->ruleVersion)
                report(PhraseCompatibility::insufficientContext, PhraseMatchReason::sourceVersion, slot, -1, "The stored source-rule version needs revalidation.");
            else
            {
                selected[static_cast<std::size_t>(slot)] = first;
                auto& context = out.contexts[static_cast<std::size_t>(slot)];
                for (const auto* candidate : matches)
                {
                    context.sourceEvidenceAlternatives.push_back(candidate->evidence);
                    context.sourceInterpretationIndices.push_back(candidate->interpretationIndex);
                }
            }
        }
    }
    bool patternFound = phrase.harmonicPattern == HarmonicPatternType::undefined || phrase.harmonicPattern == HarmonicPatternType::none;
    bool anyPatternKnown = false;
    double previousEnd = -1.0;
    for (std::size_t slot = 0; slot < request.slots.size(); ++slot)
    {
        if (!used[slot]) continue;
        const auto& result = request.slots[slot].material;
        const auto& s = result.context;
        auto& projection = out.contexts[slot];
        projection.actualChord = s.currentChord;
        if (s.nextChordAvailable) projection.actualNextChord = s.nextChord;
        const auto d = dominantDestination(s);
        projection.destination = {d.valid, d.confirmed, d.rootPitchClass, d.quality, d.evidence};
        projection.resolution = s.resolution;
        if (selected[slot])
        {
            projection.sourceRuleId = selected[slot]->ruleId;
            projection.sourceRuleVersion = selected[slot]->ruleVersion;
            projection.sourceRootPitchClass = selected[slot]->source.rootPitchClass;
            projection.sourceEvidence = selected[slot]->evidence;
        }
        if (!result.valid || !s.currentChord.valid)
            report(PhraseCompatibility::insufficientContext, PhraseMatchReason::missingContext, static_cast<int>(slot), -1, "No analyzed chord/material context for this slot.");
        patternFound = patternFound || patternMatches(s, phrase.harmonicPattern);
        anyPatternKnown = anyPatternKnown || s.pattern.recognized() || s.localPattern.recognized() || s.patternContext.valid;
        const auto& timing = request.slots[slot];
        if (!std::isfinite(timing.startBeat) || !std::isfinite(timing.endBeat)
            || timing.startBeat < 0.0 || timing.endBeat <= timing.startBeat || previousEnd > timing.startBeat + epsilon)
            report(PhraseCompatibility::incompatible, PhraseMatchReason::invalidTiming, static_cast<int>(slot), -1, "Harmonic slots have invalid times, overlap or run backwards.");
        previousEnd = timing.endBeat;
    }
    if (!patternFound)
        report(anyPatternKnown ? PhraseCompatibility::incompatible : PhraseCompatibility::insufficientContext,
            PhraseMatchReason::patternMismatch, -1, -1, "Required local/top-level harmonic turn is not established in these slots.");

    for (const auto& group : phrase.approaches)
    {
        const int slot = group.chordIndex;
        if (!validSlot(slot) || !request.slots[static_cast<std::size_t>(slot)].material.valid)
        {
            report(PhraseCompatibility::insufficientContext, PhraseMatchReason::missingContext, slot, -1, "Approach lacks an analyzed preparation slot.");
            continue;
        }
        const auto& result = request.slots[static_cast<std::size_t>(slot)].material;
        const auto idea = std::find_if(result.concepts.begin(), result.concepts.end(), [&](const auto& c)
        { return c.ruleId == group.conceptRuleId && (c.kind == HarmonicConceptKind::chromaticApproach || c.kind == HarmonicConceptKind::enclosure); });
        if (idea == result.concepts.end() || group.noteIndices.size() != idea->approachShape.size() || group.noteIndices.empty())
        {
            report(PhraseCompatibility::incompatible, PhraseMatchReason::invalidApproach, slot, -1, "Approach must use a complete approved approach/enclosure shape and target.");
            continue;
        }
        bool valid = true;
        for (std::size_t k = 0; k < group.noteIndices.size(); ++k)
        {
            const auto index = group.noteIndices[k];
            if (index >= phrase.notes.size() || covered[index] || (k > 0 && index != group.noteIndices[k-1] + 1)) { valid = false; break; }
            const auto& note = phrase.notes[index];
            if (k > 0 && note.beatOffset + epsilon < phrase.notes[group.noteIndices[k-1]].beatOffset
                + phrase.notes[group.noteIndices[k-1]].durationBeats) valid = false;
            const auto& step = idea->approachShape[k];
            if (out.notePitchClasses[index] != wrap(idea->target.pitchClass + step.semitonesFromTarget)) { valid = false; break; }
            if (!step.isTarget)
            {
                if (note.pitch.chordIndex != slot || note.harmonicRole != PhraseNoteRole::passingApproach || note.target) valid = false;
            }
            else
            {
                if (!(note.target || note.harmonicRole == PhraseNoteRole::resolutionTarget)
                    || note.harmonicRole == PhraseNoteRole::passingApproach || note.harmonicRole == PhraseNoteRole::outsideTone) valid = false;
                if (idea->targetScope == ConceptTargetScope::currentChord)
                    valid = valid && note.pitch.chordIndex == slot;
                else if (idea->targetScope == ConceptTargetScope::nextChord)
                    valid = valid && note.pitch.chordIndex == slot + 1 && validSlot(slot + 1)
                        && sameChord(request.slots[static_cast<std::size_t>(slot+1)].material.context.currentChord, idea->targetChord)
                        && sameChord(result.context.nextChord, idea->targetChord)
                        && std::abs(note.beatOffset - request.slots[static_cast<std::size_t>(slot+1)].startBeat) <= epsilon;
                else valid = false;
            }
        }
        if (!valid)
            report(PhraseCompatibility::incompatible, PhraseMatchReason::invalidApproach, slot, -1, "Approach notes, order, roles or actual landing chord do not match the approved concept.");
        else for (auto index : group.noteIndices) covered[index] = true;
    }
    for (std::size_t i = 0; i < phrase.notes.size(); ++i)
    {
        const auto& note = phrase.notes[i];
        const int slot = note.pitch.chordIndex, pc = out.notePitchClasses[i];
        if (!validSlot(slot) || pc < 0 || covered[i]) continue;
        const auto& result = request.slots[static_cast<std::size_t>(slot)].material;
        const auto& chord = result.context.currentChord;
        const auto anchor = chord.hasTone(wrap(pc - chord.rootPitchClass))
            || (chord.slashBass && pc == chord.bassPitchClass);
        const auto* source = selected[static_cast<std::size_t>(slot)];
        bool allowed = false;
        if (note.harmonicRole == PhraseNoteRole::outsideTone)
        {
            report(PhraseCompatibility::insufficientContext, PhraseMatchReason::unsupportedOutside, slot, static_cast<int>(i), "No outside/side-slip policy is implemented; target/tension labels cannot authorize it.");
            continue;
        }
        if (note.harmonicRole == PhraseNoteRole::passingApproach && note.target)
        {
            report(PhraseCompatibility::incompatible, PhraseMatchReason::invalidApproach, slot, static_cast<int>(i), "A preparation note cannot also be a landing target.");
            continue;
        }
        const auto role = note.target ? PhraseNoteRole::resolutionTarget : note.harmonicRole;
        switch (role)
        {
            case PhraseNoteRole::chordAnchor: case PhraseNoteRole::resolutionTarget: allowed = anchor; break;
            case PhraseNoteRole::guideTone:
                for (const auto& strategy : result.strategies) allowed = allowed || contains(strategy.guideNotes, pc);
                break;
            case PhraseNoteRole::characteristicTone:
                if (source) allowed = contains(source->characteristicNotes, pc);
                break;
            case PhraseNoteRole::sourceTone:
                if (source) allowed = contains(source->source.notes, pc);
                break;
            case PhraseNoteRole::passingApproach:
                report(PhraseCompatibility::insufficientContext, PhraseMatchReason::missingApproach, slot, static_cast<int>(i), "A passing note needs its complete approved approach/enclosure group.");
                continue;
            case PhraseNoteRole::outsideTone:
                report(PhraseCompatibility::insufficientContext, PhraseMatchReason::unsupportedOutside, slot, static_cast<int>(i), "No outside/side-slip policy is implemented; a tension label cannot authorize it.");
                continue;
        }
        if (!source && (role == PhraseNoteRole::sourceTone || role == PhraseNoteRole::characteristicTone))
            report(PhraseCompatibility::insufficientContext, PhraseMatchReason::sourceUnavailable, slot, static_cast<int>(i), "Source-based notes require an explicit compatible source application.");
        else if (!allowed)
            report(PhraseCompatibility::incompatible, PhraseMatchReason::unsupportedNote, slot, static_cast<int>(i), "Note does not fulfill its declared role in the actual chord/source.");
    }
    if (out.harmonic != PhraseCompatibility::compatible) return out; // Do not tension-filter an invalid harmonic candidate.
    out.tension = matchPhraseTension(phrase, request.requestedTension);
    if (out.tension == PhraseTensionMatch::invalidProfile)
        out.diagnostics.push_back({PhraseMatchReason::tensionProfileInvalid, -1, -1, "Descriptive profile has invalid times, bounds or assignments."});
    else if (out.tension == PhraseTensionMatch::unclassified)
        out.diagnostics.push_back({PhraseMatchReason::tensionUnclassified, -1, -1, "Phrase has no assigned tension; it is only eligible in All."});
    else if (out.tension == PhraseTensionMatch::differentLevel)
        out.diagnostics.push_back({PhraseMatchReason::tensionMismatch, -1, -1, "Phrase tension differs from the requested level."});
    else out.diagnostics.push_back({PhraseMatchReason::matched, -1, -1, "Harmonic requirements matched; tension selection permits this candidate."});
    return out;
}
}
