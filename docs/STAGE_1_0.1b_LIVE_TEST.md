# Smart Improviser 0.1b — Stage 1 Edge-Case Live Test

## Статус

**ACCEPTED.** Подэтап `0.1b` принят пользователем после живого теста в Fender Studio Pro и используется как основа финальной стабильной версии Stage 1 — `0.2`.

## Цель

`0.1b` проверяет устойчивость уже работающего ARA Context Provider на неполных данных и закрепляет контракт Stage 1 → Stage 2.

Сборка checkpoint:

```text
Smart-Improviser-0.1b-Windows
└── Smart Improviser.vst3
```

Финальная сборка Stage 1:

```text
Smart-Improviser-0.2-Windows
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

## Результаты live test

### Базовая регрессия

- [x] `ARA binding = BOUND`;
- [x] обычный проект продолжает показывать правильные Key/Previous/Current/Next;
- [x] PLAY / STOP / seek работают как в принятом `0.1a fix1`;
- [x] Tempo / Time Signature совпадают с DAW.

### Нет Chord Track / нет активного harmonic context

Подтверждено безопасное состояние без активного chord context:

```text
ARA binding      BOUND
Shared context   YES
Previous chord   -
Current chord    (no chord) / unavailable по семантике host event
Next chord       -
```

ARA connection и transport при этом сохраняются.

### Нет Key Track / нет key events

Подтверждено:

```text
Key              -
```

При этом ARA binding, Tempo/Time Signature и chord context продолжают работать независимо.

### До первого chord event

Подтверждено поведение с будущим первым аккордом:

```text
Previous chord   -
Current chord    (no chord)
Next chord       C
```

### После последнего chord event

Проверено безопасное поведение после последнего harmonic event; контекст не приводит к disconnect/crash и остаётся детерминированным согласно mapper policy.

### Explicit no-chord

Подтверждено ранее и не регрессировало:

```text
Current chord    (no chord)
```

Соседние Previous/Next сохраняются, если существуют.

### Tempo / Time Signature

В live test сохранены корректные значения `120 BPM` и `4/4`; отсутствие Key/Chord content не нарушает tempo/bar context.

### Multiple Musical Context

Отдельный воспроизводимый Studio Pro сценарий с несколькими Musical Context не является блокером релиза `0.2`. Реализована детерминированная policy выбора:

1. максимум доступных типов Key / Chords / Tempo / Bars;
2. максимум общего количества events;
3. первый context в host order при равенстве.

Diagnostic UI показывает:

```text
Musical contexts N | selected M
```

## Автоматические regression tests

CI запускает:

```text
SmartImproviserTimelineContextTests
```

Тесты проверяют без DAW:

- connected provider без Key/Chord/Tempo sources;
- unknown position safety;
- позицию до первого chord event;
- exact event boundary;
- explicit no-chord event;
- позицию после последнего chord event;
- undefined key event;
- отсутствие tempo data;
- PPQ ↔ seconds conversion;
- single tempo anchor без выдумывания неизвестного tempo slope.

## Итог

`0.1b` принят. Stage 1 считается функционально завершённым и финализируется стабильной версией:

```text
Smart Improviser 0.2
```

Следующий этап: Stage 2 / `0.2a` — Harmonic Engine.
