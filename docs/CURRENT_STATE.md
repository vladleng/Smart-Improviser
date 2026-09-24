# Smart Improviser — Current State

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Последний принятый checkpoint:** `0.2e / 0.2e fix1 — Ambiguity / Confidence`
- **Следующий checkpoint:** `0.2f — Integration / musical validation`
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **Принятый PR:** #19 — `0.2e fix1 — Ambiguity / Confidence + enharmonic spelling`

`0.2e / 0.2e fix1` принят после успешного Windows CI и полного live-test в Fender Studio Pro. Все заявленные проверки ambiguity/confidence пройдены; `0.2e fix1` дополнительно подтвердил корректное enharmonic spelling локальных центров (`F# major`, а не `Gb major`). PR #19 слит в `main`.

## Архитектурная граница

Stage 1 остаётся закрытым. Harmonic Engine по-прежнему получает только host-neutral:

```text
previous chord
current chord
next chord
global key
```

Project key в DAW автоматически не меняется.

Цепочка Stage 2:

```text
TimelineHarmonicSnapshot
        ↓
Global harmonic analysis
        ↓
Pattern Recognizer
        ↓
Tritone Substitution
        ↓
Local Key Center Analyzer
        ↓
Ambiguity / Confidence Analyzer
        ↓
HarmonicSituation
```

Core остаётся независимым от JUCE / ARA / Fender Studio Pro.

## Принятый checkpoint 0.2e — Ambiguity / Confidence

Главная задача: Harmonic Engine не должен выдавать единственную трактовку там, где данных ещё недостаточно.

`HarmonicSituation` хранит фиксированный набор interpretation candidates:

```text
Global interpretation
Local-center interpretation
Modal-interchange interpretation
```

Максимум — 4 candidates без dynamic allocation.

### Semantics

```text
UNIQUE
→ evidence достаточно для primary interpretation

AMBIGUOUS
→ одновременно существуют несколько правдоподобных трактовок
→ primaryInterpretationIndex = -1
```

Подтверждённая локальная каденция или confirmed tonicization может разрешить конфликт и сделать local interpretation primary. Candidate local center и modulationCandidate намеренно не делают этого.

### Реализовано и принято

- `HarmonicInterpretation` model;
- `interpretations[]`, `interpretationCount`, `primaryInterpretationIndex` в `HarmonicSituation`;
- Global candidate;
- Local-center candidate;
- parallel-mode / modal-interchange candidate;
- candidate local center → global/local ambiguity;
- modulationCandidate → ambiguity;
- established / tonicized local center с confirmed evidence → local primary;
- borrowed chord → global chromatic + modal alternative;
- evidence flags `alternativeInterpretation`, `globalLocalConflict`, `borrowedAmbiguity`;
- отдельный `SmartImproviserAmbiguityConfidenceTests`;
- diagnostic UI выводит interpretation state, primary и список candidates;
- enharmonic spelling KeyCenter берётся из `rootFifths` и сохраняет `F#/Gb`, `C#/Db` и т. п.

## Live-test 0.2e / fix1

Проверено в Fender Studio Pro:

### Plain unique

```text
Global key: C major
Current: Cmaj7

Interpretation: unique
Primary: Global
```

### Borrowed/modal ambiguity

```text
Global key: C major
Current: Fm7

Candidate 1: Global / chromatic
Candidate 2: Modal interchange / C minor
Interpretation: AMBIGUOUS
Primary: UNRESOLVED
```

### Candidate local center

```text
Global key: F major
Em7b5 → A7

Local center candidate: D minor
Interpretation: AMBIGUOUS
Primary: UNRESOLVED
```

### Confirmed local cadence

```text
Global key: F major
Em7b5 → A7 → Dm
Current: A7

Local center: D minor | established
Interpretation: unique
Primary: Local center
```

### Possible modulation

```text
Global key: C major
C#7 → F#maj7 → Bmaj7
Current: F#maj7

Local center: F# major | modulation candidate
Interpretation: AMBIGUOUS
Primary: UNRESOLVED
```

### 0.2e fix1 retest

```text
Global key: C major
C#7 → F#maj7
Current: C#7

Resolution: F#maj7 | CONFIRMED
Local center: F# major | temporary | tonicized
Primary: Local center
Candidate Local center: F# major
```

Fix подтверждён: enharmonic spelling сохраняется корректно.

## Версия принятого checkpoint

```text
Build label: Smart Improviser 0.2e fix1
CMake:      0.2.7
Artifact:   Smart-Improviser-0.2e-fix1-Windows
Package:    Smart Improviser.vst3
Fix Build #199: SUCCESS
Final PR HEAD Build #205: SUCCESS
```

## Линия Stage 2

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACCEPTED]
0.2c — Tritone Substitution                [ACCEPTED]
0.2d — Local Key Center                    [ACCEPTED]
0.2e — Ambiguity / Confidence              [ACCEPTED]
0.2e fix1 — Enharmonic spelling            [ACCEPTED]
0.2f — Integration / musical validation    [NEXT]
0.3  — Stage 2 complete
```

## Следующий checkpoint — 0.2f

Цель `0.2f`: проверить Stage 2 как единый Harmonic Engine на комплексных последовательностях перед стабильной `0.3`.

Основные задачи:

1. комплексные harmonic sequences с global/local/borrowed/SubV контекстом;
2. полный regression всех Stage 2 analyzers;
3. проверка conflict/ambiguity transitions на границах паттернов;
4. live musical validation в Fender Studio Pro;
5. финальная подготовка стабильной версии `0.3`.

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue #3 — Stage 2 — Harmonic Engine;
9. PR #19 — принятый и слитый `0.2e / 0.2e fix1`.
