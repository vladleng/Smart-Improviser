# Stage 3 / 0.3f fix2 — Pattern evidence / false-positive guards

> **Исторический статус:** Windows Build #328 прошёл regression/Windows gate, но live-разбор Corcovado после билда выявил ошибочную музыкальную трактовку `D7/A → Ab° → Gm7` как passing diminished к G minor. Этот единственный пункт fix2 **отменён и заменён** `0.3f fix3 — Rootless dominant / Corcovado diminished correction`. Остальные evidence guards и `iii–vi–ii–V` сохраняются. Fix2 не считается отдельным принятым checkpoint до acceptance fix3.

База: принятый `0.3f fix1` (`PR #31`, Windows Build #310). Stable остаётся `0.3`. Этот refinement появился после real-harmony validation концертной гармонии **Corcovado** и должен быть принят вместе с последующим fix3 до перехода к `0.3g`.

## Причина fix2

Синтетические regression-кейсы подтвердили continuity, но реальная гармония показала, что движок иногда слишком рано принимает знакомый шаблон и игнорирует уже известное продолжение.

Ключевые FAIL-кейсы:

```text
C major: Fm7 → Bb7 → Em7
```

Первые два аккорда похожи на `ii–V` в Eb major, но известный третий аккорд не подтверждает Eb. Нельзя показывать `Мажорный ii–V–I • 1/3` как будто tonic ещё неизвестен.

```text
C major: Em7 → A7 → D7
```

`A7→D7` — валидная applied-dominant / dominant-chain связь, но `D7` не является tonic major. Совпадения root недостаточно для `V→I` или локального D major.

```text
C major: Dm7 → G7 → D7/A
```

`Dm7→G7` похоже на `ii–V` в C, но известный третий аккорд опровергает `I`.

```text
C major: Em7 → Am7 → Dm7 → G7
```

Это самостоятельный устойчивый оборот `iii–vi–ii–V`. Движок не должен реконструировать отсутствующий `I` и называть середину последовательности `I–VI–ii–V • 3/4`.

## 1. Known-future veto для cadence candidates

Если bounded `PatternTimelineWindow` уже содержит ожидаемый третий member, incomplete candidate разрешён только когда этот member действительно подтверждает ожидаемую тонику и её quality.

Правило:

- `ii → V → I` — I должен иметь ожидаемый root **и major quality**;
- `iiø → V → i` — i должен иметь ожидаемый root и minor-family quality;
- `iv → V → i` — i должен иметь ожидаемый root и minor-family quality;
- то же правило применяется к `ii/iiø → SubV → I/i` candidates;
- если третьего member ещё нет в bounded window, прежняя осторожная candidate semantics сохраняется.

Это veto, а не новый winner: при конфликте движок имеет право показать `оборот не распознан` или другую подтверждённую связь.

## 2. Target-quality guard

Dominant target нельзя автоматически интерпретировать как major tonic:

```text
A7 → D7
```

может быть applied dominant / dominant chain, но не `V→I in D major`.

Инвариант:

> tonicization требует не только target root, но и совместимую tonic quality.

`major` может подтверждать major tonic, `minor` / minor-family — minor tonic. `dominant` не подтверждает tonic major.

## 3. `iii–vi–ii–V` как first-class pattern

В major key добавляется отдельный устойчивый четырёхчленный оборот:

```text
C major: Em7 → Am7 → Dm7 → G7
         iii     vi     ii     V
```

Ожидаемое отображение:

```text
Em7 → iii–vi–ii–V • 1/4
Am7 → iii–vi–ii–V • 2/4
Dm7 → iii–vi–ii–V • 3/4
G7  → iii–vi–ii–V • 4/4
```

Это самостоятельный top-level pattern. На текущем уровне не требуется объяснять его как замену `I`; более глубокий функциональный анализ может появиться позже.

Полный `I–VI–ii–V` сохраняется, но позиции `3/4` и `4/4` нельзя достраивать только из `VI→ii→V` или `ii→V` без реального earlier evidence. Полный четырёхчленный PatternContext восстанавливается из bounded timeline, если I действительно присутствует.

`iii–vi–ii–V` завершается на V, поэтому `4/4` означает completion самого pattern, **не confirmed tonic resolution**.

## 4. Passing diminished bridge — SUPERSEDED

Первоначальная гипотеза fix2:

```text
D7/A → Ab° → Gm7
```

как `passingDiminished • 1/3…3/3` к G minor оказалась музыкально неверной для реального Corcovado voicing.

Правильная трактовка зафиксирована в [STAGE_3_0.3f_FIX3_PLAN.md](STAGE_3_0.3f_FIX3_PLAN.md): `Ab° = {Ab,B,D,F}` является rootless `G7(b9)` в C major, а при наличии E — rootless `G13(b9)`. Поэтому специальный bridge к G minor удаляется.

Сам тип `passingDiminished` остаётся в каталоге для будущих корректных случаев.

## 5. Presentation semantics для будущего 0.3g

Цвета фиксируются сейчас как семантический контракт, но полноценный UI для implied/conflicting members относится к presentation-layer `0.3g`:

- **серый** — неявный/отсутствующий member полного шаблона;
- **янтарный / оранжевый** — предполагаемый/implied member или ещё не подтверждённая гипотеза;
- **красный** — известное продолжение конфликтует с ожидаемым member / гипотеза опровергнута.

Важно: first-class `iii–vi–ii–V` не должен показывать отсутствующий I как обязательную «дыру»; он является полноценным самостоятельным оборотом.

## 6. Scope fix2

Сохраняется:

- known-future veto false `ii–V–I / iiø–V–i / iv–V–i` candidates;
- strict target-quality semantics;
- самостоятельный `iii–vi–ii–V`;
- full-window reconstruction настоящего `I–VI–ii–V` без fabricated earlier member;
- regression cases из Corcovado;
- сохранение fix1 continuity и bounded deterministic reconstruction.

Заменено fix3:

- специальная трактовка `D7/A→Ab°→Gm7` как passing diminished.

Не входит:

- универсальный graph всех cycle-of-fifths progressions;
- автоматическое распознавание любого diminished как passing/common-tone;
- deceptive cadence catalog;
- глубокое объяснение функциональных substitutions;
- T1/T2/T3;
- финальный Why? UX и цветной implied-member UI — это `0.3g`.

## 7. Regression / live gate

Обязательные regression cases fix2:

```text
Fm7 → Bb7 → Em7               => НЕ Eb-major ii–V–I
Em7 → A7 → D7                 => НЕ D-major ii–V–I / V–I
Dm7 → G7 → D7/A               => НЕ C-major ii–V–I
Em7 → Am7 → Dm7 → G7          => iii–vi–ii–V • 1/4…4/4
Cmaj7 → Am7 → Dm7 → G7        => I–VI–ii–V • 1/4…4/4
Em7b5 → A7b13 → Dm7           => fix1 minor iiø–V–i regression remains green
Gm7 → C7 → Fmaj7              => fix1 local ii–V–I regression remains green
```

Diminished/Corcovado gate перенесён в fix3.

## Gate

Build #328 подтвердил regression suite и Windows artifact для fix2, но музыкальное acceptance отложено до fix3. После acceptance fix3 продолжаем `0.3g — Explanation / usable output`.
