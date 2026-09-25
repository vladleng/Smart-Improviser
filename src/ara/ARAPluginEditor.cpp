#include "core/analysis/HarmonicConcepts.h"
#include "ara/ARAPluginEditor.h"
#include "ara/ARAPluginProcessor.h"
#include "ara/ARAContextDebugState.h"
#include "context/SharedHarmonicContext.h"
#include "context/TimelineContextMapper.h"
#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include "core/model/ChordModel.h"
#include "core/model/KeyModel.h"

#include <algorithm>
#include <cmath>
#include <string>

#ifndef SMART_IMPROVISER_BUILD_VERSION
#define SMART_IMPROVISER_BUILD_VERSION "dev"
#endif

namespace
{
juce::String ru(const char* text)
{
    return juce::String::fromUTF8(text);
}

juce::String yesNo(bool value)
{
    return value ? ru("ДА") : ru("НЕТ");
}

juce::String utf8String(const std::string& value)
{
    return juce::String::fromUTF8(value.c_str());
}

juce::String localizeGeneratedText(juce::String text)
{
    // Translate complete phrases first so later word-level replacements cannot
    // break their matching (for example "Prepare over ...").
    text = text.replace("Waiting for valid position, chord and key context.",
                        ru("Ожидание корректной позиции, аккорда и тональности."));
    text = text.replace("No explicit chord tones available.",
                        ru("Нет доступных явно заданных звуков аккорда."));

    text = text.replace("No primary interpretation: use the explicit chord tones.",
                        ru("Нет основной трактовки: используй явно заданные звуки аккорда."));
    text = text.replace("This chord needs a dedicated source rule; retain its explicit tones.",
                        ru("Для этого аккорда нужен отдельный источник; сохраняй явно заданные звуки."));
    text = text.replace("No compatible SubV source; retain the confirmed targets.",
                        ru("Нет совместимого источника для SubV; сохраняй подтверждённые цели."));
    text = text.replace("No compatible minor-target source; retain anchors and resolution.",
                        ru("Нет совместимого источника для минорной цели; сохраняй опоры и разрешение."));
    text = text.replace("No confirmed major target for the basic dominant source.",
                        ru("Нет подтверждённой мажорной цели для базового доминантового источника."));
    text = text.replace("No supported major/minor center in the selected interpretation.",
                        ru("В выбранной трактовке нет поддерживаемого мажорного/минорного центра."));
    text = text.replace("No compatible diatonic source for all explicit chord tones/degrees.",
                        ru("Нет диатонического источника, совместимого со всеми заданными звуками и ступенями аккорда."));

    text = text.replace("Build the line around the actual chord tones; connect the available thirds and sevenths.",
                        ru("Строй линию вокруг реальных звуков аккорда; связывай доступные терции и септимы."));
    text = text.replace("Explicit colors remain colors; the list does not make every chord tone equally stable.",
                        ru("Явные краски остаются красками; список не делает все звуки аккорда одинаково устойчивыми."));
    text = text.replace("Emphasize the available thirds/sevenths; no next-chord move is asserted.",
                        ru("Подчёркивай доступные терции и септимы; движение в следующий аккорд не утверждается."));
    text = text.replace("Connect these guides using the confirmed harmonic resolution.",
                        ru("Связывай эти направляющие тоны через подтверждённое гармоническое разрешение."));
    text = text.replace("Try these melodic connections to the next chord.",
                        ru("Попробуй эти мелодические связки к следующему аккорду."));
    text = text.replace("Only present guides are used; optional melodic moves do not establish harmonic function.",
                        ru("Используются только реально присутствующие направляющие тоны; необязательные мелодические движения не определяют гармоническую функцию."));
    text = text.replace("Use these source colors between chord anchors, preserving the selected harmonic context.",
                        ru("Используй эти краски источника между опорными звуками, сохраняя выбранный гармонический контекст."));
    text = text.replace("Natural 11 against major 3 and b13 against natural 5 are passing colors here, not default landing notes.",
                        ru("Натуральная 11-я против большой 3-й и b13 против натуральной 5-й здесь проходящие краски, а не основные точки приземления."));
    text = text.replace("Use the four-note structure as a melodic skeleton; connect it with ",
                        ru("Используй четырёхзвучную структуру как мелодический каркас; связывай её с "));
    text = text.replace(" These notes are shown relative to the actual chord; the full source remains available.",
                        ru(" Эти ноты показаны относительно реального аккорда; полный источник остаётся доступен."));
    text = text.replace("Above (+1 semitone), below (-1), then target (0).",
                        ru("Сверху (+1 полутон), снизу (-1), затем цель (0)."));
    text = text.replace("Below (-1 semitone), then target (0).",
                        ru("Снизу (-1 полутон), затем цель (0)."));
    text = text.replace("Prepare over the current chord; land when the next chord sounds. Choose rhythm and register yourself.",
                        ru("Подготовь движение на текущем аккорде; приди в цель при смене аккорда. Ритм и регистр выбирай самостоятельно."));
    text = text.replace("Resolve within the current chord. Choose rhythm and register yourself.",
                        ru("Разрешай внутри текущего аккорда. Ритм и регистр выбирай самостоятельно."));
    text = text.replace(" Approach notes are passing; chord membership does not guarantee stability. No Phrase or MIDI is generated.",
                        ru(" Подходящие ноты являются проходящими; принадлежность аккорду не гарантирует устойчивость. Фразы и MIDI не генерируются."));

    text = text.replace("Connect the chord anchors using ", ru("Соединяй опорные ноты, используя "));
    text = text.replace("Use chord anchors and targets.", ru("Используй опорные и целевые ноты."));
    text = text.replace("Natural 4th: passing against major 3rd.",
                        ru("Натуральная 4-я: проходящая относительно большой 3-й."));
    text = text.replace("Major 7th on m7: passing to root, not an anchor.",
                        ru("Большая 7-я на m7: проходящая к тонике аккорда, не опора."));
    text = text.replace("Natural 9 color; keep b3, b5 and b7 anchors.",
                        ru("Натуральная 9-я — краска; сохраняй опоры b3, b5 и b7."));
    text = text.replace("#11 color; resolve to the shown target.",
                        ru("#11 — краска; разрешай в показанную цель."));
    text = text.replace("b9/#9/b5/b13; omit natural 5 in this line.",
                        ru("b9/#9/b5/b13; натуральную 5-ю в этой линии не используй."));
    text = text.replace("Whole-half on dim7; follow the actual next chord.",
                        ru("Тон–полутон на dim7; ориентируйся на реальный следующий аккорд."));
    text = text.replace("Melodic minor color; keep the actual chord anchors.",
                        ru("Окраска мелодического минора; сохраняй реальные опорные звуки аккорда."));

    text = text.replace("Chord-tone playing", ru("Игра по звукам аккорда"));
    text = text.replace("Guide-tone targeting", ru("Ведение по направляющим тонам"));
    text = text.replace("Diatonic colors from ", ru("Диатонические краски из "));
    text = text.replace("Chromatic approach", ru("Хроматический подход"));
    text = text.replace("Chromatic enclosure", ru("Хроматическое окружение"));
    text = text.replace("Minor melodic color", ru("Окраска мелодического минора"));
    text = text.replace("Locrian natural 2", ru("Локрийский с натуральной 2-й"));
    text = text.replace("Lydian dominant", ru("Лидийский доминантовый"));
    text = text.replace("Altered dominant", ru("Альтерированная доминанта"));
    text = text.replace("Diminished whole-half", ru("Уменьшённая тон–полутон"));

    text = text.replace(" Ionian", ru(" ионийский"));
    text = text.replace(" Dorian", ru(" дорийский"));
    text = text.replace(" Phrygian", ru(" фригийский"));
    text = text.replace(" Lydian", ru(" лидийский"));
    text = text.replace(" Mixolydian", ru(" миксолидийский"));
    text = text.replace(" Aeolian", ru(" эолийский"));
    text = text.replace(" Locrian", ru(" локрийский"));
    text = text.replace(" melodic minor", ru(" мелодический минор"));
    text = text.replace(" whole-half diminished", ru(" уменьшённая (тон–полутон)"));

    text = text.replace("Material on ", ru("Материал на "));
    text = text.replace(" [passing]", ru(" [проходящая]"));
    text = text.replace("Target: ", ru("Цель: "));
    text = text.replace(" [next chord]", ru(" [следующий аккорд]"));
    text = text.replace(" [current chord]", ru(" [текущий аккорд]"));
    text = text.replace("Confirmed moves: ", ru("Подтверждённые движения: "));
    text = text.replace("Optional moves: ", ru("Необязательные движения: "));
    text = text.replace("Preparation over ", ru("Подготовка на "));
    text = text.replace(" [chord tone]", ru(" [звук аккорда]"));
    text = text.replace(" [non-chord tone]", ru(" [неаккордовый звук]"));

    // Short generic fragments come last.
    text = text.replace("Think ", ru("Мыслить "));
    text = text.replace(" over ", ru(" поверх "));
    text = text.replace(" in ", ru(" в "));
    text = text.replace("guide tones", ru("направляющие тоны"));
    text = text.replace("Phrase", ru("фраза"));

    text = text.replace(" | dominant to major", ru(" | доминанта в мажор"));
    text = text.replace(" | dominant to minor", ru(" | доминанта в минор"));
    text = text.replace(" | dominant chain", ru(" | цепь доминант"));
    text = text.replace(" | other target", ru(" | другая цель"));
    text = text.replace(" | resolution unconfirmed", ru(" | разрешение не подтверждено"));
    text = text.replace(" | secondary", ru(" | вторичная доминанта"));
    text = text.replace(" | interpretation unresolved", ru(" | трактовка не определена"));
    return text;
}

std::string fifthsName(std::int32_t fifths)
{
    switch (fifths)
    {
        case -7: return "Cb";
        case -6: return "Gb";
        case -5: return "Db";
        case -4: return "Ab";
        case -3: return "Eb";
        case -2: return "Bb";
        case -1: return "F";
        case  0: return "C";
        case  1: return "G";
        case  2: return "D";
        case  3: return "A";
        case  4: return "E";
        case  5: return "B";
        case  6: return "F#";
        case  7: return "C#";
        case  8: return "G#";
        case  9: return "D#";
        case 10: return "A#";
        case 11: return "E#";
        default: break;
    }

    static constexpr const char* pitchClassNames[smartimproviser::harmony::kPitchClassCount] =
        { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
    return pitchClassNames[smartimproviser::harmony::circleOfFifthsToPitchClass(fifths)];
}

const char* pitchClassName(int pitchClass) noexcept
{
    static constexpr const char* names[smartimproviser::harmony::kPitchClassCount] =
        { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

    pitchClass %= smartimproviser::harmony::kPitchClassCount;
    if (pitchClass < 0)
        pitchClass += smartimproviser::harmony::kPitchClassCount;
    return names[pitchClass];
}

juce::String keyModeNameRu(smartimproviser::harmony::KeyMode mode)
{
    using smartimproviser::harmony::KeyMode;
    switch (mode)
    {
        case KeyMode::major: return ru("мажор");
        case KeyMode::minor: return ru("минор");
        case KeyMode::custom: return ru("пользовательский");
        case KeyMode::undefined:
        default: return ru("не определён");
    }
}

juce::String harmonicFunctionNameRu(smartimproviser::harmony::HarmonicFunction function)
{
    using smartimproviser::harmony::HarmonicFunction;
    switch (function)
    {
        case HarmonicFunction::tonic: return ru("Тоника");
        case HarmonicFunction::predominant: return ru("Преддоминанта");
        case HarmonicFunction::dominant: return ru("Доминанта");
        case HarmonicFunction::substituteDominant: return "SubV";
        case HarmonicFunction::other: return ru("Другая");
        case HarmonicFunction::undefined:
        default: return "-";
    }
}

juce::String harmonicRelationNameRu(smartimproviser::harmony::HarmonicRelation relation)
{
    using smartimproviser::harmony::HarmonicRelation;
    switch (relation)
    {
        case HarmonicRelation::diatonic: return ru("Диатоническая");
        case HarmonicRelation::chromatic: return ru("Хроматическая");
        case HarmonicRelation::undefined:
        default: return "-";
    }
}

juce::String interpretationKindNameRu(smartimproviser::harmony::HarmonicInterpretationKind kind)
{
    using smartimproviser::harmony::HarmonicInterpretationKind;
    switch (kind)
    {
        case HarmonicInterpretationKind::globalContext: return ru("Глобальная");
        case HarmonicInterpretationKind::localCenter: return ru("Локальный центр");
        case HarmonicInterpretationKind::modalInterchange: return ru("Модальный обмен");
        case HarmonicInterpretationKind::undefined:
        default: return ru("Не определена");
    }
}

juce::String chordDisplayName(const smartimproviser::harmony::ChordContext& context)
{
    if (! context.available)
        return "-";

    const auto chord = smartimproviser::harmony::normalizeChord(context);
    return chord.valid
        ? utf8String(smartimproviser::harmony::normalizedChordSymbol(chord))
        : ru("(нет аккорда)");
}

juce::String keyDisplayName(const smartimproviser::harmony::KeyContext& context)
{
    if (! context.available)
        return "-";

    const auto key = smartimproviser::harmony::normalizeKey(context);
    if (! key.valid)
        return ru("(нет тональности)");

    return utf8String(fifthsName(key.rootFifths))
         + " "
         + keyModeNameRu(key.mode);
}

juce::String patternName(smartimproviser::harmony::HarmonicPatternType type)
{
    using smartimproviser::harmony::HarmonicPatternType;
    switch (type)
    {
        case HarmonicPatternType::none: return ru("Нет");
        case HarmonicPatternType::majorIiVI: return ru("Мажорный ii-V-I");
        case HarmonicPatternType::minorIiHalfDimVi: return ru("Минорный iio-V-i");
        case HarmonicPatternType::dominantToTonic: return "V-I";
        case HarmonicPatternType::turnaroundIVIiiV: return "I-VI-ii-V";
        case HarmonicPatternType::secondaryDominant: return ru("Вторичная доминанта");
        case HarmonicPatternType::tritoneSubstitution: return ru("Тритоновая замена");
        case HarmonicPatternType::backdoorDominant: return ru("Backdoor-доминанта");
        case HarmonicPatternType::minorIvToI: return ru("Минорный iv-I");
        case HarmonicPatternType::passingDiminished: return ru("Проходящий уменьшённый");
        case HarmonicPatternType::commonToneDiminished: return ru("Уменьшённый с общим тоном");
        case HarmonicPatternType::dominantChain: return ru("Цепь доминант");
        case HarmonicPatternType::modalVamp: return ru("Модальный вамп");
        case HarmonicPatternType::undefined:
        default: return "-";
    }
}

juce::String patternRoleName(smartimproviser::harmony::PatternMemberRole role)
{
    using smartimproviser::harmony::PatternMemberRole;
    switch (role)
    {
        case PatternMemberRole::preparation: return ru("Подготовка");
        case PatternMemberRole::predominant: return ru("Преддоминанта");
        case PatternMemberRole::dominant: return ru("Доминанта");
        case PatternMemberRole::substituteDominant: return "SubV";
        case PatternMemberRole::tonic: return ru("Тоника");
        case PatternMemberRole::resolution: return ru("Разрешение");
        case PatternMemberRole::passing: return ru("Проходящий");
        case PatternMemberRole::undefined:
        default: return "-";
    }
}

juce::String confidenceName(smartimproviser::harmony::ConfidenceLevel level)
{
    using smartimproviser::harmony::ConfidenceLevel;
    switch (level)
    {
        case ConfidenceLevel::low: return ru("низкая");
        case ConfidenceLevel::medium: return ru("средняя");
        case ConfidenceLevel::high: return ru("высокая");
        case ConfidenceLevel::confirmed: return ru("подтверждена");
        case ConfidenceLevel::unknown:
        default: return ru("неизвестна");
    }
}

juce::String interpretationName(smartimproviser::harmony::InterpretationStatus status)
{
    using smartimproviser::harmony::InterpretationStatus;
    switch (status)
    {
        case InterpretationStatus::unique: return ru("однозначная");
        case InterpretationStatus::ambiguous: return ru("НЕОДНОЗНАЧНАЯ");
        case InterpretationStatus::unknown:
        default: return ru("неизвестна");
    }
}

juce::String keyCenterScopeName(smartimproviser::harmony::KeyCenterScope scope)
{
    using smartimproviser::harmony::KeyCenterScope;
    switch (scope)
    {
        case KeyCenterScope::global: return ru("глобальный");
        case KeyCenterScope::local: return ru("локальный");
        case KeyCenterScope::temporary: return ru("временный");
        case KeyCenterScope::modal: return ru("модальный");
        case KeyCenterScope::undefined:
        default: return ru("не определён");
    }
}

juce::String keyCenterStatusName(smartimproviser::harmony::KeyCenterStatus status)
{
    using smartimproviser::harmony::KeyCenterStatus;
    switch (status)
    {
        case KeyCenterStatus::candidate: return ru("кандидат");
        case KeyCenterStatus::tonicized: return ru("тонизирован");
        case KeyCenterStatus::established: return ru("установлен");
        case KeyCenterStatus::modulationCandidate: return ru("кандидат на модуляцию");
        case KeyCenterStatus::undefined:
        default: return ru("не определён");
    }
}

juce::String centerKeyDisplayName(const smartimproviser::harmony::KeyCenter& center)
{
    if (! center.valid || ! center.key.valid)
        return "-";

    return utf8String(fifthsName(center.key.rootFifths))
         + " "
         + keyModeNameRu(center.key.mode);
}

juce::String localCenterDisplayName(const smartimproviser::harmony::KeyCenter& center)
{
    if (! center.valid || ! center.key.valid)
        return "-";

    return centerKeyDisplayName(center)
         + "  |  "
         + keyCenterScopeName(center.scope)
         + "  |  "
         + keyCenterStatusName(center.status);
}

juce::String harmonicDisplay(const smartimproviser::harmony::HarmonicAnalysis& harmonic)
{
    if (! harmonic.valid)
        return "-";

    return juce::String(smartimproviser::harmony::scaleDegreeName(harmonic.rootScaleDegree))
         + "  |  "
         + harmonicFunctionNameRu(harmonic.effectiveFunction);
}

juce::String interpretationDisplay(const smartimproviser::harmony::HarmonicInterpretation& interpretation)
{
    if (! interpretation.valid)
        return "-";

    return interpretationKindNameRu(interpretation.kind)
         + "  |  " + centerKeyDisplayName(interpretation.center)
         + "  |  " + harmonicDisplay(interpretation.harmonic)
         + "  |  " + confidenceName(interpretation.evidence.confidence);
}

juce::String patternPositionDisplay(const smartimproviser::harmony::HarmonicPattern& pattern)
{
    if (! pattern.recognized())
        return "-";

    juce::String value(patternRoleName(pattern.role));
    if (pattern.positionIndex >= 0 && pattern.length > 0)
        value += "  |  " + juce::String(pattern.positionIndex + 1)
              + " / " + juce::String(pattern.length);
    return value;
}

juce::String resolutionDisplay(const smartimproviser::harmony::HarmonicSituation& situation)
{
    if (! situation.resolution.available || ! situation.resolution.targetChord.valid)
        return "-";

    auto value = utf8String(smartimproviser::harmony::normalizedChordSymbol(situation.resolution.targetChord));
    value += situation.resolution.confirmed ? ru("  |  ПОДТВЕРЖДЕНО") : ru("  |  ожидается");
    return value;
}

double localBpm(const SharedHarmonicContextSnapshot& shared, double ppq) noexcept
{
    const auto count = shared.tempoEntryStoredCount;
    if (! shared.tempoEntriesAvailable || count < 2 || ppq < 0.0)
        return -1.0;

    int right = 1;
    while (right < count && shared.tempoEntries[right].quarterPosition < ppq)
        ++right;
    right = std::clamp(right, 1, count - 1);

    const auto& a = shared.tempoEntries[right - 1];
    const auto& b = shared.tempoEntries[right];
    const auto deltaTime = b.timePosition - a.timePosition;
    const auto deltaQuarter = b.quarterPosition - a.quarterPosition;
    if (std::abs(deltaTime) <= 1.0e-12)
        return -1.0;

    return 60.0 * deltaQuarter / deltaTime;
}

void drawRow(juce::Graphics& g,
             int y,
             const juce::String& label,
             const juce::String& value,
             bool emphasize = false)
{
    g.setColour(juce::Colour::fromRGB(165, 170, 180));
    g.setFont(15.0f);
    g.drawText(label, 24, y, 190, 22, juce::Justification::centredLeft);

    g.setColour(emphasize ? juce::Colour::fromRGB(235, 240, 245)
                          : juce::Colour::fromRGB(205, 210, 220));
    g.setFont(emphasize ? 15.5f : 15.0f);
    g.drawText(value, 220, y, 390, 22, juce::Justification::centredLeft);
}
}

SmartImproviserARAEditor::SmartImproviserARAEditor(SmartImproviserARAProcessor& p)
    : juce::AudioProcessorEditor(p), processor(p)
{
    setSize(1020, 1010);
    improvisationDetails.setMultiLine(true, true);
    improvisationDetails.setReadOnly(true);
    improvisationDetails.setScrollbarsShown(true);
    improvisationDetails.setCaretVisible(false);
    improvisationDetails.setFont(juce::Font(juce::FontOptions(15.0f)));
    improvisationDetails.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromRGB(42, 46, 53));
    improvisationDetails.setColour(juce::TextEditor::textColourId, juce::Colour::fromRGB(225, 230, 238));
    improvisationDetails.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    improvisationDetails.setBounds(652, 96, 332, 836);
    addAndMakeVisible(improvisationDetails);
    timerCallback();
    startTimerHz(10);
}

void SmartImproviserARAEditor::timerCallback()
{
    cachedShared = SharedHarmonicContextBridge::instance().read();
    const auto ppq = cachedShared.transportAvailable ? cachedShared.transportPpq : -1.0;
    cachedSituation = smartimproviser::harmony::analyzeHarmonicSituation(
        smartimproviser::harmony::mapTimelineHarmonicSnapshot(cachedShared, ppq));
    const auto result = smartimproviser::harmony::analyzeImprovisation(cachedSituation);
    improvisationText = ru("ЭТАП 3 / 0.3e fix2\nГармонические идеи / источники\n\n");
    if (! result.valid)
        improvisationText += localizeGeneratedText(utf8String(result.unavailableReason));
    else
    {
        const auto& strategy = result.strategies.front();
        improvisationText += ru("КОНТЕКСТ\n") + localizeGeneratedText(utf8String(result.contextDescription));
        improvisationText += ru("\n\nГАРМОНИЧЕСКИЕ ИДЕИ\n")
            + localizeGeneratedText(utf8String(smartimproviser::harmony::harmonicConceptsText(result)));
        improvisationText += ru("\nИСТОЧНИКИ ГАММ\n");
        bool hasScale = false;
        for (const auto& scalar : result.strategies)
        {
            if (scalar.source.kind != smartimproviser::harmony::MaterialKind::scale) continue;
            if (hasScale) improvisationText += "\n\n";
            hasScale = true;
            improvisationText += localizeGeneratedText(utf8String(scalar.source.name)) + "\n";
            for (const auto& note : scalar.source.notes) improvisationText += utf8String(note.spelling) + " ";
            if (!scalar.sourceReference.empty())
            {
                improvisationText += "\n" + localizeGeneratedText(utf8String(scalar.idea));
                improvisationText += ru("\nМыслить: ") + utf8String(smartimproviser::harmony::normalizedChordSymbol(scalar.thinkingStructure));
                improvisationText += ru("\nОтносительно аккорда: ");
                for (const auto& note : scalar.source.chordRelativeNotes)
                    improvisationText += utf8String(note.spelling) + " ";
            }
            improvisationText += "\n" + localizeGeneratedText(utf8String(scalar.usageHint));
            if (!scalar.sourceTransitions.empty())
            {
                improvisationText += ru("\nНеобязательные движения красок: ");
                for (const auto& move : scalar.sourceTransitions)
                    improvisationText += juce::String(pitchClassName(move.fromPitchClass)) + "->"
                        + pitchClassName(move.toPitchClass) + " ";
            }
        }
        if (!hasScale) improvisationText += localizeGeneratedText(utf8String(result.scaleUnavailableReason));
        improvisationText += ru("\n\nОПОРНЫЕ НОТЫ АККОРДА\n");
        for (const auto& note : strategy.source.notes)
            improvisationText += juce::String(pitchClassName(note.pitchClass)) + " ";
        const auto notesText = [](const std::vector<smartimproviser::harmony::MaterialNote>& notes)
        {
            juce::String text;
            for (const auto& note : notes) text += juce::String(pitchClassName(note.pitchClass)) + " ";
            return text.isEmpty() ? ru("Нет") : text;
        };
        improvisationText += ru("\n(классы высот)\n\nНАПРАВЛЯЮЩИЕ ТОНЫ (3 / 7)\n") + notesText(strategy.guideNotes);
        improvisationText += ru("\n\nХАРАКТЕРНЫЕ НОТЫ\n") + notesText(strategy.characteristicNotes);
        improvisationText += ru("\n\nЦЕЛИ СЛЕДУЮЩЕГО АККОРДА\n");
        if (strategy.nextChord.valid)
            improvisationText += utf8String(smartimproviser::harmony::normalizedChordSymbol(strategy.nextChord))
                + ": " + notesText(strategy.targetNotes);
        else
            improvisationText += ru("Нет следующего аккорда");
        const bool confirmed = strategy.resolution.available && strategy.resolution.confirmed;
        improvisationText += confirmed ? ru("\n\nПОДТВЕРЖДЁННОЕ РАЗРЕШЕНИЕ\n") : ru("\n\nПРЕДЛОЖЕННЫЕ СВЯЗКИ\n");
        const auto moveText = [](const smartimproviser::harmony::ResolutionMove& move)
        {
            return juce::String(pitchClassName(move.fromPitchClass)) + " -> "
                + pitchClassName(move.toPitchClass) + "  ";
        };
        if (confirmed)
        {
            improvisationText += utf8String(smartimproviser::harmony::normalizedChordSymbol(strategy.resolution.targetChord)) + "\n";
            for (std::size_t i = 0; i < strategy.resolution.moveCount; ++i)
                improvisationText += moveText(strategy.resolution.moves[i]);
            if (strategy.resolution.moveCount == 0) improvisationText += ru("Нет доступных структурных движений");
        }
        else
        {
            for (const auto& move : strategy.suggestedTransitions) improvisationText += moveText(move);
            if (strategy.suggestedTransitions.empty()) improvisationText += ru("Нет");
            improvisationText += ru("\n(не подтверждённое гармоническое разрешение)");
        }
    }
    if (improvisationDetails.getText() != improvisationText)
    {
        improvisationDetails.setText(improvisationText, false);
        improvisationDetails.moveCaretToTop(false);
    }
    repaint();
}

void SmartImproviserARAEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(31, 33, 37));

    g.setColour(juce::Colour::fromRGB(245, 246, 248));
    g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    g.drawText("Smart Improviser " SMART_IMPROVISER_BUILD_VERSION,
               24, 18, 590, 30, juce::Justification::centredLeft);

    g.setColour(juce::Colour::fromRGB(150, 156, 168));
    g.setFont(14.0f);
    g.drawText(ru("Диагностика контекста этапа 1 + гармонического анализа этапа 2"),
               24, 48, 590, 22, juce::Justification::centredLeft);

    const auto debug = ARAContextDebugState::instance().getSnapshot();
    const auto& shared = cachedShared;
    const auto ppq = shared.transportAvailable ? shared.transportPpq : -1.0;
    const auto timeline = smartimproviser::harmony::mapTimelineHarmonicSnapshot(shared, ppq);
    const auto context = smartimproviser::harmony::mapHarmonicContext(shared, ppq);
    const auto& situation = cachedSituation;

    g.setColour(juce::Colour::fromRGB(42, 46, 53));
    g.fillRoundedRectangle(640.0f, 84.0f, 356.0f, 860.0f, 8.0f);
    g.setColour(juce::Colour::fromRGB(225, 230, 238));
    g.setFont(15.0f);

    int y = 84;
    drawRow(g, y, ru("ARA-подключение"), processor.isAraBound() ? ru("ПОДКЛЮЧЕНО") : ru("НЕ ПОДКЛЮЧЕНО"), true); y += 25;
    drawRow(g, y, ru("Контроллер документа"), yesNo(debug.documentControllerCreated)); y += 25;
    drawRow(g, y, ru("Доступ к данным хоста"), yesNo(debug.hostContentAccessAvailable)); y += 25;

    juce::String contextsText = juce::String(debug.musicalContextCount);
    if (debug.selectedMusicalContextIndex >= 0)
        contextsText += ru("  |  выбран ") + juce::String(debug.selectedMusicalContextIndex + 1);
    drawRow(g, y, ru("Музыкальные контексты"), contextsText); y += 25;
    drawRow(g, y, ru("Общий контекст"), yesNo(shared.connected), true); y += 34;

    juce::String transportText;
    if (! shared.transportAvailable)
        transportText = ru("НЕДОСТУПЕН");
    else
        transportText = shared.transportPlaying ? "PLAY" : "STOP";
    drawRow(g, y, ru("Транспорт"), transportText, true); y += 25;
    drawRow(g, y, "PPQ", shared.transportAvailable ? juce::String(shared.transportPpq, 3) : "-"); y += 25;
    drawRow(g, y, ru("Секунды"), shared.transportAvailable ? juce::String(shared.transportSeconds, 3) : "-"); y += 34;

    drawRow(g, y, ru("Тональность"), keyDisplayName(timeline.globalKey), true); y += 25;
    drawRow(g, y, ru("Предыдущий аккорд"),
            timeline.previousChordAvailable ? chordDisplayName(timeline.previousChord) : juce::String("-")); y += 25;
    drawRow(g, y, ru("Текущий аккорд"), chordDisplayName(timeline.currentChord), true); y += 25;
    drawRow(g, y, ru("Следующий аккорд"),
            timeline.nextChordAvailable ? chordDisplayName(timeline.nextChord) : juce::String("-")); y += 25;

    juce::String timeSignature = "-";
    if (context.timeSignature.available)
    {
        timeSignature = juce::String(context.timeSignature.numerator)
                      + "/"
                      + juce::String(context.timeSignature.denominator);
    }
    drawRow(g, y, ru("Размер"), timeSignature); y += 25;

    const auto bpm = localBpm(shared, ppq);
    drawRow(g, y, ru("Темп"), bpm > 0.0 ? juce::String(bpm, 2) + " BPM" : "-"); y += 34;

    const auto counts = ru("Тональность ") + juce::String(shared.keySignatureEventCount)
                      + ru("  |  Аккорды ") + juce::String(shared.sheetChordEventCount)
                      + ru("  |  Темп ") + juce::String(shared.tempoEntryEventCount)
                      + ru("  |  Такты ") + juce::String(shared.barSignatureEventCount);
    drawRow(g, y, ru("ARA-события"), counts); y += 25;
    drawRow(g, y, ru("Изменения"),
            ru("гармония ") + juce::String(static_cast<juce::int64>(shared.revision))
            + ru("  |  транспорт ") + juce::String(static_cast<juce::int64>(shared.transportRevision))); y += 36;

    g.setColour(juce::Colour::fromRGB(77, 81, 89));
    g.drawHorizontalLine(y, 24.0f, 616.0f); y += 12;

    g.setColour(juce::Colour::fromRGB(190, 195, 205));
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText(ru("ЭТАП 2 — ГАРМОНИЧЕСКИЙ АНАЛИЗ"), 24, y, 590, 22, juce::Justification::centredLeft); y += 28;

    drawRow(g, y, ru("Ситуация"), situation.valid ? ru("КОРРЕКТНА") : ru("НЕТ АНАЛИЗА"), true); y += 24;
    drawRow(g, y, ru("Глобальная функция"), harmonicDisplay(situation.harmonic), true); y += 24;

    drawRow(g, y, ru("Связь"),
            situation.valid && situation.harmonic.valid
                ? harmonicRelationNameRu(situation.harmonic.relation)
                : juce::String("-")); y += 24;

    drawRow(g, y, ru("Глобальный оборот"),
            situation.valid ? patternName(situation.pattern.type) : juce::String("-")); y += 24;
    drawRow(g, y, ru("Позиция в обороте"), patternPositionDisplay(situation.pattern)); y += 24;
    drawRow(g, y, ru("Разрешение"), resolutionDisplay(situation)); y += 24;

    g.setColour(juce::Colour::fromRGB(77, 81, 89));
    g.drawHorizontalLine(y, 24.0f, 616.0f); y += 10;

    drawRow(g, y, ru("Локальный центр"), localCenterDisplayName(situation.localKey), true); y += 24;
    drawRow(g, y, ru("Локальная функция"), harmonicDisplay(situation.localHarmonic), true); y += 24;
    drawRow(g, y, ru("Локальный оборот"),
            situation.localPattern.recognized()
                ? patternName(situation.localPattern.type)
                : juce::String("-")); y += 24;
    drawRow(g, y, ru("Локальная позиция"), patternPositionDisplay(situation.localPattern)); y += 24;

    juce::String localConfidence = "-";
    if (situation.localKey.valid)
    {
        localConfidence = confidenceName(situation.localKey.evidence.confidence);
        localConfidence += "  |  ";
        localConfidence += interpretationName(situation.localKey.evidence.interpretation);
    }
    drawRow(g, y, ru("Локальная уверенность"), localConfidence); y += 28;

    g.setColour(juce::Colour::fromRGB(77, 81, 89));
    g.drawHorizontalLine(y, 24.0f, 616.0f); y += 10;

    juce::String confidence = "-";
    if (situation.valid)
    {
        confidence = confidenceName(situation.evidence.confidence);
        confidence += "  |  ";
        confidence += interpretationName(situation.evidence.interpretation);
        confidence += ru("  |  кандидатов ") + juce::String(situation.interpretationCount);
    }
    drawRow(g, y, ru("Трактовка"), confidence, true); y += 24;

    juce::String primary = "-";
    if (situation.primaryInterpretationIndex >= 0
        && situation.primaryInterpretationIndex < situation.interpretationCount)
    {
        primary = interpretationKindNameRu(
            situation.interpretations[static_cast<std::size_t>(situation.primaryInterpretationIndex)].kind);
    }
    else if (situation.valid
             && situation.evidence.interpretation == smartimproviser::harmony::InterpretationStatus::ambiguous)
    {
        primary = ru("НЕ ОПРЕДЕЛЕНА");
    }
    drawRow(g, y, ru("Основная трактовка"), primary, true); y += 24;

    for (std::uint8_t i = 0; i < situation.interpretationCount; ++i)
    {
        drawRow(g,
                y,
                ru("Кандидат ") + juce::String(static_cast<int>(i) + 1),
                interpretationDisplay(situation.interpretations[i]));
        y += 24;
    }

    g.setColour(juce::Colour::fromRGB(105, 110, 120));
    g.setFont(12.5f);
    g.drawText(ru("Диагностический интерфейс для проверки этапов 1/2/3. Продуктовый интерфейс будет разработан позже."),
               24, getHeight() - 28, getWidth() - 48, 20,
               juce::Justification::centredLeft);
}