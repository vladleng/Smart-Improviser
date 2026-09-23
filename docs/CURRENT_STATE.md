# Smart Improviser — Current State

> **Назначение:** короткая точка входа для нового чата или новой рабочей сессии.  
> Этот файл должен отвечать на вопрос: **где проект находится сейчас и что делать следующим?**  
> Подробная архитектура хранится в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Активный Stage:** Stage 2 — Harmonic Engine
- **Текущая рабочая версия:** `0.2a`
- **Stage 1 Issue:** #2 — Stage 1 — ARA Context Monitor
- **Stage 2 Issue:** #3 — Stage 2 — Harmonic Engine
- **Активная ветка:** `stage-2-harmonic-engine`
- **Активный PR:** #15 — `0.2a — Stage 2 Harmonic Engine foundation`

Stage 1 принят после живых тестов `0.1a fix1` и `0.1b` в Fender Studio Pro. Финальная сборка этапа имеет версию `0.2`.

Stage 2 начат с `0.2a`. На этом подэтапе музыкальная логика впервые вынесена в отдельный host-neutral `HarmonicEngine`, работающий поверх уже готового `TimelineHarmonicSnapshot`.

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
```

Core и Harmonic Engine не зависят от Fender Studio Pro, ARA, JUCE, VST3, UI или старого Smart Voicing engine.

## Что подтверждено в живых тестах Stage 1

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
- exact chord boundary корректно переключает current chord;
- Tempo / Time Signature совпадают с DAW;
- изменения Chord Track / Key Track обновляют snapshot;
- после повторного открытия проекта binding/context восстанавливаются;
- длина Audio Event не ограничивает Musical Context: Event является только ARA anchor;
- explicit no-chord event обрабатывается как `available=true`, `defined=false`;
- отсутствие Key events не ломает Chord context;
- отсутствие активного chord event не ломает ARA connection;
- до первого chord event корректно сохраняется future/next context;
- после последнего event обработка остаётся безопасной и детерминированной.

Multiple Musical Context не является блокером `0.2`. В `0.1b` реализована детерминированная policy выбора: максимум доступных content types → максимум events → первый context при равенстве; UI показывает `Musical contexts N | selected M`.

## Стабильная 0.2 — Stage 1

### ARA / host integration

- ARA 2 Event FX для Fender Studio Pro;
- Key Signature, Sheet Chords, Tempo Entries, Bar Signatures;
- transport position / playing state;
- изолированный Smart Improviser shared-memory ABI.

### Host-neutral context layer

- `SharedHarmonicContextData.h`;
- `TimelineContextMapper.h/.cpp`;
- `SmartImproviserContext` без JUCE/ARA зависимости;
- `ARAContextProvider`;
- `HarmonicContext`;
- `TimelineHarmonicSnapshot`.

### Stage 1 → Stage 2 contract

Формальный контракт:

```text
docs/STAGE_1_TO_STAGE_2_CONTRACT.md
```

Stage 2 получает host-neutral `TimelineHarmonicSnapshot` и не знает деталей ARA SDK, JUCE, shared memory или Fender Studio Pro.

## Рабочая 0.2a — Harmonic Engine foundation

В `0.2a` реализуется первый контекстный музыкальный анализ.

Уже добавлено в активной ветке:

- `src/core/analysis/HarmonicEngine.h/.cpp`;
- host-neutral entry point `analyzeHarmonicSituation()`;
- распознавание major `ii–V–I` для текущей позиции V;
- распознавание minor `iiø–V–i` для текущей позиции V;
- fallback `V–I`;
- secondary dominant с приоритетом над generic dominant-to-target;
- temporary local key center для подтверждённого applied dominant;
- scope `temporary`, чтобы tonicization не трактовалась как полноценная модуляция;
- safe non-analysis при missing position/current chord/global key;
- отдельный regression target `SmartImproviserHarmonicEngineTests`.

Пока **не входят / не завершены**:

- полный pattern-position analysis для всех членов оборота;
- `I–VI–ii–V`;
- tritone substitution / `ii–SubV–I`;
- длинные tonicization chains;
- полноценное local-key/modulation inference;
- ambiguity / alternative interpretations.

## Версия 0.2a

Build label:

```text
Smart Improviser 0.2a
```

CMake project version:

```text
0.2.1
```

GitHub Actions artifact после успешного CI:

```text
Smart-Improviser-0.2a-Windows
└── Smart Improviser.vst3
```

Устанавливаемая папка остаётся `Smart Improviser.vst3` и целиком заменяет предыдущую версию.

## Regression tests

Для `0.2a` CI должен запускать:

```text
SmartImproviserCoreTests
SmartImproviserDataModelTests
SmartImproviserHarmonicEngineTests
SmartImproviserTimelineContextTests
```

## Рабочая линия

```text
Stage 0 → 0.1  [COMPLETED]
Stage 1 → 0.2  [COMPLETED]
Stage 2 → 0.2a ... → 0.3  [ACTIVE]
```

## Что делать следующим

1. дождаться и проверить Windows CI PR #15;
2. если CI зелёный — скачать `Smart-Improviser-0.2a-Windows`;
3. проверить, что Stage 1 ARA/context diagnostics не получили regression;
4. после принятия `0.2a` перейти к следующему подэтапу Stage 2: расширению pattern-position analysis, turnaround/tritone-substitution и ambiguity/local-center logic.

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
