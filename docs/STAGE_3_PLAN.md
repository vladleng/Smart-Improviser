# Stage 3 — Improvisation Engine: рабочий план

> Обновлено 2026-09-26. База: stable 0.3, Stage 2 принят. `0.3a–0.3f`, `0.3f fix1` и refinement `fix2–fix4` приняты в итоговом состоянии fix4. Windows Build #331 и live-test fix4 зелёные. Текущий checkpoint — **`0.3g Explanation / usable output`**. Stable остаётся 0.3.
> Начальная методика и текстовая форма подсказки согласованы: [IMPROVISATION_METHOD.md](IMPROVISATION_METHOD.md). См. [PRODUCT_WORKFLOW.md](PRODUCT_WORKFLOW.md).

## Результат Stage

Для валидного HarmonicSituation выдавать объяснимые музыкальные стратегии: логика мышления, source material, chord/guide/target notes, resolution и альтернативы. Результат полезен сам по себе и пригоден для будущего подбора библиотечных идей.

Stage 3 описывает допустимый материал и стратегии. Stage 4 управляет их выбором по Tension 1/2/3. Song workspace и Phrase editing относятся к последующим Stage.

## Границы архитектуры

- Вход — готовый HarmonicSituation; Core не зависит от JUCE/ARA/DAW.
- Расширяется существующий ImprovisationStrategy из ImprovisationContracts.h.
- ResolutionTarget и результаты Stage 2 используются повторно; анализ не дублируется.
- Global/local/modal interpretation и unresolved primary сохраняют свою семантику.
- Confidence гармонического вывода отделён от recommendation priority.
- Explicit chord information имеет приоритет над inferred source.
- Stage 1 contract сохраняется: `previous/current/next`; fix1/fix2/fix3/fix4 не добавляют скрытый `previousN`.
- Подтверждённый harmonic pattern сохраняется как отдельный host-neutral `PatternContext`.
- PatternContext реконструируется детерминированно из bounded timeline window и не хранит stale runtime history.
- Bounded future evidence может опровергать provisional cadence candidate; известное противоречие нельзя скрывать под прежним шаблоном.
- Target root без совместимой tonic quality недостаточен для tonicization.
- Written chord identity и inferred functional alias — разные сущности: rootless-dominant reading не переписывает исходный chord symbol.
- `IncompleteCadence` — presentation/evidence object для незавершённого шаблона, а не local key, confirmed pattern или resolution.
- Source/degree/target data должны подходить для будущих Phrase metadata, не требуя ранней реализации библиотеки.
- Вычисления и форматирование отделяются от paint; UI отображает результат.

## Checkpoints

```text
0.3a — Strategy contract / foundation
0.3b — Structural tones / targets / resolution
0.3c — Diatonic / modal sources
0.3d — Melodic minor / diminished sources
0.3e — Harmonic concepts
0.3f — Context-aware ranking / ambiguity
0.3f fix1 — Pattern continuity / hierarchical cadence context
0.3f fix2 — Pattern evidence / false-positive guards
0.3f fix3 — Rootless dominant / Corcovado diminished correction
0.3f fix4 — Незавершённые обороты / отсутствующие ступени
0.3g — Explanation / usable output
0.3g fix1 — Corcovado V/V → implied V presentation
0.3h — Integration / musical validation
0.4  — Stage 3 complete
```

### 0.3a — Strategy contract / foundation [ACCEPTED]
Windows Build #231 — success; живой тест принят Владом 2026-09-24; PR #23 слит в main.

### 0.3b — Structural tones / targets / resolution [ACCEPTED]
Windows Build #234 — success; живой тест принят Владом; PR #24 слит в main.

### 0.3c — Diatonic / modal sources [ACCEPTED]
Windows Build #237 success; принят Владом 2026-09-25, PR #25 merged.

### 0.3d — Melodic minor / diminished sources [ACCEPTED]
Windows Build #246 — success; живой тест 0.3d fix2 принят Владом 2026-09-25; PR #26 слит в main.

### 0.3e — Harmonic concepts [ACCEPTED]
Музыкальная часть и структурный UI приняты Владом 2026-09-25. Windows Build #274 — success; PR #28 слит в main.

### 0.3f — Context-aware ranking / ambiguity [ACCEPTED]

Принято Владом 2026-09-25. Windows Build #284 — success. PR #30 слит в `main`; Issue #29 закрыт.

