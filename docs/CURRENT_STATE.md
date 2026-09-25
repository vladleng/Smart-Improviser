# Smart Improviser — Current State

> Обновлено 2026-09-25: `0.3a–0.3f` и `0.3f fix1` приняты. Текущий checkpoint — **`0.3g — Explanation / usable output`**. Последняя stable — `0.3`.

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённые Stage:** Stage 0, Stage 1, Stage 2
- **Текущая стабильная версия:** `0.3`
- **Текущий Stage:** Stage 3 — Improvisation Engine
- **Рабочая линия:** `0.3a → 0.3x`
- **Итог Stage 3:** `0.4`
- **Stage 2 Issue:** #3 — закрыт стабильной `0.3`
- **Stage 3 Issue:** #4 — активен
- **Принято Stage 3:** `0.3a–0.3f` + `0.3f fix1`
- **PR #30:** merged — базовый `0.3f`
- **PR #31:** merged — `0.3f fix1`, squash commit `1117502220887fe05e294b3b6a4af3037c7b5d88`
- **Windows Build #310:** success; финальный code/live gate fix1
- **Текущий checkpoint:** `0.3g — Explanation / usable output`
- **После 0.3g:** `0.3h — Integration / musical validation`

## Архитектурная граница

Stage 1 contract остаётся закрытым:

```text
previous chord
current chord
next chord
global key
```

Для длиннее-живущего harmonic context Stage 3 использует отдельный host-neutral `PatternContext / RecognizedPatternInstance`, реконструируемый детерминированно из bounded `PatternTimelineWindow`. Это не arbitrary history и не расширение Stage 1 до `previousN`.

Project key в DAW автоматически не меняется. Core не зависит от JUCE / ARA / Fender Studio Pro.

```text
TimelineHarmonicSnapshot
        ↓
Global harmonic analysis
        ↓
Pattern Recognizer
        ↓
PatternContext / recognized pattern continuity
        ↓
Tritone Substitution
        ↓
Local Key Center Analyzer
        ↓
Ambiguity / Confidence Analyzer
        ↓
HarmonicSituation
        ↓
Improvisation Engine
        ↓
Context-aware strategy ranking
```

## Stable 0.3 — Stage 2 complete

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

## Stage 3 / Improvisation Engine

Stage 3 использует готовый `HarmonicSituation` и выдаёт chord tones, guide tones, target notes, scales/source material, harmonic concepts, resolution notes, improvisation strategies и context-specific alternatives с provenance по harmonic interpretation.

Рабочая декомпозиция: [STAGE_3_PLAN.md](STAGE_3_PLAN.md). Методика: [IMPROVISATION_METHOD.md](IMPROVISATION_METHOD.md). Основной первичный источник текущего метода — Игорь Бойко, «Мой метод», раздел 2 (2010); проектные правила Smart Improviser отделяются от авторской методики.

## Принятые checkpoints Stage 3

### 0.3a — Strategy contract / foundation
Windows Build #231 — success; live-test принят 2026-09-24; PR #23 merged.

### 0.3b — Structural tones / targets / resolution
Windows Build #234 — success; PR #24 merged.

### 0.3c — Diatonic / modal sources
Windows Build #237 — success; принят 2026-09-25; PR #25 merged.

### 0.3d — Melodic minor / diminished sources
Windows Build #246 — success; live-test принят 2026-09-25; PR #26 merged.

### 0.3e + 0.3e-ui — Harmonic concepts / diagnostic UI
Windows Build #274 — success; принято 2026-09-25; PR #28 merged.

### 0.3f — Context-aware ranking / ambiguity [ACCEPTED]
Windows Build #284 — success; live-test принят 2026-09-25; PR #30 merged; Issue #29 закрыт.

Реализованы context-dependent alternatives для global/local/modal interpretations, явная unresolved ambiguity без hidden winner, deterministic ranking отдельно от harmonic confidence, minor `iv–V–i`, safe generic `V→I/i` fallback и защита от false-positive `Fm7→G7→Cmaj7`.

### 0.3f fix1 — Pattern continuity / hierarchical cadence context [ACCEPTED]
Windows Build #310 — success; финальный live-test принят Владом 2026-09-25; PR #31 merged.

Реализовано:

- bounded deterministic `PatternTimelineWindow` и host-neutral `PatternContext` без runtime stale-history;
- continuity `ii–V–I`, `iiø–V–i`, `iv–V–i` до resolution member `3/3`;
- generic `V→I/i` сохраняется, если полного carried evidence нет;
- единый top-level `iii–VI7–ii–V–I • 1/5…5/5`;
- nested evidence `V/ii→ii` и `ii–V–I` не дробит основной каданс;
- direct seek/chord edits/reopen пересчитывают context;
- STOP-boundary snap исправляет рассинхрон визуальной границы Studio Pro и ARA chord-event, а PLAY остаётся strict по реальной позиции;
- Stage 1 contract `previous/current/next` не изменён.

Acceptance: [STAGE_3_0.3f_FIX1_PLAN.md](STAGE_3_0.3f_FIX1_PLAN.md).

## Текущий checkpoint — 0.3g

`0.3g — Explanation / usable output` должен завершить объяснимую пользовательскую цепочку:

```text
идея → source → важные ноты → target/resolution → почему
```

Основные задачи:

- deterministic explanations из уже рассчитанного Core/evidence;
- Why?-data без повторного harmonic analysis в UI;
- presentation-layer схлопывает одинаковый музыкальный материал из нескольких interpretations, сохраняя provenance;
- top-level/nested PatternContext используется как источник объяснения;
- T1/T2/T3 policy остаётся Stage 4.

## Что читать при продолжении

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_3_PLAN.md`;
3. `docs/STAGE_3_0.3f_FIX1_PLAN.md` — принятый refinement;
4. `docs/STAGE_3_0.3f_LIVE_TEST.md` — принятый базовый 0.3f;
5. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
6. `docs/ARCHITECTURAL_DECISIONS.md`;
7. `docs/IMPROVISATION_METHOD.md`;
8. Issue #4 — Stage 3.
