# Smart Improviser — Current State

> **Назначение:** короткая точка входа для нового чата или новой рабочей сессии.  
> Этот файл должен отвечать на вопрос: **где проект находится сейчас и что делать следующим?**  
> Подробная архитектура хранится в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Текущая рабочая версия:** `0.2a fix1`
- **Stage 1 Issue:** #2 — Stage 1 — ARA Context Monitor
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **Активная ветка:** `stage-2-harmonic-engine`
- **Активный PR:** #15 — `0.2a fix1 — Stage 2 Harmonic Engine diagnostics`

Stage 1 принят после живых тестов `0.1a fix1` и `0.1b` в Fender Studio Pro. Финальная сборка этапа имеет версию `0.2`.

Stage 2 начат с `0.2a`. Базовая `0.2a` успешно установилась в Fender Studio Pro и визуально подтвердила отсутствие regression Stage 1: ARA binding, Musical Context, transport, key, previous/current/next chords, tempo/time signature и revisions работают. При этом первоначальный UI показывал только Stage 1 diagnostics, поэтому для полноценной приёмки Harmonic Engine создан `0.2a fix1`.

## Общее правило Stage / build versions

Каждый Stage проекта делится на логически завершённые подэтапы. Каждому подэтапу соответствует отдельная буквенная build-версия.

```text
новая буква = новая логическая часть Stage
fixN        = исправление текущей логической части
числовая версия без буквы = Stage complete
```

Буквенная версия является самостоятельным build checkpoint с понятной целью, checklist и тестами. Stage не должен разрабатываться одной большой буквенной версией.

Нормативное описание этого правила находится в `docs/VERSIONING.md`, а план подэтапов — в `docs/ROADMAP.md` и соответствующем Stage Issue.

## Архитектурная цепочка

```text
Fender Studio Pro / ARA 2
        ↓
Musical Context
        ↓
SharedHarmonicContextSnapshot
        ↓
TimelineContextMapper
        ↓
ARAContextProvider
        ↓
TimelineHarmonicSnapshot
        ↓
buildHarmonicSituation()
        ↓
analyzeHarmonicSituation() / Harmonic Engine
        ↓
HarmonicSituation
        ↓
Diagnostic UI / future product UI
```

Core и Harmonic Engine не зависят от Fender Studio Pro, ARA, JUCE, VST3, UI или старого Smart Voicing engine. UI только отображает уже рассчитанный host-neutral `HarmonicSituation`.

## Что подтверждено в живых тестах Stage 1 / regression 0.2a

На реальном проекте Fender Studio Pro подтверждено:

- ARA binding — **BOUND**;
- Document Controller — **YES**;
- Host Content Access — **YES**;
- Musical Context — доступен;
- Shared Context — **YES**;
- STOP / PLAY / seek — корректны;
- PPQ и seconds — корректно обновляются;
- Key Track — читается и нормализуется;
- Chord Track — читается и нормализуется;
- previous/current/next совпадают с Chord Track;
- Tempo / Time Signature совпадают с DAW;
- revisions обновляются;
- базовая `0.2a` действительно установлена и показывает build label `Smart Improviser 0.2a`.

## Stage 2 — план логических подэтапов

```text
0.2a — Harmonic Engine foundation
0.2b — Pattern Recognizer
0.2c — Tritone Substitution
0.2d — Local Key Center
0.2e — Ambiguity / Confidence
0.2f — Integration / musical validation
0.3  — Stage 2 complete
```

План может уточняться, если в ходе разработки обнаружится отдельная самостоятельная задача. Такая задача должна получить собственную следующую буквенную версию, а не быть незаметно добавлена в уже принятый подэтап.

## 0.2a — Harmonic Engine foundation

Реализовано:

- `src/core/analysis/HarmonicEngine.h/.cpp`;
- host-neutral entry point `analyzeHarmonicSituation()`;
- анализ global key + previous/current/next context;
- major `ii–V–I` для текущей позиции V;
- minor `iiø–V–i` для текущей позиции V;
- fallback `V–I`;
- secondary dominant с приоритетом над generic dominant-to-target;
- temporary local key center для подтверждённого applied dominant;
- scope `temporary`, чтобы tonicization не трактовалась как полноценная модуляция;
- safe non-analysis при missing position/current chord/global key;
- regression target `SmartImproviserHarmonicEngineTests`.

## 0.2a fix1 — Harmonic Engine diagnostics

Причина fix:

Базовая `0.2a` показывала в плагине только Stage 1 ARA Context Monitor. Harmonic Engine работал в Core и покрывался тестами, но его результат невозможно было проверить непосредственно в Fender Studio Pro.

В `0.2a fix1` добавлено:

- расширенное диагностическое окно Stage 1 + Stage 2;
- прямой вызов `analyzeHarmonicSituation(timeline)` только на UI/diagnostic boundary;
- отображение `Situation: VALID / NO ANALYSIS`;
- `Function`: scale degree + effective harmonic function;
- `Relation`: diatonic/chromatic;
- `Local center`: tonic/mode + scope (`temporary` и т.д.);
- `Pattern`;
- `Pattern position`: role + index/length;
- `Resolution`: target chord + confirmed status;
- `Confidence`: confidence + interpretation status;
- увеличена высота diagnostic UI, чтобы Stage 1 и Stage 2 данные были видны одновременно.

Важно: это **не новый функциональный подэтап Stage 2**, поэтому версия остаётся `0.2a fix1`, а не `0.2b`.

## Версия 0.2a fix1

Build label:

```text
Smart Improviser 0.2a fix1
```

CMake project version:

```text
0.2.2
```

GitHub Actions artifact:

```text
Smart-Improviser-0.2a-fix1-Windows
└── Smart Improviser.vst3
```

Устанавливаемая папка остаётся `Smart Improviser.vst3` и целиком заменяет предыдущую версию.

## Regression tests

CI запускает:

```text
SmartImproviserCoreTests
SmartImproviserDataModelTests
SmartImproviserHarmonicEngineTests
SmartImproviserTimelineContextTests
```

## Что относится к следующим буквенным версиям

- полный pattern-position analysis для всех членов оборота → `0.2b`;
- `I–VI–ii–V` и расширенный Pattern Recognizer → `0.2b`;
- tritone substitution / `ii–SubV–I` → `0.2c`;
- длинные tonicization chains и полноценный local-key analysis → `0.2d`;
- ambiguity / alternative interpretations → `0.2e`;
- общая интеграция и musical validation → `0.2f`.

## Рабочая линия

```text
Stage 0 → 0.1  [COMPLETED]
Stage 1 → 0.2  [COMPLETED]
Stage 2 → 0.2a → 0.2a fix1 → 0.2b → 0.2c → 0.2d → 0.2e → 0.2f → 0.3  [ACTIVE]
```

## Что делать следующим

1. дождаться зелёного Windows CI для `0.2a fix1`;
2. скачать `Smart-Improviser-0.2a-fix1-Windows`;
3. заменить установленную папку `Smart Improviser.vst3`;
4. проверить Stage 1 regression;
5. проверить Stage 2 diagnostics на нескольких гармонических ситуациях;
6. после принятия `0.2a fix1` закрыть checkpoint `0.2a` и перейти к `0.2b — Pattern Recognizer`.

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue #3 — Stage 2 — Harmonic Engine.

## Правило обновления этого файла

`CURRENT_STATE.md` обновляется при переходе на следующую буквенную версию, после существенного `fixN`, важного live-test, смены активного PR/ветки, завершения Stage и перед переходом разработки в новый чат.
