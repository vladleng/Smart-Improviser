# Smart Improviser — Current State

> Обновлено 2026-09-26: `0.3a–0.3f` и `0.3f fix1` приняты. `0.3f fix2` прошёл Windows Build #328, но real-harmony validation Corcovado выявил неверную diminished-трактовку; текущий refinement — **`0.3f fix3 — Rootless dominant / Corcovado diminished correction`**. `0.3g` начинается только после acceptance fix3. Последняя stable — `0.3`.

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
- **0.3f fix2:** code/regression gate Build #328 success, но музыкально не принят из-за неверной трактовки Corcovado diminished
- **PR #32:** draft fix2; будет superseded fix3, не должен сливаться как финальное состояние
- **Рабочая ветка:** `stage-3-context-ranking-fix3`
- **Текущий checkpoint:** `0.3f fix3 — Rootless dominant / Corcovado diminished correction`
- **После fix3:** `0.3g — Explanation / usable output`

## Архитектурная граница

Stage 1 contract остаётся закрытым:

```text
previous chord
current chord
next chord
global key
```

Для длиннее-живущего harmonic context Stage 3 использует отдельный host-neutral `PatternContext`, реконструируемый детерминированно из bounded `PatternTimelineWindow`. Это не arbitrary history и не расширение Stage 1 до `previousN`.

`0.3f fix2` усилил evidence semantics: уже известный future member может **опровергнуть** provisional cadence candidate. Это veto ложной гипотезы, а не скрытый выбор другой трактовки.

`0.3f fix3` добавляет отдельную functional alias-модель `ImpliedDominantReading`: написанный chord symbol не переписывается, но pitch content diminished chord может получить contextual rootless-dominant reading, если это подтверждается явным global key.

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

## 0.3f fix2 — evidence guards [TECHNICALLY GREEN, SUPERSEDED IN ONE MUSICAL RULE]

Windows Build #328 — success.

Сохраняемые результаты fix2:

```text
Fm7 → Bb7 → Em7        != Eb-major ii–V–I
Em7 → A7 → D7          != D-major ii–V–I / V–I
Dm7 → G7 → D7/A        != C-major ii–V–I
Em7 → Am7 → Dm7 → G7   = iii–vi–ii–V • 1/4…4/4
```

Также target quality теперь обязана соответствовать tonic reading, а `I–VI–ii–V` нельзя достраивать без реального I в bounded evidence.

Первоначальный пункт `D7/A→Ab°→Gm7 = passing diminished to G minor` **отменён**. История и причина: [STAGE_3_0.3f_FIX2_PLAN.md](STAGE_3_0.3f_FIX2_PLAN.md).

## Текущий refinement — 0.3f fix3

Real-harmony review уточнил фактическую функцию Corcovado voicing:

```text
Ab° = Ab–B–D–F
    = b9–3–5–b7 от G
    ≈ rootless G7(b9) в C major
```

Если сверху присутствует E:

```text
Ab–B–D–F–E ≈ rootless G13(b9)
```

Поэтому ожидаемая модель:

```text
D7/A → Ab°                  = V/V → implied V (dominant chain)
Ab°                          = written diminished + implied rootless G7(b9)
Gm7 → C7 → Fmaj7            = новый local ii–V–I in F
```

Инварианты fix3:

- written chord identity сохраняется;
- implied functional root не превращается в новый chord symbol;
- `D7/A` и `Ab°` не получают ложный local G minor;
- rootless dominant не считается автоматически разрешённым, если фактический C не наступил;
- `Gm7` начинает следующий подтверждённый F-major cadence;
- generic `passingDiminished` остаётся в каталоге, но Corcovado-специализация удалена.

Полный scope и gate: [STAGE_3_0.3f_FIX3_PLAN.md](STAGE_3_0.3f_FIX3_PLAN.md).

### Presentation semantics, обязательные для 0.3g

- **серый** — неявный/отсутствующий member шаблона;
- **янтарный/оранжевый** — implied/provisional member или гипотеза;
- **красный** — известное продолжение опровергает ожидаемый member / конфликт.

Rootless G у `Ab°` в будущем presentation может быть показан как implied/amber, а не как error. `iii–vi–ii–V` не показывает missing I как «дыру»: это самостоятельный pattern.

## После fix3 — 0.3g

`0.3g — Explanation / usable output` должен завершить объяснимую пользовательскую цепочку:

```text
идея → source → важные ноты → target/resolution → почему
```

Основные задачи:

- deterministic explanations из уже рассчитанного Core/evidence;
- Why?-data без повторного harmonic analysis в UI;
- presentation-layer схлопывает одинаковый музыкальный материал из нескольких interpretations, сохраняя provenance;
- top-level/nested PatternContext используется как источник объяснения;
- presentation для implied/provisional/conflicting evidence использует принятую цветовую семантику;
- T1/T2/T3 policy остаётся Stage 4.

## Что читать при продолжении

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_3_PLAN.md`;
3. `docs/STAGE_3_0.3f_FIX3_PLAN.md` — текущий refinement;
4. `docs/STAGE_3_0.3f_FIX2_PLAN.md` — evidence guards и история superseded diminished rule;
5. `docs/STAGE_3_0.3f_FIX1_PLAN.md` — принятый continuity refinement;
6. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
7. `docs/ARCHITECTURAL_DECISIONS.md`;
8. `docs/IMPROVISATION_METHOD.md`;
9. Issue #4 — Stage 3.
