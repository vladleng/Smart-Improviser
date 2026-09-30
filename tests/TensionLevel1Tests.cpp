#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/MaterialViewer.h"
#include "core/analysis/TensionEngine.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>

using namespace smartimproviser::harmony;

namespace
{
void expect(bool value, const char* message)
{
    if (!value) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}

ChordContext chord(int rootFifths, std::initializer_list<int> tones, int bassFifths = 999)
{
    ChordContext result;
    result.available = result.defined = true;
    result.root = rootFifths;
    result.bass = bassFifths == 999 ? rootFifths : bassFifths;
    for (const int interval : tones)
        result.intervals.values[static_cast<std::size_t>(interval)] = 0xFFu;
    return result;
}

TimelineHarmonicSnapshot snapshot(ChordContext current, ChordContext next)
{
    TimelineHarmonicSnapshot result;
    result.positionAvailable = true;
    result.ppq = 4.0;
    result.globalKey.available = result.globalKey.defined = true;
    result.globalKey.root = 0;
    for (const int tone : {0, 2, 4, 5, 7, 9, 11})
        result.globalKey.intervals.values[static_cast<std::size_t>(tone)] = 0xFFu;
    result.previousChordAvailable = true;
    result.previousChord = chord(2, {0, 3, 7, 10});
    result.currentChord = current;
    result.nextChordAvailable = true;
    result.nextChord = next;
    return result;
}

ImprovisationResult analyze(const TimelineHarmonicSnapshot& timeline)
{
    return analyzeImprovisation(analyzeHarmonicSituation(timeline));
}

const TensionCandidate& first(const TensionProfile& profile)
{
    const auto* band = profile.band(TensionLevel::stable);
    expect(band && band->hasRecommendation(), "T1 has an explicit recommendation");
    expect(profile.band(TensionLevel::color)->assessment == TensionAssessment::notEvaluated
           && profile.band(TensionLevel::outsideMaximum)->assessment == TensionAssessment::notEvaluated,
           "T1 evaluation leaves T2/T3 unevaluated");
    return band->alternatives.front();
}

bool has(const std::vector<MaterialNote>& notes, int pitch)
{
    return std::any_of(notes.begin(), notes.end(), [pitch](const auto& note)
    { return note.pitchClass == pitch; });
}

bool role(const TensionCandidate& candidate, int pitch, TensionRole wanted)
{
    return std::any_of(candidate.noteRoles.begin(), candidate.noteRoles.end(), [&](const auto& assignment)
    { return assignment.note.pitchClass == pitch && assignment.role == wanted; });
}
}

