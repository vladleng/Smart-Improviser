# Smart Improviser

**Smart Improviser** — VST3/ARA 2 плагин для DAW, который помогает гитаристу строить джазовую импровизацию на основе **гармонического контекста**, а не только текущего аккорда.

> Главная идея: отвечать не только на вопрос «что здесь можно сыграть?», но и «что музыкально уместно сыграть именно здесь, зачем, куда это разрешить и с какой степенью напряжения?»

## Статус

**Stage 0 — Foundation / Specification завершён.**  
**Stage 1 — ARA Context Monitor завершён.**  
**Активный Stage:** Stage 2 — Harmonic Engine.

**Текущая стабильная версия:** `0.2`  
**Последний принятый checkpoint:** `0.2d — Local Key Center`  
**Текущая рабочая версия:** `0.2e — Ambiguity / Confidence`  
**Активная ветка:** `stage-2-ambiguity-confidence`  
**Stage 2 Issue:** #3  
**Stage 2 PR:** #19

`0.2d` принят после CI и полного live-test в Fender Studio Pro и слит в `main` через PR #18.

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

Музыкальное ядро отделено от DAW, ARA, VST3 и UI:

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

Stage 1 contract закрыт: Harmonic Engine получает только host-neutral timeline context и не зависит от JUCE/ARA/Fender Studio Pro.

## Global key и Local Key Center

Начиная с `0.2d`, project key и активный тональный центр — разные сущности.

```text
GLOBAL KEY
F major
    ↓
LOCAL CENTER
D minor
    ↓
CURRENT LOCAL FUNCTION
A7 = V of D minor
```

Project key в DAW не требуется менять при каждом временном отклонении.

Local-center states:

```text
candidate
    ↓
tonicized / temporary
    ↓
established local center
    ↓
modulationCandidate
```

`modulationCandidate` — только гипотеза, а не автоматическая смена global key.

## Ambiguity / Confidence

`0.2e` добавляет слой интерпретаций поверх global/local harmonic facts.

Вместо принудительной единственной трактовки `HarmonicSituation` может хранить несколько candidates:

```text
Global interpretation
Local-center interpretation
Modal-interchange interpretation
```

Семантика:

```text
UNIQUE
→ evidence достаточно для primary interpretation

AMBIGUOUS
→ несколько правдоподобных трактовок существуют параллельно
→ primary остаётся unresolved
```

Примеры:

```text
C major / Fm7
→ Global: chromatic
→ Modal interchange: C minor
→ AMBIGUOUS
```

```text
F major / Em7b5 → A7
→ Global interpretation
→ Candidate D minor
→ AMBIGUOUS
```

```text
F major / Em7b5 → A7 → Dm
→ Local D minor established
→ Local interpretation становится unique primary
```

## Что уже умеет Harmonic Engine

К принятому `0.2d`:

- basic harmonic functions;
- major `ii–V–I` на `ii / V / I`;
- minor `iiø–V–i` на `iiø / V / i`;
- `I–VI–ii–V`;
- secondary dominants;
- dominant chains;
- ordinary `V7` / `SubV7`;
- major/minor `ii–SubV–I`;
- applied SubV;
- guide-tone resolution;
- candidate/temporary/established local centers;
- remote local centers;
- `localHarmonic` / `localPattern`;
- cautious `modulationCandidate`;
- Stage 2 diagnostic UI.

Рабочий `0.2e` добавляет:

- fixed-size `HarmonicInterpretation` candidates;
- unique / ambiguous state;
- unresolved primary (`primaryInterpretationIndex = -1`);
- global/local conflict evidence;
- borrowed/modal ambiguity;
- confirmed local resolution of ambiguity;
- diagnostic UI со списком candidates и primary.

## Правило разработки Stage

Каждый Stage делится на логически завершённые build checkpoints:

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
0.2e — Ambiguity / Confidence              [ACTIVE]
0.2f — Integration / musical validation
0.3  — Stage 2 complete
```

## Tension Engine

Следующий крупный слой после Harmonic Engine — три уровня напряжения:

- **Tension 1 — Stable:** chord tones, guide tones, устойчивые extensions.
- **Tension 2 — Color:** chromatic approaches, enclosures, melodic-minor applications, upper structures.
- **Tension 3 — Outside / Maximum:** altered/diminished language, substitutions, side slipping, superimposed harmony и delayed resolution.

В дальнейшем tension должен работать и как **Tension Curve** для нескольких тактов или chorus.

## Долгосрочное направление

Smart Improviser должен объединить:

- harmonic analysis;
- local tonal-center detection;
- ambiguity / confidence;
- tension levels и tension curve;
- target notes и resolution logic;
- jazz vocabulary;
- semantic Phrase Library;
- functional transpose;
- major ↔ minor adaptation;
- V7 ↔ SubV7 adaptation;
- approach notes / enclosures;
- fretboard / notation / TAB;
- Phrase Transformation Engine;
- драматургию импровизации.

Цель проекта — не генерировать музыку вместо музыканта, а помогать **понимать гармонический контекст, управлять напряжением и превращать изученный vocabulary в собственный музыкальный язык**.

## Документация

- [`docs/CURRENT_STATE.md`](docs/CURRENT_STATE.md) — текущий checkpoint и следующий шаг.
- [`docs/STAGE_1_TO_STAGE_2_CONTRACT.md`](docs/STAGE_1_TO_STAGE_2_CONTRACT.md) — граница Stage 1 → Harmonic Engine.
- [`docs/CORE_DATA_MODEL_0.0b.md`](docs/CORE_DATA_MODEL_0.0b.md) — host-neutral data model.
- [`docs/PROJECT_CONTEXT.md`](docs/PROJECT_CONTEXT.md) — living document проекта.
- [`docs/ROADMAP.md`](docs/ROADMAP.md) — Stage и build checkpoints.
- [`docs/VERSIONING.md`](docs/VERSIONING.md) — схема версий и fix-сборок.
- [`docs/ARCHITECTURAL_DECISIONS.md`](docs/ARCHITECTURAL_DECISIONS.md) — архитектурные решения.

## Ближайший технический шаг

Довести CI PR #19 до зелёного состояния, получить `Smart-Improviser-0.2e-Windows` и провести live-test ambiguity/confidence в Fender Studio Pro.
