# Smart Improviser — Current State

> Планирование обновлено 2026-09-25: 0.3a–0.3e приняты; `0.3e-ui` принят вместе с 0.3e. Текущий checkpoint — **0.3f Context-aware ranking / ambiguity**. Последняя принятая stable — `0.3`.

> Короткая точка входа для нового чата или рабочей сессии. Подробная архитектура — в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённые Stage:** Stage 0, Stage 1, Stage 2
- **Текущая стабильная версия:** `0.3`
- **Последний завершённый Stage:** Stage 2 — Harmonic Engine
- **Текущий Stage:** Stage 3 — Improvisation Engine
- **Рабочая линия:** `0.3a → 0.3x`
- **Итог Stage 3:** `0.4`
- **Stage 2 Issue:** #3 — закрыт стабильной `0.3`
- **Stage 3 Issue:** #4 — рабочий план `0.3a–0.3h`; 0.3a–0.3e приняты; текущий checkpoint — 0.3f
- **Future/refinement Issue #29:** minor `iv–V–i` + boundary semantics; включён в scope 0.3f, но закрывается только после live acceptance
- **Рабочая ветка 0.3f:** `stage-3-context-ranking`
- **PR:** #30 — `0.3f — context-aware ranking, ambiguity and minor iv-V-i`

`0.3` — релизное закрытие Stage 2. Stable release фиксирует принятое состояние Harmonic Engine; последующие refinement-правила проходят отдельные Stage 3 checkpoints и regression/live validation.

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
        ↓
Improvisation Engine
        ↓
Context-aware strategy ranking
```

В 0.3f этот контракт **не расширяется**. Если на финальном `I/i` более ранний predominant уже не входит в `previous/current/next`, движок не должен угадывать, был ли это `ii`, `iiø` или `iv`.

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

### Возможности Harmonic Engine stable 0.3

- basic harmonic functions;
- major `ii–V–I`;
- minor `iiø–V–i`;
- `V–I`;
- `I–VI–ii–V`;
- secondary dominants и dominant chains;
- ordinary `V7` / `SubV7`;
- major/minor `ii–SubV–I`;
- applied SubV и guide-tone resolution;
- local key center: candidate / tonicized / established / modulationCandidate;
- remote local centers;
- global/local harmonic interpretations;
- borrowed/modal ambiguity;
- `unique / ambiguous` semantics и explicit primary interpretation;
- enharmonic-aware KeyCenter spelling;
- boundary false-positive guards;
- diagnostic UI для global/local/ambiguity/confidence.

### Integration validation stable 0.3

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

## Версия 0.3

```text
Build label: Smart Improviser 0.3
CMake:      0.3.0
Artifact:   Smart-Improviser-0.3-Windows
Package:    Smart Improviser.vst3
Tests:      7 regression/integration targets
```

## Stage 3 / Improvisation Engine

Stage 3 использует готовый `HarmonicSituation` и выдаёт музыкально применимый материал для импровизации:

- chord tones;
- guide tones;
- target notes;
- scales;
- harmonic concepts;
- resolution notes;
- базовые improvisation strategies;
- context-specific alternatives с provenance по harmonic interpretation.

Рабочая декомпозиция опубликована в [STAGE_3_PLAN.md](STAGE_3_PLAN.md) и Issue #4. Начальная методика согласована в [IMPROVISATION_METHOD.md](IMPROVISATION_METHOD.md). Основной первичный источник текущего метода — Игорь Бойко, «Мой метод», раздел 2 (2010); методические правила в коде отделяются от проектных правил Smart Improviser.

Согласованная продуктовая цель: [PRODUCT_WORKFLOW.md](PRODUCT_WORKFLOW.md). Общая/личная библиотеки — Stage 5; сохраняемый Song workspace — Stage 7; быстрый ввод нот/TAB и ручной конструктор — Stage 8; Planner — Stage 10. Смена tension не переписывает сохранённые идеи.

## Что читать при продолжении Stage 3

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_3_PLAN.md`;
3. `docs/STAGE_3_0.3f_LIVE_TEST.md`;
4. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
5. `docs/PROJECT_CONTEXT.md`;
6. `docs/ARCHITECTURAL_DECISIONS.md`;
7. `docs/IMPROVISATION_METHOD.md`;
8. `docs/HARMONIC_ENGINE_FUTURE_REFINEMENTS.md`;
9. Issue #4 — текущий Stage 3;
10. Issue #29 — `iv–V–i` refinement, теперь включён в 0.3f.

