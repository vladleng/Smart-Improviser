# Smart Improviser — Roadmap

> Roadmap фиксирует последовательность разработки. Подробная схема версионирования описана в [`VERSIONING.md`](VERSIONING.md).

## Продуктовый workflow и статус

Stable **0.3**: Stage 0–2 завершены. Текущий этап — Stage 3, 0.3a и 0.3b приняты; начат checkpoint 0.3c.

Центральный сценарий — работа над конкретной песней: быстро сохранить идею на такте, взять подходящий материал из общей/личной библиотеки, доработать, сохранить проект и продолжить. Подсказки по выбранному tension доступны независимо от наличия готовой фразы. Подробнее: [PRODUCT_WORKFLOW.md](PRODUCT_WORKFLOW.md).

Нумерация Stage и stable targets сохраняется. Song workspace появляется в Stage 7, ручной нотный/TAB конструктор — в Stage 8; интеллектуальная драматургия остаётся Stage 10.

## Общее правило разработки Stage

Каждый Stage делится на **логические подэтапы**, и каждому такому подэтапу соответствует отдельная буквенная build-версия.

Принцип:

```text
Stage N
→ N.xa — подэтап A
→ N.xb — подэтап B
→ N.xc — подэтап C
→ ...
→ следующая стабильная числовая версия — Stage complete
```

Буквенная версия является самостоятельным build checkpoint: у неё должна быть понятная цель, проверяемый результат, checklist и набор тестов. Новая буква означает новую функциональную часть Stage. Исправления уже существующего подэтапа получают `fixN`.

Количество подэтапов может уточняться по мере разработки, но Stage не должен сворачиваться в одну большую рабочую версию.

## Схема версий по Stage

| Stage | Рабочие версии | Итог Stage |
|---|---|---|
| Stage 0 | `0.0a`, `0.0b`, ... | `0.1` |
| Stage 1 | `0.1a`, `0.1b`, ... | `0.2` |
| Stage 2 | `0.2a`, `0.2b`, ... | `0.3` |
| Stage 3 | `0.3a`, `0.3b`, ... | `0.4` |
| Stage 4 | `0.4a`, `0.4b`, ... | `0.5` |
| Stage 5 | `0.5a`, `0.5b`, ... | `0.6` |
| Stage 6 | `0.6a`, `0.6b`, ... | `0.7` |
| Stage 7 | `0.7a`, `0.7b`, ... | `0.8` |
| Stage 8 | `0.8a`, `0.8b`, ... | `0.9` |
| Stage 9 | `0.9a`, `0.9b`, ... | `1.0` |
| Stage 10 | `1.0a`, `1.0b`, ... | `1.1` |

Если при живом тестировании рабочей версии обнаруживается ошибка, используется формат `fixN`, например `0.2b fix1`, `0.2b fix2`. Новая буква означает новый подэтап, а `fixN` — исправление текущего подэтапа.

---

## Stage 0 — Foundation / Specification

**Рабочая линия:** `0.0a → 0.0x`  
**Итог Stage:** `0.1`

Цель: формально описать музыкальное ядро до начала активной реализации.

Нужно определить:

- `Chord`
- `KeyCenter`
- `HarmonicFunction`
- `HarmonicPattern`
- `HarmonicSituation`
- `ResolutionTarget`
- `TensionLevel`
- `ImprovisationStrategy`
- `Phrase`

Результат:

- согласованная data model;
- архитектурные границы Core / ARA / UI;
- перечень базовых тест-кейсов;
- первая спецификация Smart Improviser Core v0.1.

---

## Stage 1 — ARA Context Monitor

**Рабочая линия:** `0.1a → 0.1x`  
**Итог Stage:** `0.2`

Получение из DAW:

- timeline;
- playback position;
- key;
- chord events;
- tempo / time signature при необходимости.

Первая целевая DAW — Fender Studio Pro.

Stage завершён версией `0.2`.

---

## Stage 2 — Harmonic Engine

**Статус:** завершён и принят, stable 0.3; Issue #3 закрыт. Нижеследующая декомпозиция сохранена как история этапа.

**Рабочая линия:** `0.2a → 0.2x`  
**Итог Stage:** `0.3`

Добавить:

- chord parsing;
- global key;
- local key center;
- basic harmonic functions;
- previous/current/next chord analysis;
- basic harmonic pattern recognition.

Первый набор паттернов:

- major ii-V-I;
- minor iiø-V-i;
- V-I;
- I-VI-ii-V;
- secondary dominant;
- tritone substitution.

### Логические подэтапы Stage 2

