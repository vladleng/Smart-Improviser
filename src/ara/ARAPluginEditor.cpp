#include "core/analysis/MajorIiVPalette.h"
#include "core/analysis/HarmonicConcepts.h"
#include "ara/ARAPluginEditor.h"
#include "ara/ARAPluginProcessor.h"
#include "ara/ARAContextDebugState.h"
#include "context/SharedHarmonicContext.h"
#include "context/TimelineContextMapper.h"
#include "core/analysis/HarmonicEngine.h"
#include "core/analysis/ImprovisationEngine.h"
#include "core/analysis/ManualTensionKey.h"
#include "core/analysis/MaterialSelection.h"
#include "core/analysis/TensionEngine.h"
#include "core/model/ChordModel.h"
#include "core/model/KeyModel.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
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
    text = text.replace("Minor destination confirmed; sources follow its quality independently of the preceding chord.",
                        ru("Минорная цель подтверждена; набор источников определяется ею независимо от предыдущего аккорда."));
    text = text.replace("Hypothetical minor destination from tonal context; the actual next chord remains separate and no local tonic is established.",
                        ru("Минорная цель предполагается по тональному контексту; фактический следующий аккорд учитывается отдельно, локальная тоника не устанавливается."));
    text = text.replace("Diminished seventh arpeggio from bII of the dominant", ru("Уменьшённое арпеджио от ♭II доминанты"));
    text = text.replace("Four-note b9-3-5-b7 line. Keep the written dominant root and b13 as accompaniment anchors outside this arpeggio; no natural 13 is added.",
                        ru("Четыре звука: b9–3–5–b7. Корень доминанты и записанная b13 сохраняются в аккомпанементе вне арпеджио; натуральная 13 не добавляется."));
    text = text.replace("Hypothetical minor I from tonal context, not a played tonic or established local key.",
                        ru("Предполагаемая минорная i следует из тонального контекста; она не прозвучала и не устанавливает локальную тональность."));
    text = text.replace("Presumed minor destination follows the turn and tonal context; the actual next chord remains the target. Use 11 as passing against the dominant third; sustain b13 only when melody supports it.",
                        ru("Минорная цель предполагается по обороту и тональному контексту; реальная цель нот — следующий записанный аккорд. 11 проходящая против терции доминанты; удерживай b13 при поддержке мелодии."));
    text = text.replace("Harmonic minor from the turn's minor destination", ru("Гармонический минор от предполагаемой минорной цели оборота"));
    text = text.replace("Whole-half on a diminished triad or dim7; follow the actual next chord.",
                        ru("Уменьшённая тон–полутон для уменьшённого трезвучия или dim7; следуй реальному следующему аккорду."));
    text = text.replace("Waiting for valid position, chord and key context.",
                        ru("Ожидание корректной позиции, аккорда и тональности."));
    text = text.replace("No explicit chord tones available.",
                        ru("Нет доступных явно заданных звуков аккорда."));
    text = text.replace("T1: use the written chord tones and available guide tones.",
                        ru("T1: звуки записанного аккорда и его направляющие."));
    text = text.replace("T1: written chord anchors with the natural 9 from the confirmed diatonic source.",
                        ru("T1: опоры аккорда с натуральной 9 из доступного диатонического источника."));
    text = text.replace("T1: play the four-note m6 thinking structure over the written dominant.",
                        ru("T1: четыре звука m6-структуры над записанной доминантой."));
    text = text.replace("The written chord, including any alteration or slash bass, remains the harmony; no scale or tonic is inferred.",
                        ru("Записанный аккорд, включая альтерации и бас, остаётся гармонией; гамма и тоника не выводятся."));
    text = text.replace("Only the 9 is added; the natural 4th against a major third is passing, never a stable landing note.",
                        ru("Добавлена только 9; натуральная 4 против большой терции допускается как проходящая, не как устойчивая цель."));
    text = text.replace("The dominant root and any written tones remain harmonic anchors outside this m6 line. The full melodic-minor scale, especially #11, is not assigned T1.",
                        ru("Корень доминанты и записанные звуки остаются опорами вне линии m6. Полная гамма melodic minor, особенно #11, не объявлена T1."));
    text = text.replace("T1 chord anchors; connect to actual next-chord targets only when known.",
                        ru("T1: опоры аккорда; направляй к действительной цели, если она известна."));
    text = text.replace("Natural 9 is a stable extension; a major chord's 4th is passing only.",
                        ru("Натуральная 9 — устойчивая надстройка; 4 мажорного аккорда только проходящая."));
    text = text.replace("m6 structure only; optional diatonic passing note is not a stable target.",
                        ru("Только m6-структура; дополнительный диатонический проходящий звук не является устойчивой целью."));

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
    text = text.replace("Candidate major ii-V: the expected I has not been observed; no next-chord target is claimed.",
                        ru("Кандидат мажорного ii–V: ожидаемая I не прозвучала; цель следующего аккорда не утверждается."));
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

    text = text.replace("Use with harmonic interpretation ", ru("Применяется в трактовке "));
    text = text.replace("; source root is not a song key.", ru("; корень источника не является тональностью песни."));
    text = text.replace("Incomplete ii-V: the expected major I did not sound; source root is not an established key.",
                        ru("Незавершённый ii–V: ожидаемая мажорная I не прозвучала; корень источника не является установленной тональностью."));
    text = text.replace("Chord-local optional color; no tonic, key or functional resolution is inferred.",
                        ru("Дополнительная краска от аккорда; тоника, тональность и функциональное разрешение не выводятся."));
    text = text.replace("Resolve to the actual major target (including its major third).",
                        ru("Разрешай в реальную мажорную цель, включая её большую терцию."));
    text = text.replace("Resolve to the actual minor target (including its minor third).",
                        ru("Разрешай в реальную минорную цель, включая её малую терцию."));
    text = text.replace("Optional m6 overlay: keep the written dominant third as an anchor outside this source; the source's b3 is a color, not a replacement third.",
                        ru("Дополнительное наложение m6: записанная терция доминанты остаётся опорой вне источника; малая терция источника — краска, а не замена терции аккорда."));
    text = text.replace("Scale notes are available material, not equally stable landing notes; use the chord anchors and targets.",
                        ru("Ноты гаммы — доступный материал, но не одинаково устойчивые опоры; используй аккордовые и целевые ноты."));
    text = text.replace(" Treat the natural 4th as a passing tone against the major 3rd.",
                        ru(" Натуральную 4 используй как проходящую против большой 3."));
    text = text.replace("The absent I is a source hypothesis only. Follow the actual next chord; no major resolution is confirmed.",
                        ru("Отсутствующая I — гипотеза источника. Следуй реальному следующему аккорду; мажорное разрешение не подтверждено."));
    text = text.replace("Optional material from explicit chord tones. Follow the actual next chord; a dominant chain is not a confirmed tonic resolution.",
                        ru("Дополнительный материал от записанного аккорда. Следуй реальному следующему аккорду; цепь доминант не подтверждает разрешение в тонику."));
    text = text.replace("Confirmed moves come from harmonic analysis; suggested moves are optional.",
                        ru("Подтверждённые движения взяты из гармонического анализа; предложенные движения необязательны."));

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
    text = text.replace("Dominant half-whole diminished", ru("Доминантовая уменьшённая полутон–тон"));
    text = text.replace("Dominant whole-tone", ru("Доминантовая целотоновая"));
    text = text.replace("Rare fifth-mode melodic-minor color", ru("Редкий цвет пятого лада мелодического минора"));
    text = text.replace("Rare conditional color on written b13: 11 and b13 can clash when sustained. It does not imply a minor next chord; follow the actual target.",
                        ru("Редкий условный цвет на записанной ♭13: удержанные 11 и ♭13 могут конфликтовать. Гамма сама не устанавливает минорную цель; ноты-цели берутся из фактического следующего аккорда."));
    text = text.replace("Chord-local Mixolydian on the written dominant.",
                        ru("Миксолидийский лад от написанной доминанты, без вывода тоники."));
    text = text.replace("Chord-local source; use written guides and actual next-chord targets.",
                        ru("Источник от аккорда: опирайся на записанные направляющие и фактический следующий аккорд."));
    text = text.replace("Use for an explicit augmented dominant; the natural fifth is absent.",
                        ru("Для увеличенной доминанты: натуральная квинта в источнике отсутствует."));
    text = text.replace("Optional b9/#9/#11 color on a compatible dominant; written natural 9 or b13 blocks this collection.",
                        ru("Необязательная краска ♭9/#9/#11; явно записанная натуральная 9 или ♭13 исключает этот набор."));
    text = text.replace("On m7 this is an optional overlay: keep the written b7 as an anchor and use the major 7 only as a passing color.",
                        ru("На m7 это дополнительное наложение: ♭7 остаётся опорой, большая 7 — проходящей краской."));
    text = text.replace("Native minor melodic sound on the written m6 or m(maj7).",
                        ru("Основной звук мелодического минора на записанном m6 или m(maj7)."));

    text = text.replace(" Ionian", ru(" ионийский"));
    text = text.replace(" Dorian", ru(" дорийский"));
    text = text.replace(" Phrygian", ru(" фригийский"));
    text = text.replace(" Lydian", ru(" лидийский"));
    text = text.replace(" Mixolydian", ru(" миксолидийский"));
    text = text.replace(" Aeolian", ru(" эолийский"));
    text = text.replace(" Locrian", ru(" локрийский"));
    text = text.replace(" melodic minor", ru(" мелодический минор"));
    text = text.replace(" whole-half diminished", ru(" уменьшённая (тон–полутон)"));
    text = text.replace(" half-whole diminished", ru(" уменьшённая (полутон–тон)"));
    text = text.replace(" whole-tone", ru(" целотоновая"));
    text = text.replace(" harmonic minor", ru(" гармонический минор"));
    text = text.replace("dim7 arpeggio", ru("°7 арпеджио"));

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
    text = text.replace("ПОЧЕМУ / КОНТЕКСТ", ru("ОБЪЯСНЕНИЕ / КОНТЕКСТ"));
    text = text.replace("GLOBAL: ", ru("Глобальная тональность: "));
    text = text.replace("LOCAL: ", ru("Локальный центр: "));
    text = text.replace("MODAL: ", ru("Модальная трактовка: "));
    text = text.replace("ИДЕЯ ", ru("ИДЕЯ "));
    text = text.replace("ИСТОЧНИК: ", ru("ИСТОЧНИК: "));
    text = text.replace("ВАЖНЫЕ НОТЫ: ", ru("ВАЖНЫЕ НОТЫ: "));
    text = text.replace("СЛЕДУЮЩИЙ АККОРД: ", ru("СЛЕДУЮЩИЙ АККОРД: "));
    text = text.replace("[OK]", ru("[подтверждено]"));
    text = text.replace("[EXPECTED]", ru("[ожидается]"));
    text = text.replace("[MISSING]", ru("[отсутствует]"));
    text = text.replace("[IMPLIED]", ru("[подразумевается]"));
    text = text.replace("[CONFLICT]", ru("[противоречие]"));
    text = text.replace("[AMBIGUOUS]", ru("[неоднозначно]"));
    text = text.replace("[INFO]", ru("[информация]"));
    text = text.replace("why.pattern-context", ru("гармонический оборот"));
    text = text.replace("why.nested-pattern", ru("вложенный оборот"));
    text = text.replace("why.confirmed-resolution", ru("подтверждённое разрешение"));
    text = text.replace("why.implied-dominant", ru("подразумеваемая доминанта"));
    text = text.replace("why.missing-tonic", ru("отсутствующая тоника"));
    text = text.replace("why.expected-tonic-not-played", ru("ожидаемая тоника отсутствует"));
    text = text.replace("why.actual-continuation-conflicts", ru("фактическое продолжение не совпало с ожидаемым"));
    text = text.replace("why.interpretation-ambiguous", ru("несколько допустимых трактовок"));
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
        case HarmonicPatternType::halfDiminishedIiViMajor: return ru("iiø–V–I в мажорную цель");
        case HarmonicPatternType::minorIvVi: return ru("Минорный iv-V-i");
        case HarmonicPatternType::dominantToTonic: return "V-I";
        case HarmonicPatternType::turnaroundIVIiiV: return "I-VI-ii-V";
        case HarmonicPatternType::majorIiiViIiV: return "iii-vi-ii-V";
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

