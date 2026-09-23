# Smart Improviser 0.1a — Stage 1 Live Test

## Статус

`0.1a fix1` — **ACCEPTED**.

Этот checkpoint подтвердил основной реальный ARA/timeline path в Fender Studio Pro. Дальнейшие edge-case проверки перенесены в `docs/STAGE_1_0.1b_LIVE_TEST.md`.

## Подтверждено

- [x] `Smart Improviser.vst3` виден DAW и открывается как Event FX;
- [x] ARA binding: `BOUND`;
- [x] Document Controller создан;
- [x] Host Content Access доступен;
- [x] Musical Context доступен;
- [x] Shared Context доступен;
- [x] STOP position и PPQ корректны;
- [x] PLAY обновляет PPQ/context;
- [x] seek обновляет context немедленно;
- [x] Key Track читается и нормализуется;
- [x] Chord Track читается и нормализуется;
- [x] Previous / Current / Next совпадают с Chord Track;
- [x] exact chord boundaries работают;
- [x] Tempo / Time Signature совпадают с DAW;
- [x] Chord/Key Track refresh работает без перезагрузки плагина;
- [x] reopen проекта восстанавливает binding/context;
- [x] короткий Audio Event достаточен как ARA anchor и не ограничивает Musical Context;
- [x] explicit no-chord event отображается безопасно;
- [x] диагностический UI fix1 отображает музыкальные названия Key/Chord;
- [x] `Revisions` и footer не перекрываются;
- [x] mojibake заголовка устранён.

## Подтвержденные примеры

```text
PPQ 172.000
Key             C major
Previous chord  C
Current chord   (no chord)
Next chord      Cmaj7
Time signature  4/4
Tempo           110.00 BPM
```

```text
PPQ 180.000
Key             C major
Previous chord  Cmaj7
Current chord   Am7
Next chord      Dm7
Time signature  4/4
Tempo           110.00 BPM
```

```text
Previous chord  Am7
Current chord   Dm7
Next chord      G13
```

`G13` — нормализованное представление `G7add13` из Chord Track.

## ARA anchor rule

В Fender Studio Pro Smart Improviser подключается как **Event FX / ARA extension**.

Audio Event служит только точкой ARA binding. Его длина не ограничивает доступ к Musical Context проекта.

```text
Audio Event любой длины
        ↓
Event FX: Smart Improviser
        ↓
ARA 2 binding
        ↓
Musical Context проекта
```

## Следующий checkpoint

Edge cases отсутствующих источников данных, before/after chord range и multiple Musical Context относятся к `0.1b`:

```text
docs/STAGE_1_0.1b_LIVE_TEST.md
```
