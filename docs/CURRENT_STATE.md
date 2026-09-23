# Smart Improviser — Current State

> **Назначение:** короткая точка входа для нового чата или новой рабочей сессии.  
> Этот файл должен отвечать на вопрос: **где проект находится сейчас и что делать следующим?**  
> Подробная архитектура хранится в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Текущий Stage:** Stage 0 — Спецификация Smart Improviser Core
- **Текущая рабочая версия:** `0.0a`
- **Итоговая версия Stage 0:** `0.1`
- **Последняя стабильная версия:** пока нет — проект находится до первого стабильного milestone `0.1`
- **Активный Issue:** #1 — Stage 0 — Спецификация Smart Improviser Core
- **Активный PR:** #12 — `0.0a — Stage 0: перенос чистого Harmony Core и ARA context layer`
- **Активная ветка:** `stage-0-core-migration`

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
Сознательно **не перенесены**:
- `CloseVoicingHarmonizer`;
- `LiveReharmonizer`;
- 4-voice router / voice ownership;
- Distribution Mode;
- Voicing strategies;
- Voice Leading;
- sustain/retrigger infrastructure;
- старый Instrument Plugin Processor / Editor;
- старый Voicing UI.

## Что проверено

- Windows CMake Configure — **OK**.
- Windows Build — **OK**.
- Host-neutral Core tests — **OK**.
- Сборка headless ARA helper — **OK**.
- Поиск остаточных Smart Voicing / Voicing dependencies в перенесённой архитектуре — выполнен.

## Что ещё НЕ проверено

- Живой тест `Smart Improviser ARA.vst3` в Fender Studio Pro.
- ARA binding в реальном проекте Studio Pro.
- Получение Key Track / Chord Track в живом проекте.
- Transport STOP / PLAY / seek в живом проекте.
- Обновление контекста после редактирования Chord/Key Track.

Это относится главным образом к Stage 1 и не блокирует завершение спецификации Core Stage 0.

## Что осталось в Stage 0

Главная незавершённая часть — собственная data model Smart Improviser Core v0.1.

Нужно формально определить и/или реализовать:

- `KeyCenter` — global / local / temporary / modal center;
- `HarmonicPattern`;
- `HarmonicSituation` — центральную сущность проекта;
- `ResolutionTarget`;
- `TensionLevel`;
- `ImprovisationStrategy`;
- `Phrase`;
- API `Timeline Context → HarmonicSituation`;
- confidence / ambiguous interpretation contract;
- regression tests для новой data model.

Полный чек-лист находится в Issue #1.

## Следующий подэтап

После принятия `0.0a` следующий буквенный подэтап Stage 0:

**`0.0b` — собственная data model Smart Improviser Core**

Ожидаемый фокус:

1. `HarmonicSituation`;
2. `HarmonicPattern`;
3. `ResolutionTarget`;
4. `KeyCenter`;
5. contracts для `TensionLevel`, `ImprovisationStrategy`, `Phrase`;
6. tests новой модели.

После завершения всех буквенных подэтапов Stage 0 получается стабильная версия **`0.1`**.

## Правило версий

Внутри Stage используется буквенная рабочая линия:

```text
0.0a → 0.0b → 0.0c → ... → 0.1
```

Если живой тест выявил ошибку текущей буквенной версии:

```text
0.0b → 0.0b fix1 → 0.0b fix2 → 0.0c
```

Подробнее: `docs/VERSIONING.md`.

## Что читать в новом чате

Рекомендуемый порядок:

1. `docs/CURRENT_STATE.md` — где проект находится сейчас;
2. текущий Stage Issue — фактический чек-лист прогресса;
3. активный PR — что именно реализуется сейчас;
4. `docs/PROJECT_CONTEXT.md` — архитектура и продуктовая логика;
5. `docs/ARCHITECTURAL_DECISIONS.md` — принятые архитектурные решения;
6. `docs/ROADMAP.md` — следующие Stage;
7. `docs/VERSIONING.md` — правила именования версий;
8. `docs/MIGRATION_FROM_SMART_VOICING.md` — происхождение перенесённого фундамента, если это имеет отношение к задаче.

## Шаблон старта нового чата

> Продолжаем разработку Smart Improviser. Репозиторий: `https://github.com/vladleng/Smart-Improviser`. Сначала прочитай `docs/CURRENT_STATE.md`, текущий Stage Issue, активный PR и архитектурную документацию. Проверь актуальное состояние GitHub, после чего продолжай работу с текущего незавершённого пункта чек-листа.

## Правило обновления этого файла

`CURRENT_STATE.md` должен обновляться:

- при переходе на следующую буквенную версию;
- после `fixN`, если исправление меняет фактическое состояние проекта;
- после важного live test;
- при смене активного PR/ветки;
- при завершении Stage;
- перед переходом разработки в новый чат.

Файл должен оставаться коротким оперативным summary и **не дублировать полностью** Project Context, Roadmap или Issues.