// Compact upper-case Roman notation for the current-context line. The
// diagnostic panels keep their more descriptive pattern names and chord data.
juce::String summaryPatternName(smartimproviser::harmony::HarmonicPatternType type)
{
    using smartimproviser::harmony::HarmonicPatternType;
    switch (type)
    {
        case HarmonicPatternType::majorIiVI: return ru("II–V–I");
        case HarmonicPatternType::minorIiHalfDimVi: return ru("IIø–V–i");
        case HarmonicPatternType::halfDiminishedIiViMajor: return ru("IIø–V–Imaj");
        case HarmonicPatternType::minorIvVi: return ru("IV–V–I");
        case HarmonicPatternType::dominantToTonic: return ru("V–I");
        case HarmonicPatternType::turnaroundIVIiiV: return ru("I–VI–II–V");
        case HarmonicPatternType::majorIiiViIiV: return ru("III–VI–II–V");
        case HarmonicPatternType::majorCadentialChain: return ru("III–VI7–II–V–I");
        case HarmonicPatternType::minorIvToI: return ru("IV–I");
        default: return patternName(type);
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

juce::String pitchText(int pitch, const smartimproviser::harmony::NormalizedChord& chord,
                       const std::vector<smartimproviser::harmony::MaterialNote>& preferred = {})
{
    const auto found = std::find_if(preferred.begin(), preferred.end(), [pitch](const auto& note)
    { return note.pitchClass == pitch; });
    if (found != preferred.end())
        return utf8String(smartimproviser::harmony::spelledChordNote(chord, *found));

    smartimproviser::harmony::MaterialNote note;
    note.pitchClass = pitch;
    note.semitonesFromRoot = (pitch - chord.rootPitchClass + 12) % 12;
    note.degree = chord.degrees[static_cast<std::size_t>(note.semitonesFromRoot)];
    return utf8String(smartimproviser::harmony::spelledChordNote(chord, note));
}

juce::String notesText(const std::vector<smartimproviser::harmony::MaterialNote>& notes,
                       const smartimproviser::harmony::NormalizedChord& chord)
{
    juce::String text;
    for (const auto& note : notes)
        text += utf8String(smartimproviser::harmony::spelledChordNote(chord, note)) + " ";
    return text.isEmpty() ? ru("Нет") : text;
}

juce::String moveText(const smartimproviser::harmony::ResolutionMove& move,
                      const smartimproviser::harmony::ImprovisationStrategy& strategy)
{
    return pitchText(move.fromPitchClass, strategy.actualChord, strategy.source.chordRelativeNotes)
        + " -> " + pitchText(move.toPitchClass, strategy.nextChord, strategy.targetNotes) + "  ";
}

juce::String materialSelectionKey(const smartimproviser::harmony::ExplanationItem& item)
{
    juce::String key = utf8String(item.source.name) + "|" + utf8String(item.idea)
        + "|" + utf8String(item.conditions) + "|" + utf8String(item.usageHint);
    for (const int index : item.interpretationIndices) key += "|" + juce::String(index);
    for (const auto& note : item.source.notes)
        key += "|" + utf8String(note.spelling) + ":" + juce::String(note.pitchClass);
    return key;
}
}

SmartImproviserARAEditor::SmartImproviserARAEditor(SmartImproviserARAProcessor& p)
    : juce::AudioProcessorEditor(p), processor(p)
{
    setResizable(true, true);
    setResizeLimits(940, 1000, 1900, 1800);
    const auto size = processor.editorSize();
    setSize(size.x, size.y);

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

    for (auto* selector : { &layerSelector, &fretSelector, &tensionSelector })
    {
        selector->setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(45, 49, 56));
        selector->setColour(juce::ComboBox::textColourId, juce::Colour::fromRGB(225, 230, 238));
        selector->setColour(juce::ComboBox::outlineColourId, juce::Colour::fromRGB(85, 94, 105));
        addAndMakeVisible(*selector);
    }
    tensionSelector.addItem(ru("Все материалы"), 1);
    tensionSelector.addItem(ru("T1 · базовое"), 2);
    tensionSelector.addItem(ru("T2 · цвет"), 3);
    tensionSelector.addItem(ru("T3 · максимум"), 4);
    tensionSelector.setSelectedId(processor.tensionFilter() + 1, juce::dontSendNotification);
    tensionSelector.setTooltip(ru("Выбор по вашим меткам. Для назначения скрытых материалов выберите «Все материалы»."));
    tensionSelector.onChange = [this]
    {
        processor.setTensionFilter(tensionSelector.getSelectedId() - 1);
        selectedMaterialKey.clear();
        showStableSubset = false;
        timerCallback();
    };
    tensionHint.setFont(juce::FontOptions(13.0f));
    tensionHint.setColour(juce::Label::textColourId, juce::Colour::fromRGB(190, 201, 216));
    tensionHint.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(tensionHint);
    layerSelector.addItem(ru("Все роли"), 1);
    layerSelector.addItem(ru("Источник"), 2);
    layerSelector.addItem(ru("Аккорд"), 3);
    layerSelector.addItem(ru("Направляющие"), 4);
    layerSelector.addItem(ru("Характерные"), 5);
    layerSelector.addItem(ru("Цели следующего аккорда"), 6);
    layerSelector.setSelectedId(1, juce::dontSendNotification);
    fretSelector.addItem(ru("Лады 0–12"), 1);
    fretSelector.addItem(ru("Лады 5–17"), 2);
    fretSelector.addItem(ru("Лады 12–24"), 3);
    fretSelector.setSelectedId(1, juce::dontSendNotification);
    fretLabelButton.setColour(juce::TextButton::buttonColourId,
                              juce::Colour::fromRGB(45, 49, 56));
    fretLabelButton.setColour(juce::TextButton::textColourOffId,
                              juce::Colour::fromRGB(225, 230, 238));
    fretLabelButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    fretLabelButton.setButtonText(processor.fretDegreeLabelsEnabled()
        ? ru("Гриф: ступени") : ru("Гриф: ноты"));
    fretLabelButton.onClick = [this]
    {
        const bool enabled = !processor.fretDegreeLabelsEnabled();
        processor.setFretDegreeLabelsEnabled(enabled);
        fretLabelButton.setButtonText(enabled ? ru("Гриф: ступени") : ru("Гриф: ноты"));
        materialViewer.setFretDegreeLabels(enabled);
    };
    addAndMakeVisible(fretLabelButton);
    sourceFormButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(45, 49, 56));
    sourceFormButton.setColour(juce::TextButton::textColourOffId, juce::Colour::fromRGB(225, 230, 238));
    sourceFormButton.onClick = [this]
    {
        showStableSubset = !showStableSubset;
        updateMaterialSelection();
        refreshPanelView(true);
    };
    addAndMakeVisible(sourceFormButton);
    strategyList.setModel(this);
    strategyList.setRowHeight(29);
    strategyList.setMultipleSelectionEnabled(false);
    strategyList.setColour(juce::ListBox::backgroundColourId,
                           juce::Colour::fromRGB(42, 46, 53));
    strategyList.setOutlineThickness(0);
    addAndMakeVisible(strategyList);
    layerSelector.onChange = [this] { updateMaterialSelection(); };
    fretSelector.onChange = [this] { updateMaterialSelection(); };
    addAndMakeVisible(materialViewer);

    for (auto* button : { &contextButton, &materialButton, &sourcesButton,
                          &harmonicButton, &araButton })
    {
        button->setMouseCursor(juce::MouseCursor::PointingHandCursor);
        button->setColour(juce::TextButton::textColourOffId, juce::Colour::fromRGB(225, 230, 238));
        addAndMakeVisible(*button);
    }

    contextButton.onClick = [this] { setActivePanel(Panel::context); };
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
    processor.setEditorSize({getWidth(), getHeight()});
    const int margin = 24;
    const int gap = 8;
    const int buttonY = 82;
    const int buttonH = 36;
    const int available = getWidth() - margin * 2 - gap * 4;
    const int buttonW = available / 5;

    contextButton.setBounds(margin, buttonY, buttonW, buttonH);
    materialButton.setBounds(margin + buttonW + gap, buttonY, buttonW, buttonH);
    sourcesButton.setBounds(margin + 2 * (buttonW + gap), buttonY, buttonW, buttonH);
    harmonicButton.setBounds(margin + 3 * (buttonW + gap), buttonY, buttonW, buttonH);
    araButton.setBounds(margin + 4 * (buttonW + gap), buttonY,
                        getWidth() - margin - (margin + 4 * (buttonW + gap)), buttonH);

    const int columnX = getWidth() / 2;
    tensionSelector.setBounds(getWidth() - 205, 136, 165, 28);
    strategyList.setBounds(columnX + 16, 174, getWidth() - columnX - 56, 166);
    tensionHint.setBounds(columnX + 16, 344, getWidth() - columnX - 56, 34);
    detailsView.setBounds(margin + 12, 143, getWidth() - margin * 2 - 24, 235);
    sourceFormButton.setBounds(margin, 403, 250, 32);
    layerSelector.setBounds(getWidth() - 376, 403, 207, 32);
    fretSelector.setBounds(getWidth() - 161, 403, 137, 32);
    fretLabelButton.setBounds(getWidth() - 534, 403, 150, 32);
    materialViewer.setBounds(margin, 443, getWidth() - margin * 2, 284);
}

