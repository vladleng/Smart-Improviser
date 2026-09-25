# Harmonic Engine — уточнения и расширения после stable 0.3

Этот файл фиксирует музыкальные и архитектурные наблюдения, обнаруженные уже после принятия stable `0.3` / Stage 2. Каждое изменение должно входить в явный checkpoint/fix и проходить regression/live validation.

## 1. Minor iv–V–i — реализовано в 0.3f

Во время live-test `0.3e-ui` был обнаружен классический случай:

```text
C minor: Fm7 → G7 → Cm7
         iv7    V7    i7
```

Пробел закрыт в `0.3f`:

- добавлен отдельный `minor iv–V–i`;
- `iv→V` поддерживает boundary candidate;
- полный `iv→V→i` подтверждается на доступном окне;
- local-minor semantics поддерживаются;
- false-positive `C major: Fm7→G7→Cmaj7` устранён;
- Issue #29 закрыт;
- Windows Build #284 и live-test приняты.

Историческое ограничение оставалось следующим: на финальном `i`, когда earlier predominant выпал из `previous/current/next`, движок использовал безопасный generic `V→i`, потому что без дополнительного evidence нельзя было узнать, был ли перед доминантой `iiø` или `iv`.

## 2. 0.3f fix1 — continuity подтверждённого pattern

После принятия `0.3f` live-проверка реального материала показала, что safe fallback слишком консервативен, **если полный паттерн уже был подтверждён на предыдущей позиции**.

Пример в `F major`:

```text
Gm7 → C7 → Fmaj7
 ii     V      I
```

На `C7` snapshot содержит все три аккорда, поэтому `ii–V–I` уже достоверно известен. На `Fmaj7` текущий snapshot видит только `C7→Fmaj7` и деградирует до `V–I`.

Принятое уточнение:

- generic `V→I/i` остаётся безопасным fallback, если подтверждённого earlier pattern действительно нет;
- если `ii–V–I`, `iiø–V–i` или `iv–V–i` был подтверждён ранее, этот факт становится новым достаточным evidence;
- resolution chord сохраняет membership и позицию `3/3` через ограниченный host-neutral `PatternContext` / `RecognizedPatternInstance`;
- Stage 1 contract не расширяется до arbitrary history.

Целевое поведение:

```text
Gm7    → ii–V–I • 1/3
C7     → ii–V–I • 2/3
Fmaj7  → ii–V–I • 3/3
```

Аналогично:

```text
Dm7b5 → G7 → Cm7   = iiø–V–i • 1/3 … 3/3
Fm7   → G7 → Cm7   = iv–V–i  • 1/3 … 3/3
```

## 3. Иерархический расширенный каданс

В том же fix1 добавляется ограниченный top-level pattern для:

```text
F major: Am7 → D7 → Gm7 → C7 → Fmaj7
          iii    VI7    ii     V      I
```

где:

```text
D7 = V/ii
```

Пользовательский основной контекст должен быть единым:

```text
Расширенный каданс iii–VI7–ii–V–I • 1/5 … 5/5
```

а локальные отношения сохраняются как nested evidence/subpatterns:

```text
D7 → Gm7               = V/ii → ii
Gm7 → C7 → Fmaj7       = ii–V–I
```

Это важно для Improvisation Engine: мелодическая и функциональная мысль не должна искусственно обнуляться на границе вложенного `ii–V–I`, если вся цепь является одним кадансовым событием.

## 4. Ограничение Stage 1 contract сохраняется

Harmonic Engine получает только:

```text
previous chord
current chord
next chord
global key
```

`0.3f fix1` не добавляет `previous2`, `previous3` и т. п.

Вместо этого Core хранит только уже распознанный event context:

```text
pattern type
center / interpretation
member count / current position
status: candidate / confirmed / completed
start / resolution identity
nested evidence
confidence
```

Это не общая память истории. PatternContext должен иметь чёткую lifetime и пересчитываться/сбрасываться при seek, chord edits и reopen.

## 5. Перекрытие harmonic events

Один аккорд может одновременно:

- завершать confirmed pattern;
- быть текущей функцией нового локального контекста;
- давать candidate evidence следующего pattern.

Например `Fmaj7` может быть `5/5` завершённого `iii–VI7–ii–V–I`, а следующий `Fm7` уже открывает borrowed/modal ситуацию.

Поэтому модель различает top-level completed/current pattern и candidate next event. UI не обязан показывать все уровни одновременно; detailed provenance предназначен также для `0.3g Why?`.

## 6. Regression-набор 0.3f fix1

Обязательные кейсы:

```text
F major: Gm7 → C7 → Fmaj7
C minor: Dm7b5 → G7 → Cm7
C minor: Fm7 → G7 → Cm7
F major: Am7 → D7 → Gm7 → C7 → Fmaj7
```

Проверить:

- continuity `3/3` только при carried confirmed PatternContext;
- generic `V→I/i` при отсутствии такого evidence;
- единый top-level `iii–VI7–ii–V–I`;
- сохранение `V/ii→ii` и `ii–V–I` как nested evidence;
- отсутствие stale context после seek/edit/reopen;
- отсутствие regression `0.3f` ambiguity/local-center/SubV;
- Stage 1 contract остаётся без расширения.

Полная спецификация: [STAGE_3_0.3f_FIX1_PLAN.md](STAGE_3_0.3f_FIX1_PLAN.md).
