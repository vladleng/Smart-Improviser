# Smart Improviser

**Smart Improviser** — VST3/ARA 2 плагин для DAW, который помогает гитаристу строить джазовую импровизацию на основе **гармонического контекста**, а не только текущего аккорда.

> Главная идея: отвечать не только на вопрос «что здесь можно сыграть?», но и «что музыкально уместно сыграть именно здесь, зачем, куда это разрешить и с какой степенью напряжения?»

## Статус

**Stage 0 — Foundation / Specification завершён.**  
**Stage 1 — ARA Context Monitor завершён.**  
**Активный Stage:** Stage 2 — Harmonic Engine.

**Текущая стабильная версия:** `0.2`  
**Последний принятый checkpoint:** `0.2e / 0.2e fix1 — Ambiguity / Confidence`  
**Текущая рабочая версия:** `0.2f — Integration / musical validation`  
**Активная ветка:** `stage-2-integration-validation`  
**Stage 2 Issue:** #3  
**Активный PR:** #20

`0.2e / 0.2e fix1` принят и слит в `main` через PR #19. `0.2f` — последний буквенный checkpoint Stage 2 перед стабильной `0.3`.

Первая целевая среда:

- **DAW:** Fender Studio Pro
- **Интеграция:** ARA 2
- **Формат:** VST3
- **Платформа:** Windows
- **Основной сценарий:** джазовая импровизация на гитаре

## Ключевая идея

Smart Improviser строится не вокруг простой схемы `Chord → Scale`, а вокруг цепочки:

```text
Harmony
→ Context
→ Global / Local Function
→ Harmonic Pattern
→ Ambiguity / Confidence
→ Tension
→ Strategy
→ Resolution
→ Phrase / Vocabulary
```

Центральная сущность ядра — `HarmonicSituation`.

## Архитектура

```text
Fender Studio Pro
        ↓
ARA 2 Adapter
        ↓
TimelineHarmonicSnapshot
(previous / current / next)
        ↓
Harmonic Engine
        ├── Pattern Recognizer
        ├── Tritone Substitution
        ├── Local Key Center Analyzer
        └── Ambiguity / Confidence Analyzer
        ↓
HarmonicSituation
        ↓
Tension / Strategy / Resolution / Phrase engines
        ↓
UI / Fretboard / Notation / TAB
```

Stage 1 contract закрыт: Harmonic Engine получает только host-neutral timeline context и не зависит от JUCE / ARA / Fender Studio Pro. Project key в DAW автоматически не меняется.

## Global key и Local Key Center

Project key и активный тональный центр — разные сущности:

```text
GLOBAL KEY
F major
    ↓
LOCAL CENTER
D minor
    ↓
CURRENT LOCAL FUNCTION
A7 = V of D minor
```

Local-center states:

```text
candidate
    ↓
tonicized / temporary
    ↓
established local center
    ↓
modulationCandidate
```

`modulationCandidate` — гипотеза, а не автоматическая смена global key.

## Ambiguity / Confidence

`HarmonicSituation` может хранить несколько interpretations:

```text
Global interpretation
Local-center interpretation
Modal-interchange interpretation
```

Семантика:

```text
UNIQUE
→ evidence достаточно для primary interpretation

AMBIGUOUS
→ несколько правдоподобных трактовок существуют параллельно
→ primary остаётся unresolved
```

Примеры:

```text
C major / Fm7
→ Global: chromatic
→ Modal interchange: C minor
→ AMBIGUOUS
```

```text
F major / Em7b5 → A7 → Dm
→ Local D minor established
→ Local interpretation становится unique primary
```

`0.2e fix1` сохранил enharmonic spelling локальных центров через `rootFifths`, поэтому F# major и Gb major больше не схлопываются в одну diagnostic spelling.

## Что уже умеет Harmonic Engine

К началу `0.2f` приняты:

