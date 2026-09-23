# Smart Improviser

**Smart Improviser** — VST3/ARA 2 плагин для DAW, который помогает гитаристу строить джазовую импровизацию на основе **гармонического контекста**, а не только текущего аккорда.

> Главная идея: плагин должен отвечать не только на вопрос «что здесь можно сыграть?», но и «что музыкально уместно сыграть именно здесь, зачем и с какой степенью напряжения?»

## Статус

Проект находится на **Stage 0 — Foundation / Specification**.

**Текущая рабочая версия:** `0.0a`  
**Итог Stage 0:** `0.1`

На текущем этапе уже перенесён чистый Harmony Core и ARA/context foundation из Smart Voicing без voicing-specific кода. Следующая задача — завершить собственную data model Smart Improviser Core.

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

Музыкальное ядро должно быть отделено от DAW, ARA, VST3 и UI.

```text
Fender Studio Pro
        ↓
ARA 2 Adapter
        ↓
Timeline Context
        ↓
Smart Improviser Core
        ├── Harmonic Analyzer
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

Рабочая версия меняет букву после завершения отдельного подэтапа внутри Stage:

```text
0.0a → 0.0b → 0.0c → ... → 0.1
```

Если после живого теста конкретной буквенной версии требуется исправление:

```text
0.0b fix1
0.0b fix2
```

Подробно: [`docs/VERSIONING.md`](docs/VERSIONING.md).

## Документация

- [`docs/CURRENT_STATE.md`](docs/CURRENT_STATE.md) — короткая точка входа: текущая версия, активный Stage, PR, ветка, что уже сделано и что делать следующим.
- [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md) — основной living document проекта и архитектурный контекст.
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — этапы разработки от спецификации ядра до Improvisation Planner.
- [`docs/VERSIONING.md`](docs/VERSIONING.md) — официальная схема версий, буквенных подэтапов и fix-сборок.
- [`docs/ARCHITECTURAL_DECISIONS.md`](docs/ARCHITECTURAL_DECISIONS.md) — журнал ключевых архитектурных решений.
- [`docs/MIGRATION_FROM_SMART_VOICING.md`](docs/MIGRATION_FROM_SMART_VOICING.md) — границы переноса компонентов из Smart Voicing.

## Ближайший технический этап

Текущая задача Stage 0 — закончить формальное описание `Chord`, `KeyCenter`, `HarmonicFunction`, `HarmonicPattern`, `HarmonicSituation`, `ResolutionTarget`, `TensionLevel`, `ImprovisationStrategy` и `Phrase`.

После полного закрытия Stage 0 проект получает стабильную версию `0.1`, а Stage 1 начинается с `0.1a`.
