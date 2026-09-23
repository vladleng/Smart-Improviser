# Smart Improviser — Current State

> **Назначение:** короткая точка входа для нового чата или новой рабочей сессии.  
> Этот файл должен отвечать на вопрос: **где проект находится сейчас и что делать следующим?**  
> Подробная архитектура хранится в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Текущий Stage:** Stage 0 — Спецификация Smart Improviser Core
- **Текущая рабочая версия:** `0.0b`
- **Итоговая версия Stage 0:** `0.1`
- **Последний завершённый подэтап:** `0.0a` — перенос чистого Harmony Core и ARA/context foundation
- **Последняя стабильная версия:** пока нет — проект находится до первого стабильного milestone `0.1`
- **Активный Issue:** #1 — Stage 0 — Спецификация Smart Improviser Core
- **Активный PR:** #13 — `0.0b — Stage 0: Core data model`
- **Активная ветка:** `stage-0-0.0b-core-data-model`
- **Активный фокус:** собственная data model Smart Improviser Core

## Что уже сделано в 0.0a

### Host-neutral Core
- `HarmonicContext` очищен от Voicing/Voice сущностей.
- Перенесён `ChordModel`.
- Перенесён `KeyModel`.
- Перенесён базовый `HarmonicFunction`.
- Сохранён анализ реального next-chord resolution для доминант.
- Сохранены базовые regression tests.

### ARA / Context infrastructure
- Перенесён headless ARA helper.
- Получаются Chord / Key / Tempo / Bar данные из ARA Musical Context.
- Получается transport position и playing state.
- Перенесён `ARAContextProvider`.
- Перенесён shared-memory bridge.
- Smart Improviser использует собственный изолированный IPC ABI, не совместный со Smart Voicing.
- Shared context bridge не зависит от JUCE.

### Архитектурная чистота
Сознательно **не перенесены** Voicing/4-voice router/LiveReharmonizer/Voice Leading и старый Instrument UI.

## Что реализуется в 0.0b

- единый `AnalysisEvidence` / `ConfidenceLevel` contract;
- `KeyCenter` с scope `global/local/temporary/modal`;
- `HarmonicPattern` + роль/позиция внутри паттерна;
- `ResolutionTarget` + структурные tendency/resolution moves;
- `TensionLevel` 1/2/3;
- базовый `ImprovisationStrategy` contract;
- минимальный семантический `Phrase` contract;
- центральный `HarmonicSituation`;
- host-neutral `TimelineHarmonicSnapshot → HarmonicSituation` builder;
- regression tests новой data model.

Pattern recognition и local-key inference намеренно **не выполняются внутри data-model layer**. Модель только хранит их будущий результат; сами analyzers относятся к Stage 2.

## Что проверено

Для завершённого `0.0a`:
- Windows CMake Configure — **OK**;
- Windows Build — **OK**;
- Host-neutral Core tests — **OK**;
- сборка headless ARA helper — **OK**.

Для `0.0b` CI выполняется в PR #13. До зелёного Build/Test подэтап считается **в работе**.

## Что ещё НЕ проверено

- Живой тест `Smart Improviser ARA.vst3` в Fender Studio Pro.
- ARA binding в реальном проекте Studio Pro.
- Получение Key Track / Chord Track в живом проекте.
- Transport STOP / PLAY / seek в живом проекте.
- Обновление контекста после редактирования Chord/Key Track.

Это относится главным образом к Stage 1 и не блокирует завершение спецификации Core Stage 0.

## Что останется после 0.0b

После принятия data model нужно закрыть оставшиеся пункты Issue #1 и подготовить Stage 0 к стабильной версии `0.1`. Если потребуется ещё один самостоятельный подэтап спецификации, он получит версию `0.0c`.

## Правило версий

```text
0.0a → 0.0b → 0.0c → ... → 0.1
```

Если живой тест выявил ошибку текущей буквенной версии:

```text
0.0b → 0.0b fix1 → 0.0b fix2 → 0.0c
```

Подробнее: `docs/VERSIONING.md`.

## Что читать в новом чате

1. `docs/CURRENT_STATE.md`;
2. текущий Stage Issue;
3. активный PR;
4. `docs/PROJECT_CONTEXT.md`;
5. `docs/ARCHITECTURAL_DECISIONS.md`;
6. `docs/ROADMAP.md`;
7. `docs/VERSIONING.md`;
8. `docs/MIGRATION_FROM_SMART_VOICING.md` при необходимости.

## Правило обновления этого файла

`CURRENT_STATE.md` обновляется при переходе на следующую буквенную версию, после существенного `fixN`, важного live-test, смены активного PR/ветки, завершения Stage и перед переходом разработки в новый чат.
