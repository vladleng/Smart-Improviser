# Smart Improviser

**Smart Improviser** — VST3/ARA 2 плагин для DAW, который помогает гитаристу строить джазовую импровизацию на основе **гармонического контекста**, а не только текущего аккорда.

> Главная идея: отвечать не только на вопрос «что здесь можно сыграть?», но и «что музыкально уместно сыграть именно здесь, зачем, куда это разрешить и с какой степенью напряжения?»

## Статус

**Stage 0 — Foundation / Specification завершён.**  
**Stage 1 — ARA Context Monitor завершён.**  
**Активный Stage:** Stage 2 — Harmonic Engine.

**Текущая стабильная версия:** `0.2`  
**Последний принятый checkpoint:** `0.2f — Integration / musical validation`  
**Следующий шаг:** стабильная `0.3 — Stage 2 complete`  
**Stage 2 Issue:** #3  
**PR #20:** `0.2f — Integration / musical validation`

`0.2f` принят после успешного Windows Build #217 и полного live musical validation в Fender Studio Pro. Это последний буквенный checkpoint Stage 2 перед стабильной `0.3`.

Первая целевая среда:

- **DAW:** Fender Studio Pro
- **Интеграция:** ARA 2
- **Формат:** VST3
- **Платформа:** Windows
- **Основной сценарий:** джазовая импровизация на гитаре

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
Tension / Strategy / Resolution / Phrase engines
        ↓
UI / Fretboard / Notation / TAB
```

Stage 1 contract закрыт: Harmonic Engine получает только host-neutral timeline context и не зависит от JUCE / ARA / Fender Studio Pro. Project key в DAW автоматически не меняется.

## Что умеет Harmonic Engine к завершению 0.2f

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
- integration false-positive guard для известного противоречащего `next` chord.

## 0.2f — Integration / musical validation [ACCEPTED]

`0.2f` не добавляет новый крупный музыкальный слой. Он проверяет все принятые части Stage 2 **вместе** через отдельный regression target:

```text
SmartImproviserIntegrationValidationTests
```

Проверенные progression cases:

```text
Cmaj7 → A7 → Dm7 → G7 → Cmaj7
Cmaj7 → Fm7 → G7 → Cmaj7
Em7b5 → Eb7 → Dm → G7 → Cmaj7
C#7 → F#maj7 → Bmaj7
Dm7 → G7 → Abmaj7   // contradictory-next guard
```

Подтверждены переходы:

```text
Global → Local primary → Global
Unique → Ambiguous → Unique
Tonicized → Modulation candidate
```

Также подтверждено, что при `Dm7 → G7 → Abmaj7` движок не придумывает ложное завершение `I–VI–ii–V`, если противоречащий `next` уже известен.

Версия принятого checkpoint:

```text
Build label: Smart Improviser 0.2f
CMake:      0.2.8
Artifact:   Smart-Improviser-0.2f-Windows
Package:    Smart Improviser.vst3
Windows Build #217: SUCCESS
Tests:      7 / 7 PASS
```

## Правило разработки Stage

```text
новая буква = новая функциональная часть Stage
fixN        = исправление текущего checkpoint
версия без буквы = весь Stage завершён
```

Stage 2:

```text
0.2a — Harmonic Engine foundation          [ACCEPTED]
0.2a fix1 — Harmonic Engine diagnostics    [ACCEPTED]
0.2b — Pattern Recognizer                  [ACCEPTED]
0.2c — Tritone Substitution                [ACCEPTED]
0.2d — Local Key Center                    [ACCEPTED]
0.2e — Ambiguity / Confidence              [ACCEPTED]
0.2e fix1 — Enharmonic spelling            [ACCEPTED]
0.2f — Integration / musical validation    [ACCEPTED]
0.3  — Stage 2 complete                    [NEXT]
```

## Tension Engine

Следующий крупный слой после Harmonic Engine — три уровня напряжения:

- **Tension 1 — Stable:** chord tones, guide tones, устойчивые extensions.
- **Tension 2 — Color:** chromatic approaches, enclosures, melodic-minor applications, upper structures.
- **Tension 3 — Outside / Maximum:** altered/diminished language, substitutions, side slipping, superimposed harmony и delayed resolution.

## Ближайший технический шаг

Подготовить стабильную `0.3 — Stage 2 complete` без добавления новой музыкальной логики: финальный version bump, документация, artifact и проверка принятого состояния `0.2a…0.2f`.

## Документация

- [`docs/CURRENT_STATE.md`](docs/CURRENT_STATE.md) — текущий checkpoint и следующий шаг.
- [`docs/STAGE_1_TO_STAGE_2_CONTRACT.md`](docs/STAGE_1_TO_STAGE_2_CONTRACT.md) — граница Stage 1 → Harmonic Engine.
- [`docs/CORE_DATA_MODEL_0.0b.md`](docs/CORE_DATA_MODEL_0.0b.md) — host-neutral data model.
- [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md) — living document проекта.
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — Stage и build checkpoints.
- [`docs/VERSIONING.md`](docs/VERSIONING.md) — схема версий и fix-сборок.
- [`docs/ARCHITECTURAL_DECISIONS.md`](docs/ARCHITECTURAL_DECISIONS.md) — архитектурные решения.
