#pragma once

#include "core/model/AnalysisEvidence.h"
#include "core/model/ImprovisationContracts.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace smartimproviser::harmony
{
enum class ExplanationEvidenceKind : std::uint8_t
{
    interpretation = 0,
    pattern,
    nestedPattern,
    resolution,
    incompleteCadence,
    impliedDominant,
    continuationConflict
};

enum class ExplanationEvidenceState : std::uint8_t
{
    neutral = 0,
    confirmed,
    missing,
    implied,
    contradicted,
    ambiguous
};

enum class ExplanationContextScope : std::uint8_t
{
    global = 0,
    local,
    modal
};

struct ExplanationEvidenceItem
{
    ExplanationEvidenceKind kind = ExplanationEvidenceKind::interpretation;
    ExplanationEvidenceState state = ExplanationEvidenceState::neutral;
    std::string ruleId;
    int interpretationIndex = -1;
    HarmonicPattern pattern;
    NormalizedChord chord;
    int pitchClass = -1;
    AnalysisEvidence evidence;
};

struct ExplanationContextLayer
{
    ExplanationContextScope scope = ExplanationContextScope::global;
    int interpretationIndex = -1;
    KeyCenter center;
    HarmonicAnalysis harmonic;
    AnalysisEvidence evidence;
};

struct ExplanationItem
{
    std::vector<std::size_t> strategyIndices;
    std::vector<int> interpretationIndices;
    bool interpretationIndependent = false;

    std::string idea;
    std::string conditions;
    std::string usageHint;
    SourceMaterial source;
    NormalizedChord actualChord;
    NormalizedChord thinkingStructure;
    std::vector<MaterialNote> importantNotes;

    NormalizedChord targetChord;
    std::vector<MaterialNote> targetNotes;
    ResolutionTarget resolution;

    std::vector<ExplanationEvidenceItem> why;
};

struct ExplanationResult
{
    bool valid = false;
    std::vector<ExplanationContextLayer> contextLayers;
    std::vector<ExplanationItem> items;
};

// Presentation of the two-member V/V -> rootless V reading. The pattern and
// implied dominant have already been inferred by HarmonicEngine; this only
// exposes their provenance and the actual continuation to a host UI.
struct ExplanationImpliedDominantLink
{
    bool valid = false;
    int positionIndex = -1;
    NormalizedChord firstChord;
    NormalizedChord secondChord;
    NormalizedChord actualContinuation;
    bool continuationIsMinorOnImpliedRoot = false;
};

inline ExplanationImpliedDominantLink explainImpliedDominantLink(
    const HarmonicSituation& situation) noexcept
{
    ExplanationImpliedDominantLink link;
    if (! situation.valid
        || situation.pattern.type != HarmonicPatternType::dominantChain
        || situation.pattern.length != 2
        || situation.pattern.positionIndex < 0
        || situation.pattern.positionIndex > 1)
        return link;

    link.positionIndex = situation.pattern.positionIndex;
    if (link.positionIndex == 0 && situation.nextChordAvailable)
    {
        link.firstChord = situation.currentChord;
        link.secondChord = situation.nextChord;
    }
    else if (link.positionIndex == 1 && situation.previousChordAvailable
             && situation.impliedDominant.valid)
    {
        link.firstChord = situation.previousChord;
        link.secondChord = situation.currentChord;
        if (situation.nextChordAvailable)
        {
            link.actualContinuation = situation.nextChord;
            link.continuationIsMinorOnImpliedRoot =
                situation.nextChord.rootPitchClass
                    == situation.impliedDominant.rootPitchClass
                && situation.nextChord.quality == ChordQuality::minor;
        }
    }
    link.valid = link.firstChord.valid && link.secondChord.valid;
    return link;
}

namespace explanation_detail
{
inline bool sameChordIdentity(const NormalizedChord& a, const NormalizedChord& b) noexcept
{
    if (a.valid != b.valid)
        return false;
    if (! a.valid)
        return true;
    return a.rootPitchClass == b.rootPitchClass
        && a.rootFifths == b.rootFifths
        && a.bassPitchClass == b.bassPitchClass
        && a.bassFifths == b.bassFifths
        && a.slashBass == b.slashBass
        && a.quality == b.quality
        && a.extensions == b.extensions
        && a.alterations == b.alterations;
}

inline bool sameNotes(const std::vector<MaterialNote>& a,
                      const std::vector<MaterialNote>& b) noexcept
{
    if (a.size() != b.size())
        return false;
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if (a[i].pitchClass != b[i].pitchClass
            || a[i].semitonesFromRoot != b[i].semitonesFromRoot
            || a[i].degree != b[i].degree
            || a[i].role != b[i].role
            || a[i].characteristic != b[i].characteristic
            || a[i].spelling != b[i].spelling)
            return false;
    }
    return true;
}

inline bool sameSource(const SourceMaterial& a, const SourceMaterial& b) noexcept
{
    return a.kind == b.kind
        && a.mode == b.mode
        && a.rootFifths == b.rootFifths
        && a.rootPitchClass == b.rootPitchClass
        && a.name == b.name
        && sameNotes(a.notes, b.notes)
        && sameNotes(a.chordRelativeNotes, b.chordRelativeNotes);
}

inline bool visuallySameMaterial(const ExplanationItem& item,
                                 const ImprovisationStrategy& strategy) noexcept
{
    return item.idea == strategy.idea
        && item.conditions == strategy.conditions
        && item.usageHint == strategy.usageHint
        && sameSource(item.source, strategy.source)
        && sameChordIdentity(item.actualChord, strategy.actualChord)
        && sameChordIdentity(item.thinkingStructure, strategy.thinkingStructure)
        && sameChordIdentity(item.targetChord, strategy.nextChord)
        && sameNotes(item.targetNotes, strategy.targetNotes)
        && item.resolution.confirmed == strategy.resolution.confirmed
        && sameChordIdentity(item.resolution.targetChord, strategy.resolution.targetChord);
}

inline void addUniqueInterpretation(std::vector<int>& indices, int index)
{
    if (index < 0)
        return;
    if (std::find(indices.begin(), indices.end(), index) == indices.end())
        indices.push_back(index);
}

inline std::vector<MaterialNote> importantNotes(const ImprovisationStrategy& strategy)
{
    std::vector<MaterialNote> notes;
    const auto append = [&notes](const std::vector<MaterialNote>& source)
    {
        for (const auto& note : source)
        {
            const auto duplicate = std::any_of(notes.begin(), notes.end(), [&](const auto& present)
            {
                return present.pitchClass == note.pitchClass
                    && present.spelling == note.spelling;
            });
            if (! duplicate)
                notes.push_back(note);
        }
    };

    append(strategy.guideNotes);
    append(strategy.characteristicNotes);
    if (notes.empty())
        append(strategy.source.notes);
    return notes;
}

inline ExplanationEvidenceState patternState(PatternContextStatus status) noexcept
{
    switch (status)
    {
        case PatternContextStatus::confirmed:
        case PatternContextStatus::completed:
            return ExplanationEvidenceState::confirmed;
        case PatternContextStatus::candidate:
            return ExplanationEvidenceState::implied;
        case PatternContextStatus::none:
        default:
            return ExplanationEvidenceState::neutral;
    }
}

inline bool sameCenter(const KeyCenter& a, const KeyCenter& b) noexcept
{
    if (a.valid != b.valid)
        return false;
    if (! a.valid)
        return true;
    return a.key.rootPitchClass == b.key.rootPitchClass
        && a.key.rootFifths == b.key.rootFifths
        && a.key.mode == b.key.mode
        && a.scope == b.scope;
}

inline void appendContextLayer(ExplanationResult& explanation,
                               ExplanationContextScope scope,
                               int interpretationIndex,
                               const KeyCenter& center,
                               const HarmonicAnalysis& harmonic,
                               const AnalysisEvidence& evidence)
{
    if (! center.valid)
        return;

    const auto duplicate = std::any_of(explanation.contextLayers.begin(),
                                       explanation.contextLayers.end(),
        [&](const auto& present)
        {
            return present.scope == scope
                && sameCenter(present.center, center)
                && present.harmonic.rootScaleDegree == harmonic.rootScaleDegree
                && present.harmonic.effectiveFunction == harmonic.effectiveFunction;
        });
    if (duplicate)
        return;

    ExplanationContextLayer layer;
    layer.scope = scope;
    layer.interpretationIndex = interpretationIndex;
    layer.center = center;
    layer.harmonic = harmonic;
    layer.evidence = evidence;
    explanation.contextLayers.push_back(std::move(layer));
}

inline void appendContextLayers(ExplanationResult& explanation,
                                const HarmonicSituation& context)
{
    appendContextLayer(explanation,
                       ExplanationContextScope::global,
                       -1,
                       context.globalKey,
                       context.harmonic,
                       context.harmonic.valid ? context.evidence : AnalysisEvidence{});

    if (context.localKey.valid)
    {
        appendContextLayer(explanation,
                           ExplanationContextScope::local,
                           -1,
                           context.localKey,
                           context.localHarmonic,
                           context.localKey.evidence);
    }

    for (std::uint8_t index = 0; index < context.interpretationCount
         && index < context.interpretations.size(); ++index)
    {
        const auto& interpretation = context.interpretations[index];
        if (! interpretation.valid)
            continue;

        ExplanationContextScope scope = ExplanationContextScope::global;
        switch (interpretation.kind)
        {
            case HarmonicInterpretationKind::localCenter:
                scope = ExplanationContextScope::local;
                break;
            case HarmonicInterpretationKind::modalInterchange:
                scope = ExplanationContextScope::modal;
                break;
            case HarmonicInterpretationKind::globalContext:
            case HarmonicInterpretationKind::undefined:
            default:
                scope = ExplanationContextScope::global;
                break;
        }

        appendContextLayer(explanation,
                           scope,
                           static_cast<int>(index),
                           interpretation.center,
                           interpretation.harmonic,
                           interpretation.evidence);
    }
}

inline void appendContextEvidence(ExplanationItem& item,
                                  const ImprovisationResult& result)
{
    const auto& context = result.context;

    if (context.patternContext.valid && context.patternContext.topLevel.recognized())
    {
        ExplanationEvidenceItem evidence;
        evidence.kind = ExplanationEvidenceKind::pattern;
        evidence.state = patternState(context.patternContext.status);
        evidence.ruleId = "why.pattern-context";
        evidence.pattern = context.patternContext.topLevel;
        evidence.evidence = context.patternContext.topLevel.evidence;
        item.why.push_back(std::move(evidence));

        for (std::uint8_t i = 0; i < context.patternContext.nestedPatternCount
             && i < context.patternContext.nestedPatterns.size(); ++i)
        {
            const auto& nested = context.patternContext.nestedPatterns[i];
            if (! nested.recognized())
                continue;
            ExplanationEvidenceItem nestedEvidence;
            nestedEvidence.kind = ExplanationEvidenceKind::nestedPattern;
            nestedEvidence.state = ExplanationEvidenceState::confirmed;
            nestedEvidence.ruleId = "why.nested-pattern";
            nestedEvidence.pattern = nested;
            nestedEvidence.evidence = nested.evidence;
            item.why.push_back(std::move(nestedEvidence));
        }
    }

    if (item.resolution.available && item.resolution.confirmed
        && item.resolution.targetChord.valid)
    {
        ExplanationEvidenceItem resolution;
        resolution.kind = ExplanationEvidenceKind::resolution;
        resolution.state = ExplanationEvidenceState::confirmed;
        resolution.ruleId = "why.confirmed-resolution";
        resolution.chord = item.resolution.targetChord;
        resolution.evidence = item.resolution.evidence;
        item.why.push_back(std::move(resolution));
    }

    if (context.impliedDominant.valid)
    {
        ExplanationEvidenceItem implied;
        implied.kind = ExplanationEvidenceKind::impliedDominant;
        implied.state = ExplanationEvidenceState::implied;
        implied.ruleId = "why.implied-dominant";
        implied.pitchClass = context.impliedDominant.rootPitchClass;
        implied.evidence = context.impliedDominant.evidence;
        item.why.push_back(std::move(implied));
    }

    if (context.incompleteCadence.valid)
    {
        ExplanationEvidenceItem missing;
        missing.kind = ExplanationEvidenceKind::incompleteCadence;
        missing.state = ExplanationEvidenceState::missing;
        missing.ruleId = "why.missing-tonic";
        missing.pitchClass = circleOfFifthsToPitchClass(context.incompleteCadence.missingTonicRootFifths);
        item.why.push_back(std::move(missing));

        if (context.incompleteCadence.actualContinuation.valid)
        {
            ExplanationEvidenceItem contradicted;
            contradicted.kind = ExplanationEvidenceKind::continuationConflict;
            contradicted.state = ExplanationEvidenceState::contradicted;
            contradicted.ruleId = "why.actual-continuation-conflicts";
            contradicted.chord = context.incompleteCadence.actualContinuation;
            item.why.push_back(std::move(contradicted));
        }
    }

    if (result.context.primaryInterpretationIndex < 0
        && result.context.interpretationCount > 1)
    {
        ExplanationEvidenceItem ambiguous;
        ambiguous.kind = ExplanationEvidenceKind::interpretation;
        ambiguous.state = ExplanationEvidenceState::ambiguous;
        ambiguous.ruleId = "why.interpretation-ambiguous";
        ambiguous.evidence = result.context.evidence;
        item.why.push_back(std::move(ambiguous));
    }
}
}

