# Smart Improviser — Architectural Decisions

Этот файл фиксирует решения, которые считаются базовыми до тех пор, пока они явно не пересмотрены.

## ADR-001 — Музыкальное ядро отделено от DAW

**Статус:** принято

`Smart Improviser Core` не должен зависеть от Fender Studio Pro, ARA 2, VST3 или UI.

ARA 2 является адаптером входных данных, а не частью музыкальной логики.

Причина: это упрощает тестирование и позволяет позже подключать другие DAW, ручной ввод, MIDI-анализ и standalone.

---

## ADR-002 — Центральная сущность: HarmonicSituation

**Статус:** принято

Система не строится вокруг одного `Chord`.

Главный объект — `HarmonicSituation`, который объединяет:

- current / previous / next chord;
- global / local key center;
- harmonic function;
- harmonic pattern;
- pattern position;
- resolution target;
- guide tones;
- target notes;
- tension;
- confidence.

---

## ADR-003 — Запрещена архитектура «Chord → Scale» как основная модель

**Статус:** принято

Гамма является одним из способов реализации improvisation strategy, но не основным уровнем музыкального мышления.

Целевой поток:

```text
Context → Function → Pattern → Tension → Strategy → Resolution
```

---

## ADR-004 — Tension имеет три пользовательских уровня

**Статус:** принято

На первом этапе:

1. Stable
2. Color
3. Outside / Maximum

Внутренняя модель в будущем может быть более детальной, но пользовательский интерфейс должен сохранять простую трехуровневую систему, пока не появится причина изменить её.

---

## ADR-005 — Tension зависит от разрешения

**Статус:** принято

Tension не классифицируется только по pitch class.

В перспективе учитываются:

- harmonic context;
- metric position;
- duration;
- target;
- expected resolution;
- actual resolution;
- delay before resolution.

---

## ADR-006 — Phrase хранится семантически

**Статус:** принято

Фраза не должна существовать только как массив MIDI note numbers.

Необходимо хранить относительную гармоническую роль, start/target degree, rhythm, contour, concepts, tension, context и другие метаданные.

Это является необходимым условием для будущего Phrase Transformation Engine.

---

## ADR-007 — Phrase Transformation отличается от Transpose

**Статус:** принято

Транспонирование сохраняет интервалы.

Transformation должна сохранять музыкальную идею и функциональную роль, адаптируя материал к новому контексту.

Будущие сценарии:

- major ii-V-I → minor iiø-V-i;
- V7 → SubV7;
- смена start degree;
- смена target;
- добавление enclosure;
- изменение tension.

---

## ADR-008 — Основная музыкальная логика должна быть детерминированной

**Статус:** принято

Базовые решения по функции, паттернам, tension и resolution должны быть воспроизводимыми и тестируемыми.

AI может использоваться как дополнительный слой для объяснений, semantic search и генерации упражнений, но не как единственный источник базового гармонического анализа.

---

## ADR-009 — Viewer раньше Editor

**Статус:** принято

Fretboard / Notation / TAB сначала реализуются как просмотр текущего материала.

Полноценное редактирование откладывается до момента, когда Harmonic Engine и Phrase model будут стабильны.

---

## ADR-010 — Первый MVP должен доказать музыкальный цикл

**Статус:** принято

Первый полезный MVP должен доказать:

```text
Context
→ Function
→ Pattern
→ Tension
→ Strategy
→ Resolution
```

Большая Phrase Library, AI, сложный notation editor и Improvisation Planner не являются условием первого MVP.

---

## ADR-011 — Автоматические тесты обязательны для музыкального ядра

**Статус:** принято

По мере роста числа правил обязательны тесты как минимум для:

- chord parsing;
- harmonic function;
- harmonic patterns;
- resolution targets;
- tension strategies;
- phrase transpose;
- phrase transformation.

Это нужно, чтобы добавление новых правил не ломало уже работающие гармонические ситуации.

---

## ADR-012 — Global project key и Local Key Center являются разными слоями

**Статус:** принято

Тональность проекта, полученная от DAW, является **global key** и не должна автоматически переписываться Harmonic Engine при каждом временном отклонении.

Джазовая гармония часто содержит кратковременные tonicization, local ii–V, secondary dominants, tritone substitutions и локальные центры, не требующие смены project key.

Поэтому `HarmonicSituation` хранит параллельно:

```text
Global key
Local key center
Global harmonic interpretation
Local harmonic interpretation
Global pattern
Local pattern
```

Local Key Center может находиться в состояниях:

```text
candidate
→ tonicized / temporary
→ established local center
→ modulationCandidate
```

`modulationCandidate` не означает автоматическую модуляцию и никогда сам не меняет explicit global key. Он только фиксирует evidence, что видимое окно лучше объясняется новой локальной тональностью.

При полном локальном `ii–V–I`, `iiø–V–i` или `ii–SubV–I` local interpretation имеет собственную harmonic function и pattern. Конфликты между global и local interpretations должны разрешаться отдельным Ambiguity / Confidence layer, а не скрытым перезаписыванием одного анализа другим.

Причины:

- это соответствует реальному восприятию jazz harmony;
- пользователь не должен вручную менять тональность проекта при каждом отклонении;
- global key остаётся стабильной опорой формы;
- local function полезнее для импровизации над временными центрами;
- архитектура остаётся детерминированной и тестируемой.

---

# Как обновлять этот файл

При появлении важного решения добавляется новый ADR:

```text
ADR-NNN — Название
Статус: proposed / accepted / superseded
Контекст
Решение
Причины
Последствия
```

Если решение пересмотрено, старый ADR не удаляется: он помечается как `superseded` и содержит ссылку на новое решение.
