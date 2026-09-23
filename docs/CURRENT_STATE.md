# Smart Improviser — Current State

> **Назначение:** короткая точка входа для нового чата или новой рабочей сессии.  
> Этот файл должен отвечать на вопрос: **где проект находится сейчас и что делать следующим?**  
> Подробная архитектура хранится в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 0 — Спецификация Smart Improviser Core
- **Текущая стабильная версия:** `0.1`
- **Активный Stage:** Stage 1 — ARA Context Monitor
- **Текущая рабочая версия:** `0.1a`
- **Активный Issue:** #2 — Stage 1 — ARA Context Monitor
- **Активная ветка:** `stage-1-ara-context-monitor`

Stage 1 начат. Версия `0.1a` готовится как первый живой ARA/context checkpoint для Fender Studio Pro.

## Что вошло в 0.1

### Фундамент из 0.0a
- host-neutral `HarmonicContext` без Voicing/Voice сущностей;
- `ChordModel`;
- `KeyModel`;
- базовый `HarmonicFunction`;
- real next-chord resolution evidence;
- headless ARA helper;
- ARA Chord/Key/Tempo/Bar/Transport context;
- `ARAContextProvider`;
- изолированный Smart Improviser shared-memory ABI;
- Windows CI и базовые Core regression tests.

### Core data model из 0.0b
- `AnalysisEvidence` / `ConfidenceLevel`;
- `InterpretationStatus` (`unknown / unique / ambiguous`) + alternative count;
- `KeyCenter` со scope `global / local / temporary / modal`;
- `HarmonicPattern` + роль/позиция внутри паттерна;
- `ResolutionTarget` + structural tendency/resolution moves;
- `TensionLevel` 1/2/3;
- базовый `ImprovisationStrategy` contract;
- минимальный семантический `Phrase` contract;
- центральный `HarmonicSituation`;
- host-neutral `TimelineHarmonicSnapshot → HarmonicSituation` builder;
- major ii–V–I и minor iiø–V–i reference cases;
- отдельные Core data-model regression tests.

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

Pattern recognition и local-key inference **не выполняются data-model layer**. Модель хранит их результат, а вычисление относится к Stage 2.

## Что уже сделано в 0.1a

- создана ветка `stage-1-ara-context-monitor`;
- `ARAContextProvider` теперь имеет явный Core-ready API:
  - `currentTimelineSnapshot()`;
  - `timelineSnapshotAt(ppq)`;
- snapshot формирует:
  - previous chord;
  - current chord;
  - next chord;
  - global key;
  - PPQ position;
- `ARAContextProvider.cpp` теперь реально входит в Windows build target, поэтому Stage 1 provider проверяется компилятором в CI;
- ARA helper линкуется с `SmartImproviserCore`;
- build version для checkpoint зафиксирована как `0.1a`;
- Windows workflow публикует VST3 artifact `Smart-Improviser-0.1a-Windows`;
- добавлен `docs/STAGE_1_0.1a_LIVE_TEST.md` с чек-листом первого теста в Fender Studio Pro.

## Что проверено ранее

Версия `0.1` прошла:

- Windows CMake Configure — **OK**;
- Windows Build — **OK**;
- `SmartImproviserCoreTests` — **OK**;
- `SmartImproviserDataModelTests` — **OK**;
- сборка headless ARA helper — **OK**.

Эталонный major case:

```text
C major
Dm7 → G7 → Cmaj7
```

Для `G7` builder формирует confirmed resolution на `Cmaj7` и структурные движения `B→C`, `F→E`.

Эталонный minor case:

```text
A minor
Bm7b5 → E7 → Am
```

Для `E7` builder подтверждает resolution на `Am` и сохраняет major/minor quality target.

## Что должен подтвердить 0.1a

Первый live test относится к реальной DAW и должен проверить:

- загрузку `Smart Improviser ARA.vst3` в Fender Studio Pro;
- ARA binding;
- получение Key Track / Chord Track;
- Tempo / Time Signature;
- STOP / PLAY / seek;
- точные chord boundaries;
- previous/current/next snapshot на реальном timeline;
- обновление context после редактирования Chord/Key Track;
- несколько Musical Context;
- повторное открытие проекта.

Подробный сценарий: `docs/STAGE_1_0.1a_LIVE_TEST.md`.

## Рабочая линия Stage 1

```text
0.1a → 0.1b → ... → 0.2
```

Если живой тест `0.1a` выявит ошибку, применяется `0.1a fix1`, `0.1a fix2` и т.д. Новая буква означает новый подэтап, а не исправление текущего checkpoint.

## Ближайший следующий шаг

1. дождаться успешного Windows CI для ветки/PR 0.1a;
2. скачать artifact `Smart-Improviser-0.1a-Windows`;
3. выполнить live test по `docs/STAGE_1_0.1a_LIVE_TEST.md`;
4. результаты занести в Issue #2;
5. при ошибках выпустить `0.1a fixN`, при успешном принятии перейти к следующему подэтапу `0.1b`.

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
