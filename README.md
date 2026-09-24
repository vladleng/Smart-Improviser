# Smart Improviser

**Smart Improviser** — VST3/ARA 2 плагин для DAW, который помогает гитаристу строить джазовую импровизацию на основе **гармонического контекста**, а не только текущего аккорда.

> Главная идея: отвечать не только на вопрос «что здесь можно сыграть?», но и «что музыкально уместно сыграть именно здесь, зачем, куда это разрешить и с какой степенью напряжения?»

## Статус

**Stage 0 — Foundation / Specification завершён.**  
**Stage 1 — ARA Context Monitor завершён.**  
**Stage 2 — Harmonic Engine завершён.**

**Текущая стабильная версия:** `0.3`  
**Текущий Stage:** Stage 3 — Improvisation Engine<br>
**Следующая рабочая линия:** `0.3a → 0.3x`  
**Итог Stage 3:** `0.4`

Stable `0.3` фиксирует полностью принятый Stage 2. Новая музыкальная логика относительно `0.2f` в release не добавлялась.

Первая целевая среда:

- **DAW:** Fender Studio Pro
- **Интеграция:** ARA 2
- **Формат:** VST3
- **Платформа:** Windows
- **Основной сценарий:** джазовая импровизация на гитаре

## Работа над песней

Центральный сценарий — сохранить идею на конкретном такте, доработать её при следующем открытии проекта и использовать удачные находки в других песнях. Материал можно записать самостоятельно или взять из общей/личной библиотеки. Подсказки предлагают логику мышления, замены, источники и разрешения с учётом выбранного tension.

Библиотечный источник и редактируемый экземпляр в песне независимы. Смена tension обновляет подсказки без автоматического переписывания нот.

Это целевой workflow будущих Stage: stable 0.3 пока реализует Harmonic Engine. План: [PRODUCT_WORKFLOW.md](docs/PRODUCT_WORKFLOW.md).

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

## Следующий Stage — Improvisation Engine

Stage 3 использует готовый `HarmonicSituation` и должен выдавать:

- chord tones;
- guide tones;
- target notes;
- scales;
- harmonic concepts;
- resolution notes;
- базовые improvisation strategies.

Рабочий план Stage 3: [0.3a–0.3h → 0.4](docs/STAGE_3_PLAN.md). Checkpoint **0.3a принят**. Текущая рабочая версия — **0.3b**, structural tones / targets / resolution. [Начальная методика](docs/IMPROVISATION_METHOD.md): материал Бойко, контекст доминанты перед выбором источника, простая расширяемая модель T1–T3.

## Дальнейшее направление

После Improvisation Engine планируются:

- Tension Engine;
- Phrase Library;
- Phrase Transposition;
- Fretboard / Notation / TAB Viewer;
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
