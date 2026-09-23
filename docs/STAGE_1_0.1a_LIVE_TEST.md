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

## Результаты fix1

### Harmonic context

- [x] Key отображается музыкальным именем (`C major`), а не `(unnamed)`;
- [x] Previous / Current / Next chord отображаются как реальные символы;
- [x] current chord меняется на реальной event boundary;
- [x] previous/current/next сдвигаются без задержки;
- [x] участок без активного chord event обрабатывается безопасно: `Current=(no chord)` при сохранении корректных Previous/Next.

Подтверждённые кейсы:

```text
PPQ 172.000
Previous chord  C
Current chord   (no chord)
Next chord      Cmaj7
```

```text
PPQ 180.000
Previous chord  Cmaj7
Current chord   Am7
Next chord      Dm7
```

```text
Previous chord  Am7
Current chord   Dm7
Next chord      G13
```

### Timeline / transport

- [x] STOP position остаётся корректным;
- [x] PLAY обновляет PPQ/context во время воспроизведения;
- [x] seek назад/вперёд немедленно обновляет контекст;
- [x] tempo/time signature продолжают совпадать с DAW.

### UI

- [x] строка `Revisions` полностью видна;
- [x] footer не перекрывает данные;
- [x] заголовок `Stage 1 - ARA Context Monitor` отображается без битой кодировки.

### Изменения проекта

- [x] после редактирования Chord Track context обновляется;
- [x] после редактирования Key Track context обновляется;
- [x] после повторного открытия проекта ARA binding/context восстанавливаются.

## Статус 0.1a

`0.1a fix1` принят как успешный checkpoint. Основной live-path Stage 1 подтверждён.

Для следующего подэтапа остаются отдельные edge cases:

- [ ] поведение при полном отсутствии Key/Chord/Tempo данных;
- [ ] несколько Musical Context, если удастся воспроизвести такой сценарий в Studio Pro;
- [ ] формальный контракт данных Stage 1 → Stage 2.

## Правило fix-версий

Если следующий подэтап выявит ошибку именно принятого поведения 0.1a, можно вернуться к `0.1a fix2`; иначе дальнейшая разработка продолжается как `0.1b`.