// Stage 3 / 0.3g host-neutral presentation model. It consumes only the already
// analyzed ImprovisationResult and never re-runs harmony/history analysis.
// Visually identical material is collapsed while interpretation provenance is
// retained in interpretationIndices. Enharmonic spelling is part of visual
// identity, so Db and C# material are not accidentally merged.
inline ExplanationResult buildExplanation(const ImprovisationResult& result)
{
    ExplanationResult explanation;
    if (! result.valid || result.strategies.empty())
        return explanation;

    explanation_detail::appendContextLayers(explanation, result.context);

    for (std::size_t index = 0; index < result.strategies.size(); ++index)
    {
        const auto& strategy = result.strategies[index];
        auto found = std::find_if(explanation.items.begin(), explanation.items.end(),
            [&](const auto& item)
            {
                return explanation_detail::visuallySameMaterial(item, strategy);
            });

        if (found == explanation.items.end())
        {
            ExplanationItem item;
            item.strategyIndices.push_back(index);
            item.interpretationIndependent = strategy.interpretationIndependent;
            explanation_detail::addUniqueInterpretation(item.interpretationIndices,
                                                        strategy.interpretationIndex);
            item.idea = strategy.idea;
            item.conditions = strategy.conditions;
            item.usageHint = strategy.usageHint;
            item.source = strategy.source;
            item.actualChord = strategy.actualChord;
            item.thinkingStructure = strategy.thinkingStructure;
            item.importantNotes = explanation_detail::importantNotes(strategy);
            item.targetChord = strategy.nextChord;
            item.targetNotes = strategy.targetNotes;
            item.resolution = strategy.resolution;
            explanation.items.push_back(std::move(item));
        }
        else
        {
            found->strategyIndices.push_back(index);
            found->interpretationIndependent = found->interpretationIndependent
                && strategy.interpretationIndependent;
            explanation_detail::addUniqueInterpretation(found->interpretationIndices,
                                                        strategy.interpretationIndex);
        }
    }

    for (auto& item : explanation.items)
        explanation_detail::appendContextEvidence(item, result);

    explanation.valid = ! explanation.items.empty();
    return explanation;
}
}
