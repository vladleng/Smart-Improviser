# Stage 3 / 0.3f — Context-aware ranking / ambiguity [ACCEPTED]

База: принятые `0.3e` и `0.3e-ui`. Stable остаётся `0.3`.

## Статус

**Принято Владом 2026-09-25.** Windows Build #284 — success. Live-test в Studio Pro пройден. Issue #29 закрыт как реализованный в `0.3f`.

## Цель

`0.3f` не добавляет новый большой каталог гамм. Подэтап делает уже накопленные стратегии контекстно корректными при нескольких гармонических трактовках и устраняет обнаруженный пробел Pattern Recognizer для классического минорного `iv–V–i`.

Основное правило: **если Stage 2 не выбрал единственную трактовку, Stage 3 не выбирает её скрыто**. Музыкально разные варианты сохраняются раздельно и привязаны к конкретному `global / local / modal` interpretation.

## 1. Context-aware strategies

- foundation по явным звукам аккорда остаётся `interpretationIndependent` и доступен всегда;
- scalar / special-source стратегии строятся для валидных harmonic interpretations, а не только для `primaryInterpretationIndex`;
- каждая context-dependent strategy сохраняет свой `interpretationIndex` и evidence;
- если `primaryInterpretationIndex >= 0`, стратегии основной трактовки выводятся раньше альтернатив;
- если `primaryInterpretationIndex == -1`, ни global, ни local, ни modal вариант не получает скрытого статуса primary;
- разные interpretations не смешиваются в одну strategy или concept;
- `priority` остаётся recommendation/catalog priority и не подменяет harmonic confidence.

## 2. Deterministic ranking

Порядок воспроизводим:

1. interpretation-independent foundation;
2. при наличии primary — стратегии primary interpretation;
3. альтернативные interpretations;
4. внутри одного interpretation — существующий `priority`, затем стабильный `ruleId`.

При unresolved primary interpretations выводятся в стабильном порядке Stage 2, но сам порядок **не означает**, что первая трактовка музыкально предпочтительнее.

## 3. Minor iv–V–i — Issue #29 закрыт

Добавлен отдельный harmonic pattern:

```text
Fm7 → G7 → Cm7
iv7 → V7 → i7
```

Принятое поведение в `C minor`:

```text
Fm7:  Минорный iv–V–i • 1 / 3
G7:   Минорный iv–V–i • 2 / 3   (когда видны Fm7 и Cm7)
Cm7:  общий V–i / resolution, если более ранний predominant уже недоступен
```

На финальном `i` Stage 1 contract содержит только `previous / current / next`, поэтому движок не угадывает, был predominant `iiø` или `iv`. То же правило применяется к major `V→I`.

Отдельно подтверждён false-positive guard:

```text
C major: Fm7 → G7 → Cmaj7
```

На `Fm7` допустима unresolved borrowed/modal/local-minor ambiguity. На `G7`, когда `Cmaj7` уже известен как next chord, локальный C minor / `iv–V–i` не подтверждается; используется глобальный `V→I`.

## 4. UI / диагностика

Текущий layout `0.3e-ui` сохранён.

- верхняя строка «Мышление» не склеивает материалы из разных unresolved interpretations;
- при `primaryInterpretationIndex == -1` показывается явная неоднозначность и количество трактовок;
- в разделе «Источники / ноты» context-dependent sources сохраняют provenance по harmonic interpretation;
- финальный product UX и Why?-объяснение остаются `0.3g`.

## 5. Acceptance checklist

- [x] `iv–V–i` распознаётся на доступных позициях;
- [x] финальный `V→i` / `V→I` не придумывает неизвестный predominant;
- [x] local `iv–V–i` может сформировать candidate/established minor center;
- [x] unresolved primary сохраняет несколько context-specific strategies;
- [x] у unresolved primary нет скрытого выбранного interpretation;
- [x] unique primary ранжируется раньше alternatives, но alternatives не теряются;
- [x] strategy evidence/confidence не подменяются recommendation priority;
- [x] UI не смешивает разные unresolved interpretations в одну строку «Мышление»;
- [x] существующие SubV spelling, melodic-minor/diminished sources и 0.3e concepts не регрессировали;
- [x] Stage 1 contract `previous/current/next` не расширен;
- [x] все 12 regression targets зелёные в Windows Build #284;
- [x] live-test принят пользователем 2026-09-25.

## Граница

Не входит в `0.3f`:

- T1/T2/T3 policy;
- генерация Phrase/MIDI;
- финальный Why?-текст и product UI;
- расширение timeline history глубже `previous/current/next`;
- широкий новый каталог гармонических оборотов помимо `iv–V–i`.

Следующий checkpoint: `0.3g — Explanation / usable output`.
