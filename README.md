# Smart Improviser

**Smart Improviser** — VST3/ARA 2 плагин для DAW, который помогает гитаристу строить джазовую импровизацию на основе **гармонического контекста**, а не только текущего аккорда.

> Главная идея: отвечать не только на вопрос «что здесь можно сыграть?», но и «что музыкально уместно сыграть именно здесь, зачем и с какой степенью напряжения?»

## Статус

**Stage 0 — Foundation / Specification завершён.**  
**Stage 1 — ARA Context Monitor завершён.**  
**Активный Stage:** Stage 2 — Harmonic Engine.

**Текущая стабильная версия:** `0.2`  
**Последний принятый checkpoint:** `0.2b — Pattern Recognizer`  
**Текущая рабочая версия:** `0.2c — Tritone Substitution`  
**Активная ветка:** `stage-2-tritone-substitution`  
**Stage 2 Issue:** #3  
**Stage 2 PR:** #17

`0.2b` принят после CI и live-test в Fender Studio Pro и слит в `main` через PR #16.

`0.2c` добавляет понимание tritone substitution: ordinary `V7` и `SubV7`, `ii–SubV–I`, applied SubV и отдельную guide-tone resolution logic.

Первая целевая среда:

- **DAW:** Fender Studio Pro
- **Интеграция:** ARA 2
- **Формат:** VST3
- **Платформа:** Windows
- **Основной сценарий:** джазовая импровизация на гитаре

## Ключевая идея

Вместо простой схемы:

```text
Chord → Scale
```

Smart Improviser строится вокруг цепочки:

```text
Harmony
→ Context
→ Function
→ Harmonic Pattern
→ Tension
→ Strategy
→ Resolution
→ Phrase / Vocabulary
```

Плагин должен понимать:

- global key и local tonal center;
- current / previous / next chord;
- harmonic function;
- harmonic pattern;
- положение аккорда внутри оборота;
- ordinary dominant / secondary dominant / substitute dominant;
- ожидаемое разрешение;
- guide tones и target notes;
- tensions и степень музыкального напряжения;
- подходящие стратегии импровизации.

## Архитектура

Музыкальное ядро отделено от DAW, ARA, VST3 и UI.

```text
Fender Studio Pro
        ↓
ARA 2 Adapter
        ↓
TimelineHarmonicSnapshot
(previous / current / next)
        ↓
buildHarmonicSituation()
        ↓
analyzeHarmonicSituation()
        ↓
HarmonicSituation
        ↓
Smart Improviser Core analyzers
        ├── Harmonic Engine
        ├── Pattern Recognizer
        ├── Resolution Analyzer
        ├── Local Key Center
        ├── Tension Engine
        ├── Improvisation Strategy Engine
        ├── Phrase Library
        └── Explanation Engine
        ↓
UI / Fretboard / Notation / TAB
```

Центральная сущность ядра — **`HarmonicSituation`**. Stage 1 contract закрыт: Harmonic Engine получает только host-neutral timeline snapshot и не зависит от JUCE/ARA/Fender Studio Pro.

## Правило разработки Stage

Каждый Stage делится на логически завершённые подэтапы. Каждому соответствует отдельная буквенная build-версия.

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
0.2c — Tritone Substitution                [ACTIVE]
0.2d — Local Key Center
0.2e — Ambiguity / Confidence
0.2f — Integration / musical validation
0.3  — Stage 2 complete
```

## Что уже умеет Harmonic Engine

К принятому `0.2b` реализованы:

- basic harmonic functions;
- major `ii–V–I` на позициях `ii / V / I`;
- minor `iiø–V–i` на позициях `iiø / V / i`;
- `I–VI–ii–V`;
- secondary dominants;
- dominant chains;
- pattern role / position;
- evidence-aware confidence;
- temporary local center для подтверждённого applied dominant;
- live diagnostic UI.

## 0.2c — Tritone Substitution

Рабочая версия добавляет:

- `HarmonicFunction::substituteDominant`;
- distinction `V7` / `SubV7`;
- major `ii–SubV–I`;
- minor `iiø–SubV–i`;
- boundary pattern positions;
- applied SubV, например `Ab7 → G` в C major;
- guide-tone resolution для SubV;
- regression protection от ошибочной трактовки SubV как secondary dominant.

Основной тест:

```text
C major
Dm7 → Db7 → Cmaj7

Db7:
Function = Substitute dominant
Pattern = Tritone substitution
Role = Substitute dominant | 2 / 3
Resolution = Cmaj7 | CONFIRMED
```

## Local Key Center — следующий шаг

После принятия `0.2c` checkpoint `0.2d` должен научить движок автоматически определять локальные/субтональные центры **без необходимости менять project key в DAW**.

Предполагаемая модель:

```text
GLOBAL KEY
F major
    ↓
LOCAL / TEMPORARY CENTER
D minor
    ↓
CURRENT FUNCTION
A7 = V of D minor
```

Local-center engine должен различать candidate center, temporary tonicization, устойчивый local center и настоящую modulation. Evidence будут давать `ii–V`, `iiø–V`, secondary dominants, SubV и реальные resolution.

## Tension Engine

Одна из главных идей проекта — три уровня напряжения:

- **Tension 1 — Stable:** chord tones, guide tones, устойчивые extensions и ясное проведение гармонии.
- **Tension 2 — Color:** chromatic approaches, enclosures, melodic-minor applications, upper structures и контролируемые alterations.
- **Tension 3 — Outside / Maximum:** altered/diminished language, substitutions, side slipping, superimposed harmony и delayed resolution.

В дальнейшем tension должен работать и как **Tension Curve** для нескольких тактов или chorus.

## Долгосрочное направление

Smart Improviser должен объединить:

- гармонический анализ;
- local tonal-center detection;
- три уровня tension;
- target notes и resolution logic;
- jazz vocabulary;
- семантическое хранение фраз;
- functional transpose;
- major ↔ minor adaptation;
- V7 ↔ SubV7 adaptation;
- approach notes и enclosures;
- fretboard / notation / TAB;
- пользовательскую Phrase Library;
- Phrase Transformation Engine;
- планирование драматургии импровизации.

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

Дождаться CI PR #17, установить `Smart-Improviser-0.2c-Windows` и провести live-test tritone substitution перед переходом к `0.2d — Local Key Center`.