int SmartImproviserARAEditor::getNumRows()
{
    return static_cast<int>(strategyRows.size());
}

int SmartImproviserARAEditor::rowForMaterial(int index) const
{
    for (int row = 0; row < static_cast<int>(strategyRows.size()); ++row)
        if (strategyRows[static_cast<std::size_t>(row)].materialIndex == index)
            return row;
    return -1;
}

void SmartImproviserARAEditor::rebuildStrategyRows()
{
    std::vector<smartimproviser::harmony::TensionFilterItem> items;
    for (int index = 0; index < strategyLabels.size(); ++index)
        items.push_back({index, manualTension(index), isBaseMode(index), redundantMaterial(index)});
    auto rows = smartimproviser::harmony::filteredTensionRows(items, processor.tensionFilter());
    const int filter = processor.tensionFilter();
    const bool emptyLevel = filter != 0 && !smartimproviser::harmony::hasTensionRecommendation(rows);
    tensionHint.setText(emptyLevel
        ? ru("Нет назначений T") + juce::String(filter) + ru(". Метки — в режиме «Все материалы».")
        : (filter == 0 ? ru("Кружок слева — назначить или снять метку.")
                       : ru("Метки всех способов — в режиме «Все материалы».")), juce::dontSendNotification);
    bool changed = rows.size() != strategyRows.size();
    for (std::size_t i = 0; ! changed && i < rows.size(); ++i)
        changed = rows[i].materialIndex != strategyRows[i].materialIndex
            || rows[i].level != strategyRows[i].level
            || rows[i].baseMode != strategyRows[i].baseMode;
    if (changed)
    {
        strategyRows = std::move(rows);
        updatingSelector = true;
        strategyList.updateContent();
        const int selectedRow = rowForMaterial(selectedMaterialIndex);
        if (selectedRow >= 0) strategyList.selectRow(selectedRow);
        else strategyList.deselectAllRows();
        updatingSelector = false;
    }
    strategyList.repaint();
}

bool SmartImproviserARAEditor::isBaseMode(int index) const
{
    return index >= 0 && index == smartimproviser::harmony::playingBaseIndex(cachedResult, cachedExplanation);
}

