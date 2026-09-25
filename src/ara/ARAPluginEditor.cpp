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
#include <vector>

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
    text = text.replace("Waiting for valid position, chord and key context.",
                        ru("Ожидание корректной позиции, аккорда и тональности."));
    text = text.replace("No explicit chord tones available.",
                        ru("Нет доступных явно заданных звуков аккорда."));

    text = text.replace("No primary interpretation: use the explicit chord tones.",
                        ru("Нет основной трактовки: используй явно заданные звуки аккорда."));
    text = text.replace("No harmonic interpretation: use the explicit chord tones.",
                        ru("Нет гармонической трактовки: используй явно заданные звуки аккорда."));
    text = text.replace("This chord needs a dedicated source rule; retain its explicit tones.",
                        ru("Для этого аккорда нужен отдельный источник; сохраняй явно заданные звуки."));
    text = text.replace("No compatible SubV source; retain the confirmed targets.",
                        ru("Нет совместимого источника для SubV; сохраняй подтверждённые цели."));
    text = text.replace("No compatible minor-target source; retain anchors and resolution.",
                        ru("Нет совместимого источника для минорной цели; сохраняй опоры и разрешение."));
    text = text.replace("No basic diatonic source for this minor-target dominant; retain anchors and contextual alternatives.",
                        ru("Нет базового диатонического источника для доминанты в минор; сохраняй опоры и контекстные альтернативы."));
    text = text.replace("No confirmed major target for the basic dominant source.",
                        ru("Нет подтверждённой мажорной цели для базового доминантового источника."));
    text = text.replace("No supported major/minor center in the selected interpretation.",
                        ru("В выбранной трактовке нет поддерживаемого мажорного/минорного центра."));
    text = text.replace("No compatible diatonic source for all explicit chord tones/degrees.",
                        ru("Нет диатонического источника, совместимого со всеми заданными звуками и ступенями аккорда."));
    text = text.replace("No compatible diatonic source for the available harmonic interpretations.",
                        ru("Нет совместимого диатонического источника для доступных гармонических трактовок."));

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
        case HarmonicFunction::predominant: return ru("Субдоминанта");
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

    return utf8String(fifthsName(key.rootFifths)) + " " + keyModeNameRu(key.mode);
}

