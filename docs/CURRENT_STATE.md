# Smart Improviser — Current State

> **Назначение:** короткая точка входа для нового чата или новой рабочей сессии.  
> Этот файл должен отвечать на вопрос: **где проект находится сейчас и что делать следующим?**  
> Подробная архитектура хранится в `PROJECT_CONTEXT.md`, этапы — в `ROADMAP.md`, правила версий — в `VERSIONING.md`, фактический прогресс — в GitHub Issues.

## Текущее состояние

- **Завершённый Stage:** Stage 0 — Спецификация Smart Improviser Core
- **Текущая стабильная версия:** `0.1`
- **Активный Stage:** Stage 1 — ARA Context Monitor
- **Принятый checkpoint Stage 1:** `0.1a fix1`
- **Следующий подэтап:** `0.1b`
- **Активный Issue:** #2 — Stage 1 — ARA Context Monitor
- **Активная ветка:** `stage-1-ara-context-monitor`
- **Активный PR:** #14

`0.1a fix1` принят после живых тестов в Fender Studio Pro. Основной ARA/timeline path подтверждён на реальном проекте, включая PLAY, seek, chord boundaries, edit refresh и reopen.

## Архитектурная граница

```text
DAW / ARA
    ↓
Timeline Context
    ↓
ARAContextProvider
    ↓
TimelineHarmonicSnapshot
    ↓
HarmonicSituation
    ↓
будущие Harmonic / Tension / Improvisation analyzers
```

Core не зависит от Fender Studio Pro, ARA, JUCE, VST3, UI или старого Voicing engine.

## Что подтверждено в 0.1a / 0.1a fix1

На реальном проекте Fender Studio Pro подтверждено:

- ARA binding — **BOUND**;
- Document Controller — **YES**;
- Host Content Access — **YES**;
- Musical Context — доступен;
- Shared Context — **YES**;
- STOP position — корректна;
- PLAY — PPQ/context обновляются во время воспроизведения;
- seek — контекст обновляется немедленно;
- Key Track — читается и нормализуется;
- Chord Track — читается и нормализуется;
- previous/current/next — совпадают с реальным Chord Track;
- точное переключение на chord boundary — подтверждено;
- Tempo / Time Signature — совпадают с DAW;
- изменение Chord Track / Key Track обновляет context snapshot;
- после повторного открытия проекта ARA binding/context восстанавливаются;
- короткий Audio Event не ограничивает Musical Context и служит только точкой ARA binding;
- участок без активного chord event обрабатывается безопасно.

Подтверждённые edge cases:

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

Также подтверждён рабочий кейс:

```text
Previous chord  Am7
Current chord   Dm7
Next chord      G13
Key             C major
Tempo           110 BPM
Time signature  4/4
```

`G13` — нормализованное представление `G7add13` из Chord Track.

## Что сделал fix1

- Chord symbols строятся из структурных ARA-данных через `ChordModel` / `normalizedChordSymbol()`;
- Key display строится через `KeyModel` из `root + intervals`;
- ARA `name` используется только как fallback;
- исправлены размеры диагностического UI;
- `Revisions` и footer больше не перекрываются;
- устранён mojibake в заголовке;
- build label — `0.1a fix1`;
- CI artifact — `Smart-Improviser-0.1a-fix1-Windows`;
- внутри artifact — готовая папка `Smart Improviser.vst3`.

## Статус checkpoint 0.1a

`0.1a fix1` **принят**. Основной путь Stage 1 работает:

```text
Studio Pro
→ Event FX / ARA binding
→ Musical Context
→ SharedHarmonicContext
→ ARAContextProvider
→ TimelineHarmonicSnapshot
→ previous/current/next + key + timeline
```

Новых исправлений `0.1a fixN` сейчас не требуется.

## Следующий подэтап — 0.1b

`0.1b` должен закрыть устойчивость и формальный выход Stage 1 к Stage 2:

1. безопасное поведение при полном отсутствии Key Track;
2. безопасное поведение при полном отсутствии Chord Track;
3. безопасное поведение при отсутствующих/неполных Tempo данных;
4. начало timeline до первого chord event и конец timeline после последнего;
5. несколько Musical Context, если Studio Pro позволяет воспроизвести такой сценарий;
6. формально зафиксировать минимальный контракт данных Stage 1 → Stage 2;
7. добавить regression tests для edge cases, которые можно проверить без DAW.

После успешной приёмки оставшихся задач Stage 1 рабочая линия должна завершиться стабильной версией `0.2`.

## Рабочая линия Stage 1

```text
0.1a
→ 0.1a fix1  [ACCEPTED]
→ 0.1b
→ при необходимости следующие 0.1x
→ 0.2
```

## Ближайший следующий шаг

Начать `0.1b`: сначала формализовать expected behavior для missing-data и timeline edge cases, затем реализовать/проверить их и закрепить контракт Stage 1 → Stage 2.

## Что читать в новом чате Stage 1

1. `docs/CURRENT_STATE.md`;
2. Issue #2 — Stage 1 — ARA Context Monitor;
3. `docs/STAGE_1_0.1a_LIVE_TEST.md`;
4. `docs/CORE_DATA_MODEL_0.0b.md`;
5. `docs/PROJECT_CONTEXT.md`;
6. `docs/ARCHITECTURAL_DECISIONS.md`;
7. `docs/ROADMAP.md`;
8. `docs/VERSIONING.md`;
9. `docs/MIGRATION_FROM_SMART_VOICING.md` при необходимости.

## Правило обновления этого файла

`CURRENT_STATE.md` обновляется при переходе на следующую буквенную версию, после существенного `fixN`, важного live-test, смены активного PR/ветки, завершения Stage и перед переходом разработки в новый чат.
