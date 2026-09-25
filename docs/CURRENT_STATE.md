# Smart Improviser — Current State

> Планирование обновлено 2026-09-25: `0.3a–0.3f` приняты; после live-feedback открыт **`0.3f fix1 — Pattern continuity / hierarchical cadence context`**. `0.3g` начинается после принятия fix1. Последняя принятая stable — `0.3`.

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённые Stage:** Stage 0, Stage 1, Stage 2
- **Текущая стабильная версия:** `0.3`
- **Текущий Stage:** Stage 3 — Improvisation Engine
- **Рабочая линия:** `0.3a → 0.3x`
- **Итог Stage 3:** `0.4`
- **Stage 2 Issue:** #3 — закрыт стабильной `0.3`
- **Stage 3 Issue:** #4 — рабочий план `0.3a–0.3h`; `0.3a–0.3f` приняты; текущий refinement — `0.3f fix1`
- **Issue #29:** minor `iv–V–i` + boundary semantics — закрыт в `0.3f`
- **PR #30:** merged в `main`, squash commit `5cdc126733393a7c69e864eda9c925b146cc56f1`
- **Рабочая ветка fix1:** `stage-3-context-ranking-fix1`
- **Текущий checkpoint:** `0.3f fix1 — Pattern continuity / hierarchical cadence context`
- **После fix1:** `0.3g — Explanation / usable output`

`0.3` — релизное закрытие Stage 2. Stable release фиксирует принятое состояние Harmonic Engine; subsequent Stage 3 checkpoints расширяют Improvisation Engine с отдельными regression/live gates.

## Архитектурная граница

Stage 1 contract остаётся закрытым. Harmonic Engine получает только host-neutral timeline context:

```text
previous chord
current chord
next chord
global key
```

`0.3f fix1` **не расширяет этот контракт до previousN**. Вместо этого Core получает ограниченный state уже распознанного harmonic event (`PatternContext` / `RecognizedPatternInstance`): тип паттерна, центр/interpretation, position, status, expected/confirmed resolution и evidence. Это музыкальная память подтверждённого события, а не произвольная история всех предыдущих аккордов.

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

Принятые checkpoints Stage 2:

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

Stage 2 умеет: basic harmonic functions, major `ii–V–I`, minor `iiø–V–i`, `V–I`, `I–VI–ii–V`, secondary dominants/chains, ordinary `V7` / `SubV7`, local key centers, global/local/modal ambiguity, enharmonic-aware spelling и boundary guards.

## Stage 3 / Improvisation Engine

Stage 3 использует готовый `HarmonicSituation` и выдаёт:

- chord tones;
- guide tones;
- target notes;
- scales/source material;
- harmonic concepts;
- resolution notes;
- improvisation strategies;
- context-specific alternatives с provenance по harmonic interpretation.

Рабочая декомпозиция: [STAGE_3_PLAN.md](STAGE_3_PLAN.md). Методика: [IMPROVISATION_METHOD.md](IMPROVISATION_METHOD.md). Основной первичный источник текущего метода — Игорь Бойко, «Мой метод», раздел 2 (2010); проектные правила Smart Improviser отделяются от авторской методики.

## Принятые checkpoints Stage 3

### 0.3a — Strategy contract / foundation
Windows Build #231 — success; live-test принят 2026-09-24; PR #23 merged.

### 0.3b — Structural tones / targets / resolution
Guide tones, characteristic tones, реальные next-chord targets, confirmed ResolutionTarget moves и отдельные optional melodic connections. Windows Build #234 — success; PR #24 merged.

### 0.3c — Diatonic / modal sources
Ограниченный context-aware diatonic/modal catalog; source хранит degrees/spelling и interpretation provenance; tension не назначается. Windows Build #237 — success; принят 2026-09-25; PR #25 merged.

### 0.3d — Melodic minor / diminished sources
Четыре melodic-minor applications, project-rule SubV и whole-half diminished. Confirmed SubV spelling выводится функционально: host `C#7→C` показывается музыкально как `Db7 / Ab melodic minor / Abm6`. Windows Build #246 — success; live-test принят 2026-09-25; PR #26 merged.