bool SmartImproviserARAEditor::redundantMaterial(int index) const
{
    return smartimproviser::harmony::compactMaterialHidden(cachedExplanation, index)
        || (index >= 0 && index < static_cast<int>(cachedExplanation.items.size())
            && smartimproviser::harmony::isDiatonicFoundation(cachedExplanation.items[static_cast<std::size_t>(index)])
            && !isBaseMode(index));
}

std::string SmartImproviserARAEditor::tensionKey(int index) const
{
    if (index < 0 || index >= static_cast<int>(cachedExplanation.items.size())) return {};
    return smartimproviser::harmony::manualTensionKey(
        cachedResult, cachedExplanation.items[static_cast<std::size_t>(index)]);
}

int SmartImproviserARAEditor::manualTension(int index) const
{
    if (isBaseMode(index)) return 0;
    return processor.manualTensionFor(tensionKey(index));
}

void SmartImproviserARAEditor::paintListBoxItem(int row, juce::Graphics& g,
                                                int width, int height, bool selected)
{
    if (row < 0 || row >= static_cast<int>(strategyRows.size())) return;
    const auto entry = strategyRows[static_cast<std::size_t>(row)];
    const auto blue = juce::Colour::fromRGB(91, 158, 223);
    const auto orange = juce::Colour::fromRGB(242, 157, 62);
    const auto red = juce::Colour::fromRGB(233, 94, 96);
    const auto gray = juce::Colour::fromRGB(154, 165, 179);
    const auto level = entry.level;
    const auto dot = level == 1 ? blue : level == 2 ? orange : level == 3 ? red : gray;
    const bool groupStart = row > 0
        && (strategyRows[static_cast<std::size_t>(row - 1)].level != level
            || strategyRows[static_cast<std::size_t>(row - 1)].baseMode != entry.baseMode);
    if (groupStart)
    {
        g.setColour(juce::Colour::fromRGB(76, 84, 94));
        g.drawHorizontalLine(0, 8.0f, static_cast<float>(width - 8));
    }
    const auto frame = juce::Rectangle<float>(1.0f, 1.0f,
                                              static_cast<float>(width - 3),
                                              static_cast<float>(height - 3));
    g.setColour(selected ? juce::Colour::fromRGB(48, 65, 83)
                         : juce::Colour::fromRGB(39, 44, 50));
    g.fillRoundedRectangle(frame, 5.0f);
    g.setColour(selected ? blue : juce::Colour::fromRGB(70, 80, 91));
    g.drawRoundedRectangle(frame, 5.0f, 1.0f);
    if (entry.baseMode)
    {
        g.setColour(juce::Colour::fromRGB(225, 230, 238));
        g.setFont(juce::FontOptions(12.3f, juce::Font::bold));
        g.drawText(strategyLabels[entry.materialIndex], 12, 0, width - 88, height,
                   juce::Justification::centredLeft, true);
        g.setColour(juce::Colour::fromRGB(150, 185, 212));
        g.setFont(juce::FontOptions(10.5f));
        g.drawText(ru("ОСНОВА"), width - 78, 0, 66, height,
                   juce::Justification::centredRight);
        return;
    }
    g.setColour(dot);
    g.drawEllipse(10.0f, (height - 18) * 0.5f, 18.0f, 18.0f, 1.5f);
    if (level != 0)
        g.fillEllipse(14.0f, (height - 10) * 0.5f, 10.0f, 10.0f);
    g.setColour(juce::Colour::fromRGB(225, 230, 238));
    g.setFont(juce::FontOptions(12.3f));
    g.drawText(strategyLabels[entry.materialIndex], 38, 0, width - 46, height,
               juce::Justification::centredLeft, true);
}

void SmartImproviserARAEditor::selectMaterial(int index)
{
    if (index < 0 || index >= static_cast<int>(cachedExplanation.items.size())) return;
    if (selectedMaterialIndex == index) return;
    selectedMaterialIndex = index;
    selectedMaterialKey = materialSelectionKey(cachedExplanation.items[static_cast<std::size_t>(index)]);
    updatingSelector = true;
    strategyList.selectRow(rowForMaterial(index));
    updatingSelector = false;
    updateMaterialSelection();
    refreshPanelView(true);
    strategyList.repaint();
}

void SmartImproviserARAEditor::selectedRowsChanged(int row)
{
    if (updatingSelector || row < 0 || row >= static_cast<int>(strategyRows.size())) return;
    selectMaterial(strategyRows[static_cast<std::size_t>(row)].materialIndex);
}

void SmartImproviserARAEditor::listBoxItemClicked(int row, const juce::MouseEvent& event)
{
    if (row < 0 || row >= static_cast<int>(strategyRows.size())) return;
    const int index = strategyRows[static_cast<std::size_t>(row)].materialIndex;
    selectMaterial(index);
    if (!strategyRows[static_cast<std::size_t>(row)].baseMode && event.x < 34)
        openTensionMenu(index, event);
}

void SmartImproviserARAEditor::openTensionMenu(int index, const juce::MouseEvent& event)
{
    if (isBaseMode(index)) return;
    const auto key = tensionKey(index);
    if (key.empty()) return;
    const int current = manualTension(index);
    const auto icon = [](int level)
    {
        juce::Image image(juce::Image::ARGB, 18, 18, true);
        juce::Graphics graphics(image);
        const auto colour = level == 1 ? juce::Colour::fromRGB(91, 158, 223)
            : level == 2 ? juce::Colour::fromRGB(242, 157, 62)
            : level == 3 ? juce::Colour::fromRGB(233, 94, 96)
            : juce::Colour::fromRGB(154, 165, 179);
        graphics.setColour(colour);
        graphics.drawEllipse(1.0f, 1.0f, 16.0f, 16.0f, 1.5f);
        if (level != 0) graphics.fillEllipse(5.0f, 5.0f, 8.0f, 8.0f);
        return image;
    };
    juce::PopupMenu menu;
    menu.addItem(1, ru("Без метки"), true, current == 0, icon(0));
    menu.addItem(2, ru("T1 · базовое"), true, current == 1, icon(1));
    menu.addItem(3, ru("T2 · цвет"), true, current == 2, icon(2));
    menu.addItem(4, ru("T3 · максимум"), true, current == 3, icon(3));
    const auto screen = event.getScreenPosition();
    const auto options = juce::PopupMenu::Options().withTargetScreenArea(
        juce::Rectangle<int>(screen.x, screen.y, 1, 1));
    juce::Component::SafePointer<SmartImproviserARAEditor> safe(this);
    menu.showMenuAsync(options, [safe, key](int choice)
    {
        if (safe == nullptr || choice < 1 || choice > 4) return;
        if (!safe->processor.setManualTension(key, choice - 1))
        {
            juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                ru("Настройки tensions"), ru("Не удалось сохранить общий профиль tensions. Проверьте доступ к папке настроек пользователя."));
            return;
        }
        safe->timerCallback();
    });
}

