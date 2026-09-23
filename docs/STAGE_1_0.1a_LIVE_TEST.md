# Smart Improviser 0.1a — Stage 1 Live Test

## Цель

Первый живой тест Stage 1 в Fender Studio Pro. Версия `0.1a` проверяет, что перенесённая ARA-инфраструктура действительно работает в целевой DAW и публикует корректный timeline context.

## Сборки

Первый live-test checkpoint:

```text
Smart-Improviser-0.1a-Windows
└── Smart Improviser.vst3
```

Исправление диагностического слоя после первого теста:

```text
Smart-Improviser-0.1a-fix1-Windows
└── Smart Improviser.vst3
```

Во время Stage 1 плагин имеет временный **ARA Context Monitor**. Это диагностический UI, а не будущий основной интерфейс Smart Improviser.

## Важное правило ARA-подключения

В Fender Studio Pro Smart Improviser нужно подключать как **Event FX / ARA extension**.

Audio Event служит **только точкой подключения ARA**. Первый live-тест Smart Improviser подтвердил поведение, ранее обнаруженное в Smart Voicing: короткий Audio Event не ограничивает диапазон Musical Context. При коротком Event в начале проекта Smart Improviser корректно считал transport/context при курсоре далеко за его пределами.

```text
Audio Event любой длины
        ↓
Event FX: Smart Improviser
        ↓
ARA 2 binding
        ↓
Musical Context проекта
```

## Результат первого live-теста 0.1a

На реальном проекте Fender Studio Pro подтверждено:

- [x] `Smart Improviser.vst3` виден DAW и открывается как Event FX;
- [x] ARA binding: `BOUND`;
- [x] Document Controller создан;
- [x] Host Content Access доступен;
- [x] обнаружен один Musical Context;
- [x] Shared Context доступен;
- [x] STOP position читается корректно;
- [x] PPQ совпал с позицией DAW: начало такта 30 = `116.000 PPQ`;
- [x] Time Signature совпала с DAW: `4/4`;
- [x] Tempo совпал с DAW: `110.00 BPM`;
- [x] ARA вернул реальные event counts: Key=2, Chords=13, Tempo=16, Bars=1;
- [x] короткий Audio Event подтверждён как достаточная точка ARA binding.

Обнаруженные проблемы диагностического UI:

- [x] ARA Chord/Key events приходят структурно, но Studio Pro не заполняет их `name`, поэтому 0.1a показывал `(unnamed)`;
- [x] строка `Revisions` перекрывалась footer-текстом;
- [x] длинное тире в заголовке отображалось с ошибкой кодировки.

## 0.1a fix1

Fix1 не меняет уже работающий ARA transport/context path. Исправляется только диагностический слой:

- Chord names строятся из `root + bass + intervals` через существующий `ChordModel` / `normalizedChordSymbol()`;
- Key names строятся из `root + intervals` через существующий `KeyModel`;
- ARA `name` используется только как fallback;
- окно увеличено по высоте, чтобы `Revisions` не перекрывался footer;
- диагностические строки переведены на ASCII punctuation для устранения mojibake;
- build label: `0.1a fix1`;
- artifact: `Smart-Improviser-0.1a-fix1-Windows`.

## Что проверить в fix1

### Harmonic context

- [ ] Key отображается музыкальным именем, например `C major`, а не `(unnamed)`;
- [ ] Previous / Current / Next chord отображаются как реальные символы;
- [ ] при перемещении курсора по Chord Track current chord меняется на реальной event boundary;
- [ ] на `G7` в `Dm7 | G7 | Cmaj7` отображается previous=`Dm7`, current=`G7`, next=`Cmaj7`;
- [ ] на следующем аккорде previous/current/next сдвигаются без задержки.

### Timeline / transport

- [ ] STOP position остаётся корректным;
- [ ] PLAY обновляет PPQ во время воспроизведения;
- [ ] seek назад/вперёд немедленно обновляет контекст;
- [ ] tempo/time signature продолжают совпадать с DAW.

### UI

- [ ] строка `Revisions` полностью видна;
- [ ] footer не перекрывает данные;
- [ ] заголовок `Stage 1 - ARA Context Monitor` отображается без битой кодировки.

### Изменения проекта

- [ ] после редактирования Chord Track context обновляется;
- [ ] после редактирования Key Track context обновляется;
- [ ] после повторного открытия проекта ARA binding/context восстанавливаются.

### Несколько Musical Context

- [ ] если в проекте появится больше одного Musical Context, проверить выбор и отсутствие смешивания событий.

## Правило fix-версий

Если `0.1a fix1` выявит следующую ошибку текущего checkpoint, используется `0.1a fix2`. Новая буква `0.1b` начинается только после принятия задач 0.1a.