juce::String patternName(smartimproviser::harmony::HarmonicPatternType type)
{
    using smartimproviser::harmony::HarmonicPatternType;
    switch (type)
    {
        case HarmonicPatternType::none: return ru("Нет");
        case HarmonicPatternType::majorIiVI: return ru("Мажорный ii-V-I");
        case HarmonicPatternType::minorIiHalfDimVi: return ru("Минорный iiø-V-i");
        case HarmonicPatternType::minorIvVi: return ru("Минорный iv-V-i");
        case HarmonicPatternType::dominantToTonic: return "V-I";
        case HarmonicPatternType::turnaroundIVIiiV: return "I-VI-ii-V";
        case HarmonicPatternType::majorCadentialChain: return ru("Расширенный каданс iii-VI7-ii-V-I");
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
        case PatternMemberRole::predominant: return ru("Субдоминанта");
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

    return utf8String(fifthsName(center.key.rootFifths)) + " " + keyModeNameRu(center.key.mode);
}

juce::String localCenterDisplayName(const smartimproviser::harmony::KeyCenter& center)
{
    if (! center.valid || ! center.key.valid)
        return "-";

    return centerKeyDisplayName(center)
         + "  |  " + keyCenterScopeName(center.scope)
         + "  |  " + keyCenterStatusName(center.status);
}

juce::String harmonicDisplay(const smartimproviser::harmony::HarmonicAnalysis& harmonic)
{
    if (! harmonic.valid)
        return "-";

    return juce::String(smartimproviser::harmony::scaleDegreeName(harmonic.rootScaleDegree))
         + "  |  " + harmonicFunctionNameRu(harmonic.effectiveFunction);
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

juce::String notesText(const std::vector<smartimproviser::harmony::MaterialNote>& notes)
{
    juce::String text;
    for (const auto& note : notes)
        text += juce::String(pitchClassName(note.pitchClass)) + " ";
    return text.isEmpty() ? ru("Нет") : text;
}

juce::String moveText(const smartimproviser::harmony::ResolutionMove& move)
{
    return juce::String(pitchClassName(move.fromPitchClass)) + " -> "
         + pitchClassName(move.toPitchClass) + "  ";
}
}

SmartImproviserARAEditor::SmartImproviserARAEditor(SmartImproviserARAProcessor& p)
    : juce::AudioProcessorEditor(p), processor(p)
{
    setSize(1020, 760);

    detailsView.setMultiLine(true, true);
    detailsView.setReadOnly(true);
    detailsView.setScrollbarsShown(true);
    detailsView.setCaretVisible(false);
    detailsView.setFont(juce::Font(juce::FontOptions(15.0f)));
    detailsView.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromRGB(42, 46, 53));
    detailsView.setColour(juce::TextEditor::textColourId, juce::Colour::fromRGB(225, 230, 238));
    detailsView.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    detailsView.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(detailsView);

    for (auto* button : { &materialButton, &sourcesButton, &harmonicButton, &araButton })
    {
        button->setMouseCursor(juce::MouseCursor::PointingHandCursor);
        button->setColour(juce::TextButton::textColourOffId, juce::Colour::fromRGB(225, 230, 238));
        addAndMakeVisible(*button);
    }

    materialButton.onClick = [this] { setActivePanel(Panel::material); };
    sourcesButton.onClick = [this] { setActivePanel(Panel::sources); };
    harmonicButton.onClick = [this] { setActivePanel(Panel::harmonic); };
    araButton.onClick = [this] { setActivePanel(Panel::ara); };

    updatePanelButtons();
    resized();
    timerCallback();
    startTimerHz(10);
}

void SmartImproviserARAEditor::resized()
{
    const int margin = 24;
    const int gap = 8;
    const int buttonY = 244;
    const int buttonH = 36;
    const int available = getWidth() - margin * 2 - gap * 3;
    const int buttonW = available / 4;

    materialButton.setBounds(margin, buttonY, buttonW, buttonH);
    sourcesButton.setBounds(margin + (buttonW + gap), buttonY, buttonW, buttonH);
    harmonicButton.setBounds(margin + 2 * (buttonW + gap), buttonY, buttonW, buttonH);
    araButton.setBounds(margin + 3 * (buttonW + gap), buttonY,
                        getWidth() - margin - (margin + 3 * (buttonW + gap)), buttonH);

    detailsView.setBounds(margin, 292, getWidth() - margin * 2, getHeight() - 334);
}

void SmartImproviserARAEditor::setActivePanel(Panel panel)
{
    if (activePanel == panel)
        return;

    activePanel = panel;
    updatePanelButtons();
    refreshPanelView(true);
    repaint();
}

void SmartImproviserARAEditor::updatePanelButtons()
{
    const auto selectedColour = juce::Colour::fromRGB(65, 79, 96);
    const auto normalColour = juce::Colour::fromRGB(45, 49, 56);

    const auto configure = [&](juce::TextButton& button,
                               Panel panel,
                               const char* openText,
                               const char* closedText)
    {
        const bool selected = activePanel == panel;
        button.setButtonText(ru(selected ? openText : closedText));
        button.setColour(juce::TextButton::buttonColourId, selected ? selectedColour : normalColour);
    };

    configure(materialButton, Panel::material,
              "▼ Материал", "▶ Материал");
    configure(sourcesButton, Panel::sources,
              "▼ Источники / ноты", "▶ Источники / ноты");
    configure(harmonicButton, Panel::harmonic,
              "▼ Гармонический анализ", "▶ Гармонический анализ");
    configure(araButton, Panel::ara,
              "▼ ARA / диагностика", "▶ ARA / диагностика");
}

