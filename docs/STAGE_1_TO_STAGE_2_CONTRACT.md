# Stage 1 → Stage 2 Context Contract

## Назначение

Этот документ фиксирует границу между ARA/DAW-инфраструктурой Stage 1 и Harmonic Engine Stage 2.

Stage 2 не должен знать ничего о Fender Studio Pro, ARA SDK, JUCE, shared memory или конкретном формате ARA events. Его вход — только host-neutral структуры Smart Improviser Core.

```text
DAW / ARA
    ↓
SharedHarmonicContextSnapshot
    ↓
TimelineContextMapper
    ↓
HarmonicContext / TimelineHarmonicSnapshot
    ↓
Stage 2 Harmonic Engine
```

## Основной анализируемый snapshot

Для гармонического анализа Stage 2 использует:

```cpp
TimelineHarmonicSnapshot
```

Семантика полей:

- `positionAvailable=false` — позиция timeline неизвестна; анализ выполнять нельзя;
- `ppq` — позиция snapshot в quarter-note coordinates;
- `currentChord.available=false` — источник Chord Track отсутствует либо до первого chord event ещё нет активного события;
- `currentChord.available=true && currentChord.defined=false` — хост явно передал no-chord / пустое harmonic event;
- `currentChord.available=true && currentChord.defined=true` — есть структурно определённый текущий аккорд;
- `previousChordAvailable` / `nextChordAvailable` относятся к соседним chord events в timeline и не означают, что эти аккорды обязательно определены;
- перед первым chord event `currentChord` отсутствует, но `nextChord` может содержать первый будущий аккорд;
- после последнего chord event последний event остаётся текущим, пока хост не передаст следующий event или явный no-chord event;
- `globalKey.available=false` — Key Track/key content недоступен либо ещё нет активного key event;
- `globalKey.available=true && globalKey.defined=false` — получено явное неопределённое key event;
- `globalKey.available=true && globalKey.defined=true` — есть структурно определённая глобальная тональность.

## Operational context

Для диагностики подключения и transport/UI используется:

```cpp
HarmonicContext
```

Ключевая семантика:

- `providerConnected=true` означает, что ARA helper привязан к usable Musical Context;
- наличие конкретных Key/Chord/Tempo/Bar данных не является условием `providerConnected`;
- отсутствие отдельного источника отражается его `available=false`;
- `positionAvailable` отделено от connection state;
- `harmonicRevision` меняется при обновлении harmonic map;
- `transportRevision` меняется независимо при transport updates.

Это позволяет отличить два принципиально разных состояния:

```text
1. Provider disconnected
2. Provider connected, но в проекте нет Chord Track / Key Track / Tempo data
```

## Tempo conversion

`secondsAtPpq()` и `ppqAtSeconds()`:

- возвращают `-1.0`, если Tempo content отсутствует;
- при двух и более tempo anchors выполняют интерполяцию/экстраполяцию по доступному сегменту;
- при единственном tempo anchor возвращают значение только в точке самого anchor и не придумывают неизвестный tempo slope.

## Musical Context selection

Если Fender Studio Pro предоставляет несколько `Musical Context`, Stage 1 выбирает один context детерминированно:

1. context с наибольшим числом доступных типов из `Key / Chords / Tempo / Bars`;
2. при равенстве — context с большим общим количеством events;
3. при полном равенстве — первый context в порядке, предоставленном host.

Диагностический UI показывает количество Musical Context и выбранный ordinal (`selected N`).

Stage 2 не участвует в выборе Musical Context и получает уже единый нормализованный timeline snapshot.

## Что считается валидным входом Stage 2

Базовый `HarmonicSituation` может быть построен только тогда, когда присутствуют необходимые музыкальные данные. На текущем этапе это означает как минимум:

- известная timeline position;
- определённый current chord;
- определённый global key.

Отсутствие этих данных — нормальное состояние входа, а не ошибка или повод создавать фиктивные значения.

## Архитектурное правило

Stage 2 и более поздние музыкальные engines не должны включать или использовать:

- ARA SDK types;
- JUCE types;
- `SharedHarmonicContextBridge`;
- `SharedHarmonicContextSnapshot`;
- Fender Studio Pro specific behavior.

Все host-specific детали заканчиваются на `TimelineContextMapper` / `ARAContextProvider`.
