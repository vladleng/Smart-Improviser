# Smart Improviser — Current State

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённые Stage:** Stage 0, Stage 1, Stage 2
- **Текущая стабильная версия:** `0.3`
- **Последний завершённый Stage:** Stage 2 — Harmonic Engine
- **Следующий Stage:** Stage 3 — Improvisation Engine
- **Следующая рабочая линия:** `0.3a → 0.3x`
- **Итог Stage 3:** `0.4`
- **Stage 2 Issue:** #3 — закрывается стабильной `0.3`

`0.3` — релизное закрытие Stage 2. Новая музыкальная логика относительно принятого `0.2f` не добавлялась: stable release фиксирует уже проверенное состояние Harmonic Engine.

## Архитектурная граница

Stage 1 contract остаётся закрытым. Harmonic Engine получает только host-neutral timeline context:

```text
previous chord
current chord
next chord
global key
```

Project key в DAW автоматически не меняется. Core не зависит от JUCE / ARA / Fender Studio Pro.

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

## Stable 0.3 — Stage 2 complete

В стабильную `0.3` входят все принятые checkpoints:

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACCEPTED]
0.2c — Tritone Substitution                [ACCEPTED]
0.2d — Local Key Center                    [ACCEPTED]
0.2e — Ambiguity / Confidence              [ACCEPTED]
0.2e fix1 — Enharmonic spelling            [ACCEPTED]
0.2f — Integration / musical validation    [ACCEPTED]
0.3  — Stage 2 complete                    [STABLE]
```

### Возможности Harmonic Engine

- basic harmonic functions;
- major `ii–V–I` на `ii / V / I`;
- minor `iiø–V–i` на `iiø / V / i`;
- `V–I`;
- `I–VI–ii–V`;
- secondary dominants и dominant chains;
- ordinary `V7` / `SubV7`;
- major/minor `ii–SubV–I`;
- applied SubV и guide-tone resolution;
- local key center: `candidate / tonicized / established / modulationCandidate`;
- remote local centers;
- global/local harmonic interpretations;
- borrowed/modal ambiguity;
- `unique / ambiguous` semantics и explicit primary interpretation;
- enharmonic-aware KeyCenter display через `rootFifths`;
- boundary false-positive guards;
- diagnostic UI для global/local/ambiguity/confidence.

### Integration validation

`0.2f` добавил отдельный regression target:

```text
SmartImproviserIntegrationValidationTests
```

Live-tested в Fender Studio Pro:

```text
Cmaj7 → A7 → Dm7 → G7 → Cmaj7
Cmaj7 → Fm7 → G7 → Cmaj7
Em7b5 → Eb7 → Dm → G7 → Cmaj7
C#7 → F#maj7 → Bmaj7
Dm7 → G7 → Abmaj7
```

Подтверждены переходы:

```text
Global → Local primary → Global
Unique → Ambiguous → Unique
Tonicized → Modulation candidate
```

Stage 1 / ARA regression не обнаружен.

## Версия 0.3

```text
Build label: Smart Improviser 0.3
CMake:      0.3.0
Artifact:   Smart-Improviser-0.3-Windows
Package:    Smart Improviser.vst3
Tests:      7 regression/integration targets
```

## Следующий этап — Stage 3 / Improvisation Engine

Stage 3 должен использовать готовый `HarmonicSituation` и выдавать музыкально применимый материал для импровизации:

- chord tones;
- guide tones;
- target notes;
- scales;
- harmonic concepts;
- resolution notes;
- базовые improvisation strategies.

Перед началом реализации Stage 3 необходимо отдельно разложить его на буквенные checkpoints `0.3a`, `0.3b`, `0.3c`, ... .

## Что читать при переходе к Stage 3

1. `docs/CURRENT_STATE.md`;
2. `docs/ROADMAP.md`;
3. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/VERSIONING.md`;
7. Issue #3 — история завершённого Stage 2.