void SmartImproviserARAEditor::refreshPanelView(bool resetScroll)
{
    const juce::String* text = &materialText;
    switch (activePanel)
    {
        case Panel::material: text = &materialText; break;
        case Panel::sources: text = &sourcesText; break;
        case Panel::harmonic: text = &harmonicText; break;
        case Panel::ara: text = &araText; break;
    }

    if (detailsView.getText() != *text)
    {
        detailsView.setText(*text, false);
        if (resetScroll)
            detailsView.moveCaretToTop(false);
    }
}

void SmartImproviserARAEditor::timerCallback()
{
    cachedShared = SharedHarmonicContextBridge::instance().read();
    const auto ppq = cachedShared.transportAvailable ? cachedShared.transportPpq : -1.0;
    const auto timeline = smartimproviser::harmony::mapTimelineHarmonicSnapshot(cachedShared, ppq);
    const auto patternWindow = smartimproviser::harmony::mapPatternTimelineWindow(cachedShared, ppq);
    const auto context = smartimproviser::harmony::mapHarmonicContext(cachedShared, ppq);
    cachedSituation = smartimproviser::harmony::analyzeHarmonicSituation(timeline, patternWindow);
    const auto result = smartimproviser::harmony::analyzeImprovisation(cachedSituation);
    const auto debug = ARAContextDebugState::instance().getSnapshot();

    summaryContext = chordDisplayName(timeline.currentChord);
    if (timeline.nextChordAvailable)
        summaryContext += " -> " + chordDisplayName(timeline.nextChord);

    summaryMeta = ru("Тональность: ") + keyDisplayName(timeline.globalKey);
    if (cachedSituation.harmonic.valid)
        summaryMeta += ru("   •   Функция: ")
            + harmonicFunctionNameRu(cachedSituation.harmonic.effectiveFunction);
    if (cachedSituation.localKey.valid)
        summaryMeta += ru("   •   Локальный центр: ") + centerKeyDisplayName(cachedSituation.localKey);

    const auto* activePattern = &cachedSituation.pattern;
    if (cachedSituation.localPattern.recognized())
        activePattern = &cachedSituation.localPattern;

    summaryPattern = ru("Оборот: ");
    if (activePattern->recognized())
    {
        summaryPattern += patternName(activePattern->type);
        if (activePattern->positionIndex >= 0 && activePattern->length > 0)
            summaryPattern += ru("   •   ") + juce::String(activePattern->positionIndex + 1)
                + " / " + juce::String(activePattern->length);
    }
    else
    {
        summaryPattern += ru("не распознан");
    }

    summaryThinking = ru("Мышление: ");
    if (! result.valid || result.strategies.empty())
    {
        summaryThinking += ru("—");
    }
    else if (cachedSituation.primaryInterpretationIndex < 0
             && cachedSituation.evidence.interpretation
                == smartimproviser::harmony::InterpretationStatus::ambiguous)
    {
        summaryThinking += ru("неоднозначно • кандидатов: ")
            + juce::String(cachedSituation.interpretationCount)
            + ru(" • см. Источники / ноты");
    }
    else
    {
        juce::StringArray thoughts;
        const auto primaryIndex = cachedSituation.primaryInterpretationIndex;
        for (const auto& strategy : result.strategies)
        {
            if (strategy.source.kind != smartimproviser::harmony::MaterialKind::scale)
                continue;
            if (! strategy.interpretationIndependent
                && primaryIndex >= 0
                && strategy.interpretationIndex != primaryIndex)
                continue;

            juce::String item;
            if (strategy.source.mode != smartimproviser::harmony::DiatonicMode::none)
                item = utf8String(strategy.source.name);
            else if (strategy.thinkingStructure.valid)
                item = utf8String(smartimproviser::harmony::normalizedChordSymbol(strategy.thinkingStructure));
            else
                item = utf8String(strategy.source.name);

            if (item.isNotEmpty())
                thoughts.addIfNotAlreadyThere(item);
        }

        if (thoughts.isEmpty())
            thoughts.add(ru("звуки аккорда + направляющие тоны"));

        summaryThinking += thoughts.joinIntoString(ru(" • "));

        const auto& strategy = result.strategies.front();
        if (strategy.resolution.available && strategy.resolution.confirmed
            && strategy.resolution.moveCount > 0)
        {
            summaryThinking += ru("   →   ");
            for (std::size_t i = 0; i < strategy.resolution.moveCount; ++i)
            {
                if (i > 0)
                    summaryThinking += "  ";
                summaryThinking += juce::String(pitchClassName(strategy.resolution.moves[i].fromPitchClass))
                    + "->" + pitchClassName(strategy.resolution.moves[i].toPitchClass);
            }
            if (strategy.resolution.targetChord.valid)
                summaryThinking += ru(" в ")
                    + utf8String(smartimproviser::harmony::normalizedChordSymbol(strategy.resolution.targetChord));
        }
    }

    materialText.clear();
    materialText += ru("МАТЕРИАЛ ДЛЯ ИМПРОВИЗАЦИИ\n\n");
    if (! result.valid)
    {
        materialText += localizeGeneratedText(utf8String(result.unavailableReason));
    }
    else
    {
        materialText += localizeGeneratedText(
            utf8String(smartimproviser::harmony::harmonicConceptsText(result)));
    }

    sourcesText.clear();
    sourcesText += ru("ИСТОЧНИКИ И НОТЫ\n\n");
    if (! result.valid || result.strategies.empty())
    {
        sourcesText += result.valid
            ? ru("Нет доступного материала.")
            : localizeGeneratedText(utf8String(result.unavailableReason));
    }
    else
    {
        const auto& strategy = result.strategies.front();
        bool hasScale = false;
        int lastInterpretationIndex = -999;
        for (const auto& scalar : result.strategies)
        {
            if (scalar.source.kind != smartimproviser::harmony::MaterialKind::scale)
                continue;

            if (hasScale)
                sourcesText += "\n\n";
            hasScale = true;

            if (scalar.interpretationIndex != lastInterpretationIndex)
            {
                lastInterpretationIndex = scalar.interpretationIndex;
                if (scalar.interpretationIndex >= 0
                    && scalar.interpretationIndex < cachedSituation.interpretationCount)
                {
                    const auto& interpretation = cachedSituation.interpretations[
                        static_cast<std::size_t>(scalar.interpretationIndex)];
                    if (cachedSituation.primaryInterpretationIndex < 0)
                    {
                        sourcesText += ru("КАНДИДАТ ")
                            + juce::String(scalar.interpretationIndex + 1) + ": ";
                    }
                    else if (scalar.interpretationIndex == cachedSituation.primaryInterpretationIndex)
                    {
                        sourcesText += ru("ОСНОВНАЯ ТРАКТОВКА: ");
                    }
                    else
                    {
                        sourcesText += ru("АЛЬТЕРНАТИВА: ");
                    }
                    sourcesText += interpretationKindNameRu(interpretation.kind)
                        + " • " + centerKeyDisplayName(interpretation.center) + "\n";
                }
            }

            sourcesText += localizeGeneratedText(utf8String(scalar.source.name)) + "\n";
            for (const auto& note : scalar.source.notes)
                sourcesText += utf8String(note.spelling) + " ";

            if (! scalar.sourceReference.empty())
            {
                sourcesText += "\n" + localizeGeneratedText(utf8String(scalar.idea));
                sourcesText += ru("\nМыслить: ")
                    + utf8String(smartimproviser::harmony::normalizedChordSymbol(scalar.thinkingStructure));
                sourcesText += ru("\nОтносительно аккорда: ");
                for (const auto& note : scalar.source.chordRelativeNotes)
                    sourcesText += utf8String(note.spelling) + " ";
            }

            sourcesText += "\n" + localizeGeneratedText(utf8String(scalar.usageHint));
            if (! scalar.sourceTransitions.empty())
            {
                sourcesText += ru("\nНеобязательные движения красок: ");
                for (const auto& move : scalar.sourceTransitions)
                    sourcesText += juce::String(pitchClassName(move.fromPitchClass)) + "->"
                        + pitchClassName(move.toPitchClass) + " ";
            }
        }

        if (! hasScale)
            sourcesText += localizeGeneratedText(utf8String(result.scaleUnavailableReason));

        sourcesText += ru("\n\nОПОРНЫЕ НОТЫ АККОРДА\n");
        for (const auto& note : strategy.source.notes)
            sourcesText += juce::String(pitchClassName(note.pitchClass)) + " ";
        sourcesText += ru("\n(классы высот)");

        sourcesText += ru("\n\nНАПРАВЛЯЮЩИЕ ТОНЫ (3 / 7)\n") + notesText(strategy.guideNotes);
        sourcesText += ru("\n\nХАРАКТЕРНЫЕ НОТЫ\n") + notesText(strategy.characteristicNotes);
        sourcesText += ru("\n\nЦЕЛИ СЛЕДУЮЩЕГО АККОРДА\n");
        if (strategy.nextChord.valid)
            sourcesText += utf8String(smartimproviser::harmony::normalizedChordSymbol(strategy.nextChord))
                + ": " + notesText(strategy.targetNotes);
        else
            sourcesText += ru("Нет следующего аккорда");

        const bool confirmed = strategy.resolution.available && strategy.resolution.confirmed;
        sourcesText += confirmed
            ? ru("\n\nПОДТВЕРЖДЁННОЕ РАЗРЕШЕНИЕ\n")
            : ru("\n\nПРЕДЛОЖЕННЫЕ СВЯЗКИ\n");

        if (confirmed)
        {
            sourcesText += utf8String(
                smartimproviser::harmony::normalizedChordSymbol(strategy.resolution.targetChord)) + "\n";
            for (std::size_t i = 0; i < strategy.resolution.moveCount; ++i)
                sourcesText += moveText(strategy.resolution.moves[i]);
            if (strategy.resolution.moveCount == 0)
                sourcesText += ru("Нет доступных структурных движений");
        }
        else
        {
            for (const auto& move : strategy.suggestedTransitions)
                sourcesText += moveText(move);
            if (strategy.suggestedTransitions.empty())
                sourcesText += ru("Нет");
            sourcesText += ru("\n(не подтверждённое гармоническое разрешение)");
        }
    }

    harmonicText.clear();
    harmonicText += ru("ГАРМОНИЧЕСКИЙ АНАЛИЗ\n\n");
    harmonicText += ru("Глобальная функция: ") + harmonicDisplay(cachedSituation.harmonic) + "\n";
    harmonicText += ru("Связь: ")
        + (cachedSituation.valid && cachedSituation.harmonic.valid
            ? harmonicRelationNameRu(cachedSituation.harmonic.relation)
            : juce::String("-")) + "\n";
    harmonicText += ru("Глобальный оборот: ")
        + (cachedSituation.valid ? patternName(cachedSituation.pattern.type) : juce::String("-")) + "\n";
    harmonicText += ru("Позиция в обороте: ") + patternPositionDisplay(cachedSituation.pattern) + "\n";
    harmonicText += ru("Разрешение: ") + resolutionDisplay(cachedSituation) + "\n\n";

    harmonicText += ru("ЛОКАЛЬНЫЙ КОНТЕКСТ\n");
    harmonicText += ru("Центр: ") + localCenterDisplayName(cachedSituation.localKey) + "\n";
    harmonicText += ru("Функция: ") + harmonicDisplay(cachedSituation.localHarmonic) + "\n";
    harmonicText += ru("Оборот: ")
        + (cachedSituation.localPattern.recognized()
            ? patternName(cachedSituation.localPattern.type)
            : juce::String("-")) + "\n";
    harmonicText += ru("Позиция: ") + patternPositionDisplay(cachedSituation.localPattern) + "\n";

    juce::String localConfidence = "-";
    if (cachedSituation.localKey.valid)
    {
        localConfidence = confidenceName(cachedSituation.localKey.evidence.confidence);
        localConfidence += "  |  ";
        localConfidence += interpretationName(cachedSituation.localKey.evidence.interpretation);
    }
    harmonicText += ru("Уверенность: ") + localConfidence + "\n\n";

    juce::String confidence = "-";
    if (cachedSituation.valid)
    {
        confidence = confidenceName(cachedSituation.evidence.confidence);
        confidence += "  |  ";
        confidence += interpretationName(cachedSituation.evidence.interpretation);
        confidence += ru("  |  кандидатов ") + juce::String(cachedSituation.interpretationCount);
    }
    harmonicText += ru("ТРАКТОВКА\nСтатус: ") + confidence + "\n";

    juce::String primary = "-";
    if (cachedSituation.primaryInterpretationIndex >= 0
        && cachedSituation.primaryInterpretationIndex < cachedSituation.interpretationCount)
    {
        primary = interpretationKindNameRu(
            cachedSituation.interpretations[
                static_cast<std::size_t>(cachedSituation.primaryInterpretationIndex)].kind);
    }
    else if (cachedSituation.valid
             && cachedSituation.evidence.interpretation
                == smartimproviser::harmony::InterpretationStatus::ambiguous)
    {
        primary = ru("НЕ ОПРЕДЕЛЕНА");
    }
    harmonicText += ru("Основная: ") + primary + "\n";

    for (std::uint8_t i = 0; i < cachedSituation.interpretationCount; ++i)
    {
        harmonicText += ru("Кандидат ") + juce::String(static_cast<int>(i) + 1)
            + ": " + interpretationDisplay(cachedSituation.interpretations[i]) + "\n";
    }

    araText.clear();
    araText += ru("ARA / ТЕХНИЧЕСКАЯ ДИАГНОСТИКА\n\n");
    araText += ru("ARA-подключение: ")
        + (processor.isAraBound() ? ru("ПОДКЛЮЧЕНО") : ru("НЕ ПОДКЛЮЧЕНО")) + "\n";
    araText += ru("Контроллер документа: ") + yesNo(debug.documentControllerCreated) + "\n";
    araText += ru("Доступ к данным хоста: ") + yesNo(debug.hostContentAccessAvailable) + "\n";
    araText += ru("Музыкальные контексты: ") + juce::String(debug.musicalContextCount);
    if (debug.selectedMusicalContextIndex >= 0)
        araText += ru("  |  выбран ") + juce::String(debug.selectedMusicalContextIndex + 1);
    araText += "\n";
    araText += ru("Общий контекст: ") + yesNo(cachedShared.connected) + "\n\n";

    juce::String transportText;
    if (! cachedShared.transportAvailable)
        transportText = ru("НЕДОСТУПЕН");
    else
        transportText = cachedShared.transportPlaying ? "PLAY" : "STOP";
    araText += ru("Транспорт: ") + transportText + "\n";
    araText += "PPQ: " + (cachedShared.transportAvailable
        ? juce::String(cachedShared.transportPpq, 3) : juce::String("-")) + "\n";
    araText += ru("Секунды: ") + (cachedShared.transportAvailable
        ? juce::String(cachedShared.transportSeconds, 3) : juce::String("-")) + "\n\n";

    araText += ru("Тональность: ") + keyDisplayName(timeline.globalKey) + "\n";
    araText += ru("Предыдущий аккорд: ")
        + (timeline.previousChordAvailable ? chordDisplayName(timeline.previousChord) : juce::String("-")) + "\n";
    araText += ru("Текущий аккорд: ") + chordDisplayName(timeline.currentChord) + "\n";
    araText += ru("Следующий аккорд: ")
        + (timeline.nextChordAvailable ? chordDisplayName(timeline.nextChord) : juce::String("-")) + "\n";

    juce::String timeSignature = "-";
    if (context.timeSignature.available)
        timeSignature = juce::String(context.timeSignature.numerator)
                      + "/" + juce::String(context.timeSignature.denominator);
    araText += ru("Размер: ") + timeSignature + "\n";

    const auto bpm = localBpm(cachedShared, ppq);
    araText += ru("Темп: ") + (bpm > 0.0 ? juce::String(bpm, 2) + " BPM" : juce::String("-")) + "\n\n";

    araText += ru("ARA-события: Тональность ") + juce::String(cachedShared.keySignatureEventCount)
        + ru("  |  Аккорды ") + juce::String(cachedShared.sheetChordEventCount)
        + ru("  |  Темп ") + juce::String(cachedShared.tempoEntryEventCount)
        + ru("  |  Такты ") + juce::String(cachedShared.barSignatureEventCount) + "\n";
    araText += ru("Изменения: гармония ")
        + juce::String(static_cast<juce::int64>(cachedShared.revision))
        + ru("  |  транспорт ")
        + juce::String(static_cast<juce::int64>(cachedShared.transportRevision));

    refreshPanelView(false);
    repaint();
}

void SmartImproviserARAEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(31, 33, 37));

    g.setColour(juce::Colour::fromRGB(245, 246, 248));
    g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    g.drawText("Smart Improviser " SMART_IMPROVISER_BUILD_VERSION,
               24, 16, getWidth() - 48, 30, juce::Justification::centredLeft);

    g.setColour(juce::Colour::fromRGB(150, 156, 168));
    g.setFont(14.0f);
    g.drawText(ru("0.3f fix1 • непрерывность кадансового контекста"),
               24, 47, getWidth() - 48, 22, juce::Justification::centredLeft);

    g.setColour(juce::Colour::fromRGB(42, 46, 53));
    g.fillRoundedRectangle(24.0f, 82.0f, static_cast<float>(getWidth() - 48), 146.0f, 8.0f);

    g.setColour(juce::Colour::fromRGB(150, 156, 168));
    g.setFont(12.5f);
    g.drawText(ru("ТЕКУЩИЙ КОНТЕКСТ"), 40, 92, getWidth() - 80, 18,
               juce::Justification::centredLeft);

    g.setColour(juce::Colour::fromRGB(240, 243, 247));
    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.drawText(summaryContext, 40, 111, getWidth() - 80, 23,
               juce::Justification::centredLeft, true);

    g.setColour(juce::Colour::fromRGB(190, 196, 207));
    g.setFont(13.2f);
    g.drawText(summaryMeta, 40, 136, getWidth() - 80, 18,
               juce::Justification::centredLeft, true);
    g.drawText(summaryPattern, 40, 157, getWidth() - 80, 18,
               juce::Justification::centredLeft, true);

    g.setColour(juce::Colour::fromRGB(220, 225, 234));
    g.setFont(13.5f);
    g.drawFittedText(summaryThinking, 40, 180, getWidth() - 80, 38,
                     juce::Justification::topLeft, 2, 0.86f);

    g.setColour(juce::Colour::fromRGB(105, 110, 120));
    g.setFont(12.0f);
    g.drawText(ru("Диагностический UI: основной материал открыт первым; подробности вынесены в отдельные сворачиваемые разделы."),
               24, getHeight() - 28, getWidth() - 48, 20,
               juce::Justification::centredLeft);
}