juce::String SmartImproviserARAEditor::selectedMaterialText() const
{
    const int selected = displayedMaterialIndex();
    if (selected < 0 || selected >= static_cast<int>(cachedExplanation.items.size()))
        return {};
    const auto& item = cachedExplanation.items[static_cast<std::size_t>(selected)];
    juce::String text = ru("ВЫБРАННЫЙ МАТЕРИАЛ: ") + localizeGeneratedText(utf8String(item.source.name)) + "\n";
    if (const int manual = manualTension(selectedMaterialIndex); manual != 0)
        text += ru("Моя метка напряжения: T") + juce::String(manual) + "\n";
    if (!item.strategyIndices.empty()
        && item.strategyIndices.front() < cachedResult.strategies.size()
        && cachedResult.strategies[item.strategyIndices.front()].tensionClassified)
        text += ru("Оценка Core: T1 • базовое мышление для текущего контекста\n");
    if (item.interpretationIndependent)
    {
        const auto destination = smartimproviser::harmony::dominantDestination(cachedSituation);
        text += item.source.kind == smartimproviser::harmony::MaterialKind::chordTones
            ? ru("Опоры аккорда • независимо от трактовки\n")
            : destination.minor()
            ? ru("Источник для минорной цели доминанты: ") + utf8String(fifthsName(destination.rootFifths)) + "m\n"
            : ru("Источник от написанного аккорда • без выбора тонального центра\n");
    }
    if (item.missingTonicApplication)
    {
        text += ru("Материал незавершённого ii–V • ожидаемый I: ")
            + utf8String(fifthsName(item.missingTonicRootFifths))
            + ru(" [отсутствует]\n");
        if (item.targetChord.valid)
            text += ru("Фактическое продолжение: ")
                + utf8String(smartimproviser::harmony::normalizedChordSymbol(item.targetChord))
                + ru(". Разрешение в ожидаемый I не подтверждено.\n");
    }
    for (const int index : item.interpretationIndices)
    {
        if (index < 0 || index >= cachedSituation.interpretationCount) continue;
        const auto& interpretation = cachedSituation.interpretations[static_cast<std::size_t>(index)];
        text += ru("Трактовка ") + juce::String(index + 1) + ": "
            + interpretationKindNameRu(interpretation.kind) + " • "
            + centerKeyDisplayName(interpretation.center) + "\n";
    }
    if (cachedSituation.primaryInterpretationIndex < 0 && cachedSituation.interpretationCount > 1)
        text += ru("[неоднозначно] Выбор показа не определяет главную трактовку.\n");
    if (!showStableSubset
        && smartimproviser::harmony::stableSubsetIndex(cachedExplanation, selectedMaterialIndex) >= 0)
        text += ru("Полная гамма не объявлена T1. Кнопка «Материал» переключает на четыре звука T1 m6.\n");
    text += localizeGeneratedText(utf8String(item.idea)) + "\n";
    for (const auto& condition : item.applicationConditions)
        text += ru("Условия: ") + localizeGeneratedText(utf8String(condition)) + "\n";
    if (! item.usageHint.empty())
        text += localizeGeneratedText(utf8String(item.usageHint)) + "\n";
    const auto view = smartimproviser::harmony::buildMaterialView(
        cachedResult, cachedExplanation, static_cast<std::size_t>(selected));
    if (item.source.kind == smartimproviser::harmony::MaterialKind::scale
        || item.source.kind == smartimproviser::harmony::MaterialKind::arpeggio)
    {
        text += ru("Ноты источника: ");
        for (const auto& note : item.source.notes)
            text += utf8String(note.spelling) + " ";
        text += "\n";
        if (! item.source.chordRelativeNotes.empty())
        {
            text += ru("Те же звуки на ")
                + utf8String(smartimproviser::harmony::normalizedChordSymbol(item.actualChord)) + ": ";
            for (const auto& note : item.source.chordRelativeNotes)
                text += utf8String(note.spelling) + " ";
            text += "\n";
        }
    }
    for (const auto& note : view.current)
        if (! note.chordSpelling.empty() && note.spelling != note.chordSpelling)
            text += utf8String(note.spelling) + " = " + utf8String(note.chordSpelling)
                + ru(" в контексте аккорда\n");
    if (item.targetChord.valid)
        text += ru("Цели реального следующего аккорда: ")
            + utf8String(smartimproviser::harmony::normalizedChordSymbol(item.targetChord)) + "\n";
    for (const auto& why : item.why)
    {
        using State = smartimproviser::harmony::ExplanationEvidenceState;
        if (why.state == State::missing || why.state == State::implied
            || why.state == State::contradicted || why.state == State::ambiguous)
        {
            const char* marker = why.state == State::missing ? "[отсутствует] " :
                why.state == State::implied ? "[подразумевается] " :
                why.state == State::contradicted ? "[противоречие] " : "[неоднозначно] ";
            text += ru(marker) + localizeGeneratedText(utf8String(why.ruleId)) + "\n";
        }
    }
    return text + "\n";
}

int SmartImproviserARAEditor::displayedMaterialIndex() const
{
    if (showStableSubset)
    {
        const int subset = smartimproviser::harmony::stableSubsetIndex(cachedExplanation, selectedMaterialIndex);
        if (subset >= 0) return subset;
    }
    return selectedMaterialIndex;
}

void SmartImproviserARAEditor::updateMaterialSelection()
{
    const bool degreeLabels = processor.fretDegreeLabelsEnabled();
    materialViewer.setFretDegreeLabels(degreeLabels);
    fretLabelButton.setButtonText(degreeLabels ? ru("Гриф: ступени") : ru("Гриф: ноты"));
    const bool hasSubset = smartimproviser::harmony::stableSubsetIndex(cachedExplanation, selectedMaterialIndex) >= 0;
    sourceFormButton.setEnabled(hasSubset);
    sourceFormButton.setButtonText(hasSubset
        ? (showStableSubset ? ru("Материал: T1 • m6") : ru("Материал: полная гамма"))
        : ru("Материал: источник"));
    const int selected = displayedMaterialIndex();
    const auto layer = static_cast<smartimproviser::harmony::ViewerLayer>(
        juce::jlimit(0, 5, layerSelector.getSelectedId() - 1));
    const int range = juce::jlimit(1, 3, fretSelector.getSelectedId());
    const int first = range == 1 ? 0 : range == 2 ? 5 : 12;
    materialViewer.showMaterial(smartimproviser::harmony::buildMaterialView(
        cachedResult, cachedExplanation, selected < 0 ? cachedExplanation.items.size()
                                                   : static_cast<std::size_t>(selected)),
        layer, first, first + 12);
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

    configure(contextButton, Panel::context, "Текущий контекст", "Текущий контекст");
    configure(materialButton, Panel::material, "Материал", "Материал");
    configure(sourcesButton, Panel::sources, "Источники / ноты", "Источники / ноты");
    configure(harmonicButton, Panel::harmonic, "Гармонический анализ", "Гармонический анализ");
    configure(araButton, Panel::ara, "ARA / диагностика", "ARA / диагностика");
    strategyList.setVisible(activePanel == Panel::context);
    tensionSelector.setVisible(activePanel == Panel::context);
    tensionHint.setVisible(activePanel == Panel::context);
    detailsView.setVisible(activePanel != Panel::context);
}

void SmartImproviserARAEditor::refreshPanelView(bool resetScroll)
{
    if (activePanel == Panel::context) return;
    const juce::String* text = &materialText;
    switch (activePanel)
    {
        case Panel::context: break;
        case Panel::material: text = &materialText; break;
        case Panel::sources: text = &sourcesText; break;
        case Panel::harmonic: text = &harmonicText; break;
        case Panel::ara: text = &araText; break;
    }

    const juce::String content = (activePanel == Panel::material || activePanel == Panel::sources)
        ? selectedMaterialText() + *text : *text;
    if (detailsView.getText() != content)
    {
        detailsView.setText({}, false);
        const auto ordinary = juce::Colour::fromRGB(225, 230, 238);
        const auto missing = juce::Colour::fromRGB(150, 156, 168);
        const auto implied = juce::Colour::fromRGB(238, 181, 85);
        const auto contradicted = juce::Colour::fromRGB(244, 108, 108);
        // TextEditor stores the current text colour on each inserted run.
        // Only the ready-made explanation state markers drive this styling.
        int start = 0;
        while (start < content.length())
        {
            const int newline = content.indexOfChar(start, '\n');
            const int end = newline < 0 ? content.length() : newline + 1;
            const auto line = content.substring(start, end);
            auto colour = ordinary;
            if (activePanel == Panel::material || activePanel == Panel::sources)
            {
                if (line.contains(ru("[отсутствует]"))) colour = missing;
                else if (line.contains(ru("[подразумевается]")) || line.contains(ru("[ожидается]"))) colour = implied;
                else if (line.contains(ru("[противоречие]"))) colour = contradicted;
            }
            detailsView.setColour(juce::TextEditor::textColourId, colour);
            detailsView.insertTextAtCaret(line);
            start = end;
        }
        detailsView.setColour(juce::TextEditor::textColourId, ordinary);
        if (resetScroll)
            detailsView.moveCaretToTop(false);
    }
}