Текущий план:

```text
0.2a — Harmonic Engine foundation
0.2b — Pattern Recognizer
0.2c — Tritone Substitution
0.2d — Local Key Center
0.2e — Ambiguity / Confidence
0.2f — Stage 2 integration / musical validation
0.3  — Stage 2 complete
```

#### 0.2a — Harmonic Engine foundation

- отдельный host-neutral `HarmonicEngine`;
- реальный `TimelineHarmonicSnapshot` → `HarmonicSituation`;
- global key;
- previous/current/next context;
- basic harmonic function;
- resolution;
- первые major `ii–V–I`, minor `iiø–V–i`, `V–I`;
- confirmed secondary dominant;
- минимальный temporary local center;
- regression tests.

#### 0.2b — Pattern Recognizer

- pattern positions для `ii / V / I`;
- анализ current chord в разных позициях оборота;
- `I–VI–ii–V`;
- secondary-dominant chains;
- pattern evidence/confidence;
- boundary cases.

#### 0.2c — Tritone Substitution

- `SubV`;
- `ii–SubV–I`;
- отличие `V7` от `SubV7`;
- resolution logic для substitute dominant;
- regression tests.

#### 0.2d — Local Key Center

- temporary tonicization;
- короткие local centers;
- отличие tonicization от modulation;
- возврат к global key;
- secondary dominants внутри local context;
- тесты tonicization chains.

#### 0.2e — Ambiguity / Confidence

- `unique / ambiguous` interpretations;
- confidence levels;
- alternative interpretation candidates;
- borrowed/modal ambiguity;
- controlled evidence rules.

#### 0.2f — Integration / musical validation

- комплексные harmonic sequences;
- regression всего Stage 2;
- diagnostics/UI для наблюдения анализа;
- live validation в Fender Studio Pro;
- подготовка стабильной `0.3`.

План `0.2b–0.2f` может уточняться по мере разработки. Если появляется новая самостоятельная музыкальная или техническая задача, она получает отдельную следующую буквенную версию, а не добавляется скрыто внутрь уже принятого подэтапа.

---

## Stage 3 — Improvisation Engine

**Рабочая линия:** `0.3a → 0.3x`  
**Итог Stage:** `0.4`

Для каждого `HarmonicSituation` выдавать:

- chord tones;
- guide tones;
- target notes;
- scales;
- harmonic concepts;
- resolution notes;
- базовые improvisation strategies.

Пока без большой библиотеки готовых фраз.

Рабочая декомпозиция (обновлено 2026-09-24):

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

Цели и gates каждой буквы: [STAGE_3_PLAN.md](STAGE_3_PLAN.md), Issue #4. Начальная методика согласована: [IMPROVISATION_METHOD.md](IMPROVISATION_METHOD.md). 0.3a и 0.3b приняты; начат 0.3c. Принятие каждой буквы отдельно.

Результат стратегии нужен как для самостоятельной подсказки, так и для будущего поиска Phrase. Используются существующие ImprovisationStrategy и ResolutionTarget; confidence анализа отделён от приоритета рекомендации.

---

## Stage 4 — Tension Engine

**Рабочая линия:** `0.4a → 0.4x`  
**Итог Stage:** `0.5`

Реализовать три уровня:

- Tension 1 — Stable;
- Tension 2 — Color;
- Tension 3 — Outside / Maximum.

Для одного контекста должны существовать разные стратегии в зависимости от выбранного уровня.

Выбранный tension направляет подсказки и поиск. Смена уровня не переписывает сохранённые ноты/ритм. Желаемый tension участка и описательный tension/profile фразы разделены; явное преобразование материала относится к Stage 9.

Перед началом Stage 4 определяется отдельный план буквенных подэтапов.

---

## Stage 5 — Общая / пользовательская Phrase & Idea Library

**Рабочая линия:** `0.5a → 0.5x`  
**Итог Stage:** `0.6`

Добавить базу фраз с метаданными:

- harmonic context;
- function;
- tension;
- target notes;
- source / license;
- explanation;
- scales / source structures;
- concepts;
- relative representation.

Дополнительно:
- общая и пользовательская библиотеки, копирование общих материалов и сохранение своих идей;
- варианты, source identity/version и происхождение переработок;
- минимальный ввод/сохранение незавершённой Idea без обязательного полного нотного редактора;
- поиск по контексту и выбранному tension с объяснением совместимости;
- контракт независимого экземпляра в песне, anchors и версионированной persistence для Stage 7.

