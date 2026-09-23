# Smart Improviser — Current State

> **Назначение:** короткая точка входа для нового чата или новой рабочей сессии.  
> Этот файл должен отвечать на вопрос: **где проект находится сейчас и что делать следующим?**  
> Подробная архитектура хранится в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 0 — Спецификация Smart Improviser Core
- **Текущая стабильная версия:** `0.1`
- **Последний рабочий подэтап:** `0.0b` — Core data model
- **Следующий Stage:** Stage 1 — ARA Context Monitor
- **Следующая рабочая версия:** `0.1a`
- **Следующий Issue:** #2 — Stage 1 — ARA Context Monitor

Stage 0 завершён. Новый чат проекта должен начинаться уже со Stage 1.

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

## Архитектурная граница после Stage 0

```text
DAW / ARA
    ↓
Timeline Context
    ↓
TimelineHarmonicSnapshot
    ↓
HarmonicSituation
    ↓
будущие Harmonic / Tension / Improvisation analyzers
```

Core не зависит от Fender Studio Pro, ARA, JUCE, VST3, UI или старого Voicing engine.

Pattern recognition и local-key inference **не выполняются data-model layer**. Модель хранит их результат, а вычисление относится к Stage 2.

## Что проверено

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

## Что ещё НЕ проверено

Живые DAW-тесты относятся к Stage 1:

- загрузка `Smart Improviser ARA.vst3` в Fender Studio Pro;
- ARA binding;
- получение Key Track / Chord Track;
- Tempo / Time Signature;
- STOP / PLAY / seek;
- точные chord boundaries;
- обновление context после редактирования Chord/Key Track;
- повторное открытие проекта.

## Следующий Stage — Stage 1

Рабочая линия:

```text
0.1a → 0.1b → ... → 0.2
```

Первый фокус `0.1a`:

1. подготовить тестовый VST3 package;
2. выполнить первый live test в Fender Studio Pro;
3. подтвердить ARA binding и Chord/Key context;
4. проверить transport и event boundaries;
5. зафиксировать найденные host-specific проблемы.

Если живой тест `0.1a` выявит ошибку, применяется `0.1a fix1`, `fix2` и т.д.

## Что читать в новом чате Stage 1

1. `docs/CURRENT_STATE.md`;
2. Issue #2 — Stage 1 — ARA Context Monitor;
3. `docs/CORE_DATA_MODEL_0.0b.md`;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. `docs/MIGRATION_FROM_SMART_VOICING.md` при необходимости.

## Шаблон старта следующего чата

> Продолжаем разработку Smart Improviser. Stage 0 завершён версией `0.1`. Репозиторий: `https://github.com/vladleng/Smart-Improviser`. Прочитай `docs/CURRENT_STATE.md`, Issue #2 и архитектурную документацию. Начинаем Stage 1 с версии `0.1a`.

## Правило обновления этого файла

`CURRENT_STATE.md` обновляется при переходе на следующую буквенную версию, после существенного `fixN`, важного live-test, смены активного PR/ветки, завершения Stage и перед переходом разработки в новый чат.