- [x] Strategies для всех валидных global/local/modal interpretations.
- [x] `primaryInterpretationIndex = -1` без скрытого winner.
- [x] Deterministic ranking отдельно от harmonic confidence.
- [x] Minor `iv–V–i`, boundary/local semantics.
- [x] Generic `V→I/i` как safe fallback при недостатке earlier evidence.
- [x] False-positive `C major: Fm7 → G7 → Cmaj7` устранён.
- [x] Stage 1 contract `previous/current/next` сохранён.
- [x] UI сохраняет provenance unresolved alternatives.

### 0.3f fix1 — Pattern continuity / hierarchical cadence context [ACCEPTED]

Полный acceptance: [STAGE_3_0.3f_FIX1_PLAN.md](STAGE_3_0.3f_FIX1_PLAN.md).

Windows Build #310 — success; live-test принят Владом 2026-09-25.

- [x] Введён host-neutral `PatternContext` без arbitrary history.
- [x] Confirmed major `ii–V–I` сохраняется до `I • 3/3`.
- [x] Confirmed minor `iiø–V–i` сохраняется до `i • 3/3`.
- [x] Confirmed minor `iv–V–i` сохраняется до `i • 3/3`.
- [x] Без полного carried evidence сохраняется safe fallback `V→I/i`.
- [x] Добавлен top-level `iii–VI7–ii–V–I • 1/5 … 5/5`.
- [x] `V/ii→ii` и `ii–V–I` сохраняются как nested evidence/subpatterns.
- [x] Seek/chord edits/reopen реконструируют context без stale runtime state.
- [x] Stage 1 contract остаётся `previous/current/next`.
- [x] STOP-boundary resolver синхронизирует визуальную границу Studio Pro с chord-event; PLAY остаётся strict.
- [x] Regression suite зелёный в Windows Build #310.
- [x] Live-test принят пользователем 2026-09-25.

### 0.3f fix2 — Pattern evidence / false-positive guards [ACCEPTED VIA FIX4]

Полный scope/history: [STAGE_3_0.3f_FIX2_PLAN.md](STAGE_3_0.3f_FIX2_PLAN.md). Первоначальная diminished-специализация была отменена; сохраняемые evidence guards вошли в fix4 и прошли финальный regression/live gate.

- [x] Known-future veto: `Fm7→Bb7→Em7` не создаёт ложный Eb-major `ii–V–I`.
- [x] Known-future veto: `Em7→A7→D7` не создаёт ложный D-major `ii–V–I`.
- [x] Known-future veto: `Dm7→G7→D7/A` не создаёт ложный C-major `ii–V–I`.
- [x] Target-quality guard: dominant target (`D7`) не считается tonic major только из-за root.
- [x] Не достраивать отсутствующий `I` в `I–VI–ii–V`.
- [x] First-class `iii–vi–ii–V • 1/4…4/4` для `Em7→Am7→Dm7→G7`.
- [x] Настоящий `I–VI–ii–V • 1/4…4/4` сохраняется, если I реально присутствует.
- [x] Fix1 continuity / ambiguity / SubV regression сохранены.
- [x] ~~`D7/A→Ab°→Gm7` как passingDiminished bridge к G minor~~ — гипотеза отменена и заменена fix3 rootless-dominant reading.

### 0.3f fix3 — Rootless dominant / Corcovado diminished correction [ACCEPTED VIA FIX4]

Полный scope: [STAGE_3_0.3f_FIX3_PLAN.md](STAGE_3_0.3f_FIX3_PLAN.md). Windows Build #330 был green; финальная live-проверка и regression подтверждены в составе fix4.

- [x] Удалить специальный Corcovado `passingDiminished` bridge к G minor.
- [x] Добавить host-neutral `ImpliedDominantReading`, не меняющий written chord identity.
- [x] В C major читать `Ab° = Ab–B–D–F` как contextual rootless `G7(b9)`.
- [x] Studio Pro shell `Ab–B–D` без F: provisional/implied G7(b9), medium confidence; явная F повышает до high.
- [x] Confidence dominant chain `1/2 → 2/2` соответствует shell/full voicing; written Abdim identity сохранена.
- [x] При явной E фиксировать дополнительную 13: rootless `G13(b9)`.
- [x] `D7/A → Ab°` трактовать как `V/V → implied V` / dominant chain.
- [x] Не создавать local G minor на `D7/A` или `Ab°`.
- [x] Не считать rootless G7(b9) разрешённым в C, если фактический C не наступил.
- [x] После `Ab°` позволить `Gm7→C7→Fmaj7` начать новый local F-major `ii–V–I`.
- [x] Сохранить generic enum/catalog `passingDiminished` для будущих корректных случаев.
- [x] Цветовой контракт закреплён: серый=implicit/missing, янтарный=implied/provisional, красный=contradicted.
- [x] Regression suite green в итоговом Build #331.
- [x] Live-test Corcovado/rootless chain принят пользователем 2026-09-26 в составе fix4.

