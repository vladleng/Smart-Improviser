# Stage 3 / 0.3f — Context-aware ranking / ambiguity

База: принятые `0.3e` и `0.3e-ui`. Stable остаётся `0.3`.

## Цель

`0.3f` не добавляет новый большой каталог гамм. Подэтап делает уже накопленные стратегии контекстно корректными при нескольких гармонических трактовках и устраняет обнаруженный пробел Pattern Recognizer для классического минорного `iv–V–i`.

Основное правило: **если Stage 2 не выбрал единственную трактовку, Stage 3 не выбирает её скрыто**. Музыкально разные варианты должны сохраняться раздельно и быть привязаны к конкретному `global / local / modal` interpretation.

## 1. Context-aware strategies

- foundation по явным звукам аккорда остаётся `interpretationIndependent` и доступен всегда;
- scalar / special-source стратегии строятся для валидных harmonic interpretations, а не только для `primaryInterpretationIndex`;
- каждая context-dependent strategy сохраняет свой `interpretationIndex` и evidence;
- если `primaryInterpretationIndex >= 0`, стратегии основной трактовки выводятся раньше альтернатив;
- если `primaryInterpretationIndex == -1`, ни global, ни local, ни modal вариант не получает скрытого статуса primary;
- разные interpretations не смешиваются в одну strategy или concept;
- `priority` остаётся recommendation/catalog priority и не подменяет harmonic confidence.

## 2. Deterministic ranking

Порядок должен быть воспроизводимым:

1. interpretation-independent foundation;
2. при наличии primary — стратегии primary interpretation;
3. альтернативные interpretations;
4. внутри одного interpretation — существующий `priority`, затем стабильный `ruleId`.

При unresolved primary interpretations выводятся в стабильном порядке Stage 2, но сам порядок **не означает**, что первая трактовка музыкально предпочтительнее.

## 3. Minor iv–V–i — закрытие Issue #29

Добавить отдельный harmonic pattern для классического оборота:

```text
Fm7 → G7 → Cm7
iv7 → V7 → i7
```

Ожидаемое поведение в `C minor`:

```text
Fm7:  Минорный iv–V–i • 1 / 3
G7:   Минорный iv–V–i • 2 / 3   (когда видны Fm7 и Cm7)
Cm7:  общий V–i / resolution, если более ранний predominant уже недоступен
```

Причина последнего правила: Stage 1 contract содержит только `previous / current / next`; на финальном `i` нельзя достоверно восстановить, был ли predominant `iiø` или `iv`.

То же правило применяется симметрично к major `V→I`: на последней позиции без более глубокой истории не утверждать конкретный `ii–V–I`, а показывать общий `V–I / resolution`.

### Local-center cases

`iv–V–i` должен работать не только относительно global key, но и как local minor cadence, когда соседние аккорды дают достаточные evidence. Stage 1 contract при этом не расширяется.

## 4. UI / диагностика

Текущий layout `0.3e-ui` сохраняется.

- верхняя строка «Мышление» не должна склеивать материалы из разных unresolved interpretations;
- при `primaryInterpretationIndex == -1` показывается явная неоднозначность и количество трактовок;
- в разделе «Источники / ноты» context-dependent sources помечаются как primary / alternative либо candidate с названием harmonic interpretation;
- финальный product UX и Why?-объяснение остаются `0.3g`.

## 5. Regression cases

Обязательные автоматические проверки:

```text
C minor: Fm7 → G7 → Cm7
C minor: Fm7 → G7 boundary
C minor: G7 → Cm7 без более глубокой истории
C minor: Dm7b5 → G7 → Cm7
C major: G7 → Cmaj7 без более глубокой истории
borrowed iv / modal ambiguity
candidate local center с primaryInterpretationIndex = -1
confirmed local cadence с выбранным primary
unresolved dominant / missing next chord
```

Проверить:

- [ ] `iv–V–i` распознаётся на доступных позициях;
- [ ] финальный `V→i` / `V→I` не придумывает неизвестный predominant;
- [ ] local `iv–V–i` может сформировать candidate/established minor center;
- [ ] unresolved primary сохраняет несколько context-specific strategies;
- [ ] у unresolved primary нет скрытого выбранного interpretation;
- [ ] unique primary ранжируется раньше alternatives, но alternatives не теряются;
- [ ] strategy evidence/confidence не подменяются recommendation priority;
- [ ] UI не смешивает разные unresolved interpretations в одну строку «Мышление»;
- [ ] существующие SubV spelling, melodic-minor/diminished sources и 0.3e concepts не регрессировали;
- [ ] Stage 1 contract `previous/current/next` не расширен;
- [ ] все regression targets зелёные.

## Граница

Не входит в `0.3f`:

- T1/T2/T3 policy;
- генерация Phrase/MIDI;
- финальный Why?-текст и product UI;
- расширение timeline history глубже `previous/current/next`;
- широкий новый каталог гармонических оборотов помимо явно зафиксированного `iv–V–i`.

После принятия `0.3f` переходим к `0.3g — Explanation / usable output`.
