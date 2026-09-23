# Smart Improviser — Current State

> **Назначение:** короткая точка входа для нового чата или новой рабочей сессии.  
> Этот файл должен отвечать на вопрос: **где проект находится сейчас и что делать следующим?**  
> Подробная архитектура хранится в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 0 — Спецификация Smart Improviser Core
- **Текущая стабильная версия:** `0.1`
- **Активный Stage:** Stage 1 — ARA Context Monitor
- **Текущая рабочая версия:** `0.1a fix1`
- **Активный Issue:** #2 — Stage 1 — ARA Context Monitor
- **Активная ветка:** `stage-1-ara-context-monitor`
- **Активный PR:** #14

`0.1a fix1` успешно прошёл второй живой тест в Fender Studio Pro. ARA binding, Musical Context, STOP position, PPQ, Key, previous/current/next chord, Tempo и Time Signature отображаются корректно на реальном проекте.

## Архитектурная граница

```text
DAW / ARA
    ↓
Timeline Context
    ↓
ARAContextProvider
    ↓
TimelineHarmonicSnapshot
    ↓
HarmonicSituation
    ↓
будущие Harmonic / Tension / Improvisation analyzers
```

Core не зависит от Fender Studio Pro, ARA, JUCE, VST3, UI или старого Voicing engine.

## Что вошло в 0.1a

- `ARAContextProvider` получил Core-ready API:
  - `currentTimelineSnapshot()`;
  - `timelineSnapshotAt(ppq)`;
- snapshot формирует previous/current/next chord, global key и PPQ;
- `ARAContextProvider.cpp` реально входит в Windows build target;
- ARA helper линкуется с `SmartImproviserCore`;
- добавлен временный Stage 1 UI `ARA Context Monitor`;
- Windows CI формирует готовый drop-in package:

```text
Smart Improviser.vst3
```

Имя установленной VST3-папки остаётся постоянным между версиями и `fixN`.

## Результат первого live-теста 0.1a

На реальном проекте Fender Studio Pro подтверждено:

- ARA binding — **BOUND**;
- Document Controller — **YES**;
- Host Content Access — **YES**;
- Musical Contexts — **1**;
- Shared Context — **YES**;
- STOP position — **OK**;
- PPQ — **OK**, начало такта 30 = `116.000`;
- Time Signature — **4/4**, совпадает с DAW;
- Tempo — **110.00 BPM**, совпадает с DAW;
- ARA events получены: `Key 2 | Chords 13 | Tempo 16 | Bars 1`;
- короткий Audio Event не ограничивает доступный Musical Context и работает только как точка ARA binding.

Первый тест также показал, что Studio Pro передаёт Key/Chord структурно (`root`, `bass`, `intervals`), но не обязан заполнять ARA `event.name`.

## Что исправлено в 0.1a fix1

- Chord symbols строятся из структурных ARA-данных через `ChordModel` / `normalizedChordSymbol()`;
- Key display строится через `KeyModel` из `root + intervals`;
- ARA `name` остаётся только fallback;
- высота диагностического окна увеличена;
- footer больше не перекрывает `Revisions`;
- устранена проблема кодировки диагностического заголовка;
- build label — `0.1a fix1`;
- CI artifact — `Smart-Improviser-0.1a-fix1-Windows`;
- внутри artifact — готовая папка `Smart Improviser.vst3`.

## Результат live-теста 0.1a fix1

На позиции внутри реальной последовательности:

```text
Am7 → Dm7 → G7add13
```

Smart Improviser показывает:

```text
ARA binding          BOUND
Document controller  YES
Host content access  YES
Musical contexts     1
Shared context       YES
Transport            STOP
PPQ                  186.000
Seconds              120.007
Key                  C major
Previous chord       Am7
Current chord        Dm7
Next chord           G13
Time signature       4/4
Tempo                110.00 BPM
ARA events           Key 2 | Chords 22 | Tempo 16 | Bars 1
```

`G13` — нормализованное представление `G7add13` из Chord Track.

Это подтверждает реальную цепочку:

```text
Studio Pro
→ Event FX / ARA binding
→ Musical Context
→ SharedHarmonicContext
→ ARAContextProvider
→ previous/current/next + key + timeline
```

## Что ещё нужно проверить перед принятием 0.1a

1. PLAY: PPQ/context обновляются во время воспроизведения.
2. Seek: прыжки курсора назад/вперёд немедленно обновляют context.
3. Chord boundaries: переключение current chord происходит точно на границе события.
4. Edit refresh: изменение Chord Track и Key Track обновляет snapshot без перезагрузки плагина.
5. Reopen: после повторного открытия проекта ARA binding/context восстанавливаются.
6. Missing data: безопасное поведение без Key/Chord/Tempo данных.
7. Multiple Musical Context: проверить реальный проект с более чем одним Musical Context, если Studio Pro создаёт такой сценарий.

## Рабочая линия Stage 1

```text
0.1a
→ 0.1a fix1
→ при необходимости 0.1a fix2...
→ 0.1b
→ ...
→ 0.2
```

Новая буква означает новый подэтап. Исправления текущего live-test checkpoint используют `fixN`.

## Ближайший следующий шаг

Продолжить live-тест `0.1a fix1` по оставшимся сценариям: PLAY, seek, chord boundaries, edit refresh и reopen. Если новых ошибок нет, принять `0.1a` и определить состав следующего подэтапа `0.1b`. Если найдётся ошибка текущего checkpoint — выпустить `0.1a fix2`.

## Что читать в новом чате Stage 1

1. `docs/CURRENT_STATE.md`;
2. Issue #2 — Stage 1 — ARA Context Monitor;
3. `docs/STAGE_1_0.1a_LIVE_TEST.md`;
4. `docs/CORE_DATA_MODEL_0.0b.md`;
5. `docs/PROJECT_CONTEXT.md`;
6. `docs/ARCHITECTURAL_DECISIONS.md`;
7. `docs/ROADMAP.md`;
8. `docs/VERSIONING.md`;
9. `docs/MIGRATION_FROM_SMART_VOICING.md` при необходимости.

## Правило обновления этого файла

`CURRENT_STATE.md` обновляется при переходе на следующую буквенную версию, после существенного `fixN`, важного live-test, смены активного PR/ветки, завершения Stage и перед переходом разработки в новый чат.
