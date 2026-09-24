# Smart Improviser — Project Context

> **Статус:** living document  
> **Версия документа:** 0.2 (уточнение workflow 2026-09-24; версия плагина остаётся 0.3)  
> **Назначение:** единый архитектурный и продуктовый контекст проекта для GitHub и переноса между чатами.

## 1. Идея

Smart Improviser — VST3/ARA 2 плагин для DAW, предназначенный для помощи гитаристу в джазовой импровизации.

Основная идея — уйти от примитивной модели `Chord → Scale`. Плагин должен понимать музыкальную ситуацию целиком: тональность, текущий аккорд, его функцию, соседние аккорды, гармонический оборот, ожидаемое разрешение и драматургическую степень напряжения.

Целевая формула:

```text
Harmony
+
Context
+
Function
+
Tension
+
Resolution
+
Vocabulary
+
Transformation
=
Smart Improviser
```

## 1.1. Центральный пользовательский workflow

Центральное пространство — работа над конкретной песней в проекте DAW. Пользователь фиксирует идею на выбранном такте в нотах/TAB, подбирает материал из общей/личной библиотеки, дорабатывает и сохраняет работу для следующей сессии. Собственные и сохранённые из общей библиотеки идеи могут становиться личными вариантами и применяться в других песнях.

Подсказка, исследование, личный vocabulary и конструктор являются постоянно доступными способами работы, не уровнями tension. Три уровня tension управляют рекомендациями внутри этих сценариев.

Общая библиотека, пользовательская библиотека и размещения материала в песне разделены. Редактирование экземпляра не меняет источник или другие песни; происхождение сохраняется. Переключение tension не переписывает записанный материал.

Подробный согласованный сценарий, модель содержания, порядок Stage и открытые технические решения: [PRODUCT_WORKFLOW.md](PRODUCT_WORKFLOW.md). Рабочий план следующего этапа: [STAGE_3_PLAN.md](STAGE_3_PLAN.md).

## 2. Главный пользовательский вопрос

Плагин должен отвечать не только:

> Что здесь можно сыграть?

но и:

> Что музыкально уместно сыграть именно здесь, зачем, куда это разрешить и с какой степенью напряжения?

## 3. Первая целевая среда

- DAW: Fender Studio Pro
- Интеграция: ARA 2
- Формат: VST3
- Платформа: Windows
- Инструментальный фокус: гитара
- Стили: jazz / fusion / related improvisational music

## 4. Базовая архитектура

```text
DAW / ARA 2
    ↓
Timeline Context
    ↓
Harmonic Analyzer
    ↓
Harmonic Situation
    ↓
Tension Engine
    ↓
Improvisation Strategy
    ↓
Phrase / Scale / Concept Engine
    ↓
Notation / TAB / Fretboard / UI
```

Музыкальное ядро должно быть отделено от DAW, ARA, VST3 и UI.

```text
Fender Studio Pro
        ↓
ARA 2 Adapter
        ↓
Smart Improviser Core
```

Это позволит позже подключать другие DAW, ручной ввод аккордов, MIDI-анализ, standalone и другие интерфейсы без переписывания музыкальной логики.

## 5. Центральная сущность — HarmonicSituation

Главным объектом системы должен быть не просто `Chord`, а `HarmonicSituation`.

Пример:

```text
Key: C major
LocalKeyCenter: C major
CurrentChord: G7
Function: V7
PreviousChord: Dm7
NextChord: Cmaj7
Pattern: Major ii-V-I
PatternPosition: V
ResolutionTarget: Cmaj7
TensionLevel: 2
PhraseRole: Development / preparation for resolution
```

Предварительные поля:

```text
HarmonicSituation
├── currentChord
├── previousChord
├── nextChord
├── globalKey
├── localKeyCenter
├── harmonicFunction
├── harmonicPattern
├── patternPosition
├── resolutionTarget
├── availableTensions
├── guideTones
├── targetNotes
├── tensionLevel
├── phraseRole
└── confidence
```

## 6. Базовые сущности ядра

### Chord

```text
Chord
├── root
├── quality
├── extensions
├── alterations
├── bassNote
└── pitchClasses
```

### KeyCenter

```text
KeyCenter
├── tonic
├── mode
├── confidence
└── scope
```

### HarmonicFunction

Примеры:

```text
Imaj7
ii7
V7
V/ii
V/V
SubV/I
iv minor
bVII7
borrowed chord
passing diminished
common-tone diminished
modal color
```

### HarmonicPattern

Стартовые паттерны:

```text
Major ii-V-I
Minor iiø-V-i
V-I
I-VI-ii-V
Secondary dominant
Tritone substitution
Backdoor dominant
Minor iv → I
Passing diminished
Dominant chain
Turnaround
Modal vamp
```

### ResolutionTarget

Описывает ожидаемое направление разрешения.

Пример `G7 → Cmaj7`:

```text
B → C
F → E
Ab → G
Db → C
Eb → E
```

### ImprovisationStrategy

Стратегия стоит выше конкретной гаммы.

Примеры:

