# Smart Improviser

**Smart Improviser** — VST3/ARA 2 плагин для DAW, который помогает гитаристу строить джазовую импровизацию на основе **гармонического контекста**, а не только текущего аккорда.

> Главная идея: плагин должен отвечать не только на вопрос «что здесь можно сыграть?», но и «что музыкально уместно сыграть именно здесь, зачем и с какой степенью напряжения?»

## Статус

**Stage 0 — Foundation / Specification завершён.**  
**Stage 1 — ARA Context Monitor завершён.**  
**Активный Stage:** Stage 2 — Harmonic Engine.

**Текущая стабильная версия:** `0.2`  
**Последний принятый checkpoint:** `0.2a / 0.2a fix1`  
**Текущая рабочая версия:** `0.2b — Pattern Recognizer`  
**Активная ветка:** `stage-2-pattern-recognizer`  
**Stage 2 Issue:** #3  
**Stage 2 PR:** #16

`0.2a / 0.2a fix1` принят после CI и live-test в Fender Studio Pro и слит в `main`.

`0.2b` расширяет Harmonic Engine до полноценного базового Pattern Recognizer, сохраняя закрытый Stage 1 contract `previous / current / next`.

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

## Правило разработки Stage

Каждый Stage делится на отдельные **логически завершённые подэтапы**, и каждому подэтапу соответствует собственная буквенная build-версия.

```text
новая буква = новая функциональная часть Stage
fixN        = исправление текущего подэтапа
версия без буквы = весь Stage завершён
```

Буквенная версия — отдельный build checkpoint с собственной целью, checklist и тестами.

Stage 2:

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACTIVE]
0.2c — Tritone Substitution
0.2d — Local Key Center
0.2e — Ambiguity / Confidence
0.2f — Integration / musical validation
0.3  — Stage 2 complete
```

Подробные правила: [`docs/VERSIONING.md`](docs/VERSIONING.md). Актуальная декомпозиция: [`docs/ROADMAP.md`](docs/ROADMAP.md).

## Принятый checkpoint 0.2a

В `0.2a / 0.2a fix1` реализованы и подтверждены живым тестом:

- отдельный host-neutral `HarmonicEngine`;
- анализ current chord относительно global key;
- previous/current/next context;
- basic harmonic functions;
- major `ii–V–I` для current V;
- minor `iiø–V–i` для current V;
- `V–I` fallback;
- secondary dominant;
- evidence-backed temporary local center;
- Stage 2 diagnostic UI;
- regression tests.

## Stage 2 / 0.2b — Pattern Recognizer

`0.2b` не расширяет Stage 1 ARA/context contract. Pattern Recognizer работает только на host-neutral окне:

```text
previous → current → next
```

Если полного оборота в окне нет, движок не выдумывает отсутствующие аккорды. Confidence отражает фактический объём evidence.

Реализуется:

- major `ii–V–I` для позиций `ii / V / I`;
- minor `iiø–V–i` для позиций `iiø / V / i`;
- `I–VI–ii–V`;
- dominant / secondary-dominant chains;
- pattern role и position;
- pattern evidence;
- confidence для полного и boundary context;
- boundary false-positive guards;
- расширенные regression tests.

### Confidence policy

```text
полный ii–V–I на current V        → confirmed
boundary ii или I по сильной паре → high
I–VI–ii–V, внутреннее окно        → high
turnaround boundary pair          → medium
```

Примеры:

```text
Dm7 → G7 → Cmaj7, current G7
Pattern: Major ii-V-I
Position: Dominant | 2 / 3
Confidence: confirmed
```

```text
Dm7 → G7, current Dm7
Pattern: Major ii-V-I
Position: Predominant | 1 / 3
Confidence: high
```

```text
A7 → D7 → G7, current D7
Pattern: Dominant chain
Position: Dominant | 2 / 3
Confidence: confirmed
```

## Tension Engine

Одна из главных идей проекта — три уровня напряжения:

- **Tension 1 — Stable:** chord tones, guide tones, устойчивые extensions и ясное проведение гармонии.
- **Tension 2 — Color:** хроматические подходы, enclosures, melodic minor applications, upper structures и контролируемые alterations.
- **Tension 3 — Outside / Maximum:** altered, diminished language, substitutions, side slipping, superimposed harmony и delayed resolution.

В дальнейшем tension должен работать не только на уровне отдельного аккорда, но и как **Tension Curve** для нескольких тактов или целого chorus.

## Долгосрочное направление

Smart Improviser должен объединить:

- гармонический анализ;
- три уровня tension;
- target notes и resolution logic;
- библиотеку jazz vocabulary;
- семантическое хранение фраз;
- functional transpose;
- major ↔ minor adaptation;
- V7 ↔ SubV7 adaptation;
- approach notes и enclosures;
- fretboard / notation / TAB;
- пользовательскую библиотеку фраз;
- Phrase Transformation Engine;
- планирование драматургии импровизации.

Цель проекта — не генерировать музыку вместо музыканта, а помогать **понимать гармонический контекст, управлять напряжением и превращать изученный vocabulary в собственный музыкальный язык**.

## Версионирование

```text
Stage 0: 0.0a → ... → 0.1
Stage 1: 0.1a → ... → 0.2
Stage 2: 0.2a → 0.2a fix1 → 0.2b → 0.2c → ... → 0.3
```

`fixN` исправляет существующий подэтап и не используется вместо новой буквенной версии.

Подробно: [`docs/VERSIONING.md`](docs/VERSIONING.md).

## Документация

- [`docs/CURRENT_STATE.md`](docs/CURRENT_STATE.md) — текущая версия, активный Stage и следующий шаг.
- [`docs/STAGE_1_TO_STAGE_2_CONTRACT.md`](docs/STAGE_1_TO_STAGE_2_CONTRACT.md) — граница ARA/context layer → Harmonic Engine.
- [`docs/CORE_DATA_MODEL_0.0b.md`](docs/CORE_DATA_MODEL_0.0b.md) — host-neutral data model.
- [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md) — основной living document.
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — Stage и build checkpoints.
- [`docs/VERSIONING.md`](docs/VERSIONING.md) — схема версий и fix-сборок.
- [`docs/ARCHITECTURAL_DECISIONS.md`](docs/ARCHITECTURAL_DECISIONS.md) — архитектурные решения.
- [`docs/MIGRATION_FROM_SMART_VOICING.md`](docs/MIGRATION_FROM_SMART_VOICING.md) — границы переноса компонентов.

## Ближайший технический шаг

Завершить CI и live-test `0.2b — Pattern Recognizer`, принять checkpoint и только после этого перейти к `0.2c — Tritone Substitution`.
