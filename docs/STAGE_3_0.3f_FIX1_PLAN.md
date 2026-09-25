# Stage 3 / 0.3f fix1 — Pattern continuity / hierarchical cadence context

База: принятый `0.3f` (`PR #30`, Windows Build #284). Stable остаётся `0.3`.

## Причина fix1

Live-проверка реального материала после принятия `0.3f` выявила ограничение текущей модели паттернов.

На последовательности в `F major`:

```text
Gm7 → C7 → Fmaj7
 ii     V      I
```

на `C7` движок видит полное окно `Gm7 → C7 → Fmaj7` и может подтвердить `ii–V–I`, но на `Fmaj7` snapshot Stage 1 уже содержит только:

```text
previous = C7
current  = Fmaj7
next     = ...
```

из-за чего UI деградирует до общего `V–I • 2/2` и теряет факт, что текущая тоника является **третьей позицией уже подтверждённого `ii–V–I`**.

Для импровизации это недостаточно: разрешающая тоника остаётся частью той же кадансовой мысли и должна сохранять контекст оборота.

Второй выявленный случай:

```text
Am7 → D7 → Gm7 → C7 → Fmaj7
 iii    VI7    ii     V      I
```

где `D7 = V/ii`. Музыкально это единый расширенный кадансовый оборот / цепь к `F major`, а не набор независимых фрагментов `D7→Gm7` и `Gm7→C7→Fmaj7`.

## Цель

Добавить **continuity уже распознанного harmonic pattern**, не расширяя Stage 1 contract произвольной историей аккордов.

Stage 1 по-прежнему передаёт только:

```text
previous / current / next / global key
```

Но Harmonic Engine получает собственный host-neutral слой состояния распознанного музыкального события — условно `RecognizedPatternInstance` / `PatternContext`.

Это не «память всех прошлых аккордов». Хранится только уже распознанный и подтверждённый структурный факт с ограниченной жизнью и явными boundaries.

## 1. Pattern continuity на разрешающем аккорде

Если полный паттерн был достоверно подтверждён на предыдущей позиции и текущий аккорд является его ожидаемым resolution/member, контекст переносится на текущую позицию.

Ожидаемое отображение:

```text
Gm7    → Major ii–V–I • 1/3
C7     → Major ii–V–I • 2/3
Fmaj7  → Major ii–V–I • 3/3
```

Аналогично для minor cadences:

```text
Dm7b5 → G7 → Cm7
 iiø     V     i
```

и

```text
Fm7 → G7 → Cm7
 iv     V     i
```

На финальном `i` конкретный `iiø–V–i` либо `iv–V–i` разрешено показывать как `3/3` **только если этот pattern instance был подтверждён ранее и перенесён как evidence**.

Если такой сохранённый pattern context отсутствует, действует безопасный fallback принятого `0.3f`:

```text
V→I / V→i resolution
```

без выдумывания неизвестного predominant.

Таким образом fix1 не отменяет ambiguity rule `0.3f`, а добавляет новый вид достаточного evidence — ранее подтверждённый pattern instance.

## 2. Иерархический расширенный каданс

Для последовательности:

```text
Am7 → D7 → Gm7 → C7 → Fmaj7
 iii    VI7    ii     V      I
```

в `F major` вводится единый top-level cadence context.

Предлагаемая пользовательская семантика:

```text
Am7    → Расширенный каданс iii–VI7–ii–V–I • 1/5
D7     → Расширенный каданс iii–VI7–ii–V–I • 2/5
Gm7    → Расширенный каданс iii–VI7–ii–V–I • 3/5
C7     → Расширенный каданс iii–VI7–ii–V–I • 4/5
Fmaj7  → Расширенный каданс iii–VI7–ii–V–I • 5/5
```

Внутри top-level pattern сохраняются вложенные структурные отношения:

```text
D7 → Gm7               = V/ii → ii
Gm7 → C7 → Fmaj7       = ii–V–I
```

Они используются как evidence и для локальных функций/targets, но не должны дробить пользовательский **основной** кадансовый контекст на несколько независимых оборотов.

Рабочее внутреннее имя типа: `majorCadentialChain` (точное имя enum можно скорректировать при реализации). Пользовательский термин: **«Расширенный каданс iii–VI7–ii–V–I»**.

## 3. Иерархия и перекрытие паттернов

Один аккорд может одновременно:

- быть завершающим member подтверждённого предыдущего pattern;
- участвовать в boundary/candidate следующего harmonic event;
- иметь собственную текущую функцию и resolution semantics.

Например `Fmaj7` может быть `5/5` завершённого `iii–VI7–ii–V–I`, а следующий `Fm7` уже давать evidence нового borrowed/modal события.

Поэтому модель должна различать:

```text
primary / top-level pattern context
nested subpatterns
candidate next pattern
completed carried pattern
```

Не требуется показывать пользователю все слои одновременно в верхней карточке. Для `0.3f fix1` достаточно корректного top-level контекста; детальная подача вложенных отношений может использоваться в diagnostic / future Why? (`0.3g`).

## 4. Предлагаемый контракт PatternContext

Минимальные данные сохраняемого pattern instance:

```text
pattern type
center / harmonic interpretation
member count
current member index
status: candidate / confirmed / completed
start event identity / position
expected or confirmed resolution event
nested/subpattern evidence
confidence/evidence
```

Invariants:

- PatternContext host-neutral и живёт в Core;
- ARA/JUCE не владеют музыкальной логикой;
- deterministic behavior;
- один и тот же timeline/context даёт одинаковый результат;
- pattern instance не может жить бесконечно: после completion и выхода за релевантную позицию он сбрасывается;
- chord edit/seek/reopen должны пересчитывать context из timeline, а не доверять stale runtime memory;
- нельзя превращать continuity в скрытое расширение Stage 1 до произвольной `previousN` истории.

## 5. Scope 0.3f fix1

Входит:

- continuity confirmed `ii–V–I` до `I • 3/3`;
- continuity confirmed `iiø–V–i` до `i • 3/3`;
- continuity confirmed `iv–V–i` до `i • 3/3`;
- отдельный top-level extended cadence `iii–VI7–ii–V–I`;
- nested evidence `V/ii→ii` и `ii–V–I` без дробления main pattern;
- безопасный fallback к generic `V→I/i`, если сохранённого подтверждённого pattern instance нет;
- overlapping completed/current candidate semantics;
- regression/live diagnostics.

Не входит:

- произвольный pattern graph для любых длинных прогрессий;
- неограниченная harmonic history;
- phrase planning на несколько тактов;
- T1/T2/T3 policy;
- изменение каталога Бойко;
- финальный Why? UX (`0.3g`), кроме данных/evidence, необходимых для continuity;
- автоматическое объединение любых соседних ii–V / secondary-dominant цепей без отдельного правила.

## 6. Обязательные regression cases

```text
F major: Gm7 → C7 → Fmaj7
C minor: Dm7b5 → G7 → Cm7
C minor: Fm7 → G7 → Cm7
F major: Am7 → D7 → Gm7 → C7 → Fmaj7
```

Проверить:

- [ ] `Fmaj7` сохраняет `Major ii–V–I • 3/3`, если `ii–V–I` был подтверждён;
- [ ] `Cm7` сохраняет `Minor iiø–V–i • 3/3`, если этот pattern был подтверждён;
- [ ] `Cm7` сохраняет `Minor iv–V–i • 3/3`, если именно этот pattern был подтверждён;
- [ ] без carried PatternContext финальный `V→I/i` остаётся generic resolution;
- [ ] `Am7–D7–Gm7–C7–Fmaj7` распознаётся как один `iii–VI7–ii–V–I` top-level pattern;
- [ ] `D7` сохраняет функцию `V/ii`, `Gm7–C7–Fmaj7` остаётся валидным nested `ii–V–I` evidence;
- [ ] main UI не дробит расширенный каданс на отдельные top-level обороты;
- [ ] chord edits / seek не оставляют stale pattern context;
- [ ] Stage 1 contract остаётся `previous/current/next`;
- [ ] regression `0.3f` ambiguity / local-center / SubV не ломается.

## Gate

`0.3f fix1` принимается только после:

1. автоматических regression tests;
2. зелёного Windows artifact;
3. live-test в Studio Pro минимум на двух обязательных сценариях:
   - `Gm7 → C7 → Fmaj7`;
   - `Am7 → D7 → Gm7 → C7 → Fmaj7`;
4. проверки seek/chord-edit, чтобы continuity не становился stale state.

После acceptance продолжаем `0.3g — Explanation / usable output`.
