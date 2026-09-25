#include "core/analysis/ContextRanking.h"
#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"

#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace smartimproviser::harmony;

namespace
{
void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

KeyContext makeKey(std::int32_t rootFifths, bool minor)
{
    KeyContext key;
    key.available = true;
    key.defined = true;
    key.root = rootFifths;
    const int majorIntervals[] = {0,2,4,5,7,9,11};
    const int minorIntervals[] = {0,2,3,5,7,8,10};
    const auto* values = minor ? minorIntervals : majorIntervals;
    for (int i = 0; i < 7; ++i)
        key.intervals.values[static_cast<std::size_t>(values[i])] = 0xFFu;
    return key;
}

ChordContext makeChord(std::int32_t rootFifths,
                       std::initializer_list<int> relativeTones)
{
    ChordContext chord;
    chord.available = true;
    chord.defined = true;
    chord.root = rootFifths;
    chord.bass = rootFifths;
    for (const auto semitone : relativeTones)
        chord.intervals.values[static_cast<std::size_t>(semitone)] = 0xFFu;
    return chord;
}

TimelineHarmonicSnapshot currentOnly(const KeyContext& key,
                                     const ChordContext& current)
{
    TimelineHarmonicSnapshot snapshot;
    snapshot.positionAvailable = true;
    snapshot.ppq = 8.0;
    snapshot.globalKey = key;
    snapshot.currentChord = current;
    return snapshot;
}

TimelineHarmonicSnapshot currentNext(const KeyContext& key,
                                     const ChordContext& current,
                                     const ChordContext& next)
{
    auto snapshot = currentOnly(key, current);
    snapshot.nextChordAvailable = true;
    snapshot.nextChord = next;
    return snapshot;
}

TimelineHarmonicSnapshot fullWindow(const KeyContext& key,
                                    const ChordContext& previous,
                                    const ChordContext& current,
                                    const ChordContext& next)
{
    auto snapshot = currentNext(key, current, next);
    snapshot.previousChordAvailable = true;
    snapshot.previousChord = previous;
    return snapshot;
}

ImprovisationStrategy rankedStrategy(std::string id, int priority,
                                     int interpretation, bool independent = false)
{
    ImprovisationStrategy strategy;
    strategy.ruleId = std::move(id);
    strategy.priority = priority;
    strategy.interpretationIndex = interpretation;
    strategy.interpretationIndependent = independent;
    return strategy;
}

std::vector<std::pair<std::string, int>> signature(const ImprovisationResult& result)
{
    std::vector<std::pair<std::string, int>> values;
    for (const auto& strategy : result.strategies)
        values.emplace_back(strategy.ruleId, strategy.interpretationIndex);
    return values;
}
}