## Принятый checkpoint 0.3a

Реализована основа Improvisation Engine: одна стратегия по explicit chord tones, сохранение гармонического контекста и target, отдельные source/thinking/resolution fields, deterministic rule ID/version и priority. Windows Build #231 — success. Влад принял live-test 2026-09-24, PR #23 merged.

## Принятый checkpoint 0.3b

Guide tones, characteristic tones, реальные next-chord targets, confirmed ResolutionTarget moves и отдельные optional melodic connections. Windows Build #234 — success; live-test принят; PR #24 merged.

## Принятый checkpoint 0.3c

Ограниченный diatonic/modal catalog: режимы selected major/minor center и Mixolydian для ordinary V7 → major. Source хранит degrees/spelling и interpretation provenance; tension не назначается. Windows Build #237 success; принят 2026-09-25; PR #25 merged.

## Принятый checkpoint 0.3d

Четыре melodic-minor applications, project-rule SubV и whole-half diminished. Source, m6 thinking, реальные anchors и target разделены. Fix2 перенёс confirmed SubV spelling в функциональный слой Stage 3: host C#7→C показывается музыкально как Db7, source Ab melodic minor / Abm6. Windows Build #246 success; live-test принят 2026-09-25; PR #26 merged.

## Принятый checkpoint 0.3e + 0.3e-ui

0.3e добавил:

- chord-tone playing;
- guide-tone targeting;
- diatonic extensions;
- m6-thinking concepts;
- symbolic chromatic approach / enclosure с реальной целью;
- сохранение confirmed vs optional moves;
- без Phrase/MIDI generation и без Tension policy.

`0.3e-ui` сделал диагностическое окно рабочим для дальнейших live-tests:

- компактная карточка **«Текущий контекст»**;
- функция коротко: Тоника / Субдоминанта / Доминанта / SubV;
- harmonic pattern и позиция;
- строка **«Мышление»**, например `D Dorian • Dm6`;
- четыре раздела: Материал / Источники / Гармонический анализ / ARA.

Windows Build #274 success. Влад принял 0.3e и 0.3e-ui 2026-09-25; PR #28 merged.

Во время этого live-test обнаружен реальный catalog gap `Fm7 → G7 → Cm7 = iv–V–i`. Он зафиксирован в Issue #29 и `HARMONIC_ENGINE_FUTURE_REFINEMENTS.md` и по решению пользователя включён в 0.3f.

## Рабочий checkpoint 0.3f

Цель: сохранить музыкально разные strategies при нескольких harmonic interpretations и не превращать deterministic ordering в скрытый harmonic choice.

Реализуется в ветке `stage-3-context-ranking`, PR #30:

- foundation остаётся `interpretationIndependent`;
- diatonic/modal и special sources собираются для каждого валидного interpretation;
- context-dependent strategy хранит `interpretationIndex` + evidence;
- при выбранной primary её strategies ранжируются раньше alternatives;
- при `primaryInterpretationIndex = -1` нет скрытого winner;
- recommendation priority и harmonic confidence остаются раздельными;
- добавлен отдельный `minorIvVi` для `iv–V–i`;
- `iv→V` может быть boundary candidate, полный `iv→V→i` — confirmed cadence;
- local minor center поддерживает тот же pattern;
- финальный `V→I/i` без earlier predominant обозначается общим двухаккордовым resolution;
- UI при unresolved ambiguity показывает явную неоднозначность, а Sources подписываются по harmonic interpretation.

Новый regression target: `SmartImproviserContextRankingTests`. Build label `0.3f`, artifact `Smart-Improviser-0.3f-Windows`.

[Scope / live-test](STAGE_3_0.3f_LIVE_TEST.md). До зелёного CI и live acceptance 0.3f не считается принятым.