Минимальный способ ввода своей идеи согласуется до Stage 5. Полный нотный/TAB редактор остаётся Stage 8. Перед началом Stage 5 определяется отдельный план буквенных подэтапов.

---

## Stage 6 — Phrase Transposition

**Рабочая линия:** `0.6a → 0.6x`  
**Итог Stage:** `0.7`

Сначала:

- simple transpose.

Затем:

- functional transpose.

Перенос применяется к новому/локальному экземпляру без изменения источника. Подготовить применение материала к целевому участку песни и явное сообщение о несовместимом контексте. Смена лада/функциональной идеи относится к Stage 9.

Эти и дополнительные задачи Stage 6 должны быть распределены по отдельным буквенным build checkpoints.

---

## Stage 7 — Viewer / Song Workspace

**Рабочая линия:** `0.7a → 0.7x`  
**Итог Stage:** `0.8`

Добавить визуализацию:

- scale;
- chord tones;
- guide tones;
- tensions;
- target notes;
- phrase.

На этом этапе нужен viewer, а не полноценный нотный редактор.

Добавить минимальный сохраняемый Song workspace:
- song/project identity и musical anchors с отображением bar/beat;
- размещение готового материала на участке, локальные экземпляры, заметки и варианты;
- отображение гармонии и подсказок для выбранного участка/tension;
- сохранение и reopen с восстановлением содержимого независимо от доступности библиотечного источника;
- явное поведение при изменении harmony/tempo/time signature; правила структурных правок DAW согласовать и проверить.

Полноценное редактирование нот/ритма — Stage 8. Схема хранения (DAW state / файл / сочетание) выбирается до реализации persistence.

Перед началом Stage 7 определяется отдельный план буквенных подэтапов.

---

## Stage 8 — Phrase Editor / ручной конструктор импровизации

**Рабочая линия:** `0.8a → 0.8x`  
**Итог Stage:** `0.9`

Добавить:

- создание пользовательских фраз;
- редактирование нот и ритма;
- TAB;
- fingerings;
- сохранение;
- tagging;
- context assignment;
- tension assignment.

В контексте песни:
- быстрый ввод нот/TAB прямо на выбранном такте, включая незавершённый набросок;
- вставка материала общей/личной библиотеки, локальная доработка и явное сохранение варианта в user library;
- ручная сборка: копирование, перемещение, соединение, альтернативы на одном участке;
- проверка стыков и targets без автоматической генерации законченного соло;
- сохранение/восстановление всей работы над песней и Undo/redo правок.

Перед началом Stage 8 определяется отдельный план буквенных подэтапов.

---

## Stage 9 — Phrase Transformation Engine

**Рабочая линия:** `0.9a → 0.9x`  
**Итог Stage:** `1.0`

Добавить:

- Major → Minor;
- modal adaptation (например Dorian → Aeolian) с явно определёнными invariants и согласованным набором mappings;
- V7 → SubV7;
- ii-V-I → related context;
- change start degree;
- change target;
- add approach notes / enclosures;
- change tension;
- preserve phrase role and melodic contour where possible.

Преобразования применяются явно, сохраняют источник/происхождение и предоставляют результат для сравнения. Переключение tension не запускает transformation автоматически.

Перед началом Stage 9 определяется отдельный план буквенных подэтапов.

---

## Stage 10 — Improvisation Planner

**Рабочая линия:** `1.0a → 1.0x`  
**Итог Stage:** `1.1`

Добавить интеллектуальную помощь с драматургией нескольких тактов и целого chorus поверх сохраняемого Song workspace Stage 7 и ручного конструктора Stage 8. Базовое размещение/сохранение идей не откладывается до Planner.

Функции:

- tension curve;
- phrase role;
- development;
- climax;
- release;
- рекомендации, где использовать более сильные концепции;
- вариативность между повторными chorus.

Перед началом Stage 10 определяется отдельный план буквенных подэтапов.

---

# Приоритеты

Основной порядок ценности:

```text
1. Correct harmonic context
2. Correct harmonic function
3. Correct pattern recognition
4. Useful tension strategies
5. Clear resolution logic
6. Phrase vocabulary
7. Phrase transformation
8. Advanced editing
9. Improvisation planning
```

Если первые пять пунктов работают плохо, более поздние функции теряют музыкальный смысл.

# Что сознательно откладывается

На ранних этапах не приоритетны:

- анализ аудио;
- автоматическая транскрипция соло;
- AI-генерация законченного соло;
- поддержка всех DAW;
- большая облачная инфраструктура;
- полноценный notation editor;
- сложное планирование chorus до того, как доказана ценность базового Harmonic/Tension Engine.
