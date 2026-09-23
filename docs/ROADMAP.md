# Smart Improviser — Roadmap

> Roadmap фиксирует последовательность разработки. Подробная схема версионирования описана в [`VERSIONING.md`](VERSIONING.md).

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

Если при живом тестировании рабочей версии обнаруживается ошибка, используется формат `fixN`, например `0.0b fix1`, `0.0b fix2`. Новая буква означает новый подэтап, а `fixN` — исправление текущего подэтапа.

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

---

## Stage 2 — Harmonic Engine

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

---

## Stage 4 — Tension Engine

**Рабочая линия:** `0.4a → 0.4x`  
**Итог Stage:** `0.5`

Реализовать три уровня:

- Tension 1 — Stable;
- Tension 2 — Color;
- Tension 3 — Outside / Maximum.

Для одного контекста должны существовать разные стратегии в зависимости от выбранного уровня.

---

## Stage 5 — Phrase Library

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

---

## Stage 6 — Phrase Transposition

**Рабочая линия:** `0.6a → 0.6x`  
**Итог Stage:** `0.7`

Сначала:

- simple transpose.

Затем:

- functional transpose.

---

## Stage 7 — Fretboard / Notation / TAB Viewer

**Рабочая линия:** `0.7a → 0.7x`  
**Итог Stage:** `0.8`

Добавить визуализацию:

- scale;
- chord tones;
- guide tones;
- tensions;
- target notes;
- phrase.

На этом этапе нужен viewer, а не полноценный редактор.

---

## Stage 8 — Phrase Editor

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

---

## Stage 9 — Phrase Transformation Engine

**Рабочая линия:** `0.9a → 0.9x`  
**Итог Stage:** `1.0`

Добавить:

- Major → Minor;
- V7 → SubV7;
- ii-V-I → related context;
- change start degree;
- change target;
- add approach notes / enclosures;
- change tension;
- preserve phrase role and melodic contour where possible.

---

## Stage 10 — Improvisation Planner

**Рабочая линия:** `1.0a → 1.0x`  
**Итог Stage:** `1.1`

Перейти от анализа отдельного аккорда к нескольким тактам и целому chorus.

Функции:

- tension curve;
- phrase role;
- development;
- climax;
- release;
- рекомендации, где использовать более сильные концепции;
- вариативность между повторными chorus.

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