int main()
{
    // Ranking contract itself: unresolved interpretations are stable peers,
    // while interpretation-independent material always remains first.
    ImprovisationResult synthetic;
    synthetic.context.primaryInterpretationIndex = -1;
    synthetic.strategies.push_back(rankedStrategy("alt.one.low", 40, 1));
    synthetic.strategies.push_back(rankedStrategy("core", 100, -1, true));
    synthetic.strategies.push_back(rankedStrategy("alt.zero.low", 40, 0));
    synthetic.strategies.push_back(rankedStrategy("alt.zero.high", 50, 0));
    rankImprovisationStrategies(synthetic);
    expect(synthetic.strategies[0].interpretationIndependent,
           "interpretation-independent foundation ranks first");
    expect(synthetic.strategies[1].interpretationIndex == 0
           && synthetic.strategies[1].ruleId == "alt.zero.high"
           && synthetic.strategies[2].interpretationIndex == 0
           && synthetic.strategies[3].interpretationIndex == 1,
           "unresolved alternatives use deterministic index then priority order");

    synthetic.context.primaryInterpretationIndex = 1;
    synthetic.strategies = {
        rankedStrategy("global.high", 90, 0),
        rankedStrategy("local.low", 10, 1),
        rankedStrategy("core", 100, -1, true)
    };
    rankImprovisationStrategies(synthetic);
    expect(synthetic.strategies[0].interpretationIndependent
           && synthetic.strategies[1].interpretationIndex == 1
           && synthetic.strategies[2].interpretationIndex == 0,
           "selected primary interpretation ranks before alternatives regardless of catalog priority");
    expect(synthetic.strategies[1].priority == 10 && synthetic.strategies[2].priority == 90,
           "ranking does not rewrite recommendation priority");

    const auto cMajor = makeKey(0, false);
    const auto fMinor7 = makeChord(-1, {0,3,7,10});

    // Borrowed iv is unresolved global/modal ambiguity. 0.3f must keep both
    // provenance groups and must not assign either one as hidden primary.
    const auto borrowedSituation = analyzeHarmonicSituation(currentOnly(cMajor, fMinor7));
    expect(borrowedSituation.primaryInterpretationIndex == -1
           && borrowedSituation.interpretationCount == 2,
           "borrowed iv exposes unresolved global/modal interpretations");
    const auto borrowed = analyzeImprovisation(borrowedSituation);
    expect(borrowed.valid && !borrowed.strategies.empty()
           && borrowed.strategies.front().interpretationIndependent
           && borrowed.strategies.front().interpretationIndex == -1,
           "ambiguous result keeps explicit chord foundation independent");

    bool hasInterpretation0 = false;
    bool hasInterpretation1 = false;
    for (std::size_t i = 1; i < borrowed.strategies.size(); ++i)
    {
        const auto& strategy = borrowed.strategies[i];
        expect(!strategy.interpretationIndependent && strategy.interpretationIndex >= 0,
               "every context-dependent ambiguous strategy names its interpretation");
        hasInterpretation0 |= strategy.interpretationIndex == 0;
        hasInterpretation1 |= strategy.interpretationIndex == 1;
    }
    expect(hasInterpretation0 && hasInterpretation1,
           "borrowed ambiguity retains material from both harmonic readings");

    const auto borrowedAgain = analyzeImprovisation(borrowedSituation);
    expect(signature(borrowed) == signature(borrowedAgain),
           "ambiguous strategy ordering is deterministic");

    // Candidate local center remains unresolved but both global/local material
    // must survive as separately attributed strategies.
    const auto fMajor = makeKey(-1, false);
    const auto eHalfDim7 = makeChord(4, {0,3,6,10});
    const auto a7 = makeChord(3, {0,4,7,10});
    const auto dMinor = makeChord(2, {0,3,7});
    const auto candidateSituation = analyzeHarmonicSituation(
        currentNext(fMajor, eHalfDim7, a7));
    expect(candidateSituation.primaryInterpretationIndex == -1
           && candidateSituation.interpretationCount >= 2,
           "candidate local center remains unresolved");
    const auto candidate = analyzeImprovisation(candidateSituation);
    bool candidateGlobal = false;
    bool candidateLocal = false;
    for (const auto& strategy : candidate.strategies)
    {
        if (strategy.interpretationIndependent) continue;
        candidateGlobal |= strategy.interpretationIndex == 0;
        candidateLocal |= strategy.interpretationIndex == 1;
    }
    expect(candidateGlobal && candidateLocal,
           "candidate local center retains global and local strategy groups");

    // A complete local cadence selects the local interpretation. Alternatives
    // remain present but the selected context ranks first after foundation.
    const auto establishedSituation = analyzeHarmonicSituation(
        fullWindow(fMajor, eHalfDim7, a7, dMinor));
    expect(establishedSituation.primaryInterpretationIndex >= 0,
           "complete local cadence selects a primary interpretation");
    const auto established = analyzeImprovisation(establishedSituation);
    expect(established.strategies.size() >= 3,
           "confirmed local cadence retains at least foundation, primary and alternative material");
    expect(established.strategies[0].interpretationIndependent,
           "foundation still ranks first for confirmed local cadence");

    std::size_t firstDependent = 1;
    while (firstDependent < established.strategies.size()
           && established.strategies[firstDependent].interpretationIndependent)
        ++firstDependent;
    expect(firstDependent < established.strategies.size()
           && established.strategies[firstDependent].interpretationIndex
                == establishedSituation.primaryInterpretationIndex,
           "primary interpretation ranks before context alternatives");

    bool hasAlternative = false;
    for (std::size_t i = firstDependent; i < established.strategies.size(); ++i)
        hasAlternative |= established.strategies[i].interpretationIndex
            != establishedSituation.primaryInterpretationIndex;
    expect(hasAlternative,
           "confirmed primary does not erase alternative interpretation material");

    std::cout << "SmartImproviser ContextRankingTests: OK\n";
    return 0;
}
