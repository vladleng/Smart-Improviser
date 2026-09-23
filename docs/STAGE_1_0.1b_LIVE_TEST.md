# Smart Improviser 0.1b — Stage 1 Edge-Case Live Test

## Цель

`0.1b` проверяет устойчивость уже работающего ARA Context Provider на неполных данных и закрепляет контракт Stage 1 → Stage 2.

Сборка:

```text
Smart-Improviser-0.1b-Windows
└── Smart Improviser.vst3
```

## Что изменено

- mapping ARA/shared snapshot → Core вынесен в отдельный JUCE-free `TimelineContextMapper`;
- diagnostics UI использует тот же mapper, что и `ARAContextProvider`;
- provider остаётся connected при валидном ARA/Musical Context даже если отдельные Key/Chord/Tempo источники отсутствуют;
- пустой chord/key event различается с отсутствующим источником;
- до первого chord event текущий аккорд отсутствует, но `Next chord` показывает первый будущий event;
- после последнего chord event последний event остаётся current до следующего события/no-chord marker;
- при нескольких Musical Context UI показывает выбранный ordinal;
- выбор Musical Context детерминирован: максимум доступных типов → максимум events → первый при равенстве.

## Чек-лист live test

### Базовая регрессия

- [ ] `ARA binding = BOUND`;
- [ ] существующий обычный проект продолжает показывать правильные Key/Previous/Current/Next;
- [ ] PLAY / STOP / seek работают как в принятом `0.1a fix1`;
- [ ] Tempo / Time Signature совпадают с DAW.

### Нет Chord Track / нет chord events

Создать копию проекта без chord events либо очистить Chord Track.

Ожидается:

```text
ARA binding      BOUND
Shared context   YES
Current chord    -
Previous chord   -
Next chord       -
```

Key/Tempo при наличии должны продолжить работать.

### Нет Key Track / нет key events

Удалить/очистить key events, не меняя Chord Track.

Ожидается:

```text
Shared context   YES
Key              -
```

Chord context должен продолжать работать.

### Нет/неполные Tempo data

Если Studio Pro позволяет получить Musical Context без usable tempo entries, плагин не должен падать или создавать фиктивные значения. В диагностике Tempo допустимо `-`.

### До первого chord event

Поставить курсор до первого события Chord Track.

Ожидается:

```text
Previous chord   -
Current chord    -
Next chord       <первый chord event>
```

### После последнего chord event

Поставить курсор после последнего chord event, если последний event не является no-chord marker.

Ожидается, что последний chord event остаётся Current, а Next отсутствует.

### Явный no-chord event

Если Chord Track содержит участок `No Chord`, ожидается:

```text
Current chord    (no chord)
```

При этом соседние `Previous` / `Next` сохраняются, если существуют.

### Несколько Musical Context

Если удаётся воспроизвести проект с несколькими Musical Context:

- [ ] `Musical contexts` показывает значение > 1;
- [ ] UI показывает `selected N`;
- [ ] Key/Chord/Tempo/Bar events принадлежат одному выбранному context и не смешиваются;
- [ ] при повторном открытии выбор остаётся детерминированным для одинакового состояния проекта.

## Автоматические regression tests

CI дополнительно запускает:

```text
SmartImproviserTimelineContextTests
```

Они проверяют без DAW:

- connected provider без Key/Chord/Tempo sources;
- позицию до первого chord event;
- exact event boundary;
- explicit no-chord event;
- позицию после последнего chord event;
- undefined key event;
- отсутствие tempo data;
- PPQ ↔ seconds conversion;
- single tempo anchor без выдумывания неизвестного tempo slope.

## Критерий приёмки 0.1b

`0.1b` принимается, если:

1. CI зелёный, включая `SmartImproviserTimelineContextTests`;
2. обычные live-сценарии `0.1a fix1` не регрессировали;
3. доступные missing-data edge cases в Studio Pro ведут себя безопасно;
4. контракт `docs/STAGE_1_TO_STAGE_2_CONTRACT.md` соответствует фактической реализации;
5. если multiple Musical Context удаётся воспроизвести — выбранный context понятен и стабилен.
