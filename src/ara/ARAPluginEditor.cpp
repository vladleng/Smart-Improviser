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
juce::String yesNo(bool value)
{
    return value ? "ДА" : "НЕТ";
}

juce::String utf8String(const std::string& value)
{
    return juce::String::fromUTF8(value.c_str());
}

juce::String localizeGeneratedText(juce::String text)
{
    text = text.replace("Waiting for valid position, chord and key context.",
                        "Ожидание корректной позиции, аккорда и тональности.");
    text = text.replace("No explicit chord tones available.",
                        "Нет доступных явно заданных звуков аккорда.");

    text = text.replace(" | dominant to major", " | доминанта в мажор");
    text = text.replace(" | dominant to minor", " | доминанта в минор");
    text = text.replace(" | dominant chain", " | цепь доминант");
    text = text.replace(" | other target", " | другая цель");
    text = text.replace(" | resolution unconfirmed", " | разрешение не подтверждено");
    text = text.replace(" | secondary", " | вторичная доминанта");
    text = text.replace(" | interpretation unresolved", " | трактовка не определена");

    text = text.replace("No primary interpretation: use the explicit chord tones.",
                        "Нет основной трактовки: используй явно заданные звуки аккорда.");
    text = text.replace("This chord needs a dedicated source rule; retain its explicit tones.",
                        "Для этого аккорда нужен отдельный источник; сохраняй явно заданные звуки.");
    text = text.replace("No compatible SubV source; retain the confirmed targets.",
                        "Нет совместимого источника для SubV; сохраняй подтверждённые цели.");
    text = text.replace("No compatible minor-target source; retain anchors and resolution.",
                        "Нет совместимого источника для минорной цели; сохраняй опоры и разрешение.");
    text = text.replace("No confirmed major target for the basic dominant source.",
                        "Нет подтверждённой мажорной цели для базового доминантового источника.");
    text = text.replace("No supported major/minor center in the selected interpretation.",
                        "В выбранной трактовке нет поддерживаемого мажорного/минорного центра.");
    text = text.replace("No compatible diatonic source for all explicit chord tones/degrees.",
                        "Нет диатонического источника, совместимого со всеми заданными звуками и ступенями аккорда.");

    text = text.replace("Connect the chord anchors using ", "Соединяй опорные ноты, используя ");
    text = text.replace("Use chord anchors and targets.", "Используй опорные и целевые ноты.");
    text = text.replace("Natural 4th: passing against major 3rd.",
                        "Натуральная 4-я: проходящая относительно большой 3-й.");
    text = text.replace("Major 7th on m7: passing to root, not an anchor.",
                        "Большая 7-я на m7: проходящая к тонике аккорда, не опора.");
    text = text.replace("Natural 9 color; keep b3, b5 and b7 anchors.",
                        "Натуральная 9-я — краска; сохраняй опоры b3, b5 и b7.");
    text = text.replace("#11 color; resolve to the shown target.",
                        "#11 — краска; разрешай в показанную цель.");
    text = text.replace("b9/#9/b5/b13; omit natural 5 in this line.",
                        "b9/#9/b5/b13; натуральную 5-ю в этой линии не используй.");
    text = text.replace("Whole-half on dim7; follow the actual next chord.",
                        "Тон–полутон на dim7; ориентируйся на реальный следующий аккорд.");
    text = text.replace("Melodic minor color; keep the actual chord anchors.",
                        "Окраска melodic minor; сохраняй реальные опорные звуки аккорда.");

    text = text.replace("Chord-tone playing", "Игра по звукам аккорда");
    text = text.replace("Guide-tone targeting", "Ведение по guide tones");
    text = text.replace("Diatonic colors from ", "Диатонические краски из ");
    text = text.replace("Chromatic approach", "Хроматический подход");
    text = text.replace("Chromatic enclosure", "Хроматическое окружение");
    text = text.replace("Think ", "Мыслить ");
    text = text.replace(" over ", " поверх ");

    text = text.replace("Build the line around the actual chord tones; connect the available thirds and sevenths.",
                        "Строй линию вокруг реальных звуков аккорда; связывай доступные терции и септимы.");
    text = text.replace("Explicit colors remain colors; the list does not make every chord tone equally stable.",
                        "Явные краски остаются красками; список не делает все звуки аккорда одинаково устойчивыми.");
    text = text.replace("Emphasize the available thirds/sevenths; no next-chord move is asserted.",
                        "Подчёркивай доступные терции/септимы; движение в следующий аккорд не утверждается.");
    text = text.replace("Connect these guides using the confirmed harmonic resolution.",
                        "Связывай эти guide tones через подтверждённое гармоническое разрешение.");
    text = text.replace("Try these melodic connections to the next chord.",
                        "Попробуй эти мелодические связки к следующему аккорду.");
    text = text.replace("Only present guides are used; optional melodic moves do not establish harmonic function.",
                        "Используются только реально присутствующие guide tones; необязательные мелодические движения не определяют гармоническую функцию.");
    text = text.replace("Use these source colors between chord anchors, preserving the selected harmonic context.",
                        "Используй эти краски источника между опорными звуками, сохраняя выбранный гармонический контекст.");
    text = text.replace("Natural 11 against major 3 and b13 against natural 5 are passing colors here, not default landing notes.",
                        "Натуральная 11-я против большой 3-й и b13 против натуральной 5-й здесь проходящие краски, а не основные точки приземления.");
    text = text.replace("Use the four-note structure as a melodic skeleton; connect it with ",
                        "Используй четырёхзвучную структуру как мелодический каркас; связывай её с ");
    text = text.replace(" These notes are shown relative to the actual chord; the full source remains available.",
                        " Эти ноты показаны относительно реального аккорда; полный источник остаётся доступен.");
    text = text.replace("Above (+1 semitone), below (-1), then target (0).",
                        "Сверху (+1 полутон), снизу (-1), затем цель (0).");
    text = text.replace("Below (-1 semitone), then target (0).",
                        "Снизу (-1 полутон), затем цель (0).");
    text = text.replace("Prepare over the current chord; land when the next chord sounds. Choose rhythm and register yourself.",
                        "Подготовь движение на текущем аккорде; приди в цель при смене аккорда. Ритм и регистр выбирай самостоятельно.");
    text = text.replace("Resolve within the current chord. Choose rhythm and register yourself.",
                        "Разрешай внутри текущего аккорда. Ритм и регистр выбирай самостоятельно.");
    text = text.replace(" Approach notes are passing; chord membership does not guarantee stability. No Phrase or MIDI is generated.",
                        " Подходящие ноты являются проходящими; принадлежность аккорду не гарантирует устойчивость. Phrase и MIDI не генерируются.");

    text = text.replace("Material on ", "Материал на ");
    text = text.replace(" [passing]", " [проходящая]");
    text = text.replace("Target: ", "Цель: ");
    text = text.replace(" [next chord]", " [следующий аккорд]");
    text = text.replace(" [current chord]", " [текущий аккорд]");
    text = text.replace("Confirmed moves: ", "Подтверждённые движения: ");
    text = text.replace("Optional moves: ", "Необязательные движения: ");
    text = text.replace("Preparation over ", "Подготовка на ");
    text = text.replace(" [chord tone]", " [звук аккорда]");
    text = text.replace(" [non-chord tone]", " [неаккордовый звук]");
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

const char* keyModeNameRu(smartimproviser::harmony::KeyMode mode) noexcept
{
    using smartimproviser::harmony::KeyMode;
    switch (mode)
    {
        case KeyMode::major: return "мажор";
        case KeyMode::minor: return "минор";
        case KeyMode::custom: return "пользовательский";
        case KeyMode::undefined:
        default: return "не определён";
    }
}

const char* harmonicFunctionNameRu(smartimproviser::harmony::HarmonicFunction function) noexcept
{
    using smartimproviser::harmony::HarmonicFunction;
    switch (function)
    {
        case HarmonicFunction::tonic: return "Тоника";
        case HarmonicFunction::predominant: return "Преддоминанта";
        case HarmonicFunction::dominant: return "Доминанта";
        case HarmonicFunction::substituteDominant: return "SubV";
        case HarmonicFunction::other: return "Другая";
        case HarmonicFunction::undefined:
        default: return "-";
    }
}

const char* harmonicRelationNameRu(smartimproviser::harmony::HarmonicRelation relation) noexcept
{
    using smartimproviser::harmony::HarmonicRelation;
    switch (relation)
    {
        case HarmonicRelation::diatonic: return "Диатоническая";
        case HarmonicRelation::chromatic: return "Хроматическая";
        case HarmonicRelation::undefined:
        default: return "-";
    }
}

const char* interpretationKindNameRu(smartimproviser::harmony::HarmonicInterpretationKind kind) noexcept
{
    using smartimproviser::harmony::HarmonicInterpretationKind;
    switch (kind)
    {
        case HarmonicInterpretationKind::globalContext: return "Глобальная";
        case HarmonicInterpretationKind::localCenter: return "Локальный центр";
        case HarmonicInterpretationKind::modalInterchange: return "Модальный обмен";
        case HarmonicInterpretationKind::undefined:
        default: return "Не определена";
    }
}

juce::String chordDisplayName(const smartimproviser::harmony::ChordContext& context)
{
    if (! context.available)
        return "-";

    const auto chord = smartimproviser::harmony::normalizeChord(context);
    return chord.valid
        ? utf8String(smartimproviser::harmony::normalizedChordSymbol(chord))
        : juce::String("(нет аккорда)");
}

juce::String keyDisplayName(const smartimproviser::harmony::KeyContext& context)
{
    if (! context.available)
        return "-";

    const auto key = smartimproviser::harmony::normalizeKey(context);
    if (! key.valid)
        return "(нет тональности)";

    return utf8String(fifthsName(key.rootFifths))
         + " "
         + keyModeNameRu(key.mode);
}

const char* patternName(smartimproviser::harmony::HarmonicPatternType type) noexcept
{
    using smartimproviser::harmony::HarmonicPatternType;
    switch (type)
    {
        case HarmonicPatternType::none: return "Нет";
        case HarmonicPatternType::majorIiVI: return "Мажорный ii-V-I";
        case HarmonicPatternType::minorIiHalfDimVi: return "Минорный iio-V-i";
        case HarmonicPatternType::dominantToTonic: return "V-I";
        case HarmonicPatternType::turnaroundIVIiiV: return "I-VI-ii-V";
        case HarmonicPatternType::secondaryDominant: return "Вторичная доминанта";
        case HarmonicPatternType::tritoneSubstitution: return "Тритоновая замена";
        case HarmonicPatternType::backdoorDominant: return "Backdoor-доминанта";
        case HarmonicPatternType::minorIvToI: return "Минорный iv-I";
        case HarmonicPatternType::passingDiminished: return "Проходящий уменьшённый";
        case HarmonicPatternType::commonToneDiminished: return "Уменьшённый с общим тоном";
        case HarmonicPatternType::dominantChain: return "Цепь доминант";
        case HarmonicPatternType::modalVamp: return "Модальный vamp";
        case HarmonicPatternType::undefined:
        default: return "-";
    }
}

const char* patternRoleName(smartimproviser::harmony::PatternMemberRole role) noexcept
{
    using smartimproviser::harmony::PatternMemberRole;
    switch (role)
    {
        case PatternMemberRole::preparation: return "Подготовка";
        case PatternMemberRole::predominant: return "Преддоминанта";
        case PatternMemberRole::dominant: return "Доминанта";
        case PatternMemberRole::substituteDominant: return "SubV";
        case PatternMemberRole::tonic: return "Тоника";
        case PatternMemberRole::resolution: return "Разрешение";
        case PatternMemberRole::passing: return "Проходящий";
        case PatternMemberRole::undefined:
        default: return "-";
    }
}

const char* confidenceName(smartimproviser::harmony::ConfidenceLevel level) noexcept
{
    using smartimproviser::harmony::ConfidenceLevel;
    switch (level)
    {
        case ConfidenceLevel::low: return "низкая";
        case ConfidenceLevel::medium: return "средняя";
        case ConfidenceLevel::high: return "высокая";
        case ConfidenceLevel::confirmed: return "подтверждено";
        case ConfidenceLevel::unknown:
        default: return "неизвестно";
    }
}

const char* interpretationName(smartimproviser::harmony::InterpretationStatus status) noexcept
{
    using smartimproviser::harmony::InterpretationStatus;
    switch (status)
    {
        case InterpretationStatus::unique: return "однозначная";
        case InterpretationStatus::ambiguous: return "НЕОДНОЗНАЧНАЯ";
        case InterpretationStatus::unknown:
        default: return "неизвестна";
    }
}

const char* keyCenterScopeName(smartimproviser::harmony::KeyCenterScope scope) noexcept
{
    using smartimproviser::harmony::KeyCenterScope;
    switch (scope)
    {
        case KeyCenterScope::global: return "глобальный";
        case KeyCenterScope::local: return "локальный";
        case KeyCenterScope::temporary: return "временный";
        case KeyCenterScope::modal: return "модальный";
        case KeyCenterScope::undefined:
        default: return "не определён";
    }
}

const char* keyCenterStatusName(smartimproviser::harmony::KeyCenterStatus status) noexcept
{
    using smartimproviser::harmony::KeyCenterStatus;
    switch (status)
    {
        case KeyCenterStatus::candidate: return "кандидат";
        case KeyCenterStatus::tonicized: return "тонизирован";
        case KeyCenterStatus::established: return "установлен";
        case KeyCenterStatus::modulationCandidate: return "кандидат на модуляцию";
        case KeyCenterStatus::undefined:
        default: return "не определён";
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

    return juce::String(interpretationKindNameRu(interpretation.kind))
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
    value += situation.resolution.confirmed ? "  |  ПОДТВЕРЖДЕНО" : "  |  ожидается";
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
    improvisationText = "ЭТАП 3 / 0.3e fix1\nГармонические идеи / источники\n\n";
    if (! result.valid)
        improvisationText += localizeGeneratedText(utf8String(result.unavailableReason));
    else
    {
        const auto& strategy = result.strategies.front();
        improvisationText += "КОНТЕКСТ\n" + localizeGeneratedText(utf8String(result.contextDescription));
        improvisationText += "\n\nГАРМОНИЧЕСКИЕ ИДЕИ\n"
            + localizeGeneratedText(utf8String(smartimproviser::harmony::harmonicConceptsText(result)));
        improvisationText += "\nИСТОЧНИКИ ГАММ\n";
        bool hasScale = false;
        for (const auto& scalar : result.strategies)
        {
            if (scalar.source.kind != smartimproviser::harmony::MaterialKind::scale) continue;
            if (hasScale) improvisationText += "\n\n";
            hasScale = true;
            improvisationText += utf8String(scalar.source.name) + "\n";
            for (const auto& note : scalar.source.notes) improvisationText += utf8String(note.spelling) + " ";
            if (!scalar.sourceReference.empty())
            {
                improvisationText += "\n" + localizeGeneratedText(utf8String(scalar.idea));
                improvisationText += "\nМыслить: " + utf8String(smartimproviser::harmony::normalizedChordSymbol(scalar.thinkingStructure));
                improvisationText += "\nОтносительно аккорда: ";
                for (const auto& note : scalar.source.chordRelativeNotes)
                    improvisationText += utf8String(note.spelling) + " ";
            }
            improvisationText += "\n" + localizeGeneratedText(utf8String(scalar.usageHint));
            if (!scalar.sourceTransitions.empty())
            {
                improvisationText += "\nНеобязательные движения красок: ";
                for (const auto& move : scalar.sourceTransitions)
                    improvisationText += juce::String(pitchClassName(move.fromPitchClass)) + "->"
                        + pitchClassName(move.toPitchClass) + " ";
            }
        }
        if (!hasScale) improvisationText += localizeGeneratedText(utf8String(result.scaleUnavailableReason));
        improvisationText += "\n\nОПОРНЫЕ НОТЫ АККОРДА\n";
        for (const auto& note : strategy.source.notes)
            improvisationText += juce::String(pitchClassName(note.pitchClass)) + " ";
        const auto notesText = [](const std::vector<smartimproviser::harmony::MaterialNote>& notes)
        {
            juce::String text;
            for (const auto& note : notes) text += juce::String(pitchClassName(note.pitchClass)) + " ";
            return text.isEmpty() ? juce::String("Нет") : text;
        };
        improvisationText += "\n(классы высот)\n\nGUIDE TONES (3 / 7)\n" + notesText(strategy.guideNotes);
        improvisationText += "\n\nХАРАКТЕРНЫЕ НОТЫ\n" + notesText(strategy.characteristicNotes);
        improvisationText += "\n\nЦЕЛИ СЛЕДУЮЩЕГО АККОРДА\n";
        if (strategy.nextChord.valid)
            improvisationText += utf8String(smartimproviser::harmony::normalizedChordSymbol(strategy.nextChord))
                + ": " + notesText(strategy.targetNotes);
        else
            improvisationText += "Нет следующего аккорда";
        const bool confirmed = strategy.resolution.available && strategy.resolution.confirmed;
        improvisationText += confirmed ? "\n\nПОДТВЕРЖДЁННОЕ РАЗРЕШЕНИЕ\n" : "\n\nПРЕДЛОЖЕННЫЕ СВЯЗКИ\n";
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
            if (strategy.resolution.moveCount == 0) improvisationText += "Нет доступных структурных движений";
        }
        else
        {
            for (const auto& move : strategy.suggestedTransitions) improvisationText += moveText(move);
            if (strategy.suggestedTransitions.empty()) improvisationText += "Нет";
            improvisationText += "\n(не подтверждённое гармоническое разрешение)";
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
    g.drawText("Диагностика контекста Stage 1 + Harmonic Engine Stage 2",
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
    drawRow(g, y, "ARA-подключение", processor.isAraBound() ? "ПОДКЛЮЧЕНО" : "НЕ ПОДКЛЮЧЕНО", true); y += 25;
    drawRow(g, y, "Контроллер документа", yesNo(debug.documentControllerCreated)); y += 25;
    drawRow(g, y, "Доступ к данным хоста", yesNo(debug.hostContentAccessAvailable)); y += 25;

    juce::String contextsText = juce::String(debug.musicalContextCount);
    if (debug.selectedMusicalContextIndex >= 0)
        contextsText += "  |  выбран " + juce::String(debug.selectedMusicalContextIndex + 1);
    drawRow(g, y, "Музыкальные контексты", contextsText); y += 25;
    drawRow(g, y, "Общий контекст", yesNo(shared.connected), true); y += 34;

    const auto transportText = shared.transportAvailable
        ? (shared.transportPlaying ? "PLAY" : "STOP")
        : "НЕДОСТУПЕН";
    drawRow(g, y, "Транспорт", transportText, true); y += 25;
    drawRow(g, y, "PPQ", shared.transportAvailable ? juce::String(shared.transportPpq, 3) : "-"); y += 25;
    drawRow(g, y, "Секунды", shared.transportAvailable ? juce::String(shared.transportSeconds, 3) : "-"); y += 34;

    drawRow(g, y, "Тональность", keyDisplayName(timeline.globalKey), true); y += 25;
    drawRow(g, y, "Предыдущий аккорд",
            timeline.previousChordAvailable ? chordDisplayName(timeline.previousChord) : "-"); y += 25;
    drawRow(g, y, "Текущий аккорд", chordDisplayName(timeline.currentChord), true); y += 25;
    drawRow(g, y, "Следующий аккорд",
            timeline.nextChordAvailable ? chordDisplayName(timeline.nextChord) : "-"); y += 25;

    juce::String timeSignature = "-";
    if (context.timeSignature.available)
    {
        timeSignature = juce::String(context.timeSignature.numerator)
                      + "/"
                      + juce::String(context.timeSignature.denominator);
    }
    drawRow(g, y, "Размер", timeSignature); y += 25;

    const auto bpm = localBpm(shared, ppq);
    drawRow(g, y, "Темп", bpm > 0.0 ? juce::String(bpm, 2) + " BPM" : "-"); y += 34;

    const auto counts = "Тональность " + juce::String(shared.keySignatureEventCount)
                      + "  |  Аккорды " + juce::String(shared.sheetChordEventCount)
                      + "  |  Темп " + juce::String(shared.tempoEntryEventCount)
                      + "  |  Такты " + juce::String(shared.barSignatureEventCount);
    drawRow(g, y, "ARA-события", counts); y += 25;
    drawRow(g, y, "Ревизии",
            "гармония " + juce::String(static_cast<juce::int64>(shared.revision))
            + "  |  транспорт " + juce::String(static_cast<juce::int64>(shared.transportRevision))); y += 36;

    g.setColour(juce::Colour::fromRGB(77, 81, 89));
    g.drawHorizontalLine(y, 24.0f, 616.0f); y += 12;

    g.setColour(juce::Colour::fromRGB(190, 195, 205));
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText("STAGE 2 — HARMONIC ENGINE", 24, y, 590, 22, juce::Justification::centredLeft); y += 28;

    drawRow(g, y, "Ситуация", situation.valid ? "ВАЛИДНА" : "НЕТ АНАЛИЗА", true); y += 24;
    drawRow(g, y, "Глобальная функция", harmonicDisplay(situation.harmonic), true); y += 24;

    drawRow(g, y, "Связь",
            situation.valid && situation.harmonic.valid
                ? juce::String(harmonicRelationNameRu(situation.harmonic.relation))
                : juce::String("-")); y += 24;

    drawRow(g, y, "Глобальный паттерн",
            situation.valid ? juce::String(patternName(situation.pattern.type)) : "-"); y += 24;
    drawRow(g, y, "Позиция паттерна", patternPositionDisplay(situation.pattern)); y += 24;
    drawRow(g, y, "Разрешение", resolutionDisplay(situation)); y += 24;

    g.setColour(juce::Colour::fromRGB(77, 81, 89));
    g.drawHorizontalLine(y, 24.0f, 616.0f); y += 10;

    drawRow(g, y, "Локальный центр", localCenterDisplayName(situation.localKey), true); y += 24;
    drawRow(g, y, "Локальная функция", harmonicDisplay(situation.localHarmonic), true); y += 24;
    drawRow(g, y, "Локальный паттерн",
            situation.localPattern.recognized()
                ? juce::String(patternName(situation.localPattern.type))
                : juce::String("-")); y += 24;
    drawRow(g, y, "Локальная позиция", patternPositionDisplay(situation.localPattern)); y += 24;

    juce::String localConfidence = "-";
    if (situation.localKey.valid)
    {
        localConfidence = confidenceName(situation.localKey.evidence.confidence);
        localConfidence += "  |  ";
        localConfidence += interpretationName(situation.localKey.evidence.interpretation);
    }
    drawRow(g, y, "Локальная уверенность", localConfidence); y += 28;

    g.setColour(juce::Colour::fromRGB(77, 81, 89));
    g.drawHorizontalLine(y, 24.0f, 616.0f); y += 10;

    juce::String confidence = "-";
    if (situation.valid)
    {
        confidence = confidenceName(situation.evidence.confidence);
        confidence += "  |  ";
        confidence += interpretationName(situation.evidence.interpretation);
        confidence += "  |  кандидатов " + juce::String(situation.interpretationCount);
    }
    drawRow(g, y, "Интерпретация", confidence, true); y += 24;

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
        primary = "НЕ ОПРЕДЕЛЕНА";
    }
    drawRow(g, y, "Основная", primary, true); y += 24;

    for (std::uint8_t i = 0; i < situation.interpretationCount; ++i)
    {
        drawRow(g,
                y,
                "Кандидат " + juce::String(static_cast<int>(i) + 1),
                interpretationDisplay(situation.interpretations[i]));
        y += 24;
    }

    g.setColour(juce::Colour::fromRGB(105, 110, 120));
    g.setFont(12.5f);
    g.drawText("Диагностический интерфейс для проверки Stage 1/2/3. Продуктовый интерфейс будет разработан позже.",
               24, getHeight() - 28, getWidth() - 48, 20,
               juce::Justification::centredLeft);
}