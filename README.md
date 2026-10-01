# Smart Improviser

**Smart Improviser** — VST3/ARA 2 плагин для DAW, который помогает гитаристу строить джазовую импровизацию на основе **гармонического контекста**, а не только текущего аккорда.

> Главная идея: отвечать не только на вопрос «что здесь можно сыграть?», но и «что музыкально уместно сыграть именно здесь, зачем, куда это разрешить и с какой степенью напряжения?»

## Статус

**Stage 0 — Foundation / Specification завершён.**  
**Stage 1 — ARA Context Monitor завершён.**  
**Stage 2 — Harmonic Engine завершён.**

**Текущая стабильная версия:** `0.4`

**Stage 3:** завершён, stable `0.4`

**Текущий Stage:** Stage 4 — viewer (`0.4a`) и контракт (`0.4b`) приняты; `0.4c` с fix1–fix5 принята; `0.4d` с выбором tension и белым основным тоном аккорда на грифе принята. Рабочий checkpoint — [`0.4e`](docs/STAGE_4_0.4e_PHRASE_CONTRACT.md), контракт проверки будущих фраз; приёмка ожидается.

Stable `0.4` фиксирует принятый Stage 3: стратегии и объяснения на основе HarmonicSituation. Viewer `0.4a` и уточнение `upd4` по карте/Левину приняты. Контракт `0.4b` принят после живого теста; [0.4c](docs/STAGE_4_0.4c_LEVEL1.md) принята с fix1–fix5. [0.4d](docs/STAGE_4_0.4d_SELECTION.md) фильтрует подсказки по ручным T1/T2/T3; назначение уровней остаётся пользовательским.

Первая целевая среда:

- **DAW:** Fender Studio Pro
- **Интеграция:** ARA 2
- **Формат:** VST3
- **Платформа:** Windows
- **Основной сценарий:** джазовая импровизация на гитаре

## Работа над песней

Центральный сценарий — сохранить идею на конкретном такте, доработать её при следующем открытии проекта и использовать удачные находки в других песнях. Материал можно записать самостоятельно или взять из общей/личной библиотеки. Подсказки предлагают логику мышления, замены, источники и разрешения с учётом выбранного tension.

Библиотечный источник и редактируемый экземпляр в песне независимы. Смена tension обновляет подсказки без автоматического переписывания нот.

Это целевой workflow будущих Stage: Stage 3 добавил стратегии и объяснения, а song workspace остаётся будущим этапом. План: [PRODUCT_WORKFLOW.md](docs/PRODUCT_WORKFLOW.md).

## Ключевая идея

Smart Improviser строится не вокруг простой схемы `Chord → Scale`, а вокруг цепочки:

```text
Harmony
→ Context
→ Global / Local Function
→ Harmonic Pattern
→ Ambiguity / Confidence
→ Tension
→ Strategy
→ Resolution
→ Phrase / Vocabulary
```

Центральная сущность ядра — `HarmonicSituation`.

## Архитектура

```text
Fender Studio Pro
        ↓
ARA 2 Adapter
        ↓
TimelineHarmonicSnapshot
(previous / current / next)
        ↓
Harmonic Engine
        ├── Pattern Recognizer
        ├── Tritone Substitution
        ├── Local Key Center Analyzer
        └── Ambiguity / Confidence Analyzer
        ↓
HarmonicSituation
        ↓
Improvisation / Tension / Phrase layers
        ↓
UI / Fretboard / Notation / TAB
```

Stage 1 contract закрыт: Harmonic Engine получает только host-neutral timeline context и не зависит от JUCE / ARA / Fender Studio Pro. Project key в DAW автоматически не меняется.

## Stable 0.3 — Harmonic Engine

В `0.3` входят все принятые checkpoints Stage 2:

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

### Что умеет Harmonic Engine

- basic harmonic functions;
- major `ii–V–I` на `ii / V / I`;
- minor `iiø–V–i` на `iiø / V / i`;
- `V–I`;
- `I–VI–ii–V`;
- secondary dominants;
- dominant chains;
- ordinary `V7` / `SubV7`;
- major/minor `ii–SubV–I`;
- applied SubV;
- guide-tone resolution;
- candidate / tonicized / established local centers;
- remote local centers;
- `localHarmonic` / `localPattern`;
- cautious `modulationCandidate`;
- fixed-size `HarmonicInterpretation` candidates;
- unique / ambiguous state;
- global/local conflict evidence;
- borrowed/modal ambiguity;
- diagnostic UI со списком candidates и primary;
- enharmonic-aware KeyCenter display;
- boundary false-positive guards для известных противоречащих событий.

### Integration validation

Отдельный regression target:

```text
SmartImproviserIntegrationValidationTests
```

Live-tested progression cases:

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

## Release 0.3

```text
Build label: Smart Improviser 0.3
CMake:      0.3.0
Artifact:   Smart-Improviser-0.3-Windows
Package:    Smart Improviser.vst3
Tests:      7 regression/integration targets
```

## Release 0.4

```text
Build label: Smart Improviser 0.4
CMake:      0.4.0
Artifact:   Smart-Improviser-0.4-Windows
Package:    Smart Improviser.vst3
Tests:      15 regression/integration targets
```

Релиз включает принятый Improvisation Engine и explanation layer. Tension T1/T2/T3 и библиотека фраз остаются задачами следующих Stage.

## Stage 3 — Improvisation Engine [ACCEPTED]

`0.3a–0.3h fix3` выдают chord/guide/target notes, допустимые scale/source materials, harmonic concepts, resolution и объяснимые стратегии с provenance global/local/modal. Недостаток данных и неоднозначность показываются явно. Для `III–VI–II–V–I` несыгранная ожидаемая I остаётся серой и не становится выдуманным аккордом.

[План и музыкальная приёмка](docs/STAGE_3_PLAN.md). Windows Build #360 прошёл 15/15 тестовых целей, финальный UI принят в Studio Pro. Stable `0.4` выпущена отдельным Windows artifact `Smart-Improviser-0.4-Windows` (папка установки `Smart Improviser.vst3`).

## Дальнейшее направление

После Improvisation Engine планируются:

- Visual Material Viewer (Stage 4);
- Tension Engine (Stage 4);
- Phrase Library;
- Phrase Transposition;
- Phrase Viewer / Song Workspace (Stage 7);
- Phrase Editor;
- Phrase Transformation Engine;
- Improvisation Planner.

## Документация

- [`docs/CURRENT_STATE.md`](docs/CURRENT_STATE.md) — текущее состояние и следующий Stage.
- [`docs/STAGE_1_TO_STAGE_2_CONTRACT.md`](docs/STAGE_1_TO_STAGE_2_CONTRACT.md) — граница Stage 1 → Harmonic Engine.
- [`docs/CORE_DATA_MODEL_0.0b.md`](docs/CORE_DATA_MODEL_0.0b.md) — host-neutral data model.
- [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md) — living document проекта.
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — Stage и build checkpoints.
- [`docs/VERSIONING.md`](docs/VERSIONING.md) — схема версий и fix-сборок.
- [`docs/ARCHITECTURAL_DECISIONS.md`](docs/ARCHITECTURAL_DECISIONS.md) — архитектурные решения.

- [`docs/PRODUCT_WORKFLOW.md`](docs/PRODUCT_WORKFLOW.md) — работа над песней, общая/личная библиотеки и user content.
- [`docs/STAGE_3_PLAN.md`](docs/STAGE_3_PLAN.md) — checkpoints Improvisation Engine и открытые решения.
