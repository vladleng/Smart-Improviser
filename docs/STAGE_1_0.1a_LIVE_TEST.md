# Smart Improviser 0.1a — Stage 1 Live Test

## Цель

Первый живой тест Stage 1 в Fender Studio Pro. Версия `0.1a` проверяет, что перенесённая ARA-инфраструктура действительно работает в целевой DAW и публикует корректный timeline context.

## Сборка

GitHub Actions artifact:

```text
Smart-Improviser-0.1a-Windows
└── Smart Improviser.vst3
```

Плагин остаётся headless ARA helper. Наличие отдельного полноценного Smart Improviser UI на этом этапе не требуется.

## Важное правило ARA-подключения

В Fender Studio Pro Smart Improviser нужно подключать как **Event FX / ARA extension**.

Audio Event в данном случае служит **только точкой подключения ARA**. По результатам предыдущих live-тестов Smart Voicing длина этого Audio Event не ограничивает диапазон Musical Context, доступного через ARA. Поэтому не требуется растягивать Event на всю композицию: достаточно любого подходящего Audio Event, на который можно установить Smart Improviser как Event FX.

То есть модель использования такая:

```text
Audio Event любой длины
        ↓
Event FX: Smart Improviser
        ↓
ARA 2 binding
        ↓
Musical Context всего проекта
```

## Подготовка тестового проекта

Рекомендуемый минимальный кейс:

```text
Key: C major
Chords: Dm7 | G7 | Cmaj7
Tempo: 120 BPM
Time Signature: 4/4
```

Дополнительно полезно создать смену тональности и хотя бы одну смену tempo/time signature, чтобы проверить обновление Musical Context.

## Чек-лист 0.1a

### ARA binding

- [ ] `Smart Improviser.vst3` виден Fender Studio Pro.
- [ ] Плагин успешно создаётся как ARA/Event-FX helper.
- [ ] ARA document controller получает Musical Context.
- [ ] Короткий Audio Event подтверждает, что его длина не ограничивает доступный Musical Context.

### Harmonic context

- [ ] Key Track доступен.
- [ ] Chord Track / Sheet Chords доступны.
- [ ] В `Dm7 | G7 | Cmaj7` текущий аккорд переключается точно на event boundary.
- [ ] Для позиции на `G7` доступны previous=`Dm7`, current=`G7`, next=`Cmaj7`.
- [ ] При переходе на `Cmaj7` previous/current/next обновляются без задержки на один аккорд.

### Timeline / transport

- [ ] STOP: позиция курсора читается корректно.
- [ ] PLAY: PPQ обновляется во время воспроизведения.
- [ ] Seek назад/вперёд немедленно обновляет активный контекст.
- [ ] Tempo map доступна для PPQ ↔ seconds conversion.
- [ ] Time Signature читается из Bar Signatures.

### Изменения проекта

- [ ] После редактирования Chord Track context обновляется.
- [ ] После редактирования Key Track context обновляется.
- [ ] После повторного открытия проекта ARA binding и context восстанавливаются.

### Несколько Musical Context

- [ ] Зафиксировать `musicalContextCount` в реальном проекте.
- [ ] Если контекстов больше одного, проверить, какой набор событий публикуется и нет ли смешивания разных timeline context.

## Что считать ошибкой 0.1a

Для найденных ошибок сохраняем текущую букву и используем:

```text
0.1a fix1
0.1a fix2
...
```

Новая версия `0.1b` начинается только после того, как задачи, определённые для 0.1a, приняты.

## Что прислать после теста

Достаточно короткого отчёта по пунктам:

```text
ARA binding: OK / FAIL
Key Track: OK / FAIL
Chord Track: OK / FAIL
Tempo/Time Signature: OK / FAIL
STOP/PLAY: OK / FAIL
Seek: OK / FAIL
Chord boundaries: OK / FAIL
Edit refresh: OK / FAIL
Reopen project: OK / FAIL
Multiple Musical Context: result
Short Event as ARA anchor: OK / FAIL
```

При `FAIL` желательно указать точный сценарий и что наблюдается в Studio Pro.
