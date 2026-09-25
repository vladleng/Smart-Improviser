# Stage 3 — Improvisation Engine: рабочий план

> Обновлено 2026-09-25. База: stable 0.3, Stage 2 принят. `0.3a–0.3f` приняты; следующий checkpoint — **0.3g Explanation / usable output**. Stable остаётся 0.3.
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
- Stage 1 contract сохраняется: previous/current/next; планирование нескольких будущих тактов требует отдельного контракта.
- Сейчас invalid HarmonicSituation включает отсутствие global key/позиции/аккорда; Stage 3 не меняет это скрыто.
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
0.3g — Explanation / usable output
0.3h — Integration / musical validation
0.4  — Stage 3 complete
```

### 0.3a — Strategy contract / foundation [ACCEPTED]

Windows Build #231 — success; живой тест принят Владом 2026-09-24; PR #23 слит в main.

- [x] Расширить существующий ImprovisationStrategy: идея, source material, роли нот, targets, explanation, условия применения и ссылка на harmonic interpretation.
- [x] Определить host-neutral вход/выход, deterministic ordering и missing-data semantics; confidence анализа отделить от приоритета рекомендации.
- [x] Провести минимальную chord-tone стратегию до диагностического отображения; подготовить API для Tension Engine и будущего поиска Phrase.

### 0.3b — Structural tones / targets / resolution [ACCEPTED]

Windows Build #234 — success; живой тест принят Владом; PR #24 слит в main.

- [x] Выдавать chord tones, доступные guide tones и characteristic tones с учётом explicit extensions/alterations; не придумывать отсутствующие 3/7 у sus/triads.
- [x] Использовать ResolutionTarget Stage 2; отличать подтверждённое гармоническое разрешение от предлагаемого мелодического перехода.
- [x] Предлагать target notes реального следующего аккорда; корректно обрабатывать неизвестный next chord.

### 0.3c — Diatonic / modal sources [ACCEPTED]

Windows Build #237 success; принят Владом 2026-09-25, PR #25 merged.

- [x] Добавить согласованные diatonic/modal sources и базовые dominant sources с учётом функции, центра и аккорда.
- [x] Различать major/minor target; начальный каталог следует разделу 2 Бойко. Harmonic-minor source — последующее расширение.
- [x] Сохранять relative degrees и enharmonic spelling; Scale остаётся частью Strategy.

### 0.3d — Melodic minor / diminished sources [ACCEPTED]

Windows Build #246 — success; живой тест 0.3d fix2 принят Владом 2026-09-25; PR #26 слит в main.

- [x] Добавить согласованный ограниченный набор melodic-minor и diminished applications.
- [x] Для каждого источника определить условия применения, характерные интервалы, ограничения и разрешения.
- [x] Различать ordinary V / SubV и major/minor target; не закреплять за гаммой безусловный Tension Level.

### 0.3e — Harmonic concepts [ACCEPTED]

Музыкальная часть и структурный UI приняты Владом 2026-09-25. Windows Build #274 — success; PR #28 слит в main. UI-checkpoint `0.3e-ui` оставляет сверху компактный текущий контекст и переносит детали в разделы **Материал / Источники / Гармонический анализ / ARA**.

- [x] Реализовать chord-tone playing, guide-tone targeting и diatonic extensions как конкретные идеи.
- [x] Добавить chromatic approach / enclosure concepts с целью как символические схемы; генерация коротких нотных примеров/Phrase не вводится.
- [x] Оформить ограниченные модели мышления из начальной методики; широкий каталог upper structures, замен и альтернатив отложить.
- [x] Русский UTF-8 UI и компактная contextual summary, включая формат `D Dorian • Dm6`.

### 0.3f — Context-aware ranking / ambiguity [ACCEPTED]

Принято Владом 2026-09-25. Windows Build #284 — success. PR #30 слит в `main` squash commit `5cdc126733393a7c69e864eda9c925b146cc56f1`. Issue #29 закрыт. Полный acceptance: [STAGE_3_0.3f_LIVE_TEST.md](STAGE_3_0.3f_LIVE_TEST.md).

- [x] Собирать context-dependent diatonic/special strategies для всех валидных global/local/modal interpretations, не только для primary.
- [x] Обрабатывать `primaryInterpretationIndex = -1` без скрытого выбора трактовки; каждая strategy хранит provenance через `interpretationIndex` и evidence.
- [x] Ввести deterministic presentation ranking, сохранив harmonic confidence отдельно от recommendation priority.
- [x] Включить Issue #29: отдельный minor `iv–V–i`, boundary `iv→V`, full cadence и local-minor candidate/established semantics.
- [x] На финальном `V→I/i` без более глубокой истории не выдумывать конкретный predominant; показывать общий двухаккордовый resolution.
- [x] Устранить false-positive `C major: Fm7 → G7 → Cmaj7`: известный major tonic блокирует ложный local C minor.
- [x] Сохранить Stage 1 contract `previous/current/next` без расширения истории.
- [x] Диагностический UI не склеивает unresolved alternatives в одну строку «Мышление» и сохраняет provenance sources.
- [x] Все 12 regression targets зелёные в Windows Build #284.
- [x] Live-test принят пользователем 2026-09-25.

### 0.3g — Explanation / usable output [NEXT]

- [ ] Показать цепочку: идея → source → важные ноты → target/resolution → почему.
- [ ] Использовать детерминированные explanations, согласованные с evidence и ограничениями; подготовить Why? data.
- [ ] Отделить вычисление результата от paint; диагностический UI читает готовые данные, Core не зависит от JUCE/ARA.
- [ ] Определить, как показывать одинаковый музыкальный материал, возникающий из нескольких interpretations, без потери provenance и без визуального дублирования.

**Gate:** regression tests + отдельный build artifact + live-test в Studio Pro.

### 0.3h — Integration / musical validation

- [ ] Проверить major/minor ii–V–I, V→major/minor, secondary dominants, SubV, borrowed/modal ambiguity и boundary cases Stage 2.
- [ ] Проверить детерминированность, все 12 тональностей и enharmonic spelling; сохранить regression Stage 1–2.
- [ ] Провести live-test PLAY/STOP/seek/chord edits/reopen в Fender Studio Pro и музыкальную проверку материала перед stable 0.4.

**Gate:** проверяемый результат этих пунктов, соответствующие regression tests, отдельный build artifact и live-test перед стабильной `0.4`.

## Общие правила

Минимальные explanation, diagnostics и тесты появляются с первого checkpoint; 0.3g завершает подачу, 0.3h — интеграцию. fixN исправляет текущий checkpoint и не подменяет новую функциональную букву.

## Stable 0.4

- Все checkpoints приняты.
- Для согласованных базовых контекстов есть осмысленные стратегии, chord/guide/target notes и объяснимые разрешения.
- Недостаток данных и неоднозначность отражаются явно.
- Все regression/integration checks проходят, live musical validation завершён.
- Релиз фиксирует принятое состояние без новой музыкальной логики.
