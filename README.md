# Smart Improviser

**Smart Improviser** — VST3/ARA 2 плагин для DAW, который помогает гитаристу строить джазовую импровизацию на основе **гармонического контекста**, а не только текущего аккорда.

> Главная идея: плагин должен отвечать не только на вопрос «что здесь можно сыграть?», но и «что музыкально уместно сыграть именно здесь, зачем и с какой степенью напряжения?»

## Статус

**Stage 0 — Foundation / Specification завершён.**  
**Stage 1 — ARA Context Monitor завершён.**  
**Активный Stage:** Stage 2 — Harmonic Engine.

**Текущая стабильная версия:** `0.2`  
**Текущая рабочая версия:** `0.2a`  
**Активная ветка:** `stage-2-harmonic-engine`  
**Stage 2 Issue:** #3  
**Stage 2 PR:** #15

В `0.2` завершён ARA/context pipeline: Fender Studio Pro передаёт host-neutral `TimelineHarmonicSnapshot` с current/previous/next chord, global key, tempo/time-signature и transport context.

В `0.2a` начинается Harmonic Engine: отдельный host-neutral analyzer поверх `HarmonicSituation`, первый pattern recognition и evidence-backed temporary local center.

Первая целевая среда:

- **DAW:** Fender Studio Pro
- **Интеграция:** ARA 2
- **Формат:** VST3
- **Платформа:** Windows
- **Основной сценарий:** джазовая импровизация на гитаре

## Ключевая особенность

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

- общую и локальную тональность;
- текущий, предыдущий и следующий аккорды;
- функцию аккорда;
- гармонический оборот;
- положение аккорда внутри оборота;
- ожидаемое разрешение;
- guide tones и target notes;
- допустимые tensions;
- степень музыкального напряжения;
- подходящие стратегии импровизации.

## Tension Engine

Одна из главных идей проекта — три уровня напряжения:

- **Tension 1 — Stable:** chord tones, guide tones, устойчивые extensions и ясное проведение гармонии.
- **Tension 2 — Color:** хроматические подходы, enclosures, melodic minor applications, upper structures и контролируемые alterations.
- **Tension 3 — Outside / Maximum:** altered, diminished language, substitutions, side slipping, superimposed harmony и delayed resolution.

В дальнейшем tension должен работать не только на уровне отдельного аккорда, но и как **Tension Curve** для нескольких тактов или целого chorus.

## Smart Improviser Core

Музыкальное ядро отделено от DAW, ARA, VST3 и UI.

```text
Fender Studio Pro
        ↓
ARA 2 Adapter
        ↓
Timeline Context
        ↓
TimelineHarmonicSnapshot
        ↓
buildHarmonicSituation()
        ↓
analyzeHarmonicSituation()
        ↓
HarmonicSituation
        ↓
Smart Improviser Core analyzers
        ├── Harmonic Engine
        ├── Harmonic Pattern Recognizer
        ├── Resolution Analyzer
        ├── Tension Engine
        ├── Improvisation Strategy Engine
        ├── Phrase Library
        ├── Phrase Transformation Engine
        └── Explanation Engine
        ↓
UI / Fretboard / Notation / TAB
```

Центральная сущность ядра — **`HarmonicSituation`**, которая описывает не просто аккорд, а его функцию и положение в музыкальном контексте.

## Stage 2 / 0.2a

Первый подэтап Harmonic Engine включает:

- отдельный host-neutral `HarmonicEngine`;
- анализ current chord относительно global key;
- использование previous/current/next context;
- major `ii–V–I` для текущей позиции V;
- minor `iiø–V–i` для текущей позиции V;
- `V–I` fallback;
- secondary dominant pattern;
- temporary local center только при подтверждённом applied dominant;
- regression tests для Harmonic Engine.

Следующие подэтапы Stage 2 должны добавить:

- pattern positions для всех членов оборота;
- `I–VI–ii–V`;
- tritone substitution / `ii–SubV–I`;
- более длинные local-center/tonicization rules;
- ambiguity и alternative interpretations.

## Долгосрочное направление

В зрелом состоянии Smart Improviser должен объединить:

- гармонический анализ;
- три уровня tension;
- target notes и resolution logic;
- библиотеку классического jazz vocabulary;
- семантическое хранение фраз;
- транспонирование и функциональный перенос фраз;
- перенос major ↔ minor;
- адаптацию V7 ↔ SubV7;
- построение фраз от разных ступеней;
- автоматические approach notes и enclosures;
- fretboard / notation / TAB;
- пользовательскую библиотеку фраз;
- Phrase Transformation Engine;
- планирование драматургии импровизации.

Цель проекта — не генерировать музыку вместо музыканта, а помогать **понимать гармонический контекст, управлять напряжением и превращать изученный vocabulary в собственный музыкальный язык**.

## Версионирование

```text
Stage 0: 0.0a → ... → 0.1
Stage 1: 0.1a → ... → 0.2
Stage 2: 0.2a → ... → 0.3
```

Если после живого теста конкретной буквенной версии требуется исправление:

```text
0.2a fix1
0.2a fix2
```

Подробно: [`docs/VERSIONING.md`](docs/VERSIONING.md).

## Документация

- [`docs/CURRENT_STATE.md`](docs/CURRENT_STATE.md) — короткая точка входа: текущая версия, активный Stage и что делать дальше.
- [`docs/STAGE_1_TO_STAGE_2_CONTRACT.md`](docs/STAGE_1_TO_STAGE_2_CONTRACT.md) — формальная граница ARA/context layer → Harmonic Engine.
- [`docs/CORE_DATA_MODEL_0.0b.md`](docs/CORE_DATA_MODEL_0.0b.md) — спецификация host-neutral data model.
- [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md) — основной living document проекта и архитектурный контекст.
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — этапы разработки от спецификации ядра до Improvisation Planner.
- [`docs/VERSIONING.md`](docs/VERSIONING.md) — официальная схема версий, буквенных подэтапов и fix-сборок.
- [`docs/ARCHITECTURAL_DECISIONS.md`](docs/ARCHITECTURAL_DECISIONS.md) — журнал ключевых архитектурных решений.
- [`docs/MIGRATION_FROM_SMART_VOICING.md`](docs/MIGRATION_FROM_SMART_VOICING.md) — границы переноса компонентов из Smart Voicing.

## Ближайший технический шаг

Завершить CI/проверку `0.2a`, затем продолжить Stage 2 расширением pattern-position analysis, turnaround/tritone-substitution и ambiguity/local-center logic.
