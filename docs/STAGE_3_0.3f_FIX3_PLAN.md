# Stage 3 / 0.3f fix3 — Rootless dominant / Corcovado diminished correction

База: принятый `0.3f fix1` и реализованный `0.3f fix2`. Windows Build #328 подтвердил техническую целостность fix2, но live-разбор **Corcovado** выявил неверную музыкальную гипотезу для `D7/A → Ab° → Gm7`. Поэтому fix2 не принимается как финальный checkpoint без этого refinement.

## Причина fix3

В fix2 последовательность

```text
D7/A → Ab° → Gm7
```

была трактована как `passingDiminished` bridge к G minor. Это оказалось неверно для фактического voicing Corcovado.

В реальном voicing `Ab°` содержит:

```text
Ab – B – D – F
```

Относительно G это:

```text
Ab = b9
B  = 3
D  = 5
F  = b7
```

то есть **G7(b9) без корня G**. В оригинальном voicing сверху также присутствует E, что добавляет 13 и даёт функциональное чтение **G13(b9) без корня**.

Следовательно, этот diminished-looking chord не должен автоматически создавать локальный G minor.

## 1. Functional alias без разрушения написанного аккорда

Вводится host-neutral `ImpliedDominantReading`.

Принцип:

- написанный / нормализованный аккорд остаётся `Ab°`;
- если его pitch content в явном глобальном key совпадает с `b9–3–5–b7` глобальной V, создаётся дополнительное функциональное чтение **rootless V7(b9)**;
- implied root не подменяет chord symbol;
- если присутствует 13, чтение фиксирует дополнительную dominant color;
- confidence функциональной трактовки отделён от identity исходного аккорда.

Для C major:

```text
Ab° = {Ab, B, D, F}
    ≈ rootless G7(b9)

Ab° + E
    ≈ rootless G13(b9)
```

Из-за симметрии diminished выбор implied root нельзя делать из pitch collection без контекста. В начальной реализации alias привязан к явному global key: проверяется именно rootless V этого key, чтобы не создавать четыре равноправных произвольных корня.

## 2. Коррекция Corcovado context

Ожидаемая структура начала:

```text
D7/A → Ab° → Gm7 → C7 → Fmaj7
 V/V    V*     ii      V      I
         |        \_____ F major _____/
         +-- rootless G7(b9) in C
```

Где `V*` означает записанный diminished, функционально читаемый как rootless global V.

Ожидаемое поведение:

- `D7/A` не получает преждевременно local G minor;
- `D7/A → Ab°` может быть показан как dominant chain `V/V → implied V`;
- на `Ab°` effective function = Dominant, implied root = G;
- отсутствие фактического C после него означает, что confirmed `V→I` не заявляется;
- `Gm7` не считается tonic resolution предыдущей dominant chain;
- `Gm7 → C7 → Fmaj7` начинает новый подтверждённый local `ii–V–I in F`.

Это важный инвариант: **функциональная направленность и фактическое разрешение — не одно и то же**.

## 3. Что удаляется из fix2

Специальное правило

```text
D7/A → Ab° → Gm7 = passingDiminished • 1/3…3/3
```

для Corcovado удаляется.

Также удаляется следствие этого правила — автоматическая temporary tonicization `G minor` уже на `D7/A` / `Ab°`.

Тип `passingDiminished` остаётся в общем каталоге проекта для будущих корректных случаев; fix3 отменяет только неверную Corcovado-специализацию.

## 4. Presentation contract — не забыть в 0.3g

Цветовая семантика подтверждена пользователем и является обязательным контрактом presentation layer:

- **приглушённый серый** — отсутствующий / неявный member полного шаблона;
- **янтарный / оранжевый** — implied / provisional member или ещё не подтверждённая гипотеза;
- **красный** — фактический конфликт, известное продолжение опровергает гипотезу.

Применение к rootless dominant в будущем UI:

- отсутствующий физический root G у `Ab°` может быть показан как **implied**, то есть янтарным, а не красным;
- если known future противоречит ожидаемому resolution, это уже красный conflict;
- `iii–vi–ii–V` остаётся самостоятельным first-class pattern и не показывает отсутствующий `I` как серую «дыру».

Полноценная цветная отрисовка остаётся задачей `0.3g`; fix3 фиксирует данные и семантику, но не раздувает текущий диагностический UI.

## 5. Сохраняемый scope fix2

Fix3 не отменяет полезные результаты fix2:

- known-future veto false `ii–V–I / iiø–V–i / iv–V–i` candidates;
- strict target-quality guard;
- запрет fabricated missing `I`;
- first-class `iii–vi–ii–V • 1/4…4/4`;
- full-window reconstruction настоящего `I–VI–ii–V`;
- fix1 continuity и bounded deterministic PatternContext.

## 6. Regression / live gate

Обязательные кейсы:

```text
C major: D7/A → Ab°
=> dominant chain V/V → implied V
=> Ab° has implied G7(b9) reading
=> local G minor НЕ создаётся

C major: D7/A → Ab°(+E)
=> implied G13(b9), root omitted

C major: D7/A → Ab° → Gm7 → C7 → Fmaj7
=> Gm7 starts local F-major ii–V–I • 1/3
=> C7 = 2/3
=> Fmaj7 = 3/3

Fm7 → Bb7 → Em7
=> НЕ Eb-major ii–V–I

Em7 → A7 → D7
=> НЕ D-major ii–V–I / V–I

Dm7 → G7 → D7/A
=> НЕ C-major ii–V–I

Em7 → Am7 → Dm7 → G7
=> iii–vi–ii–V • 1/4…4/4

Em7b5 → A7b13 → Dm7
=> fix1 minor iiø–V–i сохраняется
```

## Gate

`0.3f fix3` принимается только после:

1. зелёного regression suite;
2. зелёного Windows artifact `Smart-Improviser-0.3f-fix3-Windows`;
3. live-test начала Corcovado в Studio Pro;
4. проверки, что `D7/A` и `Ab°` больше не получают ложный G-minor center;
5. проверки, что `Gm7→C7→Fmaj7` по-прежнему распознаётся как local F-major ii–V–I;
6. подтверждения пользователя.

После acceptance продолжаем `0.3g — Explanation / usable output`.