void SmartImproviserARAEditor::timerCallback()
{
    processor.refreshSharedManualTensions();
    tensionSelector.setSelectedId(processor.tensionFilter() + 1, juce::dontSendNotification);
    cachedShared = SharedHarmonicContextBridge::instance().read();
    const auto ppq = cachedShared.transportAvailable ? cachedShared.transportPpq : -1.0;
    const auto timeline = smartimproviser::harmony::mapTimelineHarmonicSnapshot(cachedShared, ppq);
    const auto patternWindow = smartimproviser::harmony::mapPatternTimelineWindow(cachedShared, ppq);
    const auto context = smartimproviser::harmony::mapHarmonicContext(cachedShared, ppq);
    cachedSituation = smartimproviser::harmony::analyzeHarmonicSituation(timeline, patternWindow);
    auto result = smartimproviser::harmony::analyzeImprovisation(cachedSituation);
    const auto stableProfile = smartimproviser::harmony::analyzeStableTension(result);
    if (const auto* band = stableProfile.band(smartimproviser::harmony::TensionLevel::stable))
        for (const auto& candidate : band->alternatives)
            result.strategies.push_back(candidate.strategy);
    const auto explanation = smartimproviser::harmony::explainImprovisation(result);
    cachedResult = result;
    cachedExplanation = explanation;
    updatingSelector = true;
    int keep = -1;
    for (std::size_t i = 0; i < explanation.items.size(); ++i)
    {
        const auto& item = explanation.items[i];
        const auto key = materialSelectionKey(item);
        if (key == selectedMaterialKey && selectedMaterialKey.isNotEmpty()) keep = static_cast<int>(i);
    }
    juce::StringArray labels;
    for (std::size_t i = 0; i < explanation.items.size(); ++i)
    {
        const auto& item = explanation.items[i];
        juce::String label = localizeGeneratedText(utf8String(item.source.name));
        if (label.isEmpty()) label = localizeGeneratedText(utf8String(item.idea));
        labels.add(label);
    }
    bool changed = strategyLabels.size() != labels.size();
    for (int i = 0; ! changed && i < labels.size(); ++i)
        changed = strategyLabels[i] != labels[i];
    if (changed)
    {
        strategyLabels = labels;
    }
    rebuildStrategyRows();
    if (! explanation.items.empty())
    {
        const auto index = smartimproviser::harmony::filteredMaterialSelection(
            strategyRows, keep, processor.tensionFilter());
        selectedMaterialIndex = index;
        if (index >= 0 && strategyList.getSelectedRow() != rowForMaterial(index))
            strategyList.selectRow(rowForMaterial(index));
        if (index >= 0)
            selectedMaterialKey = materialSelectionKey(explanation.items[static_cast<std::size_t>(index)]);
        else { selectedMaterialKey.clear(); strategyList.deselectAllRows(); }
    }
    else { selectedMaterialKey.clear(); selectedMaterialIndex = -1; }
    updatingSelector = false;
    strategyList.repaint();
    updateMaterialSelection();
    const auto impliedLink = smartimproviser::harmony::explainImpliedDominantLink(cachedSituation);
    const auto debug = ARAContextDebugState::instance().getSnapshot();

    summaryContext = timeline.previousChordAvailable
        ? chordDisplayName(timeline.previousChord) + " -> " + chordDisplayName(timeline.currentChord)
        : chordDisplayName(timeline.currentChord);
    if (timeline.nextChordAvailable)
        summaryContext += " -> " + chordDisplayName(timeline.nextChord);
    summaryContextDisplay.clear();
    const juce::Font chordFont { juce::FontOptions(17.0f, juce::Font::bold) };
    const auto ordinaryChord = juce::Colour::fromRGB(240, 243, 247);
    const auto currentChordColour = juce::Colour::fromRGB(246, 191, 88);
    if (timeline.previousChordAvailable)
        summaryContextDisplay.append(chordDisplayName(timeline.previousChord) + ru("  →  "),
                                     chordFont, ordinaryChord);
    summaryContextDisplay.append(chordDisplayName(timeline.currentChord), chordFont,
                                 currentChordColour);
    if (timeline.nextChordAvailable)
        summaryContextDisplay.append(ru("  →  ") + chordDisplayName(timeline.nextChord),
                                     chordFont, ordinaryChord);
    summaryContextDisplay.setWordWrap(juce::AttributedString::none);

    summaryMeta = ru("Глобальная тональность: ") + keyDisplayName(timeline.globalKey);
    summaryGlobalFunction.clear();
    summaryLocal.clear();
    for (const auto& layer : explanation.contextLayers)
    {
        using Scope = smartimproviser::harmony::ExplanationContextScope;
        if (layer.scope == Scope::global && layer.interpretationIndex < 0)
        {
            if (layer.harmonic.valid)
                summaryGlobalFunction = ru("Глобальная функция: ")
                    + harmonicFunctionNameRu(layer.harmonic.effectiveFunction);
        }
        else if (layer.scope == Scope::local && layer.interpretationIndex < 0)
        {
            summaryLocal = ru("Локальный центр: ") + centerKeyDisplayName(layer.center);
            if (layer.center.status == smartimproviser::harmony::KeyCenterStatus::candidate)
                summaryLocal += ru(" (кандидат)");
            if (layer.harmonic.valid)
                summaryLocal += ru("   •   Локальная функция: ")
                    + harmonicFunctionNameRu(layer.harmonic.effectiveFunction);
        }
        else if (layer.scope == Scope::modal)
        {
            if (summaryLocal.isNotEmpty())
                summaryLocal += ru("   •   ");
            summaryLocal += ru("Модальная трактовка: ") + centerKeyDisplayName(layer.center);
            if (layer.harmonic.valid)
                summaryLocal += ru(" (") + harmonicFunctionNameRu(layer.harmonic.effectiveFunction) + ")";
        }
    }
    if (cachedSituation.localKey.valid
        && cachedSituation.localKey.evidence.confidence
            == smartimproviser::harmony::ConfidenceLevel::confirmed
        && cachedSituation.primaryInterpretationIndex >= 0)
    {
        // The project key remains in the Core/explanation diagnostics. The
        // compact playing context follows the established local cadence.
        summaryMeta = summaryLocal;
        summaryLocal.clear();
    }
    if (cachedSituation.incompleteCadence.valid)
    {
        // Core has recognized an ordinary ii-V with an unplayed major I.
        // A project-key SubV candidate is unrelated to this turn; show its
        // provisional functional direction here and keep global diagnostics
        // in the harmonic analysis panel.
        summaryMeta = ru("Направление II–V: ")
            + utf8String(fifthsName(cachedSituation.incompleteCadence.missingTonicRootFifths))
            + ru(" мажор");
        if (smartimproviser::harmony::hasContextualMinorIiVTarget(cachedSituation))
            summaryMeta = ru("Предполагаемая цель оборота: ")
                + utf8String(fifthsName(cachedSituation.incompleteCadence.missingTonicRootFifths))
                + ru("m (по тональному контексту)");
        summaryGlobalFunction = ru("Функция в обороте: ")
            + (cachedSituation.incompleteCadence.positionIndex == 0
                ? ru("Субдоминанта") : ru("Доминанта"));
        summaryLocal.clear();
    }

    const auto destination = smartimproviser::harmony::dominantDestination(cachedSituation);
    if (destination.minor())
    {
        summaryMeta = (destination.confirmed ? ru("Цель оборота: ") : ru("Предполагаемая цель оборота: "))
            + utf8String(fifthsName(destination.rootFifths))
            + (destination.confirmed ? ru("m (подтверждена)") : ru("m (по тональному контексту)"));
        summaryGlobalFunction = ru("Функция в обороте: Доминанта");
        summaryLocal.clear();
    }
    const bool minorTurnWithIi = destination.minor()
        && cachedSituation.previousChordAvailable
        && cachedSituation.previousChord.rootPitchClass == (cachedSituation.currentChord.rootPitchClass + 7) % 12
        && (cachedSituation.previousChord.quality == smartimproviser::harmony::ChordQuality::minor
            || cachedSituation.previousChord.quality == smartimproviser::harmony::ChordQuality::halfDiminished);

    const bool missingMinorArrival = minorTurnWithIi
        && (!cachedSituation.nextChordAvailable
            || cachedSituation.nextChord.quality != smartimproviser::harmony::ChordQuality::minor);

    const auto* activePattern = &cachedSituation.pattern;
    if (cachedSituation.localPattern.recognized())
        activePattern = &cachedSituation.localPattern;

    summaryPatternDisplay.clear();
    const juce::Font labelFont { juce::FontOptions(14.0f) };
    const juce::Font romanFont { juce::FontOptions(16.0f, juce::Font::bold) };
    const auto presentColour = juce::Colour::fromRGB(220, 225, 234);
    const auto missingColour = juce::Colour::fromRGB(140, 146, 157);
    const auto impliedColour = juce::Colour::fromRGB(224, 172, 85);
    const auto label = ru("Оборот: ");
    juce::String patternRomanText;
    const auto progress = [](int position, int length)
    {
        return position >= 0 && length > 0
            ? ru("   •   ") + juce::String(position + 1) + " / " + juce::String(length)
            : juce::String();
    };
    const auto showPattern = [&](const juce::String& roman,
                                 const juce::String& last,
                                 juce::Colour lastColour,
                                 const juce::String& suffix)
    {
        patternRomanText = roman + last;
        summaryPattern = label + roman + last + suffix;
        summaryPatternDisplay.append(label, labelFont, presentColour);
        summaryPatternDisplay.append(roman, romanFont, presentColour);
        if (last.isNotEmpty())
            summaryPatternDisplay.append(last, romanFont, lastColour);
        if (suffix.isNotEmpty())
            summaryPatternDisplay.append(suffix, labelFont, presentColour);
    };
    const auto& incomplete = cachedSituation.incompleteCadence;
    if (minorTurnWithIi)
    {
        showPattern(cachedSituation.previousChord.quality == smartimproviser::harmony::ChordQuality::halfDiminished
                        ? ru("IIø–V–") : ru("II–V–"), "i",
                    missingMinorArrival ? missingColour : presentColour, progress(1, 3));
    }
    else if (incomplete.valid)
    {
        showPattern(ru("II–V–"), smartimproviser::harmony::hasContextualMinorIiVTarget(cachedSituation) ? "i" : "I",
                    missingColour, progress(incomplete.positionIndex, 3));
    }
    else if (impliedLink.valid)
    {
        showPattern(ru("V/V → "), "V", impliedColour, progress(impliedLink.positionIndex, 2));
    }
    else if (cachedSituation.expectedTonic.valid)
    {
        showPattern(ru("III–VI–II–V–"), "I", missingColour,
                    progress(cachedSituation.patternContext.topLevel.positionIndex,
                             cachedSituation.patternContext.topLevel.length));
    }
    else if (! cachedSituation.patternContext.valid
             && cachedSituation.localKey.valid
             && cachedSituation.localKey.status
                 == smartimproviser::harmony::KeyCenterStatus::candidate
             && cachedSituation.localPattern.type
                 == smartimproviser::harmony::HarmonicPatternType::majorIiVI
             && cachedSituation.localPattern.positionIndex >= 0
             && cachedSituation.localPattern.positionIndex < 2)
    {
        // A visible ii–V without a known I is only a candidate. The expected
        // tonic is not a played chord or an established local center.
        showPattern(ru("II–V–"), "I", impliedColour,
                    progress(cachedSituation.localPattern.positionIndex, 3));
    }
    else
    {
        showPattern(activePattern->recognized()
                        ? summaryPatternName(activePattern->type) : ru("не распознан"),
                    {}, presentColour,
                    progress(activePattern->positionIndex, activePattern->length));
    }
    summaryPatternDisplay.setWordWrap(juce::AttributedString::none);
    patternMembers.clear();
    patternDisplayPosition = -1;
    auto sequence = patternRomanText.replace(ru(" → "), ru("–"));
    if (sequence.contains(ru("–")) && sequence.length() < 40)
    {
        juce::StringArray romanTokens;
        while (sequence.isNotEmpty())
        {
            const int separator = sequence.indexOfChar(0x2013);
            romanTokens.add(separator < 0 ? sequence : sequence.substring(0, separator));
            if (separator < 0) break;
            sequence = sequence.substring(separator + 1);
        }
        const bool missingLast = minorTurnWithIi ? missingMinorArrival : incomplete.valid || cachedSituation.expectedTonic.valid
            || (!cachedSituation.patternContext.valid && cachedSituation.localKey.valid
                && cachedSituation.localKey.status
                    == smartimproviser::harmony::KeyCenterStatus::candidate
                && cachedSituation.localPattern.type
                    == smartimproviser::harmony::HarmonicPatternType::majorIiVI);
        const int position = minorTurnWithIi ? 1 : incomplete.valid ? incomplete.positionIndex
            : impliedLink.valid ? impliedLink.positionIndex : activePattern->positionIndex;
        patternDisplayPosition = position;
        const int start = patternWindow.currentIndex - position;
        for (int i = 0; i < romanTokens.size(); ++i)
        {
            PatternDisplayMember member;
            member.roman = romanTokens[i];
            member.current = i == position;
            member.expected = missingLast && i == romanTokens.size() - 1;
            if (member.expected)
                member.chord = ru("—");
            else if (minorTurnWithIi && i < 2)
                member.chord = utf8String(smartimproviser::harmony::normalizedChordSymbol(
                    i == 0 ? cachedSituation.previousChord : cachedSituation.currentChord));
            else if (minorTurnWithIi && i == 2)
                member.chord = utf8String(smartimproviser::harmony::normalizedChordSymbol(cachedSituation.nextChord));
            else if (incomplete.valid && i < 2)
                member.chord = utf8String(smartimproviser::harmony::normalizedChordSymbol(
                    i == 0 ? incomplete.ii : incomplete.v));
            else if (impliedLink.valid && i < 2)
                member.chord = utf8String(smartimproviser::harmony::normalizedChordSymbol(
                    i == 0 ? impliedLink.firstChord : impliedLink.secondChord));
            else if (start + i >= 0 && start + i < patternWindow.chordCount)
                member.chord = chordDisplayName(patternWindow.chords[static_cast<std::size_t>(start + i)]);
            else if (member.current)
                member.chord = chordDisplayName(timeline.currentChord);
            patternMembers.push_back(std::move(member));
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
        for (const auto& sourceItem : cachedExplanation.items)
        {
            if (sourceItem.strategyIndices.empty()) continue;
            const auto& scalar = result.strategies[sourceItem.strategyIndices.front()];
            if (scalar.source.kind != smartimproviser::harmony::MaterialKind::scale
                && scalar.source.kind != smartimproviser::harmony::MaterialKind::arpeggio)
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

            if (scalar.missingTonicApplication)
            {
                sourcesText += ru("ГИПОТЕЗА НЕЗАВЕРШЁННОГО II–V: ожидаемый ")
                    + utf8String(fifthsName(scalar.missingTonicRootFifths))
                    + ru(" [отсутствует], далее фактически ")
                    + utf8String(smartimproviser::harmony::normalizedChordSymbol(scalar.nextChord))
                    + ru("; разрешение в ожидаемый мажорный I не подтверждено.\n");
            }

            sourcesText += (scalar.tensionClassified ? ru("T1 • ") : juce::String())
                + localizeGeneratedText(utf8String(scalar.source.name)) + "\n";
            for (const auto& note : scalar.source.notes)
                sourcesText += utf8String(note.spelling) + " ";

            if (! scalar.sourceReference.empty())
            {
                sourcesText += "\n" + localizeGeneratedText(utf8String(scalar.idea));
                if (scalar.thinkingStructure.valid)
                    sourcesText += ru("\nМыслить: ")
                        + utf8String(smartimproviser::harmony::normalizedChordSymbol(scalar.thinkingStructure));
                sourcesText += ru("\nТе же звуки относительно ")
                    + utf8String(smartimproviser::harmony::normalizedChordSymbol(scalar.actualChord)) + ": ";
                for (const auto& note : scalar.source.chordRelativeNotes)
                    sourcesText += utf8String(note.spelling) + " ";
            }

            sourcesText += "\n" + localizeGeneratedText(utf8String(scalar.usageHint));
            for (const auto& condition : sourceItem.applicationConditions)
                sourcesText += ru("\nУсловия: ") + localizeGeneratedText(utf8String(condition));
            if (sourceItem.interpretationIndices.size() > 1)
            {
                sourcesText += ru("\nТрактовки: ");
                for (const int index : sourceItem.interpretationIndices)
                    sourcesText += juce::String(index + 1) + " ";
            }
            if (!scalar.sourceReference.empty())
                sourcesText += ru("\nИсточник правила: ") + utf8String(scalar.sourceReference);
            if (! scalar.sourceTransitions.empty())
            {
                sourcesText += ru("\nНеобязательные движения красок: ");
                for (const auto& move : scalar.sourceTransitions)
                    sourcesText += pitchText(move.fromPitchClass, scalar.actualChord,
                                             scalar.source.chordRelativeNotes) + "->"
                        + pitchText(move.toPitchClass, scalar.nextChord, scalar.targetNotes) + " ";
            }
        }

        if (! hasScale)
            sourcesText += localizeGeneratedText(utf8String(result.scaleUnavailableReason));

        sourcesText += ru("\n\nОПОРНЫЕ НОТЫ АККОРДА\n");
        for (const auto& note : strategy.source.notes)
            sourcesText += utf8String(smartimproviser::harmony::spelledChordNote(strategy.actualChord, note)) + " ";

        sourcesText += ru("\n\nНАПРАВЛЯЮЩИЕ ТОНЫ (3 / 7)\n")
            + notesText(strategy.guideNotes, strategy.actualChord);
        sourcesText += ru("\n\nХАРАКТЕРНЫЕ НОТЫ\n")
            + notesText(strategy.characteristicNotes, strategy.actualChord);
        sourcesText += ru("\n\nЦЕЛИ СЛЕДУЮЩЕГО АККОРДА\n");
        if (strategy.nextChord.valid)
            sourcesText += utf8String(smartimproviser::harmony::normalizedChordSymbol(strategy.nextChord))
                + ": " + notesText(strategy.targetNotes, strategy.nextChord);
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
                sourcesText += moveText(strategy.resolution.moves[i], strategy);
            if (strategy.resolution.moveCount == 0)
                sourcesText += ru("Нет доступных структурных движений");
        }
        else
        {
            for (const auto& move : strategy.suggestedTransitions)
                sourcesText += moveText(move, strategy);
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
        + (impliedLink.valid ? ru("V/V → подразумеваемая V")
           : cachedSituation.valid ? patternName(cachedSituation.pattern.type)
                                   : juce::String("-")) + "\n";
    harmonicText += ru("Позиция в обороте: ") + patternPositionDisplay(cachedSituation.pattern) + "\n";
    harmonicText += ru("Разрешение: ") + resolutionDisplay(cachedSituation) + "\n\n";

    if (impliedLink.valid)
    {
        harmonicText += ru("ДВОЙНАЯ ДОМИНАНТА / ПОДРАЗУМЕВАЕМАЯ V\n") + summaryPattern + "\n";
        harmonicText += ru("Второй записанный аккорд сохраняет своё имя: это возможное ")
            + ru("безосновное доминантовое прочтение, без подтверждённого разрешения в глобальную тонику.\n");
        if (impliedLink.continuationIsMinorOnImpliedRoot)
            harmonicText += ru("Далее звучит ")
                + utf8String(smartimproviser::harmony::normalizedChordSymbol(
                    impliedLink.actualContinuation))
                + ru(": минорный аккорд на том же звуке; доминантовое разрешение в глобальную тонику не состоялось.\n");
        harmonicText += "\n";
    }

    if (incomplete.valid)
    {
        harmonicText += ru("НЕЗАВЕРШЁННЫЙ ОБОРОТ\n") + summaryPattern + "\n";
        harmonicText += ru("Серый I - отсутствующая тоника шаблона, не аккорд дорожки.\n");
        harmonicText += ru("Фактическое продолжение после V: ")
            + utf8String(smartimproviser::harmony::normalizedChordSymbol(incomplete.actualContinuation))
            + ru(". Ожидаемое разрешение в I не состоялось.\n");
        harmonicText += ru("Шаблон не устанавливает локальную тональность и не подтверждает разрешение.\n\n");
    }

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
    g.drawText(ru("0.5 • материал и tension"),
               24, 47, getWidth() - 48, 22, juce::Justification::centredLeft);

    g.setColour(juce::Colour::fromRGB(42, 46, 53));
    g.fillRoundedRectangle(24.0f, 128.0f, static_cast<float>(getWidth() - 48), 264.0f, 8.0f);
    g.fillRoundedRectangle(24.0f, 739.0f, static_cast<float>(getWidth() - 48),
                           static_cast<float>(getHeight() - 767), 8.0f);
    if (activePanel == Panel::context)
    {
    const int columnX = getWidth() / 2;
    const int leftWidth = columnX - 64;

    summaryContextDisplay.draw(g, juce::Rectangle<float>(40.0f, 148.0f,
                               static_cast<float>(leftWidth), 28.0f));

    g.setColour(juce::Colour::fromRGB(190, 196, 207));
    g.setFont(13.2f);
    g.drawText(summaryMeta, 40, 185, leftWidth, 20,
               juce::Justification::centredLeft, true);
    g.drawText(summaryGlobalFunction, 40, 208, leftWidth, 20,
               juce::Justification::centredLeft, true);
    g.drawText(summaryLocal, 40, 231, leftWidth, 20,
               juce::Justification::centredLeft, true);
    g.setColour(juce::Colour::fromRGB(150, 156, 168));
    g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    g.drawText(ru("ОБОРОТ"), 40, 270, leftWidth, 16, juce::Justification::centredLeft);
    if (!patternMembers.empty())
    {
        const int cellWidth = leftWidth / static_cast<int>(patternMembers.size());
        for (int i = 0; i < static_cast<int>(patternMembers.size()); ++i)
        {
            const auto& member = patternMembers[static_cast<std::size_t>(i)];
            const int x = 40 + i * cellWidth;
            g.setColour(member.expected ? juce::Colour::fromRGB(140, 146, 157)
                        : member.current ? juce::Colour::fromRGB(246, 191, 88)
                        : juce::Colour::fromRGB(229, 234, 242));
            g.setFont(juce::FontOptions(21.0f, juce::Font::bold));
            g.drawFittedText(member.roman, x, 288, cellWidth - 4, 28,
                             juce::Justification::centred, 1);
            g.setFont(juce::FontOptions(13.0f));
            g.drawFittedText(member.chord, x, 321, cellWidth - 4, 20,
                             juce::Justification::centred, 1);
        }
        g.setColour(juce::Colour::fromRGB(150, 156, 168));
        g.setFont(11.5f);
        if (patternDisplayPosition >= 0)
            g.drawText(juce::String(patternDisplayPosition + 1) + " / "
                + juce::String(static_cast<int>(patternMembers.size())),
                40, 350, leftWidth, 18, juce::Justification::centred);
    }
    else
        summaryPatternDisplay.draw(g, juce::Rectangle<float>(40.0f, 290.0f,
                                    static_cast<float>(leftWidth), 30.0f));

    g.setColour(juce::Colour::fromRGB(76, 84, 94));
    g.drawLine(static_cast<float>(columnX), 143.0f,
               static_cast<float>(columnX), 378.0f);
    g.setColour(juce::Colour::fromRGB(207, 215, 227));
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText(ru("СПОСОБЫ ОБЫГРЫВАНИЯ"), columnX + 16, 140,
               getWidth() - columnX - 230, 19,
               juce::Justification::centredLeft);
    }

    g.setColour(juce::Colour::fromRGB(105, 110, 120));
    g.setFont(12.0f);
    g.drawText("Smart Improviser", 24, getHeight() - 28, getWidth() - 48, 20,
               juce::Justification::centredLeft);
}
