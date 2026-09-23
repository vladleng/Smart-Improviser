# Smart Improviser — Current State

> **Назначение:** короткая точка входа для нового чата или новой рабочей сессии.  
> Этот файл должен отвечать на вопрос: **где проект находится сейчас и что делать следующим?**  
> Подробная архитектура хранится в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 1 — ARA Context Monitor
- **Текущая стабильная версия:** `0.2`
- **Следующий Stage:** Stage 2 — Harmonic Engine
- **Следующая рабочая версия:** `0.2a`
- **Stage 1 Issue:** #2 — Stage 1 — ARA Context Monitor
- **Stage 1 ветка:** `stage-1-ara-context-monitor`
- **Stage 1 PR:** #14

Stage 1 принят после живых тестов `0.1a fix1` и `0.1b` в Fender Studio Pro. Финальная сборка этапа имеет версию `0.2`.

## Завершённая архитектурная цепочка Stage 1

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
HarmonicContext / TimelineHarmonicSnapshot
        ↓
HarmonicSituation
        ↓
Stage 2 Harmonic Engine
```

Core не зависит от Fender Studio Pro, ARA, JUCE, VST3, UI или старого Smart Voicing engine.

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

Подтверждённые примеры:

```text
Previous chord  C
Current chord   (no chord)
Next chord      Cmaj7
```

```text
Previous chord  Cmaj7
Current chord   Am7
Next chord      Dm7
```

```text
Key             -
Previous chord  -
Current chord   (no chord)
Next chord      C
Tempo           120 BPM
Time signature  4/4
```

Multiple Musical Context не является блокером `0.2`, если отдельный воспроизводимый сценарий Studio Pro недоступен. В `0.1b` реализована детерминированная policy выбора: максимум доступных content types → максимум events → первый context при равенстве; UI показывает `Musical contexts N | selected M`.

## Что входит в стабильную 0.2

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

### Edge-case semantics

- missing Key / Chord / Tempo sources;
- undefined key/chord event;
- explicit no-chord;
- before-first chord;
- after-last chord;
- exact event boundary;
- safe PPQ ↔ seconds conversion;
- single tempo anchor без выдумывания неизвестного tempo slope.

### Regression tests

CI запускает:

```text
SmartImproviserCoreTests
SmartImproviserDataModelTests
SmartImproviserTimelineContextTests
```

### Stage 1 → Stage 2 contract

Формальный контракт зафиксирован в:

```text
docs/STAGE_1_TO_STAGE_2_CONTRACT.md
```

Stage 2 получает host-neutral `TimelineHarmonicSnapshot` / `HarmonicContext` и не должен знать деталей ARA SDK, JUCE, shared memory или Fender Studio Pro.

## Финальная сборка Stage 1

Build label:

```text
Smart Improviser 0.2
```

CMake project version:

```text
0.2.0
```

GitHub Actions artifact:

```text
Smart-Improviser-0.2-Windows
└── Smart Improviser.vst3
```

Устанавливаемая папка всегда называется `Smart Improviser.vst3` и целиком заменяет предыдущую версию.

## Следующий этап — Stage 2 / 0.2a

Stage 2 — **Harmonic Engine**.

Первый подэтап `0.2a` должен начать музыкальный анализ уже готового `TimelineHarmonicSnapshot`:

1. global key и подготовка local key center;
2. basic harmonic functions;
3. previous/current/next analysis;
4. базовый pattern recognition;
5. первые целевые паттерны: major ii–V–I, minor iiø–V–i, V–I, I–VI–ii–V, secondary dominant, tritone substitution.

## Рабочая линия

```text
Stage 0 → 0.1  [COMPLETED]
Stage 1 → 0.2  [COMPLETED]
Stage 2 → 0.2a ... → 0.3
```

## Что читать в новом чате Stage 2

1. `docs/CURRENT_STATE.md`;
2. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. Issue следующего Stage 2 после его создания.

## Правило обновления этого файла

`CURRENT_STATE.md` обновляется при переходе на следующую буквенную версию, после существенного `fixN`, важного live-test, смены активного PR/ветки, завершения Stage и перед переходом разработки в новый чат.
