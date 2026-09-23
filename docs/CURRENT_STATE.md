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

Stage 1 находится в первом live-test checkpoint. Базовая ARA-инфраструктура уже подтверждена в Fender Studio Pro; `0.1a fix1` исправляет диагностическое отображение Key/Chord и UI после первого живого теста.

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
- короткий Audio Event в начале проекта не ограничивает доступный Musical Context и работает только как точка ARA binding.

Таким образом основной ARA path Stage 1 уже работает:

```text
Studio Pro
→ Event FX / ARA binding
→ Musical Context
→ SharedHarmonicContext
→ transport / key / chord / tempo / bar events
```

## Почему понадобился 0.1a fix1

Первый live test выявил три проблемы диагностического слоя, а не ARA foundation:

1. Studio Pro передаёт Chord/Key структурно (`root`, `bass`, `intervals`), но не заполняет `event.name`, поэтому UI показывал `(unnamed)`.
2. Строка `Revisions` перекрывалась footer-текстом.
3. Длинное тире в диагностическом заголовке отображалось с mojibake.

## Что сделано в 0.1a fix1

- Chord symbols теперь строятся из структурных ARA-данных через `ChordModel` / `normalizedChordSymbol()`;
- Key display строится через `KeyModel` из `root + intervals`;
- ARA `name` остаётся только fallback;
- высота диагностического окна увеличена с 520 до 570 px;
- footer отделён от строки `Revisions`;
- диагностический UI использует ASCII punctuation в местах, где была проблема кодировки;
- build label изменён на `0.1a fix1`;
- CI artifact: `Smart-Improviser-0.1a-fix1-Windows`;
- внутри artifact по-прежнему готовая папка `Smart Improviser.vst3`.

## Что проверить в 0.1a fix1

1. Key должен отображаться как музыкальное имя, например `C major`.
2. Previous / Current / Next chord должны отображаться реальными символами.
3. На последовательности `Dm7 | G7 | Cmaj7` при курсоре на G7 ожидается:

```text
Previous chord  Dm7
Current chord   G7
Next chord      Cmaj7
```

4. Перемещение курсора должно переключать current chord точно на event boundary.
5. PLAY / STOP / seek должны сохранять корректное обновление PPQ/context.
6. Tempo и Time Signature должны продолжать совпадать с DAW.
7. `Revisions` и footer должны отображаться без перекрытия.
8. После редактирования Chord/Key Track context должен обновляться.
9. После повторного открытия проекта ARA binding/context должен восстанавливаться.

Подробно: `docs/STAGE_1_0.1a_LIVE_TEST.md`.

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

1. получить успешный Windows CI для `0.1a fix1`;
2. скачать artifact `Smart-Improviser-0.1a-fix1-Windows`;
3. заменить предыдущую папку на новую `Smart Improviser.vst3`;
4. проверить Key и previous/current/next chord на реальном Chord Track;
5. проверить PLAY / seek / chord boundaries;
6. результат занести в Issue #2;
7. при ошибке выпустить `0.1a fix2`, при принятии 0.1a перейти к следующему подэтапу `0.1b`.

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
