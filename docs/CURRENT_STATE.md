# Smart Improviser — Current State

> **Назначение:** короткая точка входа для нового чата или новой рабочей сессии.  
> Этот файл должен отвечать на вопрос: **где проект находится сейчас и что делать следующим?**  
> Подробная архитектура хранится в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 0 — Спецификация Smart Improviser Core
- **Текущая стабильная версия:** `0.1`
- **Активный Stage:** Stage 1 — ARA Context Monitor
- **Принятый checkpoint Stage 1:** `0.1a fix1`
- **Текущая рабочая версия:** `0.1b`
- **Активный Issue:** #2 — Stage 1 — ARA Context Monitor
- **Активная ветка:** `stage-1-ara-context-monitor`
- **Активный PR:** #14

`0.1a fix1` принят после живых тестов в Fender Studio Pro. Основной ARA/timeline path подтверждён на реальном проекте, включая PLAY, seek, chord boundaries, edit refresh и reopen.

`0.1b` начат как подэтап устойчивости и формального контракта Stage 1 → Stage 2.

## Архитектурная граница

```text
DAW / ARA
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

Core не зависит от Fender Studio Pro, ARA, JUCE, VST3, UI или старого Voicing engine.

## Что подтверждено в 0.1a / 0.1a fix1

На реальном проекте Fender Studio Pro подтверждено:

- ARA binding — **BOUND**;
- Document Controller — **YES**;
- Host Content Access — **YES**;
- Musical Context — доступен;
- Shared Context — **YES**;
- STOP position — корректна;
- PLAY — PPQ/context обновляются во время воспроизведения;
- seek — контекст обновляется немедленно;
- Key Track — читается и нормализуется;
- Chord Track — читается и нормализуется;
- previous/current/next — совпадают с реальным Chord Track;
- точное переключение на chord boundary — подтверждено;
- Tempo / Time Signature — совпадают с DAW;
- изменение Chord Track / Key Track обновляет context snapshot;
- после повторного открытия проекта ARA binding/context восстанавливаются;
- короткий Audio Event не ограничивает Musical Context и служит только точкой ARA binding;
- явный no-chord участок обрабатывается безопасно.

Примеры:

```text
PPQ 172.000
Previous chord  C
Current chord   (no chord)
Next chord      Cmaj7
```

```text
PPQ 180.000
Previous chord  Cmaj7
Current chord   Am7
Next chord      Dm7
```

```text
Previous chord  Am7
Current chord   Dm7
Next chord      G13
Key             C major
Tempo           110 BPM
Time signature  4/4
```

`G13` — нормализованное представление `G7add13` из Chord Track.

## Что реализовано в 0.1b

### 1. Testable Stage 1 context layer

Добавлены:

- `src/context/SharedHarmonicContextData.h` — JUCE-free структуры shared snapshot;
- `src/context/TimelineContextMapper.h/.cpp` — единая логика преобразования shared/ARA context в host-neutral Core context;
- `SmartImproviserContext` — отдельная статическая библиотека без JUCE/ARA зависимости.

`ARAContextProvider` и диагностический UI теперь используют один и тот же `TimelineContextMapper`, чтобы live diagnostics и фактический Core-ready snapshot не расходились.

### 2. Missing-data semantics

ARA connection больше не считается потерянным только из-за отсутствия конкретных Key/Chord/Tempo/Bar данных.

```text
providerConnected = usable ARA Musical Context
source.available   = наличие конкретного типа данных
source.defined     = конкретное event содержит музыкальное определение
```

Это позволяет отличать:

```text
Provider disconnected
```

от:

```text
Provider connected, но Chord Track / Key Track / Tempo content отсутствует
```

### 3. Timeline edge cases

Зафиксировано поведение:

- до первого chord event:
  - current отсутствует;
  - previous отсутствует;
  - next = первый будущий chord event;
- exact event boundary активирует новый current chord;
- explicit no-chord event имеет `available=true`, `defined=false`;
- после последнего chord event последний event остаётся current до следующего event/no-chord marker;
- empty/undefined key event не превращается в фиктивную тональность;
- отсутствие Tempo data возвращает unavailable conversion вместо фиктивных значений;
- один tempo anchor не используется для выдумывания неизвестного tempo slope.

### 4. Multiple Musical Context policy

Если host предоставляет несколько Musical Context, Stage 1 выбирает один детерминированно:

1. максимум доступных типов `Key / Chords / Tempo / Bars`;
2. затем максимум общего количества events;
3. при равенстве — первый context в host order.

Diagnostic UI теперь показывает:

```text
Musical contexts  N  |  selected M
```

### 5. Regression tests

Добавлен новый CI test target:

```text
SmartImproviserTimelineContextTests
```

Он проверяет без DAW:

- connected provider при полном отсутствии harmonic content sources;
- before-first-chord;
- exact boundary;
- explicit no-chord;
- after-last-chord;
- undefined key event;
- missing tempo;
- PPQ ↔ seconds conversion;
- single tempo anchor.

### 6. Формальный Stage 1 → Stage 2 contract

Добавлен:

```text
docs/STAGE_1_TO_STAGE_2_CONTRACT.md
```

Stage 2 не должен зависеть от ARA SDK, JUCE, shared memory или Fender Studio Pro behavior. Его вход — `HarmonicContext` / `TimelineHarmonicSnapshot`.

## Сборка 0.1b

Build label:

```text
Smart Improviser 0.1b
```

GitHub Actions artifact:

```text
Smart-Improviser-0.1b-Windows
└── Smart Improviser.vst3
```

Устанавливаемая папка по-прежнему всегда называется `Smart Improviser.vst3` и заменяет предыдущую версию целиком.

## Что нужно проверить для принятия 0.1b

Подробный сценарий:

```text
docs/STAGE_1_0.1b_LIVE_TEST.md
```

Основные live checks:

1. убедиться, что обычный проект 0.1a fix1 не регрессировал;
2. очистить Chord Track и проверить, что connection сохраняется, а chord context становится unavailable;
3. очистить Key Track и проверить, что chords продолжают работать;
4. проверить позицию до первого chord event — `Next` должен показывать первый будущий chord;
5. проверить позицию после последнего chord event;
6. проверить explicit no-chord;
7. если удаётся создать несколько Musical Context — проверить `selected N` и отсутствие смешивания событий.

## Рабочая линия Stage 1

```text
0.1a
→ 0.1a fix1  [ACCEPTED]
→ 0.1b       [IN DEVELOPMENT]
→ при необходимости следующие 0.1x
→ 0.2
```

## Ближайший следующий шаг

1. получить зелёный Windows CI для `0.1b`;
2. скачать `Smart-Improviser-0.1b-Windows`;
3. заменить `Smart Improviser.vst3`;
4. выполнить edge-case live test по `docs/STAGE_1_0.1b_LIVE_TEST.md`;
5. при успешной приёмке определить, нужен ли ещё один Stage 1 checkpoint или можно финализировать Stage 1 как `0.2`.

## Что читать в новом чате Stage 1

1. `docs/CURRENT_STATE.md`;
2. Issue #2 — Stage 1 — ARA Context Monitor;
3. `docs/STAGE_1_0.1b_LIVE_TEST.md`;
4. `docs/STAGE_1_TO_STAGE_2_CONTRACT.md`;
5. `docs/CORE_DATA_MODEL_0.0b.md`;
6. `docs/PROJECT_CONTEXT.md`;
7. `docs/ARCHITECTURAL_DECISIONS.md`;
8. `docs/ROADMAP.md`;
9. `docs/VERSIONING.md`;
10. `docs/MIGRATION_FROM_SMART_VOICING.md` при необходимости.

## Правило обновления этого файла

`CURRENT_STATE.md` обновляется при переходе на следующую буквенную версию, после существенного `fixN`, важного live-test, смены активного PR/ветки, завершения Stage и перед переходом разработки в новый чат.