- basic harmonic functions;
- major `ii–V–I` на `ii / V / I`;
- minor `iiø–V–i` на `iiø / V / i`;
- `V–I`;
- `I–VI–ii–V`;
- secondary dominants;
- dominant chains;
- ordinary `V7` / `SubV7`;
- major/minor `ii–SubV–I`;
- applied SubV;
- guide-tone resolution;
- candidate / tonicized / established local centers;
- remote local centers;
- `localHarmonic` / `localPattern`;
- cautious `modulationCandidate`;
- fixed-size `HarmonicInterpretation` candidates;
- unique / ambiguous state;
- global/local conflict evidence;
- borrowed/modal ambiguity;
- diagnostic UI со списком candidates и primary;
- enharmonic-aware KeyCenter display.

## 0.2f — Integration / musical validation

`0.2f` не добавляет новый крупный музыкальный слой. Его задача — проверить все принятые части Stage 2 **вместе**.

Добавлен отдельный regression target:

```text
SmartImproviserIntegrationValidationTests
```

Он перемещает окно `previous/current/next` по длинным progression cases:

```text
Cmaj7 → A7 → Dm7 → G7 → Cmaj7
Cmaj7 → Fm7 → G7 → Cmaj7
Em7b5 → Eb7 → Dm → G7 → Cmaj7
C#7 → F#maj7 → Bmaj7
Dm7 → G7 → Abmaj7   // contradictory-next guard
```

Проверяются переходы:

```text
Global → Local primary → Global
Unique → Ambiguous → Unique
Tonicized → Modulation candidate
```

Также `0.2f` закрыл integration false-positive: конец `I–VI–ii–V` по паре `ii→V` теперь считается boundary candidate только когда `next` действительно недоступен. Если `next` уже известен и противоречит ожидаемому разрешению, pattern не придумывается.

Версия checkpoint:

```text
Build label: Smart Improviser 0.2f
CMake:      0.2.8
Artifact:   Smart-Improviser-0.2f-Windows
Package:    Smart Improviser.vst3
```

## Правило разработки Stage

```text
новая буква = новая функциональная часть Stage
fixN        = исправление текущего checkpoint
версия без буквы = весь Stage завершён
```

Stage 2:

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACCEPTED]
0.2c — Tritone Substitution                [ACCEPTED]
0.2d — Local Key Center                    [ACCEPTED]
0.2e — Ambiguity / Confidence              [ACCEPTED]
0.2e fix1 — Enharmonic spelling            [ACCEPTED]
0.2f — Integration / musical validation    [ACTIVE]
0.3  — Stage 2 complete
```

## Tension Engine

Следующий крупный слой после Harmonic Engine — три уровня напряжения:

- **Tension 1 — Stable:** chord tones, guide tones, устойчивые extensions.
- **Tension 2 — Color:** chromatic approaches, enclosures, melodic-minor applications, upper structures.
- **Tension 3 — Outside / Maximum:** altered/diminished language, substitutions, side slipping, superimposed harmony и delayed resolution.

В дальнейшем tension должен работать и как **Tension Curve** для нескольких тактов или chorus.

## Долгосрочное направление

Smart Improviser должен объединить harmonic analysis, local tonal centers, ambiguity/confidence, tension, target notes, jazz vocabulary, Phrase Library, functional transpose, fretboard/notation/TAB и драматургию импровизации.

Цель проекта — не генерировать музыку вместо музыканта, а помогать **понимать гармонический контекст, управлять напряжением и превращать изученный vocabulary в собственный музыкальный язык**.

## Документация

- [`docs/CURRENT_STATE.md`](docs/CURRENT_STATE.md) — текущий checkpoint и следующий шаг.
- [`docs/STAGE_1_TO_STAGE_2_CONTRACT.md`](docs/STAGE_1_TO_STAGE_2_CONTRACT.md) — граница Stage 1 → Harmonic Engine.
- [`docs/CORE_DATA_MODEL_0.0b.md`](docs/CORE_DATA_MODEL_0.0b.md) — host-neutral data model.
- [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md) — living document проекта.
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — Stage и build checkpoints.
- [`docs/VERSIONING.md`](docs/VERSIONING.md) — схема версий и fix-сборок.
- [`docs/ARCHITECTURAL_DECISIONS.md`](docs/ARCHITECTURAL_DECISIONS.md) — архитектурные решения.

## Ближайший технический шаг

Довести CI PR #20 до зелёного состояния, затем провести live musical validation `0.2f` в Fender Studio Pro и подготовить стабильную `0.3`.
