#include "core/analysis/AmbiguityAnalyzer.h"

namespace smartimproviser::harmony
{
namespace
{
NormalizedKey makeParallelKey(const NormalizedKey& source) noexcept
{
    NormalizedKey key;
    if (! source.valid)
        return key;

    const auto targetMode = source.mode == KeyMode::major
        ? KeyMode::minor
        : (source.mode == KeyMode::minor ? KeyMode::major : KeyMode::undefined);
    if (targetMode == KeyMode::undefined)
        return key;

    static constexpr int majorIntervals[] = { 0, 2, 4, 5, 7, 9, 11 };
    static constexpr int minorIntervals[] = { 0, 2, 3, 5, 7, 8, 10 };

    key.valid = true;
    key.rootPitchClass = source.rootPitchClass;
    key.rootFifths = source.rootFifths;
    key.mode = targetMode;

    const auto* intervals = targetMode == KeyMode::major ? majorIntervals : minorIntervals;
    for (int i = 0; i < 7; ++i)
        key.tones[static_cast<std::size_t>(intervals[i])] = true;

    return key;
}

bool sameTonalCenter(const HarmonicInterpretation& a,
                     const HarmonicInterpretation& b) noexcept
{
    return a.valid && b.valid
        && a.center.valid && b.center.valid
        && a.center.key.valid && b.center.key.valid
        && a.center.key.rootPitchClass == b.center.key.rootPitchClass
        && a.center.key.mode == b.center.key.mode;
}

void addInterpretation(HarmonicSituation& situation,
                       HarmonicInterpretation interpretation) noexcept
{
    if (! interpretation.valid
        || situation.interpretationCount >= kMaxHarmonicInterpretations)
        return;

    situation.interpretations[situation.interpretationCount++] = interpretation;
}

HarmonicInterpretation makeGlobalInterpretation(const HarmonicSituation& situation) noexcept
{
    HarmonicInterpretation interpretation;
    if (! situation.valid || ! situation.globalKey.valid || ! situation.harmonic.valid)
        return interpretation;

    interpretation.valid = true;
    interpretation.kind = HarmonicInterpretationKind::globalContext;
    interpretation.center = situation.globalKey;
    interpretation.harmonic = situation.harmonic;
    interpretation.pattern = situation.pattern;
    interpretation.evidence.confidence = situation.evidence.confidence;
    interpretation.evidence.markUnique();
    interpretation.evidence.add(EvidenceFlag::explicitChord);
    interpretation.evidence.add(EvidenceFlag::explicitKey);

    if (situation.harmonic.relation == HarmonicRelation::diatonic)
        interpretation.evidence.add(EvidenceFlag::diatonicMatch);
    else if (situation.harmonic.relation == HarmonicRelation::chromatic)
        interpretation.evidence.add(EvidenceFlag::chromaticRelation);

    if (situation.pattern.recognized())
        interpretation.evidence.add(EvidenceFlag::patternMatch);
    if (situation.resolution.confirmed)
        interpretation.evidence.add(EvidenceFlag::confirmedResolution);

    return interpretation;
}

HarmonicInterpretation makeLocalInterpretation(const HarmonicSituation& situation) noexcept
{
    HarmonicInterpretation interpretation;
    if (! situation.localKey.valid || ! situation.localHarmonic.valid)
        return interpretation;

    interpretation.valid = true;
    interpretation.kind = HarmonicInterpretationKind::localCenter;
    interpretation.center = situation.localKey;
    interpretation.harmonic = situation.localHarmonic;
    interpretation.pattern = situation.localPattern;
    interpretation.evidence = situation.localKey.evidence;
    interpretation.evidence.markUnique();
    interpretation.evidence.add(EvidenceFlag::inferredLocalCenter);

    if (situation.localPattern.recognized())
        interpretation.evidence.add(EvidenceFlag::patternMatch);

    return interpretation;
}

HarmonicInterpretation makeModalInterpretation(const HarmonicSituation& situation) noexcept
{
    HarmonicInterpretation interpretation;
    if (! situation.harmonic.modalInterchangeCandidate)
        return interpretation;

    const auto parallelKey = makeParallelKey(situation.globalKey.key);
    if (! parallelKey.valid)
        return interpretation;

    auto harmonic = situation.nextChordAvailable
        ? analyzeHarmonicFunction(situation.currentChord, parallelKey, situation.nextChord)
        : analyzeHarmonicFunction(situation.currentChord, parallelKey);

    // The modal alternative is useful only when the current chord becomes
    // structurally diatonic in the parallel mode. Otherwise the original
    // modal-interchange flag is not enough to justify a separate candidate.
    if (! harmonic.valid || harmonic.relation != HarmonicRelation::diatonic)
        return interpretation;

    interpretation.valid = true;
    interpretation.kind = HarmonicInterpretationKind::modalInterchange;
    interpretation.center.valid = true;
    interpretation.center.key = parallelKey;
    interpretation.center.scope = KeyCenterScope::modal;
    interpretation.center.status = KeyCenterStatus::candidate;
    interpretation.center.evidence.confidence = ConfidenceLevel::medium;
    interpretation.center.evidence.markUnique();
    interpretation.center.evidence.add(EvidenceFlag::modalInterchange);
    interpretation.center.evidence.add(EvidenceFlag::borrowedAmbiguity);

    interpretation.harmonic = harmonic;
    interpretation.evidence.confidence = ConfidenceLevel::medium;
    interpretation.evidence.markUnique();
    interpretation.evidence.add(EvidenceFlag::modalInterchange);
    interpretation.evidence.add(EvidenceFlag::borrowedAmbiguity);
    interpretation.evidence.add(EvidenceFlag::alternativeInterpretation);
    return interpretation;
}

int findInterpretation(const HarmonicSituation& situation,
                       HarmonicInterpretationKind kind) noexcept
{
    for (std::uint8_t i = 0; i < situation.interpretationCount; ++i)
    {
        if (situation.interpretations[i].valid
            && situation.interpretations[i].kind == kind)
            return static_cast<int>(i);
    }
    return -1;
}

bool localInterpretationIsDecisive(const HarmonicSituation& situation) noexcept
{
    if (! situation.localKey.valid)
        return false;

    const auto status = situation.localKey.status;
    const auto confidence = situation.localKey.evidence.confidence;
    return confidence == ConfidenceLevel::confirmed
        && (status == KeyCenterStatus::tonicized
            || status == KeyCenterStatus::established);
}

void markAmbiguous(HarmonicSituation& situation,
                   bool globalLocalConflict,
                   bool borrowedAmbiguity) noexcept
{
    situation.primaryInterpretationIndex = -1;
    situation.evidence.markAmbiguous(situation.interpretationCount);
    situation.evidence.add(EvidenceFlag::alternativeInterpretation);

    if (globalLocalConflict)
        situation.evidence.add(EvidenceFlag::globalLocalConflict);
    if (borrowedAmbiguity)
        situation.evidence.add(EvidenceFlag::borrowedAmbiguity);
}
}

void analyzeAmbiguityAndConfidence(HarmonicSituation& situation) noexcept
{
    situation.interpretations = {};
    situation.interpretationCount = 0;
    situation.primaryInterpretationIndex = -1;

    if (! situation.valid)
        return;

    auto globalInterpretation = makeGlobalInterpretation(situation);
    auto localInterpretation = makeLocalInterpretation(situation);
    const auto modalInterpretation = makeModalInterpretation(situation);

    // 0.3f: a local-center inference and a modal-interchange reading can point
    // to exactly the same tonal center. Example: Fm7 -> G7 in project C major
    // yields a candidate C-minor iv-V reading, while borrowed-harmony analysis
    // independently derives the same parallel C-minor center. These are two
    // evidence paths for one musical interpretation, not two different source
    // groups. Keep the structurally richer local reading and carry the modal
    // evidence into it so Stage 3 does not duplicate identical material.
    const bool modalCollapsedIntoLocal = sameTonalCenter(localInterpretation,
                                                         modalInterpretation);
    if (modalCollapsedIntoLocal)
    {
        localInterpretation.evidence.add(EvidenceFlag::modalInterchange);
        localInterpretation.evidence.add(EvidenceFlag::borrowedAmbiguity);
        localInterpretation.evidence.add(EvidenceFlag::alternativeInterpretation);
    }

    addInterpretation(situation, globalInterpretation);
    addInterpretation(situation, localInterpretation);
    if (! modalCollapsedIntoLocal)
        addInterpretation(situation, modalInterpretation);

    const auto globalIndex = findInterpretation(situation,
                                                HarmonicInterpretationKind::globalContext);
    const auto localIndex = findInterpretation(situation,
                                               HarmonicInterpretationKind::localCenter);
    const auto modalIndex = findInterpretation(situation,
                                               HarmonicInterpretationKind::modalInterchange);

    if (situation.interpretationCount <= 1)
    {
        situation.primaryInterpretationIndex = globalIndex >= 0 ? globalIndex : 0;
        situation.evidence.markUnique();
        return;
    }

    // A confirmed tonicization or complete local cadence is stronger evidence
    // than a merely chromatic global reading. Keep the alternatives for
    // diagnostics, but expose the local interpretation as the unique primary.
    if (localIndex >= 0 && localInterpretationIsDecisive(situation))
    {
        situation.primaryInterpretationIndex = localIndex;
        situation.evidence.markUnique();
        situation.evidence.add(EvidenceFlag::alternativeInterpretation);
        if (situation.localKey.evidence.confidence
            > situation.evidence.confidence)
        {
            situation.evidence.confidence = situation.localKey.evidence.confidence;
        }
        return;
    }

    // Candidate and modulation-candidate local centers intentionally remain
    // unresolved. This is the central 0.2e rule: do not silently choose the
    // global or local reading before the timeline supplies enough evidence.
    if (localIndex >= 0)
    {
        markAmbiguous(situation,
                      true,
                      modalIndex >= 0 || modalCollapsedIntoLocal);
        return;
    }

    // Borrowed/modal harmony has at least two musically useful readings:
    // chromatic relative to the project key and diatonic relative to the
    // parallel mode. Preserve both until later context resolves the choice.
    if (modalIndex >= 0)
    {
        markAmbiguous(situation, false, true);
        return;
    }

    situation.primaryInterpretationIndex = globalIndex;
    situation.evidence.markUnique();
}
}
