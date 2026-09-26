# Smart Improviser — Current State

> Обновлено 2026-09-26: `0.3a–0.3f`, `0.3f fix1` и refinement `fix2–fix4` приняты в итоговом состоянии fix4. Windows Build #331 и live-test fix4 — PASS. Текущий checkpoint — **`0.3g — Explanation / usable output`**. Последняя stable — `0.3`.

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённые Stage:** Stage 0, Stage 1, Stage 2
- **Текущая стабильная версия:** `0.3`
- **Текущий Stage:** Stage 3 — Improvisation Engine
- **Рабочая линия:** `0.3a → 0.3x`
- **Итог Stage 3:** `0.4`
- **Stage 2 Issue:** #3 — закрыт стабильной `0.3`
- **Stage 3 Issue:** #4 — активен
- **Принято Stage 3:** `0.3a–0.3f` + `0.3f fix1` + итоговый refinement `0.3f fix4`
- **PR #30:** merged — базовый `0.3f`
- **PR #31:** merged — `0.3f fix1`, squash commit `1117502220887fe05e294b3b6a4af3037c7b5d88`
- **PR #32:** superseded, closed without merge
- **PR #33:** superseded by fix4, closed without merge
- **PR #34:** accepted fix4, готов к merge
- **Windows Build #331:** success; 14/14 test targets green
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

Для длиннее-живущего harmonic context Stage 3 использует отдельный host-neutral `PatternContext`, реконструируемый детерминированно из bounded `PatternTimelineWindow`. Это не arbitrary history и не расширение Stage 1 до `previousN`.

Refinement 0.3f теперь закрепляет три дополнительных правила evidence/presentation:

1. уже известный future member может опровергнуть provisional cadence candidate;
2. written chord identity отделена от inferred functional alias (`ImpliedDominantReading`);
3. незавершённый шаблон может существовать как `IncompleteCadence`, не превращаясь в local key, confirmed pattern или resolution.

Project key в DAW автоматически не меняется. Core не зависит от JUCE / ARA / Studio Pro.

```text
TimelineHarmonicSnapshot
        ↓
Global harmonic analysis
        ↓
Pattern Recognizer
        ↓
PatternContext / bounded timeline evidence
        ↓
Incomplete / contradicted pattern evidence
        ↓
Rootless/implied functional reading
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
        ↓
Explanation / Why? presentation
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
Windows Build #310 — success; live-test принят Владом 2026-09-25; PR #31 merged.

Реализованы bounded deterministic `PatternTimelineWindow`, host-neutral `PatternContext`, continuity до resolution member, top-level `iii–VI7–ii–V–I`, nested evidence, seek/edit/reopen reconstruction и STOP-boundary resolver при неизменном Stage 1 contract.

Acceptance: [STAGE_3_0.3f_FIX1_PLAN.md](STAGE_3_0.3f_FIX1_PLAN.md).

### 0.3f fix2 — Pattern evidence guards [ACCEPTED VIA FIX4]

Сохраняемые результаты:

```text
Fm7 → Bb7 → Em7        != confirmed Eb-major ii–V–I
Em7 → A7 → D7          != D-major ii–V–I / V–I
Dm7 → G7 → D7/A        != C-major ii–V–I
Em7 → Am7 → Dm7 → G7   = iii–vi–ii–V • 1/4…4/4
```

Target quality обязана соответствовать tonic reading. `I–VI–ii–V` нельзя достраивать без реального I в bounded evidence.

Первоначальная гипотеза `D7/A→Ab°→Gm7 = passing diminished to G minor` отменена. История: [STAGE_3_0.3f_FIX2_PLAN.md](STAGE_3_0.3f_FIX2_PLAN.md).

### 0.3f fix3 — Rootless dominant / Corcovado correction [ACCEPTED VIA FIX4]

В C major voicing `Ab–B–D` читается как provisional rootless G7(b9), explicit F повышает confidence до high, explicit E добавляет 13. Written Abdim identity сохраняется. `D7/A → Abdim` трактуется как `V/V → implied V`, без ложного local G minor. Следующий `Gm7 → C7 → Fmaj7` остаётся отдельным F-major ii–V–I.

Scope: [STAGE_3_0.3f_FIX3_PLAN.md](STAGE_3_0.3f_FIX3_PLAN.md).

### 0.3f fix4 — Incomplete cadence / missing member [ACCEPTED]

Windows Build #331 — success; 14/14 regression targets green; live-test Studio Pro принят Владом 2026-09-26.

На `Fm7 → Bb7 → Em7` теперь сохраняется описательный незавершённый `ii–V–I`:

```text
Fm7  = 1/3
Bb7  = 2/3
Eb   = missing I, показана серым
Em7  = фактическое продолжение, не 3/3
```

`IncompleteCadence` не создаёт local Eb major, не подтверждает resolution и не участвует в ranking. Неизвестное future не объявляется missing. Полные/carried patterns и самостоятельный `iii–vi–ii–V` имеют приоритет.

Полный acceptance: [STAGE_3_0.3f_FIX4_PLAN.md](STAGE_3_0.3f_FIX4_PLAN.md).

### Presentation semantics, принятые перед 0.3g

- **серый** — implicit/missing member;
- **янтарный/оранжевый** — implied/provisional member или functional alias;
- **красный** — известное продолжение противоречит гипотезе.

Самостоятельный `iii–vi–ii–V` не показывается как «неполный ii–V–I». Rootless implied root не показывается как error.

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
- presentation для missing/implied/contradicted evidence использует принятую цветовую семантику;
- функция и tonal center должны маркироваться по уровню, чтобы случаи вроде Gm7 в global C / local F не выглядели как смешение двух разных выводов;
- T1/T2/T3 policy остаётся Stage 4.

## Что читать при продолжении

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_3_PLAN.md`;
3. `docs/STAGE_3_0.3f_FIX4_PLAN.md` — последний принятый refinement;
4. `docs/STAGE_3_0.3f_FIX3_PLAN.md` — rootless dominant;
5. `docs/STAGE_3_0.3f_FIX2_PLAN.md` — evidence guards и история superseded diminished rule;
6. `docs/STAGE_3_0.3f_FIX1_PLAN.md` — continuity refinement;
7. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
8. `docs/ARCHITECTURAL_DECISIONS.md`;
9. `docs/IMPROVISATION_METHOD.md`;
10. Issue #4 — Stage 3.
