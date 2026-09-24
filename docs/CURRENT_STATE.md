# Smart Improviser — Current State

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Последний принятый checkpoint:** `0.2d — Local Key Center`
- **Текущая рабочая версия:** `0.2e — Ambiguity / Confidence`
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **Активная ветка:** `stage-2-ambiguity-confidence`
- **Активный PR:** #19 — `0.2e — Ambiguity / Confidence`

`0.2d` принят после CI и полного live-test в Fender Studio Pro и слит в `main` через PR #18.

## Архитектурная граница

Stage 1 остаётся закрытым. `0.2e` по-прежнему получает только host-neutral:

```text
previous chord
current chord
next chord
global key
```

Project key в DAW автоматически не меняется.

Цепочка Stage 2 теперь выглядит так:

```text
TimelineHarmonicSnapshot
        ↓
Global harmonic analysis
        ↓
Pattern Recognizer
        ↓
Local Key Center Analyzer
        ↓
Ambiguity / Confidence Analyzer
        ↓
HarmonicSituation
```

Core остаётся независимым от JUCE / ARA / Fender Studio Pro.

## Рабочая 0.2e — Ambiguity / Confidence

Главная задача: Harmonic Engine не должен выдавать единственную трактовку там, где данных ещё недостаточно.

`HarmonicSituation` теперь хранит фиксированный набор interpretation candidates:

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

### Уже реализовано

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
- diagnostic UI выводит interpretation state, primary и список candidates.

## Основные regression cases 0.2e

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
Current: Em7b5

Global interpretation
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

## Версия 0.2e

```text
Build label: Smart Improviser 0.2e
CMake:      0.2.6
Artifact:   Smart-Improviser-0.2e-Windows
Package:    Smart Improviser.vst3
```

## Линия Stage 2

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACCEPTED]
0.2c — Tritone Substitution                [ACCEPTED]
0.2d — Local Key Center                    [ACCEPTED]
0.2e — Ambiguity / Confidence              [ACTIVE]
0.2f — Integration / musical validation
0.3  — Stage 2 complete
```

## Что осталось для принятия 0.2e

1. зелёный Windows CI PR #19;
2. artifact `Smart-Improviser-0.2e-Windows`;
3. live-test borrowed/modal ambiguity;
4. live-test candidate local center;
5. live-test confirmed local primary;
6. live-test modulationCandidate ambiguity;
7. Stage 1 regression.

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue #3 — Stage 2 — Harmonic Engine;
9. PR #19 — `0.2e — Ambiguity / Confidence`.