```text
Chord-tone based dominant
Guide-tone resolution
Diatonic dominant color
Lydian dominant color
Altered dominant tension
Diminished dominant language
Tritone superimposition
Chromatic enclosure language
Side-slip outside phrase
```

## 7. Harmonic Context Engine

Каждый аккорд должен анализироваться на нескольких уровнях:

1. локальный состав аккорда;
2. функция внутри тональности;
3. роль внутри гармонического оборота;
4. направление движения и ожидаемое разрешение.

Один и тот же `G7` может означать `V7` в C major, `V/IV` в G major, secondary dominant или dominant-color chord без классического разрешения. Поэтому абсолютного имени аккорда недостаточно.

## 8. Tension Engine

Главная отличительная особенность — три уровня напряжения.

### Tension 1 — Stable

- chord tones;
- guide tones;
- consonant extensions;
- arpeggios;
- diatonic passing tones;
- простые approaches;
- ясное проведение гармонии.

### Tension 2 — Color

- chromatic approaches;
- enclosures;
- melodic minor applications;
- lydian dominant;
- upper structures;
- controlled alterations;
- substitutions;
- характерный jazz color.

### Tension 3 — Outside / Maximum

- altered dominant;
- diminished language;
- side slipping;
- superimposed harmony;
- tritone concepts;
- symmetrical structures;
- substitutions by minor thirds;
- delayed resolution;
- dense chromaticism.

Важно: tension — не просто свойство отдельной ноты. Он зависит от гармонического контекста, метрической позиции, длительности, направления и разрешения.

## 9. Tension Curve и Tension Profile

В будущем tension должен описывать драматургию нескольких тактов:

```text
1 → 1 → 2 → 3 → 2 → 1
```

Отдельная фраза тоже может иметь внутренний `TensionProfile`, что позволит позже выполнять операции вроде:

- сделать фразу спокойнее;
- повысить напряжение;
- перенести кульминацию;
- усилить доминантовое разрешение;
- уменьшить/увеличить хроматизм.

## 10. Общая и пользовательская Phrase / Idea Library

Фразы нельзя хранить только как абсолютный MIDI. Нужна семантическая модель.

Пользовательская библиотека хранит собственные идеи, копии общей библиотеки и переработки. Source identity/version и происхождение варианта сохраняются. Незаконченная Idea допускает частичные ноты, ритм и текст без обязательного полного tagging.

Library item отделён от размещённого в песне экземпляра. В Song workspace хранятся позиции/диапазоны, локальное содержимое, заметки, альтернативы и выбранный tension участка. Сохранение песни должно восстанавливать материал даже без библиотечного источника. Точные типы, anchors и persistence backend определяются до реализации соответствующих Stage.

Пример:

```text
Context: Major ii-V-I
Relative harmony: ii7 | V7 | Imaj7
Start: 5th of ii
Target: 3rd of I
Tension: 2
Concepts:
- chromatic approach
- dominant enclosure
- guide-tone resolution
PhraseRole: Development → Resolution
```

Предварительная структура `Phrase`:

```text
Phrase
├── id
├── name
├── source
├── sourceLicense
├── author
├── tags
├── harmonicContext
├── relativeHarmony
├── startDegree
├── endDegree
├── targetNotes
├── tensionLevel
├── tensionProfile
├── phraseRole
├── concepts
├── rhythm
├── melodicContour
├── intervalStructure
├── relativeNotes
├── absoluteExample
├── scales
├── substitutions
├── explanations
└── guitarFingerings
```

При наполнении базы необходимо учитывать авторские права: предпочтительны собственные фразы, public domain, лицензированные материалы и абстрактные vocabulary-паттерны.

## 11. Phrase Transformation Engine

Долгосрочно система должна уметь переносить не только ноты, но и музыкальную логику.

Уровни трансформации:

1. **Simple Transpose** — перенос в другую тональность.
2. **Functional Transpose** — перенос `ii-V-I` в другую тональность с сохранением функций.
3. **Major ↔ Minor Adaptation** — `ii-V-I` ↔ `iiø-V-i`.
4. **Harmonic Substitution Adaptation** — например `V7 → SubV7`.
5. **Context Remapping** — перенос идеи в родственный, но другой оборот.
6. **Modal Adaptation** — перенос между ладами (например Dorian → Aeolian) по явно выбранным invariants: ступени, контур, targets и характер идеи. Конкретные правила и поддерживаемые пары согласуются на Stage 9.

Трансформация запускается явно, показывает результат и сохраняет происхождение. Смена tension или harmony сама по себе не переписывает пользовательскую Phrase.

При адаптации необходимо по возможности сохранять:

- rhythm;
- melodic contour;
- phrase role;
- target logic;
- voice-leading intention;
- характер фразы.

## 12. Target Notes и Approach Notes

Плагин должен явно показывать целевые ноты и разрешения.

Пользователь сможет выбирать цель:

```text
root
3rd
5th
7th
9th
specific guide tone
```

Для target note система должна уметь предлагать:

```text
chromatic below
chromatic above
double chromatic
diatonic approach
enclosure
extended enclosure
```

Выбор подхода зависит от аккорда, метрической позиции, tension и роли фразы.