### 0.3f fix4 — Незавершённые обороты / отсутствующие ступени [ACCEPTED]

Полный acceptance: [STAGE_3_0.3f_FIX4_PLAN.md](STAGE_3_0.3f_FIX4_PLAN.md).

Windows Build #331 — success; 14/14 test targets green; live-test принят Владом 2026-09-26.

- [x] Отдельное описание незавершённого ii–V–I без ложной локальной тональности.
- [x] Fm7/Bb7: позиции 1/3 и 2/3; отсутствующая I (Eb) показана серым.
- [x] Фактический Em7 сохранён; отсутствующий I не становится реальным аккордом.
- [x] Полные/carried обороты и самостоятельный iii–vi–ii–V сохраняют приоритет.
- [x] Неизвестное future не объявляется missing resolution.
- [x] Все 12 тональностей и target-quality guard покрыты regression.
- [x] Rootless chain fix3 и настоящий F-major `ii–V–I` сохранены.
- [x] Live-проверка серой I в Studio Pro и acceptance пользователя.

### 0.3g — Explanation / usable output [CURRENT]

- [ ] Показать цепочку: идея → source → важные ноты → target/resolution → почему.
- [ ] Использовать детерминированные explanations, согласованные с evidence и ограничениями; подготовить Why? data.
- [ ] Отделить вычисление результата от paint; диагностический UI читает готовые данные, Core не зависит от JUCE/ARA.
- [ ] Определить, как показывать одинаковый музыкальный материал, возникающий из нескольких interpretations, без потери provenance и без визуального дублирования.
- [ ] Использовать top-level/nested PatternContext как источник Why?-данных, не переанализируя историю в UI.
- [ ] Реализовать presentation semantics: серый implicit/missing, янтарный implied/provisional, красный contradicted.
- [ ] Rootless implied note/root отображается как implied, а не error.
- [ ] Самостоятельные patterns (`iii–vi–ii–V`) не превращаются в «неполные версии» других patterns.
- [ ] Явно маркировать уровень функции/центра в случаях вроде Gm7: global function и local F-major center не должны визуально смешиваться.

**Gate:** regression tests + отдельный build artifact + live-test в Studio Pro.

### 0.3h — Integration / musical validation

- [ ] Проверить major/minor ii–V–I, V→major/minor, secondary dominants, SubV, borrowed/modal ambiguity и boundary cases Stage 2.
- [ ] Проверить детерминированность, все 12 тональностей и enharmonic spelling; сохранить regression Stage 1–2.
- [ ] Провести live-test PLAY/STOP/seek/chord edits/reopen в Studio Pro и музыкальную проверку материала перед stable `0.4`.

**Gate:** regression tests + отдельный build artifact + live-test перед стабильной `0.4`.

## Общие правила

Минимальные explanation, diagnostics и тесты появляются с первого checkpoint; fixN исправляет контекстную семантику принятой буквы и получает собственный regression/live gate. Real-harmony validation может открыть новый fix-checkpoint до presentation layer, если выявляет системные false-positive/continuity ошибки. `0.3g` завершает подачу, `0.3h` — интеграцию.

## Stable 0.4

- Все checkpoints и fix-checkpoints приняты.
- Для согласованных базовых контекстов есть осмысленные стратегии, chord/guide/target notes и объяснимые разрешения.
- Недостаток данных и неоднозначность отражаются явно.
- Confirmed harmonic event сохраняет continuity до resolution member без stale history.
- Known future evidence не допускает уверенных patterns, уже опровергнутых фактическим продолжением.
- Inferred functional alias не уничтожает written chord identity и не создаёт ложное resolution.
- Все regression/integration checks проходят, live musical validation завершён.
- Релиз фиксирует принятое состояние без новой музыкальной логики.