### 0.3e + 0.3e-ui — Harmonic concepts / diagnostic UI
Добавлены chord-tone playing, guide-tone targeting, diatonic extensions, m6-thinking, symbolic chromatic approach/enclosure, confirmed vs optional moves. UI: компактная карточка «Текущий контекст», функция Тоника / Субдоминанта / Доминанта / SubV, pattern/позиция, строка «Мышление», четыре collapsible sections. Windows Build #274 — success; принято 2026-09-25; PR #28 merged.

### 0.3f — Context-aware ranking / ambiguity [ACCEPTED]

Принято Владом 2026-09-25. Windows Build #284 — success. Live-test пройден. PR #30 merged. Issue #29 закрыт.

Реализовано:

- foundation остаётся `interpretationIndependent`;
- diatonic/modal и special sources собираются для каждого валидного interpretation;
- context-dependent strategy хранит `interpretationIndex` + evidence;
- при выбранной primary её strategies идут раньше alternatives;
- при `primaryInterpretationIndex = -1` нет скрытого winner;
- recommendation priority и harmonic confidence остаются раздельными;
- добавлен отдельный `minorIvVi` для `iv–V–i`;
- `iv→V` работает как boundary candidate, полный `iv→V→i` — confirmed cadence;
- local minor center поддерживает тот же pattern;
- финальный `V→I/i` без sufficient carried evidence обозначается общим двухаккордовым resolution;
- false-positive `C major: Fm7 → G7 → Cmaj7` устранён;
- UI при unresolved ambiguity явно показывает несколько candidates;
- Stage 1 contract `previous/current/next` не расширен.

Acceptance: [STAGE_3_0.3f_LIVE_TEST.md](STAGE_3_0.3f_LIVE_TEST.md).

## Текущий refinement — 0.3f fix1

После acceptance обнаружено, что на resolution chord snapshot теряет более ранний predominant и UI показывает только `V–I`, хотя непосредственно перед этим полный pattern уже был подтверждён.

Fix1 вводит continuity подтверждённого pattern instance:

```text
F major: Gm7 → C7 → Fmaj7
         1/3    2/3    3/3
```

и аналогично для `iiø–V–i` и `iv–V–i`. Generic `V→I/i` остаётся fallback, если carried PatternContext действительно отсутствует.

В этот же fix входит top-level расширенный каданс:

```text
F major: Am7 → D7 → Gm7 → C7 → Fmaj7
          iii    VI7    ii     V      I
```

Пользовательский основной контекст должен быть единым `iii–VI7–ii–V–I • 1/5 … 5/5`; `D7→Gm7 = V/ii→ii` и `Gm7→C7→Fmaj7 = ii–V–I` сохраняются как nested evidence/subpatterns, а не как независимые top-level обороты.

Один аккорд может завершать предыдущий pattern и одновременно давать evidence следующего события. Это не требует расширения timeline history; контекст должен пересчитываться безопасно на seek/chord edits/reopen.

Полный scope: [STAGE_3_0.3f_FIX1_PLAN.md](STAGE_3_0.3f_FIX1_PLAN.md).

## После fix1 — 0.3g

`0.3g — Explanation / usable output` должен завершить объяснимую пользовательскую цепочку:

```text
идея → source → важные ноты → target/resolution → почему
```

Основные задачи: deterministic explanations, Why?-data, аккуратное presentation-layer представление уже рассчитанного Core-результата. T1/T2/T3 policy остаётся Stage 4.

## Что читать при продолжении

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_3_PLAN.md`;
3. `docs/STAGE_3_0.3f_FIX1_PLAN.md` — текущий refinement;
4. `docs/STAGE_3_0.3f_LIVE_TEST.md` — принятый базовый 0.3f;
5. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
6. `docs/ARCHITECTURAL_DECISIONS.md`;
7. `docs/IMPROVISATION_METHOD.md`;
8. Issue #4 — Stage 3;
9. Issue #29 — закрытая история `iv–V–i` refinement.