## 13. Scale / Concept Engine

Гамма — не верхний уровень логики, а один из способов реализовать стратегию.

Приведённые ниже и в разделе 8 соответствия источников уровням tension — ориентиры для проектирования, а не безусловные правила. Stage 4 должен учитывать explicit chord, функцию и реальное разрешение. Stage 3 сначала формирует допустимые стратегии и их условия.

Пример для `G7 → Cmaj7`:

```text
T1 → Chord tones / Mixolydian
T2 → Lydian dominant / melodic-minor color
T3 → Altered / diminished / stronger substitutions
```

UI должен показывать цепочку:

```text
Strategy
→ Scale / source structure
→ Important notes
→ Resolution
→ Why it works
```

## 14. Explanation Engine

Каждая рекомендация должна быть объяснима.

Пример:

```text
G7 является V7 в C major.
Он разрешается в Cmaj7.
Level 3 допускает altered dominant language.
Ab melodic minor относительно G дает b9, #9, b5/#11 и b13.
Ключевые разрешения: B→C, F→E, Ab→G, Db→C, Eb→E.
```

На ранних этапах Explanation Engine желательно делать детерминированным и основанным на правилах, а не на AI.

## 15. Fretboard / Notation / TAB

В первых версиях достаточно Viewer, а не полноценного редактора.

Режимы:

```text
Scale
Chord tones
Guide tones
Tensions
Targets
Phrase
```

Главная идея: пользователь видит не просто расположение нот, а их функцию в текущем контексте.

Позже редактор должен поддерживать создание пользовательских фраз, ритм, TAB, fingerings, context assignment, tension assignment и трансформации.

## 15.1. Работа над песней / ручной конструктор

Stage 7 добавляет read-only отображение нот/TAB и минимальный сохраняемый Song workspace: размещение готового материала, заметки, варианты и настройки участков. Stage 8 добавляет быстрый нотный/TAB ввод на выбранном такте и ручную сборку: редактирование, перемещение, копирование, соединение и сравнение вариантов.

Это не требует готового Stage 10 Planner. Planner позже предлагает развитие, tension curve и кульминации поверх сохранённой ручной работы.

Stage 5 уже должен давать минимальный способ сохранить собственную идею и переработку библиотечного материала; полноценный графический редактор появляется позже.

## 16. MVP

Первый реально полезный интерфейс может быть очень простым:

```text
KEY
C major

CURRENT
G7

FUNCTION
V7

CONTEXT
Dm7 → G7 → Cmaj7

PATTERN
ii → V → I

TENSION
[1] [2] [3]

STRATEGY
Altered dominant

SOURCE
Ab melodic minor

TARGETS
B → C
F → E

WHY
V7 resolving to I
```

Даже без библиотеки фраз такой MVP уже будет полезен.

## 17. Автоматические тесты

Музыкальное ядро должно быть покрыто тестами.

Минимальные категории:

- chord parsing;
- function detection;
- pattern recognition;
- target-note generation;
- resolution detection;
- tension classification;
- phrase transposition;
- phrase transformation.

Пример:

```text
Input:
C major
Dm7 | G7 | Cmaj7

Expected:
Dm7 = ii
G7 = V
Cmaj7 = I
Pattern = Major ii-V-I
```

## 18. Детерминированность и AI

Основную музыкальную логику нужно строить на правилах, таблицах, формальных структурах и тестируемых алгоритмах.

AI в будущем может использоваться для:

- естественных объяснений;
- semantic tagging;
- поиска похожих фраз;
- генерации упражнений;
- помощи при создании вариантов.

Но AI не должен быть единственным источником ответа на вопрос о гармонической функции или базовой логике.

## 19. Что не включать в первые версии

Не следует сразу пытаться реализовать:

- полноценный notation editor;
- сложную AI-генерацию;
- анализ аудио;
- автоматическую транскрипцию соло;
- огромную библиотеку фраз;
- сложную драматургию chorus;
- поддержку всех DAW;
- генерацию законченного соло.

Сначала необходимо доказать основной цикл:

```text
Context
→ Function
→ Pattern
→ Tension
→ Strategy
→ Resolution
```

## 20. Долгосрочное видение

Smart Improviser должен постепенно переводить пользователя от вопроса:

```text
Какая здесь гамма?
```

к вопросам:

```text
Какую функцию выполняет этот аккорд?
Какое напряжение я хочу создать?
Куда должна разрешиться идея?
Как развить её через несколько тактов?
Как построить драматургию всей импровизации?
```

Финальная цель — не генерировать музыку вместо музыканта, а помогать музыканту понимать контекст, управлять tension и превращать vocabulary в собственный язык.

## 21. Правило обновления

Этот файл является **living document**.

Сюда следует вносить:

- новые архитектурные решения;
- изменения терминологии;
- новые harmonic patterns;
- новые правила tension;
- правила Phrase Transformation;
- решения по UI;
- результаты тестов и экспериментов;
- ссылки на важные Issues и Milestones.

Крупные решения также фиксируются в `ARCHITECTURAL_DECISIONS.md`.
