# Smart Improviser — Roadmap

> Roadmap фиксирует последовательность разработки. Код пока не пишется; текущая задача — подготовить архитектуру, документацию и backlog.

## Stage 0 — Foundation / Specification

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

Получение из DAW:

- timeline;
- playback position;
- key;
- chord events;
- tempo / time signature при необходимости.

Первая целевая DAW — Fender Studio Pro.

---

## Stage 2 — Harmonic Engine

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

Реализовать три уровня:

- Tension 1 — Stable;
- Tension 2 — Color;
- Tension 3 — Outside / Maximum.

Для одного контекста должны существовать разные стратегии в зависимости от выбранного уровня.

---

## Stage 5 — Phrase Library

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

Сначала:

- simple transpose.

Затем:

- functional transpose.

---

## Stage 7 — Fretboard / Notation / TAB Viewer

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