int main()
{
    auto major = analyze(snapshot(chord(1, {0, 4, 7, 10}), chord(0, {0, 4, 7, 11})));
    expect(major.valid, "G7 to Cmaj7 fixture is analyzed by Core");
    const auto t1 = analyzeStableTension(major);
    const auto& m6 = first(t1);
    expect(m6.strategy.ruleId == "project.t1.major-V-m6"
           && m6.strategy.source.name == "Dm6" && m6.strategy.source.notes.size() == 4,
           "confirmed major V offers a four-note Dm6 thinking structure");
    for (int pitch : {2, 5, 9, 11})
        expect(has(m6.strategy.source.notes, pitch), "Dm6 retains all four pitches");
    expect(!has(m6.strategy.source.notes, 1) && !has(m6.strategy.source.notes, 4),
           "C# from the full melodic minor and its passing E are not stable m6 source notes");
    expect(role(m6, 9, TensionRole::stableExtension)
           && role(m6, 7, TensionRole::chordAnchor)
           && role(m6, 4, TensionRole::passingApproach),
           "m6 extension, literal G root, and optional passing E have separate jobs");
    expect(m6.strategy.actualChord.rootPitchClass == 7
           && m6.strategy.nextChord.rootPitchClass == 0
           && m6.strategy.resolution.confirmed,
           "the applied source preserves real G7 and confirmed C target");
    major.strategies.push_back(m6.strategy);
    auto explanation = buildExplanation(major);
    bool viewed = false;
    for (std::size_t i = 0; i < explanation.items.size(); ++i)
        if (explanation.items[i].source.name == "Dm6"
            && !explanation.items[i].strategyIndices.empty()
            && major.strategies[explanation.items[i].strategyIndices.front()].tensionClassified)
        {
            const auto view = buildMaterialView(major, explanation, i);
            const auto root = std::find_if(view.current.begin(), view.current.end(), [](const auto& note)
            { return note.pitchClass == 7; });
            expect(view.valid && root != view.current.end()
                   && visibleInLayer(*root, ViewerLayer::chord)
                   && !visibleInLayer(*root, ViewerLayer::source),
                   "written dominant root stays on the chord layer, outside four-note m6");
            viewed = true;
        }
    expect(viewed, "classified T1 has its own selectable visual item");

    auto minorTarget = analyze(snapshot(chord(1, {0, 4, 7, 10}), chord(0, {0, 3, 7, 10})));
    expect(first(analyzeStableTension(minorTarget)).strategy.ruleId == "project.t1.explicit-anchors",
           "V to minor never inherits the confirmed major V m6 rule");

    auto flatNine = analyze(snapshot(chord(1, {0, 1, 4, 7, 10}), chord(0, {0, 4, 7, 11})));
    const auto b9Profile = analyzeStableTension(flatNine);
    const auto& b9 = first(b9Profile);
    expect(b9.strategy.ruleId == "project.t1.explicit-anchors"
           && has(b9.strategy.source.notes, 8) && !has(b9.strategy.source.notes, 9),
           "written G7b9 keeps Ab and cannot silently become ordinary Dm6");
    auto sharpFive = analyze(snapshot(chord(1, {0, 4, 8, 10}), chord(0, {0, 4, 7, 11})));
    const auto augProfile = analyzeStableTension(sharpFive);
    const auto& aug = first(augProfile);
    expect(aug.strategy.ruleId == "project.t1.explicit-anchors"
           && has(aug.strategy.source.notes, 3),
           "written #5 survives as a literal anchor rather than forcing a perfect fifth");

    auto unknown = snapshot(chord(1, {0, 4, 7, 10}), chord(0, {0, 4, 7, 11}));
    unknown.nextChordAvailable = false;
    expect(first(analyzeStableTension(analyze(unknown))).strategy.ruleId == "project.t1.explicit-anchors",
           "an unknown next chord does not invent a tonic or m6 application");

    auto dm = analyze(snapshot(chord(2, {0, 3, 7, 10}), chord(1, {0, 4, 7, 10})));
    const auto minorNineProfile = analyzeStableTension(dm);
    const auto& minorNine = first(minorNineProfile);
    expect(minorNine.strategy.ruleId == "project.t1.diatonic-nine"
           && has(minorNine.strategy.source.notes, 4)
           && role(minorNine, 4, TensionRole::stableExtension),
           "diatonic Dm7 permits E as a stable natural ninth");

    auto cmaj = analyze(snapshot(chord(0, {0, 4, 7, 11}), chord(2, {0, 3, 7, 10})));
    const auto majorNineProfile = analyzeStableTension(cmaj);
    const auto& majorNine = first(majorNineProfile);
    expect(majorNine.strategy.ruleId == "project.t1.diatonic-nine"
           && has(majorNine.strategy.source.notes, 2)
           && !has(majorNine.strategy.source.notes, 5)
           && role(majorNine, 5, TensionRole::passingApproach),
           "Cmaj7 adds D9 but F4 is only a passing role, not a stable source note");

    auto sus = analyze(snapshot(chord(1, {0, 5, 7, 10}), chord(0, {0, 4, 7, 11})));
    expect(first(analyzeStableTension(sus)).strategy.ruleId == "project.t1.explicit-anchors",
           "suspended chord never receives an invented third");

    auto ambiguous = analyze(snapshot(chord(1, {0, 4, 7, 10}), chord(0, {0, 4, 7, 11})));
    ambiguous.context.primaryInterpretationIndex = -1;
    ambiguous.context.evidence.markAmbiguous(2);
    for (auto& strategy : ambiguous.strategies)
        if (strategy.ruleId == "boyko.melodic-minor.V")
        {
            auto alternative = strategy;
            strategy.interpretationIndex = 0;
            strategy.interpretationIndependent = false;
            alternative.interpretationIndex = 1;
            alternative.interpretationIndependent = false;
            ambiguous.strategies.push_back(alternative);
            break;
        }
    auto profile = analyzeStableTension(ambiguous);
    expect(profile.context.primaryInterpretationIndex == -1
           && profile.context.evidence.interpretation == InterpretationStatus::ambiguous
           && profile.band(TensionLevel::stable)->alternatives.size() == 2
           && profile.band(TensionLevel::stable)->alternatives[0].strategy.interpretationIndex == 0
           && profile.band(TensionLevel::stable)->alternatives[1].strategy.interpretationIndex == 1,
           "T1 retains two source interpretations without selecting a hidden winner");

    std::cout << "Tension Level 1 tests passed\n";
}
